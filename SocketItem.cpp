#include "SocketItem.h"
#include "GraphicsSectionItem.h"
#include "constants.h"

SocketItem::SocketItem(QGraphicsItem *parent) : QGraphicsEllipseItem(parent)
{
    setAcceptHoverEvents(true);
    textLabel = new QGraphicsSimpleTextItem;
	// fuck Z-system
	textLabel->setZValue(1.0);
	scene()->addItem(textLabel);
	//textLabel->setParentItem(this);
    textLabel->hide(); // disable until hover/press
	textLabel->setBrush(QBrush(QColor(255, 51, 133)));
	textLabel->setFont( QFont("Arial", 8) );
}
SocketItem::SocketItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent) : QGraphicsEllipseItem(x, y, width, height, parent)
{
    setAcceptHoverEvents(true);
	textLabel = new QGraphicsSimpleTextItem;

	// fuck Z-system
	textLabel->setZValue(1.0);
	scene()->addItem(textLabel);
	//textLabel->setParentItem(this);
    textLabel->hide(); // disable until hover/press
    textLabel->setBrush(QBrush(QColor(255, 0, 102)));
	textLabel->setFont( QFont("Arial", 8) );
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
    for (int i = 0; i < edges.size(); ++i) {
        edges[i]->draw();
    }
}

EdgeItem* SocketItem::getLastEdge() {
	if (edges.isEmpty())
		return nullptr;
	else
		return edges.last();
}

void SocketItem::setLabel(QString text) {
    textLabel->setText(text);
}

bool SocketItem::hasEdges() {
    return !edges.isEmpty();
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
        EdgeItem* edge = edges.last();
        SocketItem* socketStart = static_cast<SocketItem*>( edge->socketStart );
        socketStart->textLabel->hide();

        QPointF dropPos = event->scenePos();
        QGraphicsItem* dropItem = scene()->itemAt(dropPos, scene()->views().first()->transform());

        if (inMove) {
            inMove = false;

            QString key = dropItem->data(0).toString();
            if (dropItem != nullptr && (key == "inputSocket" || key == "section")) {
                GraphicsSectionItem* oldSection = qgraphicsitem_cast<GraphicsSectionItem*>( edge->socketEnd->parentItem() );
                GraphicsSectionItem* startSection = qgraphicsitem_cast<GraphicsSectionItem*>( socketStart->parentItem() );
                GraphicsSectionItem* nextSection = nullptr;
                if (key == "inputSocket")
                    nextSection = qgraphicsitem_cast<GraphicsSectionItem*>( dropItem->parentItem() );
                else
                    nextSection = qgraphicsitem_cast<GraphicsSectionItem*>( dropItem );

                if ( (key == "inputSocket" && dropItem != this) ||
                     (key == "section" && oldSection != nextSection) ) {
                    qDebug() << "Dropped at SOCKET!";
                    edges.pop_back();
                    oldSection->removeInputEdge(edge);
                    startSection->addOutputEdge(nextSection, false, edge);
                } else {
                    edges.last()->setState(EdgeItem::normal);
                    edges.last()->draw();
                }
            } else {
                GraphicsSectionItem* startSection = qgraphicsitem_cast<GraphicsSectionItem*>( socketStart->parentItem() );
                if ( !startSection->removeOutputEdge(edges.last()) ) {
                    qDebug() << "Error while removing edge!";
                }
            }
        } else if (inCreate) {
            inCreate = false;

            QString key = dropItem->data(0).toString();
            if (dropItem != nullptr && (key == "inputSocket" || key == "section")) {
                qDebug() << "Dropped at SOCKET!";

                GraphicsSectionItem* startSection = qgraphicsitem_cast<GraphicsSectionItem*>( parentItem() );
                GraphicsSectionItem* nextSection = nullptr;
                if (key == "inputSocket")
                    nextSection = qgraphicsitem_cast<GraphicsSectionItem*>( dropItem->parentItem() );
                else
                    nextSection = qgraphicsitem_cast<GraphicsSectionItem*>( dropItem );
                startSection->addOutputEdge(nextSection, false, edge);
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
    if (isInputSocket) {
        for (auto it: edges) {
            SocketItem* begin = static_cast<SocketItem*>( it->socketStart );
			begin->textLabel->setPos( begin->scenePos() + QPointF(WIDTH / 10, 0) );
            begin->textLabel->show();
        }
    } else {
		textLabel->setPos( scenePos() + QPointF(WIDTH / 10, 0) );
        textLabel->show();
    }
    QGraphicsEllipseItem::hoverEnterEvent(event);
}
void SocketItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    if (isInputSocket) {
        for (auto it: edges) {
            SocketItem* begin = static_cast<SocketItem*>( it->socketStart );
            begin->textLabel->hide();
        }
    } else {
        textLabel->hide();
    }
    QGraphicsEllipseItem::hoverLeaveEvent(event);
}
