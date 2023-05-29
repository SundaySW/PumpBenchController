//
// Created by user on 17.05.2023.
//

#ifndef PUMPBENCHCONTROLLER_PIDCONTROL_H
#define PUMPBENCHCONTROLLER_PIDCONTROL_H

#include "QJsonObject"

class PIDControl {
public:
    explicit PIDControl(double min = 0, double max = 5000, double Kp = 0.1, double Ki = 0.5, double Kd = 0.01);
    double calculate(double setPoint, double processValue, double dt);
    void changeValueBounds(double min, double max);
    void changeKs(double Kp, double Ki, double Kd);
    QJsonObject toJson();
    void fromJson(const QJsonObject& jsonObject);
    void reset();
private:
    double valueMin, valueMax,
           proportionalGain, integralGain, derivativeGain,
           preError,
           integral;
};

#endif //PUMPBENCHCONTROLLER_PIDCONTROL_H
