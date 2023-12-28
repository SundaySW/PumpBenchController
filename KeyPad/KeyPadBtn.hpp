#pragma once

#include <QPushButton>
#include <QMouseEvent>

class KeyPadBtn: public QPushButton
{
    Q_OBJECT
public:
    explicit KeyPadBtn(QWidget *parent = nullptr);
    explicit KeyPadBtn(QString name, QWidget *parent = nullptr);
    void SetMsg(QString msg);
    void SetMsgList(QStringList);
    void SetMsgList(const QJsonArray& array);
    QString GetMsg();
    QStringList GetMsgList();
signals:
    void rightClicked();
private:
    QString protos_msg_;
    QStringList msg_list_;
    void mousePressEvent(QMouseEvent *e) override;
};

