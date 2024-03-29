#include "scenariosdock.h"

#include <utility>
#include "ui_scenariosdock.h"

ScenariosDock::ScenariosDock(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ScenariosDock)
{
    ui->setupUi(this);
    connect(ui->addMsgScenario_pushButton, &QPushButton::clicked, [this](){ addMsgItem();});
    connect(ui->addSetScenario_pushButton, &QPushButton::clicked, [this](){
        QMessageBox msgBox(this);
        msgBox.setText("Bzzzzzzzz!");
        msgBox.setIconPixmap(QPixmap(":/icons/bee.svg"));
        msgBox.exec();
    });
}

ScenariosItemBox* ScenariosDock::addMsgItem(){
    auto shrd_ptr = QSharedPointer<ScenariosItemBox>(new ScenariosItemBox(this));
    auto* scenariosItemBoxPtr = shrd_ptr.get();
    emit reqViewItemsList(scenariosItemBoxPtr);
    scenariosItemBoxPtr->setTitle(QString("Action №%1").arg(++scenarioCount));
    msgItemsMap.insert(scenariosItemBoxPtr, shrd_ptr);

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

    ui->itemsLayout->insertWidget(0, scenariosItemBoxPtr);
    return scenariosItemBoxPtr;
}

ScenariosDock::~ScenariosDock()
{
    delete ui;
}

void ScenariosDock::loadDataFromJson(const QJsonObject& scenarioArr){
    if(scenarioArr.empty())
        return;
    auto msgScenarios = scenarioArr["MsgScenarios"].toArray();
    for(const auto& scenario : msgScenarios)
    {
        auto loadItemJson = scenario.toObject();
        if(!loadItemJson.isEmpty())
            addMsgItem()->loadDataFromJson(loadItemJson);
    }
}

QJsonObject ScenariosDock::SaveDataToJson(){
    auto retVal = QJsonObject();
    QJsonArray paramArr;
    for(auto& s: msgItemsMap)
        paramArr.append(s->saveDataToJSon());
    retVal["MsgScenarios"] = paramArr;
    return retVal;
}