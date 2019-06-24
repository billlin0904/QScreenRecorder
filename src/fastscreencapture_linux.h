#pragma once

#include <cstdint>
#include <vector>
#include <memory>

namespace screenshot {

class FastScreenCapture {
public:
    FastScreenCapture();

    ~FastScreenCapture();

    FastScreenCapture(const FastScreenCapture&) = delete;
    FastScreenCapture& operator=(const FastScreenCapture&) = delete;

    void setInterval(int mills);

    const std::vector<uint8_t>& getImage();

private:
    class X11Screen;
    std::unique_ptr<X11Screen> screen_;
};

}
