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
    BenchViewItem(QString name, QWidget*, QLabel*, QPushButton*, ParamService*, QWidget *parent = nullptr);
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
signals:
    void signalValueUpdated_itemValue(double value, bool ok);
    void signalValueUpdated_itemName(const QString& name);
protected:
    double normalValueUpperBound,normalValueLowerBound;
    BenchItemSettingsDlg* settingsDlg;
    QColor color;
    ParamService* paramService;
private:
    QString name;
    QSharedPointer<ParamItem> item;
    QVariant currentValue;
    QIcon iconDef, iconOK, iconError;
    double currentValueDouble;
    bool currentStatus;
    QCustomPlot* Plot;
    QCPAxisRect* rect;
    QCPGraph* selfGraph;
    QCPGraph::LineStyle lineStyle = QCPGraph::lsLine;
    QVector<QCPGraphData> graphData;
    QPushButton* button;
    QLabel* label;
    QTimer* updateValueTimer;
    void itemButtonClicked();
    void updateValueTimerFinished();
    void changeIconColor(bool ref);

    void setPlot();
};

#endif //PUMPBENCHCONTROLLER_BENCHVIEWITEM_H