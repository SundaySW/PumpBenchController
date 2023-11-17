#ifndef EXPERIMENT_POINT_SET_H
#define EXPERIMENT_POINT_SET_H

#include <QWidget>
#include <QRegExpValidator>

namespace Ui {
class ExperimentPoint;
}

class ExperimentPoint : public QWidget
{
    Q_OBJECT

public:
    explicit ExperimentPoint(QWidget *parent = nullptr);
    ~ExperimentPoint();
    QPair<double, double> GetTargetValueSpreadPair() const;
    double GetTargetValue() const;
    int GetQuantity() const;
    double GetSpread() const;
    void LoadDataFromJson(const QJsonObject&);
    QJsonObject SaveDataToJson();
signals:
    void deleteMe();
private:
    Ui::ExperimentPoint *ui;
    QSharedPointer<QRegExpValidator> doubleValueVal;

    double target_value_ {100};
    int qty_ {20};
    double spread_ {10};
};

#endif // EXPERIMENT_POINT_SET_H
