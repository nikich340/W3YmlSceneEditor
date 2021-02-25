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
	YmlSceneManager* ymlManager = nullptr;

public:
	MyGraphicsView(QWidget *parent = nullptr);
	void setYmlManager(YmlSceneManager* manager);

protected:
#if QT_CONFIG(wheelevent)
    void wheelEvent(QWheelEvent *event) override;
#endif
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
	void contextMenuEvent(QContextMenuEvent *event) override;
    //void mouseDoubleClickEvent(QMouseEvent *event) override;
};

#endif // MYGRAPHICSVIEW_H
