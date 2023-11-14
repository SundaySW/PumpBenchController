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
    explicit ExperimentSettings(QSharedPointer<BenchViewCtrlItem> ctrlItem,
                                QSharedPointer<ParamService> ps,
                                QWidget *parent = nullptr);
    ~ExperimentSettings();

private:
    Ui::ExperimentSettings *ui;
    QMap<ExperimentPoint*, QSharedPointer<ExperimentPoint>> points_map_;
    QMap<ExperimentParam*, QSharedPointer<ExperimentParam>> params_map_;
    QSharedPointer<ParamService> param_service_;
    Experiment experiment_;
    void MakeConnections();
    ExperimentPoint *addExpPoint();
    ExperimentParam *addExpParam();
    void StartExperiment();
};

#endif // EXPERIMENTSETTINGS_H
