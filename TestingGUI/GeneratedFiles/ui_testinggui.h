/********************************************************************************
** Form generated from reading UI file 'testinggui.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TESTINGGUI_H
#define UI_TESTINGGUI_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TestingGUIClass
{
public:
    QWidget *centralWidget;
    QPushButton *test_learn_button;

    void setupUi(QMainWindow *TestingGUIClass)
    {
        if (TestingGUIClass->objectName().isEmpty())
            TestingGUIClass->setObjectName(QStringLiteral("TestingGUIClass"));
        TestingGUIClass->resize(600, 400);
        centralWidget = new QWidget(TestingGUIClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        test_learn_button = new QPushButton(centralWidget);
        test_learn_button->setObjectName(QStringLiteral("test_learn_button"));
        test_learn_button->setGeometry(QRect(130, 130, 75, 23));
        TestingGUIClass->setCentralWidget(centralWidget);

        retranslateUi(TestingGUIClass);

        QMetaObject::connectSlotsByName(TestingGUIClass);
    } // setupUi

    void retranslateUi(QMainWindow *TestingGUIClass)
    {
        TestingGUIClass->setWindowTitle(QApplication::translate("TestingGUIClass", "TestingGUI", nullptr));
        test_learn_button->setText(QApplication::translate("TestingGUIClass", "PushButton", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TestingGUIClass: public Ui_TestingGUIClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TESTINGGUI_H
