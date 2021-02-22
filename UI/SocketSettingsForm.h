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
    void setupMe(QStringList sectionsList);

    Ui::SocketSettingsForm *ui;

public slots:
    void onChoseAction(QString);
    void onClickedCondCheck(bool);
};

#endif // SOCKETSETTINGSFORM_H
