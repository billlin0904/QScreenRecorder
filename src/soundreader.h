#pragma once

#include <memory>
#include <vector>
#include <string>

class SoundReader {
public:
	SoundReader();

	~SoundReader();

    void start();

    int read(float *buffer, int size) const;

    std::vector<std::string> getInputName() const;
private:
	class SoundReaderImpl;
	std::unique_ptr<SoundReaderImpl> impl;
};
