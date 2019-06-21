#include <QPainter>
#include <QMouseEvent>
#include <QShortcut>

#include "screenhelper.h"
#include "screenselector_widget.h"

ScreenSelectorWidget::ScreenSelectorWidget(QWidget* parent)
	: QWidget(parent)
	, status_(NONE_SELECT)
	, cursor_pos_(Anchor::NONE)
	, border_width_(1)
	, mask_color_(0, 0, 0, 100)
	, border_color_(Qt::cyan)
	, border_style_(Qt::DashLine)
	, select_size_(this) {
	setAttribute(Qt::WA_TranslucentBackground);
	setMouseTracking(true);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint /*| Qt::WindowStaysOnTopHint | Qt::BypassWindowManagerHint*/);
	resizeForScreenSize();
	auto start_record = new QShortcut(Qt::CTRL + Qt::Key_R, this);
	QObject::connect(start_record, &QShortcut::activated, [this]() {
		emit startRecord(select_rect_.width(), select_rect_.height());
		});
	auto stop_record = new QShortcut(Qt::CTRL + Qt::Key_S, this);
	QObject::connect(stop_record, &QShortcut::activated, [this]() {
		emit stopRecord();
		});
}

void ScreenSelectorWidget::resizeForScreenSize() {
    const auto rect = ScreenHelper::getDisplayRect();
	setFixedSize(rect.width(), rect.height());
}

void ScreenSelectorWidget::setMaskColor(QColor color) {
	mask_color_ = color;
}

void ScreenSelectorWidget::setBorderColor(QColor color) {
	border_color_ = color;
}

QImage ScreenSelectorWidget::grabImage() {
	return grab(select_rect_).toImage().convertToFormat(QImage::Format_RGB888);
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
	select_size_.show();

    activateWindow();
    select_rect_ = ScreenHelper::getWindowBoundRect();
}

void ScreenSelectorWidget::mousePressEvent(QMouseEvent* event) {
    const auto pos = event->pos();

	if (event->button() == Qt::LeftButton) {
		cursor_pos_ = Anchor(select_rect_).anchorPos(pos);

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
		case CAPTURED:
			if (cursor_pos_ == Anchor::INSIDE) {
				inside_move_begin_ = pos;
				inside_move_end_ = pos;
				status_ = MOVING;
			}
			else {
				outside_move_begin_ = pos;
				outside_move_end_ = pos;
				status_ = RESIZING;
			}
			break;
		}
	}
	else {
		stop();
	}

	update();
}

void ScreenSelectorWidget::updateSelected() {
	if (status_ == MOVING) {
		QPoint diff(inside_move_end_.x() - inside_move_begin_.x(), inside_move_end_.y() - inside_move_begin_.y());

		Anchor anchor(select_rect_);
		auto diff_min_x = std::max(anchor.left(), 0);
		auto diff_max_x = std::max(width() - anchor.right(), 0);
		auto diff_min_y = std::max(anchor.top(), 0);
		auto diff_max_y = std::max(height() - anchor.bottom(), 0);

		diff.rx() = (diff.x() < 0) ? std::max(diff.x(), -diff_min_x) : std::min(diff.x(), diff_max_x);
		diff.ry() = (diff.y() < 0) ? std::max(diff.y(), -diff_min_y) : std::min(diff.y(), diff_max_y);

		select_rect_.translate(diff);
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
	{
		cursor_pos_ = Anchor(select_rect_).anchorPos(pos);
		switch (cursor_pos_) {
		case Anchor::INSIDE:
			setCursor(Qt::SizeAllCursor);
			break;
		case Anchor::OUTSIDE:
			setCursor(Qt::ForbiddenCursor);
			break;

		case Anchor::TOP:
		case Anchor::BOTTOM:
		case Anchor::TOP_BORDER:
		case Anchor::BOTTOM_BORDER:
			setCursor(Qt::SizeVerCursor);
			break;

		case Anchor::LEFT:
		case Anchor::RIGHT:
		case Anchor::LEFT_BORDER:
		case Anchor::RIGHT_BORDER:
			setCursor(Qt::SizeHorCursor);
			break;

		case Anchor::NONE:
		default:
			break;
		}
	}
	break;
	case MOVING:
		inside_move_end_ = pos;
		updateSelected();
		inside_move_begin_ = pos;
		break;
	case RESIZING:
		break;
    }

	update();
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
		status_ = CAPTURED;
        break;
    }

	update();
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
	
	Anchor anchor(select_rect_);

    // Show size info
    select_size_.setSize(select_rect_.size());
    auto screen_size_y = anchor.topLeft().y() - select_size_.geometry().height();
    select_size_.move(anchor.topLeft().x() + 1, (screen_size_y < 0 ? anchor.topLeft().y() + 1 : screen_size_y));
    // Show border
    painter.setPen(QPen(border_color_, border_width_, border_style_));
    painter.drawRect(select_rect_);
    // Show anchor    
    painter.fillRect(anchor.anchorRect(Anchor::TOP_LEFT), border_color_);
    painter.fillRect(anchor.anchorRect(Anchor::BOTTOM_RIGHT), border_color_);
    painter.fillRect(anchor.anchorRect(Anchor::TOP_RIGHT), border_color_);
    painter.fillRect(anchor.anchorRect(Anchor::BOTTOM_LEFT), border_color_);
}
