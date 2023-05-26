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

class BenchViewItem: public QObject{
    Q_OBJECT
public:
    BenchViewItem() = delete;
    BenchViewItem(QString name, QWidget*, QLabel*, QPushButton*, ParamService*, QWidget *parent = nullptr);
    void updateView();
    void processValue();
    QString getName();
    QString getParamKey();
    std::tuple<double, double> getBounds();
    [[nodiscard]]ParamItem* getParamPtr() const;
    bool getCurrentStatus() const;
    const QIcon &getIcon() const;
    QString getLastValueDateTimeStr();
    const QVariant &getCurrentValue() const;
    QPushButton *getButton() const;
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
    QWidget* plot;
    QPushButton* button;
    QLabel* label;
    QTimer* updateValueTimer;
    void itemButtonClicked();
    void updateValueTimerFinished();

    void changeIconColor(bool ref);
};

#endif //PUMPBENCHCONTROLLER_BENCHVIEWITEM_H