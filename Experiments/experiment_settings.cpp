#include "experiment_settings.h"

#include <utility>
#include "ui_experiment_settings.h"

ExperimentSettings::ExperimentSettings(QWidget *parent)
    :QDialog(parent),
    ui(new Ui::ExperimentSettings)
{
    ui->setupUi(this);
    MakeConnections();
    ui->stable_value_target->setValidator(new QRegExpValidator(QRegExp("[-+]?[0-9]+(\\.[0-9]+)?")));
    ui->stable_value_spread->setValidator(new QRegExpValidator(QRegExp("[-+]?[0-9]+(\\.[0-9]+)?")));
    ui->count_stable->setValidator(new QIntValidator());
}

ExperimentSettings::~ExperimentSettings()
{
    delete ui;
}

void ExperimentSettings::MakeConnections() {
    connect(ui->add_point_btn, &QPushButton::clicked, [this](){ addExpPoint(); });
    connect(ui->add_param_btn, &QPushButton::clicked, [this](){ addExpParam(); });
    connect(ui->start_exp_btn, &QPushButton::clicked, [this](){
        StartExperiment();
        if(ui->start_exp_btn->isEnabled()){
            StopExperiment();
        }else{
            StopExperiment();
        }
    });
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
    QVector<ParamEntity> params;
    params.reserve(params_map_.size());
    for(auto& view_param : params_map_){
        params.push_back(view_param->GetParamEntity());
    }
    QVector<PointEntity> points;
    points.reserve(points_map_.size());
    for(auto& view_point : points_map_){
        points.push_back(PointEntity(view_point->GetTargetValueSpreadPair(),
                                     view_point->GetQuantity(),
                                     params));
    }
    auto tg_value = ui->stable_value_target->text().toDouble();
    auto spread = ui->stable_value_spread->text().toDouble();
    auto qty = ui->count_stable->text().toInt();
    experiment_.reset(new Experiment(ctrlItem_, param_service_, {tg_value,spread}, qty, std::move(params)));
    connect(ctrlItem_->getFeedBackParamRawPtr(), &ParamItem::newParamValue, [this](){ UpdateExperiment(); });
}

void ExperimentSettings::StopExperiment(){
    disconnect(ctrlItem_->getFeedBackParamRawPtr(), &ParamItem::newParamValue, this, nullptr);
    experiment_->FinishExperiment();
}

void ExperimentSettings::UpdateExperiment(){
    auto value = ctrlItem_->getFeedBackParamRawPtr()->getValue().toDouble();
    experiment_->UpdateExperiment(value);
}

void ExperimentSettings::LoadPtrs(QSharedPointer<BenchViewCtrlItem> ctrlItem, QSharedPointer<ParamService> ps) {
    param_service_ = std::move(ps);
    ctrlItem_ = std::move(ctrlItem);
}

void ExperimentSettings::LoadDataFromJson(const QJsonObject& jsonObject) {
    if(jsonObject.empty())
        return;
    ui->stable_value_target->setText(jsonObject["stable_value_target"].toString());
    ui->stable_value_spread->setText(jsonObject["stable_value_spread"].toString());
    ui->device_n_edit->setText(jsonObject["device_n_edit"].toString());
    ui->count_stable->setText(jsonObject["count_stable"].toString());
    auto point_arr = jsonObject["Points"].toArray();
    for(const auto& point : point_arr)
    {
        auto point_json_obj = point.toObject();
        if(!point_json_obj.isEmpty())
            addExpPoint()->LoadDataFromJson(point_json_obj);
    }
    auto param_arr = jsonObject["Params"].toArray();
    for(const auto& param : param_arr)
    {
        auto param_json_obj = param.toObject();
        if(!param_json_obj.isEmpty())
            addExpParam()->LoadDataFromJson(param_json_obj);
    }
}

QJsonObject ExperimentSettings::SaveDataToJson(){
    QJsonObject retVal;
    retVal["stable_value_target"] = ui->stable_value_target->text();
    retVal["stable_value_spread"] = ui->stable_value_spread->text();
    retVal["device_n_edit"] = ui->device_n_edit->text();
    retVal["count_stable"] = ui->count_stable->text();

    QJsonArray point_arr;
    for(auto& point: points_map_)
        point_arr.append(point->SaveDataToJson());
    retVal["Points"] = point_arr;

    QJsonArray param_arr;
    for(auto& param: params_map_)
        param_arr.append(param->SaveDataToJson());
    retVal["Params"] = param_arr;
    return retVal;
}

void ExperimentSettings::receiveItemsNameList(const QVector<BenchViewItem::ViewItemData> &data) {
    ui->ctrl_param_box->clear();
    for(const auto& item: data){
        ui->ctrl_param_box->addItem(item.icon, item.name);
    }
}

void ExperimentSettings::NewFeedBackParamName(const QString& name) {
    ui->ctrl_param_box->setCurrentText(name);
}