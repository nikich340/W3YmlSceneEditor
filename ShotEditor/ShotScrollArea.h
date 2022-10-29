#ifndef SHOTSCROLLAREA_H
#define SHOTSCROLLAREA_H

#include <QApplication>
#include <QScrollArea>
#include <QScrollBar>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QGraphicsSceneMouseEvent>
#include <QContextMenuEvent>
#include <QGraphicsView>
#include <QObject>
#include <QWidget>
#include <QVector>

class ShotScrollArea : public QScrollArea
{
    Q_OBJECT
    using super = QScrollArea;
private:
    QScrollArea* m_pLabelArea;
    QGraphicsView* m_pDialogView;
    QVector<QGraphicsView*> m_pShotViews;
    int m_originX, m_originY;
    bool m_inPress = false;

    qreal m_scaleFactor = 1.0;
    qreal m_lastHorizontalPos = 1.0;
    qint64 m_lastHorizontalAdjust = -1;

public:
    ShotScrollArea(QWidget *parent = nullptr);
    void addShotView(QGraphicsView* newShotView);
    void removeShotView(QGraphicsView* newShotView);
    void clearAllShotViews();
    void setChildViews(QScrollArea* newLabelArea, QGraphicsView* newDialogView);

    qreal scaleFactor() const;
    void resetScaleFactor();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
#if QT_CONFIG(wheelevent)
    void wheelEvent(QWheelEvent *event) override;
#endif
    void contextMenuEvent(QContextMenuEvent* event) override;

signals:
    void lineMoveEvent(QPointF scenePos);
    void contextEvent(QGraphicsScene* scene, QPoint screenPos);
public slots:
    void onUpdateHorizontalSlider();
};

#endif // SHOTSCROLLAREA_H
