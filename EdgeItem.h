#pragma once
#ifndef EDGEITEM_H

#define EDGEITEM_H
#include <QtWidgets>

class EdgeItem : public QGraphicsPathItem
{
public:
    enum EdgeState { normal, loop, change };
    bool hasStart = false;

    QGraphicsEllipseItem *socketStart = nullptr, *socketEnd = nullptr;
    EdgeItem(QGraphicsItem *parent = nullptr);
    void setState(EdgeState state);
    EdgeState getState();
    void setStartPoint(QGraphicsEllipseItem* socket);
    void setEndPoint(QGraphicsEllipseItem* socket);
    void draw();
    void drawToPoint(QPointF endPoint);
    void destroy();

private:
    QGraphicsSimpleTextItem* text;
    QString sectionNameStart, sectionNameEnd;
    QPainterPath path;
    EdgeState currentState;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif // EDGEITEM_H
