#include "scenariosdock.h"

#include <utility>
#include "ui_scenariosdock.h"

ScenariosDock::ScenariosDock(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ScenariosDock)
{
    ui->setupUi(this);
    connect(ui->addMsgScenario_pushButton, &QPushButton::clicked, [this](){ addMsgItem();});
//    connect(ui->addSetScenario_pushButton, &QPushButton::clicked, [this](){ ;});
}

void ScenariosDock::addMsgItem(){
    auto shrdPtr = QSharedPointer<ScenariosItemBox>(new ScenariosItemBox(this));
    auto* scenariosItemBoxPtr = shrdPtr.get();
    msgItemsMap.insert(scenariosItemBoxPtr, shrdPtr);
    connect(scenariosItemBoxPtr, &ScenariosItemBox::requestItemByName, [this, scenariosItemBoxPtr](const QString& name){
        emit reqNewItemFromScenario(name, scenariosItemBoxPtr);
    });
    connect(scenariosItemBoxPtr, &ScenariosItemBox::requestItemsList,[this, scenariosItemBoxPtr](){
        emit reqViewItemsList(scenariosItemBoxPtr);
    });
    connect(scenariosItemBoxPtr, &ScenariosItemBox::protosMsgToSend,[this](const QString& m){
        emit protosMsgToSend(m);
    });
    connect(scenariosItemBoxPtr, &ScenariosItemBox::deleteMe, [this, scenariosItemBoxPtr](){
        disconnect(scenariosItemBoxPtr, nullptr, this, nullptr);
        msgItemsMap.remove(scenariosItemBoxPtr);
        ui->itemsLayout->removeWidget(scenariosItemBoxPtr);
    });
    ui->itemsLayout->addWidget(scenariosItemBoxPtr);
    emit reqViewItemsList(scenariosItemBoxPtr);
}

ScenariosDock::~ScenariosDock()
{
    delete ui;
}