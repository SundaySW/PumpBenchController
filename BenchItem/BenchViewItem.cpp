//
// Created by user on 12.05.2023.
//

#include "BenchViewItem.h"
#include <QGraphicsItem>
#include <QGraphicsColorizeEffect>
#include <utility>
#include <QDomDocument>

BenchViewItem::BenchViewItem(QString _name, QWidget* _plot, QLabel* _label, QPushButton* _button, ParamService* ps, QWidget *parent):
    name(std::move(_name)),
    plot(_plot),
    label(_label),
    button(_button),
    paramService(ps),
    normalValueUpperBound(0),
    normalValueLowerBound(0),
    QObject(parent),
    updateValueTimer(new QTimer(this))
{
    settingsDlg = new BenchItemSettingsDlg(name, paramService, true, parent);

    connect(settingsDlg, &BenchItemSettingsDlg::itemParamChanged, [this](QSharedPointer<ParamItem>& newItem){
        item = newItem;
        connect(item.get(), &ParamItem::newParamValue, [this](){ updateView(); });
    });
    connect(settingsDlg, &BenchItemSettingsDlg::newUpdateValueBounds, [this](const QPair<double, double>& newPairValues){
        normalValueLowerBound = newPairValues.first;
        normalValueUpperBound = newPairValues.second;
    });
    connect(button, &QPushButton::clicked, [this](){ itemButtonClicked(); });
    connect(updateValueTimer, &QTimer::timeout, [this]() { updateValueTimerFinished(); });

    iconDef = button->icon();
    iconOK = QIcon(QString(":/item_icons/item_svg/%1%2.svg").arg(name, "_active"));
    iconError = QIcon(QString(":/item_icons/item_svg/%1%2.svg").arg(name, "_error"));
}

void BenchViewItem::updateView(){
    processValue();
    label->setText(currentValue.toString());
    currentStatus ? label->setStyleSheet("color:#B0CB1F;") : label->setStyleSheet("color:#EA556F;");
    changeIconColor(currentStatus);
    //todo plot add point + rePlot
}

void BenchViewItem::processValue() {
    currentValue = item->getValue();
    bool ok;
    currentValueDouble = currentValue.toDouble(&ok);
    if(ok){
        currentStatus = currentValueDouble >= normalValueLowerBound && currentValueDouble <= normalValueUpperBound;
        updateValueTimer->start(item->getViewUpdateRate());
        signalValueUpdated_itemName(name);
    }
}

void BenchViewItem::changeIconColor(bool isOk){
    button->setIcon(isOk ? iconOK: iconError);
}

void BenchViewItem::updateValueTimerFinished(){
    updateValueTimer->stop();
    label->setStyleSheet("color:grey;");
    button->setIcon(iconDef);
}

void BenchViewItem::itemButtonClicked(){
    settingsDlg->show();
    settingsDlg->raise();
}

QString BenchViewItem::getName(){
    return name;
}

QString BenchViewItem::getParamKey(){
    return item->getTableName();
}

std::tuple<double, double> BenchViewItem::getBounds() {
    return { normalValueLowerBound, normalValueUpperBound };
}

ParamItem* BenchViewItem::getParamPtr() const{
    return item.get();
}

bool BenchViewItem::getCurrentStatus() const{
    return currentStatus;
}

const QIcon &BenchViewItem::getErrorIcon() const{
    return iconError;
}

QString BenchViewItem::getLastValueDateTimeStr() {
    return item->getLastValueDateTime().toString("yyyy.MM.dd-hh:mm:ss");
}

const QVariant &BenchViewItem::getCurrentValue() const {
    return currentValue;
}

QPushButton *BenchViewItem::getButton() const{
    return button;
}

void BenchViewItem::loadDataFromJson(const QJsonObject& jsonObject){
    if(jsonObject.empty())
        return;
    name = jsonObject["ItemName"].toString();
    normalValueLowerBound = jsonObject["NormalLowerBoundValue"].toInt();
    normalValueUpperBound = jsonObject["NormalUpperBoundValue"].toInt();
    currentValue = jsonObject["LastValue"].toDouble();
    currentStatus = jsonObject["LastStatus"].toBool();
    settingsDlg->setUpdateValueBounds(QPair<double,double>(normalValueLowerBound, normalValueUpperBound));
    auto isNull = jsonObject["paramTableName"].isNull();
    if(!isNull)
        settingsDlg->setUpdateParam(jsonObject["paramTableName"].toString());
}

QJsonObject BenchViewItem::saveDataToJSon(){
    QJsonObject jsonObject;
    if(!item.isNull())
        jsonObject["paramTableName"] = item->getTableName();
    jsonObject["ItemName"] = name;
    jsonObject["NormalLowerBoundValue"] = normalValueLowerBound;
    jsonObject["NormalUpperBoundValue"] = normalValueUpperBound;
    jsonObject["LastValue"] = currentValue.toString();
    jsonObject["LastStatus"] = currentStatus;
    return jsonObject;
}

bool BenchViewItem::isProtosParamSelected(){
    return !item.isNull();
}
