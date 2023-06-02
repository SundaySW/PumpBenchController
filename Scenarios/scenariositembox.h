#ifndef SCENARIOSITEMBOX_H
#define SCENARIOSITEMBOX_H

#include <QGroupBox>

namespace Ui {
class ScenariosItemBox;
}

class ScenariosItemBox : public QGroupBox
{
    Q_OBJECT
public:
    explicit ScenariosItemBox(QWidget *parent = nullptr);
    ~ScenariosItemBox();
signals:
    void deleteMe();
private:
    Ui::ScenariosItemBox *ui;
};

#endif // SCENARIOSITEMBOX_H
