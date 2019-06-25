#include <vector>

#include <bass.h>
#include <bassmix.h>

#include "module.h"
#include "libraryloader.h"
#include "soundreader.h"

class BassLib {
public:
	static BassLib& get() {
		static BassLib lib;
		return lib;
	}

private:
	ModuleHandle module;

public:
	DEFINE_EXPORT_API(BASS_Init) BASS_Init;
	DEFINE_EXPORT_API(BASS_Free) BASS_Free;
	DEFINE_EXPORT_API(BASS_StreamFree) BASS_StreamFree;
	DEFINE_EXPORT_API(BASS_SetConfig) BASS_SetConfig;
	DEFINE_EXPORT_API(BASS_RecordGetInfo) BASS_RecordGetInfo;
	DEFINE_EXPORT_API(BASS_RecordInit) BASS_RecordInit;
	DEFINE_EXPORT_API(BASS_RecordStart) BASS_RecordStart;
	DEFINE_EXPORT_API(BASS_RecordFree) BASS_RecordFree;
	DEFINE_EXPORT_API(BASS_ChannelPlay) BASS_ChannelPlay;
	DEFINE_EXPORT_API(BASS_ChannelStop) BASS_ChannelStop;

private:
	BassLib()
#ifdef Q_OS_LINUX
		: module(LoadModuleLibrary("./packages/bass/linux/libbass.so"))
#else
		: module(LoadModuleLibrary("./packages/bass/win32/bass.dll"))
#endif
		, BASS_Init(module, "BASS_Init")
		, BASS_Free(module, "BASS_Free")
		, BASS_StreamFree(module, "BASS_StreamFree")
		, BASS_SetConfig(module, "BASS_SetConfig")
		, BASS_RecordGetInfo(module, "BASS_RecordGetInfo")
		, BASS_RecordInit(module, "BASS_RecordInit")
		, BASS_RecordStart(module, "BASS_RecordStart")
		, BASS_RecordFree(module, "BASS_RecordFree")
		, BASS_ChannelPlay(module, "BASS_ChannelPlay")
		, BASS_ChannelStop(module, "BASS_ChannelStop") {
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
	DEFINE_EXPORT_API(BASS_Mixer_StreamCreate) BASS_Mixer_StreamCreate;
	DEFINE_EXPORT_API(BASS_Mixer_StreamAddChannel) BASS_Mixer_StreamAddChannel;

private:
	BassMix()
#ifdef Q_OS_LINUX
		: module(LoadModuleLibrary("./packages/bass/linux/libbassmix.so"))
#else
		: module(LoadModuleLibrary("./packages/bass/win32/bassmix.dll"))
#endif
		, BASS_Mixer_StreamCreate(module, "BASS_Mixer_StreamCreate")
		, BASS_Mixer_StreamAddChannel(module, "BASS_Mixer_StreamAddChannel") {
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
		BassLib::get().BASS_Init(-1, 44100, 0, nullptr, nullptr);
	}

	~BasInit() {
		BassLib::get().BASS_Free();
	}
};

struct BassStreamTraits {
	static HSTREAM invalid() noexcept {
		return 0;
	}
	static void close(HSTREAM value) noexcept {
		BassLib::get().BASS_StreamFree(value);
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

class SoundReader::SoundReaderImpl {
public:
	SoundReaderImpl(int sample_rate) {
		BasInit::get();
		BassLib::get().BASS_SetConfig(BASS_CONFIG_REC_LOOPBACK, 1);

		mix_streams_.emplace_back(BassMix::get().BASS_Mixer_StreamCreate(sample_rate, 2, 0));

		BassLib::get().BASS_RecordInit(-1);

		BASS_RECORDINFO info = { 0 };
		BassLib::get().BASS_RecordGetInfo(&info);

		auto record = BassLib::get().BASS_RecordStart(info.freq,
			info.formats >> 24, 
			BASS_SAMPLE_FLOAT | BASS_RECORD_PAUSE,
			recordCb,
			this);

		BassMix::get().BASS_Mixer_StreamAddChannel(mix_streams_[0].get(),
			record,
			BASS_MIXER_DOWNMIX);
		
		BassLib::get().BASS_ChannelPlay(record, false);
		BassLib::get().BASS_ChannelPlay(mix_streams_[0].get(), false);
	}

private:
	static BOOL CALLBACK recordCb(
		HRECORD handle,
		const void* buffer,
		DWORD length,
		void* user) {
		return TRUE;
	}
	std::vector<BassStreamHandle> mix_streams_;
};

SoundReader::SoundReader() 
	: impl(new SoundReaderImpl(48000)) {
}

SoundReader::~SoundReader() {
}