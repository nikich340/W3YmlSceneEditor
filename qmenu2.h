#include <QMenu>

#ifndef QMENU2_H
#define QMENU2_H


class QMenu2 : public QMenu
{
    Q_OBJECT
public:
    QMenu2(QWidget *parent);

signals:
    void clicked();

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
};

#endif // QMENU2_H
