#include "screenselector_widget.h"
#include "qscreenrecorder.h"

QScreenRecorder::QScreenRecorder(QWidget *parent)
	: QMainWindow(parent) {
	ui.setupUi(this);
	selector_.reset(new ScreenSelectorWidget(this));
    video_writer_.open("12345.mp4", 1920, 1080);
	QObject::connect(ui.startRecordButton, &QPushButton::clicked, [this]() {
        timer_.setInterval(1000 / 25);
        timer_.setTimerType(Qt::PreciseTimer);
        QObject::connect(&timer_, &QTimer::timeout, this, &QScreenRecorder::saveScreen);
		selector_->start();
		});
}

void QScreenRecorder::saveScreen() {
    auto frame = grab().toImage().convertToFormat(QImage::Format_RGB888);
    video_writer_.addFrame(frame);
}

QScreenRecorder::~QScreenRecorder() {
}


