#ifdef _WIN32
#pragma comment(lib, "Dwmapi.lib")

#include <Windows.h>
#include <dwmapi.h>

#include <QApplication>
#include <QScreen>

#include "screenhelper.h"

QRect ScreenHelper::getDisplayRect() noexcept {
    const auto screens = QGuiApplication::screens();
    auto width = 0;
    auto height = 0;
    foreach(const auto & screen, screens) {
        auto geometry = screen->geometry();
        width += geometry.width();
        if (height < geometry.height())
            height = geometry.height();
    }
    return QRect(0, 0, width, height);
}

QRect ScreenHelper::getWindowBoundRect() noexcept {
    auto result = getDisplayRect();
    auto cpos = QCursor::pos();

    auto hwnd = GetDesktopWindow();
    hwnd = GetWindow(hwnd, GW_CHILD);

    while (hwnd != nullptr) {
        RECT rect{};
        DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(RECT));
        QRect window(QPoint(rect.left, rect.top), QPoint(rect.right, rect.bottom));

        if (IsWindowVisible(hwnd) && window.contains(cpos)) {
            if (result.width() * result.height() > window.width() * window.height()) {
                result = window;
            }
        }
        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
    }
    return result;
}
#else

#include <QX11Info>

#include <QCursor>
#include <QApplication>
#include <QScreen>

#include <X11/Xlib.h>

#include "screenhelper.h"

QRect ScreenHelper::getDisplayRect() noexcept {
    const auto screens = QGuiApplication::screens();
    auto width = 0;
    auto height = 0;
    foreach(const auto & screen, screens) {
        auto geometry = screen->geometry();
        width += geometry.width();
        if (height < geometry.height())
            height = geometry.height();
    }
    return QRect(0, 0, width, height);
}

QRect ScreenHelper::getWindowBoundRect() noexcept {
    auto result = getDisplayRect();
    auto cpos = QCursor::pos();

    auto display = QX11Info::display();
    auto root_window = DefaultRootWindow(display);

    Window root_return, parent_return;
    Window *child_windows = nullptr;
    unsigned int child_num = 0;
    XQueryTree(display, root_window, &root_return, &parent_return, &child_windows, &child_num);

    for (unsigned int i = 0; i < child_num; ++i) {
        XWindowAttributes attrs;
        XGetWindowAttributes(display, child_windows[i], &attrs);
        QRect win_rect{ attrs.x, attrs.y, attrs.width, attrs.height };
        if (win_rect.contains(cpos)) {
            if(result.width() * result.height() > win_rect.width() * win_rect.height()) {
                result = win_rect;
            }
        }
    }

    XFree(child_windows);
    return result;
}
#endif
