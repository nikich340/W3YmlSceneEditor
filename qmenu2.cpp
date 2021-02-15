#include "qmenu2.h"
#include <QMouseEvent>
#include <QDebug>

QMenu2::QMenu2(QWidget *parent) : QMenu(parent)
{

}

void QMenu2::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit clicked();
        qDebug() << "Clicked!";
    }
    QMenu::mousePressEvent(event);
}
