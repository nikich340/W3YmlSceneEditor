#ifndef SHOTMANAGER_H
#define SHOTMANAGER_H

#include <QObject>
#include "YmlSceneManager.h"

class ShotManager : public QObject
{
	Q_OBJECT

private:
	YmlSceneManager* ymlManager = nullptr;
	dialogLink* currentDgLink = nullptr;
	QGraphicsScene* pDgScene = nullptr;
	QGraphicsScene* pLabelScene = nullptr;
	QGraphicsScene* pShotScene = nullptr;

	// elements
	QGraphicsLineItem* navigateLine = nullptr;
	QMap<QString, QGraphicsRectItem*> dgElementByShotname;
	QMap<QString, QGraphicsTextItem*> dgLabelByShotname;
	QVector<QString> assignedSpeakers;

public:
	explicit ShotManager(YmlSceneManager* newYmlManager, QObject *parent = nullptr);
	void setShotScenes(QGraphicsScene* gDgScene, QGraphicsScene* gLabelScene, QGraphicsScene* gShotScene);
	bool eventFilter(QObject *obj, QEvent *event) override;
	void clearShotEditor();
	void updateDgLabel(QString shotname, double factor = 1.0);

signals:

public slots:
	void onLoadShots(QString sectionName);
	void onScaledView(double factor);
};

#endif // SHOTMANAGER_H
