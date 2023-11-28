#pragma once

#include <utility>

#include "QMap"
#include "QDateTime"
#include "QJsonObject"
#include "QJsonArray"
#include "QJsonDocument"
#include "QFile"

#include "BenchItem/bench_view_ctrl_item.h"

struct ParamEntity{
    QString protos_param_name;
    QString name;
};

class PointEntity{
public:
    PointEntity() = delete;
    explicit PointEntity(QPair<double, double> tg_spread, int qty, QVector<ParamEntity> params, bool start_point = false)
        : target_value_(tg_spread.first),
          quantity_(qty),
          params_(std::move(params)),
          start_point_(start_point)
    {
        bounds_ = {target_value_ - tg_spread.second, target_value_ + tg_spread.second};
        try{
            for(auto& param : params_){
                param_results_.insert(param.protos_param_name, QVector<double>());
                param_results_.last().reserve(quantity_);
            }
        }catch (...){
            //todo log error
        }
    };
    [[nodiscard]] int GetQty() const{ return quantity_; }
    [[nodiscard]] int GetCnt() const{ return count_; }
    bool IsStartPoint() { return start_point_; }
    void IncCnt(){ count_++; }
    [[nodiscard]] bool IsEnough() const { return count_ >= quantity_; }
    auto& GetResultsStorage() { return param_results_; }
    [[nodiscard]] double TargetValue() const { return target_value_; }
    [[nodiscard]] bool IsInBounds(double value) const { return value > bounds_.first && value < bounds_.second; }
    void Started() { start_time_ = QDateTime::currentDateTime().toMSecsSinceEpoch(); }
    void SetFinished() { finished_ = true; stop_time_ = QDateTime::currentDateTime().toMSecsSinceEpoch(); }
    [[nodiscard]] bool IsFinished() const { return finished_; }
    [[nodiscard]] QDateTime GetElapsedDateTime() const { return QDateTime::fromMSecsSinceEpoch(stop_time_ - start_time_, Qt::UTC);}
private:
    QPair<double,double> bounds_;
    double target_value_;
    int quantity_;
    int count_{0};
    bool finished_ {false};
    bool start_point_;
    QVector<ParamEntity> params_;
    QMap<QString, QVector<double>> param_results_;
    qint64 start_time_;
    qint64 stop_time_;
};

class Experiment: public QObject{
    Q_OBJECT
public:
    Experiment() = delete;
    Experiment(QVector<PointEntity> points,
               QSharedPointer<BenchViewCtrlItem> ctrlItem,
               QSharedPointer<ParamService> ps,
               QVector<ParamEntity>&& params,
               QString device_name)
        : points_(std::move(points)),
          control_item_(std::move(ctrlItem)),
          param_service_(std::move(ps)),
          params_(std::move(params)),
          device_name_(std::move(device_name)),
          current_point_(points.first())
    {
        StartExperiment();
    }

    void UpdateExperiment(double value){
        PointProcess(value);
    }

    void FinishExperiment(){
        control_item_->SetPIDControl(true);
        WriteResultsToFile(PrepareResults());
    }

    signals:
    void ExperimentFinished();
    void PointStarted(int total, int current, double current_point_v);

private:
    int point_cnt_{0};
    std::reference_wrapper<PointEntity> current_point_;
    QVector<PointEntity> points_;
    QVector<ParamEntity> params_;
    QString device_name_;
    QDateTime start_dateTime_;

    QSharedPointer<BenchViewCtrlItem> control_item_;
    QSharedPointer<ParamService> param_service_;

    void WriteResultsToFile(const QJsonObject& results){
        QJsonDocument doc;
        doc.setObject(results);
        auto dir = QCoreApplication::applicationDirPath() + "/Experiment_Results/";
        QDir().mkdir(dir);
        QString time_stamp = QDateTime::currentDateTime().toString(QString("hh.mm_dd.MM.yyyy"));
        auto fileName = dir + QString("%1 at %2.json").arg(device_name_, time_stamp);
        auto file = QFile(fileName);
        file.open(QIODevice::ReadWrite);
        file.resize(0);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }

    void StartExperiment(){
        control_item_->SetTargetValue(current_point_.get().TargetValue());
        control_item_->SetPIDControl(false);
        current_point_.get().Started();
        start_dateTime_ = QDateTime::currentDateTime();
        emit PointStarted(points_.size(), point_cnt_, current_point_.get().TargetValue());
    }

    void PointProcess(double value){
        if(current_point_.get().IsInBounds(value)){
            current_point_.get().IncCnt();
            RecordPointValues();
            if(current_point_.get().IsEnough())
                FinishPoint();
        }else{
            //todo add total count
        }
    }

    void RecordPointValues(){
        auto& params_map = current_point_.get().GetResultsStorage();
        for(auto it = params_map.begin(); it != params_map.end(); it++){
            it.value().push_back(param_service_->getParamValue(it.key()).toDouble());
        }
    }

    void FinishPoint(){
        current_point_.get().SetFinished();
        SetNewPoint();
    }

    void SetNewPoint(){
        if(++point_cnt_ >= points_.size())
            emit ExperimentFinished();
        else{
            current_point_ = points_[point_cnt_];
            current_point_.get().Started();
            auto current_point_v = current_point_.get().TargetValue();
            control_item_->SetTargetValue(current_point_v);
            emit PointStarted(points_.size(), point_cnt_, current_point_v);
        }
    }

    QJsonObject PrepareResults(){
        QJsonObject points;
        for(auto& point : points_){
            if(!point.IsFinished()){
                points.insert(QString::number(point.TargetValue()), "skipped");
                continue;
            }
            QJsonObject params_arr;
            auto& params_map = point.GetResultsStorage();
            for(auto it = params_map.begin(); it != params_map.end(); it++){
                auto& param_name = it.key();
                auto& storage = it.value();
                auto average_value = std::accumulate( storage.begin(), storage.end(), 0.0) / storage.size();
                params_arr[param_name] = average_value;
            }
            QJsonObject point_record;
            point_record["time elapsed"] = point.GetElapsedDateTime().toString(QString("hh:mm:ss.zzz"));
            point_record.insert("Params", params_arr);
            auto point_name = point.IsStartPoint() ? "start point:" + QString::number(point.TargetValue())
                                                        : QString::number(point.TargetValue());
            points.insert(point_name, point_record);
        }

        QJsonObject result;
        result.insert("Points", points);

        QJsonObject param_names;
        for(auto& param_entity : params_)
            param_names[param_entity.protos_param_name] = param_entity.name;
        result.insert("Param_names", param_names);

        QJsonObject device;
        device["N"] = device_name_;
        device["STARTED Date"] = start_dateTime_.toString(QString("dd.MM.yyyy"));
        device["STARTED Time"] = start_dateTime_.toString(QString("hh:mm.ss"));
        device["Finished Date"] = QDateTime::currentDateTime().toString(QString("dd.MM.yyyy"));
        device["Finished Time"] = QDateTime::currentDateTime().toString(QString("hh:mm.ss"));
        result.insert("Device", device);

        return result;
    }
};