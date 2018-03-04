/********************************************************************************
** Form generated from reading UI file 'image_viewer.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_IMAGE_VIEWER_H
#define UI_IMAGE_VIEWER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ImageViewerWidget
{
public:
    QLabel *img;

    void setupUi(QWidget *ImageViewerWidget)
    {
        if (ImageViewerWidget->objectName().isEmpty())
            ImageViewerWidget->setObjectName(QStringLiteral("ImageViewerWidget"));
        ImageViewerWidget->resize(400, 300);
        img = new QLabel(ImageViewerWidget);
        img->setObjectName(QStringLiteral("img"));
        img->setGeometry(QRect(10, 20, 371, 261));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(img->sizePolicy().hasHeightForWidth());
        img->setSizePolicy(sizePolicy);

        retranslateUi(ImageViewerWidget);

        QMetaObject::connectSlotsByName(ImageViewerWidget);
    } // setupUi

    void retranslateUi(QWidget *ImageViewerWidget)
    {
        ImageViewerWidget->setWindowTitle(QApplication::translate("ImageViewerWidget", "Form", nullptr));
        img->setText(QApplication::translate("ImageViewerWidget", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ImageViewerWidget: public Ui_ImageViewerWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_IMAGE_VIEWER_H
