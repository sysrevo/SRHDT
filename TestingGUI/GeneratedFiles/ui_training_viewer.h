/********************************************************************************
** Form generated from reading UI file 'training_viewer.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TRAINING_VIEWER_H
#define UI_TRAINING_VIEWER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TrainingViewer
{
public:
    QWidget *formLayoutWidget;
    QFormLayout *formLayout;
    QLabel *label_curr_layer;
    QLabel *label_val_curr_layer;
    QLabel *label_n_samples;
    QLabel *label_val_n_samples;
    QLabel *label_n_leaves;
    QLabel *label_val_n_leaves;
    QLabel *label_n_nodes;
    QLabel *label_val_n_nodes;
    QLabel *label_n_test;
    QLabel *label_val_n_test;

    void setupUi(QWidget *TrainingViewer)
    {
        if (TrainingViewer->objectName().isEmpty())
            TrainingViewer->setObjectName(QStringLiteral("TrainingViewer"));
        TrainingViewer->resize(600, 480);
        formLayoutWidget = new QWidget(TrainingViewer);
        formLayoutWidget->setObjectName(QStringLiteral("formLayoutWidget"));
        formLayoutWidget->setGeometry(QRect(120, 130, 321, 111));
        formLayout = new QFormLayout(formLayoutWidget);
        formLayout->setObjectName(QStringLiteral("formLayout"));
        formLayout->setContentsMargins(0, 0, 0, 0);
        label_curr_layer = new QLabel(formLayoutWidget);
        label_curr_layer->setObjectName(QStringLiteral("label_curr_layer"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label_curr_layer);

        label_val_curr_layer = new QLabel(formLayoutWidget);
        label_val_curr_layer->setObjectName(QStringLiteral("label_val_curr_layer"));

        formLayout->setWidget(0, QFormLayout::FieldRole, label_val_curr_layer);

        label_n_samples = new QLabel(formLayoutWidget);
        label_n_samples->setObjectName(QStringLiteral("label_n_samples"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_n_samples);

        label_val_n_samples = new QLabel(formLayoutWidget);
        label_val_n_samples->setObjectName(QStringLiteral("label_val_n_samples"));

        formLayout->setWidget(1, QFormLayout::FieldRole, label_val_n_samples);

        label_n_leaves = new QLabel(formLayoutWidget);
        label_n_leaves->setObjectName(QStringLiteral("label_n_leaves"));

        formLayout->setWidget(2, QFormLayout::LabelRole, label_n_leaves);

        label_val_n_leaves = new QLabel(formLayoutWidget);
        label_val_n_leaves->setObjectName(QStringLiteral("label_val_n_leaves"));

        formLayout->setWidget(2, QFormLayout::FieldRole, label_val_n_leaves);

        label_n_nodes = new QLabel(formLayoutWidget);
        label_n_nodes->setObjectName(QStringLiteral("label_n_nodes"));

        formLayout->setWidget(3, QFormLayout::LabelRole, label_n_nodes);

        label_val_n_nodes = new QLabel(formLayoutWidget);
        label_val_n_nodes->setObjectName(QStringLiteral("label_val_n_nodes"));

        formLayout->setWidget(3, QFormLayout::FieldRole, label_val_n_nodes);

        label_n_test = new QLabel(formLayoutWidget);
        label_n_test->setObjectName(QStringLiteral("label_n_test"));

        formLayout->setWidget(4, QFormLayout::LabelRole, label_n_test);

        label_val_n_test = new QLabel(formLayoutWidget);
        label_val_n_test->setObjectName(QStringLiteral("label_val_n_test"));

        formLayout->setWidget(4, QFormLayout::FieldRole, label_val_n_test);


        retranslateUi(TrainingViewer);

        QMetaObject::connectSlotsByName(TrainingViewer);
    } // setupUi

    void retranslateUi(QWidget *TrainingViewer)
    {
        TrainingViewer->setWindowTitle(QApplication::translate("TrainingViewer", "Form", nullptr));
        label_curr_layer->setText(QApplication::translate("TrainingViewer", "Current Layer", nullptr));
        label_val_curr_layer->setText(QApplication::translate("TrainingViewer", "TextLabel", nullptr));
        label_n_samples->setText(QApplication::translate("TrainingViewer", "Number of samples", nullptr));
        label_val_n_samples->setText(QApplication::translate("TrainingViewer", "TextLabel", nullptr));
        label_n_leaves->setText(QApplication::translate("TrainingViewer", "Number of leaves", nullptr));
        label_val_n_leaves->setText(QApplication::translate("TrainingViewer", "TextLabel", nullptr));
        label_n_nodes->setText(QApplication::translate("TrainingViewer", "Number of nodes", nullptr));
        label_val_n_nodes->setText(QApplication::translate("TrainingViewer", "TextLabel", nullptr));
        label_n_test->setText(QApplication::translate("TrainingViewer", "Number of test generated", nullptr));
        label_val_n_test->setText(QApplication::translate("TrainingViewer", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TrainingViewer: public Ui_TrainingViewer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TRAINING_VIEWER_H
