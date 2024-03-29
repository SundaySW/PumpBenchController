#include "experiment_param.h"
#include "ui_experiment_param.h"

ExperimentParam::ExperimentParam(const QStringList& params, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ExperimentParam)
{
    ui->setupUi(this);
    connect(ui->param_delete_btn, &QPushButton::clicked, [this](){ emit deleteMe();});
    ui->param_comboBox->clear();
    ui->param_comboBox->setCurrentIndex(-1);
    ui->param_comboBox->addItems(params);
}

ExperimentParam::~ExperimentParam()
{
    delete ui;
}

ParamEntity ExperimentParam::GetParamEntity() {
    return {ui->param_comboBox->currentText(), ui->param_name_lineEdit->text()};
}

void ExperimentParam::LoadDataFromJson(const QJsonObject& jsonObject) {
    ui->param_comboBox->setCurrentText(jsonObject["param_comboBox"].toString());
    ui->param_name_lineEdit->setText(jsonObject["param_name_lineEdit"].toString());
}

QJsonObject ExperimentParam::SaveDataToJson() {
    QJsonObject retVal;
    retVal["param_comboBox"] = ui->param_comboBox->currentText();
    retVal["param_name_lineEdit"] = ui->param_name_lineEdit->text();
    return retVal;
}


