#include "SocketItem.h"
#include "GraphicsSectionItem.h"
#include "constants.h"

SocketItem::SocketItem(QGraphicsItem *parent) : QGraphicsEllipseItem(parent)
{
    setAcceptHoverEvents(true);
}
SocketItem::SocketItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent) : QGraphicsEllipseItem(x, y, width, height, parent)
{
    //setFlags(QGraphicsItem::ItemIsMovable);
}
SocketItem::~SocketItem() {
    /*if ( !isInputSocket ) {
        deleteAllEdges();
    }*/
}
void SocketItem::deleteAllEdges() {
    // ALL DELETIONS MUST BE IN GraphicsSectionItem !!!
    for (int i = 0; i < edges.size(); ++i) {
        delete edges[i];
    }
    edges.clear();
}
void SocketItem::redrawAllEdges() {
    qDebug() << "redraw edges: " << edges.size();
    for (int i = 0; i < edges.size(); ++i) {
        edges[i]->draw();
    }
}

void SocketItem::addEdge(EdgeItem *newEdge) {
    edges.push_back(newEdge);
}
bool SocketItem::removeEdge(EdgeItem *edge) {
    for (int i = 0; i < edges.size(); ++i) {
        if (edges[i] == edge) {
            edges.remove(i);
            return true;
        }
    }
    return false;
}

void SocketItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (!edges.isEmpty() && isInputSocket) {
            qDebug() << "Pressed exitsing!";
            inMove = true;
            edges.back()->setState(EdgeItem::change);
        } else if (edges.isEmpty() && !isInputSocket) {
            qDebug() << "Pressed new!";
            inCreate = true;
            EdgeItem* newEdge = new EdgeItem;
            newEdge->setState(EdgeItem::change);
            newEdge->setStartPoint(this);
            scene()->addItem(newEdge);
            edges.push_back(newEdge);
        }
    } else {
        QGraphicsEllipseItem::mousePressEvent(event);
    }
}

void SocketItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (inMove || inCreate) {
        if (!edges.isEmpty()) {
            //qDebug() << "Move! " << event->scenePos();
            edges.back()->drawToPoint(event->scenePos());
        }
    } else {
        QGraphicsEllipseItem::mouseMoveEvent(event);
    }
}

void SocketItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    qDebug() << "Released! " << event->button();
    if (event->button() == Qt::LeftButton && (inMove || inCreate)) {
        if (inMove) {
            inMove = false;
            QPointF dropPos = event->scenePos();
            qDebug() << "dropPos 1: " << dropPos;

            // VERY dirty :D
            QGraphicsItem* dropItem = scene()->itemAt(dropPos, scene()->views().first()->transform());
            if (dropItem != nullptr && dropItem->data(0).toString() == "inputSocket") {
                if ( dropItem != this ) {
                    qDebug() << "Dropped at SOCKET!";
                    EdgeItem* edge = edges.back();
                    GraphicsSectionItem* curSection = qgraphicsitem_cast<GraphicsSectionItem*>( edge->socketStart->parentItem() );
                    GraphicsSectionItem* nextSection = qgraphicsitem_cast<GraphicsSectionItem*>( dropItem->parentItem() );
                    edges.pop_back();
                    curSection->addOutputEdge(nextSection, edge);
                } else {
                    edges.last()->setState(EdgeItem::normal);
                    edges.last()->draw();
                }
            } else {
                GraphicsSectionItem* curSection = qgraphicsitem_cast<GraphicsSectionItem*>( edges.last()->socketStart->parentItem() );
                if ( !curSection->removeOutputEdge(edges.last()) ) {
                    qDebug() << "Error while removing edge!";
                }
            }
        } else if (inCreate) {
            inCreate = false;
            QPointF dropPos = event->scenePos();
            qDebug() << "dropPos 2: " << dropPos;

            // VERY dirty :D
            QGraphicsItem* dropItem = scene()->itemAt(dropPos, scene()->views().first()->transform());
            if (dropItem != nullptr && dropItem != this && dropItem->data(0).toString() == "inputSocket") {
                qDebug() << "Dropped at SOCKET!";
                EdgeItem* edge = edges.last();

                GraphicsSectionItem* curSection = qgraphicsitem_cast<GraphicsSectionItem*>( parentItem() );
                GraphicsSectionItem* nextSection = qgraphicsitem_cast<GraphicsSectionItem*>( dropItem->parentItem() );
                curSection->addOutputEdge(nextSection, edge);
            } else {
                // nobody more knows about temp edge
                scene()->removeItem( edges.last() );
                delete edges.last();
                edges.pop_back();
            }
        }
        event->accept();
    } else {
        QGraphicsEllipseItem::mouseReleaseEvent(event);
    }
}

void SocketItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    qDebug() << "HOVER!";
    QGraphicsEllipseItem::hoverEnterEvent(event);
}
