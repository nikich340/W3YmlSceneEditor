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
    const QHash< EShotActionType, QColor > actionColors = {
        { EShotCam,                 QColor("#FF5D5D")   },
        { EShotCamBlendStart,       QColor("#FF5D5D")   },
        { EShotCamBlendKey,         QColor("#FF5D5D")   },
        { EShotCamBlendEnd,         QColor("#FF5D5D")   },
        { EShotCamBlendTogame,      QColor("#CF4D4D")   },
        { EShotActorAnim,           QColor("#5DBDFF")   },
        { EShotActorAnimAdditive,   QColor("#5DFAFF")   },
        { EShotActorAnimPose,       QColor("#5D5DFF")   },
        { EShotActorMimicAnim,      QColor("#5DFFBF")   },
        { EShotActorMimicPose,      QColor("#5DFF62")   },
        { EShotActorPlacement,      QColor("#FF5DE2")   },
        { EShotActorPlacementStart, QColor("#FF5DE2")   },
        { EShotActorPlacementKey,   QColor("#FF5DE2")   },
        { EShotActorPlacementEnd,   QColor("#FF5DE2")   },
        { EShotActorGamestate,      QColor("#FF5D5D")   },
        { EShotActorLookat,         QColor("#BF5DFF")   },
        { EShotActorScabbardShow,   QColor("#FF5D5D")   },
        { EShotActorScabbardHide,   QColor("#ADADAD")   },
        { EShotActorShow,           QColor("#FF5D5D")   },
        { EShotActorHide,           QColor("#ADADAD")   },
        { EShotActorEffectStart,    QColor("#F8FF5D")   },
        { EShotActorEffectStop,     QColor("#ADADAD")   },
        { EShotActorSound,          QColor("#FFC25D")   },
        { EShotActorEquipRight,     QColor("#FF5D5D")   },
        { EShotActorEquipLeft,      QColor("#FF5D5D")   },
        { EShotActorUnequipRight,   QColor("#FF5D5D")   },
        { EShotActorUnequipLeft,    QColor("#FF5D5D")   },
        { EShotPropShow,            QColor("#FF5D5D")   },
        { EShotPropHide,            QColor("#ADADAD")   },
        { EShotPropPlacement,       QColor("#FF5DE2")   },
        { EShotPropPlacementStart,  QColor("#FF5DE2")   },
        { EShotPropPlacementKey,    QColor("#FF5DE2")   },
        { EShotPropPlacementEnd,    QColor("#FF5DE2")   },
        { EShotPropEffectStart,     QColor("#F8FF5D")   },
        { EShotPropEffectStop,      QColor("#ADADAD")   },
        { EShotEnvBlendIn,          QColor("#5DD5FF")   },
        { EShotEnvBlendOut,         QColor("#5DD5FF")   },
        { EShotFadeIn,              QColor("#5DD5FF")   },
        { EShotFadeOut,             QColor("#5DD5FF")   },
        { EShotWorldAddfact,        QColor("#5DD5FF")   },
        { EShotWorldWeather,        QColor("#5DD5FF")   },
        { EShotWorldEffectStart,    QColor("#5DD5FF")   },
        { EShotWorldEffectStop,     QColor("#5DD5FF")   }
    };
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
