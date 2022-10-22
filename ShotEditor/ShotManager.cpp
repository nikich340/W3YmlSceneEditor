#include <QRandomGenerator>
#include "ShotManager.h"

ShotManager::ShotManager(YmlSceneManager* newYmlManager, QObject *parent) : QObject(parent)
{
    m_pYmlManager = newYmlManager;

    //m_LightYellowGradient = QLinearGradient(QGradient::CloudyKnoxville);
    m_LightYellowGradient.setStart(0, 0);
    m_LightYellowGradient.setFinalStop(SEC_TO_X(30), SHOT_LABEL_HEIGHT * m_groupsActorNumMax);
    m_LightYellowGradient.setCoordinateMode(QGradient::LogicalMode);
    m_LightYellowGradient.setColorAt(0, QColor("#eeeeec"));
    m_LightYellowGradient.setColorAt(1.0, QColor("#e9e9e7"));
}

void ShotManager::setWidgets(QGraphicsScene *newDialogScene, QScrollArea *newShotLabelArea, QScrollArea *newShotArea)
{
    m_pDialogScene = newDialogScene;
    m_pShotLabelArea = newShotLabelArea;
    m_pShotArea = newShotArea;
    m_pShotLabelWidget = newShotLabelArea->widget();
    m_pShotWidget = newShotArea->widget();

    m_pShotLabelArea->setFixedWidth(SHOT_LABEL_WIDTH);
    /* one-time constant UI things - shared scene */
    //m_pShotSceneShared->views()[0]->setMouseTracking(true);
    //m_pShotSceneShared->installEventFilter(this);
}

bool ShotManager::eventFilter(QObject *obj, QEvent *event) {
	//qDebug() << "event:" << event->type();
    if (event->type() == QEvent::GraphicsSceneMouseMove) {
		QGraphicsSceneMouseEvent* hoverEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
        upn(i, 0, m_pAssets.count() - 1) {
            if (m_pAssets[i]->pNavigationLine != nullptr)
            m_pAssets[i]->pNavigationLine->setPos( hoverEvent->scenePos().x(), 0 );
        }
	}
    return false;
}

bool ShotManager::isAssetSpecificType(EShotActionType type)
{
    return !NonAssetEShotActions.contains(type);
}

void ShotManager::clearShotEditor() {
    m_pDialogScene->clear();
    m_dialogCueRectByShotname.clear();
    m_blocksByShotName.clear();
    m_pDialogSecondNumbers.clear();

    upn(i, 0, m_pAssets.count() - 1) {
        m_pShotLabelWidget->layout()->removeWidget(m_pAssets[i]->pViewLabel);
        m_pAssets[i]->pViewLabel->deleteLater();
        m_pAssets[i]->pSceneLabel->deleteLater();
        m_pShotWidget->layout()->removeWidget(m_pAssets[i]->pView);
        m_pAssets[i]->pView->deleteLater();
        m_pAssets[i]->pScene->deleteLater();
        delete m_pAssets[i];
    }
    m_pAssets.clear();
    m_pAssetByID.clear();
    m_pAssetByScene.clear();
}

/* actually update */
void ShotManager::updateDialogCueText(QString shotname) {
    int idx = m_pDialogLink->getIdx(shotname);
    QString cleanLine = m_pYmlManager->getCleanLine(m_pDialogLink->lines[idx]);
    QString durationStr = QString::number(m_pDialogLink->durations[idx], 'f', 3);

    m_dialogCueRectByShotname[shotname]->setTextMain(cleanLine);
    m_dialogCueRectByShotname[shotname]->setTextSecondary(durationStr);
}

QColor ShotManager::getBlockColorForActionType(EShotActionType type)
{
    switch (type) {
        case EShotCam:
            return QColorLight(0, 18);
        case EShotCamBlendStart:
        case EShotCamBlendKey:
        case EShotCamBlendTogame:
            return QColorLight(14, 15);
        case EShotCamBlendEnd:
            return QColorLightGray(14, 15);

        case EShotEnvBlendIn:
        case EShotFadeIn:
        case EShotWorldAddfact:
        case EShotWorldWeather:
        case EShotWorldEffectStart:
            return QColorLight(12, 15);
        case EShotEnvBlendOut:
        case EShotFadeOut:
        case EShotWorldEffectStop:
            return QColorLightGray(12, 15);

        case EShotActorAppearance:
        case EShotActorGamestate:
            return QColorLight(1, 15);
        case EShotActorScabbardShow:
        case EShotActorShow:
        case EShotPropShow:
        case EShotActorEquipRight:
        case EShotActorEquipLeft:
            return QColorLight(2, 15);
        case EShotActorScabbardHide:
        case EShotActorUnequipRight:
        case EShotActorUnequipLeft:
        case EShotActorHide:
        case EShotPropHide:
            return QColorLightGray(2, 15);

        case EShotActorAnim:
            return QColorLight(3, 15);
        case EShotActorAnimAdditive:
            return QColorLight(4, 15);
        case EShotActorAnimPose:
            return QColorLight(5, 15);

        case EShotActorMimicAnim:
            return QColorLight(6, 15);
        case EShotActorMimicPose:
            return QColorLight(7, 15);

        case EShotActorLookat:
            return QColorLight(8, 15);

        case EShotActorPlacement:
        case EShotPropPlacement:
            return QColorLight(9, 15);
        case EShotActorPlacementStart:
        case EShotActorPlacementKey:
        case EShotPropPlacementStart:
        case EShotPropPlacementKey:
            return QColorLight(10, 15);
        case EShotActorPlacementEnd:
        case EShotPropPlacementEnd:
            return QColorLightGray(10, 15);

        case EShotActorSound:
            return QColorLight(11, 15);
        case EShotActorEffectStart:
        case EShotPropEffectStart:
            return QColorLight(13, 15);
        case EShotPropEffectStop:
        case EShotActorEffectStop:
            return QColorLightGray(13, 15);
        default:
            return QColor(180, 180, 180);
    }
}

/*int ShotManager::getGroupNumForType(EShotActionType type) {
    switch (type) {
        case EShotCam:
        case EShotCamBlendStart:
        case EShotCamBlendKey:
        case EShotCamBlendEnd:
        case EShotCamBlendTogame:
            return 0;
            break;
        case EShotEnvBlendIn:
        case EShotEnvBlendOut:
        case EShotFadeIn:
        case EShotFadeOut:
        case EShotWorldAddfact:
        case EShotWorldWeather:
        case EShotWorldEffectStart:
        case EShotWorldEffectStop:
            return 1;
            break;
        // for every actor
        case EShotActorScabbardShow:
        case EShotActorScabbardHide:
        case EShotActorEquipRight:
        case EShotActorEquipLeft:
        case EShotActorUnequipRight:
        case EShotActorUnequipLeft:
        case EShotActorAppearance:
        case EShotActorGamestate:
        case EShotActorShow:
        case EShotActorHide:
            return 0;
            break;
        case EShotActorAnim:
            return 1;
            break;
        case EShotActorAnimPose:
        case EShotActorAnimAdditive:
            return 2;
            break;
        case EShotActorMimicAnim:
        case EShotActorMimicPose:
            return 3;
            break;
        case EShotActorLookat:
            return 4;
            break;
        case EShotActorPlacement:
        case EShotActorPlacementStart:
        case EShotActorPlacementKey:
        case EShotActorPlacementEnd:
            return 5;
            break;
        case EShotActorSound:
        case EShotActorEffectStart:
        case EShotActorEffectStop:
            return 6;
            break;

        // for every prop
        case EShotPropShow:
        case EShotPropHide:
            return 0;
            break;
        case EShotPropPlacement:
        case EShotPropPlacementStart:
        case EShotPropPlacementKey:
        case EShotPropPlacementEnd:
            return 1;
            break;
        case EShotPropEffectStart:
        case EShotPropEffectStop:
            return 2;
            break;

        default:
            return 0;
            break;
    }
}*/

double ShotManager::getMinYForAction(shotAction *action)
{
    //int asset_id = getAssetIDForAction(action);
    //qDebug() << QString("getMinYForAction: %1, isActorSpecificType = %2, actor id = %3").arg(EShotActionToString[action->actionType]).arg(isActorSpecificType(action->actionType)).arg(actor_id);
    /*if ( isAssetSpecificType(action->actionType) && asset_id >= 0 ) {
        int asset_idx = m_knownAssetsID.indexOf(asset_id);
        //qDebug() << QString("getMinYForAction: actor IDX = %1").arg(actor_idx);
        return m_assetsStartY[asset_idx] + SHOT_LABEL_HEIGHT * EShotActionToGroupNum[action->actionType];
    }*/
    return SHOT_LABEL_HEIGHT * EShotActionToGroupNum[action->actionType];
    // Y_max = Y_min + SHOT_DG_HEIGHT - SHOT_LABEL_PEN_WIDTH - 1;
}

QString ShotManager::shotNameByNum(int shotNum)
{
    return m_pDialogLink->shots[shotNum].shotName;
}

void ShotManager::getShotInfoForPoint(QPoint p, int &shotNum, double &shotCoord, int &actorNum, int &groupNum)
{
    // get groupNum, actorNum (Y)
    /*if (p.y() < SHOT_LABEL_HEIGHT * 2) {
        groupNum = p.y() / SHOT_LABEL_HEIGHT;
        actorNum = -1;
    } else {
        int y = p.y();
        upn(i, 0, m_knownAssetsID.count() - 1) {
            if (m_isAssetCollapsed[i]) {
                if (y >= SHOT_LABEL_HEIGHT) {
                    y -= SHOT_LABEL_HEIGHT;
                } else {
                    groupNum = 0;
                    actorNum = i;
                    break;
                }
            } else {
                if (y >= SHOT_LABEL_HEIGHT * m_groupsActorNumMax) {
                    y -= SHOT_LABEL_HEIGHT * m_groupsActorNumMax;
                } else {
                    groupNum = y / SHOT_LABEL_HEIGHT;
                    actorNum = i;
                    break;
                }
            }
        }
    }

    // get shotNum, shotCoord
    double x_sec = p.x() * SHOT_SECOND;
    upn(j, 0, m_pDialogLink->durations.count() - 1) {
        if (x_sec - m_pDialogLink->durations[j] > 0) {
            x_sec -= m_pDialogLink->durations[j];
        } else {
            shotNum = j;
            shotCoord = x_sec / m_pDialogLink->durations[j];
            break;
        }
    }
    qDebug() << QString("getShotInfoForPoint: group = %1, actor = %2 (%3), shot = %4 (%5), shotCoord = %6")
                .arg(groupNum).arg(actorNum).arg(m_pYmlManager->sceneGlobals()->getName( m_knownAssetsID[actorNum] ))
                .arg(shotNum).arg(shotNameByNum(shotNum)).arg(shotCoord);*/
}

int ShotManager::getAssetIDForAction(shotAction *action)
{
    if ( !isAssetSpecificType(action->actionType) )
        return -1;

    if (action->values.contains("actor"))
        return action->values.value("actor").toInt();

    int id;
    switch (action->actionType) {
        case EShotActorAnim:
        case EShotActorAnimAdditive:
            id = action->values["animation"].toInt();
            return m_pYmlManager->sceneGlobals()->anims[id].actorID;
            break;
        case EShotActorMimicAnim:
            id = action->values["animation"].toInt();
            return m_pYmlManager->sceneGlobals()->mimics[id].actorID;
            break;
        case EShotActorAnimPose:
            id = action->values["pose"].toInt();
            return m_pYmlManager->sceneGlobals()->poses[id].actorID;
            break;
        case EShotActorMimicPose:
            id = action->values["mimic"].toInt();
            return m_pYmlManager->sceneGlobals()->mimic_poses[id].actorID;
            break;
        default:
            break;
    }
    return -1;
}

double ShotManager::getDurationForAction(shotAction* sa) {
    double duration = -1.0;
    int id = 0;
    /*
        animation id may be production/repository anim
        duration() will calcualte correct value for any
    */
    switch (sa->actionType) {
        case EShotActorAnim:
        case EShotActorAnimAdditive:
            id = sa->values["animation"].toInt();
            duration = m_pYmlManager->sceneGlobals()->anims[id].duration();
            break;
        case EShotActorMimicAnim:
            id = sa->values["animation"].toInt();
            duration = m_pYmlManager->sceneGlobals()->mimics[id].duration();
            break;
        // case EShotActorSound: - we can't know exact audio from event
        case EShotEnvBlendIn:
        case EShotEnvBlendOut:
        case EShotWorldWeather:
            if (sa->values.contains("blendTime"))
                duration = sa->values["blendTime"].toDouble();
            break;
        case EShotActorMimicPose:
        case EShotFadeIn:
        case EShotFadeOut:
            if (sa->values.contains("duration"))
                duration = sa->values["duration"].toDouble();
            break;
        default:
            break;
    }
    return duration;
}

void ShotManager::onLoadShots(QString sectionName) {
	clearShotEditor();

    m_pYmlManager->info(QString("onLoadShots()>: section [%1]")
                     .arg(sectionName));
    m_pDialogLink = &(m_pYmlManager->dgLinkBySectionName[sectionName]);

    m_pDialogScene->setSceneRect(0,0, sceneWidth(), SHOT_DG_HEIGHT);
    m_pDialogScene->views().at(0)->setMaximumWidth(sceneWidth());

    /*m_pShotSceneShared->setSceneRect(0,0, sceneWidth, sceneHeight);
    m_pShotSceneShared->views().at(0)->setMaximumWidth(sceneWidth);
    m_pShotSceneShared->views().at(0)->setMaximumHeight(sceneHeight);

    m_pLabelSceneShared->setSceneRect(0,0, SHOT_LABEL_WIDTH, sceneHeight);
    m_pLabelSceneShared->views().at(0)->setMaximumHeight(sceneHeight);*/

    onAssetLoad(-1); // camera & env

    /* load actors and props */
    for (asset actor : m_pYmlManager->sceneGlobals()->actors) {
        qDebug() << "actor: " << m_pYmlManager->sceneGlobals()->getName( actor.nameID );
        onAssetLoad(actor.nameID);
    }
    for (asset prop : m_pYmlManager->sceneGlobals()->props) {
        qDebug() << "prop: " << m_pYmlManager->sceneGlobals()->getName( prop.nameID );
        onAssetLoad(prop.nameID);
    }

    /* horizontal blue dash line after actors rects */
    // TODO
    m_pDialogScene->addLine(0, SHOT_DG_HEIGHT * 0.6, sceneWidth(), SHOT_DG_HEIGHT * 0.6, QPen(Qt::blue));

    /* draw actor name+phrase+duration label rects and dash lines */
    for (int i = 0; i < m_pDialogLink->lines.count(); ++i) {
        onShotLoad(i);
    }
    onRepaintSecondNumbers();
    onRepaintVerticalLines();
}

void ShotManager::onRepaintSecondNumbers()
{
    for (QGraphicsSimpleTextItem* itemNumber : m_pDialogSecondNumbers) {
        m_pDialogScene->removeItem(itemNumber);
        delete itemNumber;
    }
    m_pDialogSecondNumbers.clear();

    int current_x = 0;
    QFont fontNum("Arial", 9);
    QBrush brush(Qt::darkGreen);
    for (int i = 1; i < m_pDialogLink->totalDuration + 30; ++i) {
        current_x += SHOT_SECOND;

        QGraphicsSimpleTextItem *textItem = new QGraphicsSimpleTextItem( qn(i) );
        textItem->setPos(current_x - 1.5, SHOT_DG_HEIGHT * 0.65);
        textItem->setFont(fontNum);
        textItem->setBrush(brush);
        textItem->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        m_pDialogSecondNumbers.append(textItem);
        m_pDialogScene->addItem(textItem);
    }
}

void ShotManager::onRepaintVerticalLines()
{
    upn(i, 0, m_pAssets.count() - 1) {
        onRepaintVerticalLinesForAssetID(m_pAssets[i]->assetID);
    }
}

void ShotManager::onRepaintVerticalLinesForAssetID(int assetID)
{
    ShotAsset* pAsset = m_pAssetByID[assetID];
    upn(j, 0, pAsset->pVerticalLines.count() - 1) {
            pAsset->pScene->removeItem(pAsset->pVerticalLines[j]);
            delete pAsset->pVerticalLines[j];
    }
    pAsset->pVerticalLines.clear();

    /* Bold lines between shots */
    QPen blackBoldPen(Qt::black, 2.0, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
    blackBoldPen.setCosmetic(true);
    double current_sec = 0;

    upn(i, 0, m_pDialogLink->shots.count() - 1) {
        current_sec += m_pDialogLink->durations[i];
        QGraphicsLineItem* boldLine = pAsset->pScene->addLine(SEC_TO_X(current_sec), 0, SEC_TO_X(current_sec), pAsset->pScene->height(), blackBoldPen);
        pAsset->pVerticalLines.append( boldLine );
    }

    /* Dash lines for seconds */
    QPen mediumPen2(QSvg::darkslateblue, 1.0, Qt::DashLine, Qt::FlatCap, Qt::BevelJoin);
    mediumPen2.setCosmetic(true);
    int current_x = 0;

    for (int i = 1; i < m_pDialogLink->totalDuration + 30; ++i) {
        current_x += SHOT_SECOND;
        QGraphicsLineItem* dashLine = pAsset->pScene->addLine(current_x, 0, current_x, pAsset->pScene->height(), mediumPen2);
        pAsset->pVerticalLines.append( dashLine );
    }
}

void ShotManager::onShotLoad(QString shotName) {
    upn(i, 0, m_pDialogLink->shots.size() - 1) {
        if (m_pDialogLink->shots[i].shotName == shotName) {
            onShotLoad(i);
            return;
        }
    }
}

void ShotManager::onShotLoad(int shotNum) {
    double start_sec = m_pDialogLink->getStartTimeForLine(shotNum);
    double duration_sec = m_pDialogLink->durations[shotNum];
    QString shotName = shotNameByNum(shotNum);

    CustomRectItem* actorCueLabel = new CustomRectItem;
    actorCueLabel->setRect(0, 0, duration_sec * SHOT_SECOND, SHOT_DG_HEIGHT * 0.6);
    actorCueLabel->setPos(start_sec * SHOT_SECOND, 0);

    QPen dialogLabelPen(Qt::black, 2.0, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
    dialogLabelPen.setCosmetic(true);
    actorCueLabel->setPen( dialogLabelPen );
    actorCueLabel->setTextParamsMain(QColorConstants::Svg::navy, 9, "Segoe UI", Qt::AlignLeft);
    actorCueLabel->setTextParamsSecondary(QColorConstants::Svg::purple, 9, "DejaVu Sans Mono");

    if (m_pDialogLink->speakers[shotNum] == m_pYmlManager->sceneGlobals()->getID(SASSETS, "PAUSE")) {
        actorCueLabel->setBrush( QBrush(colorDgViewPause) );
    } else if (m_pDialogLink->speakers[shotNum] == m_pYmlManager->sceneGlobals()->getID(SASSETS, "CHOICE")) {
        actorCueLabel->setBrush( QBrush(colorDgViewChoice) );
    } else {
        //QString speakerName = m_ymlManager->sceneGlobals()->getName(currentDgLink->speakers[shotNum]);
        //int speakerIdx = m_assets.indexOf( m_pDialogLink->speakers[shotNum] );
        int speakerIdx = 1; // TODO
        if (speakerIdx == -1) {
            speakerIdx = 0;
            m_pYmlManager->error("ShotManager: actor not in m_knownActors: " + m_pYmlManager->sceneGlobals()->getName(m_pDialogLink->speakers[shotNum]));
        }
        speakerIdx = speakerIdx % colorDgViewActors.count();
        actorCueLabel->setBrush( QBrush(colorDgViewActors[speakerIdx]) );
    }

    m_pDialogScene->addItem(actorCueLabel);
    m_dialogCueRectByShotname[shotName] = actorCueLabel;
    updateDialogCueText(shotName);

    /* draw action blocks for shot in editor */
    m_blocksByShotName[shotName] = QSet<CustomRectItem*>();
    upn(j, 0, m_pDialogLink->shots[shotNum].actions.count() - 1) {
        onShotActionLoad(shotNum, j);
    }
}

void ShotManager::onShotActionLoad(int shotNum, int actionNum) {
    double start_sec = m_pDialogLink->getStartTimeForLine(shotNum);
    double dur_sec = m_pDialogLink->durations[shotNum];
    QString shotName = shotNameByNum(shotNum);

    shotAction* sa = &m_pDialogLink->shots[shotNum].actions[actionNum];
    QString secondaryInfo = QString("[%1]").arg( sa->start, 0, 'f', 3 );
    double actionDuration = getDurationForAction(sa);

    // connect: click/clone/delete action ?
    CustomRectItem* actionRect = new CustomRectItem;
    if (actionDuration > 0) {
        actionRect->setRect( QRectF(0, 0, actionDuration * SHOT_SECOND, SHOT_LABEL_HEIGHT / 2.2) );
        secondaryInfo += QString(": %1 s").arg(actionDuration, 0, 'f', 2 );
    } else {
        actionRect->setRect( QRectF(0, 0, 0.5 * SHOT_SECOND, SHOT_LABEL_HEIGHT / 2.2) );
    }
    double minY = getMinYForAction(sa) + SHOT_LABEL_PEN_WIDTH;
    double maxY = minY + SHOT_LABEL_HEIGHT - actionRect->rect().height() - SHOT_LABEL_PEN_WIDTH;
    actionRect->setBordersRect(QRectF(0, minY, m_pDialogLink->totalDuration * SHOT_SECOND, maxY - minY));
    actionRect->setPos((start_sec + dur_sec * sa->start) * SHOT_SECOND, QRandomGenerator::global()->bounded((int)minY, (int)maxY + 1));
    actionRect->setDuration(actionDuration);
    actionRect->setFlag(QGraphicsItem::ItemIsMovable);
    actionRect->setFlag(QGraphicsItem::ItemIsSelectable);
    actionRect->setBackgroundColor( getBlockColorForActionType(sa->actionType) );
    actionRect->setTextMain( EShotActionToString[sa->actionType] );
    actionRect->setTextSecondary( secondaryInfo );
    actionRect->setZValue(2.0);
    actionRect->setShotAction( sa );
    actionRect->setData( "shotName", shotName );

    int assetID = getAssetIDForAction(sa);
    m_pAssetByID[assetID]->pScene->addItem(actionRect);
    actionRect->setData( "assetID", assetID );
    qDebug() << QString("Insert rect to ID: %2").arg(assetID);
    m_pAssetByID[assetID]->actionRects.insert(actionRect);
    m_blocksByShotName[shotName].insert(actionRect);

    m_pYmlManager->info(QString("Add shot: [%1] %2")
                     .arg(sa->start, 0, 'f', 3 )
                        .arg( EShotActionToString[sa->actionType] ));
}

void ShotManager::onShotActionAdd(int shotNum, EShotActionType type)
{

}

void ShotManager::onShotActionRemove(int shotNum, int actionNum)
{
    //shotAction* sa = &m_pDialogLink->shots[shotNum].actions[actionNum];
    // TODO
}

void ShotManager::onShotActionUpdate(int shotNum, int actionNum)
{

}

void ShotManager::onAssetLoad(int assetID)
{
    QGraphicsView* pNewView = new QGraphicsView(m_pShotWidget);
    QGraphicsScene* pNewScene = new QGraphicsScene(pNewView);
    //pNewView->setInteractive(false);
    pNewScene->setBackgroundBrush(m_LightYellowGradient);
    pNewView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pNewView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //pNewView->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QGraphicsView* pNewViewLabel = new QGraphicsView(m_pShotLabelWidget);
    QGraphicsScene* pNewSceneLabel = new QGraphicsScene(pNewViewLabel);
   // pNewViewLabel->setInteractive(false);
    pNewViewLabel->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pNewViewLabel->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //pNewViewLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    ShotAsset* newAsset = new ShotAsset(pNewView, pNewScene, pNewViewLabel, pNewSceneLabel, assetID);

    int asset_idx = m_pAssets.count();
    newAsset->isProp = m_pYmlManager->sceneGlobals()->props.contains(assetID);

    /* draw labels */
    QPen bigPen(QSvg::darkslateblue, 2.0, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
    QPen mediumPen(QSvg::darkolivegreen, 1.0, Qt::DashLine, Qt::FlatCap, Qt::BevelJoin);
    bigPen.setCosmetic(true);
    mediumPen.setCosmetic(true);

    QVector<QString> groups;
    QVector<EShotActionType> type_samples;
    if (assetID == -1) {
        groups = {"CAMERAS", "ENV/WORLD"};
        type_samples = {EShotCam, EShotEnvBlendIn};
        newAsset->assetName = "";
    } else if (newAsset->isProp) {
        groups = {"STATE", "PLACEMENT", "SFX"};
        type_samples = {EShotPropShow, EShotPropPlacement, EShotPropEffectStart};
        newAsset->assetName = m_pYmlManager->sceneGlobals()->getName(assetID);
    } else {
        groups = {"STATE", "ANIMS", "POSE/ADDITIVES", "MIMICS", "LOOKATS", "PLACEMENT", "SFX/VFX"};
        type_samples = {EShotActorShow, EShotActorAnim, EShotActorAnimPose, EShotActorMimicAnim, EShotActorLookat, EShotActorPlacement, EShotActorEffectStart};
        newAsset->assetName = m_pYmlManager->sceneGlobals()->getName(assetID);
    }
    pNewScene->setSceneRect(0, 0, SEC_TO_X(m_pDialogLink->totalDuration + 30), SHOT_LABEL_HEIGHT * groups.count());
    pNewView->setScene(pNewScene);
    pNewView->setSceneRect(0, 0, SEC_TO_X(m_pDialogLink->totalDuration + 30), SHOT_LABEL_HEIGHT * groups.count());
    pNewView->setFixedSize(SEC_TO_X(m_pDialogLink->totalDuration + 30), SHOT_LABEL_HEIGHT * groups.count());
    //pNewView->setSceneRect(0, 0, 5000, SHOT_LABEL_HEIGHT * groups.count());
    //pNewView->setFixedSize(SEC_TO_X(m_pDialogLink->totalDuration + 30), SHOT_LABEL_HEIGHT * groups.count());

    pNewSceneLabel->setSceneRect(0, 0, SHOT_LABEL_WIDTH, SHOT_LABEL_HEIGHT * groups.count());
    pNewViewLabel->setScene(pNewSceneLabel);
    pNewViewLabel->setSceneRect(0, 0, SHOT_LABEL_WIDTH, SHOT_LABEL_HEIGHT * groups.count());
    pNewViewLabel->setFixedSize(SHOT_LABEL_WIDTH, SHOT_LABEL_HEIGHT * groups.count());

    /* add label rects */
    upn(i, 0, groups.count() - 1) {
        CustomRectItem* labelRect = new CustomRectItem;
        labelRect->setRect(0, 0, SHOT_LABEL_WIDTH - SHOT_LABEL_PEN_WIDTH, SHOT_LABEL_HEIGHT - SHOT_LABEL_PEN_WIDTH);
        labelRect->setPos(0, SHOT_LABEL_HEIGHT * i);
        labelRect->setBackgroundColor( QColorDark(getBlockColorForActionType(type_samples[i]).hsvHue()) );
        labelRect->setPen( QPen(QSvg::honeydew, SHOT_LABEL_PEN_WIDTH, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin) );
        labelRect->setTextParamsMain(colorDgViewActors[asset_idx % colorDgViewActors.count()], 10, "Helvetica", Qt::AlignTop | Qt::AlignHCenter);
        labelRect->setTextParamsSecondary(QColorConstants::Svg::gold, 10, "Sans Serif", Qt::AlignVCenter | Qt::AlignHCenter | Qt::TextWordWrap);
        labelRect->setTextSecondary( groups[i] );
        labelRect->setData( "assetID", assetID );
        labelRect->setData( "groupNum", i );
        labelRect->setTextMain( newAsset->assetName );

        /* show/hide button */
        if (assetID >= 0) {
            labelRect->setButtonImages(QImage(":/hide.png").scaledToHeight(20, Qt::SmoothTransformation), QImage(":/show.png").scaledToHeight(20, Qt::SmoothTransformation));
            connect(labelRect, SIGNAL(onDoubleClick(bool)), this, SLOT(onAssetCollapse(bool)));
        }
        pNewSceneLabel->addItem(labelRect);
        /* horizontal separator */
        if (i > 0) {
            QGraphicsLineItem* horizontalDash = pNewScene->addLine(0, SHOT_LABEL_HEIGHT * i, pNewScene->width(), SHOT_LABEL_HEIGHT * i, mediumPen);
            newAsset->pHorizontalLines.append( horizontalDash );
        }
    }

    m_pShotWidget->layout()->addWidget(pNewView);
    m_pShotLabelWidget->layout()->addWidget(pNewViewLabel);
    m_pAssets.append(newAsset);
    m_pAssetByID[assetID] = newAsset;
    m_pAssetByScene[pNewScene] = newAsset;
    qDebug() << "onAssetAdd [" << m_pAssets.count() << "]: " << assetID;
    // TODO
}

void ShotManager::onAssetChange(int assetID)
{
    // TODO - redraw ?
    //m_assetLabels[ m_knownAssetsID.indexOf(assetID) ].first()->setTextMain( "(" + m_pYmlManager->sceneGlobals()->getName(assetID) + ")" );
}

void ShotManager::onAssetRemove(int assetID)
{
    ShotAsset* shotAsset = m_pAssetByID[assetID];
    m_pAssetByScene.remove(shotAsset->pScene);

    for (QGraphicsItem* item : shotAsset->pScene->items()) {
        CustomRectItem* rectItem = static_cast<CustomRectItem*>(item);
        m_blocksByShotName[rectItem->data("shotName").toString()].remove(rectItem);
    }
    shotAsset->pScene->clear();
    shotAsset->pVerticalLines.clear();
    shotAsset->pHorizontalLines.clear();
    m_pShotLabelWidget->layout()->removeWidget(shotAsset->pViewLabel);
    shotAsset->pViewLabel->deleteLater(); // should delete scene too
    m_pShotWidget->layout()->removeWidget(shotAsset->pView);
    shotAsset->pView->deleteLater(); // should delete scene too

    m_pAssetByID.remove(assetID);
    upn(i, 0, m_pAssets.count() - 1) {
        if (m_pAssets[i]->assetID == assetID) {
            m_pAssets.removeAt(i);
            break;
        }
    }
}

void ShotManager::onAssetCollapse(bool isCollapsed)
{
    CustomRectItem* rectItem = static_cast<CustomRectItem*>(sender());
    int assetID = rectItem->data("assetID").toInt();
    int groupNum = rectItem->data("groupNum").toInt();
    qDebug() << "assetID: " << assetID << ", isCollapsed: " << isCollapsed;
    m_pAssetByID[assetID]->isCollapsed = isCollapsed;
    if (isCollapsed) {
        m_pAssetByID[assetID]->pView->setSceneRect(0, SHOT_LABEL_HEIGHT * groupNum, SEC_TO_X(m_pDialogLink->totalDuration + 30), SHOT_LABEL_HEIGHT * (groupNum + 1));
        m_pAssetByID[assetID]->pView->setFixedHeight(SHOT_LABEL_HEIGHT);
        m_pAssetByID[assetID]->pViewLabel->setSceneRect(0, SHOT_LABEL_HEIGHT * groupNum, SHOT_LABEL_WIDTH, SHOT_LABEL_HEIGHT * (groupNum + 1));
        m_pAssetByID[assetID]->pViewLabel->setFixedHeight(SHOT_LABEL_HEIGHT);
    } else {
        m_pAssetByID[assetID]->pView->setSceneRect(0, 0, SEC_TO_X(m_pDialogLink->totalDuration + 30), SHOT_LABEL_HEIGHT * (m_pAssetByID[assetID]->isProp ? m_groupsPropNumMax : m_groupsActorNumMax));
        m_pAssetByID[assetID]->pView->setFixedHeight(SHOT_LABEL_HEIGHT * (m_pAssetByID[assetID]->isProp ? m_groupsPropNumMax : m_groupsActorNumMax));
        m_pAssetByID[assetID]->pViewLabel->setSceneRect(0, 0, SEC_TO_X(m_pDialogLink->totalDuration + 30), SHOT_LABEL_HEIGHT * (m_pAssetByID[assetID]->isProp ? m_groupsPropNumMax : m_groupsActorNumMax));
        m_pAssetByID[assetID]->pViewLabel->setFixedHeight(SHOT_LABEL_HEIGHT * (m_pAssetByID[assetID]->isProp ? m_groupsPropNumMax : m_groupsActorNumMax));
    }
    // TODO - redraw
}

void ShotManager::onShotRename(QString oldShotName, QString newShotName) {
    m_blocksByShotName[newShotName] = m_blocksByShotName[oldShotName];
    m_blocksByShotName.remove(oldShotName);
    for (CustomRectItem* rectItem : m_blocksByShotName[newShotName]) {
        rectItem->setData("shotName", newShotName);
    }
    // TODO m_ymlManager->renameShot(shotName);
}
void ShotManager::onShotRemove(QString shotName) {
    for (CustomRectItem* rectItem : m_blocksByShotName[shotName]) {
        m_pAssetByID[rectItem->data("assetID").toInt()]->actionRects.remove(rectItem);
        rectItem->scene()->removeItem(rectItem);
        rectItem->deleteLater();
    }
    m_blocksByShotName.remove(shotName);
    onRepaintVerticalLines();
    // TODO m_ymlManager->removeShot(shotName);
}
