//
// Created by user on 05.05.2023.
//
#include "BenchController.h"
#include "config.h"

#define kServerReconnectionTime 1000
#define kPlotsReDrawTime 1000

QRandomGenerator generator = QRandomGenerator();

BenchController::BenchController(Ui::MainWindow *_ui, QMainWindow* mw):
        socketAdapter_(new SocketAdapter()),
        paramService_(new ParamService()),
        ui(_ui),
        mainWindow(mw),
        serverConnectionDlg_(new ServerConnectionDlg(socketAdapter_, mw)),
        experimentSettingsDlg_(new ExperimentSettings(mw)),
        plotReDrawTimer_(new QTimer(this)),
        serverReconnectionTimer_(new QTimer(this)),
        scenariosDock_(new ScenariosDock(mw)),
        key_pad_dock_(new KeyPad(mw))
{
    ui->setupUi(mw);
    jsonSaved_ = QJsonObject();
    paramService_->setSocketAdapter(socketAdapter_);
    paramService_->loadParams(jsonSaved_);
    setViewIcons();
    makeItems();
    makeConnections();
    makeScenarioDock();
    makeExperimentDLg();
    makeKeyPadDock();
    loadFromJson();
}

void BenchController::makeKeyPadDock(){
    mainWindow->addDockWidget(Qt::DockWidgetArea::TopDockWidgetArea, key_pad_dock_.get());
    connect(key_pad_dock_.get(), &KeyPad::protosMsgToSend, [this](const QString& m){
        socketAdapter_->SendMsg(m);
    });
}

void BenchController::makeExperimentDLg(){
    experimentSettingsDlg_->LoadPtrs(controlItem_, paramService_);
    sendItemsNameLogoListToComboBoxes(experimentSettingsDlg_);
    connect(controlItem_.get(), &BenchViewCtrlItem::newFeedBackItem, [this](const QString& s){
        experimentSettingsDlg_->NewFeedBackParamName(s);
    });
}

void BenchController::makeScenarioDock(){
    mainWindow->addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, scenariosDock_.get());
    connect(scenariosDock_.get(), &ScenariosDock::reqViewItemsList, [this](ScenariosItemBox* item){
        sendItemsNameLogoListToComboBoxes(item);
    });
    connect(scenariosDock_.get(), &ScenariosDock::reqNewItemFromScenario, [this](const QString& n, ScenariosItemBox* c){
        sendItemFromName(n, c);
    });
    connect(scenariosDock_.get(), &ScenariosDock::protosMsgToSend, [this](const QString& m){
        socketAdapter_->SendMsg(m);
    });
}

void BenchController::setViewIcons(){
    logOkIcon = QIcon(":/icons/ok_circle.svg");
    logErrorIcon = QIcon(":/icons/error_circle.svg");
    statusDefIcon = QIcon(":/item_icons/item_svg/sktb_logo_def.svg");
    statusOkIcon = QIcon(":/item_icons/item_svg/sktb_logo_active.svg");
    statusErrorIcon = QIcon(":/item_icons/item_svg/sktb_logo_error.svg");
    serverConnectedIcon = QIcon(":/icons/server_connected.svg");
    serverDisconnectedIcon = QIcon(":/icons/server_disconnected.svg");
}

void BenchController::loadFromJson(){
    auto pathToFile = QString(kBUILD_TYPE) == "Debug" ? "/../" : "/";
    auto configFile = new QFile(QCoreApplication::applicationDirPath() + QString("%1/saved.json").arg(pathToFile));
    configFile->open(QIODevice::ReadWrite);
    QByteArray saveData = configFile->readAll();
    QJsonDocument jsonDocument(QJsonDocument::fromJson(saveData));
    jsonSaved_ = jsonDocument.object();

    paramService_->loadParams(jsonSaved_);

    QJsonArray paramArr = jsonSaved_["UpdateItems"].toArray();
    for(const auto& param : paramArr)
    {
        auto loadItemJson = param.toObject();
        if(!loadItemJson.isEmpty())
            updateItemsMap_.value(loadItemJson["ItemName"].toString())->loadDataFromJson(loadItemJson);
    }
    controlItem_->loadDataFromJson(jsonSaved_["ControlItem"].toObject());
    serverConnectionDlg_->loadDataFromJson(jsonSaved_["serverConnection"].toObject());
    scenariosDock_->loadDataFromJson(jsonSaved_["Scenarios"].toObject());
    experimentSettingsDlg_->LoadDataFromJson(jsonSaved_["Experiment"].toObject());
    key_pad_dock_->LoadDataFromJson(jsonSaved_["KeyPad"].toObject());
}

void BenchController::saveToJson() {
    paramService_->saveParams(jsonSaved_);
    QJsonArray paramArr;
    for(auto& p: updateItemsMap_)
        paramArr.append(p->SaveDataToJSon());
    jsonSaved_["UpdateItems"] = paramArr;
    jsonSaved_["ControlItem"] = controlItem_->SaveDataToJson();
    jsonSaved_["serverConnection"] = serverConnectionDlg_->SaveDataToJson();
    jsonSaved_["Scenarios"] = scenariosDock_->SaveDataToJson();
    jsonSaved_["Experiment"] = experimentSettingsDlg_->SaveDataToJson();
    jsonSaved_["KeyPad"] = key_pad_dock_->SaveDataToJson();

    QJsonDocument doc;
    doc.setObject(jsonSaved_);
    auto pathToFile = QString(kBUILD_TYPE) == "Debug" ? "/../" : "/";
    auto configFile = new QFile(QCoreApplication::applicationDirPath() + QString("%1/saved.json").arg(pathToFile));
    configFile->open(QIODevice::ReadWrite);
    configFile->resize(0);
    configFile->write(doc.toJson(QJsonDocument::Indented));
    configFile->close();
}

void BenchController::makeItems(){
    updateItemsMap_.insert("rpm", QSharedPointer<BenchViewItem>(
            new BenchViewItem("rpm", ui->rpm_plot, ui->rpm_label, ui->rpm_button, paramService_.get(), mainWindow)));
    updateItemsMap_.insert("flow", QSharedPointer<BenchViewItem>(
            new BenchViewItem("flow", ui->flow_plot, ui->flow_label, ui->flow_button, paramService_.get(), mainWindow)));
    updateItemsMap_.insert("in_gauge", QSharedPointer<BenchViewItem>(
            new BenchViewItem("in_gauge", ui->in_gauge_plot, ui->in_gauge_label, ui->in_gauge_button, paramService_.get(), mainWindow)));
    updateItemsMap_.insert("out_gauge", QSharedPointer<BenchViewItem>(
            new BenchViewItem("out_gauge", ui->out_gauge_plot, ui->out_gauge_label, ui->out_gauge_button, paramService_.get(), mainWindow)));
    updateItemsMap_.insert("tank_temp", QSharedPointer<BenchViewItem>(
            new BenchViewItem("tank_temp", ui->tank_temp_plot, ui->tank_temp_label, ui->tank_temp_button, paramService_.get(), mainWindow)));
    updateItemsMap_.insert("motor_temp", QSharedPointer<BenchViewItem>(
            new BenchViewItem("motor_temp", ui->motor_temp_plot, ui->motor_temp_label, ui->Motor_Temp_Button, paramService_.get(), mainWindow)));
    updateItemsMap_.insert("valve_feedback", QSharedPointer<BenchViewItem>(
            new BenchViewItem("valve_feedback", ui->valve_plot, ui->valve_label, ui->valve_button, paramService_.get(), mainWindow)));
    controlItem_ = QSharedPointer<BenchViewCtrlItem>(
            new BenchViewCtrlItem("valveSet", paramService_.get(), ui->setParamConfig_pushButton,
                                  ui->setParamValue_lineEdit, ui->pidTargetValue_lineEdit, ui->pidEnable_pushButton,
                                  ui->sendParamValue_button, ui->pidValue_comboBox, ui->valveSet_slider, mainWindow));
    sendItemsNameLogoListToComboBoxes(controlItem_.get());

    connect(controlItem_.get(), &BenchViewCtrlItem::requestItemsList, [this](){
        sendItemsNameLogoListToComboBoxes(controlItem_.get());});
    connect(controlItem_.get(), &BenchViewCtrlItem::requestParamKeyByName, [this](const QString& s){ sendItemFromName(s, controlItem_.get());});
}

void BenchController::makeConnections(){
    connect(serverReconnectionTimer_, &QTimer::timeout, [this]() { serverConnectionHandler(); });
    serverReconnectionTimer_->start(kServerReconnectionTime);
    connect(plotReDrawTimer_, &QTimer::timeout, [this]() { plotReDrawTimerHandler();});
    plotReDrawTimer_->start(kPlotsReDrawTime);
    for(const auto& viewItem : updateItemsMap_){
        connect(viewItem.get(), &BenchViewItem::signalValueUpdated_itemName, [this](const QString& name){
            onItemNewValue(name);
        });
    }
    connect(ui->status_button, &QPushButton::clicked, [this](){ statusBtnClicked(); });
    connect(ui->server_button, &QPushButton::clicked, [this](){ serverBtnClicked(); });
    connect(ui->save_button, &QPushButton::clicked, [this](){ saveToJson(); });
    connect(ui->experiment_button, &QPushButton::clicked, [this](){ experimentBtnClicked(); });

    connect(serverConnectionDlg_, &ServerConnectionDlg::eventInServerConnection,
            [this](const QString& s, bool b){ eventServerConnectionHandler(s, b);});
    connect(ui->log_listWidget, &QListWidget::itemDoubleClicked, [this](){ ui->log_listWidget->clear(); });
}

void BenchController::plotReDrawTimerHandler(){
    for(const auto& viewItem : updateItemsMap_)
        viewItem->repaintPlot();
}

void BenchController::serverConnectionHandler(){
    if(serverConnectionDlg_->reconnectIsOn() && !socketAdapter_->IsConnected())
        serverConnectionDlg_->connectToServer();
}

void BenchController::serverBtnClicked(){
    serverConnectionDlg_->show();
    serverConnectionDlg_->raise();
}

void BenchController::experimentBtnClicked(){
    experimentSettingsDlg_->show();
    experimentSettingsDlg_->raise();
}

void BenchController::statusBtnClicked(){
    for(QListWidgetItem* item : ui->status_listWidget->selectedItems()){
        ui->status_listWidget->removeItemWidget(item);
        delete item;
    }
    hasError_ = false;
    ui->status_listWidget->clear();
    if(socketAdapter_->IsConnected())
        ui->status_button->setIcon(statusOkIcon);
}

void BenchController::onItemNewValue(const QString& name){
    if(updateItemsMap_.contains(name)){
        auto item = updateItemsMap_[name];
        bool noError = item->getCurrentStatus();
        if(!noError){
            updateErrorStatusList(item.get());
            if(!hasError_)
                ui->status_button->setIcon(statusErrorIcon);
            hasError_ = true;
        }
    }
}

void BenchController::updateErrorStatusList(BenchViewItem* item){
    auto itemStr = QString("%1 at %2\nvalue: %3").arg(item->getName().toUpper(), item->getLastValueDateTimeStr(), item->getCurrentValue().toString());
    auto newItem = new QListWidgetItem(item->getErrorIcon(), itemStr);
    ui->status_listWidget->addItem(newItem);
}

template<typename T>
void BenchController::sendItemsNameLogoListToComboBoxes(T* customer){
    auto viewItemDataVec = QVector<BenchViewItem::ViewItemData>();
    viewItemDataVec.reserve(updateItemsMap_.size());
    for(const auto& viewItem : updateItemsMap_)
        viewItemDataVec.append(BenchViewItem::ViewItemData{viewItem->getIcon(),viewItem->getName()});
    customer->receiveItemsNameList(viewItemDataVec);
}

template<typename T>
void BenchController::sendItemFromName(const QString& itemName, T* customer) {
    auto item = updateItemsMap_.value(itemName);
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
    ui->server_button->setIcon(socketAdapter_->IsConnected() ? serverConnectedIcon : serverDisconnectedIcon);
    if(!hasError_)
        ui->status_button->setIcon(socketAdapter_->IsConnected() ? statusOkIcon : statusErrorIcon);
}