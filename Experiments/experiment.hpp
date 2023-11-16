#pragma once

#include <utility>

#include "QMap"

#include "BenchItem/bench_view_ctrl_item.h"

struct ParamEntity{
    QString protos_param_name;
    QString name;
};

class PointEntity{
public:
    PointEntity() = delete;
    explicit PointEntity(QPair<double, double> tg_spread, int qty, QVector<ParamEntity> params)
        : target_value_(tg_spread.first),
          quantity_(qty),
          params_(std::move(params))
    {
        bounds_ = {target_value_ - tg_spread.second, target_value_ + tg_spread.second};
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
    [[nodiscard]] double TargetValue() const { return target_value_; }
    [[nodiscard]] bool isInBounds(double value) const { return value > bounds_.first && value < bounds_.second; }
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
    Experiment() = delete;
    Experiment(QSharedPointer<BenchViewCtrlItem> ctrlItem,
               QSharedPointer<ParamService> ps,
               QPair<double, double> target_value_spread,
               int cnt,
               QVector<ParamEntity>&& params)
        : control_item_(std::move(ctrlItem)),
          param_service_(std::move(ps)),
          start_target_value_spread_(target_value_spread),
          stable_start_desired_cnt_(cnt),
          params_(std::move(params)),
          current_point_(target_value_spread, cnt, params)
    {}
    ~Experiment() = default;
    void UpdateExperiment(double value){
        if(stable_reached_){
            PointProcess(value);
        }else
            InitialStabilization(value);
    }

    void FinishExperiment(){

    }

private:
    int point_cnt_{0};
    PointEntity current_point_;
    QVector<PointEntity> points_;
    QVector<ParamEntity> params_;
    QPair<double, double> start_target_value_spread_;
    const int stable_start_desired_cnt_ {0};
    bool stable_reached_ {false};

    QSharedPointer<BenchViewCtrlItem> control_item_;
    QSharedPointer<ParamService> param_service_;

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

    QJsonObject PrepareExpResultPoints(){
        QJsonObject points;
        for(auto& point : points_){
            QJsonObject point_record;
            QJsonArray params_arr;
            for(auto& [param_name, storage] : point.GetResultsStorage().toStdMap()){
                QJsonObject param_record;
                auto average_value = std::accumulate( storage.begin(), storage.end(), 0.0) / storage.size();
                param_record[param_name] = average_value;
                params_arr.append(param_record);
            }
            point_record.insert("Params", params_arr);
            points.insert(QString::number(point.TargetValue()), point_record);
        }
        return points;
    }

    void InitialStabilization(double value){
        static std::size_t stable_start_cnt = 0;
        if(IsInStabilizationBounds(value))
            stable_start_cnt++;
        if(stable_start_cnt >= stable_start_desired_cnt_){
            stable_reached_ = true;
        }
    }

    [[nodiscard]] bool IsInStabilizationBounds(double value) const{
        auto lower_bound = start_target_value_spread_.first - start_target_value_spread_.second;
        auto upper_bound = start_target_value_spread_.first + start_target_value_spread_.second;
        return value >= lower_bound && value <= upper_bound;
    }
};