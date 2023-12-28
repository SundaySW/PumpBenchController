#ifndef KEYPAD_H
#define KEYPAD_H

#include <QDockWidget>
#include "KeyPadBtn.hpp"
#include "QMap"

namespace Ui {
class KeyPad;
}

class KeyPad : public QDockWidget
{
    Q_OBJECT
public:
    explicit KeyPad(QWidget *parent = nullptr);
    ~KeyPad();
    void LoadDataFromJson(const QJsonObject&);
    QJsonObject SaveDataToJson();
signals:
    void protosMsgToSend(const QString&);
private:
    Ui::KeyPad *ui;
    const int k_column_n_ = 2;
    int column_ = 0;
    int row_ = 0;
    QMap<KeyPadBtn*, QSharedPointer<KeyPadBtn>> btnMap;
    void AddNewBtn();
    void RightClicked(KeyPadBtn *btn);
    void AddBtnToGrid(KeyPadBtn *btn);

    void LoadBtn(QJsonObject &jsonObject);

    void ReMapGrid();
};

#endif // KEYPAD_H
