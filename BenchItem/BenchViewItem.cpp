//
// Created by user on 12.05.2023.
//

#include "BenchViewItem.h"
#include <QGraphicsItem>
#include <QGraphicsColorizeEffect>
#include <utility>
#include <QDomDocument>
#include "colors.h"

extern QRandomGenerator generator;

BenchViewItem::BenchViewItem(QString _name, QCustomPlot* _plot, QLabel* _label, QPushButton* _button, ParamService* ps, QWidget *parent):
    name(std::move(_name)),
    Plot(_plot),
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
        if(!item.isNull())
            disconnect(item.get(), nullptr, this, nullptr);
        item = newItem;
        connect(item.get(), &ParamItem::newParamValue, this, &BenchViewItem::updateView);
        resetPlotData();
    });
    connect(settingsDlg, &BenchItemSettingsDlg::itemParamUnbinded, [this](){
        disconnect(item.get(), nullptr, this, nullptr);
        item.reset();
        unsetPlotData();
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
    generateColor();
    setPlot();
}

void BenchViewItem::generateColor(){
    auto randColor = QColor(generator.bounded(120,255), generator.bounded(120,255), generator.bounded(120,255));
    auto newColor = QColor();
    newColor.setRed(randColor.red());
    newColor.setGreen(randColor.green());
    newColor.setBlue(randColor.blue());
    color = newColor;
}

void BenchViewItem::repaintPlot(){
    if(livePlot && Plot->graphCount()){
        selfGraph->rescaleAxes();
        double newL = int(rect->axis(QCPAxis::atLeft)->range().lower * 0.95);
        double newH = int(rect->axis(QCPAxis::atLeft)->range().upper * 1.05);
        rect->axis(QCPAxis::atLeft)->setRange(newL,newH);
        rect->axis(QCPAxis::atBottom)->setRange(lastKey, 50, Qt::AlignHCenter);
        Plot->replot();
    }
}

void BenchViewItem::resetPlotData(){
    plotBindGraph();
    livePlot = true;
}

void BenchViewItem::unsetPlotData(){
    Plot->clearGraphs();
    livePlot = false;
}

void BenchViewItem::updateView(){
    processValue();
    label->setText(QString::number(currentValueDouble, 'f', 2));
    currentStatus ? label->setStyleSheet(QString("color:%1;").arg(active_color_hex))
                   :label->setStyleSheet(QString("color:%1;").arg(alert_color_hex));
    changeIconColor(currentStatus);
    updatePlotData();
}

void BenchViewItem::updatePlotData(){
    lastKey = item->getLastValueDateTime().toSecsSinceEpoch();
    if(!graphData.isNull())
        graphData->add(QCPGraphData(lastKey, currentValueDouble));
    else return;
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
    auto paramName = jsonObject["paramTableName"].toString();
    if(!paramName.isEmpty())
        settingsDlg->setUpdateParamFromController(jsonObject["paramTableName"].toString());
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

const QIcon &BenchViewItem::getIcon() const{
    return iconDef;
}

void BenchViewItem::setPlot(){
    Plot->setOpenGl(true);
    Plot->setNotAntialiasedElement(QCP::AntialiasedElement::aeAxes);
    Plot->plotLayout()->clear();
//    auto pathToFile = QString(CURRENT_BUILD_TYPE_) == "Debug" ? "/../" : "/";
//    Plot->setBackground(QPixmap(QCoreApplication::applicationDirPath() + QString("%1/Resources/background.png").arg(pathToFile)));
    Plot->setBackground(QBrush(QColor(main_back_color)));
    Plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    Plot->plotLayout()->setFillOrder(QCPLayoutGrid::FillOrder::foRowsFirst);
    Plot->plotLayout()->setRowSpacing(0);

    connect(Plot, &QCustomPlot::mouseDoubleClick, [this](QMouseEvent*  event) {
        livePlot = !livePlot;
//        if(QGuiApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
//            if(rect){
//                rect->axis(QCPAxis::atBottom)->rescale();
//                Plot->replot();
//            }
//        }
    });
    prepareRect();
    plotBindGraph();
}

void BenchViewItem::plotBindGraph(){
    selfGraph = Plot->addGraph(rect->axis(QCPAxis::atBottom), rect->axis(QCPAxis::atLeft));
    graphData.clear();
    graphData = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer());
    selfGraph->setData(graphData);
    QPen graphPen = QPen(color, lineWidth);
    selfGraph->setLineStyle(lineStyle);
    selfGraph->setPen(graphPen);
}

void BenchViewItem::prepareRect(){
    rect = new QCPAxisRect(Plot);
    auto* bottomAxis = rect->axis(QCPAxis::atBottom);
    auto* topAxis = rect->axis(QCPAxis::atTop);
    auto* rightAxis = rect->axis(QCPAxis::atRight);
    auto* leftAxis = rect->axis(QCPAxis::atLeft);
    rect->setRangeZoom(Qt::Vertical);
    rect->setRangeZoomAxes(rect->axes());
    connect(rightAxis, SIGNAL(rangeChanged(QCPRange)), leftAxis, SLOT(setRange(QCPRange)));
    connect(leftAxis, SIGNAL(rangeChanged(QCPRange)), rightAxis, SLOT(setRange(QCPRange)));
    rect->setAutoMargins(QCP::msLeft|QCP::msRight|QCP::msBottom);
//    rect->setupFullAxesBox(true);
//    rect->setMargins(QMargins(0, 0, 0, 0));

    timeTicker = QSharedPointer<QCPAxisTickerTime>(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    rect->axis(QCPAxis::atBottom)->setTicker(timeTicker);

    bottomAxis->setOffset(0);
    bottomAxis->setTickLabels(false);
    bottomAxis->setBasePen(Qt::NoPen);
    bottomAxis->setTickPen(Qt::NoPen);
    bottomAxis->setSubTickPen(Qt::NoPen);
    bottomAxis->setSelectableParts(QCPAxis::SelectablePart::spNone);

    leftAxis->setTickLabels(false);
    leftAxis->setTickPen(Qt::NoPen);
    leftAxis->setSubTickPen(Qt::NoPen);
    leftAxis->setBasePen(Qt::NoPen);

    for(auto *axis : rect->axes()) {
        axis->setLayer("axes");
        axis->grid()->setLayer("grid");
        axis->grid()->setSubGridVisible(false);
        axis->grid()->setZeroLinePen(Qt::NoPen);
        axis->grid()->setPen(Qt::NoPen);
        axis->grid()->setSubGridPen(Qt::NoPen);
    }
    Plot->plotLayout()->addElement(rect);
}