#ifndef SCENARIOSDOCK_H
#define SCENARIOSDOCK_H

#include <QDockWidget>
#include "scenariositembox.h"
#include "scenariositemcontrol.h"

namespace Ui {
class ScenariosDock;
}

class ScenariosDock : public QDockWidget
{
    Q_OBJECT
public:
    explicit ScenariosDock(QWidget *parent = nullptr);
    ~ScenariosDock();
    void loadDataFromJson(const QJsonObject&);
    QJsonObject SaveDataToJson();
signals:
    void reqNewItemFromScenario(const QString&, ScenariosItemBox*);
    void reqViewItemsList(ScenariosItemBox*);
    void protosMsgToSend(const QString&);
private:
    int scenarioCount = 0;
    Ui::ScenariosDock *ui;
    QMap<ScenariosItemBox*, QSharedPointer<ScenariosItemBox>> msgItemsMap;
    QList<QSharedPointer<ScenariosItemControl>> setItemsList;
    ScenariosItemBox* addMsgItem();
};

#endif // SCENARIOSDOCK_H
