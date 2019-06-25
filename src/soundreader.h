#pragma once

#include <memory>

class SoundReader {
public:
	SoundReader();
	~SoundReader();
private:
	class SoundReaderImpl;
	std::unique_ptr<SoundReaderImpl> impl;
};
