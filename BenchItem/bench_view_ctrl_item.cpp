//
// Created by user on 17.05.2023.
//
#include "bench_view_ctrl_item.h"
#include <utility>
#include <QMessageBox>

BenchViewCtrlItem::BenchViewCtrlItem(QString _name, ParamService* ps, QPushButton* cfgBtn,
                                     QLineEdit *setV, QLineEdit *pidV, QPushButton *pid,
                                     QPushButton *send, QComboBox *tpc, QSlider *vs,
                                     QWidget *parent)
   : name(std::move(_name)),
     paramService(ps),
     configBtn(cfgBtn),
     set_item_value_edit_(setV),
     targetValueEdit(pidV),
     pidEnabledBtn(pid),
     sendValueBtn(send),
     targetParamCombobox(tpc),
     valueSlider(vs),
     pidControl()
{
    settingsDlg = new BenchItemSettingsDlg(name, paramService, false, parent);
    set_item_value_edit_->setValidator(new QRegExpValidator(QRegExp("[0-9a-FA-F]+")));
    targetValueEdit->setValidator(new QRegExpValidator(QRegExp("[-+]?[0-9]+(\\.[0-9]+)?")));

    connect(set_item_value_edit_, &QLineEdit::editingFinished, [this]() { textValueEdited();});
    connect(valueSlider, &QAbstractSlider::valueChanged, [this](int pos) { sliderMoved(pos);});
    connect(pidEnabledBtn, &QPushButton::clicked, [this]() { pidButtonClicked();});
    connect(sendValueBtn, &QPushButton::clicked, [this]() { sendValue();});
    connect(targetValueEdit, &QLineEdit::editingFinished, [this]() { targetValueChanged();});
    connect(targetParamCombobox, &QComboBox::currentTextChanged, [this](const QString& s){ newTargetValueItem(s);});
    connect(configBtn, &QPushButton::clicked, [this](){ configBtnClicked();});
    connect(settingsDlg, &BenchItemSettingsDlg::newSetParamId, [this](uchar id){ setParamId = id;});
    connect(settingsDlg, &BenchItemSettingsDlg::newSetParamHost, [this](uchar Host){ setParamHost = Host;});
    connect(settingsDlg, &BenchItemSettingsDlg::newPIDSettings, [this](const BenchItemSettingsDlg::PIDSettings& settings){
        pidControl.changeKs(settings.Kp,settings.Ki,settings.Kd);
        pidControl.changeValueBounds(settings.min, settings.max);
    });
    connect(settingsDlg, &BenchItemSettingsDlg::newSetValueBounds, [this](const QPair<int, int>& newPairValues){
        minSetValueBound = newPairValues.first;
        maxSetValueBound = newPairValues.second;
        valueSlider->setValue((int) minSetValueBound);
        valueSlider->setMaximum((int) maxSetValueBound);
        valueSlider->setMinimum((int) minSetValueBound);
    });
}

void BenchViewCtrlItem::configBtnClicked(){
    settingsDlg->show();
    settingsDlg->raise();
}

void BenchViewCtrlItem::sliderMoved(int position){
    setRequestedValue(position);
}

void BenchViewCtrlItem::textValueEdited(){
    bool ok;
    auto newValue = set_item_value_edit_->text().toDouble(&ok);
    if(!ok){
        set_item_value_edit_->clear();
        return;
    }
    setRequestedValue(newValue);
}

template<typename T>
bool BenchViewCtrlItem::setRequestedValue(T val){
    if(checkValue(val)){
        requestedValue = val;
        valueSlider->setValue((int)requestedValue);
        set_item_value_edit_->setText(QString("%1").arg(requestedValue));
        pidControl.reset();
        return true;
    }
    else
        set_item_value_edit_->setText("out of set range");
    return false;
}

void BenchViewCtrlItem::sendValue(){
    if(setParamId.has_value() && setParamHost.has_value()){
        paramService->setParamValueChanged(setParamId.value(), setParamHost.value(), requestedValue);
    }else if(!setParamId.has_value() || !setParamHost.has_value()){
        showMsgBox("Set Param not set!");
    }
}

template<typename T>
bool BenchViewCtrlItem::checkValue(T val){
    return (val >= minSetValueBound && val <= maxSetValueBound);
}

void BenchViewCtrlItem::pidButtonClicked(){
    managePIDStatus(!pidEnabled);
}

void BenchViewCtrlItem::managePIDStatus(bool state){
    if(targetValueItem.isNull()){
        showMsgBox("Target value_ not set!");
        pidEnabledBtn->setChecked(false);
        return;
    }
    if(!setParamId.has_value() || !setParamHost.has_value()){
        showMsgBox("Set Param Addr not set!");
        pidEnabledBtn->setChecked(false);
        return;
    }
    if(state)
        pidControl.reset();
    pidEnabledBtn->setChecked(!state);
    pidEnabled = state;
}

void BenchViewCtrlItem::targetValueChanged() {
    bool ok;
    double newValue = targetValueEdit->text().toDouble(&ok);
    if(!ok){
        targetValueEdit->clear();
        showMsgBox("Incorrect target value!\n"
                   "Should be float xxx.xxx");
        return;
    }
    pidTargetValue = newValue;
    if(checkPIDTargetValue())
        pidControl.reset();
}

bool BenchViewCtrlItem::SetTargetValue(double newValue){
    pidTargetValue = newValue;
    if(checkPIDTargetValue()){
        pidControl.reset();
        return true;
    }
    return false;
}


void BenchViewCtrlItem::newTargetValueItem(const QString& itemName) {
    if(targetValueItem.isNull()){
        emit requestParamKeyByName(itemName);
    }else if(targetValueItem->getName() != itemName){
        disconnect(targetValueItem->getParamPtr(), &ParamItem::newParamValue, this, nullptr);
        emit requestParamKeyByName(itemName);
    }
}

void BenchViewCtrlItem::receiveItem(QSharedPointer<BenchViewItem>& item){
    if(!isOKReceivedNewParam(item))
        return;
    targetValueItem = item;
    connect(targetValueItem->getParamPtr(), &ParamItem::newParamValue, [this](){ newTargetValueItemUpdate();});
//    auto bounds = targetValueItem->getBounds();
//    auto[min, max] = bounds;
//    settingsDlg->setPidBoundsOfNewItem(bounds);
//    pidControl.changeValueBounds(min, max);
    if(checkPIDTargetValue())
        pidControl.reset();
}

bool BenchViewCtrlItem::isOKReceivedNewParam(const QSharedPointer<BenchViewItem>& item){
    if(!item->isProtosParamSelected()){
        if(targetValueItem.isNull())
            showMsgBox(QString("No Protos Param selected in %1 protos_item_").arg(item->getName()));
        else{
            showMsgBox(QString("No Protos Param selected in %1 protos_item_\nStill using %2").arg(item->getName(), targetValueItem->getName()));
            resetFeedBackItemComboBox(targetValueItem->getName());
        }
        return false;
    }else if(item == targetValueItem){
        resetFeedBackItemComboBox(targetValueItem->getName());
        return false;
    }else
        resetFeedBackItemComboBox(item->getName());
    return true;
}

void BenchViewCtrlItem::resetFeedBackItemComboBox(const QString& newName){
    targetParamCombobox->setCurrentText(newName);
    emit newFeedBackItem(newName);
}

bool BenchViewCtrlItem::checkPIDTargetValue(){
    if(targetValueItem.isNull()){
        showMsgBox(QString("No Item selected."));
        return false;
    }
    auto[min, max] = targetValueItem->getBounds();
    auto currentItemValue = targetValueItem->getCurrentValue().toDouble();
    if(pidTargetValue > max || pidTargetValue < min){
        showMsgBox(QString("Target value out of range for this protos item (bounds in settings diag)\n"
                           "value is set with currently received value! Please check!"));
        targetValueEdit->setText(QString("%1").arg(currentItemValue));
        pidTargetValue = currentItemValue;
        return false;
    }
    return true;
}

void BenchViewCtrlItem::showMsgBox(const QString& msg){
    QMessageBox msgBox(targetValueEdit);
    msgBox.setText(msg);
    msgBox.exec();
}

void BenchViewCtrlItem::receiveItemsNameList(const QVector<BenchViewItem::ViewItemData> &data){
    targetParamCombobox->clear();
    for(const auto& item: data){
        targetParamCombobox->addItem(item.icon, item.name);
    }
}

void BenchViewCtrlItem::newTargetValueItemUpdate(){
    if(!pidEnabled || !pidTargetValue.has_value())
        return;
    auto processValue = targetValueItem->getParamPtr()->getValue().toDouble();
    auto dT = double(targetValueItem->getParamPtr()->getUpdateRate()) / 10000;
    auto newVal = pidControl.calculate(pidTargetValue.value(), processValue, dT);
    if(setRequestedValue(newVal))
        sendValue();
}

void BenchViewCtrlItem::loadDataFromJson(const QJsonObject& jsonObject) {
    if (jsonObject.empty())
        return;
    name = jsonObject["ItemName"].toString();
    setParamId = (uchar) jsonObject["setParamId"].toInt();
    setParamHost = (uchar) jsonObject["setParamHost"].toInt();
    requestedValue = jsonObject["requestedValue"].toDouble();
    minSetValueBound = jsonObject["minSetValueBound"].toDouble();
    maxSetValueBound = jsonObject["maxSetValueBound"].toDouble();
    pidTargetValue = jsonObject["pidTargetValue"].toDouble();
    pidEnabled = jsonObject["pidEnabled"].toBool();
    pidControl.fromJson(jsonObject["pidSettings"].toObject());
    auto isNull = jsonObject["targetValueItemName"].isNull();
    if (!isNull) {
        newTargetValueItem(jsonObject["targetValueItemName"].toString());
    }

    settingsDlg->setPIDSettings(jsonObject["pidSettings"].toObject());
    settingsDlg->setSetValueBounds(QPair<double, double>(minSetValueBound, maxSetValueBound));
    settingsDlg->setSetParamAddr(QPair<uchar, uchar>(setParamId.value(), setParamHost.value()));

    setRequestedValue(requestedValue);
    targetValueEdit->setText(QString("%1").arg(jsonObject["pidTargetValue"].toDouble()));
    pidEnabledBtn->setChecked(pidEnabled);
}

QJsonObject BenchViewCtrlItem::SaveDataToJson(){
    QJsonObject jsonObject;
    jsonObject["ItemName"] = name;
    if(!targetValueItem.isNull())
        jsonObject["targetValueItemName"] = targetValueItem->getName();
    if(setParamId.has_value())
        jsonObject["setParamId"] = setParamId.value();
    if(setParamHost.has_value())
        jsonObject["setParamHost"] = setParamHost.value();
    if(pidTargetValue.has_value()){
        jsonObject["pidTargetValue"] = pidTargetValue.value();
    }
    jsonObject["requestedValue"] = requestedValue;
    jsonObject["minSetValueBound"] = minSetValueBound;
    jsonObject["maxSetValueBound"] = maxSetValueBound;
    jsonObject["pidEnabled"] = pidEnabled;
    jsonObject["pidSettings"] = pidControl.toJson();
    return jsonObject;
}

ParamItem *BenchViewCtrlItem::getFeedBackParamRawPtr() {
    return targetValueItem->getParamPtr();
}

void BenchViewCtrlItem::SetPIDControl(bool state) {
    managePIDStatus(state);
}