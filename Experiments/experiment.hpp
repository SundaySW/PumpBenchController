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

class Experiment: public QObject{
    Q_OBJECT
public:
    Experiment() = delete;
    Experiment(QVector<PointEntity> points,
               QSharedPointer<BenchViewCtrlItem> ctrlItem,
               QSharedPointer<ParamService> ps,
               QPair<double, double> target_value_spread,
               int cnt,
               QVector<ParamEntity>&& params,
               QString device_name)
        :points_(std::move(points)),
        control_item_(std::move(ctrlItem)),
        param_service_(std::move(ps)),
        start_target_value_spread_(target_value_spread),
        stable_start_desired_cnt_(cnt),
        params_(std::move(params)),
        current_point_(target_value_spread, cnt, params),
        device_name_(std::move(device_name))
    {
        StartExperiment();
    }

    ~Experiment() = default;

    void UpdateExperiment(double value){
        if(stable_reached_){
            PointProcess(value);
        }else
            InitialStabilization(value);
    }

    void FinishExperiment(){
        control_item_->SetPIDControl(true);
        auto results = PrepareExpResultPoints();
        WriteResultsToFile(results);
    }

    signals:
    void ExperimentFinished();
    void PointFinished(int total, int current, double current_point_v);

private:
    int point_cnt_{0};
    PointEntity current_point_;
    QVector<PointEntity> points_;
    QVector<ParamEntity> params_;
    QPair<double, double> start_target_value_spread_;
    const int stable_start_desired_cnt_ {0};
    bool stable_reached_ {false};
    QString device_name_;

    QSharedPointer<BenchViewCtrlItem> control_item_;
    QSharedPointer<ParamService> param_service_;

    void WriteResultsToFile(const QJsonObject& results){
        QJsonDocument doc;
        doc.setObject(results);
        auto fileName = QString("Device_n:%1__%2").arg(device_name_,
                                             QDateTime::currentDateTime().toString(QString("yyyy.MM.dd_hh.mm")));
        auto file = new QFile(QCoreApplication::applicationDirPath() + QString("/%1.json").arg(fileName));
        file->open(QIODevice::ReadWrite);
        file->resize(0);
        file->write(doc.toJson(QJsonDocument::Indented));
        file->close();
    }

    void StartExperiment(){
        control_item_->SetTargetValue(start_target_value_spread_.first);
        control_item_->SetPIDControl(false);
    }

    void PointProcess(double value){
        if(current_point_.isInBounds(value)){
            current_point_.IncCnt();
            RecordPointValues();
            if(current_point_.GetCnt() >= current_point_.Qty())
                FinishPoint();
        }else{
            //todo add total count
        }
    }

    void RecordPointValues(){
        for(auto& [param_name, storage] : current_point_.GetResultsStorage().toStdMap()){
            storage.push_back(param_service_->getParam(param_name)->getValue().toDouble());
        }
    }

    void FinishPoint(){
        current_point_.SetFinished();
        SetNewPoint();
    }

    void SetNewPoint(){
        if(++point_cnt_ >= points_.size())
            emit ExperimentFinished();
        else{
            current_point_ = points_[point_cnt_];
            auto current_point_v = current_point_.TargetValue();
            control_item_->SetTargetValue(current_point_v);
            emit PointFinished(points_.size(), point_cnt_, current_point_v);
        }
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
            point_cnt_ = -1; //todo remove
            SetNewPoint();
        }
    }

    [[nodiscard]] bool IsInStabilizationBounds(double value) const{
        auto lower_bound = start_target_value_spread_.first - start_target_value_spread_.second;
        auto upper_bound = start_target_value_spread_.first + start_target_value_spread_.second;
        return value >= lower_bound && value <= upper_bound;
    }
};