#include "scenariosdock.h"
#include "ui_scenariosdock.h"

ScenariosDock::ScenariosDock(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ScenariosDock)
{
    ui->setupUi(this);
    connect(ui->addMsgScenario_pushButton, &QPushButton::clicked, [this](){ addMsgItem();});
    connect(ui->addSetScenario_pushButton, &QPushButton::clicked, [this](){ ;});
}

void ScenariosDock::addMsgItem(){
    auto newItem = QSharedPointer<ScenariosItemBox>(new ScenariosItemBox(this));
    connect(newItem.get(), &ScenariosItemBox::deleteMe, [this, newItem](){
        disconnect(newItem.get(), &ScenariosItemBox::deleteMe, 0, 0);
        msgItemsList.removeOne(newItem);
    });
    ui->itemsLayout->addWidget(newItem.get());
    msgItemsList.append(newItem);
}

ScenariosDock::~ScenariosDock()
{
    delete ui;
}
