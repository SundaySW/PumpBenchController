#ifndef SCENARIOSITEMCONTROL_H
#define SCENARIOSITEMCONTROL_H

#include <QGroupBox>

namespace Ui {
class ScenariosItemControl;
}

class ScenariosItemControl : public QGroupBox
{
    Q_OBJECT

public:
    explicit ScenariosItemControl(QWidget *parent = nullptr);
    ~ScenariosItemControl();

private:
    Ui::ScenariosItemControl *ui;
};

#endif // SCENARIOSITEMCONTROL_H
