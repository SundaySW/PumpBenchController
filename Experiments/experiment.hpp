#pragma once

#include "QMap"

#include "BenchItem/bench_view_ctrl_item.h"

struct ParamEntity{
    QString protos_param_name;
    QString name;
};

class PointEntity{
public:
    PointEntity() = delete;
    explicit PointEntity(double tg, int qty, double spread, QVector<ParamEntity> params)
        : target_value_(tg),
          quantity_(qty),
          params_(std::move(params))
    {
        bounds_ = {target_value_ - spread, target_value_ + spread};
        try{
            for(auto& param : params_)
                param_results_.insert(param.protos_param_name, QVector<double>(quantity_));
        }catch (...){
            //todo log error
        }
    };
    [[nodiscard]] int Qty() const{ return quantity_; }
    void IncCnt(){ count_++; }
    [[nodiscard]] int GetCnt() const{ return count_; }
    auto& GetResultsStorage() { return param_results_; }
    double TargetValue() const { return target_value_; }
    bool isInBounds(double value) const { return value > bounds_.first && value < bounds_.second; }
    void SetFinished() {finished_ = true;}
private:
    QPair<double,double> bounds_;
    double target_value_;
    int quantity_;
    int count_{0};
    bool finished_ {false};
    QVector<ParamEntity> params_;
    QMap<QString, QVector<double>> param_results_;
};

class Experiment {
public:
    Experiment(QSharedPointer<BenchViewCtrlItem>&& ctrlItem, QSharedPointer<ParamService>&& ps);
private:
    QSharedPointer<BenchViewCtrlItem> control_item_;
    QSharedPointer<ParamService> param_service_;

    void UpdateExperiment(double value){
        if(stable_reached_){
            PointProcess(value);
        }else
            InitialStabilization(value);
    }

    void PointProcess(double value){
        if(current_point_.isInBounds(value)){
            current_point_.IncCnt();
            RecordPointValues();
            if(current_point_.GetCnt() >= current_point_.Qty())
                FinishPoint();
        }else{

        }
    }

    void RecordPointValues(){
        for(auto& [param_name, storage] : current_point_.GetResultsStorage().toStdMap()){
            storage.push_back(param_service_->getParam(param_name)->getValue().toDouble());
        }
    }

    void FinishPoint(){
        current_point_.SetFinished();
        point_cnt_++;
        if(points_.size() < point_cnt_)
            SetNewPoint();
        else
            FinishExperiment();
    }

    void SetNewPoint(){
        current_point_ = points_[point_cnt_];
        control_item_->setRequestedValue(current_point_.TargetValue());
    }

    void FinishExperiment(){
    }

    QJsonObject PrepareExpResults(){
        QJsonObject result;
        for(auto& point : points_){
            QJsonArray point_array;
            QJsonValue value;
            value
        }
    }

    void InitialStabilization(double value){
        static std::size_t stable_start_cnt = 0;
        if(IsInStabilizationBounds(value))
            stable_start_cnt++;
        if(stable_start_cnt >= stable_start_desired_cnt_){
            stable_reached_ = true;
        }
    }

    bool IsInStabilizationBounds(double value){
        return value >= start_bounds_.first && value <= start_bounds_.second;
    }

    int point_cnt_{0};
    PointEntity current_point_;
    QVector<PointEntity> points_;

    QPair<double, double> start_bounds_;
    const std::size_t stable_start_desired_cnt_;
    bool stable_reached_ {false};

    void MakeConnections();
};