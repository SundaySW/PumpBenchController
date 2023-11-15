#include "experiment_param.h"
#include "ui_experiment_param.h"

ExperimentParam::ExperimentParam(const QStringList& params, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ExperimentParam)
{
    ui->setupUi(this);
    connect(ui->param_delete_btn, &QPushButton::clicked, [this](){ emit deleteMe();});
    connect(ui->param_comboBox, &QComboBox::currentTextChanged,[this](const QString& s){
        param_entity_.protos_param_name = s;
    });
    connect(ui->param_name_lineEdit, &QLineEdit::editingFinished, [this](){
        param_entity_.name = ui->param_name_lineEdit->text();
    });
    ui->param_comboBox->clear();
    ui->param_comboBox->setCurrentIndex(-1);
    ui->param_comboBox->addItems(params);
}

ExperimentParam::~ExperimentParam()
{
    delete ui;
}

ParamEntity ExperimentParam::GetParamEntity() {
    return param_entity_;
}

