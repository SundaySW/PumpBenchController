//
// Created by user on 17.05.2023.
//

#ifndef PUMPBENCHCONTROLLER_BENCH_VIEW_CTRL_ITEM_H
#define PUMPBENCHCONTROLLER_BENCH_VIEW_CTRL_ITEM_H

#include <QSlider>
#include <QLineEdit>
#include <QComboBox>
#include <optional>
#include "ParamService.h"
#include "bench_view_item.h"
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
    void loadDataFromJson(const QJsonObject&);
    QJsonObject SaveDataToJson();
    template<typename T>
    bool setRequestedValue(T val);
    ParamItem* getFeedBackParamRawPtr();
signals:
    void requestParamKeyByName(const QString&);
    void requestItemsList();
    void newFeedBackItem(const QString&);
public slots:
    void receiveItem(QSharedPointer<BenchViewItem>& item);
    void receiveItemsNameList(const QVector<BenchViewItem::ViewItemData> &data);
private:
    QString name;
    ParamService* paramService;
    BenchItemSettingsDlg* settingsDlg;
    QPushButton* configBtn;
    QPushButton* pidEnabledBtn;
    QPushButton* sendValueBtn;
    QLineEdit* set_item_value_edit_;
    QLineEdit* targetValueEdit;
    QComboBox* targetParamCombobox;
    QSlider* valueSlider;
    PIDControl pidControl;
    QSharedPointer<BenchViewItem> targetValueItem;
    std::optional<uchar> setParamId, setParamHost;
    double requestedValue, minSetValueBound, maxSetValueBound;
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
    void checkPIDTargetValue();
    bool isOKReceivedNewParam(const QSharedPointer<BenchViewItem> &item);
    void showMsgBox(const QString &msg);
    void managePIDStatus(bool state);

    void resetFeedBackItemComboBox(const QString& newName);
};

#endif //PUMPBENCHCONTROLLER_BENCH_VIEW_CTRL_ITEM_H