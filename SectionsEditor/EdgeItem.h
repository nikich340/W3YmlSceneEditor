#pragma once
#ifndef EDGEITEM_H

#define EDGEITEM_H
#include <QtWidgets>

class EdgeItem : public QGraphicsPathItem
{
public:
	enum EdgeState { normal, loop, highlight, change };
    bool hasStart = false;

    QGraphicsEllipseItem *socketStart = nullptr, *socketEnd = nullptr;
    EdgeItem(QGraphicsItem *parent = nullptr);
    void setState(EdgeState state);
    void setStartPoint(QGraphicsEllipseItem* socket);
    void setEndPoint(QGraphicsEllipseItem* socket);
    void draw();
    void drawToPoint(QPointF endPoint);
    void destroy();

private:
    QString sectionNameStart, sectionNameEnd;
    QPainterPath path;
    EdgeState currentState = normal;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif // EDGEITEM_H
