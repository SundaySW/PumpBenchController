#include "experiment_point_set.h"
#include "ui_experiment_point_set.h"

ExperimentPoint::ExperimentPoint(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ExperimentPoint)
{
    ui->setupUi(this);
    connect(ui->delete_btn, &QPushButton::clicked, [this](){ emit deleteMe();});
    connect(ui->target_edit, &QLineEdit::editingFinished, [this](){
        bool ok;
        double new_target_value = ui->target_edit->text().toDouble(&ok);
        if(ok)
            point_.target_value = new_target_value;
    });
    connect(ui->count_edit, &QLineEdit::editingFinished, [this](){
        bool ok;
        int new_count = ui->count_edit->text().toInt(&ok);
        if(ok)
            point_.count = new_count;
    });
    connect(ui->spread_edit, &QLineEdit::editingFinished, [this](){
        bool ok;
        double new_spread = ui->spread_edit->text().toDouble(&ok);
        if(ok)
            point_.spread = new_spread;
    });
}

ExperimentPoint::~ExperimentPoint()
{
    delete ui;
}

PointEntity ExperimentPoint::GetPointEntity(){
    return point_;
}

double ExperimentPoint::GetTargetValue() {
    return point_.target_value;
}

unsigned int ExperimentPoint::GetCount() {
    return point_.count;
}

double ExperimentPoint::GetSpread() {
    return point_.spread;
}
