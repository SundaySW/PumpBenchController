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
signals:
    void deleteMe();
private:
    Ui::ExperimentParam *ui;
    ParamEntity param_entity_;
};

#endif // EXPERIMENT_PARAM_H
