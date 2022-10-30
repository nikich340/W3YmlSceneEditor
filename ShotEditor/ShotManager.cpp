#include <QElapsedTimer>
#include <QRandomGenerator>
#include "ShotManager.h"

QLinearGradient ShotManager::createGradient(QColor startColor, QColor endColor, double W, double H)
{
    QLinearGradient grad(0,0, W,H);
    grad.setCoordinateMode(QGradient::LogicalMode);
    grad.setColorAt(0, startColor);
    grad.setColorAt(1, endColor);
    return grad;
}

ShotManager::ShotManager(YmlSceneManager* newYmlManager, QObject *parent) : QObject(parent)
{
    m_pYmlManager = newYmlManager;
}

void ShotManager::setWidgets(QGraphicsScene *newDialogScene, QScrollArea *newShotLabelArea, ShotScrollArea *newShotArea)
{
    m_pDialogScene = newDialogScene;
    m_pShotLabelArea = newShotLabelArea;
    m_pShotArea = newShotArea;
    m_pShotLabelWidget = newShotLabelArea->widget();
    m_pShotWidget = newShotArea->widget();

    m_pShotLabelArea->verticalScrollBar()->setEnabled(false);
    m_pShotLabelArea->setFixedWidth(SHOT_LABEL_WIDTH);
    m_pDialogScene->setSceneRect(0,0, SHOT_SCENE_WIDTH, SHOT_DG_HEIGHT);
    m_pDialogScene->views().first()->setFixedHeight(SHOT_DG_HEIGHT);

    /* one-time constant UI things - shared scene */
    m_pixShow = QPixmap(":/show.png").scaledToHeight(20, Qt::SmoothTransformation);
    m_pixHide = QPixmap(":/hide.png").scaledToHeight(20, Qt::SmoothTransformation);
}

bool ShotManager::isAssetSpecificType(EShotActionType type)
{
    return !EShotActionsShared.contains(type);
}

void ShotManager::updateDialogCueText(QString shotname) {
    int idx = m_pDialogLink->shotNumByName(shotname);
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

/**
 * @brief ShotManager::updateHorizontalAdvance
 * No way
 */
void ShotManager::updateHorizontalAdvance()
{
    // dialogs
    m_pDialogScene->setSceneRect(0,0, sceneWidth(), SHOT_DG_HEIGHT);
    onRepaintSecondNumbers();

    // labels - no changes
    // shots
    upn(i, 0, m_pAssets.count() - 1) {
        QRectF rect = m_pAssets[i]->pScene->sceneRect();
        rect.setWidth( sceneWidth() );
        m_pAssets[i]->pScene->setSceneRect(rect);
        m_pAssets[i]->pView->setFixedWidth(sceneWidth() * m_pShotArea->scaleFactor());
    }
    onRepaintVerticalLines();
}

void ShotManager::onClearEditor()
{
    /* dialog scene */
    m_pShotArea->clearAllShotViews();
    m_pShotArea->resetScaleFactor();
    m_pDialogScene->views().first()->resetTransform();
    m_pDialogScene->clear();

    m_dialogCueRectByShotname.clear();
    m_pDialogSecondNumbers.clear();

    /* assets labels scene & actions scene */
    upn(i, 0, m_pAssets.count() - 1) {
        m_pShotLabelWidget->layout()->removeWidget(m_pAssets[i]->pViewLabel);
        m_pShotWidget->layout()->removeWidget(m_pAssets[i]->pView);
        m_pAssets[i]->clearAll();
        delete m_pAssets[i];
    }
    m_pAssets.clear();
    m_pAssetByID.clear();
    m_assetIDByScene.clear();
}

double ShotManager::getMinYForAction(shotAction *action)
{
    return SHOT_LABEL_HEIGHT * EShotActionToGroupNum[action->actionType];
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

int ShotManager::X_TO_ShotNum(double x)
{
    double sec = 0.0;
    upn(i, 0, m_pDialogLink->durations.count() - 1) {
        double beginX = SEC_TO_X(sec);
        sec += m_pDialogLink->durations[i];
        double endX = SEC_TO_X(sec);
        if (x >= beginX && x < endX) {  // < endX to avoid 1.0]
            return i;
        }
    }
    return -1;  // -1 = out of shots range
}

double ShotManager::X_TO_ShotPoint(double x)
{
    double sec = 0.0;
    upn(i, 0, m_pDialogLink->durations.count() - 1) {
        double beginX = SEC_TO_X(sec);
        sec += m_pDialogLink->durations[i];
        double endX = SEC_TO_X(sec);
        if (x >= beginX && x < endX) {  // < endX to avoid 1.0]
            return (x - beginX) / (endX - beginX);
        }
    }
    return -1.0;  // -1 = out of shots range
}

void ShotManager::onLoadSectionShots(QString sectionName) {
    QElapsedTimer timer;
    timer.start();

    onClearEditor();
    m_pShotArea->setEnabled(true);
    m_pShotLabelArea->setEnabled(true);
    m_pDialogScene->views().first()->setEnabled(true);
    m_sectionName = sectionName;

    m_pDialogLink = &(m_pYmlManager->m_dialogLinkBySectionName[sectionName]);

    m_pDialogScene->setSceneRect(0,0, sceneWidth(), SHOT_DG_HEIGHT);
    // DONT USE m_pDialogScene->views().first()->setFixedWidth(sceneWidth() * m_pShotArea->scaleFactor());

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

    m_sectionType = m_pYmlManager->getSectionLink(sectionName)->type;
    bool shotsAllowed = (m_sectionType != scriptS && m_sectionType != exitS);
    m_pShotArea->setEnabled(shotsAllowed);
    m_pShotLabelArea->setEnabled(shotsAllowed);
    m_pDialogScene->views().first()->setEnabled(shotsAllowed);

    m_pYmlManager->info(QString("onLoadShots()>: section [%1] loaded in %2 ms")
                        .arg(sectionName).arg(timer.elapsed()));
}

void ShotManager::onUpdateSectionName(QString oldSectionName, QString newSectionName)
{
    if (oldSectionName == m_sectionName) {
        m_sectionName = newSectionName;
    }
}

void ShotManager::onUpdateSectionType(QString sectionName, int newType)
{
    onLoadSectionShots(sectionName);
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

void ShotManager::onRepaintHorizontalLines()
{
    upn(i, 0, m_pAssets.count() - 1) {
        onRepaintHorizontalLinesForAssetID(m_pAssets[i]->assetID);
    }
}

void ShotManager::onRepaintHorizontalLinesForAssetID(int assetID)
{
    ShotAsset* pAsset = m_pAssetByID[assetID];
    int groupNum = 2;
    if (assetID == -1) {
        groupNum = 2;
    } else if (m_pAssetByID[assetID]->isProp) {
        groupNum = m_groupsPropNumMax;
    } else {
        groupNum = m_groupsActorNumMax;
    }

    QPen mediumPen(QSvg::darkolivegreen, 1.0, Qt::DashLine, Qt::FlatCap, Qt::BevelJoin);
    mediumPen.setCosmetic(true);
    upn(i, 0, pAsset->pHorizontalLines.count() - 1) {
        QGraphicsLineItem* pLine = pAsset->pHorizontalLines[i];
        if (pLine->scene() != nullptr)
            pLine->scene()->removeItem( pLine );
        delete pLine;
    }
    pAsset->pHorizontalLines.clear();

    upn(i, 1, groupNum - 1) {
        QGraphicsLineItem* horizontalDash = pAsset->pScene->addLine(0, SHOT_LABEL_HEIGHT * i, sceneWidth(), SHOT_LABEL_HEIGHT * i, mediumPen);
        pAsset->pHorizontalLines.append( horizontalDash );
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

    /* Remove old vertical lines */
    upn(i, 0, pAsset->pVerticalLines.count() - 1) {
        QGraphicsLineItem* pLine = pAsset->pVerticalLines[i];
        if (pLine->scene() != nullptr)
            pLine->scene()->removeItem( pLine );
        delete pLine;
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

void ShotManager::onNavigationLineMove(QPointF scenePos)
{
    upn(i, 0, m_pAssets.count() - 1) {
        if (m_pAssets[i]->pNavigationLine != nullptr)
            m_pAssets[i]->pNavigationLine->setPos( scenePos.x(), 0 );
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
    double start_sec = m_pDialogLink->getStartTimeForShot(shotNum);
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
        int speakerIdx = -1;
        upn(i, 0, m_pAssets.count() - 1) {
            if (m_pAssets[i]->assetID == m_pDialogLink->speakers[shotNum]) {
                speakerIdx = i;
            }
        }
        if (speakerIdx == -1) {
            speakerIdx = 0;
            m_pYmlManager->error("ShotManager: actor not in shot assets: " + m_pYmlManager->sceneGlobals()->getName(m_pDialogLink->speakers[shotNum]));
        }
        speakerIdx = speakerIdx % colorDgViewActors.count();
        actorCueLabel->setBrush( QBrush(colorDgViewActors[speakerIdx]) );
    }

    m_pDialogScene->addItem(actorCueLabel);
    m_dialogCueRectByShotname[shotName] = actorCueLabel;
    actorCueLabel->setData("shotName", shotName);
    updateDialogCueText(shotName);
    connect(actorCueLabel, SIGNAL(contextEvent(QPointF)), this, SLOT(onShotContextEvent(QPointF)));

    /* draw action blocks for shot in editor */
    upn(i, 0, m_pAssets.count() - 1) {
        m_pAssets[i]->actionRectsByShotName[shotName] = QSet<CustomRectItem*>();
        m_pAssets[i]->pView->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
    }
    upn(j, 0, m_pDialogLink->shots[shotNum].actions.count() - 1) {
        onShotActionLoad(shotNum, j);
    }
    upn(i, 0, m_pAssets.count() - 1) {
        m_pAssets[i]->pView->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    }
}

void ShotManager::onShotActionLoad(int shotNum, int actionNum) {
    double start_sec = m_pDialogLink->getStartTimeForShot(shotNum);
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
    m_pAssetByID[assetID]->actionRectsByShotName[shotName].insert(actionRect);
    connect(actionRect, SIGNAL(contextEvent(QPointF)), this, SLOT(onShotActionContextEvent(QPointF)));

    /*m_pYmlManager->info(QString("Add shot: [%1] %2, assetID = %3")
                        .arg(sa->start, 0, 'f', 3 )
                        .arg( EShotActionToString[sa->actionType] )
                        .arg(assetID));*/
}

void ShotManager::onShotActionAdd(int shotNum, int assetID, EShotActionType actionType, double shotPoint)
{
    m_pYmlManager->info( QString("onShotActionAdd: shotNum = %1 [%2], assetID = %3, actionType = %4")
                         .arg(shotNum).arg(shotPoint).arg(assetID).arg(EShotActionToString[actionType]));
    shotAction sh(actionType, shotPoint);
    if (assetID >= 0) {
        sh.values["actor"] = assetID;
    }
    // TODO! Use input UI interface to get default shotAction for this type
    m_pDialogLink->shots[shotNum].actions.pb(sh);
    m_pDialogLink->shots[shotNum].sortActionsByStart();
    m_pYmlManager->updateShot(m_sectionName, shotNum);

    onShotActionLoad(shotNum, m_pDialogLink->shots[shotNum].actions.count() - 1);
}

void ShotManager::onShotActionRemove(CustomRectItem *rect, bool updateYML)
{
    //shotAction* sa = &m_pDialogLink->shots[shotNum].actions[actionNum];
    /*
     * Remove from:
     * m_pAssetByID[assetID]->pScene
     * m_pAssetByID[assetID]->actionRects
     * m_blocksByShotName[shotName]
     */
    int assetID = rect->data("assetID").toInt();
    QString shotName = rect->data("shotName").toString();
    shotAction* sa = rect->getShotAction();
    qDebug() << "onShotActionRemove: [" << sa->start << "] " << EShotActionToString[sa->actionType] << " from " << shotName;

    m_pAssetByID[assetID]->actionRectsByShotName[shotName].remove(rect);
    m_pAssetByID[assetID]->pScene->removeItem(rect);
    delete rect;
    int shotNum = m_pDialogLink->shotNumByName(shotName);

    upn(i, 0, m_pDialogLink->shots[shotNum].actions.count() - 1) {
        if (&m_pDialogLink->shots[shotNum].actions[i] == sa) {
            m_pDialogLink->shots[shotNum].actions.removeAt(i);
            break;
        }
    }
    if (updateYML) {
        m_pYmlManager->updateShot(m_sectionName, shotName);
    }
}

void ShotManager::onAssetLoad(int assetID)
{
    QGraphicsView* pNewView = new QGraphicsView(m_pShotWidget);
    pNewView->viewport()->setMouseTracking(true);
    pNewView->setTransformationAnchor(QGraphicsView::NoAnchor);
    pNewView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pNewView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pNewView->verticalScrollBar()->setEnabled(false);
    pNewView->horizontalScrollBar()->setEnabled(false);
    QGraphicsScene* pNewScene = new QGraphicsScene(pNewView);
    pNewScene->installEventFilter(m_pShotArea);

    QGraphicsView* pNewViewLabel = new QGraphicsView(m_pShotLabelWidget);
    //pNewView->viewport()->setMouseTracking(true);
    pNewViewLabel->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pNewViewLabel->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pNewViewLabel->verticalScrollBar()->setEnabled(false);
    pNewViewLabel->horizontalScrollBar()->setEnabled(false);
    QGraphicsScene* pNewSceneLabel = new QGraphicsScene(pNewViewLabel);
    //pNewSceneLabel->installEventFilter(m_pShotArea);

    pNewScene->setItemIndexMethod(QGraphicsScene::NoIndex);
    pNewSceneLabel->setItemIndexMethod(QGraphicsScene::NoIndex);
    pNewView->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
    pNewViewLabel->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);

    ShotAsset* newAsset = new ShotAsset(pNewView, pNewScene, pNewViewLabel, pNewSceneLabel, assetID);
    int asset_idx = m_pAssets.count();
    newAsset->isProp = m_pYmlManager->sceneGlobals()->props.contains(assetID);

    /* draw labels */
    QPen bigPen(QSvg::darkslateblue, 2.0, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
    bigPen.setCosmetic(true);

    QVector<QString> groups;
    QVector<EShotActionType> type_samples;
    if (assetID == -1) {
        groups = {"CAMERAS", "ENV/WORLD"};
        type_samples = {EShotCam, EShotEnvBlendIn};
        newAsset->assetName = "";
        pNewScene->setBackgroundBrush( createGradient(QColor::fromHsv(340, 10, 250), QColor::fromHsv(0, 15, 240), sceneWidth(), SHOT_LABEL_HEIGHT * groups.count()) );
    } else if (newAsset->isProp) {
        groups = {"STATE", "PLACEMENT", "SFX"};
        type_samples = {EShotPropShow, EShotPropPlacement, EShotPropEffectStart};
        newAsset->assetName = m_pYmlManager->sceneGlobals()->getName(assetID);
        pNewScene->setBackgroundBrush( createGradient(QColor::fromHsv(130, 10, 250), QColor::fromHsv(150, 15, 240), sceneWidth(), SHOT_LABEL_HEIGHT * groups.count()) );
    } else {
        groups = {"STATE", "ANIMS", "POSE/ADDITIVES", "MIMICS", "LOOKATS", "PLACEMENT", "SFX/VFX"};
        type_samples = {EShotActorShow, EShotActorAnim, EShotActorAnimPose, EShotActorMimicAnim, EShotActorLookat, EShotActorPlacement, EShotActorEffectStart};
        newAsset->assetName = m_pYmlManager->sceneGlobals()->getName(assetID);
        pNewScene->setBackgroundBrush( createGradient(QColor::fromHsv(190, 10, 250), QColor::fromHsv(210, 15, 240), sceneWidth(), SHOT_LABEL_HEIGHT * groups.count()) );
    }
    pNewScene->setSceneRect(0, 0, sceneWidth(), SHOT_LABEL_HEIGHT * groups.count());
    pNewView->setScene(pNewScene);
    pNewView->setFixedWidth(sceneWidth() * m_pShotArea->scaleFactor());
    pNewView->setFixedHeight(SHOT_LABEL_HEIGHT * groups.count());
    m_pShotArea->addShotView(pNewView);

    pNewSceneLabel->setSceneRect(0, 0, SHOT_LABEL_WIDTH, SHOT_LABEL_HEIGHT * groups.count());
    pNewViewLabel->setScene(pNewSceneLabel);
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
            labelRect->setButtonImages(m_pixHide, m_pixShow);
            connect(labelRect, SIGNAL(doubleClick(bool)), this, SLOT(onAssetCollapse(bool)));
        }
        pNewSceneLabel->addItem(labelRect);
    }

    m_pShotWidget->layout()->addWidget(pNewView);
    m_pShotLabelWidget->layout()->addWidget(pNewViewLabel);
    m_pAssets.append(newAsset);
    m_pAssetByID[assetID] = newAsset;
    m_assetIDByScene[pNewScene] = assetID;

    newAsset->repaintNavigationLine();
    onRepaintVerticalLinesForAssetID(assetID);
    onRepaintHorizontalLinesForAssetID(assetID);

    pNewView->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    pNewViewLabel->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    qDebug() << "onAssetLoad [" << m_pAssets.count() << "]: " << assetID;
}

void ShotManager::onAssetChange(int assetID)
{
    // TODO - redraw ?
    //m_assetLabels[ m_knownAssetsID.indexOf(assetID) ].first()->setTextMain( "(" + m_pYmlManager->sceneGlobals()->getName(assetID) + ")" );
}

void ShotManager::onAssetRemove(int assetID)
{
    ShotAsset* pAsset = m_pAssetByID[assetID];
    m_assetIDByScene.remove(pAsset->pScene);
    m_pAssetByID.remove(assetID);
    upn(i, 0, m_pAssets.count() - 1) {
        if (m_pAssets[i]->assetID == assetID) {
            m_pAssets.removeAt(i);
            break;
        }
    }

    /*for (QString shotName : pAsset->actionRectsByShotName.keys()) {
        for (CustomRectItem* pRectItem : pAsset->actionRectsByShotName[shotName]) {
            pAsset->pScene->removeItem(pRectItem);
        }
    }
    for (CustomRectItem* pRectItem : pAsset->labelRects) {
        pAsset->pSceneLabel->removeItem(pRectItem);
    }*/
    m_pShotLabelWidget->layout()->removeWidget(pAsset->pViewLabel);
    m_pShotWidget->layout()->removeWidget(pAsset->pView);
    pAsset->clearAll();
}

void ShotManager::onAssetCollapse(bool isCollapsed)
{
    CustomRectItem* rectItem = static_cast<CustomRectItem*>(sender());
    int assetID = rectItem->data("assetID").toInt();
    int groupNum = rectItem->data("groupNum").toInt();
    qDebug() << "assetID: " << assetID << ", groupNum: " << groupNum << ", isCollapsed: " << isCollapsed;
    m_pAssetByID[assetID]->isCollapsed = isCollapsed;
    if (isCollapsed) {
        m_pAssetByID[assetID]->pView->setSceneRect(0, SHOT_LABEL_HEIGHT * groupNum, sceneWidth(), SHOT_LABEL_HEIGHT);
        m_pAssetByID[assetID]->pView->setFixedHeight(SHOT_LABEL_HEIGHT);
        m_pAssetByID[assetID]->pViewLabel->setSceneRect(0, SHOT_LABEL_HEIGHT * groupNum, SHOT_LABEL_WIDTH, SHOT_LABEL_HEIGHT);
        m_pAssetByID[assetID]->pViewLabel->setFixedHeight(SHOT_LABEL_HEIGHT);
    } else {
        m_pAssetByID[assetID]->pView->setSceneRect(0, 0, sceneWidth(), SHOT_LABEL_HEIGHT * (m_pAssetByID[assetID]->isProp ? m_groupsPropNumMax : m_groupsActorNumMax));
        m_pAssetByID[assetID]->pView->setFixedHeight(SHOT_LABEL_HEIGHT * (m_pAssetByID[assetID]->isProp ? m_groupsPropNumMax : m_groupsActorNumMax));
        m_pAssetByID[assetID]->pViewLabel->setSceneRect(0, 0, sceneWidth(), SHOT_LABEL_HEIGHT * (m_pAssetByID[assetID]->isProp ? m_groupsPropNumMax : m_groupsActorNumMax));
        m_pAssetByID[assetID]->pViewLabel->setFixedHeight(SHOT_LABEL_HEIGHT * (m_pAssetByID[assetID]->isProp ? m_groupsPropNumMax : m_groupsActorNumMax));
    }
    // TODO - redraw
}

void ShotManager::onSceneContextEvent(QGraphicsScene *pScene, QPoint screenPos, QPointF scenePos)
{
    if (pScene == m_pDialogScene) {
        QMenu menu;
        QAction *addShotActionFirst = menu.addAction("Add new shot BEFORE first");
        QAction *addShotActionLast = menu.addAction("Add new shot AFTER last");
        QAction *selectedAction = menu.exec(screenPos);
        if (selectedAction == addShotActionFirst) {
            onShotAdd( 0 );
        } else if (selectedAction == addShotActionLast) {
            onShotAdd( m_pDialogLink->shots.count() );
        }
        return;
    }
    upn(i, 0, m_pAssets.count() - 1) {
        if (pScene == m_pAssets[i]->pScene) {
            QMenu menu;
            QVector<QAction*> pActions;
            QVector<EShotActionType> pAvailableActionsVec;

            int groupNum = Y_TO_GroupNum(scenePos.y());
            int shotNum = X_TO_ShotNum(scenePos.x());
            double shotPoint = X_TO_ShotPoint(scenePos.x());

            if (m_pAssets[i]->assetID == -1) {
                if (groupNum >= GroupNumToEShotActionShared.count()) {
                    qDebug() << "onSceneContextEvent: warning: out of range: groupNum: " << groupNum;
                    return;
                }
                // camera, env
                pAvailableActionsVec = GroupNumToEShotActionShared[groupNum];
            } else if (m_pAssets[i]->isProp) {
                if (groupNum >= GroupNumToEShotActionProp.count()) {
                    qDebug() << "onSceneContextEvent: warning: out of range: groupNum: " << groupNum;
                    return;
                }
                // prop
                pAvailableActionsVec = GroupNumToEShotActionProp[groupNum];
            } else {
                if (groupNum >= GroupNumToEShotActionActor.count()) {
                    qDebug() << "onSceneContextEvent: warning: out of range: groupNum: " << groupNum;
                    return;
                }
                // actor
                pAvailableActionsVec = GroupNumToEShotActionActor[groupNum];
            }
            if (shotNum < 0) {
                qDebug() << "onSceneContextEvent: warning: out of range: shotNum: " << shotNum;
                return;
            }

            upn(j, 0, pAvailableActionsVec.count() - 1) {
                QAction* pAction = new QAction("Add shot action: " + EShotActionToString[ pAvailableActionsVec[j] ]);
                pAction->setData( pAvailableActionsVec[j] );
                menu.addAction(pAction);
                pActions.pb(pAction);
            }

            QAction *selectedAction = menu.exec(screenPos);
            upn(j, 0, pActions.count() - 1) {
                if (selectedAction == pActions[j]) {
                    onShotActionAdd(shotNum, m_assetIDByScene[pScene], pAvailableActionsVec.at(j), shotPoint);
                    break;
                }
            }
            return;
        }
    }
}

void ShotManager::onShotContextEvent(QPointF screenPos)
{
    CustomRectItem* rect = qobject_cast<CustomRectItem*>(sender());
    QString shotName = rect->data("shotName").toString();
    if (m_sectionType == choiceS)
        return;

    QMenu menu;
    QAction *addActionBEFORE = menu.addAction("Add new shot BEFORE this");
    QAction *addActionAFTER = menu.addAction("Add new shot AFTER this");
    QAction *removeAction = menu.addAction("Delete this shot");
    // TODO: more actions here
    QAction *selectedAction = menu.exec(screenPos.toPoint());

    if (selectedAction == removeAction) {
        QMessageBox msgBox;
        msgBox.setText("Are you sure want to delete shot [" + shotName + "] ?");
        msgBox.setInformativeText("It will delete its' dialog line/PAUSE and storyboard actions.");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int selected = msgBox.exec();

        if ( selected == QMessageBox::Yes ) {
            onShotRemove(shotName);
        }
    } else if (selectedAction == addActionBEFORE) {
        onShotAdd( m_pDialogLink->shotNumByName(shotName) );
    } else if (selectedAction == addActionAFTER) {
        onShotAdd( m_pDialogLink->shotNumByName(shotName) + 1 );
    }
}

void ShotManager::onShotAdd(int newShotNum)
{
    m_pYmlManager->addShot(m_sectionName, newShotNum);

    updateHorizontalAdvance();
    // TODO! reload dg rects part only + action rects
    onLoadSectionShots(m_sectionName);
}

void ShotManager::onShotRename(QString oldShotName, QString newShotName) {
    m_actionRectsByShotName[newShotName] = m_actionRectsByShotName[oldShotName];
    m_actionRectsByShotName.remove(oldShotName);
    for (CustomRectItem* rectItem : m_actionRectsByShotName[newShotName]) {
        rectItem->setData("shotName", newShotName);
    }

    // TODO m_ymlManager->renameShot(shotName);
}
void ShotManager::onShotRemove(QString shotName) {
    upn(i, 0, m_pAssets.count() - 1) {  // ? needed
        for (CustomRectItem* pRectItem : m_pAssets[i]->actionRectsByShotName[shotName]) {
            m_pAssets[i]->pScene->removeItem( pRectItem );
            pRectItem->deleteLater();
        }
        m_pAssets[i]->actionRectsByShotName.remove(shotName);
    }

    m_pYmlManager->removeShot(m_sectionName, shotName);
    updateHorizontalAdvance();
    // TODO! reload dg rects part only + action rects
    onLoadSectionShots(m_sectionName);
}

void ShotManager::onShotActionContextEvent(QPointF screenPos)
{
    CustomRectItem* rect = qobject_cast<CustomRectItem*>(sender());
    //int assetID = rect->data("assetID").toInt();
    QMenu menu;
    QAction *removeAction = menu.addAction("Delete action");
    QAction *selectedAction = menu.exec(screenPos.toPoint());
    if (selectedAction == removeAction) {
        onShotActionRemove(rect, true);
    }
}
