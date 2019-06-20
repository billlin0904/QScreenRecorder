#pragma once

#include <memory>

#include <QtWidgets/QMainWindow>
#include "ui_qscreenrecorder.h"

class ScreenSelectorWidget;

class QScreenRecorder : public QMainWindow {
	Q_OBJECT

public:
	QScreenRecorder(QWidget *parent = Q_NULLPTR);

	~QScreenRecorder() override;

private:

	Ui::QScreenRecorderClass ui;
	std::unique_ptr<ScreenSelectorWidget> selector_;
};
