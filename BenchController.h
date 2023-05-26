//
// Created by user on 05.05.2023.
//

#ifndef PUMPBENCHCONTROLLER_BENCHCONTROLLER_H
#define PUMPBENCHCONTROLLER_BENCHCONTROLLER_H

#include "ParamService.h"
#include "BenchItem/BenchViewItem.h"
#include "BenchViewCtrlItem.h"
#include "Dialogs/serverconnectiondlg.h"
#include "ui_mainwindow.h"
#include <utility>

class BenchController: public QObject{
    Q_OBJECT
public:
    explicit BenchController(Ui::MainWindow *_ui, QMainWindow* mw);
    void setViewMap();
    ~BenchController();
private:
    Ui::MainWindow *ui;
    QMainWindow* mainWindow;
    QMap<QString, QSharedPointer<BenchViewItem>> updateItemsMap;
    QSharedPointer<BenchViewCtrlItem> controlItem;
    QSharedPointer<ParamService> paramService;
    QSharedPointer<SocketAdapter> socketAdapter;
    ServerConnectionDlg* serverConnectionDlg;
    QJsonObject jsonSaved;
    QIcon logOkIcon, logErrorIcon;
    QIcon statusOkIcon, statusErrorIcon, statusDefIcon;
    QIcon serverConnectedIcon, serverDisconnectedIcon;
    void makeConnections();
    void sendItemsNameList(BenchViewCtrlItem *customer);
    void sendItemFromName(const QString &itemName, BenchViewCtrlItem *customer);
    void statusBtnClicked();
    void serverBtnClicked();
    void onItemNewValue(const QString &name);
    void updateErrorStatusList(BenchViewItem *item);
    void eventServerConnectionHandler(const QString &eventStr, bool isError);
    void setView();
};

#endif //PUMPBENCHCONTROLLER_BENCHCONTROLLER_H