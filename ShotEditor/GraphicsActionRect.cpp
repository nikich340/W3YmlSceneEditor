#include "GraphicsActionRect.h"

GraphicsActionRect::GraphicsActionRect(QGraphicsItem *parent) : QGraphicsRectItem(parent)
{
    setDefaults();
    m_pLabel = new QGraphicsTextItem();
    //this->setData(0, "actionRect");
}
void GraphicsActionRect::setDefaults() {
    setAcceptHoverEvents(true);
    setRect( QRectF(0, 0, SHOT_LABEL_WIDTH / 5, SHOT_DG_HEIGHT / 2) );
    QPen comseticPen(Qt::black);
    comseticPen.setCosmetic(true);
    setPen( comseticPen );

    //setBrush( QBrush(QColor(128, 255, 170)) );
    //setBrush( QBrush(colorShotCam) );
    setFlags( QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable );
}
void GraphicsActionRect::setLabel(QString text) {
    if (m_pLabel == nullptr) {
        m_pLabel = new QGraphicsTextItem("<->");
        m_pLabel->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        m_pLabel->setDefaultTextColor( Qt::black );
        QFont f("Arial");
        f.setPixelSize(14);
        m_pLabel->setData(0, "actionLabel"); // hack for edge drag event
        m_pLabel->setFont(f);
        m_pLabel->setParentItem(this);
    }

    // Wrap text to newlines
    int last = 0;
    QString newText = "";
    QVector<int> cPos, insPos;
    for (int i = 0; i < text.length(); ++i) {
        if (text[i] == '_')
            cPos.push_back(i);
    }
    cPos.push_back(text.length() + 1);
    for (int i = 0; i < cPos.size() - 1; ++i) {
        if (cPos[i + 1] - last > 16) {
            insPos.push_back(cPos[i]);
            last = cPos[i];
        }
    }
    last = 0;
    for (int i = 0; i < text.length(); ++i) {
        newText.push_back(text[i]);
        if (last < insPos.size() && i == insPos[last]) {
            if (last > 2) {
                newText.push_back("...");
                break;
            }
            newText.push_back('\n');
            ++last;
        }
    }

    qreal labelH = 0;
    if (last == 0)
        labelH = SHOT_DG_HEIGHT / 3.5;
    else if (last == 1)
        labelH = SHOT_DG_HEIGHT / 6;

    m_pLabel->setPos( SHOT_LABEL_WIDTH / 20, labelH );
    m_pLabel->setPlainText(newText);
}

void GraphicsActionRect::setYLimits(double minY, double maxY) {
    min_Y = minY;
    max_Y = qMax(min_Y + rect().height(), maxY);
    double dy = 0;
    if ( scenePos().y() < minY ) {
        dy = minY - scenePos().y();
    } else if ( scenePos().y() + rect().height() > maxY ) {
        dy = scenePos().y() + rect().height() - maxY;
    }

    if (dy != 0) {
        moveBy(0, dy);
    }
}

Qt::Edges GraphicsActionRect::edgesAt(const QPointF &p, const QRectF &r, qreal w) {
    Qt::Edges edges;
    auto hw = w / 2.0;
    if (QRectF(r.x()-hw, r.y()-hw, w, r.height()+w).contains(p)) edges |= Qt::LeftEdge;
    if (QRectF(r.x()+r.width()-hw, r.y()-hw, w, r.height()+w).contains(p)) edges |= Qt::RightEdge;
    if (QRectF(r.x()-hw, r.y()-hw, r.width()+w, w).contains(p)) edges |= Qt::TopEdge;
    if (QRectF(r.x()-hw, r.y()+r.height()-hw, r.width()+w, w).contains(p)) edges |= Qt::BottomEdge;
    return edges;
}
void GraphicsActionRect::hoverMoveEvent(QGraphicsSceneHoverEvent *event) {
    m_edges = edgesAt(event->pos(), rect(), rect().width() / 10);
    if (m_edges) {
        if (m_edges & (Qt::LeftEdge | Qt::RightEdge))
            QApplication::setOverrideCursor(Qt::SizeHorCursor);
        else
            QApplication::setOverrideCursor(Qt::SizeVerCursor);
    } else {
        QApplication::restoreOverrideCursor();
    }
    event->ignore();
}
void GraphicsActionRect::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {

    } else {
        super::mouseDoubleClickEvent(event);
    }
}

QVariant GraphicsActionRect::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) {
    if (change == ItemPositionChange && scene())
    {
        QPointF sceneVec = scenePos() - pos();
        QPointF newScenePos = value.toPointF() - sceneVec;

        double y = qMin( qMax(newScenePos.y(), min_Y), max_Y );
        newScenePos.setY( y );
        newScenePos = newScenePos + sceneVec;
        return newScenePos;
    }
    return super::itemChange(change, value);
}
void GraphicsActionRect::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    /*QPointF evPos = event->pos();
    double diffY = event->pos() - m_moveY;
    diffY = qMax(diffY, min_Y - scenePos().y());
    diffY = qMin(diffY, scenePos().y() - max_Y);
    event->setPos( event->pos().x(), ev );

    m_moveY = event->pos();*/

    super::mouseMoveEvent(event);
}
void GraphicsActionRect::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    m_moveY = event->pos().y();
    super::mousePressEvent(event);
}
void GraphicsActionRect::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    super::mouseReleaseEvent(event);
}

void GraphicsActionRect::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    QMenu menu;
    QAction *adjustStartAction = menu.addAction("Adjust event start");
    QAction *adjustEndAction = menu.addAction("Adjust event end");
    QAction *deleteAction = menu.addAction("Delete");
    QAction *selectedAction = menu.exec(event->screenPos());
    if (selectedAction == adjustStartAction) {
        event->accept();
    } else if (selectedAction == adjustEndAction) {
        event->accept();
    } else if (selectedAction == deleteAction) {
        event->accept();
    } else {
        event->ignore();
    }
}
