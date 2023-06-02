//
// Created by user on 05.05.2023.
//
#include "BenchController.h"

#ifdef _BUILD_TYPE_
#define CURRENT_BUILD_TYPE_ _BUILD_TYPE_
#else
#define CURRENT_BUILD_TYPE_ "CHECK CMAKE"
#endif

#define serverReconnectionTime 1000
#define plotsReDrawTime 1000


QRandomGenerator generator = QRandomGenerator();

BenchController::BenchController(Ui::MainWindow *_ui, QMainWindow* mw):
        socketAdapter(new SocketAdapter()),
        paramService(new ParamService()),
        ui(_ui),
        mainWindow(mw),
        serverConnectionDlg(new ServerConnectionDlg(socketAdapter, mw)),
        plotReDrawTimer(new QTimer(this)),
        serverReconnectionTimer(new QTimer(this)),
        scenariosDock(new ScenariosDock())
{
    ui->setupUi(mw);
    jsonSaved = QJsonObject();
    paramService->setSocketAdapter(socketAdapter);
    paramService->loadParams(jsonSaved);
    setView();
    setViewMap();
    makeConnections();
    loadFromJson();
    mw->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, scenariosDock.get());

}

void BenchController::setView(){
    logOkIcon = QIcon(":/icons/ok_circle.svg");
    logErrorIcon = QIcon(":/icons/error_circle.svg");
    statusDefIcon = QIcon(":/item_icons/item_svg/sktb_logo_def.svg");
    statusOkIcon = QIcon(":/item_icons/item_svg/sktb_logo_active.svg");
    statusErrorIcon = QIcon(":/item_icons/item_svg/sktb_logo_error.svg");
    serverConnectedIcon = QIcon(":/icons/server_connected.svg");
    serverDisconnectedIcon = QIcon(":/icons/server_disconnected.svg");
}

void BenchController::loadFromJson(){
    auto pathToFile = QString(CURRENT_BUILD_TYPE_) == "Debug" ? "/../" : "/";
    auto configFile = new QFile(QCoreApplication::applicationDirPath() + QString("%1/saved.json").arg(pathToFile));
    configFile->open(QIODevice::ReadWrite);
    QByteArray saveData = configFile->readAll();
    QJsonDocument jsonDocument(QJsonDocument::fromJson(saveData));
    jsonSaved = jsonDocument.object();

    paramService->loadParams(jsonSaved);

    QJsonArray paramArr = jsonSaved["UpdateItems"].toArray();
    for(const auto& param : paramArr)
    {
        auto loadItemJson = param.toObject();
        if(!loadItemJson.isEmpty())
            updateItemsMap.value(loadItemJson["ItemName"].toString())->loadDataFromJson(loadItemJson);
    }
    controlItem->loadDataFromJson(jsonSaved["ControlItem"].toObject());
    serverConnectionDlg->loadDataFromJson(jsonSaved["serverConnection"].toObject());
}

void BenchController::saveToJson() {
    paramService->saveParams(jsonSaved);

    QJsonArray paramArr;
    for(auto& p: updateItemsMap)
        paramArr.append(p->saveDataToJSon());
    jsonSaved["UpdateItems"] = paramArr;
    jsonSaved["ControlItem"] = controlItem->saveDataToJSon();
    jsonSaved["serverConnection"] = serverConnectionDlg->saveDataToJson();

    QJsonDocument doc;
    doc.setObject(jsonSaved);
    auto pathToFile = QString(CURRENT_BUILD_TYPE_) == "Debug" ? "/../" : "/";
    auto configFile = new QFile(QCoreApplication::applicationDirPath() + QString("%1/saved.json").arg(pathToFile));
    configFile->open(QIODevice::ReadWrite);
    configFile->resize(0);
    configFile->write(doc.toJson(QJsonDocument::Indented));
    configFile->close();
}

void BenchController::setViewMap(){
    updateItemsMap.insert("rpm", QSharedPointer<BenchViewItem>(
            new BenchViewItem("rpm", ui->rpm_plot, ui->rpm_label, ui->rpm_button, paramService.get(), mainWindow)));
    updateItemsMap.insert("flow", QSharedPointer<BenchViewItem>(
            new BenchViewItem("flow", ui->flow_plot, ui->flow_label, ui->flow_button, paramService.get(), mainWindow)));
    updateItemsMap.insert("in_gauge", QSharedPointer<BenchViewItem>(
            new BenchViewItem("in_gauge", ui->in_gauge_plot, ui->in_gauge_label, ui->in_gauge_button, paramService.get(), mainWindow)));
    updateItemsMap.insert("out_gauge", QSharedPointer<BenchViewItem>(
            new BenchViewItem("out_gauge", ui->out_gauge_plot, ui->out_gauge_label, ui->out_gauge_button, paramService.get(), mainWindow)));
    updateItemsMap.insert("tank_temp", QSharedPointer<BenchViewItem>(
            new BenchViewItem("tank_temp", ui->tank_temp_plot, ui->tank_temp_label, ui->tank_temp_button, paramService.get(), mainWindow)));
    updateItemsMap.insert("motor_temp", QSharedPointer<BenchViewItem>(
            new BenchViewItem("motor_temp", ui->motor_temp_plot, ui->motor_temp_label, ui->Motor_Temp_Button, paramService.get(), mainWindow)));
    updateItemsMap.insert("valve_feedback", QSharedPointer<BenchViewItem>(
            new BenchViewItem("valve_feedback", ui->valve_plot, ui->valve_label, ui->valve_button, paramService.get(), mainWindow)));
    controlItem = QSharedPointer<BenchViewCtrlItem>(
            new BenchViewCtrlItem("valveSet", paramService.get(), ui->setParamConfig_pushButton,
                                  ui->setParamValue_lineEdit, ui->pidTargetValue_lineEdit, ui->pidEnable_pushButton,
                                  ui->sendParamValue_button, ui->pidValue_comboBox, ui->valveSet_slider, mainWindow));
    sendItemsNameLogoListToComboBoxes(controlItem.get());
}

void BenchController::makeConnections(){
    connect(controlItem.get(), &BenchViewCtrlItem::requestItemsList, [this](){
        sendItemsNameLogoListToComboBoxes(controlItem.get());});
    connect(controlItem.get(), &BenchViewCtrlItem::requestParamKeyByName, [this](const QString& s){ sendItemFromName(s, controlItem.get());});
    connect(serverReconnectionTimer, &QTimer::timeout, [this]() { serverConnectionHandler(); });
    serverReconnectionTimer->start(serverReconnectionTime);
    connect(plotReDrawTimer, &QTimer::timeout, [this]() { plotReDrawTimerHandler();});
    plotReDrawTimer->start(plotsReDrawTime);
    for(const auto& viewItem : updateItemsMap){
        connect(viewItem.get(), &BenchViewItem::signalValueUpdated_itemName, [this](const QString& name){
            onItemNewValue(name);
        });
    }
    connect(ui->server_button, &QPushButton::clicked, [this](){ serverBtnClicked();});
    connect(serverConnectionDlg, &ServerConnectionDlg::eventInServerConnection, [this](const QString& s, bool b){ eventServerConnectionHandler(s, b);});
    connect(ui->status_button, &QPushButton::clicked, [this](){ statusBtnClicked(); });
    connect(ui->settings_button_2, &QPushButton::clicked, [this](){ saveToJson(); });
}

void BenchController::plotReDrawTimerHandler(){
    for(const auto& viewItem : updateItemsMap)
        viewItem->repaintPlot();
}

void BenchController::serverConnectionHandler(){
    if(serverConnectionDlg->reconnectIsOn() && !socketAdapter->IsConnected())
        serverConnectionDlg->connectToServer();
}

void BenchController::serverBtnClicked(){
    serverConnectionDlg->show();
    serverConnectionDlg->raise();
}

void BenchController::statusBtnClicked(){
    for(QListWidgetItem* item : ui->status_listWidget->selectedItems()){
        ui->status_listWidget->removeItemWidget(item);
        delete item;
    }
    hasError = false;
    ui->status_listWidget->clear();
    if(socketAdapter->IsConnected())
        ui->status_button->setIcon(statusOkIcon);
}

void BenchController::onItemNewValue(const QString& name){
    if(updateItemsMap.contains(name)){
        auto item = updateItemsMap[name];
        bool noError = item->getCurrentStatus();
        if(!noError){
            updateErrorStatusList(item.get());
            if(!hasError)
                ui->status_button->setIcon(statusErrorIcon);
            hasError = true;
        }
    }
}

void BenchController::updateErrorStatusList(BenchViewItem* item){
    auto itemStr = QString("%1 at %2\nvalue: %3").arg(item->getName().toUpper(), item->getLastValueDateTimeStr(), item->getCurrentValue().toString());
    auto newItem = new QListWidgetItem(item->getErrorIcon(), itemStr);
    ui->status_listWidget->addItem(newItem);
}

void BenchController::sendItemsNameLogoListToComboBoxes(BenchViewCtrlItem* customer){
    auto viewItemDataVec = QVector<BenchViewItem::ViewItemData>();
    viewItemDataVec.reserve(updateItemsMap.size());
    for(const auto& viewItem : updateItemsMap)
        viewItemDataVec.append(BenchViewItem::ViewItemData{viewItem->getIcon(),viewItem->getName()});
    customer->receiveItemsNameList(viewItemDataVec);
}

void BenchController::sendItemFromName(const QString &itemName, BenchViewCtrlItem *customer) {
    auto item = updateItemsMap.value(itemName);
    if(item)
        customer->receiveItem(item);
}

void BenchController::eventServerConnectionHandler(const QString& eventStr, bool isError){
    if(isError){
        ui->log_listWidget->addItem(eventStr);
        ui->log_listWidget->item(ui->log_listWidget->count()-1)->setIcon(logErrorIcon);
    } else{
        ui->log_listWidget->addItem(eventStr);
        ui->log_listWidget->item(ui->log_listWidget->count()-1)->setIcon(logOkIcon);
    }
    ui->server_button->setIcon(socketAdapter->IsConnected() ? serverConnectedIcon : serverDisconnectedIcon);
    if(!hasError)
        ui->status_button->setIcon(socketAdapter->IsConnected() ? statusOkIcon : statusErrorIcon);
}