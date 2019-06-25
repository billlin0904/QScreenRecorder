#include <QDebug>
#include "screenselector_widget.h"
#include "qscreenrecorder.h"

QScreenRecorder::QScreenRecorder(QWidget *parent)
	: QMainWindow(parent)
	, is_done_(true)
	, max_elapsed_(0)
	, min_elapsed_(0) {
	ui.setupUi(this);

	QFont font("Segoe UI", 10);
	setFont(font);

	const auto fps = 60;

	selector_.reset(new ScreenSelectorWidget(this));
	selector_->setBorderColor(Qt::red);
	selector_->setInterval(1000 / fps);

	capture_timer_.setTimerType(Qt::PreciseTimer);
	capture_timer_.setInterval(1000 / fps);
	QObject::connect(&capture_timer_, &QTimer::timeout, this, &QScreenRecorder::saveScreen);

	delay_record_timer.setInterval(500);
	delay_record_timer.setSingleShot(true);

	update_timer_.setTimerType(Qt::PreciseTimer);
	update_timer_.setInterval(500);
	update_timer_.start();

	QObject::connect(&update_timer_, &QTimer::timeout, [this]() {
		auto available = frame_buffer_.availableCount();
		auto percent = available * 100 / frame_buffer_.capacity();
		ui.frameBufferBar->setValue(percent);
		});

	QObject::connect(&delay_record_timer, &QTimer::timeout, [this, fps]() {		
		future_ = std::async(std::launch::async, [this, fps]() {
			std::chrono::milliseconds timeout;

			if (fps == 30) {
				timeout = std::chrono::milliseconds(15);
			}
			else if (fps > 30) {
				timeout = std::chrono::milliseconds(0);
			}

			while (!is_done_ || !frame_buffer_.emptry()) {
				QImage sceen_image;
				if (frame_buffer_.tryPop(sceen_image)) {
					try {
						video_encoder_.writeVideoFrame(sceen_image.constBits(), sceen_image.bytesPerLine());
					}
					catch (const std::exception& e) {
						qDebug() << e.what();
						break;
					}				
				}
				else {
					//qDebug() << "Empty frame buffer.";
				}
				std::this_thread::sleep_for(timeout);
			}
			video_encoder_.close();
			});
		});

	QObject::connect(selector_.get(), &ScreenSelectorWidget::startRecord, [this, fps](int width, int height) {
		startRecord(width, height, fps);
		});

	QObject::connect(selector_.get(), &ScreenSelectorWidget::stopRecord, [this]() {
		});

	QObject::connect(ui.startRecordButton, &QPushButton::clicked, [this, fps]() {
		//selector_->start();
		startRecord(1920, 1080, fps);
		});
	QObject::connect(ui.stopRecrodButton, &QPushButton::clicked, [this]() {
		capture_timer_.stop();
		is_done_ = true;
		});
}

QScreenRecorder::~QScreenRecorder() {
	is_done_ = true;

	if (future_.valid()) {
		future_.get();
	}
}

void QScreenRecorder::startRecord(int width, int height, int fps) {
	if (future_.valid()) {
		future_.get();
	}

	max_elapsed_ = 0;
	min_elapsed_ = 0;
	
	qDebug() << "Start recording " << width << " x " << height;

	auto bit_rate = 0;
	if (fps == 30) {
		bit_rate = 2030000;
	} else if (fps > 30) {
		bit_rate = 5030000;
	}

	try {
		video_encoder_.open("test.mp4",
			width,
			height,
			Preset::PRESET_VERY_FAST,
			bit_rate,
			fps);
	}
	catch (const std::exception& e) {
		qDebug() << e.what();
		return;
	}

	is_done_ = false;
	capture_timer_.start();
	delay_record_timer.start();
}

void QScreenRecorder::saveScreen() {
	snap_sceen_timer.start();

	try {
        if (!frame_buffer_.tryEnqueue(selector_->grabImage())) {
            qDebug() << "tryEnqueue fail.";
        }
        //selector_->grabImage();
	}
	catch (const std::exception& e) {
		qDebug() << e.what();
		return;
	}

	auto elapsed = snap_sceen_timer.elapsed();

	max_elapsed_ = std::max(max_elapsed_, elapsed);
	if (min_elapsed_ == 0) {
		min_elapsed_ = elapsed;
	}
	min_elapsed_ = std::min(min_elapsed_, elapsed);

	ui.curElapsedLabel->setText(QString("Cur: %1ms").arg(elapsed));
	ui.maxElapsedLabel->setText(QString("Max: %1ms").arg(max_elapsed_));
	ui.minElapsedLabel->setText(QString("Min: %1ms").arg(min_elapsed_));
}


