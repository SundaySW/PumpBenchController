//
// Created by user on 12.05.2023.
//

#ifndef PUMPBENCHCONTROLLER_BENCHVIEWITEM_H
#define PUMPBENCHCONTROLLER_BENCHVIEWITEM_H

#include <QLabel>
#include <QPushButton>
#include "Param_Item/ParamItem.h"
#include "benchitemsettingsdlg.h"
#include "ParamService.h"
#include "qcustomplot.h"

class BenchViewItem: public QObject{
    Q_OBJECT
public:
    struct ViewItemData{
        QIcon icon;
        QString name;
    };
    BenchViewItem() = delete;
    BenchViewItem(QString name, QCustomPlot*, QLabel*, QPushButton*, ParamService*, QWidget *parent = nullptr);
    void updateView();
    void processValue();
    QString getName();
    QString getParamKey();
    std::tuple<double, double> getBounds();
    [[nodiscard]]ParamItem* getParamPtr() const;
    bool isProtosParamSelected();
    bool getCurrentStatus() const;
    const QIcon &getIcon() const;
    const QIcon &getErrorIcon() const;
    QString getLastValueDateTimeStr();
    const QVariant &getCurrentValue() const;
    QPushButton *getButton() const;
    void loadDataFromJson(const QJsonObject&);
    QJsonObject saveDataToJSon();
    void repaintPlot();
signals:
    void signalValueUpdated_itemValue(double value, bool ok);
    void signalValueUpdated_itemName(const QString& name);

protected:
    double normal_value_upperBound_;
    double normal_value_lower_bound_;
    BenchItemSettingsDlg* settings_dlg_;
    QColor color;
    ParamService* param_service_;
private:
    QString item_name_;
    QSharedPointer<ParamItem> protos_item_;
    QVariant currentValue_;
    QIcon iconDef, iconOK, iconError;
    double current_value_double_;
    bool currentStatus;
    bool livePlot = true;

    QCustomPlot* Plot;
    QCPAxisRect* rect;
    QCPGraph* selfGraph;
    QSharedPointer<QCPAxisTickerTime> timeTicker;
    int lineWidth = 1;
    QCPGraph::LineStyle lineStyle = QCPGraph::lsLine;
    QSharedPointer<QCPGraphDataContainer> graphData;
    QPushButton* button;
    double lastKey;

    QLabel* label;
    QTimer* updateValueTimer;
    void itemButtonClicked();
    void updateValueTimerFinished();
    void changeIconColor(bool ref);
    void setPlot();
    void plotBindGraph();
    void prepareRect();
    void updatePlotData();
    void unsetPlotData();
    void resetPlotData();
    void generateColor();
};

#endif //PUMPBENCHCONTROLLER_BENCHVIEWITEM_H