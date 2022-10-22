#ifndef SHOTMANAGER_H
#define SHOTMANAGER_H

#include <QObject>
#include "YmlSceneManager.h"
#include "GraphicsActionRect.h"
#include <QColor>
#include "CustomRectItem.h"

struct ShotAsset {
    QGraphicsScene* pScene = nullptr;
    QGraphicsView* pView = nullptr;
    QGraphicsScene* pSceneLabel = nullptr;
    QGraphicsView* pViewLabel = nullptr;

    QGraphicsLineItem* pNavigationLine = nullptr;
    QVector<QGraphicsLineItem*> pVerticalLines = QVector<QGraphicsLineItem*>();
    QVector<QGraphicsLineItem*> pHorizontalLines = QVector<QGraphicsLineItem*>();
    QString assetName;
    int assetID = -1;
    bool isProp = false;
    //int startY, endY;
    bool isCollapsed;
    QSet<CustomRectItem*> actionRects;
    QVector<CustomRectItem*> labelRects;
    ShotAsset(QGraphicsView* newView, QGraphicsScene* newScene, QGraphicsView* newViewLabel, QGraphicsScene* newSceneLabel, int newID) {
        pView = newView;
        pScene = newScene;
        pViewLabel = newViewLabel;
        pSceneLabel = newSceneLabel;
        assetID = newID;
    }
    void repaintNavigationLine() {
        if (pNavigationLine == nullptr) {
            pNavigationLine = new QGraphicsLineItem;
            pNavigationLine->setZValue(1.0);
            pNavigationLine->setPen( QPen(Qt::red) );
            pNavigationLine->setPos(0, 0);
            pNavigationLine->setFlag(QGraphicsItem::ItemIgnoresTransformations);
            pScene->addItem(pNavigationLine);
        }
        pNavigationLine->setLine(0, 0, 0, pScene->height());
    }
};

class ShotManager : public QObject
{
	Q_OBJECT

private:
    // general
    YmlSceneManager* m_pYmlManager = nullptr;
    dialogLink* m_pDialogLink = nullptr;

    QGraphicsScene* m_pDialogScene      = nullptr;
    QScrollArea* m_pShotLabelArea = nullptr;
    QWidget* m_pShotLabelWidget = nullptr;
    QScrollArea* m_pShotArea = nullptr;
    QWidget* m_pShotWidget = nullptr;

    QVector<QGraphicsSimpleTextItem*>   m_pDialogSecondNumbers;

    // labels
    QLinearGradient m_LightYellowGradient;
    const int m_groupsActorNumMax = 7;
    const int m_groupsPropNumMax = 3;
    QHash<QString, CustomRectItem*> m_dialogCueRectByShotname;  /* cue rect by shotname (edit on: load, clone, new, remove, move) */

    //QMap<int, QVector<CustomRectItem*>> m_blocksByAssetID;
    //QVector<> m_assetLabels;
    QVector<ShotAsset*>                 m_pAssets;
    QHash<int, ShotAsset*>              m_pAssetByID;
    QHash<QGraphicsScene*, ShotAsset*>  m_pAssetByScene;
    QHash<QString, QSet<CustomRectItem*>> m_blocksByShotName;

    double sceneWidth() {
        return qMax(SHOT_SCENE_WIDTH, m_pDialogLink->totalDuration * 2.0 * SHOT_SECOND);
    }
    double sceneHeight() {
        return qMax(SHOT_SCENE_HEIGHT, SHOT_LABEL_HEIGHT * (2 + m_pYmlManager->sceneGlobals()->actors.count() * m_groupsActorNumMax + m_pYmlManager->sceneGlobals()->props.count() * m_groupsPropNumMax));
    }
    //const QPen cosmeticPen();

public:
    explicit ShotManager(YmlSceneManager* newYmlManager, QObject *parent = nullptr);
    void setWidgets(QGraphicsScene* newDialogScene, QScrollArea* newShotLabelArea, QScrollArea* newShotArea);
    bool eventFilter(QObject *obj, QEvent *event) override;

    bool isAssetSpecificType(EShotActionType type);
	void clearShotEditor();
    void updateDialogCueText(QString shotname);
    double getDurationForAction(shotAction* sa);
    double X_TO_SEC(double x) {
        return x / SHOT_SECOND;
    }
    double SEC_TO_X(double x) {
        return x * SHOT_SECOND;
    }
    //int getGroupNumForType(EShotActionType type);

    double getMinYForAction(shotAction* action);
    QString shotNameByNum(int shotNum);
    void getShotInfoForPoint(QPoint point, int &shotNum, double &shotCoord, int &actorNum, int &groupNum);
    int getAssetIDForAction(shotAction* action);
    void addLabel(QString text, EShotActionType type, double offsetX, double offsetY);
    QColor getBlockColorForActionType(EShotActionType type);
signals:

public slots:
	void onLoadShots(QString sectionName);
    void onRepaintSecondNumbers();
    void onRepaintVerticalLines();
    void onRepaintVerticalLinesForAssetID(int assetID);
    void onScaledView(double factor) {}

    void onAssetLoad(int assetID);
    void onAssetChange(int assetID);
    void onAssetRemove(int assetID);
    void onAssetCollapse(bool isCollapsed);

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
