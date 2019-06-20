#pragma once

#include <memory>

#include <QTimer>
#include <QtWidgets/QMainWindow>

#include "ffmpegvideowriter.h"
#include "ui_qscreenrecorder.h"

class ScreenSelectorWidget;

class QScreenRecorder : public QMainWindow {
	Q_OBJECT

public:
	QScreenRecorder(QWidget *parent = Q_NULLPTR);

	~QScreenRecorder() override;

private:
    void saveScreen();

	Ui::QScreenRecorderClass ui;
	std::unique_ptr<ScreenSelectorWidget> selector_;
    FFMpegVideoWriter video_writer_;
    QTimer timer_;
};
