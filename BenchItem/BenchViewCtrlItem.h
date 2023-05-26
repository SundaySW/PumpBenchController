//
// Created by user on 17.05.2023.
//

#ifndef PUMPBENCHCONTROLLER_BENCHVIEWCTRLITEM_H
#define PUMPBENCHCONTROLLER_BENCHVIEWCTRLITEM_H

#include <QSlider>
#include <QLineEdit>
#include <QComboBox>
#include <optional>
#include "ParamService.h"
#include "BenchViewItem.h"
#include "QColor"
#include "QPushButton"
#include "PID/PIDControl.h"

class BenchViewCtrlItem: public QObject{
    Q_OBJECT
public:
    BenchViewCtrlItem(QString name, ParamService* ps, QPushButton* cfgBtn,
                      QLineEdit* ive, QLineEdit* tve,
                      QPushButton* pid, QPushButton* send,
                      QComboBox* tpc,QSlider* vs,
                      QWidget *parent = nullptr);
    void loadTargetItems(const QStringList &values);
signals:
    void requestParamKeyByName(const QString&);
    void requestItemsList();
public slots:
    void receiveItem(QSharedPointer<BenchViewItem>& item);
    void receiveItemsNameList(const QStringList&);
private:
    QString name, setParamMapKey;
    ParamService* paramService;
    BenchItemSettingsDlg* settingsDlg;
    QPushButton* configBtn;
    QPushButton* pidEnabledBtn;
    QPushButton* sendValueBtn;
    QLineEdit* itemValueEdit;
    QLineEdit* targetValueEdit;
    QComboBox* targetParamCombobox;
    QSlider* valueSlider;
    PIDControl pidControl;
    QSharedPointer<BenchViewItem> targetValueItem;
    std::optional<uchar> setParamId, setParamHost;
    int requestedValue, minSetValueBound, maxSetValueBound;
    std::optional<double> pidTargetValue;
    bool pidEnabled = false;
    void sendValue();
    void textValueEdited();
    void sliderMoved(int position);
    void pidButtonClicked();
    void targetValueChanged();
    void newTargetValueItem(const QString &mapKey);
        template<class T>
    bool checkValue(T val);
    void newTargetValueItemUpdate();
    void configBtnClicked();
        template<typename T>
        bool setRequestedValue(T val);
    void checkPIDTargetValue();
    bool isOKReceivedNewParam(const QSharedPointer<BenchViewItem> &item);
    static void showMsgBox(const QString &msg);
    void managePIDStatus(bool state);
};

#endif //PUMPBENCHCONTROLLER_BENCHVIEWCTRLITEM_H