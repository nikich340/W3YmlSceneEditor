#include <QRandomGenerator>
#include "ShotManager.h"

ShotManager::ShotManager(YmlSceneManager* newYmlManager, QObject *parent) : QObject(parent)
{
    m_pYmlManager = newYmlManager;
}

void ShotManager::setShotScenes(QGraphicsScene* gDgScene, QGraphicsScene* gLabelScene, QGraphicsScene* gShotScene) {
	pDgScene = gDgScene;
	pLabelScene = gLabelScene;
	pShotScene = gShotScene;

    pShotScene->views()[0]->setMouseTracking(true);
    pShotScene->installEventFilter(this);

    /* one-time constant UI things */

    /* red navigation line */
    m_pNavigationLine = new QGraphicsLineItem;
    m_pNavigationLine->setZValue(1.0);
    m_pNavigationLine->setPen( QPen(Qt::red) );
    m_pNavigationLine->setPos(0, 0);
    m_pNavigationLine->setLine(0, 0, 0, SHOT_SCENE_HEIGHT);
    m_pNavigationLine->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    pShotScene->addItem(m_pNavigationLine);
}

bool ShotManager::eventFilter(QObject *obj, QEvent *event) {
	//qDebug() << "event:" << event->type();
	if (obj == pShotScene && event->type() == QEvent::GraphicsSceneMouseMove) {
		QGraphicsSceneMouseEvent* hoverEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
        if (m_pNavigationLine != nullptr)
            m_pNavigationLine->setPos( hoverEvent->scenePos().x(), 0 );
	}
    return false;
}

bool ShotManager::isAssetSpecificType(EShotActionType type)
{
    return !NonAssetEShotActions.contains(type);
}

void ShotManager::clearShotEditor() {
    pLabelScene->clear();
	pDgScene->clear();
	pShotScene->clear();

    m_dgCueRectByShotname.clear();
    m_knownAssetsID.clear();
    m_assetsStartY.clear();
    m_assetsEndY.clear();
    m_isAssetCollapsed.clear();
    m_assetLabels.clear();
    m_pNavigationLine = nullptr;
}

/* actually update */
void ShotManager::updateCueText(QString shotname) {
    int idx = m_pDialogLink->getIdx(shotname);
    QString cleanLine = m_pYmlManager->getCleanLine(m_pDialogLink->lines[idx]);
    QString durationStr = QString::number(m_pDialogLink->durations[idx], 'f', 3);

    m_dgCueRectByShotname[shotname]->setTextMain(cleanLine);
    m_dgCueRectByShotname[shotname]->setTextSecondary(durationStr);
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
    int asset_id = getActorIdForAction(action);
    //qDebug() << QString("getMinYForAction: %1, isActorSpecificType = %2, actor id = %3").arg(EShotActionToString[action->actionType]).arg(isActorSpecificType(action->actionType)).arg(actor_id);
    if ( isAssetSpecificType(action->actionType) && asset_id >= 0 ) {
        int asset_idx = m_knownAssetsID.indexOf(asset_id);
        //qDebug() << QString("getMinYForAction: actor IDX = %1").arg(actor_idx);
        return m_assetsStartY[asset_idx] + SHOT_LABEL_HEIGHT * EShotActionToGroupNum[action->actionType];
    }
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
    if (p.y() < SHOT_LABEL_HEIGHT * 2) {
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
                .arg(shotNum).arg(shotNameByNum(shotNum)).arg(shotCoord);
}

int ShotManager::getActorIdForAction(shotAction *action)
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

    /* set scene sizes */
    double sceneWidth = qMax(SHOT_SCENE_WIDTH, m_pDialogLink->totalDuration * 2.0 * SHOT_SECOND);
    double sceneHeight = qMax(SHOT_SCENE_HEIGHT, SHOT_LABEL_HEIGHT * (2 + m_pYmlManager->sceneGlobals()->actors.count() * m_groupsActorNumMax));

    pDgScene->setSceneRect(0,0, sceneWidth, SHOT_DG_HEIGHT);
    pDgScene->views().at(0)->setMaximumWidth(sceneWidth);

    pShotScene->setSceneRect(0,0, sceneWidth, sceneHeight);
    pShotScene->views().at(0)->setMaximumWidth(sceneWidth);
    pShotScene->views().at(0)->setMaximumHeight(sceneHeight);

    pLabelScene->setSceneRect(0,0, SHOT_LABEL_WIDTH, sceneHeight);
    pLabelScene->views().at(0)->setMaximumHeight(sceneHeight);

    /* camera & env groups */
    QVector<QString> groups = {"CAMERAS", "ENV/WORLD"};
    QVector<EShotActionType> type_samples = {EShotCam, EShotEnvBlendIn};
    upn(i, 0, 1) {
        CustomRectItem* labelRect = new CustomRectItem;
        labelRect->setRect(0, 0, SHOT_LABEL_WIDTH - SHOT_LABEL_PEN_WIDTH * 2, SHOT_LABEL_HEIGHT - SHOT_LABEL_PEN_WIDTH);
        labelRect->setPos(0, SHOT_LABEL_HEIGHT * i);
        labelRect->setBackgroundColor( QColorDark(getBlockColorForActionType(type_samples[i]).hsvHue()) );
        labelRect->setPen( QPen(QSvg::honeydew, SHOT_LABEL_PEN_WIDTH, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin) );
        labelRect->setTextParamsSecondary(QColorConstants::Svg::gold, 10, "Sans Serif", Qt::AlignCenter | Qt::TextWordWrap);
        labelRect->setTextSecondary( groups[i] );
        pLabelScene->addItem( labelRect );
    }
    QPen bigPen(QSvg::darkslateblue, 2.0, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
    QPen mediumPen(QSvg::darkolivegreen, 1.0, Qt::DashLine, Qt::FlatCap, Qt::BevelJoin);
    bigPen.setCosmetic(true);
    mediumPen.setCosmetic(true);
    pShotScene->addLine(0, SHOT_LABEL_HEIGHT * 1, sceneWidth, SHOT_LABEL_HEIGHT * 1, mediumPen);
    pShotScene->addLine(0, SHOT_LABEL_HEIGHT * 2, sceneWidth, SHOT_LABEL_HEIGHT * 2, bigPen);

    /* load actors and props */
    for (asset actor : m_pYmlManager->sceneGlobals()->actors) {
        qDebug() << "actor: " << m_pYmlManager->sceneGlobals()->getName( actor.nameID );
        onAssetLoad(actor.nameID);
    }
    for (asset prop : m_pYmlManager->sceneGlobals()->props) {
        qDebug() << "prop: " << m_pYmlManager->sceneGlobals()->getName( prop.nameID );
        onAssetLoad(prop.nameID);
    }

    /* draw seconds and dash lines */
	double cur_x = 0;
    QPen mediumPen2(QSvg::darkslateblue, 1.0, Qt::DashLine, Qt::FlatCap, Qt::BevelJoin);
    mediumPen2.setCosmetic(true);
    QFont fontNum("Arial", 9);
	QBrush brush(Qt::darkGreen);

    for (int i = 1; i < m_pDialogLink->totalDuration * 2.0; ++i) {
		cur_x += SHOT_SECOND;
        pShotScene->addLine(cur_x, 0, cur_x, sceneHeight, mediumPen2);

		QGraphicsSimpleTextItem *textItem = new QGraphicsSimpleTextItem( qn(i) );
		textItem->setPos(cur_x - 1.5, SHOT_DG_HEIGHT * 0.65);
        textItem->setFont(fontNum);
		textItem->setBrush(brush);
		textItem->setFlag(QGraphicsItem::ItemIgnoresTransformations);
		pDgScene->addItem(textItem);
	}

    /* horizontal blue dash line after actors rects */
    pDgScene->addLine(0, SHOT_DG_HEIGHT * 0.6, sceneWidth, SHOT_DG_HEIGHT * 0.6, mediumPen2);

    /* draw actor name+phrase+duration label rects and dash lines */

    for (int i = 0; i < m_pDialogLink->lines.size(); ++i) {
        onShotLoad(i);
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
        int speakerIdx = m_knownAssetsID.indexOf( m_pDialogLink->speakers[shotNum] );
        if (speakerIdx == -1) {
            speakerIdx = 0;
            m_pYmlManager->error("ShotManager: actor not in m_knownActors: " + m_pYmlManager->sceneGlobals()->getName(m_pDialogLink->speakers[shotNum]));
        }
        speakerIdx = speakerIdx % colorDgViewActors.count();
        actorCueLabel->setBrush( QBrush(colorDgViewActors[speakerIdx]) );
    }

    pDgScene->addItem(actorCueLabel);
    m_dgCueRectByShotname[shotName] = actorCueLabel;
    updateCueText(shotName);

    /* draw action blocks for shot in editor */
    m_blocksByShotName[shotName] = QVector<CustomRectItem*>();
    upn(j, 0, m_pDialogLink->shots[shotNum].actions.size() - 1) {
        onShotActionLoad(shotNum, j);
    }

    /* vertical line after shot */
    pShotScene->addLine((start_sec + duration_sec) * SHOT_SECOND, 0, (start_sec + duration_sec) * SHOT_SECOND, pShotScene->sceneRect().height(), dialogLabelPen);
}

void ShotManager::onShotActionLoad(int shotNum, int actionNum) {
    double start_sec = m_pDialogLink->getStartTimeForLine(shotNum);
    double dur_sec = m_pDialogLink->durations[shotNum];

    shotAction* sa = &m_pDialogLink->shots[shotNum].actions[actionNum];
    QString secondaryInfo = QString("[%1]").arg(sa->start, 0, 'f', 3 );
    double actionDuration = getDurationForAction(sa);

    // connect: click/clone/delete action ?
    CustomRectItem* actionRect = new CustomRectItem();
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
    actionRect->setShotName( shotNameByNum(shotNum) );
    pShotScene->addItem(actionRect);
    m_blocksByShotName[ shotNameByNum(shotNum) ].append(actionRect);

    int actorID = getActorIdForAction(sa);
    actionRect->setAssetID( actorID );
    if (actorID >= 0) {
        m_blocksByAssetID[ getActorIdForAction(sa) ].append(actionRect);
    }

    m_pYmlManager->info(QString("Add shot: [%1] %2")
                     .arg(sa->start, 0, 'f', 3 )
                        .arg( EShotActionToString[sa->actionType] ));
}

void ShotManager::onShotActionAdd(int shotNum, EShotActionType type)
{

}

void ShotManager::onShotActionRemove(int shotNum, int actionNum)
{
    shotAction* sa = &m_pDialogLink->shots[shotNum].actions[actionNum];
    // TODO
}

void ShotManager::onShotActionUpdate(int shotNum, int actionNum)
{

}

void ShotManager::onAssetLoad(int assetID)
{
    m_blocksByAssetID[assetID] = QVector<CustomRectItem*>();
    m_assetLabels.append( QVector<CustomRectItem*>() );
    m_knownAssetsID.append( assetID );
    int asset_idx = m_knownAssetsID.count() - 1;
    bool isProp = m_isAssetProp(asset_idx);
    /* draw labels */

    QPen bigPen(QSvg::darkslateblue, 2.0, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
    QPen mediumPen(QSvg::darkolivegreen, 1.0, Qt::DashLine, Qt::FlatCap, Qt::BevelJoin);
    bigPen.setCosmetic(true);
    mediumPen.setCosmetic(true);

    double startY = asset_idx > 0 ? m_assetsEndY[asset_idx - 1] : SHOT_LABEL_HEIGHT * 2;
    double endY = startY + SHOT_LABEL_HEIGHT * (isProp ? m_groupsPropNumMax : m_groupsActorNumMax);
    m_assetsStartY.append( startY );
    m_assetsEndY.append( endY );

    QVector<QString> groups;
    QVector<EShotActionType> type_samples;
    if (isProp) {
        groups = {"STATE", "PLACEMENT", "SFX"};
        type_samples = {EShotPropShow, EShotPropPlacement, EShotPropEffectStart};
    } else {
        groups = {"STATE", "ANIMS", "POSE/ADDITIVES", "MIMICS", "LOOKATS", "PLACEMENT", "SFX/VFX"};
        type_samples = {EShotActorShow, EShotActorAnim, EShotActorAnimPose, EShotActorMimicAnim, EShotActorLookat, EShotActorPlacement, EShotActorEffectStart};
    }
    upn(i, 0, groups.count() - 1) {
        CustomRectItem* labelRect = new CustomRectItem;
        labelRect->setRect(0, 0, SHOT_LABEL_WIDTH - SHOT_LABEL_PEN_WIDTH * 2, SHOT_LABEL_HEIGHT - SHOT_LABEL_PEN_WIDTH);
        labelRect->setPos(0, startY + SHOT_LABEL_HEIGHT * i);
        labelRect->setBackgroundColor( QColorDark(getBlockColorForActionType(type_samples[i]).hsvHue()) );
        labelRect->setPen( QPen(QSvg::honeydew, SHOT_LABEL_PEN_WIDTH, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin) );
        labelRect->setTextParamsMain(colorDgViewActors[asset_idx % colorDgViewActors.count()], 10, "Helvetica", Qt::AlignTop | Qt::AlignHCenter);
        labelRect->setTextParamsSecondary(QColorConstants::Svg::gold, 10, "Sans Serif", Qt::AlignVCenter | Qt::AlignHCenter | Qt::TextWordWrap);
        labelRect->setTextSecondary( groups[i] );
        //if (i == 0)
        labelRect->setTextMain( m_pYmlManager->sceneGlobals()->getName(assetID) );
        pLabelScene->addItem( labelRect );
        m_assetLabels[asset_idx].append( labelRect );
        /* horizontal separator */
        if (i > 0)
            pShotScene->addLine(0, startY + SHOT_LABEL_HEIGHT * i, pShotScene->sceneRect().width(), startY + SHOT_LABEL_HEIGHT * i, mediumPen);
    }

    /* horizontal line before/after actor */
    pShotScene->addLine(0, endY, pShotScene->sceneRect().width(), endY, bigPen);
    qDebug() << "onAssetAdd [" << m_knownAssetsID.indexOf(assetID) << "]: " << assetID;
    // TODO
}

void ShotManager::onAssetChange(int assetID)
{
    // TODO - redraw ?
    m_assetLabels[ m_knownAssetsID.indexOf(assetID) ].first()->setTextMain( "(" + m_pYmlManager->sceneGlobals()->getName(assetID) + ")" );
}

void ShotManager::onAssetRemove(int assetID)
{
    for (CustomRectItem* block : m_blocksByAssetID[assetID]) {
        pShotScene->removeItem(block);
        block->deleteLater();
    }
    m_blocksByAssetID.remove(assetID);
    m_assetLabels.remove( m_knownAssetsID.indexOf(assetID) );
    m_knownAssetsID.remove( m_knownAssetsID.indexOf(assetID) );
    // TODO - redraw or move?
}

void ShotManager::onAssetCollapse(int assetID)
{
    m_isAssetCollapsed[ m_knownAssetsID.indexOf(assetID) ] = true;
    ///pDgScene->views().at(0)->repaint();
    // TODO - redraw or move?
}

void ShotManager::onAssetUncollapse(int assetID)
{
    m_isAssetCollapsed[ m_knownAssetsID.indexOf(assetID) ] = false;
    // TODO - redraw
}

void ShotManager::onShotRename(QString oldName, QString newName) {
    m_blocksByShotName[newName] = m_blocksByShotName[oldName];
    m_blocksByShotName.remove(oldName);
    // TODO m_ymlManager->renameShot(shotName);
}
void ShotManager::onShotRemove(QString shotName) {
    for (CustomRectItem* block : m_blocksByShotName[shotName]) {
        pShotScene->removeItem(block);
        block->deleteLater();
    }
    m_blocksByShotName.remove(shotName);
    // TODO m_ymlManager->removeShot(shotName);
}
