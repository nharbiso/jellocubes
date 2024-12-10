#pragma once

#include <QMainWindow>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include "realtime.h"
#include "utils/aspectratiowidget/aspectratiowidget.hpp"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    void initialize();
    void finish();

private:
    Realtime *realtime;
    AspectRatioWidget *aspectRatioWidget;

    void addSlider(QVBoxLayout* vLayout, QString label, double minVal,
                   double maxVal, double step, double initVal, int maxDenom,
                   double* settingsVal);
};
