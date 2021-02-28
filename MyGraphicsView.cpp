#include "MyGraphicsView.h"

MyGraphicsView::MyGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);

    setMouseTracking(true);
    viewport()->setMouseTracking(true);
    //viewport()->setAttribute(Qt::WA_Hover);
}

void MyGraphicsView::setYmlManager(YmlSceneManager *manager) {
	ymlManager = manager;
}

#if QT_CONFIG(wheelevent)
void MyGraphicsView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        qreal scaleOffset = (event->angleDelta().y() > 0 ? 1.08 : 0.92);
        qreal scaleFactor = transform().m11() * scaleOffset;
        const ViewportAnchor anchor = transformationAnchor();
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        //qDebug() << scaleFactor;
        if (scaleFactor < 0.1 || scaleFactor > 10)
            return;

        scale(scaleOffset, scaleOffset);
        setTransformationAnchor(anchor);

        event->accept();
    } else {
        QGraphicsView::wheelEvent(event);
    }
}
#endif

void MyGraphicsView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton)
    {
        // Store original position.
        m_originX = event->x();
        m_originY = event->y();
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
    }
    QGraphicsView::mousePressEvent(event);
}
/*
void MyGraphicsView::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        qDebug() << "Double click!";
        event->accept();
    } else {
        QGraphicsView::mouseDoubleClickEvent(event);
    }
}*/

void MyGraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton)
    {
        // Store original position.
        m_originX = event->x();
        m_originY = event->y();
        QApplication::restoreOverrideCursor();
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void MyGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::MiddleButton)
    {
        //qDebug() << "move x:" << event->x() - m_originX << " y:" << event->y() - m_originY;
        translate(event->x() - m_originX , event->y() - m_originY);
        m_originX = event->x();
        m_originY = event->y();
    }
    QGraphicsView::mouseMoveEvent(event);
}

void MyGraphicsView::contextMenuEvent(QContextMenuEvent *event) {
	QPointF scenePos = this->mapToScene(event->pos());
	if (scene()->itemAt(scenePos, transform()) == nullptr) {
		QMenu menu;
		QAction *editAction = menu.addAction("New section");
		QAction *selectedAction = menu.exec( event->globalPos() );
		if (selectedAction == nullptr) {
			event->ignore();
		} else if (selectedAction == editAction) {
			event->accept();
			ymlManager->addSectionLink( scenePos );
		}
	} else {
		QGraphicsView::contextMenuEvent(event);
	}
}
