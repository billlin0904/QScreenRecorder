#include <QDebug>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>
#include <libavutil/opt.h>
}

#include "ffmpegvideowriter.h"

class FFMpegInit {
public:
    static FFMpegInit& get() {
        static FFMpegInit instance;
        return instance;
    }

private:
    FFMpegInit() {
        av_register_all();
        avcodec_register_all();
    }
};

class FFMpegVideoWriter::FFMpegVideoWriterImpl {
public:
    FFMpegVideoWriterImpl()
        : pixel_size_(0)
        , frame_count_(0)
        , format_context_(nullptr)
        , video_stream_(nullptr)
        , picture_(nullptr) {
    }

    ~FFMpegVideoWriterImpl() {
        close();
    }

    void open(const std::string &file_name, int width, int height) {
        FFMpegInit::get();

        close();

        createVideoStream(file_name);
        createStreamBuffer(width, height);

        avio_open(&format_context_->pb, file_name.c_str(), AVIO_FLAG_WRITE);
        avformat_write_header(format_context_, nullptr);
    }

    void close() {
        if (format_context_ != nullptr) {
            av_write_trailer(format_context_);
        }

        if (video_stream_ != nullptr) {
            avcodec_close(video_stream_->codec);
            video_stream_ = nullptr;
        }

        if (picture_ != nullptr) {
            av_freep(&(picture_->data[0]));
            av_free(picture_);
            picture_ = nullptr;
        }

        if (format_context_ != nullptr) {
            avio_close(format_context_->pb);
            avformat_free_context(format_context_);
            format_context_ = nullptr;
        }
    }

    void addFrame(const QImage& frame) {
		picture_rgb24_->data[0] = (uint8_t*) frame.constBits();
		picture_rgb24_->data[1] = nullptr;
		picture_rgb24_->data[2] = nullptr;

		picture_rgb24_->linesize[0] = frame.bytesPerLine();
		picture_rgb24_->linesize[1] = 0;
		picture_rgb24_->linesize[2] = 0;

        sws_scale(sws_context_,
                  picture_rgb24_->data,
                  picture_rgb24_->linesize,
                  0,
                  video_codec_context_->height,
                  picture_->data,
                  picture_->linesize);

        AVPacket pkt = { 0 };
        int got_packet = 0;
        av_init_packet(&pkt);

        auto ret = avcodec_encode_video2(video_codec_context_, &pkt, picture_, &got_packet);
        if (!ret && got_packet && pkt.size) {
            pkt.stream_index = video_stream_->index;
            av_interleaved_write_frame(format_context_, &pkt);
        } else {
            ret = 0;
        }
        picture_->pts += av_rescale_q(1, video_stream_->codec->time_base, video_stream_->time_base);
        ++frame_count_;
    }

private:
    void createVideoStream(const std::string &file_name) {
        avformat_alloc_output_context2(&format_context_, nullptr, nullptr, file_name.c_str());
        format_ = format_context_->oformat;

        dumpVideoFormat();

        AVCodecID avcid = format_->video_codec;
        video_codec_ = avcodec_find_encoder(avcid);
        video_stream_ = avformat_new_stream(format_context_, video_codec_);

        video_stream_->id = format_context_->nb_streams - 1;
        video_codec_context_ = video_stream_->codec;

#define AV_CODEC_FLAG_GLOBAL_HEADER (1 << 22)
#define CODEC_FLAG_GLOBAL_HEADER AV_CODEC_FLAG_GLOBAL_HEADER

        if (format_context_->oformat->flags & AVFMT_GLOBALHEADER) {
            video_codec_context_->flags |= CODEC_FLAG_GLOBAL_HEADER;
        }
    }

    void createStreamBuffer(int width, int height) {
		picture_ = av_frame_alloc();
        video_codec_context_->bit_rate = 400000;
        video_codec_context_->width = width;
        video_codec_context_->height = height;
        video_codec_context_->time_base= AVRational{1, 25};
        video_codec_context_->gop_size = 10;
        video_codec_context_->max_b_frames = 1;
        video_codec_context_->pix_fmt = AV_PIX_FMT_YUV420P;

        AVDictionary* options = nullptr;
        avcodec_open2(video_codec_context_, video_codec_, &options);
        qDebug() << "Opend codec:" << avcodec_get_name(format_->video_codec);
		av_opt_set(video_codec_context_->priv_data, "tune", "zerolatency", 0);

        picture_->data[0] = nullptr;
        picture_->linesize[0] = -1;
        picture_->format = video_codec_context_->pix_fmt;
        av_image_alloc(picture_->data,
                       picture_->linesize,
                       video_codec_context_->width,
                       video_codec_context_->height,
                       (AVPixelFormat)picture_->format,
                       32);

		picture_rgb24_ = av_frame_alloc();
        picture_rgb24_->format = AV_PIX_FMT_RGB24;
        pixel_size_ = av_image_alloc(picture_rgb24_->data,
                                     picture_rgb24_->linesize,
                                     video_codec_context_->width,
                                     video_codec_context_->height,
                                     (AVPixelFormat)picture_rgb24_->format,
                                     32);

		// convert RGB24 to YUV420
        sws_context_ = sws_getContext(video_codec_context_->width,
                                      video_codec_context_->height,
                                      (AVPixelFormat)picture_rgb24_->format,
                                      video_codec_context_->width,
                                      video_codec_context_->height,
                                      (AVPixelFormat)picture_->format,
                                      SWS_BICUBIC,
                                      nullptr,
                                      nullptr,
                                      nullptr);
    }

    void dumpVideoFormat() {
        if (format_->video_codec != AV_CODEC_ID_NONE) {
            AVCodecID avcid = format_->video_codec;
            auto codec = avcodec_find_encoder(avcid);
            const AVPixelFormat* p = codec->pix_fmts;
            while (*p != AV_PIX_FMT_NONE) {
                qDebug() << "Supported pix fmt:" << av_get_pix_fmt_name(*p);
                ++p;
            }
        }
    }

    int pixel_size_;
    int frame_count_;
    AVFormatContext *format_context_;
    AVOutputFormat *format_;
    AVStream *video_stream_;
    AVCodec *video_codec_;
    AVCodecContext *video_codec_context_;
    AVFrame *picture_;
    AVFrame *picture_rgb24_;
    SwsContext *sws_context_;
};

FFMpegVideoWriter::FFMpegVideoWriter()
    : impl_(new FFMpegVideoWriterImpl()) {
}

FFMpegVideoWriter::~FFMpegVideoWriter() {
}

void FFMpegVideoWriter::close() {
    impl_->close();
}

void FFMpegVideoWriter::addFrame(const QImage &frame) {
    impl_->addFrame(frame);
}

void FFMpegVideoWriter::open(const std::string &file_name, int width, int height) {
    impl_->open(file_name, width, height);
}
