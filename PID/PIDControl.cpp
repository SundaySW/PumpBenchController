//
// Created by user on 17.05.2023.
//

#include "PIDControl.h"

PIDControl::PIDControl(double min, double max, double Kp, double Ki, double Kd):
        valueMin(min), valueMax(max), proportionalGain(Kp), integralGain(Ki), derivativeGain(Kd),
        integral(0), preError(0)
{
}

double PIDControl::calculate(double setPoint, double processValue, double loopIntervalTime) {
    double error = setPoint - processValue;
    double Pout = proportionalGain * error;

    integral += error * loopIntervalTime;
    double Iout = integralGain * integral;

    double derivative = (error - preError) / loopIntervalTime;
    double Dout = derivativeGain * derivative;

    double output = Pout + Iout + Dout;
    if( output > valueMax )
        output = valueMax;
    else if( output < valueMin )
        output = valueMin;

    preError = error;
    return output;
}

void PIDControl::changeValueBounds(double min, double max){
    valueMin = min;
    valueMax = max;
}

void PIDControl::changeKs(double Kp, double Ki, double Kd){
    proportionalGain = Kp;
    integralGain = Ki;
    derivativeGain = Kd;
}

void PIDControl::reset(){
    integral = 0;
    preError = 0;
}

QJsonObject PIDControl::toJson(){
    QJsonObject retVal;
    retVal["valueMin"] = valueMin;
    retVal["valueMax"] = valueMax;
    retVal["proportionalGain"] = proportionalGain;
    retVal["integralGain"] = integralGain;
    retVal["derivativeGain"] = derivativeGain;
    retVal["preError"] = preError;
    retVal["integral"] = integral;
    return retVal;
}

void PIDControl::fromJson(const QJsonObject& jsonObject) {
    valueMin = jsonObject["valueMin"].toDouble();
    valueMax = jsonObject["valueMax"].toDouble();
    proportionalGain = jsonObject["proportionalGain"].toDouble();
    integralGain = jsonObject["integralGain"].toDouble();
    derivativeGain = jsonObject["derivativeGain"].toDouble();
    preError = jsonObject["preError"].toDouble();
    integral = jsonObject["integral"].toDouble();
}