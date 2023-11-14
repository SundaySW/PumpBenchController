#include "experimentsettings.h"

#include <utility>
#include "ui_experimentsettings.h"

ExperimentSettings::ExperimentSettings(QSharedPointer<BenchViewCtrlItem> ctrlItem,
                                       QSharedPointer<ParamService> ps,
                                       QWidget *parent)
    :QDialog(parent),
    ui(new Ui::ExperimentSettings),
    param_service_(ps),
    experiment_(std::move(ctrlItem), std::move(ps))
{
    ui->setupUi(this);
    MakeConnections();
}

ExperimentSettings::~ExperimentSettings()
{
    delete ui;
}

void ExperimentSettings::MakeConnections() {
    connect(ui->add_point_btn, &QPushButton::clicked, [this](){ addExpPoint(); });
    connect(ui->add_param_btn, &QPushButton::clicked, [this](){ addExpParam(); });
    connect(ui->start_exp_btn, &QPushButton::clicked, [this](){ StartExperiment(); });
}

ExperimentPoint* ExperimentSettings::addExpPoint(){
    auto point_shrdPtr = QSharedPointer<ExperimentPoint>(new ExperimentPoint(this));
    auto* point_rawPtr = point_shrdPtr.get();
    points_map_.insert(point_rawPtr, point_shrdPtr);
    connect(point_rawPtr, &ExperimentPoint::deleteMe, [this, point_rawPtr](){
        points_map_.remove(point_rawPtr);
        ui->ponts_layout->removeWidget(point_rawPtr);
    });
    ui->ponts_layout->addWidget(point_rawPtr);
    return point_rawPtr;
}

ExperimentParam* ExperimentSettings::addExpParam(){
    auto params_list_onLine = param_service_->getAllParamsStrList();
    auto param_shrdPtr = QSharedPointer<ExperimentParam>(new ExperimentParam(params_list_onLine, this));
    auto* param_rawPtr = param_shrdPtr.get();
    params_map_.insert(param_rawPtr, param_shrdPtr);
    connect(param_rawPtr, &ExperimentParam::deleteMe, [this, param_rawPtr](){
        params_map_.remove(param_rawPtr);
        ui->params_layout->removeWidget(param_rawPtr);
    });
    ui->params_layout->addWidget(param_rawPtr);
    return param_rawPtr;
}

void ExperimentSettings::StartExperiment(){
    QVector<PointEntity> points;
    points.reserve(points_map_.size());
    for(auto& view_point : points_map_){
        points.push_back(view_point->GetPointEntity());
    }
    QVector<ParamEntity> params;
    params.reserve(params_map_.size());
    for(auto& view_param : params_map_){
        params.push_back(view_param->GetParamEntity());
    }
}
