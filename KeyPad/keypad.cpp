#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QJsonObject>
#include "QJsonArray"
#include "keypad.h"
#include "ui_keypad.h"
#include "QVector"

#include "QMouseEvent"
#include "QMenu"
#include "QDialog"

KeyPad::KeyPad(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::KeyPad)
{
    ui->setupUi(this);
    connect(ui->add_btn, &QPushButton::clicked, [this](){ AddNewBtn();});
}

void KeyPad::AddNewBtn(){
    auto shrdPtr = QSharedPointer<KeyPadBtn>(new KeyPadBtn());
    auto* button_raw_ptr = shrdPtr.get();
    btnMap.insert(button_raw_ptr, shrdPtr);
    connect(button_raw_ptr, &KeyPadBtn::rightClicked, this,
            [this, button_raw_ptr](){ RightClicked(button_raw_ptr);});
    connect(button_raw_ptr, &KeyPadBtn::clicked, this,[this, button_raw_ptr](){
        auto msg_list = button_raw_ptr->GetMsgList();
        for(const auto& msg: msg_list)
            emit protosMsgToSend(msg);
    });
    AddBtnToGrid(button_raw_ptr);
}

void KeyPad::LoadBtn(QJsonObject& jsonObject){
    auto shrdPtr = QSharedPointer<KeyPadBtn>(new KeyPadBtn(jsonObject["name"].toString()));
    auto* button_raw_ptr = shrdPtr.get();
    btnMap.insert(button_raw_ptr, shrdPtr);

    button_raw_ptr->SetMsgList(jsonObject["protos_msg"].toArray());

    connect(button_raw_ptr, &KeyPadBtn::rightClicked, this,
            [this, button_raw_ptr](){ RightClicked(button_raw_ptr);});
    connect(button_raw_ptr, &KeyPadBtn::clicked, this, [this, button_raw_ptr](){
        auto msg_list = button_raw_ptr->GetMsgList();
        for(const auto& msg: msg_list)
            emit protosMsgToSend(msg);
    });

    AddBtnToGrid(button_raw_ptr);
}

KeyPad::~KeyPad()
{
    delete ui;
}

void KeyPad::RightClicked(KeyPadBtn* btn) {
    auto* menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    auto* configure = new QAction(QString("Configure"), this);
    auto* delete_btn = new QAction(QString("Delete"), this);
    menu->addAction(configure);
    menu->addAction(delete_btn);

    connect(configure, &QAction::triggered, [this, btn](){
        auto dlg = new QDialog(this);
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->setWindowTitle("Config Button");

        auto *main_layout = new QFormLayout();

        auto* btn_layout = new QHBoxLayout();
        btn_layout->setSpacing(10);
        auto add_msg_btn = new QPushButton("Add Msg");
        auto save_btn = new QPushButton("Save");
        auto send_all_btn = new QPushButton("Send All");
        btn_layout->addWidget(add_msg_btn);
        btn_layout->addWidget(send_all_btn);
        btn_layout->addWidget(save_btn);
        main_layout->addItem(btn_layout);

        auto btn_name = new QLineEdit(btn->text());
        main_layout->addRow(tr("Button Name: "), btn_name);

        auto msg_list = btn->GetMsgList();
        auto msg_edit_list = QVector<QLineEdit*>();
        int count = 0;
        for(const auto& msg: msg_list){
            auto new_msg_edit = new QLineEdit(msg);
            msg_edit_list.push_back(new_msg_edit);
            main_layout->addRow(tr("%1 protos message to send: ").arg(++count), new_msg_edit);
        }
        connect(add_msg_btn, &QPushButton::clicked, [&](){
            auto new_msg_edit = new QLineEdit();
            msg_edit_list.push_back(new_msg_edit);
            main_layout->addRow(tr("%1 protos message to send: ").arg(++count), new_msg_edit);
        });
        connect(send_all_btn, &QPushButton::clicked, [&](){
            for(const auto& edit: msg_edit_list){
                emit protosMsgToSend(edit->text());
            }
        });
        connect(save_btn, &QPushButton::clicked, [&](){
            btn->setText(btn_name->text());
            msg_list.clear();
            msg_list.reserve(msg_edit_list.count());
            for(const auto& edit: msg_edit_list){
                if(!edit->text().isEmpty())
                    msg_list.push_back(edit->text());
            }
            btn->SetMsgList(msg_list);
            dlg->close();
        });
        dlg->setLayout(main_layout);
        dlg->exec();
    });

    connect(delete_btn, &QAction::triggered, [this, btn](){
        disconnect(btn, nullptr, this, nullptr);
        btnMap.remove(btn);
        ReMapGrid();
    });
    menu->popup(QCursor::pos());
}

void KeyPad::AddBtnToGrid(KeyPadBtn *btn) {
    ui->gridLayout->removeWidget(ui->add_btn);
    ui->gridLayout->addWidget(btn, row_, column_);
    if(column_ < k_column_n_)
        column_++;
    else{
        column_ = 0;
        row_++;
    }
    ui->gridLayout->addWidget(ui->add_btn, row_, column_);
}

void KeyPad::ReMapGrid(){
    ui->gridLayout->removeWidget(ui->add_btn);
    for(auto& btn: btnMap){
        ui->gridLayout->removeWidget(btn.get());
    }
    row_ = 0;
    column_ = 0;
    for(auto& btn: btnMap){
        ui->gridLayout->addWidget(btn.get(), row_, column_);
        if(column_ < k_column_n_)
            column_++;
        else{
            column_ = 0;
            row_++;
        }
    }
    ui->gridLayout->addWidget(ui->add_btn, row_, column_);
}

void KeyPad::LoadDataFromJson(const QJsonObject& json) {
    if(json.empty())
        return;
    auto buttons = json["KeyPad"].toArray();
    for(const auto& btn : buttons)
    {
        auto loadItemJson = btn.toObject();
        if(!loadItemJson.isEmpty())
            LoadBtn(loadItemJson);
    }
}

QJsonObject KeyPad::SaveDataToJson() {
    auto retVal = QJsonObject();
    QJsonArray keyArr;
    for(auto& btn: btnMap){
        QJsonObject btn_json;
        btn_json["name"] = btn->text();
        auto msg_list = btn->GetMsgList();
        auto j_arr = QJsonArray::fromStringList(msg_list);
        btn_json["protos_msg"] = j_arr;
        keyArr.append(btn_json);
    }
    retVal["KeyPad"] = keyArr;
    return retVal;
}


