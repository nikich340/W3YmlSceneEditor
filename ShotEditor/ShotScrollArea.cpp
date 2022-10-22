#include "QDebug"
#include "ShotScrollArea.h"

ShotScrollArea::ShotScrollArea(QWidget *parent) : super(parent)
{
    //setTransformationAnchor(QScrollArea::NoAnchor);
    //setResizeAnchor(QGraphicsView::NoAnchor);
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

#if QT_CONFIG(wheelevent)
void ShotScrollArea::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        qreal scale = (event->angleDelta().y() > 0 ? 1.05 : 1/1.05);
        //qreal scaleFactor = transform().m11() * scaleOffset;
        m_scaleFactor = m_scaleFactor * scale;

        //const ViewportAnchor anchor = transformationAnchor();
        //setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

        //qDebug() << scaleFactor;
        if (m_scaleFactor > 10.0 || m_scaleFactor < 0.2)
            return;

        /* full scale
        scale(scaleOffset, scaleOffset);
        dgView->scale(scaleOffset, 1.0);
        labelView->scale(1.0, scaleOffset);
        */

        /* x-only scale */
        for (int i = 0; i < m_pShotViews.count(); ++i) {
            m_pShotViews[i]->scale(m_scaleFactor, 1.0);
        }
        m_pDialogView->scale(m_scaleFactor, 1.0);
        //labelView->scale(1.0, 1.0);
        //emit wasScaled(scaleFactor);
        event->accept();
    } else {
        super::wheelEvent(event);
    }
}
#endif

void ShotScrollArea::mousePressEvent(QMouseEvent* event)
{
    qDebug() << "mousePressEvent: " << event->x() << ", " << event->y();
    if (event->buttons() & Qt::MiddleButton)
    {
        // Store original position.
        m_originX = event->x();
        m_originY = event->y();
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
        event->accept();
    }
    super::mousePressEvent(event);
}
/*
void ShotScrollArea::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        qDebug() << "Double click!";
        event->accept();
    } else {
        QGraphicsView::mouseDoubleClickEvent(event);
    }
}*/

void ShotScrollArea::mouseReleaseEvent(QMouseEvent* event)
{
    qDebug() << "mouseReleaseEvent: " << event->x() << ", " << event->y();
    if (event->buttons() & Qt::MiddleButton)
    {
        m_originX = event->x();
        m_originY = event->y();
        QApplication::restoreOverrideCursor();
        event->accept();
    }
    super::mouseReleaseEvent(event);
}

void ShotScrollArea::mouseMoveEvent(QMouseEvent* event)
{
    qDebug() << "mouseMoveEvent: " << event->x() << ", " << event->y();
    if (event->buttons() & Qt::MiddleButton)
    {
        int new_x = horizontalScrollBar()->value() + (event->x() - m_originX);
        int new_y = verticalScrollBar()->value() + (event->y() - m_originY);
        horizontalScrollBar()->setValue( new_x );
        verticalScrollBar()->setValue( new_y );
        m_pDialogView->horizontalScrollBar()->setValue( new_x );
        m_pLabelArea->verticalScrollBar()->setValue( new_y );

        m_originX = event->x();
        m_originY = event->y();
        event->accept();
    }
    super::mouseMoveEvent(event);
}

void ShotScrollArea::contextMenuEvent(QContextMenuEvent *event)
{
    /* TODO: make view-specific
    QPointF scenePos = this->mapToScene(event->pos());
    if (scene()->itemAt(scenePos, transform()) == nullptr) {
        QMenu menu;
        QAction *addAction = menu.addAction("Add action block");
        QAction *selectedAction = menu.exec( event->globalPos() );
        if (selectedAction == addAction) {
            event->accept();
            emit addActionIntent(scenePos);
        } else {
            event->ignore();
        }
    } else {
        QGraphicsView::contextMenuEvent(event);
    }*/
}
