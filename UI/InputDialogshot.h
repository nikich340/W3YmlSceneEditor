#ifndef INPUTDIALOGSHOT_H
#define INPUTDIALOGSHOT_H

#include <QWidget>

namespace Ui {
class InputDialogshot;
}

class InputDialogshot : public QWidget
{
    Q_OBJECT

public:
    explicit InputDialogshot(QWidget *parent = nullptr);
    ~InputDialogshot();

private:
    Ui::InputDialogshot *ui;
};

#endif // INPUTDIALOGSHOT_H
