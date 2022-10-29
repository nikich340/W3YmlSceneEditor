#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include "ShotScrollArea.h"
#define hSB horizontalScrollBar

qreal ShotScrollArea::scaleFactor() const
{
    return m_scaleFactor;
}

void ShotScrollArea::resetScaleFactor()
{
    m_scaleFactor = 1.0;
}

ShotScrollArea::ShotScrollArea(QWidget *parent) : super(parent)
{
    //setTransformationAnchor(QScrollArea::NoAnchor);
    //setResizeAnchor(QGraphicsView::NoAnchor);
}

void ShotScrollArea::addShotView(QGraphicsView* newShotView)
{
    newShotView->scale(m_scaleFactor, 1.0);
    m_pShotViews.append(newShotView);
}

void ShotScrollArea::removeShotView(QGraphicsView* newShotView)
{
    m_pShotViews.removeOne(newShotView);
}

void ShotScrollArea::clearAllShotViews()
{
    m_pShotViews.clear();
}

void ShotScrollArea::setChildViews(QScrollArea *newLabelArea, QGraphicsView *newDialogView)
{
    if (newLabelArea != nullptr) {
        m_pLabelArea = newLabelArea;
        //labelView->setTransformationAnchor(QGraphicsView::NoAnchor);
        //labelView->setResizeAnchor(QGraphicsView::NoAnchor);
        connect(this->verticalScrollBar(), SIGNAL(valueChanged(int)), m_pLabelArea->verticalScrollBar(), SLOT(setValue(int)));
    }
    if (newDialogView != nullptr) {
        m_pDialogView = newDialogView;
        m_pDialogView->setTransformationAnchor(QGraphicsView::NoAnchor);
        m_pDialogView->setResizeAnchor(QGraphicsView::NoAnchor);
        connect(this->horizontalScrollBar(), SIGNAL(valueChanged(int)), m_pDialogView->horizontalScrollBar(), SLOT(setValue(int)));
    }
}

bool ShotScrollArea::eventFilter(QObject *watched, QEvent *event)
{
    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
    QGraphicsScene* pScene = static_cast<QGraphicsScene*>(watched);
    qDebug() << "EVENT: " << event->type();
    if (pScene != nullptr && mouseEvent != nullptr) {
        if (event->type() == QEvent::GraphicsSceneMousePress && !m_inPress && mouseEvent->buttons() & Qt::MiddleButton) {
            m_originX = mouseEvent->screenPos().x();
            m_originY = mouseEvent->screenPos().y();
            m_inPress = true;
            QApplication::setOverrideCursor(Qt::ClosedHandCursor);
            event->accept();
            return true;
        } else if (event->type() == QEvent::GraphicsSceneMouseMove) {
            emit lineMoveEvent(mouseEvent->scenePos());

            if (m_inPress && mouseEvent->buttons() & Qt::MiddleButton) {
                //qDebug() << "hor max: " << horizontalScrollBar()->maximum() << ", vert max: " << verticalScrollBar()->maximum();
                int dx = mouseEvent->screenPos().x() - m_originX;
                int dy = mouseEvent->screenPos().y() - m_originY;
                int new_x = qMin(horizontalScrollBar()->maximum(), qMax(0, horizontalScrollBar()->value() - dx));
                int new_y = qMin(verticalScrollBar()->maximum(), qMax(0, verticalScrollBar()->value() - dy));

                horizontalScrollBar()->setValue( new_x );
                verticalScrollBar()->setValue( new_y );
                m_pDialogView->horizontalScrollBar()->setValue( new_x );
                m_pLabelArea->verticalScrollBar()->setValue( new_y );

                m_originX = mouseEvent->screenPos().x();
                m_originY = mouseEvent->screenPos().y();
                event->accept();
                return true;
            }
        } else if (event->type() == QEvent::GraphicsSceneMouseRelease) {
            m_originX = mouseEvent->screenPos().x();
            m_originY = mouseEvent->screenPos().y();
            m_inPress = false;
            QApplication::restoreOverrideCursor();
            event->accept();
            return true;
        } else if (event->type() == QEvent::GraphicsSceneContextMenu) {
            emit contextEvent(pScene, mouseEvent->screenPos());
        }
    }
    return false; // -> handled by children
}

void ShotScrollArea::contextMenuEvent(QContextMenuEvent *event)
{
    qDebug() << "contextMenuEvent!";
}

#if QT_CONFIG(wheelevent)
void ShotScrollArea::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        qreal scaleOffset = (event->angleDelta().y() > 0 ? 1.1 : 1/1.1);
        //qDebug() << "m_scaleFactor: " << m_scaleFactor;
        if (m_scaleFactor * scaleOffset > 10.0 || m_scaleFactor * scaleOffset < 1.0)
            return;

        m_scaleFactor = m_scaleFactor * scaleOffset;
        /* x-only scale */
        // small hack to *try* to current area in center
        if (QDateTime::currentDateTime().toMSecsSinceEpoch() - m_lastHorizontalAdjust > 50) {
            if (hSB()->maximum() - hSB()->minimum() > 0) {
                double range = (hSB()->maximum() - hSB()->minimum());
                m_lastHorizontalPos = hSB()->value() * 1.0 / range;
            }
        }
        m_pDialogView->scale(scaleOffset, 1.0);
        for (int i = 0; i < m_pShotViews.count(); ++i) {
            if (m_pShotViews[i] == nullptr)
                continue;
            m_pShotViews[i]->scale(scaleOffset, 1.0);
            m_pShotViews[i]->setFixedWidth( m_pShotViews[i]->width() * scaleOffset );
        }

        QTimer::singleShot(5, this, SLOT(onUpdateHorizontalSlider()));
        event->accept();
    } else {
        super::wheelEvent(event);
    }
}
#endif

void ShotScrollArea::onUpdateHorizontalSlider()
{
    // small hack to *try* to current area in center
    m_lastHorizontalAdjust = QDateTime::currentDateTime().toMSecsSinceEpoch();
    if (hSB()->maximum() - hSB()->minimum() > 0) {
        double range = (hSB()->maximum() - hSB()->minimum());
        hSB()->setValue( range * m_lastHorizontalPos );
    }
}
