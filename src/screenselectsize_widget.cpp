#include <QPainter>
#include "screenselectsize_widget.h"

ScreenSelectSizeWidget::ScreenSelectSizeWidget(QWidget* parent)
	: QWidget(parent)
	, label_(this) {
	label_.setFixedSize(100, 24);
	label_.setAlignment(Qt::AlignCenter);
	QFont font("Consolas", 10);
	label_.setFont(font);
	QPalette p;
	p.setColor(QPalette::WindowText, Qt::white);
	label_.setPalette(p);
}

void ScreenSelectSizeWidget::setSize(const QSize& size) {
	label_.setText(QString::number(size.width()) + " x " + QString::number(size.height()));
}

void ScreenSelectSizeWidget::paintEvent(QPaintEvent* event) {
	QPainter painter(this);
	painter.fillRect(rect(), QColor(0, 0, 0, 200));
}