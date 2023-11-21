#ifndef EXPERIMENTSETTINGS_H
#define EXPERIMENTSETTINGS_H

#include <QDialog>
#include "QMap"

#include "experiment_point_set.h"
#include "experiment_param.h"
#include "experiment.hpp"

namespace Ui {
class ExperimentSettings;
}

class ExperimentSettings : public QDialog
{
    Q_OBJECT

public:
    explicit ExperimentSettings(QWidget *parent = nullptr);
    ~ExperimentSettings();
    void LoadPtrs(QSharedPointer<BenchViewCtrlItem> ctrlItem,
                  QSharedPointer<ParamService> ps);
    void LoadDataFromJson(const QJsonObject&);
    QJsonObject SaveDataToJson();
    void receiveItemsNameList(const QVector<BenchViewItem::ViewItemData>& data);
    void NewFeedBackParamName(const QString&);
private:
    Ui::ExperimentSettings *ui;
    QMap<ExperimentPoint*, QSharedPointer<ExperimentPoint>> points_map_;
    QMap<ExperimentParam*, QSharedPointer<ExperimentParam>> params_map_;
    QSharedPointer<ParamService> param_service_;
    QSharedPointer<BenchViewCtrlItem> ctrlItem_;
    QSharedPointer<Experiment> experiment_;
    void MakeConnections();
    ExperimentPoint *addExpPoint();
    ExperimentParam *addExpParam();
    void StartExperiment();
    void UpdateExperiment();
    void StopExperiment();
    void ExpPointFinished(int total, int current, double name);
};

#endif // EXPERIMENTSETTINGS_H
