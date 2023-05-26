#include "benchitemsettingsdlg.h"

#include <utility>
#include <QMessageBox>
#include "ui_benchitemsettingsdlg.h"
#include "colors.h"

BenchItemSettingsDlg::~BenchItemSettingsDlg()
{
    delete ui;
}

BenchItemSettingsDlg::BenchItemSettingsDlg(const QString& name, ParamService* ps, bool updateParam, QWidget *parent):
    paramService(ps),
    QDialog(parent),
    itemName(name),
    ui(new Ui::BenchItemSettingsDlg)
{
    ui->setupUi(this);
    ui->itemName_label->setText(itemName.toUpper());
    ui->itemName_label->setStyleSheet(QString("color: %1;").arg(default_color_hex));
    updateParam ? ui->setParamSettings_groupBox->setEnabled(false) : ui->updateParamSettings_groupBox->setEnabled(false);
    updateParamConnections();
    setParamConnections();
    paramSettingsConnection();
    pidValuesConnection();
}

void BenchItemSettingsDlg::updateParamConnections(){
    ui->lowerBound_lineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]+")));
    ui->upperBound_lineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]+")));
    connect(ui->refresh_button, &QPushButton::clicked, [this](){ refreshParamList(); });
    connect(ui->updateParam_comboBox, &QComboBox::currentTextChanged, [this](const QString& s){ loadNewParam(s);});
    connect(ui->lowerBound_lineEdit, &QLineEdit::editingFinished, [this](){
        bool okMin,okMax;
        double newMinValue = ui->lowerBound_lineEdit->text().toDouble(&okMax);
        double newMaxValue = ui->upperBound_lineEdit->text().toDouble(&okMin);
        if(okMin && okMax)
                emit newUpdateValueBounds(qMakePair(newMinValue, newMaxValue));
    });
    connect(ui->upperBound_lineEdit, &QLineEdit::editingFinished, [this](){
        bool okMin,okMax;
        double newMinValue = ui->lowerBound_lineEdit->text().toDouble(&okMax);
        double newMaxValue = ui->upperBound_lineEdit->text().toDouble(&okMin);
        if(okMin && okMax)
                emit newUpdateValueBounds(qMakePair(newMinValue, newMaxValue));
    });
}

void BenchItemSettingsDlg::setParamConnections(){
    ui->setParamID_lineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9a-fA-F]{1,2}")));
    ui->setParamHost_lineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9a-fA-F]{1,2}")));
    ui->setValueMin_lineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]+")));
    ui->setValueMax_lineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]+")));
    connect(ui->setParamHost_lineEdit, &QLineEdit::editingFinished, [this](){
        short newHost = ui->setParamHost_lineEdit->text().toShort(nullptr,16);
        short ID = ui->setParamID_lineEdit->text().toShort(nullptr,16);
        newSetParamHost(newHost);
        newSetParamKey(ParamService::makeMapKey(newHost, ID));
    });
    connect(ui->setParamID_lineEdit, &QLineEdit::editingFinished, [this](){
        short newID = ui->setParamID_lineEdit->text().toShort(nullptr,16);
        short host = ui->setParamHost_lineEdit->text().toShort(nullptr,16);
        newSetParamId(newID);
        newSetParamKey(ParamService::makeMapKey(host, newID));
    });
    connect(ui->setValueMin_lineEdit, &QLineEdit::editingFinished, [this](){
        bool okMin,okMax;
        int newMinValue = ui->setValueMin_lineEdit->text().toInt(&okMin);
        int maxValue = ui->setValueMax_lineEdit->text().toInt(&okMax);
        if(okMin && okMax)
            emit newSetValueBounds(qMakePair(newMinValue, maxValue));
    });
    connect(ui->setValueMax_lineEdit, &QLineEdit::editingFinished, [this](){
        bool okMin,okMax;
        int minValue = ui->setValueMin_lineEdit->text().toInt(&okMin);
        int newMaxValue = ui->setValueMax_lineEdit->text().toInt(&okMax);
        if(okMin && okMax)
            emit newSetValueBounds(qMakePair(minValue, newMaxValue));
    });
}

void BenchItemSettingsDlg::paramSettingsConnection(){
    ui->offset_value_lineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]+")));
    ui->mult_value_lineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]+")));
    ui->updateRate_value_lineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]+")));
    ui->controlRate_value_lineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]+")));
    ui->sendRate_value_lineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]+")));
    connect(ui->getCalib_button, &QPushButton::clicked, [this](){ getCalib(); });
    connect(ui->getRates_button, &QPushButton::clicked, [this](){ getRates(); });
    connect(ui->sendCalib_button, &QPushButton::clicked, [this](){ setCalib(); });
    connect(ui->sendRates_button, &QPushButton::clicked, [this](){ setRates(); });
}

void BenchItemSettingsDlg::pidValuesConnection(){
    ui->pidMin_lineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]+.[0-9]+")));
    ui->pidMax_lineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]+.[0-9]+")));
    ui->pidKp_lineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]+.[0-9]+")));
    ui->pidKi_lineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]+.[0-9]+")));
    ui->pidKd_lineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]+.[0-9]+")));
    connect(ui->pidKd_lineEdit, &QLineEdit::editingFinished, [this](){
        bool ok;
        auto newValue = ui->pidKd_lineEdit->text().toDouble(&ok);
        if(ok)pidSettings.Kd = newValue;
        emit newPIDSettigs(pidSettings);
    });
    connect(ui->pidKi_lineEdit, &QLineEdit::editingFinished, [this](){
        bool ok;
        auto newValue = ui->pidKi_lineEdit->text().toDouble(&ok);
        if(ok)pidSettings.Ki = newValue;
        emit newPIDSettigs(pidSettings);
    });
    connect(ui->pidKp_lineEdit, &QLineEdit::editingFinished, [this](){
        bool ok;
        auto newValue = ui->pidKp_lineEdit->text().toDouble(&ok);
        if(ok)pidSettings.Kp = newValue;
        emit newPIDSettigs(pidSettings);
    });
    connect(ui->pidMin_lineEdit, &QLineEdit::editingFinished, [this](){
        bool ok;
        auto newValue = ui->pidMin_lineEdit->text().toDouble(&ok);
        if(ok)pidSettings.min = newValue;
        emit newPIDSettigs(pidSettings);
    });
    connect(ui->pidMax_lineEdit, &QLineEdit::editingFinished, [this](){
        bool ok;
        auto newValue = ui->pidMax_lineEdit->text().toDouble(&ok);
        if(ok)pidSettings.max = newValue;
        emit newPIDSettigs(pidSettings);
    });
}

void BenchItemSettingsDlg::refreshParamList(){
    ui->updateParam_comboBox->clear();
    ui->updateParam_comboBox->addItems(paramService->getAllParamsStrList());
}

void BenchItemSettingsDlg::loadNewParam(const QString& mapKey){
    paramItem = paramService->getParam(mapKey);
    if (paramItem.get()){
        connect(paramItem.get(), &ParamItem::paramRatesChanged, [this](uchar t, short v) { setActualValues(t, v); });
        connect(paramItem.get(), &ParamItem::paramCalibDataChanged,
                [this](uchar t, double v) { setActualValues(t, v); });
        getRates();
        emit itemParamChanged(paramItem);
    }
}

void BenchItemSettingsDlg::getCalib(){
    if(paramItem.isNull())
        return;
    unsetActualValue(ui->offset_actualValue_label);
    unsetActualValue(ui->mult_actualValue_label);
    paramService->sendServiceMsgReq(paramItem.get(), ProtosMessage::OFFSET);
    paramService->sendServiceMsgReq(paramItem.get(), ProtosMessage::MULT);
}

void BenchItemSettingsDlg::getRates(){
    if(paramItem.isNull())
        return;
    unsetActualValue(ui->updateRate_actualValue_label);
    unsetActualValue(ui->sendRate_actualValue_label);
    unsetActualValue(ui->controlRate_actualValue_label);
//        param.getParamType() == UPDATE ? makeMsg(ProtosMessage::UPDATE_RATE) : makeMsg(ProtosMessage::CTRL_RATE);
    paramService->sendServiceMsgReq(paramItem.get(), ProtosMessage::UPDATE_RATE);
    paramService->sendServiceMsgReq(paramItem.get(), ProtosMessage::CTRL_RATE);
    paramService->sendServiceMsgReq(paramItem.get(), ProtosMessage::SEND_RATE);
}

void BenchItemSettingsDlg::setCalib(){
    auto offsetText = ui->offset_value_lineEdit->text();
    auto multText = ui->mult_value_lineEdit->text();
    bool ok;
    if(offsetText.length()){
        float offsetValue = offsetText.toFloat(&ok);
        if(ok) paramService->sendServiceMsgSet(paramItem.get(), offsetValue, ProtosMessage::OFFSET);
        else QMessageBox::information(this, "Wrong Offset Value", "Should be float");
    }
    if(multText.length()){
        float multValue = multText.toFloat(&ok);
        if(ok) paramService->sendServiceMsgSet(paramItem.get(), multValue, ProtosMessage::MULT);
        else QMessageBox::information(this, "Wrong Mult value", "Should be float");
    }
}
void BenchItemSettingsDlg::setRates(){
    auto updateRateText = ui->updateRate_value_lineEdit->text();
    auto controlRateText = ui->controlRate_value_lineEdit->text();
    auto sendRateText = ui->sendRate_value_lineEdit->text();
    bool ok;
    if(updateRateText.length()){
        short updateRateValue = updateRateText.toShort(&ok);
//            if(ok) param.getParamType() == UPDATE ?
//                makeMsg(ProtosMessage::UPDATE_RATE, updateRateValue, true)
//                    : makeMsg(ProtosMessage::CTRL_RATE, updateRateValue, true);
        if(ok) paramService->sendServiceMsgSet(paramItem.get(), updateRateValue, ProtosMessage::UPDATE_RATE);
        else QMessageBox::information(this, "Wrong Update Rate Value", "Should be short");
    }
    if(controlRateText.length()){
        short controlRateValue = controlRateText.toShort(&ok);
        if(ok) paramService->sendServiceMsgSet(paramItem.get(), controlRateValue, ProtosMessage::CTRL_RATE);
        else QMessageBox::information(this, "Wrong Control Rate value", "Should be short");
    }
    if(sendRateText.length()){
        short sendRateValue = sendRateText.toShort(&ok);
        if(ok) paramService->sendServiceMsgSet(paramItem.get(), sendRateValue, ProtosMessage::SEND_RATE);
        else QMessageBox::information(this, "Wrong Send Rate value", "Should be short");
    }
}

template<class T>
void BenchItemSettingsDlg::setActualValues(uchar paramField, T value) {
    auto newStringValue = QString("%1").arg(value);
    switch (paramField) {
        case ProtosMessage::CTRL_RATE:
            setLabelActualValue(ui->controlRate_actualValue_label, newStringValue);
        case ProtosMessage::UPDATE_RATE:
            setLabelActualValue(ui->updateRate_actualValue_label, newStringValue);
            break;
        case ProtosMessage::SEND_RATE:
            setLabelActualValue(ui->sendRate_actualValue_label, newStringValue);
            break;
        case ProtosMessage::MULT:
            setLabelActualValue(ui->mult_actualValue_label, newStringValue);
            break;
        case ProtosMessage::OFFSET:
            setLabelActualValue(ui->offset_actualValue_label, newStringValue);
            break;
        default:
            break;
    }
}

void BenchItemSettingsDlg::setLabelActualValue(QLabel* label, const QString& value){
    label->setText(value);
    label->setStyleSheet("color: mediumaquamarine; font-weight: bold;");
}

void BenchItemSettingsDlg::unsetActualValue(QLabel* label){
    label->clear();
    label->setStyleSheet("color: bisque;font-weight: normal;");
}

void BenchItemSettingsDlg::setPidBoundsOfNewItem(std::tuple<double, double> bounds) {
    auto[min, max] = bounds;
    ui->pidMin_lineEdit->setText(QString("%1").arg(min));
    ui->pidMax_lineEdit->setText(QString("%1").arg(max));
}
