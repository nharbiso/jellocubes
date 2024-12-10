#include "mainwindow.h"
#include "settings.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QSettings>
#include <QLabel>
#include <QGroupBox>
#include <iostream>

void MainWindow::initialize() {
    realtime = new Realtime;
    aspectRatioWidget = new AspectRatioWidget(this);
    aspectRatioWidget->setAspectWidget(realtime, 3.f/4.f);
    QHBoxLayout* hLayout = new QHBoxLayout; // horizontal alignment
    QVBoxLayout* vLayout = new QVBoxLayout(); // vertical alignment
    vLayout->setAlignment(Qt::AlignTop);
    hLayout->addLayout(vLayout);
    hLayout->addWidget(aspectRatioWidget, 1);
    this->setLayout(hLayout);

    // Buttons for manipulating scene
    QPushButton* reset = new QPushButton();
    reset->setText(QStringLiteral("Reset Scene"));
    connect(reset, &QPushButton::clicked, this, [reset, this]() {
        this->realtime->resetScene();
    });
    vLayout->addWidget(reset);

    QPushButton* scatter = new QPushButton();
    scatter->setText(QStringLiteral("Scatter cube"));
    connect(scatter, &QPushButton::clicked, this, [scatter, this]() {
        this->realtime->scatterCube();
    });
    vLayout->addWidget(scatter);

    // Add simulation parameters
    QFont font;
    font.setPointSize(12);
    font.setBold(true);
    QLabel* parameters_label = new QLabel(); // Parameters label
    parameters_label->setText("Simulation Parameters");
    parameters_label->setFont(font);
    vLayout->addWidget(parameters_label);

    this->addSlider(vLayout, "Time step (ms)", 1, 10, 0.1, settings.dt, 10, &settings.dt);
    this->addSlider(vLayout, "Hook's constant (cube)", 100, 10000, 50, settings.kElastic, 1, &settings.kElastic);
    this->addSlider(vLayout, "Damping constant (cube)", 0.1, 10, 0.05, settings.dElastic, 20, &settings.dElastic);
    this->addSlider(vLayout, "Hook's constant (bounds)", 100, 10000, 50, settings.kCollision, 1, &settings.kCollision);
    this->addSlider(vLayout, "Damping constant (bounds)", 0.1, 10, 0.05, settings.dCollision, 20, &settings.dCollision);
    this->addSlider(vLayout, "Node mass", 0.01, 100, 0.01, settings.mass, 100, &settings.mass);
    this->addSlider(vLayout, "Gravity", 0, 500, 1, settings.gravity, 1, &settings.gravity);

    // Add other scene options
    QLabel* options_label = new QLabel(); // Options label
    options_label->setText("Scene Options");
    options_label->setFont(font);
    vLayout->addWidget(options_label);

    QCheckBox* texture = new QCheckBox();
    texture->setText(QStringLiteral("Texture Mapping"));
    texture->setChecked(false);
    connect(texture, &QCheckBox::clicked, this, [texture, this]{
        settings.textureMappingEnabled = !settings.textureMappingEnabled;
    });
    vLayout->addWidget(texture);

    QCheckBox* transparent = new QCheckBox();
    transparent->setText(QStringLiteral("Transparent Cube"));
    transparent->setChecked(false);
    connect(transparent, &QCheckBox::clicked, this, [transparent, this]{
        settings.transparentCube = !settings.transparentCube;
    });
    vLayout->addWidget(transparent);
}

void MainWindow::finish() {
    realtime->finish();
    delete(realtime);
}

void MainWindow::addSlider(QVBoxLayout* vLayout, QString label, double minVal,
                           double maxVal, double step, double initVal, int maxDenom,
                           double* settingsVal) {
    QLabel* dt_label = new QLabel();
    dt_label->setText(label);
    vLayout->addWidget(dt_label);

    QSlider* dtSlider = new QSlider(Qt::Orientation::Horizontal);
    dtSlider->setMinimum(minVal * maxDenom);
    dtSlider->setMaximum(maxVal * maxDenom);
    dtSlider->setTickInterval(step * maxDenom);
    dtSlider->setValue(initVal * maxDenom);

    QDoubleSpinBox* dtSpinBox = new QDoubleSpinBox();
    dtSpinBox->setMinimum(minVal);
    dtSpinBox->setMaximum(maxVal);
    dtSpinBox->setSingleStep(step);
    dtSpinBox->setValue(initVal);

    connect(dtSlider, &QSlider::valueChanged, this,
            [this, dtSlider, dtSpinBox, maxDenom, settingsVal](int newValue) {
        dtSpinBox->setValue((double) newValue / maxDenom);
        *settingsVal = dtSpinBox->value();
    });
    connect(dtSpinBox, &QDoubleSpinBox::valueChanged, this,
            [this, dtSlider, dtSpinBox, maxDenom, settingsVal](double newValue) {
        dtSlider->setValue(newValue * maxDenom);
        *settingsVal = dtSpinBox->value();
    });

    QHBoxLayout* dtLayout = new QHBoxLayout();
    dtLayout->addWidget(dtSlider);
    dtLayout->addWidget(dtSpinBox);
    QGroupBox* dtBox = new QGroupBox();
    dtBox->setLayout(dtLayout);
    vLayout->addWidget(dtBox);
}
