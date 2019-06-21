#pragma once

#include <QRect>

class Anchor {
public:
	const int WIDTH = 7;

	enum Pos {
		NONE = 0,
		LEFT_BORDER,
		RIGHT_BORDER,
		TOP_BORDER,
		BOTTOM_BORDER,
		LEFT,
		RIGHT,
		TOP,
		BOTTOM,
		TOP_LEFT,
		BOTTOM_RIGHT,
		TOP_RIGHT,
		BOTTOM_LEFT,
		INSIDE,
		OUTSIDE,
	};

	explicit Anchor(const QRect& rect) noexcept
		: rect_(rect) {
	}

	bool isContains(const QPoint& point) const noexcept {
		return QRect(left(), top(), width(), height()).contains(point);
	}

	QPoint topLeft() const noexcept {
		return QPoint(rect_.left(), rect_.top());
	}

	Pos anchorPos(const QPoint& point) const noexcept {
		if (anchorRect(LEFT).contains(point))
			return LEFT;
		if (anchorRect(LEFT_BORDER).contains(point))
			return LEFT_BORDER;
		if (anchorRect(RIGHT).contains(point))
			return RIGHT;
		if (anchorRect(RIGHT_BORDER).contains(point))
			return RIGHT_BORDER;
		if (anchorRect(TOP).contains(point))
			return TOP;
		if (anchorRect(TOP_BORDER).contains(point))
			return TOP_BORDER;
		if (anchorRect(BOTTOM).contains(point))
			return BOTTOM;
		if (anchorRect(BOTTOM_BORDER).contains(point))
			return BOTTOM_BORDER;
		if (anchorRect(TOP_LEFT).contains(point))
			return TOP_LEFT;
		if (anchorRect(TOP_RIGHT).contains(point))
			return TOP_RIGHT;
		if (anchorRect(BOTTOM_RIGHT).contains(point))
			return BOTTOM_RIGHT;
		if (anchorRect(BOTTOM_LEFT).contains(point))
			return BOTTOM_LEFT;
		return isContains(point) ? INSIDE : OUTSIDE;
	}

	int width() const noexcept {
		auto w = std::abs(rect_.left() - rect_.right());
		return w > 1 ? w : 1;
	}

	int height() const noexcept {
		auto h = std::abs(rect_.top() - rect_.bottom());
		return h > 1 ? h : 1;
	}

	int left() const noexcept {
		return rect_.left() < rect_.right() ? rect_.left() : rect_.right();
	}

	int right() const noexcept {
		return rect_.left() > rect_.right() ? rect_.left() : rect_.right();
	}

	int top() const noexcept {
		return rect_.top() < rect_.bottom() ? rect_.top() : rect_.bottom();
	}

	int bottom() const noexcept {
		return rect_.top() > rect_.bottom() ? rect_.top() : rect_.bottom();
	}

	QRect anchorRect(Pos pos) const noexcept {
		switch (pos) {
		case LEFT:
			return { rect_.left() - WIDTH / 2, (rect_.top() + rect_.bottom()) / 2 - WIDTH / 2, WIDTH, WIDTH };
		case RIGHT:
			return { (rect_.left() + rect_.right()) / 2 - WIDTH / 2, rect_.bottom() - WIDTH / 2, WIDTH, WIDTH };
		case TOP:
			return { (rect_.left() + rect_.right()) / 2 - WIDTH / 2, rect_.top() - WIDTH / 2, WIDTH, WIDTH };
		case BOTTOM:
			return  { (rect_.left() + rect_.right()) / 2 - WIDTH / 2, rect_.bottom() - WIDTH / 2, WIDTH, WIDTH };
		case TOP_LEFT:
			return { rect_.left() - WIDTH / 2, rect_.top() - WIDTH / 2, WIDTH, WIDTH };
		case TOP_RIGHT:
			return { rect_.right() - WIDTH / 2, rect_.top() - WIDTH / 2, WIDTH, WIDTH };
		case BOTTOM_RIGHT:
			return { rect_.left() - WIDTH / 2, rect_.bottom() - WIDTH / 2, WIDTH, WIDTH };
		case BOTTOM_LEFT:
			return { rect_.right() - WIDTH / 2, rect_.bottom() - WIDTH / 2, WIDTH, WIDTH };
		case LEFT_BORDER:
			return { rect_.left() - 1 / 2, top(), 1, height() };
		case RIGHT_BORDER:
			return { rect_.right() - 1 / 2, top(), 1, height() };
		case TOP_BORDER:
			return { left(), rect_.top() - 1 / 2, width(), 1 };
		case BOTTOM_BORDER:
			return { left(), rect_.bottom() - 1 / 2, width(), 1 };
		}
		return QRect();
	}
private:
	const QRect& rect_;
};