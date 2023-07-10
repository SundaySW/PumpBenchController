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
        item_name_(std::move(_name)),
        Plot(_plot),
        label(_label),
        button(_button),
        param_service_(ps),
        normal_value_upperBound_(0),
        normal_value_lower_bound_(0),
        QObject(parent),
        updateValueTimer(new QTimer(this))
{
    settings_dlg_ = new BenchItemSettingsDlg(item_name_, param_service_, true, parent);

    connect(settings_dlg_, &BenchItemSettingsDlg::itemParamChanged, [this](QSharedPointer<ParamItem>& newItem){
        if(!protos_item_.isNull())
            disconnect(protos_item_.get(), nullptr, this, nullptr);
        protos_item_ = newItem;
        connect(protos_item_.get(), &ParamItem::newParamValue, this, &BenchViewItem::updateView);
        resetPlotData();
    });
    connect(settings_dlg_, &BenchItemSettingsDlg::itemParamUnbinded, [this](){
        disconnect(protos_item_.get(), nullptr, this, nullptr);
        protos_item_.reset();
        unsetPlotData();
    });
    connect(settings_dlg_, &BenchItemSettingsDlg::newUpdateValueBounds, [this](const QPair<double, double>& newPairValues){
        normal_value_lower_bound_ = newPairValues.first;
        normal_value_upperBound_ = newPairValues.second;
    });
    connect(button, &QPushButton::clicked, [this](){ itemButtonClicked(); });
    connect(updateValueTimer, &QTimer::timeout, [this]() { updateValueTimerFinished(); });

    iconDef = button->icon();
    iconOK = QIcon(QString(":/item_icons/item_svg/%1%2.svg").arg(item_name_, "_active"));
    iconError = QIcon(QString(":/item_icons/item_svg/%1%2.svg").arg(item_name_, "_error"));
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
    label->setText(QString::number(current_value_double_, 'f', 2));
    currentStatus ? label->setStyleSheet(QString("color:%1;").arg(active_color_hex))
                   :label->setStyleSheet(QString("color:%1;").arg(alert_color_hex));
    changeIconColor(currentStatus);
    updatePlotData();
}

void BenchViewItem::updatePlotData(){
    lastKey = protos_item_->getLastValueDateTime().toSecsSinceEpoch();
    if(!graphData.isNull())
        graphData->add(QCPGraphData(lastKey, current_value_double_));
    else return;
}

void BenchViewItem::processValue() {
    currentValue_ = protos_item_->getValue();
    bool ok;
    current_value_double_ = currentValue_.toDouble(&ok);
    if(ok){
        currentStatus = current_value_double_ >= normal_value_lower_bound_ && current_value_double_ <= normal_value_upperBound_;
        updateValueTimer->start(protos_item_->getViewUpdateRate());
        signalValueUpdated_itemName(item_name_);
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
    settings_dlg_->show();
    settings_dlg_->raise();
}

QString BenchViewItem::getName(){
    return item_name_;
}

QString BenchViewItem::getParamKey(){
    return protos_item_->getTableName();
}

std::tuple<double, double> BenchViewItem::getBounds() {
    return {normal_value_lower_bound_, normal_value_upperBound_ };
}

ParamItem* BenchViewItem::getParamPtr() const{
    return protos_item_.get();
}

bool BenchViewItem::getCurrentStatus() const{
    return currentStatus;
}

const QIcon &BenchViewItem::getErrorIcon() const{
    return iconError;
}

QString BenchViewItem::getLastValueDateTimeStr() {
    return protos_item_->getLastValueDateTime().toString("yyyy.MM.dd-hh:mm:ss");
}

const QVariant &BenchViewItem::getCurrentValue() const {
    return currentValue_;
}

QPushButton *BenchViewItem::getButton() const{
    return button;
}

void BenchViewItem::loadDataFromJson(const QJsonObject& jsonObject){
    if(jsonObject.empty())
        return;
    item_name_ = jsonObject["ItemName"].toString();
    normal_value_lower_bound_ = jsonObject["NormalLowerBoundValue"].toInt();
    normal_value_upperBound_ = jsonObject["NormalUpperBoundValue"].toInt();
    currentValue_ = jsonObject["LastValue"].toDouble();
    currentStatus = jsonObject["LastStatus"].toBool();
    settings_dlg_->setUpdateValueBounds(QPair<double,double>(normal_value_lower_bound_, normal_value_upperBound_));
    auto paramName = jsonObject["paramTableName"].toString();
    if(!paramName.isEmpty())
        settings_dlg_->setUpdateParamFromController(jsonObject["paramTableName"].toString());
}

QJsonObject BenchViewItem::saveDataToJSon(){
    QJsonObject jsonObject;
    if(!protos_item_.isNull())
        jsonObject["paramTableName"] = protos_item_->getTableName();
    jsonObject["ItemName"] = item_name_;
    jsonObject["NormalLowerBoundValue"] = normal_value_lower_bound_;
    jsonObject["NormalUpperBoundValue"] = normal_value_upperBound_;
    jsonObject["LastValue"] = currentValue_.toString();
    jsonObject["LastStatus"] = currentStatus;
    return jsonObject;
}

bool BenchViewItem::isProtosParamSelected(){
    return !protos_item_.isNull();
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