#ifndef DIALOGSELECTENV_H
#define DIALOGSELECTENV_H

#include <QDialog>
#include "StringStorage.h"

namespace Ui {
class DialogSelectEnv;
}

class DialogSelectEnv : public QDialog
{
    Q_OBJECT

public:
    using super = QDialog;
    explicit DialogSelectEnv(QWidget *parent = nullptr);
    ~DialogSelectEnv();
    void setStorage(StringStorage* newStorage);

protected:
    virtual void accept();

private:
    const int FILTER_MAX = 300;
    StringStorage* m_pStorage;
    Ui::DialogSelectEnv *ui;

public slots:
    void onFilterChanged(QString text);

signals:
    void envSelected(QString);
};

#endif // DIALOGSELECTENV_H
