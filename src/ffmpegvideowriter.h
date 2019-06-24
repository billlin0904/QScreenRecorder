#pragma once

#include <memory>
#include <string>
#include <stdexcept>

#include <QImage>

class LibavException : public std::exception {
public:
	explicit LibavException(int error);

	int error() const noexcept;

	const char* what() const noexcept override;

private:
	int error_;
	std::string message_;
};

class LibavVideEncoder {
public:
    LibavVideEncoder();

    ~LibavVideEncoder();

    void open(const std::string &file_name, int width, int height);

    void close();

    void addFrame(const QImage &frame);

private:
    class LibavVideEncoderImpl;
    std::unique_ptr<LibavVideEncoderImpl> impl_;
};

