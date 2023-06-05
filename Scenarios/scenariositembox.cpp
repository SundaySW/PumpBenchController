#include "scenariositembox.h"
#include "ui_scenariositembox.h"

ScenariosItemBox::ScenariosItemBox(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::ScenariosItemBox)
{
    ui->setupUi(this);
    msgMatch = QRegExp(R"([0-9a-fA-F]{8}\.[0-9]{1}\.[0-9a-fA-F]{16})");
//    ui->targetValue_lineEdit->setValidator(new QRegExpValidator(QRegExp(R"([0-9a-fA-F]{1,8}\.[0-9a-fA-F]{1,8})")));
    ui->msg_lineEdit->setValidator(new QRegExpValidator(msgMatch));
    connect(ui->remove_pushButton, &QPushButton::clicked, [this](){ emit deleteMe();});
    connect(ui->enable_pushButton, &QPushButton::clicked, [this](){ enableBtnClicked();});
    connect(ui->viewItem_comboBox, &QComboBox::currentTextChanged, [this](const QString& s){ newTargetValueItem(s); });
    connect(ui->targetValue_lineEdit, &QLineEdit::editingFinished, [this](){ targetValueEdited();});
    connect(ui->msg_lineEdit, &QLineEdit::editingFinished, [this](){ msgEdited();});
}

void ScenariosItemBox::msgEdited(){
    auto newMsg = ui->msg_lineEdit->text();
    if(msgMatch.indexIn(newMsg) != -1)
        msgToSend = newMsg;
    else
        showMsgBox("Incorrect msg format!");
}

void ScenariosItemBox::targetValueEdited(){
    bool ok;
    double newValue = ui->targetValue_lineEdit->text().toDouble(&ok);
    if(!ok){
        ui->targetValue_lineEdit->clear();
        if(targetValue.has_value())
            ui->targetValue_lineEdit->setText(QString::number(targetValue.value()));
        showMsgBox("Incorrect value - should be double (x.x)");
        return;
    }
    targetValue = newValue;
}

void ScenariosItemBox::newTargetValueItem(const QString& itemName){
    emit requestItemByName(itemName);
}

void ScenariosItemBox::receiveItem(QSharedPointer<BenchViewItem>& item){
    if(!isOKReceivedNewParam(item))
        return;
    if(targetValueItem)
        disconnect(targetValueItem->getParamPtr());
    targetValueItem = item;
    connect(targetValueItem->getParamPtr(), &ParamItem::newParamValue, [this](){ newTargetValueItemUpdate();});
}

bool ScenariosItemBox::isOKReceivedNewParam(const QSharedPointer<BenchViewItem>& item){
    if(!item->isProtosParamSelected()){
        if(targetValueItem.isNull())
            showMsgBox(QString("No Protos Param selected in %1 item").arg(item->getName()));
        else{
            showMsgBox(QString("No Protos Param selected in %1 item\nStill using %2").arg(item->getName(), targetValueItem->getName()));
            ui->viewItem_comboBox->setCurrentText(targetValueItem->getName());
        }
        return false;
    }else if(item == targetValueItem){
        ui->viewItem_comboBox->setCurrentText(targetValueItem->getName());
        return false;
    }else
        ui->viewItem_comboBox->setCurrentText(item->getName());
    return true;
}

void ScenariosItemBox::showMsgBox(const QString& msg){
    QMessageBox msgBox(this);
    msgBox.setText(msg);
    msgBox.exec();
}

void ScenariosItemBox::newTargetValueItemUpdate(){
    if(!enabled || !targetValue.has_value() || !msgToSend.has_value())
        return;
    auto processValue = targetValueItem->getParamPtr()->getValue().toDouble();
    switch (ui->comparison_comboBox->currentIndex()) {
        case 0: //greater
            if(processValue > targetValue.value())
                protosMsgToSend(msgToSend.value());
            break;
        case 1: //smaller
            if(processValue < targetValue.value())
                protosMsgToSend(msgToSend.value());
            break;
        case 2: //equal
            if(processValue == targetValue.value())
                protosMsgToSend(msgToSend.value());
            break;
    }
}

ScenariosItemBox::~ScenariosItemBox()
{
    delete ui;
}

void ScenariosItemBox::enableBtnClicked()
{
    if(!targetValue.has_value())
        showMsgBox("Set target value for this scenario!");
    else
        enabled = !enabled;
    ui->enable_pushButton->setChecked(enabled);
}

void ScenariosItemBox::receiveItemsNameList(const QVector<BenchViewItem::ViewItemData> &data) {
    ui->viewItem_comboBox->clear();
    for(const auto& item: data)
        ui->viewItem_comboBox->addItem(item.icon, item.name);
}