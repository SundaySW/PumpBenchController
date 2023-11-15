
#include "experiment.hpp"

#include <utility>

Experiment::Experiment(QSharedPointer<BenchViewCtrlItem>&& ctrlItem, QSharedPointer<ParamService>&& ps)
    :control_item_(std::move(ctrlItem)),
     param_service_(std::move(ps))
{
    MakeConnections();
}

void Experiment::MakeConnections(){

}

void Experiment::UpdateExperiment(){
    if(stable_reached){

    }else{

    }
}

void StartStabilization(){

}