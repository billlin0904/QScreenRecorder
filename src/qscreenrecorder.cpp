#include "screenselector_widget.h"
#include "qscreenrecorder.h"

QScreenRecorder::QScreenRecorder(QWidget *parent)
	: QMainWindow(parent) {
	ui.setupUi(this);
	selector_.reset(new ScreenSelectorWidget(this));
	QObject::connect(ui.startRecordButton, &QPushButton::clicked, [this]() {
		selector_->start();
		});
}

QScreenRecorder::~QScreenRecorder() {
}


