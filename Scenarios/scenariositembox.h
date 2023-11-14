#ifndef SCENARIOSITEMBOX_H
#define SCENARIOSITEMBOX_H

#include <QGroupBox>
#include "bench_view_item.h"
#include "optional"

namespace Ui {
class ScenariosItemBox;
}

class ScenariosItemBox : public QGroupBox
{
    Q_OBJECT
public:
    explicit ScenariosItemBox(QWidget *parent = nullptr);
    ~ScenariosItemBox();
    void loadDataFromJson(const QJsonObject&);
    QJsonObject saveDataToJSon();
signals:
    void deleteMe();
    void requestItemByName(const QString&);
    void requestItemsList();
    void protosMsgToSend(const QString&);
public slots:
    void receiveItemsNameList(const QVector<BenchViewItem::ViewItemData> &data);
    void receiveItem(QSharedPointer<BenchViewItem> &item);
private:
    QSharedPointer<BenchViewItem> targetValueItem;
    Ui::ScenariosItemBox *ui;
    bool enabled = false;
    std::optional<double> targetValue;
    std::optional<QString> msgToSend;
    QRegExp msgMatch;
    void enableBtnClicked();
    void newTargetValueItem(const QString &itemName);
    bool isOKReceivedNewParam(const QSharedPointer<BenchViewItem> &item);
    void newTargetValueItemUpdate();
    void showMsgBox(const QString &msg);
    void targetValueEdited();
    void msgEdited();
};

#endif // SCENARIOSITEMBOX_H
