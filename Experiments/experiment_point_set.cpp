#include "experiment_point_set.h"
#include "ui_experiment_point_set.h"
#include "QJsonObject"

ExperimentPoint::ExperimentPoint(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ExperimentPoint),
    doubleValueVal(new QRegExpValidator((QRegExp("[-+]?[0-9]+(\\.[0-9]+)?"))))
{
    ui->setupUi(this);
    ui->target_edit->setValidator(doubleValueVal.get());
    ui->spread_edit->setValidator(doubleValueVal.get());
    ui->count_edit->setValidator(new QIntValidator());
    connect(ui->delete_btn, &QPushButton::clicked, [this](){ emit deleteMe();});
    connect(ui->target_edit, &QLineEdit::editingFinished, [this](){
        bool ok;
        double new_target_value = ui->target_edit->text().toDouble(&ok);
        if(ok)
            target_value_ = new_target_value;
    });
    connect(ui->count_edit, &QLineEdit::editingFinished, [this](){
        bool ok;
        int new_count = ui->count_edit->text().toInt(&ok);
        if(ok)
            qty_ = new_count;
    });
    connect(ui->spread_edit, &QLineEdit::editingFinished, [this](){
        bool ok;
        double new_spread = ui->spread_edit->text().toDouble(&ok);
        if(ok)
            spread_ = new_spread;
    });
}

ExperimentPoint::~ExperimentPoint()
{
    delete ui;
}

QPair<double, double> ExperimentPoint::GetTargetValueSpreadPair() const {
    return {target_value_, spread_};
}

double ExperimentPoint::GetTargetValue() const {
    return target_value_;
}

int ExperimentPoint::GetQuantity() const {
    return qty_;
}

double ExperimentPoint::GetSpread() const {
    return spread_;
}

void ExperimentPoint::LoadDataFromJson(const QJsonObject& jsonObject) {
    ui->target_edit->setText(jsonObject["target_edit"].toString());
    ui->count_edit->setText(jsonObject["count_edit"].toString());
    ui->spread_edit->setText(jsonObject["spread_edit"].toString());
    target_value_ = ui->target_edit->text().toDouble();
    qty_ = ui->count_edit->text().toInt();
    spread_ = ui->spread_edit->text().toDouble();
}

QJsonObject ExperimentPoint::SaveDataToJson(){
    QJsonObject retVal;
    retVal["target_edit"] = ui->target_edit->text();
    retVal["count_edit"] = ui->count_edit->text();
    retVal["spread_edit"] = ui->spread_edit->text();
    return retVal;
}
