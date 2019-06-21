#include <QDebug>
#include "screenselector_widget.h"
#include "qscreenrecorder.h"

QScreenRecorder::QScreenRecorder(QWidget *parent)
	: QMainWindow(parent) {	
	ui.setupUi(this);

	QFont font("Segoe UI", 10);
	setFont(font);

	selector_.reset(new ScreenSelectorWidget(this));
	selector_->setBorderColor(Qt::red);

	QObject::connect(selector_.get(), &ScreenSelectorWidget::startRecord, [this](int width, int height) {
		qDebug() << "Start recording " << width << " x " << height;
		video_writer_.open("test.mp4", width, height);
		timer_.setInterval(1000 / 25);
		timer_.setTimerType(Qt::PreciseTimer);
		QObject::connect(&timer_, &QTimer::timeout, this, &QScreenRecorder::saveScreen);
		timer_.start();
		});

	QObject::connect(selector_.get(), &ScreenSelectorWidget::stopRecord, [this]() {
		video_writer_.close();
		timer_.stop();
		});

	QObject::connect(ui.selectRectButton, &QPushButton::clicked, [this]() {        
		selector_->start();
		});
}

void QScreenRecorder::saveScreen() {
    auto frame = selector_->grabImage();
	auto size = frame.size();
	//qDebug() << "Frame " << size.width() << " x " << size.height() << " size: " << frame.sizeInBytes();
    video_writer_.addFrame(frame);
}

QScreenRecorder::~QScreenRecorder() {
}


