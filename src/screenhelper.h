#pragma once

#include <QRect>

class ScreenHelper {
public:
    static QRect getDisplayRect() noexcept;
	static QRect getWindowBoundRect() noexcept;
};
