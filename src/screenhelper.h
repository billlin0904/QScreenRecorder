#pragma once

#include <QRect>

class ScreenHelper {
public:
	static QRect getAllScreenRect() noexcept;
	static QRect getWindowBoundRect() noexcept;
};
