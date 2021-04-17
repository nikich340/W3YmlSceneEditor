#ifndef SHOTVIEWER_H
#define SHOTVIEWER_H

#include <QtWidgets>
#include "constants.h"

class ShotViewer : public QGraphicsView
{
	Q_OBJECT
private:
	//YmlSceneManager *ymlManager;
	QGraphicsView *labelView, *dgView;
	int m_originX, m_originY;

public:
	ShotViewer(QWidget *parent = nullptr);

	//void setYmlManager(YmlSceneManager* newManager);
	void setChildViews(QGraphicsView* newLabelView, QGraphicsView* newDgView);

protected:
#if QT_CONFIG(wheelevent)
	void wheelEvent(QWheelEvent *event) override;
#endif
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;

signals:
	void wasScaled(double factor);
};

#endif // SHOTVIEWER_H
