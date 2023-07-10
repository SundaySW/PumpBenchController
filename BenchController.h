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
#include "Scenarios/scenariosdock.h"

class BenchController: public QObject{
    Q_OBJECT
public:
    explicit BenchController(Ui::MainWindow *_ui, QMainWindow* mw);
    void setViewMap();
    ~BenchController() = default;
private:
    Ui::MainWindow *ui;
    QMainWindow* mainWindow;
    QMap<QString, QSharedPointer<BenchViewItem>> updateItemsMap_;
    QSharedPointer<BenchViewCtrlItem> controlItem_;
    QSharedPointer<ParamService> paramService_;
    QSharedPointer<SocketAdapter> socketAdapter_;
    ServerConnectionDlg* serverConnectionDlg_;
    QJsonObject jsonSaved_;
    bool hasError_;
    QIcon logOkIcon, logErrorIcon;
    QIcon statusOkIcon, statusErrorIcon, statusDefIcon;
    QIcon serverConnectedIcon, serverDisconnectedIcon;
    QTimer* serverReconnectionTimer_;
    QTimer* plotReDrawTimer_;
    QSharedPointer<ScenariosDock> scenariosDock_;

    void makeConnections();
    void statusBtnClicked();
    void serverBtnClicked();
    void onItemNewValue(const QString &name);
    void updateErrorStatusList(BenchViewItem *item);
    void eventServerConnectionHandler(const QString &eventStr, bool isError);
    void setView();
    void loadFromJson();
    void saveToJson();
    void serverConnectionHandler();
    void plotReDrawTimerHandler();
    void loadScenarioDock();
    template<typename T>
    void sendItemFromName(const QString &itemName, T *customer);
    template<typename T>
    void sendItemsNameLogoListToComboBoxes(T *customer);
};

#endif //PUMPBENCHCONTROLLER_BENCHCONTROLLER_H