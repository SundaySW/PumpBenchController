#ifndef SERVERCONNECTIONDLG_H
#define SERVERCONNECTIONDLG_H

#include <QDialog>
#include "ParamService/ParamService.h"
#include "QIcon"

namespace Ui {
class ServerConnectionDlg;
}

class ServerConnectionDlg : public QDialog
{
    Q_OBJECT
public:
    explicit ServerConnectionDlg(const QSharedPointer<SocketAdapter>& sA, QWidget *parent = nullptr);
    ~ServerConnectionDlg();
signals:
    void autoConnectStateChanged(bool);
    void eventInServerConnection(const QString&, bool);
private:
    Ui::ServerConnectionDlg *ui;
    QSharedPointer<SocketAdapter> socketAdapter;
    bool autoConnect;
    void connectBtnClicked();
    void autoConnectBtnClicked();
    void connectToServer();
};

#endif // SERVERCONNECTIONDLG_H
