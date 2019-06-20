#pragma once

#include <memory>
#include <string>

#include <QImage>

class FFMpegVideoWriter {
public:
    FFMpegVideoWriter();

    ~FFMpegVideoWriter();

    void open(const std::string &file_name, int width, int height);

    void close();

    void addFrame(const QImage &frame);

private:
    class FFMpegVideoWriterImpl;
    std::unique_ptr<FFMpegVideoWriterImpl> impl_;
};

