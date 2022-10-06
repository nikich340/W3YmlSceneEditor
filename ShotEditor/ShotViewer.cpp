#include "ShotViewer.h"

ShotViewer::ShotViewer(QWidget *parent) : QGraphicsView(parent)
{
	setTransformationAnchor(QGraphicsView::NoAnchor);
	setResizeAnchor(QGraphicsView::NoAnchor);
}

void ShotViewer::setChildViews(QGraphicsView* newLabelView, QGraphicsView* newDgView) {
	if (newLabelView != nullptr) {
		labelView = newLabelView;
		labelView->setTransformationAnchor(QGraphicsView::NoAnchor);
		labelView->setResizeAnchor(QGraphicsView::NoAnchor);
		connect(this->verticalScrollBar(), SIGNAL(valueChanged(int)), labelView->verticalScrollBar(), SLOT(setValue(int)));
	}
	if (newDgView != nullptr) {
		dgView = newDgView;
		dgView->setTransformationAnchor(QGraphicsView::NoAnchor);
		dgView->setResizeAnchor(QGraphicsView::NoAnchor);
		connect(this->horizontalScrollBar(), SIGNAL(valueChanged(int)), dgView->horizontalScrollBar(), SLOT(setValue(int)));
	}
}

#if QT_CONFIG(wheelevent)
void ShotViewer::wheelEvent(QWheelEvent *event)
{
	if (event->modifiers() & Qt::ControlModifier) {
		qreal scaleOffset = (event->angleDelta().y() > 0 ? 1.05 : 1/1.05);
		qreal scaleFactor = transform().m11() * scaleOffset;
		//const ViewportAnchor anchor = transformationAnchor();
		//setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

		//qDebug() << scaleFactor;
		if (scaleFactor < 1.0 || scaleFactor > 10.0)
			return;

        /* full scale
        scale(scaleOffset, scaleOffset);
		dgView->scale(scaleOffset, 1.0);
        labelView->scale(1.0, scaleOffset);
        */
        /* x-only scale */
        scale(scaleOffset, 1.0);
        dgView->scale(scaleOffset, 1.0);
        labelView->scale(1.0, 1.0);

		emit wasScaled(scaleFactor);

		event->accept();
	} else {
		QGraphicsView::wheelEvent(event);
	}
}
#endif

void ShotViewer::mousePressEvent(QMouseEvent* event)
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
void SectionsGraphicsView::mouseDoubleClickEvent(QMouseEvent *event) {
	if (event->button() == Qt::LeftButton) {
		qDebug() << "Double click!";
		event->accept();
	} else {
		QGraphicsView::mouseDoubleClickEvent(event);
	}
}*/

void ShotViewer::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::MiddleButton)
	{
		m_originX = event->x();
		m_originY = event->y();
		QApplication::restoreOverrideCursor();
	}
	QGraphicsView::mouseReleaseEvent(event);
}

void ShotViewer::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::MiddleButton)
	{
		int diff_x = event->x() - m_originX;
		int diff_y = event->y() - m_originY;
        //qDebug() << "diff: " << diff_x << ", " << diff_y;
        translate(diff_x, diff_y);

        dgView->translate(diff_x, 0);
        labelView->translate(0, diff_y);
		m_originX = event->x();
		m_originY = event->y();
	}
	QGraphicsView::mouseMoveEvent(event);
}
