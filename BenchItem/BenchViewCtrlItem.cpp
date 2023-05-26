//
// Created by user on 17.05.2023.
//
#include "BenchViewCtrlItem.h"
#include <utility>

BenchViewCtrlItem::BenchViewCtrlItem(QString _name, ParamService* ps, QPushButton* cfgBtn,
                                     QLineEdit *ive, QLineEdit *tve, QPushButton *pid,
                                     QPushButton *send, QComboBox *tpc, QSlider *vs,
                                     QWidget *parent)
   :name(std::move(_name)),
    paramService(ps),
    configBtn(cfgBtn),
    itemValueEdit(ive),
    targetValueEdit(tve),
    pidEnabledBtn(pid),
    sendValueBtn(send),
    targetParamCombobox(tpc),
    valueSlider(vs),
    pidControl()
{
    settingsDlg = new BenchItemSettingsDlg(name, paramService, false, parent);
//    itemValueEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]{1,7}\\.[0-9]{1,5}")));
//    targetValueEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]{1,7}\\.[0-9]{1,5}")));

    connect(itemValueEdit, &QLineEdit::editingFinished, [this]() { textValueEdited();});
    connect(valueSlider, &QAbstractSlider::valueChanged, [this](int pos) { sliderMoved(pos);});
    connect(pidEnabledBtn, &QPushButton::clicked, [this]() { pidButtonClicked();});
    connect(sendValueBtn, &QPushButton::clicked, [this]() { sendValue();});
    connect(targetValueEdit, &QLineEdit::editingFinished, [this]() { targetValueChanged();});
    connect(targetParamCombobox, &QComboBox::currentTextChanged, [this](const QString& s){ newTargetValueItem(s);});
    connect(configBtn, &QPushButton::clicked, [this](){ configBtnClicked();});
    connect(settingsDlg, &BenchItemSettingsDlg::newSetParamId, [this](uchar id){ setParamId = id;});
    connect(settingsDlg, &BenchItemSettingsDlg::newSetParamHost, [this](uchar Host){ setParamHost = Host;});
    connect(settingsDlg, &BenchItemSettingsDlg::newPIDSettigs, [this](const BenchItemSettingsDlg::PIDSettings& settings){
        pidControl.changeKs(settings.Kp,settings.Ki,settings.Kd);
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
    auto newValue = itemValueEdit->text().toInt(&ok);
    if(!ok)
        return;
    setRequestedValue(newValue);
}

template<typename T>
void BenchViewCtrlItem::setRequestedValue(T val){
    if(checkValue(val)){
        requestedValue = val;
        valueSlider->setValue(requestedValue);
        itemValueEdit->setText(QString("%1").arg(requestedValue));
    }
}

void BenchViewCtrlItem::sendValue(){
    paramService->setParamValueChanged(setParamId, setParamHost, requestedValue);
}

template<typename T>
bool BenchViewCtrlItem::checkValue(T val){
    return (val >= minSetValueBound && val <= maxSetValueBound);
}

void BenchViewCtrlItem::pidButtonClicked(){
    pidEnabled = !pidEnabled;
}

void BenchViewCtrlItem::targetValueChanged() {
    bool ok;
    double newValue = targetValueEdit->text().toDouble(&ok);
    if(!ok || !checkValue(newValue))
        return;
    pidTargetValue = newValue;
}

void BenchViewCtrlItem::newTargetValueItem(const QString& itemName) {
    if(targetValueItem.isNull()){
        emit requestParamKeyByName(itemName);
    }else if(targetValueItem->getName() != itemName){
        disconnect(targetValueItem->getParamPtr());
        emit requestParamKeyByName(itemName);
    }
}

void BenchViewCtrlItem::receiveItem(QSharedPointer<BenchViewItem>& item){
    targetValueItem = item;
    connect(targetValueItem->getParamPtr(), &ParamItem::newParamValue, [this](){ newTargetValueItemUpdate();});
    auto[min, max] = targetValueItem->getBounds();
    pidTargetValue = targetValueItem->getCurrentValue().toDouble();
    pidControl.changeValueBounds(min, max);
}

void BenchViewCtrlItem::receiveItemsNameList(const QStringList& names){
    targetParamCombobox->clear();
    targetParamCombobox->addItems(names);
}

void BenchViewCtrlItem::newTargetValueItemUpdate(){
    if(!pidEnabled) return;
    auto processValue = targetValueItem->getParamPtr()->getValue().toDouble();
    auto dT = double(targetValueItem->getParamPtr()->getUpdateRate())/10000;
    auto newVal = (int)pidControl.calculate(pidTargetValue, processValue, dT);
    setRequestedValue(newVal);
    sendValue();
}

void BenchViewCtrlItem::loadTargetItems(const QStringList& values){
    targetParamCombobox->clear();
    targetParamCombobox->addItems(values);
}