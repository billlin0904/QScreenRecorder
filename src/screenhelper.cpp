#pragma comment(lib, "Dwmapi.lib")

#include <Windows.h>
#include <dwmapi.h>

#include <QApplication>
#include <QScreen>

#include "screenhelper.h"

QRect ScreenHelper::getAllScreenRect() noexcept {
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
	auto result = getAllScreenRect();
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
