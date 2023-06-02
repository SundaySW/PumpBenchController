#include "scenariositemcontrol.h"
#include "ui_scenariositemcontrol.h"

ScenariosItemControl::ScenariosItemControl(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::ScenariosItemControl)
{
    ui->setupUi(this);
}

ScenariosItemControl::~ScenariosItemControl()
{
    delete ui;
}
