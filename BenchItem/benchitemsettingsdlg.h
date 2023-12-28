#ifndef BENCHITEMSETTINGSDLG_H
#define BENCHITEMSETTINGSDLG_H

#include <QDialog>
#include <QLabel>
#include "Param_Item/ParamItem.h"
#include "ParamService.h"
#include "QRegExpValidator"

namespace Ui {
class BenchItemSettingsDlg;
}

class BenchItemSettingsDlg : public QDialog
{
    Q_OBJECT
public:
    struct PIDSettings{
        double min = 0, max = 5000;
        double Kp = 0.1, Ki = 0.5, Kd = 0.01;
    };
    explicit BenchItemSettingsDlg(QString , ParamService* ps, bool, QWidget *parent = nullptr);
    ~BenchItemSettingsDlg();
    void setActualValues(const ProtosMessage &message);
    void refreshParamList();
    void setPidBoundsOfNewItem(std::tuple<double, double>);
    void setUpdateValueBounds(const QPair<double, double>&);
    void setSetValueBounds(const QPair<int, int>&);
    void setSetParamAddr(const QPair<uchar , uchar>&);
    void setPIDSettings(const QJsonObject&);
    void setUpdateParamFromController(const QString& mapKey);
signals:
    void itemParamChanged(QSharedPointer<ParamItem>&);
    void itemParamUnbinded();
    void newSetParamKey(const QString&);
    void newSetParamId(uchar);
    void newSetParamHost(uchar);
    void newPIDSettings(const PIDSettings&);
    void newSetValueBounds(const QPair<int, int>&);
    void newUpdateValueBounds(const QPair<double, double>&);
private:
    QSharedPointer<ParamItem> paramItem;
    ParamService* paramService;
    Ui::BenchItemSettingsDlg *ui;
    PIDSettings pidSettings;
    QString itemName;
    QSharedPointer<QRegExpValidator> doubleValueVal;
    QSharedPointer<QRegExpValidator> unsFloatValueVal;

    void newParamRequested(const QString &mapKey);
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
    void unbindProtosParam();
    void ShowCalculationDlg();
    static std::pair<double, double> CalcCalibValues(double a1, double a2, double b1, double b2);
};

#endif // BENCHITEMSETTINGSDLG_H
