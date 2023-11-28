#ifndef EXPERIMENT_PARAM_H
#define EXPERIMENT_PARAM_H

#include <QWidget>

#include "experiment.hpp"

namespace Ui {
class ExperimentParam;
}

class ExperimentParam : public QWidget
{
    Q_OBJECT

public:
    explicit ExperimentParam(const QStringList& params, QWidget *parent = nullptr);
    ~ExperimentParam();
    ParamEntity GetParamEntity();
    void LoadDataFromJson(const QJsonObject&);
    QJsonObject SaveDataToJson();
signals:
    void deleteMe();
private:
    Ui::ExperimentParam *ui;
};

#endif // EXPERIMENT_PARAM_H
