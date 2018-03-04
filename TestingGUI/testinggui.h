#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_testinggui.h"

class TestingGUI : public QMainWindow
{
    Q_OBJECT

public:
    TestingGUI(QWidget *parent = Q_NULLPTR);

private:
    Ui::TestingGUIClass ui;
};
