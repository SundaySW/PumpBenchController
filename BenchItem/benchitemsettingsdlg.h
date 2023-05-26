#ifndef BENCHITEMSETTINGSDLG_H
#define BENCHITEMSETTINGSDLG_H

#include <QDialog>
#include <QLabel>
#include "Param_Item/ParamItem.h"
#include "ParamService.h"

namespace Ui {
class BenchItemSettingsDlg;
}

class BenchItemSettingsDlg : public QDialog
{
    Q_OBJECT
public:
    struct PIDSettings{
        double min, max;
        double Kp,Ki,Kd;
    };
    explicit BenchItemSettingsDlg(const QString&, ParamService* ps, bool, QWidget *parent = nullptr);
    ~BenchItemSettingsDlg();
    void setActualValues(const ProtosMessage &message);
    void refreshParamList();
    void setPidBoundsOfNewItem(std::tuple<double, double>);
signals:
    void itemParamChanged(QSharedPointer<ParamItem>&);
    void newSetParamKey(const QString&);
    void newSetParamId(uchar);
    void newSetParamHost(uchar);
    void newPIDSettigs(const PIDSettings&);
    void newSetValueBounds(const QPair<int, int>&);
    void newUpdateValueBounds(const QPair<double, double>&);
private:
    QSharedPointer<ParamItem> paramItem;
    ParamService* paramService;
    Ui::BenchItemSettingsDlg *ui;
    PIDSettings pidSettings;
    QString itemName;

    void loadNewParam(const QString &mapKey);
    void getCalib();
    void getRates();
    void setCalib();
    void setRates();

    template<class T>
    void setActualValues(uchar paramField, T value);
    static void setLabelActualValue(QLabel *label, const QString &value);
    static void unsetActualValue(QLabel *label);
    void pidValuesConnection();
    void updateParamConnections();
    void setParamConnections();
    void paramSettingsConnection();
};

#endif // BENCHITEMSETTINGSDLG_H
