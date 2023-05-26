//
// Created by user on 17.05.2023.
//

#ifndef PUMPBENCHCONTROLLER_PIDCONTROL_H
#define PUMPBENCHCONTROLLER_PIDCONTROL_H

class PIDControl {
public:
    explicit PIDControl(double min = 100, double max = 4000, double Kp = 0.1, double Ki = 0.5, double Kd = 0.01);
    double calculate(double setPoint, double processValue, double dt);
    void changeValueBounds(double min, double max);
    void changeKs(double Kp, double Ki, double Kd);
private:
    double valueMin, valueMax,
           proportionalGain, integralGain, derivativeGain,
           preError,
           integral;
};

#endif //PUMPBENCHCONTROLLER_PIDCONTROL_H
