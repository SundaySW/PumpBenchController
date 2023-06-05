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
signals:
    void reqNewItemFromScenario(const QString&, ScenariosItemBox*);
    void reqViewItemsList(ScenariosItemBox*);
    void protosMsgToSend(const QString&);
private:
    Ui::ScenariosDock *ui;
    QList<QSharedPointer<ScenariosItemBox>> msgItemsList;
    QList<QSharedPointer<ScenariosItemControl>> setItemsList;
    void addMsgItem();
};

#endif // SCENARIOSDOCK_H
