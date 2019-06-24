#include <cstdlib>
#include <cstring>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xmd.h>
#include <X11/Xatom.h>

#include "fastscreencapture_linux.h"

namespace screenshot {

class FastScreenCapture::X11Screen {
public:
    X11Screen() {
        const auto display_name = std::getenv("DISPLAY");
        display_ = XOpenDisplay(display_name);
        default_screen_ = XDefaultScreenOfDisplay(display_);
        drawable_ = XDefaultRootWindow(display_);
    }

    ~X11Screen() {
        XCloseDisplay(display_);
    }

    void allocateBuffer() {
        auto image = XGetImage(display_,
                  drawable_,
                  0,
                  0,
                  default_screen_->width,
                  default_screen_->height,
                  AllPlanes,
                  ZPixmap);
        buffer_.resize(default_screen_->height * default_screen_->width * 4);
        XDestroyImage(image);
    }

    const std::vector<uint8_t>& getImage() {
        XFlush(display_);
        auto image = XGetImage(display_,
                  drawable_,
                  0,
                  0,
                  default_screen_->width,
                  default_screen_->height,
                  AllPlanes,
                  ZPixmap);
        if (!image) {
            return buffer_;
        }
        memcpy(buffer_.data(), image->data, buffer_.size());
        XDestroyImage(image);
        return buffer_;
    }

private:
    Display *display_;
    Screen *default_screen_;
    Drawable drawable_;
    std::vector<uint8_t> buffer_;
};

FastScreenCapture::FastScreenCapture()
    : screen_(new X11Screen()) {
    screen_->allocateBuffer();
}

FastScreenCapture::~FastScreenCapture() {
}

void FastScreenCapture::setInterval(int) {
}

const std::vector<uint8_t>& FastScreenCapture::getImage() {
    return screen_->getImage();
}

}
