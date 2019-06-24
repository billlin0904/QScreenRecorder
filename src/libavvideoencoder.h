#pragma once

#include <memory>
#include <string>
#include <stdexcept>

enum class Preset {
	PRESET_ULTRAFAST,
	PRESET_SUPER_FAST,
	PRESET_VERY_FAST,
	PRESET_FASTER,
	PRESET_FAST,
	PRESET_MEDIUM,
	PRESET_SLOW,
	PRESET_SLOWER,
	PRESET_VERY_SLOW,
	PRESET_PLACE_BO
};

class LibavException : public std::exception {
public:
	explicit LibavException(const char * message);

	explicit LibavException(int error);

	int error() const noexcept;

	const char* what() const noexcept override;

private:
	int error_;
	std::string message_;
};

class LibavVideoEncoder {
public:
	LibavVideoEncoder();

    ~LibavVideoEncoder();

    void open(const std::string &file_name,
		int width,
		int height,
		Preset preset,
		int bit_rate,
		int fps);

    void close();

    void addFrame(const uint8_t* bits, int bytes_per_line);

private:
    class LibavVideoEncoderImpl;
    std::unique_ptr<LibavVideoEncoderImpl> impl_;
};

