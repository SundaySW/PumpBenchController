//
// Created by user on 05.05.2023.
//
#include "BenchController.h"

BenchController::BenchController(Ui::MainWindow *_ui, QMainWindow* mw):
        socketAdapter(new SocketAdapter()),
        paramService(new ParamService()),
        ui(_ui),
        mainWindow(mw),
        serverConnectionDlg(new ServerConnectionDlg(socketAdapter, mw))
{
    ui->setupUi(mw);
    jsonSaved = QJsonObject();
    paramService->setSocketAdapter(socketAdapter);
    paramService->loadParams(jsonSaved);
    setView();
    setViewMap();
    makeConnections();
}
BenchController::~BenchController(){}

void BenchController::setView(){
    logOkIcon = QIcon(":/icons/ok_circle.svg");
    logErrorIcon = QIcon(":/icons/error_circle.svg");
    statusDefIcon = QIcon(":/item_icons/item_svg/sktb_logo_def.svg");
    statusOkIcon = QIcon(":/item_icons/item_svg/sktb_logo_active.svg");
    statusErrorIcon = QIcon(":/item_icons/item_svg/sktb_logo_error.svg");
    serverConnectedIcon = QIcon(":/icons/server_connected.svg");
    serverDisconnectedIcon = QIcon(":/icons/server_disconnected.svg");
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
    controlItem->loadTargetItems(updateItemsMap.keys());
}

void BenchController::makeConnections(){
    connect(controlItem.get(), &BenchViewCtrlItem::requestItemsList, [this](){ sendItemsNameList(controlItem.get());});
    connect(controlItem.get(), &BenchViewCtrlItem::requestParamKeyByName, [this](const QString& s){ sendItemFromName(s, controlItem.get());});

    for(const auto& viewItem : updateItemsMap){
        connect(viewItem.get(), &BenchViewItem::signalValueUpdated_itemName, [this](const QString& name){
            onItemNewValue(name);
        });
    }
    connect(ui->server_button, &QPushButton::clicked, [this](){ serverBtnClicked();});
    connect(serverConnectionDlg, &ServerConnectionDlg::eventInServerConnection, [this](const QString& s, bool b){ eventServerConnectionHandler(s, b);});
    connect(ui->status_button, &QPushButton::clicked, [this](){ statusBtnClicked(); });
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
    ui->status_listWidget->clear();
    ui->status_button->setIcon(statusDefIcon);
}

void BenchController::onItemNewValue(const QString& name){
    if(updateItemsMap.contains(name)){
        auto item = updateItemsMap[name];
        bool noError = item->getCurrentStatus();
        if(!noError){
            updateErrorStatusList(item.get());
        }
        ui->status_button->setIcon(noError ? statusOkIcon : statusErrorIcon);
    }
}

void BenchController::updateErrorStatusList(BenchViewItem* item){
    auto itemStr = QString("%1 at %2\nvalue: %3").arg(item->getName(), item->getLastValueDateTimeStr(), item->getCurrentValue().toString());
    auto newItem = new QListWidgetItem(item->getIcon(), itemStr);
    ui->status_listWidget->addItem(newItem);
}

void BenchController::sendItemsNameList(BenchViewCtrlItem* customer){
    auto nameList = QStringList();
    for(const auto& viewItem : updateItemsMap)
        nameList.append(viewItem->getName());
    customer->receiveItemsNameList(nameList);
}

void BenchController::sendItemFromName(const QString &itemName, BenchViewCtrlItem *customer) {
    auto& item = updateItemsMap.find(itemName).value();
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
}