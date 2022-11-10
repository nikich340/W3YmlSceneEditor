#ifndef SHOTMANAGER_H
#define SHOTMANAGER_H

#include <QObject>
#include <QColor>
#include "QResizableStackedWidget.h"
#include "ShotScrollArea.h"
#include "YmlSceneManager.h"
#include "CustomRectItem.h"
#include "YmlShotActions.h"
#include "constants.h"

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

    bool isCollapsed;
    QHash<QString, QSet<CustomRectItem*>> actionRectsByShotName;
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
            pNavigationLine->setData(CONSTANTS::IsNavigationLine, true);
            pNavigationLine->setZValue(1.0);
            pNavigationLine->setPen( QPen(Qt::red) );
            pNavigationLine->setPos(0, 0);
            pNavigationLine->setFlag(QGraphicsItem::ItemIgnoresTransformations);
            pScene->addItem(pNavigationLine);
        }
        pNavigationLine->setLine(0, 0, 0, pScene->height());
    }
    void clearAll() {
        pVerticalLines.clear();
        pHorizontalLines.clear();
        actionRectsByShotName.clear();
        labelRects.clear();
        if (pScene != nullptr) {
            pScene->clear();
            pScene->deleteLater();
        }
        if (pView != nullptr)
            pView->deleteLater();

        if (pSceneLabel != nullptr) {
            pSceneLabel->clear();
            pSceneLabel->deleteLater();
        }
        if (pViewLabel != nullptr)
            pViewLabel->deleteLater();
    }
};

class ShotManager : public QObject
{
	Q_OBJECT

private:
    // general
    YmlSceneManager* m_pYmlManager = nullptr;
    dialogLink* m_pDialogLink = nullptr;
    QResizableStackedWidget* m_pEditorStackedWidget = nullptr;
    QString m_sectionName;
    int m_sectionType;
    QPixmap m_pixShow, m_pixHide;

    QGraphicsScene* m_pDialogScene      = nullptr;
    QScrollArea* m_pShotLabelArea = nullptr;
    QWidget* m_pShotLabelWidget = nullptr;
    ShotScrollArea* m_pShotArea = nullptr;
    QWidget* m_pShotWidget = nullptr;

    // labels
    const int m_groupsActorNumMax = 7;
    const int m_groupsPropNumMax = 3;
    QHash<QString, CustomRectItem*>     m_dialogCueRectByShotname;  /* cue rect by shotname (edit on: load, clone, new, remove, move) */
    QVector<QGraphicsSimpleTextItem*>   m_pDialogSecondNumbers;

    QVector<ShotAsset*>                 m_pAssets;
    QHash<int, ShotAsset*>              m_pAssetByID;
    QHash<QGraphicsScene*, int>         m_assetIDByScene;
    CustomRectItem*                     m_pSelectedShotAction = nullptr;

    QLinearGradient createGradient(QColor startColor, QColor endColor, double W, double H);
    double sceneWidth() {
        return (m_pDialogLink->totalDuration + 30.0) * CONSTANTS::SHOT_SECOND;
    }
    double sceneHeight() {
        return (2 + m_pYmlManager->sceneGlobals()->actors.count() * m_groupsActorNumMax + m_pYmlManager->sceneGlobals()->props.count() * m_groupsPropNumMax);
    }
    //const QPen cosmeticPen();

public:
    explicit ShotManager(YmlSceneManager* newYmlManager, QObject *parent = nullptr);
    ~ShotManager();
    void setWidgets(QGraphicsScene* newDialogScene, QResizableStackedWidget* newEditorStackedWidget, QScrollArea* newShotLabelArea, ShotScrollArea* newShotArea);
    //bool eventFilter(QObject *obj, QEvent *event) override;

    bool isAssetSpecificType(EShotActionType type);
	void clearShotEditor();
    void updateDialogCueText(QString shotname);
    //double getDurationForAction(SA_Base* sa);
    double X_TO_SEC(double x) {
        return x / CONSTANTS::SHOT_SECOND;
    }
    double SEC_TO_X(double x) {
        return x * CONSTANTS::SHOT_SECOND;
    }
    int Y_TO_GroupNum(double y) {
        return y / CONSTANTS::SHOT_LABEL_HEIGHT;
    }
    int X_TO_ShotNum(double x);
    double X_TO_ShotPos(double x);  // [0.0 - 1.0)
    double ShotPos_TO_X(int shotNum, double shotPoint);
    double ShotNum_TO_Xstart(int shotNum);
    //int getGroupNumForType(EShotActionType type);

    double getMinYForAction(SA_Base* action);
    QString shotNameByNum(int shotNum);
    void getShotInfoForPoint(QPoint point, int &shotNum, double &shotCoord, int &actorNum, int &groupNum);
    void addLabel(QString text, EShotActionType type, double offsetX, double offsetY);
    static QColor colorForActionType(EShotActionType type);

    void updateHorizontalAdvance();
    void updateVerticalAdvance();
signals:

public slots:
    void onClearEditor();
    void onLoadSectionShots(QString sectionName);
    void onUpdateSectionName(QString oldSectionName, QString newSectionName);
    void onUpdateSectionType(QString sectionName, int newType);
    void onRepaintSecondNumbers();
    void onRepaintHorizontalLines();
    void onRepaintHorizontalLinesForAssetID(int assetID);
    void onRepaintVerticalLines();
    void onRepaintVerticalLinesForAssetID(int assetID);
    void onNavigationLineMove(QPointF scenePos);

    void onAssetLoad(int assetID);
    void onAssetChange(int assetID);
    void onAssetRemove(int assetID);
    void onAssetCollapse(bool isCollapsed);

    void onSceneContextEvent(QGraphicsScene* pScene, QPoint screenPos, QPointF scenePos);
    void onShotContextEvent(QPointF screenPos);
    void onShotAdd(int newShotNum);
    void onShotLoad(int shotNum);
    void onShotLoad(QString shotName);
    void onShotRename(QString oldName, QString newName);
    void onShotRemove(QString shotName);

    void onShotActionContextEvent(QPointF screenPos);
    void onShotActionStartChanged(double newStart);
    void onShotActionPositionChanged(QPointF newScenePos);
    void onShotActionChanged();
    void onShotActionSelected(CustomRectItem* pRect);
    void onShotActionLoad(int shotNum, SA_Base* pSA);
    void onShotActionAdd(int shotNum, int assetID, EShotActionType actionType, double shotPoint);
    void onShotActionRemove(CustomRectItem* rect, bool updateYML = true);
    //void onShotActionUpdate(CustomRectItem* rect, bool updateYML = true);
};

#endif // SHOTMANAGER_H
