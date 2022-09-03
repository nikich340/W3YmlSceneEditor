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
	textLabel->setBrush(QBrush(colorSocketLabel));
	textLabel->setFont( QFont("Arial", 7) );
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
	textLabel->setBrush(QBrush(colorSocketLabel));
	textLabel->setFont( QFont("Arial", 7) );
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

QVector<EdgeItem*> SocketItem::getEges() {
	return edges;
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
	//qDebug() << "call remove edge! my edges:" << edges;
	if (edges.isEmpty())
		return false;
	int idx = edges.indexOf(edge);
	if (idx != -1) {
		edges.remove(idx);
		return true;
	} else {
		return false;
	}
}

bool SocketItem::makeTop(EdgeItem* edge) {
	int idx = edges.indexOf(edge);
	if (idx != -1) {
		if (idx < edges.size() - 1)
			std::rotate(edges.begin(), edges.begin() + (idx + 1), edges.end());
		return true;
	} else {
		return false;
	}
}

void SocketItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (!edges.isEmpty() && isInputSocket) {
			//qDebug() << "Pressed in-socket with edge, start moving it.";
            inMove = true;
            edges.back()->setState(EdgeItem::change);
        } else if (edges.isEmpty() && !isInputSocket) {
			//qDebug() << "Pressed out-socket without edge, create new and start moving it.";
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
    if (event->button() == Qt::LeftButton && (inMove || inCreate)) {
        EdgeItem* edge = edges.last();
        SocketItem* socketStart = static_cast<SocketItem*>( edge->socketStart );
        socketStart->textLabel->hide();

        QPointF dropPos = event->scenePos();
        QGraphicsItem* dropItem = scene()->itemAt(dropPos, scene()->views().first()->transform());

        if (inMove) {
            inMove = false;

			if (dropItem == nullptr)
				return;

            QString key = dropItem->data(0).toString();
			//qDebug() << "Existing edge was dropped on some item with key {" << key << "}";
			if (key == "inputSocket" || key == "section") {
                GraphicsSectionItem* oldSection = qgraphicsitem_cast<GraphicsSectionItem*>( edge->socketEnd->parentItem() );
                GraphicsSectionItem* startSection = qgraphicsitem_cast<GraphicsSectionItem*>( socketStart->parentItem() );
                GraphicsSectionItem* nextSection = nullptr;
                if (key == "inputSocket")
                    nextSection = qgraphicsitem_cast<GraphicsSectionItem*>( dropItem->parentItem() );
                else
                    nextSection = qgraphicsitem_cast<GraphicsSectionItem*>( dropItem );

				if ( (key == "inputSocket" && (dropItem != this) && (oldSection != nextSection)) ||
					 (key == "section" && (oldSection != nextSection)) ) {
					//qInfo() << "Move edge out-section [" << oldSection->sName() << "] -> [" << nextSection->sName() << "]";
                    edges.pop_back();
                    oldSection->removeInputEdge(edge);
                    startSection->addOutputEdge(nextSection, false, edge);
                } else {
					//qDebug() << "Don't change edge, redraw it.";
                    edges.last()->setState(EdgeItem::normal);
                    edges.last()->draw();
                }
            } else {
                GraphicsSectionItem* startSection = qgraphicsitem_cast<GraphicsSectionItem*>( socketStart->parentItem() );
                if ( !startSection->removeOutputEdge(edges.last()) ) {
					qc << "Error while removing edge from [" << startSection->sName() << "]";
                }
            }
        } else if (inCreate) {
            inCreate = false;

			if (dropItem == nullptr)
				return;

            QString key = dropItem->data(0).toString();
			//qDebug() << "Existing edge was dropped on some item with key {" << key << "}";
			if (key == "inputSocket" || key == "section") {
                GraphicsSectionItem* startSection = qgraphicsitem_cast<GraphicsSectionItem*>( parentItem() );
                GraphicsSectionItem* nextSection = nullptr;
                if (key == "inputSocket")
                    nextSection = qgraphicsitem_cast<GraphicsSectionItem*>( dropItem->parentItem() );
                else
                    nextSection = qgraphicsitem_cast<GraphicsSectionItem*>( dropItem );
                qInfo() << "Add edge from [" << startSection->sName() << "] to [" << nextSection->sName() << "]";
				startSection->addOutputEdge(nextSection, false, edge);
            } else {
                // nobody more knows about temp edge
				qDebug() << "Don't change edge, delete it.";
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
			it->setState(EdgeItem::highlight);
        }
    } else {
		textLabel->setPos( scenePos() + QPointF(WIDTH / 10, 0) );
        textLabel->show();
		if (hasEdges()) {
			edges[0]->setState(EdgeItem::highlight);
		}
    }
    QGraphicsEllipseItem::hoverEnterEvent(event);
}
void SocketItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    if (isInputSocket) {
        for (auto it: edges) {
            SocketItem* begin = static_cast<SocketItem*>( it->socketStart );
            begin->textLabel->hide();
			it->setState(EdgeItem::normal);
        }
    } else {
        textLabel->hide();
		if (hasEdges()) {
			edges[0]->setState(EdgeItem::normal);
		}
    }
    QGraphicsEllipseItem::hoverLeaveEvent(event);
}
