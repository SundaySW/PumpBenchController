//
// Created by user on 12.05.2023.
//

#include "BenchViewItem.h"
#include <QGraphicsItem>
#include <QGraphicsColorizeEffect>
#include <utility>
#include <QDomDocument>
#include "colors.h"

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
        if(!item.isNull())
            disconnect(item.get());
        item = newItem;
        connect(item.get(), &ParamItem::newParamValue, [this](){ updateView();});
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
    currentStatus ? label->setStyleSheet(QString("color:%1;").arg(active_color_hex))
                   :label->setStyleSheet(QString("color:%1;").arg(alert_color_hex));
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
        if(QGuiApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
            auto* rect = Plot->axisRectAt(event->pos());
            if(rect){
                rect->axis(QCPAxis::atBottom)->rescale();
                Plot->replot();
            }
        }
    });
}

void BenchViewItem::bindGraph(){
    selfGraph = Plot->addGraph(rect->axis(QCPAxis::atBottom), rect->axis(QCPAxis::atLeft));
    selfGraph->data()->set(graphData);

    QPen graphPen = QPen(color, lineWidth);
    QPen axisPen = QPen(color);
    selfGraph->setLineStyle(lineStyle);
    if(selfGraph->lineStyle() == QCPGraph::lsNone)
        selfGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 7));
    selfGraph->setPen(graphPen);
    axis->setBasePen(axisPen);
    axis->setSubTickPen(axisPen);
    axis->setTickPen(axisPen);
    auto itemName = getName();
    if(name.length()>12){//todo improve!
        itemName.clear();
        for(const auto& s : name.split(" "))
            itemName += s +'\n';
    }
    axis->setLabel(itemName.trimmed());
    axis->setLabelColor(itemColor);
    axis->setTickLabelColor(itemColor);

    selfGraph->rescaleValueAxis();
    selfGraph->rescaleKeyAxis();
    selfGraph->setSelectable(QCP::SelectionType::stDataRange);
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
    connect(Plot->xAxis, SIGNAL(rangeChanged(QCPRange)), bottomAxis, SLOT(setRange(QCPRange)));
    connect(bottomAxis, SIGNAL(rangeChanged(QCPRange)), Plot->xAxis, SLOT(setRange(QCPRange)));
    rect->setupFullAxesBox(true);
    rect->setMargins(QMargins(0, 0, 0, 0));
    rect->setMarginGroup(QCP::msLeft|QCP::msRight, marginGroup);
    rect->setAutoMargins(QCP::msLeft|QCP::msRight|QCP::msBottom);

    topAxis->setOffset(0);
    topAxis->setTicks(false);
    topAxis->setTickLabels(false);
    topAxis->setSubTicks(false);
    topAxis->setBasePen(Qt::NoPen);
    topAxis->setSelectableParts(QCPAxis::SelectablePart::spNone);

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
    rightAxis->setTickLabels(true);
    rightAxis->setTicks(true);
    rightAxis->setSubTicks(true);
    rightAxis->setTickLabels(false);
    rightAxis->setSubTicks(false);
    rightAxis->setTickLength(0);
    rightAxis->setBasePen(Qt::NoPen);

    QPen gridPen = QPen(QColor(LogVieverColor_grid), 1, Qt::SolidLine);
    QPen subGridPen = QPen(QColor(LogVieverColor_subgrid), 1, Qt::SolidLine);
    for(auto *axis : rect->axes()) {
        axis->setLayer("axes");
        axis->grid()->setLayer("grid");
        axis->grid()->setSubGridVisible(true);
        axis->grid()->setZeroLinePen(Qt::NoPen);
        axis->grid()->setPen(gridPen);
        axis->grid()->setSubGridPen(subGridPen);
    }
}