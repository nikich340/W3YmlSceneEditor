#ifndef GRAPHICSACTIONRECT_H
#define GRAPHICSACTIONRECT_H

#include <QtWidgets>
#include "constants.h"
#include "YmlStructs.h"

class GraphicsActionRect : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
    using super = QGraphicsRectItem;
private:
    Qt::Edges m_edges = 0;
    QGraphicsTextItem* m_pLabel = nullptr;
    double min_X = 0, max_X = 100000; // for start position only
    double min_Y = 0, max_Y = 100000;
    double m_moveY = 0;
public:
    Qt::Edges edgesAt(const QPointF& point, const QRectF& rect, qreal width);
    GraphicsActionRect(QGraphicsItem *parent = nullptr);
    void setDefaults();
    void setBorders(double minX, double maxX, double minY, double maxY) {
        min_X = minX;
        max_X = maxX;
        min_Y = minY;
        max_Y = maxY;
    }
    void setLabel(QString text);
    void setYLimits(double minY, double maxY);

protected:
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;
};

#endif // GRAPHICSACTIONRECT_H
