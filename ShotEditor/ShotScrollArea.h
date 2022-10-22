#ifndef SHOTSCROLLAREA_H
#define SHOTSCROLLAREA_H

#include <QApplication>
#include <QScrollArea>
#include <QScrollBar>
#include <QWheelEvent>
#include <QMouseEvent>
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
    qreal m_scaleFactor = 1.0;

public:
    ShotScrollArea(QWidget *parent = nullptr);
    void setChildViews(QScrollArea* newLabelArea, QGraphicsView* newDialogView);

protected:
#if QT_CONFIG(wheelevent)
    void wheelEvent(QWheelEvent *event) override;
#endif
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

signals:
    //void wasScaled(double factor);
    void addActionIntent(QPointF p);
};

#endif // SHOTSCROLLAREA_H
