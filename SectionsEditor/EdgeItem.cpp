#include "EdgeItem.h"
#include "constants.h"

EdgeItem::EdgeItem(QGraphicsItem *parent) : QGraphicsPathItem(parent)
{
	setZValue(-1.0);
}

void EdgeItem::setStartPoint(QGraphicsEllipseItem *socket) {
    socketStart = socket;
    hasStart = true;
}
void EdgeItem::setEndPoint(QGraphicsEllipseItem *socket) {
    socketEnd = socket;
}
void EdgeItem::draw() {
    QPointF endPoint = socketEnd->scenePos() + QPointF(CONSTANTS::DIAMETER/2, CONSTANTS::DIAMETER/2);
    drawToPoint( endPoint );
}
void EdgeItem::setState(EdgeState state) {
    currentState = state;
    if (state == normal) {
        setPen( QPen(CONSTANTS::colorEdgeNormal) );
	} else if (state == highlight) {
        setPen( QPen(CONSTANTS::colorEdgeHighlight) );
	} else if (state == change) {
        setPen( QPen(CONSTANTS::colorEdgeChange) );
    }
}
void EdgeItem::drawToPoint(QPointF endPoint) {
    path = QPainterPath();

    QPointF startPoint = socketStart->scenePos() + QPointF(CONSTANTS::DIAMETER/2, CONSTANTS::DIAMETER/2);
    QPointF middlePoint = ( startPoint + endPoint ) / 2;
    QPointF startOffsetPoint = startPoint + QPointF(CONSTANTS::WIDTH/3, (middlePoint.y() - startPoint.y()) / 3);
    QPointF endOffsetPoint = endPoint - QPointF(CONSTANTS::WIDTH/3, (endPoint.y() - middlePoint.y()) / 3);

    path.moveTo( startPoint );
    path.quadTo( startOffsetPoint, middlePoint );
    path.quadTo( endOffsetPoint, endPoint );

    setPath(path);
}

void EdgeItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // TODO&
        qDebug() << "Pressed on Edge!!";
    }
}
