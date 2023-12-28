//
// Created by user on 05.05.2023.
//

#ifndef PUMPBENCHCONTROLLER_BENCHCONTROLLER_H
#define PUMPBENCHCONTROLLER_BENCHCONTROLLER_H

#include "ParamService.h"
#include "BenchItem/bench_view_item.h"
#include "bench_view_ctrl_item.h"
#include "Dialogs/serverconnectiondlg.h"
#include "ui_mainwindow.h"
#include <utility>
#include "Scenarios/scenariosdock.h"
#include "experiment_settings.h"
#include "KeyPad/keypad.h"

class BenchController: public QObject{
    Q_OBJECT
public:
    explicit BenchController(Ui::MainWindow *_ui, QMainWindow* mw);
    void makeItems();
    ~BenchController() = default;
private:
    Ui::MainWindow *ui;
    QMainWindow* mainWindow;
    QMap<QString, QSharedPointer<BenchViewItem>> updateItemsMap_;
    QSharedPointer<BenchViewCtrlItem> controlItem_;
    QSharedPointer<ParamService> paramService_;
    QSharedPointer<SocketAdapter> socketAdapter_;
    ServerConnectionDlg* serverConnectionDlg_;
    ExperimentSettings* experimentSettingsDlg_;
    QJsonObject jsonSaved_;
    bool hasError_;
    QIcon logOkIcon, logErrorIcon;
    QIcon statusOkIcon, statusErrorIcon, statusDefIcon;
    QIcon serverConnectedIcon, serverDisconnectedIcon;
    QTimer* serverReconnectionTimer_;
    QTimer* plotReDrawTimer_;
    QSharedPointer<ScenariosDock> scenariosDock_;
    QSharedPointer<KeyPad> key_pad_dock_;

    void makeConnections();
    void statusBtnClicked();
    void serverBtnClicked();
    void experimentBtnClicked();
    void onItemNewValue(const QString &name);
    void updateErrorStatusList(BenchViewItem *item);
    void eventServerConnectionHandler(const QString &eventStr, bool isError);
    void setViewIcons();
    void loadFromJson();
    void saveToJson();
    void serverConnectionHandler();
    void plotReDrawTimerHandler();
    void makeScenarioDock();
    void makeExperimentDLg();
    template<typename T>
    void sendItemFromName(const QString &itemName, T *customer);
    template<typename T>
    void sendItemsNameLogoListToComboBoxes(T *customer);

    void makeKeyPadDock();
};

#endif //PUMPBENCHCONTROLLER_BENCHCONTROLLER_H