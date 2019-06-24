/********************************************************************************
** Form generated from reading UI file 'qscreenrecorder.ui'
**
** Created by: Qt User Interface Compiler version 5.12.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QSCREENRECORDER_H
#define UI_QSCREENRECORDER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QScreenRecorderClass
{
public:
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *startRecordButton;
    QPushButton *stopRecrodButton;
    QProgressBar *frameBufferBar;
    QLabel *maxElapsedLabel;
    QLabel *minElapsedLabel;
    QLabel *curElapsedLabel;
    QSpacerItem *horizontalSpacer;

    void setupUi(QMainWindow *QScreenRecorderClass)
    {
        if (QScreenRecorderClass->objectName().isEmpty())
            QScreenRecorderClass->setObjectName(QString::fromUtf8("QScreenRecorderClass"));
        QScreenRecorderClass->resize(600, 41);
        centralWidget = new QWidget(QScreenRecorderClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        horizontalLayout = new QHBoxLayout(centralWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        startRecordButton = new QPushButton(centralWidget);
        startRecordButton->setObjectName(QString::fromUtf8("startRecordButton"));

        horizontalLayout->addWidget(startRecordButton);

        stopRecrodButton = new QPushButton(centralWidget);
        stopRecrodButton->setObjectName(QString::fromUtf8("stopRecrodButton"));

        horizontalLayout->addWidget(stopRecrodButton);

        frameBufferBar = new QProgressBar(centralWidget);
        frameBufferBar->setObjectName(QString::fromUtf8("frameBufferBar"));
        frameBufferBar->setValue(0);

        horizontalLayout->addWidget(frameBufferBar);

        maxElapsedLabel = new QLabel(centralWidget);
        maxElapsedLabel->setObjectName(QString::fromUtf8("maxElapsedLabel"));

        horizontalLayout->addWidget(maxElapsedLabel);

        minElapsedLabel = new QLabel(centralWidget);
        minElapsedLabel->setObjectName(QString::fromUtf8("minElapsedLabel"));

        horizontalLayout->addWidget(minElapsedLabel);

        curElapsedLabel = new QLabel(centralWidget);
        curElapsedLabel->setObjectName(QString::fromUtf8("curElapsedLabel"));

        horizontalLayout->addWidget(curElapsedLabel);

        horizontalSpacer = new QSpacerItem(498, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        QScreenRecorderClass->setCentralWidget(centralWidget);

        retranslateUi(QScreenRecorderClass);

        QMetaObject::connectSlotsByName(QScreenRecorderClass);
    } // setupUi

    void retranslateUi(QMainWindow *QScreenRecorderClass)
    {
        QScreenRecorderClass->setWindowTitle(QApplication::translate("QScreenRecorderClass", "QScreenRecorder", nullptr));
        startRecordButton->setText(QApplication::translate("QScreenRecorderClass", "Start", nullptr));
        stopRecrodButton->setText(QApplication::translate("QScreenRecorderClass", "Stop", nullptr));
        maxElapsedLabel->setText(QApplication::translate("QScreenRecorderClass", "Max: 0ms", nullptr));
        minElapsedLabel->setText(QApplication::translate("QScreenRecorderClass", "Min: 0ms", nullptr));
        curElapsedLabel->setText(QApplication::translate("QScreenRecorderClass", "Cur: 0ms", nullptr));
    } // retranslateUi

};

namespace Ui {
    class QScreenRecorderClass: public Ui_QScreenRecorderClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QSCREENRECORDER_H
