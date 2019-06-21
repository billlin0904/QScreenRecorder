#pragma once

#include <QWidget>

#include "anchor.h"
#include "screenselectsize_widget.h"

class ScreenSelectorWidget : public QWidget {
	Q_OBJECT
public:
	enum SelectStatus {
		NONE_SELECT,
		START_SELECT,
		SELECTING,
        CAPTURED,
		MOVING,
		RESIZING
	};

	explicit ScreenSelectorWidget(QWidget* parent = nullptr);

	void start();

	void stop(); 

	void setMaskColor(QColor color);

	void setBorderColor(QColor color);

	QImage grabImage();

signals:
	void startRecord(int width, int height);

	void stopRecord();

private:
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void paintEvent(QPaintEvent* event) override;
	void updateSelected();
	void resizeForScreenSize();

	SelectStatus status_;
	Anchor::Pos cursor_pos_;
	int border_width_;	
	QColor mask_color_;
	QColor border_color_;
	Qt::PenStyle border_style_;
	QPoint inside_move_begin_;
	QPoint inside_move_end_;
	QPoint outside_move_begin_;
	QPoint outside_move_end_;
	QRect select_rect_;
	ScreenSelectSizeWidget select_size_;    
};

