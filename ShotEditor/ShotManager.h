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
    YmlSceneManager* m_pYmlManager = nullptr;
    dialogLink* m_pDialogLink = nullptr;
	QGraphicsScene* pDgScene = nullptr;
	QGraphicsScene* pLabelScene = nullptr;
	QGraphicsScene* pShotScene = nullptr;

    // labels
    const int m_groupsActorNumMax = 7;
    const int m_groupsPropNumMax = 3;
    QHash<QString, QGraphicsTextItem*> labelItemByText;
    QGraphicsLineItem* m_pNavigationLine = nullptr;
    QMap<QString, CustomRectItem*> m_dgCueRectByShotname;  /* cue rect by shotname (edit on: load, clone, new, remove, move) */

    QMap<int, QVector<CustomRectItem*>> m_blocksByAssetID;
    QMap<QString, QVector<CustomRectItem*>> m_blocksByShotName;
    QVector<int> m_knownAssetsID;
    QVector<int> m_assetsStartY;
    QVector<int> m_assetsEndY;
    QVector<bool> m_isAssetCollapsed;
    bool m_isAssetProp(int idx) {
        return m_pYmlManager->sceneGlobals()->props.contains(m_knownAssetsID[idx]);
    }
    QVector<QVector<CustomRectItem*>> m_assetLabels;
    //const QPen cosmeticPen();

public:
    explicit ShotManager(YmlSceneManager* newYmlManager, QObject *parent = nullptr);
    void setShotScenes(QGraphicsScene* gDgScene, QGraphicsScene* gLabelScene, QGraphicsScene* gShotScene);
    bool eventFilter(QObject *obj, QEvent *event) override;

    bool isAssetSpecificType(EShotActionType type);
	void clearShotEditor();
    void updateCueText(QString shotname);
    double getDurationForAction(shotAction* sa);
    //int getGroupNumForType(EShotActionType type);

    double getMinYForAction(shotAction* action);
    QString shotNameByNum(int shotNum);
    void getShotInfoForPoint(QPoint point, int &shotNum, double &shotCoord, int &actorNum, int &groupNum);
    int getActorIdForAction(shotAction* action);
    void addLabel(QString text, EShotActionType type, double offsetX, double offsetY);
    QColor getBlockColorForActionType(EShotActionType type);
signals:

public slots:
	void onLoadShots(QString sectionName);
    void onScaledView(double factor) {

    }
    void onAssetLoad(int actorID);
    void onAssetChange(int actorID);
    void onAssetRemove(int actorID);
    void onAssetCollapse(int actorID);
    void onAssetUncollapse(int actorID);

    void onShotLoad(int shotNum);
    void onShotLoad(QString shotName);
    void onShotRename(QString oldName, QString newName);
    void onShotRemove(QString shotName);

    void onShotActionLoad(int shotNum, int actionNum);
    void onShotActionAdd(int shotNum, EShotActionType type);
    void onShotActionRemove(int shotNum, int actionNum);
    void onShotActionUpdate(int shotNum, int actionNum);
};

#endif // SHOTMANAGER_H
