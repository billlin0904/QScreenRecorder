#pragma once

#include <QWidget>
#include <QLabel>

class ScreenSelectSizeWidget : public QWidget {
public:
	explicit ScreenSelectSizeWidget(QWidget* parent = nullptr);

	void setSize(const QSize &size);

private:
	void paintEvent(QPaintEvent* event) override;

	QLabel label_;
};

