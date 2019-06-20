#include <QPainter>
#include <QMouseEvent>

#include "screenhelper.h"
#include "screenselector_widget.h"

class Anchor {
public:
    enum Pos {
        LEFT_BORDER,
        RIGHT_BORDER,
        TOP_BORDER,
        BOTTOM_BORDER,
        LEFT,
        RIGHT,
        TOP,
        BOTTOM,
        LEFT_TOP,
        BOTTOM_RIGHT,
        TOP_RIGHT,
        BOTTOM_LEFT,
        IN,
        OUT,
    };

	explicit Anchor(int width = 7) noexcept
		: width_(width) {
	}

	explicit Anchor(QRect rect, int width = 7) noexcept
		: rect_(rect)
		, width_(width) {
	}

    QRect anchorRect(int index) const noexcept {
		switch (index) {
		case 0:
			return { rect_.left() - width_ / 2, rect_.top() - width_ / 2, width_, width_ };
		case 1:
			return { rect_.left() - width_ / 2, rect_.bottom() - width_ / 2, width_, width_ };
		case 2:
			return { rect_.right() - width_ / 2, rect_.top() - width_ / 2, width_, width_ };
		case 3:
			return { rect_.right() - width_ / 2, rect_.bottom() - width_ / 2, width_, width_ };
		}
		return QRect();
	}

    QPoint topLeft() const noexcept {
        return QPoint(rect_.left(), rect_.top());
    }
private:
	QRect rect_;
	int width_;
};

ScreenSelectorWidget::ScreenSelectorWidget(QWidget* parent)
	: QWidget(parent)
	, status_(NONE_SELECT)
	, border_width_(1)
	, mask_color_(0, 0, 0, 100)
	, border_color_(Qt::cyan)
	, border_style_(Qt::DashDotLine)
	, select_size_(this) {
	setAttribute(Qt::WA_TranslucentBackground);
	setMouseTracking(true);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint/* | Qt::BypassWindowManagerHint*/);
	resizeForScreenSize();    
}

void ScreenSelectorWidget::resizeForScreenSize() {
    const auto rect = ScreenHelper::getDisplayRect();
	setFixedSize(rect.width(), rect.height());
}

void ScreenSelectorWidget::stop() {
	status_ = NONE_SELECT;
	select_size_.hide();
	repaint();
	hide();
}

void ScreenSelectorWidget::start() {
	status_ = START_SELECT;
	show();
    activateWindow();
    select_rect_ = ScreenHelper::getWindowBoundRect();
}

void ScreenSelectorWidget::mousePressEvent(QMouseEvent* event) {
    auto pos = event->pos();

	if (event->button() == Qt::LeftButton) {
		switch (status_) {
		case START_SELECT:
		{			
			select_rect_.setLeft(pos.x());
			select_rect_.setRight(pos.x());
			select_rect_.setTop(pos.y());
			select_rect_.setBottom(pos.y());
			status_ = SELECTING;
		}
			break;
		case SELECTING:
			break;
		}
	}
}

void ScreenSelectorWidget::mouseMoveEvent(QMouseEvent* event) {
    auto pos = event->pos();

    switch (status_) {
    case START_SELECT:
        setCursor(Qt::CrossCursor);
        break;
    case SELECTING:
        select_rect_.setRight(pos.x());
        select_rect_.setBottom(pos.y());
        break;
    case CAPTURED:
        break;
    }
}

void ScreenSelectorWidget::mouseReleaseEvent(QMouseEvent* event) {
    auto pos = event->pos();

    switch (status_) {
    case SELECTING:
        select_rect_.setRight(pos.x());
        select_rect_.setBottom(pos.y());
        if (select_rect_.left() == select_rect_.right() && select_rect_.top() == select_rect_.bottom()) {
            select_rect_ = ScreenHelper::getWindowBoundRect();
        }

        break;
    }
}

void ScreenSelectorWidget::paintEvent(QPaintEvent* event) {
	QPainter painter(this);

	painter.fillRect(rect(), QColor(0, 0, 0, 1));
	painter.fillRect(QRect{ 0, 0, width(), select_rect_.y() }, mask_color_);
	painter.fillRect(QRect{ 0, select_rect_.y(), select_rect_.x(), select_rect_.height() }, mask_color_);
	painter.fillRect(QRect{ select_rect_.x() + select_rect_.width(), select_rect_.y(), width() - select_rect_.x() - select_rect_.width(), select_rect_.height() }, mask_color_);
	painter.fillRect(QRect{ 0, select_rect_.y() + select_rect_.height(), width(), height() - select_rect_.y() - select_rect_.height() }, mask_color_);

    if (status_ == START_SELECT) {
		select_rect_ = ScreenHelper::getWindowBoundRect();
	} 
	
    // Show size info
    select_size_.setSize(select_rect_.size());
    auto screen_size_y = topLeft().y() - select_size_.geometry().height();
    select_size_.move(topLeft().x() + 1, (screen_size_y < 0 ? topLeft().y() + 1 : screen_size_y));
    // Show border
    painter.setPen(QPen(border_color_, border_width_, border_style_));
    painter.drawRect(select_rect_);
    // Show anchor
    Anchor anchor(select_rect_);
    painter.fillRect(anchor.anchorRect(0), border_color_);
    painter.fillRect(anchor.anchorRect(1), border_color_);
    painter.fillRect(anchor.anchorRect(2), border_color_);
    painter.fillRect(anchor.anchorRect(3), border_color_);
}
