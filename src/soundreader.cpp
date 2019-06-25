#include <vector>

#include <bass.h>
#include <bassmix.h>

#include "module.h"
#include "libraryloader.h"
#include "soundreader.h"

#ifdef Q_OS_LINUX
#define BASS_DLL_PATH    "./packages/bass/linux/libbass.so"
#define BASSMIX_DLL_PATH "./packages/bass/linux/libbassmix.so"
#else
#define BASS_DLL_PATH    "./packages/bass/win32/bass.dll"
#define BASSMIX_DLL_PATH "./packages/bass/win32/bassmix.dll"
#endif

class BassLib {
public:
    static BassLib& get() {
        static BassLib lib;
        return lib;
    }

    ModuleHandle module;

public:;
    DEFINE_EXPORT_API(BASS_Init) BASS_Init_;
    DEFINE_EXPORT_API(BASS_Free) BASS_Free_;
    DEFINE_EXPORT_API(BASS_PluginLoad) BASS_PluginLoad_;
    DEFINE_EXPORT_API(BASS_GetVersion) BASS_GetVersion_;
    DEFINE_EXPORT_API(BASS_SetConfig) BASS_SetConfig_;
    DEFINE_EXPORT_API(BASS_ErrorGetCode) BASS_ErrorGetCode_;
    // Record
    DEFINE_EXPORT_API(BASS_RecordGetInfo) BASS_RecordGetInfo_;
    DEFINE_EXPORT_API(BASS_RecordGetDeviceInfo) BASS_RecordGetDeviceInfo_;
    DEFINE_EXPORT_API(BASS_RecordInit) BASS_RecordInit_;
    DEFINE_EXPORT_API(BASS_RecordStart) BASS_RecordStart_;
    DEFINE_EXPORT_API(BASS_RecordSetDevice) BASS_RecordSetDevice_;
    DEFINE_EXPORT_API(BASS_RecordGetInputName) BASS_RecordGetInputName_;
    DEFINE_EXPORT_API(BASS_RecordFree) BASS_RecordFree_;
    // Channel
    DEFINE_EXPORT_API(BASS_ChannelGetData) BASS_ChannelGetData_;
    DEFINE_EXPORT_API(BASS_ChannelPlay) BASS_ChannelPlay_;
    DEFINE_EXPORT_API(BASS_ChannelStop) BASS_ChannelStop_;
    // Stream
    DEFINE_EXPORT_API(BASS_StreamCreate) BASS_StreamCreate_;
    DEFINE_EXPORT_API(BASS_StreamFree) BASS_StreamFree_;

private:
    BassLib()
        : module(LoadModuleLibrary(BASS_DLL_PATH))
        , BASS_Init_(module, "BASS_Init")
        , BASS_Free_(module, "BASS_Free")
        , BASS_PluginLoad_(module, "BASS_PluginLoad")
        , BASS_GetVersion_(module, "BASS_GetVersion")
        , BASS_SetConfig_(module, "BASS_SetConfig")
        , BASS_ErrorGetCode_(module, "BASS_ErrorGetCode")
        , BASS_RecordGetInfo_(module, "BASS_RecordGetInfo")
        , BASS_RecordGetDeviceInfo_(module, "BASS_RecordGetDeviceInfo")
        , BASS_RecordInit_(module, "BASS_RecordInit")
        , BASS_RecordStart_(module, "BASS_RecordStart")
        , BASS_RecordSetDevice_(module, "BASS_RecordSetDevice")
        , BASS_RecordGetInputName_(module, "BASS_RecordGetInputName")
        , BASS_RecordFree_(module, "BASS_RecordFree")
        , BASS_ChannelGetData_(module, "BASS_ChannelGetData")
        , BASS_ChannelPlay_(module, "BASS_ChannelPlay")
        , BASS_ChannelStop_(module, "BASS_ChannelStop")
        , BASS_StreamCreate_(module, "BASS_StreamCreate")
        , BASS_StreamFree_(module, "BASS_StreamFree") {
    }
};

class BasInit {
public:
    static BasInit& get() {
        static BasInit instance;
        return instance;
    }

private:
    BasInit() {
        BassLib::get().BASS_Init_(-1, 44100, 0, nullptr, nullptr);
    }

    ~BasInit() {
        BassLib::get().BASS_Free_();
    }
};

class BassMix {
public:
    static BassMix& get() {
        static BassMix mix;
        return mix;
    }

private:
    ModuleHandle module;

public:
    DEFINE_EXPORT_API(BASS_Mixer_GetVersion) BASS_Mixer_GetVersion_;
    DEFINE_EXPORT_API(BASS_Mixer_StreamCreate) BASS_Mixer_StreamCreate_;
    DEFINE_EXPORT_API(BASS_Mixer_StreamAddChannel) BASS_Mixer_StreamAddChannel_;

private:
    BassMix()
        : module(LoadModuleLibrary(BASSMIX_DLL_PATH))
        , BASS_Mixer_GetVersion_(module, "BASS_Mixer_GetVersion")
        , BASS_Mixer_StreamCreate_(module, "BASS_Mixer_StreamCreate")
        , BASS_Mixer_StreamAddChannel_(module, "BASS_Mixer_StreamAddChannel") {
    }
};

struct BassStreamTraits {
    static HSTREAM invalid() noexcept {
        return 0;
    }
    static void close(HSTREAM value) noexcept {
        BassLib::get().BASS_StreamFree_(value);
    }
};

struct BassRecordTraits {
    static HRECORD invalid() noexcept {
        return 0;
    }
    static void close(HRECORD value) noexcept {
    }
};

using BassStreamHandle = UniqueHandle<HSTREAM, BassStreamTraits>;

class BassException : public std::exception {
public:
    BassException() {
        error_ = BassLib::get().BASS_ErrorGetCode_();
    }

    const char * what() const noexcept override {
        switch (error_) {
        case BASS_OK:
            return "all is OK";
        case BASS_ERROR_MEM:
            return "memory error";
        case BASS_ERROR_FILEOPEN:
            return "can't open the file";
        case BASS_ERROR_DRIVER:
            return "can't find a free/valid driver";
        case BASS_ERROR_BUFLOST:
            return "the sample buffer was lost";
        case BASS_ERROR_HANDLE:
            return "invalid handle";
        case BASS_ERROR_FORMAT:
            return "unsupported sample format";
        case BASS_ERROR_POSITION:
            return "invalid position";
        case BASS_ERROR_INIT:
            return "BASS_Init has not been successfully called";
        case BASS_ERROR_START:
            return "BASS_Start has not been successfully called";
        case BASS_ERROR_ALREADY:
            return "already initialized/paused/whatever";
        case BASS_ERROR_NOCHAN:
            return "can't get a free channel";
        case BASS_ERROR_ILLTYPE:
            return "an illegal type was specified";
        case BASS_ERROR_ILLPARAM:
            return "an illegal parameter was specified";
        case BASS_ERROR_NO3D:
            return "no 3D support";
        case BASS_ERROR_NOEAX:
            return "no EAX support";
        case BASS_ERROR_DEVICE:
            return "illegal device number";
        case BASS_ERROR_NOPLAY:
            return "not playing";
        case BASS_ERROR_FREQ:
            return "illegal sample rate";
        case BASS_ERROR_NOTFILE:
            return "the stream is not a file stream";
        case BASS_ERROR_NOHW:
            return "no hardware voices available";
        case BASS_ERROR_EMPTY:
            return "the MOD music has no sequence data";
        case BASS_ERROR_NONET:
            return "no internet connection could be opened";
        case BASS_ERROR_CREATE:
            return "couldn't create the file";
        case BASS_ERROR_NOFX:
            return "effects are not available";
        case BASS_ERROR_NOTAVAIL:
            return "requested data is not available";
        case BASS_ERROR_DECODE:
            return "the channel is/isn't a 'decoding channel'";
        case BASS_ERROR_DX:
            return "a sufficient DirectX version is not installed";
        case BASS_ERROR_TIMEOUT:
            return "connection timedout";
        case BASS_ERROR_FILEFORM:
            return "unsupported file format";
        case BASS_ERROR_SPEAKER:
            return "unavailable speaker";
        case BASS_ERROR_VERSION:
            return "invalid BASS version (used by add-ons)";
        case BASS_ERROR_CODEC:
            return "codec is not available/supported";
        case BASS_ERROR_ENDED:
            return "the channel/file has ended";
        case BASS_ERROR_BUSY:
            return "the device is busy";
        case BASS_ERROR_UNKNOWN:
        default:
            return "some other mystery problem";
        }
    }
private:
    int error_;
};

#define BASS_IF_FAIL_THROW(expr) \
    if ((expr) == 0) \
    throw BassException();


#define BASS_IF_FALSE_THROW(expr) \
    if ((expr) != TRUE) \
    throw BassException();


class SoundReader::SoundReaderImpl {
public:
    explicit SoundReaderImpl(int sample_rate)
        : sample_rate_(sample_rate) {
        BasInit::get();
        BassMix::get();

        auto bsss_version = BassLib::get().BASS_GetVersion_();
        auto bsss_mix_version = BassMix::get().BASS_Mixer_GetVersion_();

        BassLib::get().BASS_SetConfig_(BASS_CONFIG_REC_LOOPBACK, 1);

        auto stream = BassMix::get().BASS_Mixer_StreamCreate_(sample_rate, 2, BASS_SAMPLE_FLOAT);
        BASS_IF_FAIL_THROW(stream);
        streams_.emplace_back(stream);

        BASS_DEVICEINFO deviceinfo{0};
        BASS_IF_FALSE_THROW(BassLib::get().BASS_RecordGetDeviceInfo_(0, &deviceinfo));

        if(!(deviceinfo.flags & BASS_DEVICE_INIT)) {
            BASS_IF_FALSE_THROW(BassLib::get().BASS_RecordInit_(0));
        }
        BASS_IF_FALSE_THROW(BassLib::get().BASS_RecordSetDevice_(0));
    }

    ~SoundReaderImpl() {
        BassLib::get().BASS_RecordFree_();
    }

    void start() {
        // For a recording channel, that means it must not use a RECORDPROC.
        auto record = BassLib::get().BASS_RecordStart_(sample_rate_,
                                                       2,
                                                       BASS_RECORD_PAUSE,
                                                       nullptr,
                                                       nullptr);
        BASS_IF_FAIL_THROW(record);
        BASS_IF_FAIL_THROW(BassMix::get().BASS_Mixer_StreamAddChannel_(streams_[0].get(),
                           record,
                           BASS_MIXER_DOWNMIX));
        BASS_IF_FAIL_THROW(BassLib::get().BASS_ChannelPlay_(record, false));
        BASS_IF_FAIL_THROW(BassLib::get().BASS_ChannelPlay_(streams_[0].get(), false));
    }

    std::vector<std::string> getInputName() const {
        std::vector<std::string> input_name;
        const char * name = nullptr;
        for (auto i = 0; name = BassLib::get().BASS_RecordGetInputName_(i); i++) {
            input_name.push_back(name);
        }
        return input_name;
    }

    int read(float *buffer, int size) const {
        return BassLib::get().BASS_ChannelGetData_(streams_[0].get(), buffer, size);
    }
private:
    int sample_rate_;
    std::vector<BassStreamHandle> streams_;
};

SoundReader::SoundReader() 
    : impl(new SoundReaderImpl(44100)) {
}

SoundReader::~SoundReader() {
}

int SoundReader::read(float *buffer, int size) const {
    return impl->read(buffer, size);
}

void SoundReader::start() {
    impl->start();
}

std::vector<std::string> SoundReader::getInputName() const {
    return impl->getInputName();
}
