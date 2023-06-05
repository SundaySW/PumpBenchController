#include "scenariosdock.h"

#include <utility>
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
    connect(newItem.get(), &ScenariosItemBox::requestItemByName, [this, newItem](const QString& s){
        emit reqNewItemFromScenario(s, newItem.get());
    });
    connect(newItem.get(), &ScenariosItemBox::requestItemsList,[this, newItem](){
        emit reqViewItemsList(newItem.get());
    });
    connect(newItem.get(), &ScenariosItemBox::protosMsgToSend,[this](const QString& m){
        emit protosMsgToSend(m);
    });
    connect(newItem.get(), &ScenariosItemBox::deleteMe, [this, newItem](){
        disconnect(newItem.get(), &ScenariosItemBox::deleteMe, nullptr, nullptr);
        disconnect(newItem.get(), &ScenariosItemBox::requestItemByName, nullptr, nullptr);
        disconnect(newItem.get(), &ScenariosItemBox::requestItemsList, nullptr, nullptr);
        disconnect(newItem.get(), &ScenariosItemBox::protosMsgToSend, nullptr, nullptr);
        msgItemsList.removeOne(newItem);
    });
    ui->itemsLayout->addWidget(newItem.get());
    msgItemsList.append(newItem);
    emit reqViewItemsList(newItem.get());
}

ScenariosDock::~ScenariosDock()
{
    delete ui;
}
