#include "scenariositembox.h"
#include "ui_scenariositembox.h"

ScenariosItemBox::ScenariosItemBox(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::ScenariosItemBox)
{
    ui->setupUi(this);
    connect(ui->remove_pushButton, &QPushButton::clicked, [this](){emit deleteMe();});
}

ScenariosItemBox::~ScenariosItemBox()
{
    delete ui;
}
