#pragma once

#include <memory>
#include <atomic>
#include <future>

#include <QTimer>
#include <QElapsedTimer>

#include <QtWidgets/QMainWindow>

#include "spscqueue.h"
#include "waitqueue.h"

#include "libavvideoencoder.h"
#include "ui_qscreenrecorder.h"

class ScreenSelectorWidget;

class QScreenRecorder : public QMainWindow {
	Q_OBJECT

public:
	QScreenRecorder(QWidget *parent = Q_NULLPTR);

	~QScreenRecorder() override;

private:
	void startRecord(int width, int height, int fps);

    void saveScreen();

	std::atomic<bool> is_done_;
	qint64 max_elapsed_;
	qint64 min_elapsed_;	
	std::unique_ptr<ScreenSelectorWidget> selector_;
    LibavVideoEncoder video_encoder_;
	SPSCQueue<QImage, 64> frame_buffer_;
	//WaitQueue<QImage> frame_buffer_;
    QTimer capture_timer_;
	QTimer delay_record_timer;
	QTimer update_timer_;
	QElapsedTimer snap_sceen_timer;
	std::future<void> future_;
	Ui::QScreenRecorderClass ui;
};
