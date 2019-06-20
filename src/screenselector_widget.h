#pragma once

#include <QWidget>

#include "screenselectsize_widget.h"

class ScreenSelectorWidget : public QWidget {
public:
	enum SelectStatus {
		NONE_SELECT,
		START_SELECT,
		SELECTING,
        CAPTURED,
	};

	explicit ScreenSelectorWidget(QWidget* parent = nullptr);

	void start();

	void stop();    

private:
	QPoint topLeft() const noexcept {
		return QPoint(select_rect_.left(), select_rect_.top());
	}

	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void paintEvent(QPaintEvent* event) override;

	void resizeForScreenSize();

	SelectStatus status_;
	int border_width_;
	QRect select_rect_;
	QColor mask_color_;
	QColor border_color_;
	Qt::PenStyle border_style_;    
	ScreenSelectSizeWidget select_size_;    
};

