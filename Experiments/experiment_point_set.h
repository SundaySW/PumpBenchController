#ifndef EXPERIMENT_POINT_SET_H
#define EXPERIMENT_POINT_SET_H

#include <QWidget>

#include "experiment.hpp"

namespace Ui {
class ExperimentPoint;
}

class ExperimentPoint : public QWidget
{
    Q_OBJECT

public:
    explicit ExperimentPoint(QWidget *parent = nullptr);
    ~ExperimentPoint();
    double GetTargetValue();
    unsigned int GetCount();
    double GetSpread();
    PointEntity GetPointEntity();
signals:
    void deleteMe();
private:
    Ui::ExperimentPoint *ui;
    PointEntity point_;
};

#endif // EXPERIMENT_POINT_SET_H
