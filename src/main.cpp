#include "qscreenrecorder.h"

#include <QtWidgets/QApplication>

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	QScreenRecorder recorder;
	recorder.show();
	return a.exec();
}
