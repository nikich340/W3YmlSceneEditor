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
    SocketItem(QGraphicsItem *parent = nullptr);
    ~SocketItem();
    SocketItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = nullptr);
    QGraphicsSimpleTextItem* textLabel;

    bool isInputSocket = false;
    bool hasEdges();
    void addEdge(EdgeItem* newEdge);
    bool removeEdge(EdgeItem* edge);
	bool makeTop(EdgeItem* edge);
	EdgeItem* getLastEdge();
	QVector<EdgeItem*> getEges();
    void setLabel(QString text);
    void redrawAllEdges();

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
