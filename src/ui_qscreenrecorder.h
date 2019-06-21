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
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QScreenRecorderClass
{
public:
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *selectRectButton;
    QSpacerItem *horizontalSpacer;

    void setupUi(QMainWindow *QScreenRecorderClass)
    {
        if (QScreenRecorderClass->objectName().isEmpty())
            QScreenRecorderClass->setObjectName(QString::fromUtf8("QScreenRecorderClass"));
        QScreenRecorderClass->resize(600, 160);
        centralWidget = new QWidget(QScreenRecorderClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        horizontalLayout = new QHBoxLayout(centralWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        selectRectButton = new QPushButton(centralWidget);
        selectRectButton->setObjectName(QString::fromUtf8("selectRectButton"));

        horizontalLayout->addWidget(selectRectButton);

        horizontalSpacer = new QSpacerItem(498, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        QScreenRecorderClass->setCentralWidget(centralWidget);

        retranslateUi(QScreenRecorderClass);

        QMetaObject::connectSlotsByName(QScreenRecorderClass);
    } // setupUi

    void retranslateUi(QMainWindow *QScreenRecorderClass)
    {
        QScreenRecorderClass->setWindowTitle(QApplication::translate("QScreenRecorderClass", "QScreenRecorder", nullptr));
        selectRectButton->setText(QApplication::translate("QScreenRecorderClass", "Select Rect", nullptr));
    } // retranslateUi

};

namespace Ui {
    class QScreenRecorderClass: public Ui_QScreenRecorderClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QSCREENRECORDER_H
