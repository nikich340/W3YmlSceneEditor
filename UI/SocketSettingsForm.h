#ifndef SOCKETSETTINGSFORM_H
#define SOCKETSETTINGSFORM_H

#include <QWidget>

namespace Ui {
class SocketSettingsForm;
}

class SocketSettingsForm : public QWidget
{
    Q_OBJECT

public:
    explicit SocketSettingsForm(QWidget *parent = nullptr);
    ~SocketSettingsForm();

private:
    Ui::SocketSettingsForm *ui;
};

#endif // SOCKETSETTINGSFORM_H
