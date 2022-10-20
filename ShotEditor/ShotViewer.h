#ifndef SHOTVIEWER_H
#define SHOTVIEWER_H

#include <QtWidgets>
#include "constants.h"

class ShotViewer : public QGraphicsView
{
	Q_OBJECT
private:
	QGraphicsView *labelView, *dgView;
	int m_originX, m_originY;

public:
	ShotViewer(QWidget *parent = nullptr);

	void setChildViews(QGraphicsView* newLabelView, QGraphicsView* newDgView);

protected:
#if QT_CONFIG(wheelevent)
	void wheelEvent(QWheelEvent *event) override;
#endif
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

signals:
	void wasScaled(double factor);
    void addActionIntent(QPointF p);
};

#endif // SHOTVIEWER_H
