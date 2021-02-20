#ifndef DIALOGCHANGESECTION_H
#define DIALOGCHANGESECTION_H

#include <QDialog>

namespace Ui {
class DialogChangeSection;
}

class DialogChangeSection : public QDialog
{
    Q_OBJECT

public:
    explicit DialogChangeSection(QWidget *parent = nullptr);
    ~DialogChangeSection();

private:
    Ui::DialogChangeSection *ui;
};

#endif // DIALOGCHANGESECTION_H
