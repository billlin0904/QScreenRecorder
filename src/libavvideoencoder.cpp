#include <QDebug>
#include <vector>

#include <qsystemdetection.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>
#include <libavutil/opt.h>
}

#include <libyuv.h>
#include "libavvideoencoder.h"

static const char* presetOptions(Preset preset) {
	switch (preset) {
	case Preset::PRESET_ULTRAFAST:
		return "ultrafast";
	case Preset::PRESET_SUPER_FAST:
		return "superfast";
	case Preset::PRESET_VERY_FAST:
		return "veryfast";
	case Preset::PRESET_FASTER:
		return "faster";
	case Preset::PRESET_FAST:
		return "fast";
	case Preset::PRESET_MEDIUM:
		return "medium";
	case Preset::PRESET_SLOW:
		return "slow";
	case Preset::PRESET_SLOWER:
		return "slower";
	case Preset::PRESET_VERY_SLOW:
		return "veryslow";
	case Preset::PRESET_PLACE_BO:
		return "placebo";
	default:
		return "fast";
	}
}

LibavException::LibavException(const char* message)
	: error_(0)
	, message_(message) {
}

LibavException::LibavException(int error)
	: error_(error) {
	char msgbuf[1024] = { 0 };
	if (av_strerror(error, msgbuf, sizeof(msgbuf)) == 0) {
		message_ = msgbuf;
	} else {
		message_ = "";
	}
}

int LibavException::error() const noexcept {
	return error_;
}

const char* LibavException::what() const noexcept {
	return message_.c_str();
}

#define LIBAV_IF_FAILED_THROW(error) \
	if ((error) < 0) \
		throw LibavException(error);

#define LIBAV_IF_NULL_THROW(expr) \
	if ((expr) == nullptr) \
		throw LibavException(#expr " is nullptr.");

#define US_LIBYUV 1

class LibavInit {
public:
    static LibavInit& get() {
        static LibavInit instance;
        return instance;
    }

private:
    LibavInit() {
        av_register_all();
        avcodec_register_all();
    }
};

class LibavVideoEncoder::LibavVideoEncoderImpl {
public:
	LibavVideoEncoderImpl()
        : frame_count_(0)
		, format_context_(nullptr)
		, format_(nullptr)
        , video_stream_(nullptr)
        , scale_frame_(nullptr)
		, src_frame_(nullptr)
		, video_codec_context_(nullptr) {
    }

    ~LibavVideoEncoderImpl() {
        close();
    }

    void close() {
        if (format_context_ != nullptr) {
            av_write_trailer(format_context_);
        }
		
        if (video_codec_context_ != nullptr) {
            avcodec_close(video_stream_->codec);
			video_codec_context_ = nullptr;
        }

        if (scale_frame_ != nullptr) {
			av_free(scale_frame_->data[0]);
			av_frame_free(&scale_frame_);
            scale_frame_ = nullptr;
        }

		if (src_frame_ != nullptr) {
			av_frame_free(&src_frame_);
			src_frame_ = nullptr;
		}
		
        if (format_context_ != nullptr) {
			avio_close(format_context_->pb);
            avformat_free_context(format_context_);
            format_context_ = nullptr;
        }
    }

	void open(const std::string& file_name, int width, int height, Preset preset, int bit_rate, int fps) {
		LibavInit::get();
		close();		
		openVideoStream(file_name, width, height, preset, bit_rate, fps);
		openAudioStream();
	}

    void writeVideoFrame(const uint8_t* bits, int bytes_per_line) {
		src_frame_->data[0] = (uint8_t*)bits;
		src_frame_->data[1] = nullptr;
		src_frame_->data[2] = nullptr;

		src_frame_->linesize[0] = bytes_per_line;
		src_frame_->linesize[1] = 0;
		src_frame_->linesize[2] = 0;

#if US_LIBYUV
#if 0
        libyuv::RGB24ToI420(src_frame_->data[0],
			src_frame_->linesize[0],
			scale_frame_->data[0],
			scale_frame_->linesize[0],
			scale_frame_->data[1],
			scale_frame_->linesize[1],
			scale_frame_->data[2],
			scale_frame_->linesize[2],
			video_codec_context_->width,
			video_codec_context_->height);
#else
		libyuv::ARGBToI420(src_frame_->data[0],
			src_frame_->linesize[0],
			scale_frame_->data[0],
			scale_frame_->linesize[0],
			scale_frame_->data[1],
			scale_frame_->linesize[1],
			scale_frame_->data[2],
			scale_frame_->linesize[2],
			video_codec_context_->width,
			video_codec_context_->height);
#endif
#else
		LIBAV_IF_FAILED_THROW(sws_scale(sws_context_,
			src_frame_->data,
			src_frame_->linesize,
			0,
			video_codec_context_->height,
			scale_frame_->data,
			scale_frame_->linesize));
#endif

		scale_frame_->pts = frame_count_;

        AVPacket pkt = { 0 };    
        av_init_packet(&pkt);

		int got_packet = 0;
		LIBAV_IF_FAILED_THROW(avcodec_encode_video2(video_codec_context_, &pkt, scale_frame_, &got_packet));

        if (got_packet) {
            pkt.pts = av_rescale_q_rnd(pkt.pts,
                                       video_codec_context_->time_base,
                                       video_stream_->time_base,
                                       AVRounding(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pkt.dts = av_rescale_q_rnd(pkt.dts,
                                       video_codec_context_->time_base,
                                       video_stream_->time_base,
                                       AVRounding(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pkt.duration = av_rescale_q(pkt.duration,
                                        video_codec_context_->time_base,
                                        video_stream_->time_base);
			pkt.stream_index = video_stream_->index;
			LIBAV_IF_FAILED_THROW(av_interleaved_write_frame(format_context_, &pkt));
			av_packet_unref(&pkt);
        }
		++frame_count_;
    }

private:
	void openAudioStream(int sample_rate = 48000) {
		audio_codec_ = avcodec_find_encoder(format_->audio_codec);

		audio_stream_ = avformat_new_stream(format_context_, audio_codec_);
		LIBAV_IF_NULL_THROW(audio_stream_);

		audio_stream_->id = format_context_->nb_streams - 1;

		audio_codec_context_ = audio_stream_->codec;
		LIBAV_IF_NULL_THROW(audio_codec_context_);

        audio_codec_context_->strict_std_compliance = -2;

		audio_codec_context_->channel_layout = AV_CH_LAYOUT_STEREO;
		audio_codec_context_->sample_fmt = audio_codec_->sample_fmts ? audio_codec_->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
		audio_codec_context_->sample_rate = sample_rate;
		audio_codec_context_->channels = 2;
		LIBAV_IF_FAILED_THROW(avcodec_open2(audio_codec_context_, audio_codec_, nullptr));

#define CODEC_CAP_VARIABLE_FRAME_SIZE 0x10000
		auto src_nb_samples = audio_codec_context_->codec->capabilities
			& CODEC_CAP_VARIABLE_FRAME_SIZE ?
			10000 : audio_codec_context_->frame_size;

		auto audio_samples_size = av_samples_get_buffer_size(nullptr,
			audio_codec_context_->channels, 
			src_nb_samples,
			audio_codec_context_->sample_fmt, 
			0);

		audio_frame_ = av_frame_alloc();
		LIBAV_IF_NULL_THROW(audio_frame_);
	}

    void openVideoStream(const std::string& file_name, int width, int height, Preset preset, int bit_rate, int fps) {
		LIBAV_IF_FAILED_THROW(avformat_alloc_output_context2(&format_context_, nullptr, nullptr, file_name.c_str()));
		format_ = format_context_->oformat;

		auto avcid = format_->video_codec;
		video_codec_ = avcodec_find_encoder(avcid);
		LIBAV_IF_NULL_THROW(video_codec_);

		video_stream_ = avformat_new_stream(format_context_, video_codec_);
		LIBAV_IF_NULL_THROW(video_stream_);

		video_stream_->id = format_context_->nb_streams - 1;
		video_codec_context_ = video_stream_->codec;

#define AV_CODEC_FLAG_GLOBAL_HEADER (1 << 22)
#define CODEC_FLAG_GLOBAL_HEADER AV_CODEC_FLAG_GLOBAL_HEADER

		if (format_context_->oformat->flags & AVFMT_GLOBALHEADER) {
			video_codec_context_->flags |= CODEC_FLAG_GLOBAL_HEADER;
		}

        video_codec_context_->bit_rate = bit_rate;
        video_codec_context_->width = width;
        video_codec_context_->height = height;
        video_codec_context_->time_base = AVRational{1, fps};
		video_codec_context_->framerate = AVRational{fps, 1};
		video_codec_context_->gop_size = 12;
        video_codec_context_->pix_fmt = AV_PIX_FMT_YUV420P;
		video_codec_context_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

		if (video_codec_context_->codec_id == AV_CODEC_ID_H264) {
			LIBAV_IF_FAILED_THROW(av_opt_set(video_codec_context_->priv_data, "preset", presetOptions(preset), 0));
			LIBAV_IF_FAILED_THROW(av_opt_set(video_codec_context_->priv_data, "tune", "film,fastdecode,zerolatency", 0));
		}

        AVDictionary* options = nullptr;
		LIBAV_IF_FAILED_THROW(avcodec_open2(video_codec_context_, video_codec_, &options));		

		auto size = avpicture_get_size(video_codec_context_->pix_fmt,
			video_codec_context_->width,
			video_codec_context_->height);
		planes_buffer_.resize(size);

		scale_frame_ = av_frame_alloc();
		LIBAV_IF_NULL_THROW(scale_frame_);

		LIBAV_IF_FAILED_THROW(avpicture_fill(reinterpret_cast<AVPicture*>(scale_frame_),
			planes_buffer_.data(),
			video_codec_context_->pix_fmt,
			video_codec_context_->width,
			video_codec_context_->height));

		if (!(format_->flags & AVFMT_NOFILE)) {
			LIBAV_IF_FAILED_THROW(avio_open(&format_context_->pb, format_context_->filename, AVIO_FLAG_WRITE));
		}

		LIBAV_IF_FAILED_THROW(avformat_write_header(format_context_, nullptr));

        qDebug() << "Opend codec:" << avcodec_get_name(format_->video_codec);

		auto image_align_size = 32;

        scale_frame_->data[0] = nullptr;
        scale_frame_->linesize[0] = -1;
		++scale_frame_->pts;
        scale_frame_->format = video_codec_context_->pix_fmt;
		LIBAV_IF_FAILED_THROW(av_image_alloc(scale_frame_->data,
			scale_frame_->linesize,
			video_codec_context_->width,
			video_codec_context_->height,
			(AVPixelFormat)scale_frame_->format,
			image_align_size));

		src_frame_ = av_frame_alloc();
		LIBAV_IF_NULL_THROW(src_frame_);

        src_frame_->format = AV_PIX_FMT_RGB24;
		LIBAV_IF_FAILED_THROW(av_image_alloc(src_frame_->data,
			src_frame_->linesize,
			video_codec_context_->width,
			video_codec_context_->height,
			(AVPixelFormat)src_frame_->format,
			image_align_size));

		// convert RGB24 to YUV420
        sws_context_ = sws_getContext(video_codec_context_->width,
                                      video_codec_context_->height,
                                      (AVPixelFormat)src_frame_->format,
                                      video_codec_context_->width,
                                      video_codec_context_->height,
                                      (AVPixelFormat)scale_frame_->format,
                                      SWS_BICUBIC,
                                      nullptr,
                                      nullptr,
                                      nullptr);
    }

	int frame_count_;
    AVFormatContext *format_context_;
    AVOutputFormat *format_;
    AVStream *video_stream_;
	AVStream* audio_stream_;
    AVCodec *video_codec_;
	AVCodec *audio_codec_;
    AVCodecContext *video_codec_context_;
	AVCodecContext *audio_codec_context_;
    AVFrame *scale_frame_;
    AVFrame *src_frame_;
	AVFrame* audio_frame_;
    SwsContext *sws_context_;
	std::vector<uint8_t> planes_buffer_;
};

LibavVideoEncoder::LibavVideoEncoder()
    : impl_(new LibavVideoEncoderImpl()) {
}

LibavVideoEncoder::~LibavVideoEncoder() {
}

void LibavVideoEncoder::close() {
    impl_->close();
}

void LibavVideoEncoder::writeVideoFrame(const uint8_t* bits, int bytes_per_line) {
    impl_->writeVideoFrame(bits, bytes_per_line);
}

int LibavVideoEncoder::getAudioSampleSize() const  {
    return 0;
}

void LibavVideoEncoder::writeAudioFrame(const float *buffer, int size) {

}

void LibavVideoEncoder::open(const std::string &file_name, int width, int height, Preset preset, int bit_rate, int fps) {
    impl_->open(file_name, width, height, preset, bit_rate, fps);
}
