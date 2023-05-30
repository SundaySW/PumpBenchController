#include <QRegExpValidator>
#include <utility>
#include "serverconnectiondlg.h"
#include "ui_serverconnectiondlg.h"

ServerConnectionDlg::~ServerConnectionDlg()
{
    delete ui;
}

ServerConnectionDlg::ServerConnectionDlg(const QSharedPointer<SocketAdapter>& sA, QWidget *parent):
        QDialog(parent),
        ui(new Ui::ServerConnectionDlg),
        socketAdapter(sA),
        autoConnect(false)
{
    ui->setupUi(this);
    ui->ip_lineEdit->setValidator(new QRegExpValidator(QRegExp(R"([0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})")));
    ui->port_lineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]{4}")));
    connect(ui->connect_pushButton, &QPushButton::clicked, [this](){ connectBtnClicked();});
    connect(ui->autoConnect_pushButton, &QPushButton::clicked, [this](){ autoConnectBtnClicked();});
    ui->connect_pushButton->setCheckable(true);
 }

void ServerConnectionDlg::connectBtnClicked(){
    if(socketAdapter->IsConnected()){
        socketAdapter->Disconnect();
        emit eventInServerConnection(
                QString("Event is server connection:\n"
                        "disconnected from server"), false);
    }
    else
        connectToServer();
}

void ServerConnectionDlg::connectToServer(){
    QString ip, port;
    ip = ui->ip_lineEdit->text();
    port = ui->port_lineEdit->text();
    bool isConnected = socketAdapter->Connect(ip, port.toInt(), 30);
    if(isConnected){
        emit eventInServerConnection(
                QString("Event is server connection:\n"
                        "Connected to sever ip:%1 pot:%2").arg(ip, port), false);
    }
    else{
        emit eventInServerConnection(
                QString("Event is server connection:\n"
                        "Cant connect to sever ip:%1 pot:%2").arg(ip, port), true);
    }
    ui->connect_pushButton->setChecked(isConnected);
}

void ServerConnectionDlg::autoConnectBtnClicked(){
    autoConnect = !autoConnect;
    if(autoConnect)
        ui->autoConnect_pushButton->setChecked(autoConnect);
    emit autoConnectStateChanged(autoConnect);
}

QJsonObject ServerConnectionDlg::saveDataToJson(){
    QJsonObject retVal;
    retVal["IP"] = ui->ip_lineEdit->text();
    retVal["Port"] = ui->port_lineEdit->text();
    retVal["autoConnect"] = autoConnect;
    return retVal;
}

void ServerConnectionDlg::loadDataFromJson(const QJsonObject& jsonObject){
    if(jsonObject.empty())
        return;
    ui->ip_lineEdit->setText(jsonObject["IP"].toString());
    ui->port_lineEdit->setText(jsonObject["Port"].toString());
    ui->autoConnect_pushButton->setChecked(jsonObject["autoConnect"].toBool());
    autoConnect = jsonObject["autoConnect"].toBool();
}

bool ServerConnectionDlg::reconnectIsOn() const {
    return autoConnect;
}
