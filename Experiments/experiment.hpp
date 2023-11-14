#pragma once

#include "BenchItem/bench_view_ctrl_item.h"

struct PointEntity{
    double target_value;
    unsigned int count;
    double spread;
};

struct ParamEntity{
    QString protos_param_name;
    QString name;
};

class Experiment {
public:
    Experiment(QSharedPointer<BenchViewCtrlItem>&& ctrlItem, QSharedPointer<ParamService>&& ps);
private:
    QSharedPointer<BenchViewCtrlItem> control_item_;
    QSharedPointer<ParamService> param_service_;

    void MakeConnections();
};