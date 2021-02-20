#pragma once
#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QObject>
#include <QtWidgets>
#include "GraphicsSectionItem.h"

class MyGraphicsView : public QGraphicsView
{
private:
    int m_originX, m_originY;

public:
    MyGraphicsView(QWidget *parent = nullptr);

protected:
#if QT_CONFIG(wheelevent)
    void wheelEvent(QWheelEvent *event) override;
#endif
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    //void mouseDoubleClickEvent(QMouseEvent *event) override;
};

#endif // MYGRAPHICSVIEW_H
