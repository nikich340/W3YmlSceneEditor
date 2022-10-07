#ifndef SHOTMANAGER_H
#define SHOTMANAGER_H

#include <QObject>
#include "YmlSceneManager.h"
#include "GraphicsActionRect.h"
#include <QColor>
#include "CustomRectItem.h"

class ShotManager : public QObject
{
	Q_OBJECT

private:
    YmlSceneManager* m_ymlManager = nullptr;
	dialogLink* currentDgLink = nullptr;
	QGraphicsScene* pDgScene = nullptr;
	QGraphicsScene* pLabelScene = nullptr;
	QGraphicsScene* pShotScene = nullptr;

    // labels
    const int m_groupsCount = 7;
    QHash<QString, QGraphicsTextItem*> labelItemByText;
    QGraphicsLineItem* m_navigateLine = nullptr;
    QMap<QString, CustomRectItem*> m_dgCueRectByShotname;  /* cue rect by shotname (edit on: load, clone, new, remove, move) */

    QMap<int, QVector<CustomRectItem*>> m_blocksByActorId;
    QMap<QString, QVector<CustomRectItem*>> m_blocksByShotName;
    QVector<int> m_knownActors;
    QVector<bool> m_isActorCollapsed;
    QVector<QVector<CustomRectItem*>> m_actorLabels;
    //const QPen cosmeticPen();

public:
    explicit ShotManager(YmlSceneManager* newYmlManager, QObject *parent = nullptr);
    void setShotScenes(QGraphicsScene* gDgScene, QGraphicsScene* gLabelScene, QGraphicsScene* gShotScene);
    bool eventFilter(QObject *obj, QEvent *event) override;

    bool isActorSpecificType(EShotActionType type);
	void clearShotEditor();
    void updateCueText(QString shotname);
    double getDurationForAction(shotAction* sa);
    int getGroupNumForType(EShotActionType type);

    double getMinYForAction(shotAction* action);
    int getActorIdForAction(shotAction* action);
    void addLabel(QString text, EShotActionType type, double offsetX, double offsetY);
    QColor getBlockColorForActionType(EShotActionType type);
signals:

public slots:
	void onLoadShots(QString sectionName);
    void onLoadShotAction(int shotNum, int actionNum);
    void onScaledView(double factor) {

    }
    void onActorAdd(int actorID);
    void onActorChange(int actorID);
    void onActorRemove(int actorID);
    void onActorCollapse(int actorID);
    void onActorUncollapse(int actorID);

    void onShotLoad(int shotNum);
    void onShotLoad(QString shotName);
    void onShotRename(QString oldName, QString newName);
    void onShotRemove(QString shotName);
};

#endif // SHOTMANAGER_H
