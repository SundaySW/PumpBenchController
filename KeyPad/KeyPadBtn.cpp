//
// Created by user on 21.12.2023.
//

#include "KeyPadBtn.hpp"
#include "QJsonArray"
#include <utility>

KeyPadBtn::KeyPadBtn(QWidget *parent)
    :QPushButton(parent)
{
    setText(QString("NoName Button"));
}

KeyPadBtn::KeyPadBtn(QString name, QWidget *parent)
    :QPushButton(parent)
{
    setText(QString(std::move(name)));
}

void KeyPadBtn::mousePressEvent(QMouseEvent *e) {
    if(e->button()==Qt::RightButton)
        emit rightClicked();
    QAbstractButton::mousePressEvent(e);
}

void KeyPadBtn::SetMsg(QString msg) {
    protos_msg_ = std::move(msg);
}

void KeyPadBtn::SetMsgList(QStringList list){
    msg_list_ = std::move(list);
}

void KeyPadBtn::SetMsgList(const QJsonArray& array){
    msg_list_.clear();
    for(const auto& elem: array)
        msg_list_.append(elem.toString());
}

QString KeyPadBtn::GetMsg(){
    return protos_msg_;
}

QStringList KeyPadBtn::GetMsgList() {
    return msg_list_;
}


