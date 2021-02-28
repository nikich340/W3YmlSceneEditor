#ifndef GRAPHICSSOCKETITEM_H
#define GRAPHICSSOCKETITEM_H
#include <QtWidgets>
#include "EdgeItem.h"

class SocketItem : public QGraphicsEllipseItem
{
    //Q_OBJECT
private:
	QVector<EdgeItem*> edges;
    bool inMove = false;
    bool inCreate = false;

public:
    QGraphicsSimpleTextItem* textLabel;

    bool isInputSocket = false;
    bool hasEdges();
    void addEdge(EdgeItem* newEdge);
    bool removeEdge(EdgeItem* edge);
	EdgeItem* getLastEdge();
	QVector<EdgeItem*> getEges();
    void setLabel(QString text);
    void redrawAllEdges();
    void deleteAllEdges();
    SocketItem(QGraphicsItem *parent = nullptr);
    ~SocketItem() override;
    SocketItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = nullptr);
    enum { Type = QGraphicsEllipseItem::UserType + 1 };
    int type() const override { return Type; }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
};

#endif // GRAPHICSSOCKETITEM_H
