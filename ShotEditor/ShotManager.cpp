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

ShotManager::~ShotManager()
{
    //onClearEditor();
}

void ShotManager::setWidgets(QGraphicsScene *newDialogScene, QResizableStackedWidget* newEditorStackedWidget, QScrollArea *newShotLabelArea, ShotScrollArea *newShotArea)
{
    m_pDialogScene = newDialogScene;
    m_pEditorStackedWidget = newEditorStackedWidget;
    m_pShotLabelArea = newShotLabelArea;
    m_pShotArea = newShotArea;
    m_pShotLabelWidget = newShotLabelArea->widget();
    m_pShotWidget = newShotArea->widget();

    m_pShotLabelArea->verticalScrollBar()->setEnabled(false);
    m_pShotLabelArea->setFixedWidth(CONSTANTS::SHOT_LABEL_WIDTH);
    m_pDialogScene->setSceneRect(0,0, CONSTANTS::SHOT_SCENE_WIDTH, CONSTANTS::SHOT_DG_HEIGHT);
    m_pDialogScene->views().first()->setFixedHeight(CONSTANTS::SHOT_DG_HEIGHT);

    /* one-time constant UI things - shared scene */
    m_pixShow = QPixmap(":/show.png").scaledToHeight(20, Qt::SmoothTransformation);
    m_pixHide = QPixmap(":/hide.png").scaledToHeight(20, Qt::SmoothTransformation);
}

bool ShotManager::isAssetSpecificType(EShotActionType type)
{
    return !CONSTANTS::EShotActionsShared.contains(type);
}

void ShotManager::updateDialogCueText(QString shotname) {
    int idx = m_pDialogLink->shotNumByName(shotname);
    QString cleanLine = m_pYmlManager->getCleanLine(m_pDialogLink->lines[idx]);
    QString durationStr = QString::number(m_pDialogLink->durations[idx], 'f', 3);

    m_dialogCueRectByShotname[shotname]->setTextMain(cleanLine);
    m_dialogCueRectByShotname[shotname]->setTextSecondary(durationStr);
}

QColor ShotManager::colorForActionType(EShotActionType type)
{
    switch (type) {
        case EShotCam:
            return CONSTANTS::QColorLight(0, 18);
        case EShotCamBlendStart:
        case EShotCamBlendKey:
        case EShotCamBlendTogame:
            return CONSTANTS::QColorLight(14, 15);
        case EShotCamBlendEnd:
            return CONSTANTS::QColorLightGray(14, 15);

        case EShotEnvBlendIn:
        case EShotFadeIn:
        case EShotWorldAddfact:
        case EShotWorldWeather:
        case EShotWorldEffectStart:
            return CONSTANTS::QColorLight(12, 15);
        case EShotEnvBlendOut:
        case EShotFadeOut:
        case EShotWorldEffectStop:
            return CONSTANTS::QColorLightGray(12, 15);

        case EShotActorAppearance:
        case EShotActorGamestate:
            return CONSTANTS::QColorLight(1, 15);
        case EShotActorScabbardShow:
        case EShotActorShow:
        case EShotPropShow:
        case EShotActorEquipRight:
        case EShotActorEquipLeft:
            return CONSTANTS::QColorLight(2, 15);
        case EShotActorScabbardHide:
        case EShotActorUnequipRight:
        case EShotActorUnequipLeft:
        case EShotActorHide:
        case EShotPropHide:
            return CONSTANTS::QColorLightGray(2, 15);

        case EShotActorAnim:
            return CONSTANTS::QColorLight(3, 15);
        case EShotActorAnimAdditive:
            return CONSTANTS::QColorLight(4, 15);
        case EShotActorAnimPose:
            return CONSTANTS::QColorLight(5, 15);

        case EShotActorMimicAnim:
            return CONSTANTS::QColorLight(6, 15);
        case EShotActorMimicPose:
            return CONSTANTS::QColorLight(7, 15);

        case EShotActorLookat:
            return CONSTANTS::QColorLight(8, 15);

        case EShotActorPlacement:
        case EShotPropPlacement:
            return CONSTANTS::QColorLight(9, 15);
        case EShotActorPlacementStart:
        case EShotActorPlacementKey:
        case EShotPropPlacementStart:
        case EShotPropPlacementKey:
            return CONSTANTS::QColorLight(10, 15);
        case EShotActorPlacementEnd:
        case EShotPropPlacementEnd:
            return CONSTANTS::QColorLightGray(10, 15);

        case EShotActorSound:
            return CONSTANTS::QColorLight(11, 15);
        case EShotActorEffectStart:
        case EShotPropEffectStart:
            return CONSTANTS::QColorLight(13, 15);
        case EShotPropEffectStop:
        case EShotActorEffectStop:
            return CONSTANTS::QColorLightGray(13, 15);
        default:
            return QColor(180, 180, 180);
    }
}

void ShotManager::updateHorizontalAdvance()
{
    // dialogs
    m_pDialogScene->setSceneRect(0,0, sceneWidth(), CONSTANTS::SHOT_DG_HEIGHT);
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

double ShotManager::getMinYForAction(SA_Base *action)
{
    return CONSTANTS::SHOT_LABEL_HEIGHT * CONSTANTS::EShotActionToGroupNum[action->actionType()];
}

QString ShotManager::shotNameByNum(int shotNum)
{
    return m_pDialogLink->shots[shotNum].shotName;
}

void ShotManager::getShotInfoForPoint(QPoint p, int &shotNum, double &shotCoord, int &actorNum, int &groupNum)
{
    // get groupNum, actorNum (Y)
    /*if (p.y() < CONSTANTS::SHOT_LABEL_HEIGHT * 2) {
        groupNum = p.y() / CONSTANTS::SHOT_LABEL_HEIGHT;
        actorNum = -1;
    } else {
        int y = p.y();
        upn(i, 0, m_knownAssetsID.count() - 1) {
            if (m_isAssetCollapsed[i]) {
                if (y >= CONSTANTS::SHOT_LABEL_HEIGHT) {
                    y -= CONSTANTS::SHOT_LABEL_HEIGHT;
                } else {
                    groupNum = 0;
                    actorNum = i;
                    break;
                }
            } else {
                if (y >= CONSTANTS::SHOT_LABEL_HEIGHT * m_groupsActorNumMax) {
                    y -= CONSTANTS::SHOT_LABEL_HEIGHT * m_groupsActorNumMax;
                } else {
                    groupNum = y / CONSTANTS::SHOT_LABEL_HEIGHT;
                    actorNum = i;
                    break;
                }
            }
        }
    }

    // get shotNum, shotCoord
    double x_sec = p.x() * CONSTANTS::SHOT_SECOND;
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
/*
double ShotManager::getDurationForAction(SA_Base* sa) {
    double duration = -1.0;
    int id = 0;
    //    animation id may be production/repository anim
    //    duration() will calcualte correct value for any
    switch (sa->actionType()) {
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
*/

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
    return m_pDialogLink->durations.count() - 1;
}

double ShotManager::X_TO_ShotPos(double x)
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
    return 0.999;
}

double ShotManager::ShotPos_TO_X(int shotNum, double shotPoint)
{
    return ShotNum_TO_Xstart(shotNum) + SEC_TO_X( m_pDialogLink->durations[shotNum] * shotPoint );
}

double ShotManager::ShotNum_TO_Xstart(int shotNum)
{
    double sec = 0.0;
    upn(i, 0, shotNum - 1) {
        sec += m_pDialogLink->durations[i];
    }
    return SEC_TO_X(sec);
}

void ShotManager::onLoadSectionShots(QString sectionName) {
    QElapsedTimer timer;
    timer.start();

    onClearEditor();
    m_pShotArea->setEnabled(true);
    m_pShotLabelArea->setEnabled(true);
    m_pDialogScene->views().first()->setEnabled(true);
    m_sectionName = sectionName;

    m_pDialogLink = m_pYmlManager->m_pDialogLinkBySectionName[sectionName];

    m_pDialogScene->setSceneRect(0,0, sceneWidth(), CONSTANTS::SHOT_DG_HEIGHT);
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
    m_pDialogScene->addLine(0, CONSTANTS::SHOT_DG_HEIGHT * 0.6, sceneWidth(), CONSTANTS::SHOT_DG_HEIGHT * 0.6, QPen(Qt::blue));

    /* draw actor name+phrase+duration label rects and dash lines */
    for (int i = 0; i < m_pDialogLink->lines.count(); ++i) {
        onShotLoad(i);
    }
    onRepaintSecondNumbers();
    onRepaintVerticalLines();

    m_sectionType = m_pYmlManager->getSectionLink(sectionName)->type;
    bool shotsAllowed = (m_sectionType != ESectionScript && m_sectionType != ESectionExit);
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
        current_x += CONSTANTS::SHOT_SECOND;

        QGraphicsSimpleTextItem *textItem = new QGraphicsSimpleTextItem( qn(i) );
        textItem->setPos(current_x - 1.5, CONSTANTS::SHOT_DG_HEIGHT * 0.65);
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
        QGraphicsLineItem* horizontalDash = pAsset->pScene->addLine(0, CONSTANTS::SHOT_LABEL_HEIGHT * i, sceneWidth(), CONSTANTS::SHOT_LABEL_HEIGHT * i, mediumPen);
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
        current_x += CONSTANTS::SHOT_SECOND;
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
    actorCueLabel->setRect(0, 0, duration_sec * CONSTANTS::SHOT_SECOND, CONSTANTS::SHOT_DG_HEIGHT * 0.6);
    actorCueLabel->setPos(start_sec * CONSTANTS::SHOT_SECOND, 0);

    QPen dialogLabelPen(Qt::black, 2.0, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
    dialogLabelPen.setCosmetic(true);
    actorCueLabel->setPen( dialogLabelPen );
    actorCueLabel->setTextParamsMain(QColorConstants::Svg::navy, 9, "Segoe UI", Qt::AlignLeft);
    actorCueLabel->setTextParamsSecondary(QColorConstants::Svg::purple, 9, "DejaVu Sans Mono");

    if (m_pDialogLink->speakers[shotNum] == m_pYmlManager->sceneGlobals()->getID(ERepoAssets, "PAUSE")) {
        actorCueLabel->setBrush( QBrush(CONSTANTS::colorDgViewPause) );
    } else if (m_pDialogLink->speakers[shotNum] == m_pYmlManager->sceneGlobals()->getID(ERepoAssets, "CHOICE")) {
        actorCueLabel->setBrush( QBrush(CONSTANTS::colorDgViewChoice) );
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
        speakerIdx = speakerIdx % CONSTANTS::colorDgViewActors.count();
        actorCueLabel->setBrush( QBrush(CONSTANTS::colorDgViewActors[speakerIdx]) );
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
        //m_pAssets[i]->pView->setCacheMode(QGraphicsView::CacheBackground);
        m_pAssets[i]->pView->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    }
}

void ShotManager::onShotActionLoad(int shotNum, int actionNum) {
    double start_sec = m_pDialogLink->getStartTimeForShot(shotNum);
    double dur_sec = m_pDialogLink->durations[shotNum];
    QString shotName = shotNameByNum(shotNum);

    SA_Base* sa = m_pDialogLink->shots[shotNum].actions[actionNum];
    QString secondaryInfo = QString("[%1]").arg( sa->start(), 0, 'f', 3 );
    double actionDuration = -1.0;

    // connect: click/clone/delete action ?
    CustomRectItem* actionRect = new CustomRectItem;
    if (actionDuration > 0) {
        actionRect->setRect( QRectF(0, 0, actionDuration * CONSTANTS::SHOT_SECOND, CONSTANTS::SHOT_LABEL_HEIGHT / 2.2) );
        secondaryInfo += QString(": %1 s").arg(actionDuration, 0, 'f', 2 );
    } else {
        actionRect->setRect( QRectF(0, 0, 0.5 * CONSTANTS::SHOT_SECOND, CONSTANTS::SHOT_LABEL_HEIGHT / 2.2) );
    }
    double minY = getMinYForAction(sa) + CONSTANTS::SHOT_LABEL_PEN_WIDTH;
    double maxY = minY + CONSTANTS::SHOT_LABEL_HEIGHT - actionRect->rect().height() - CONSTANTS::SHOT_LABEL_PEN_WIDTH;
    actionRect->setBordersRect(QRectF(0, minY, m_pDialogLink->totalDuration * CONSTANTS::SHOT_SECOND - 1.0, maxY - minY));
    actionRect->setPos((start_sec + dur_sec * sa->start()) * CONSTANTS::SHOT_SECOND, QRandomGenerator::global()->bounded((int)minY, (int)maxY + 1));
    actionRect->setDuration(actionDuration);
    actionRect->setFlag(QGraphicsItem::ItemIsMovable);
    actionRect->setFlag(QGraphicsItem::ItemIsSelectable);
    actionRect->setBackgroundColor( colorForActionType(sa->actionType()) );
    actionRect->setTextMain( CONSTANTS::EShotActionToString[sa->actionType()] );
    actionRect->setTextSecondary( secondaryInfo );
    actionRect->setZValue(2.0);
    actionRect->setShotAction( sa );
    actionRect->setData( "shotName", shotName );

    int assetID = sa->assetID();
    m_pAssetByID[assetID]->pScene->addItem(actionRect);
    actionRect->setData( "assetID", assetID );
    m_pAssetByID[assetID]->actionRectsByShotName[shotName].insert(actionRect);
    connect(actionRect, SIGNAL(contextEvent(QPointF)), this, SLOT(onShotActionContextEvent(QPointF)));
    connect(actionRect, SIGNAL(moved(QPointF)), this, SLOT(onShotActionPositionChanged(QPointF)));
    connect(actionRect, SIGNAL(selected(CustomRectItem*)), this, SLOT(onShotActionSelected(CustomRectItem*)));

    /*m_pYmlManager->info(QString("Add shot: [%1] %2, assetID = %3")
                        .arg(sa->start, 0, 'f', 3 )
                        .arg( EShotActionToString[sa->actionType] )
                        .arg(assetID));*/
}

void ShotManager::onShotActionAdd(int shotNum, int assetID, EShotActionType actionType, double shotPoint)
{
    m_pYmlManager->info( QString("onShotActionAdd: shotNum = %1 [%2], assetID = %3, actionType = %4")
                         .arg(shotNum).arg(shotPoint).arg(assetID).arg(CONSTANTS::EShotActionToString[actionType]));
    SA_Base* sh = SA_Base::createShotAction(actionType, m_pYmlManager->sceneGlobals(), shotPoint);
    sh->initDefaults(assetID);
    // TODO! Use input UI interface to get default shotAction for this type
    m_pDialogLink->shots[shotNum].actions.pb(sh);
    m_pDialogLink->shots[shotNum].sortActionsByStart();
    m_pYmlManager->updateShot(m_sectionName, shotNum);

    onShotActionLoad(shotNum, m_pDialogLink->shots[shotNum].actions.count() - 1);
}

void ShotManager::onShotActionRemove(CustomRectItem *pRect, bool updateYML)
{
    int assetID = pRect->data("assetID").toInt();
    QString shotName = pRect->data("shotName").toString();
    SA_Base* sa = pRect->getShotAction();
    qDebug() << "onShotActionRemove: [" << sa->start() << "] " << CONSTANTS::EShotActionToString[sa->actionType()] << " from " << shotName;

    m_pAssetByID[assetID]->actionRectsByShotName[shotName].remove(pRect);
    m_pAssetByID[assetID]->pScene->removeItem(pRect);
    if (m_pSelectedShotAction == pRect) {
        onShotActionSelected(nullptr);
    }
    delete pRect;
    int shotNum = m_pDialogLink->shotNumByName(shotName);

    upn(i, 0, m_pDialogLink->shots[shotNum].actions.count() - 1) {
        if (m_pDialogLink->shots[shotNum].actions[i] == sa) {
            delete m_pDialogLink->shots[shotNum].actions[i];
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
        pNewScene->setBackgroundBrush( createGradient(QColor::fromHsv(340, 10, 250), QColor::fromHsv(0, 15, 240), sceneWidth(), CONSTANTS::SHOT_LABEL_HEIGHT * groups.count()) );
    } else if (newAsset->isProp) {
        groups = {"STATE", "PLACEMENT", "SFX"};
        type_samples = {EShotPropShow, EShotPropPlacement, EShotPropEffectStart};
        newAsset->assetName = m_pYmlManager->sceneGlobals()->getName(assetID);
        pNewScene->setBackgroundBrush( createGradient(QColor::fromHsv(130, 10, 250), QColor::fromHsv(150, 15, 240), sceneWidth(), CONSTANTS::SHOT_LABEL_HEIGHT * groups.count()) );
    } else {
        groups = {"STATE", "ANIMS", "POSE/ADDITIVES", "MIMICS", "LOOKATS", "PLACEMENT", "SFX/VFX"};
        type_samples = {EShotActorShow, EShotActorAnim, EShotActorAnimPose, EShotActorMimicAnim, EShotActorLookat, EShotActorPlacement, EShotActorEffectStart};
        newAsset->assetName = m_pYmlManager->sceneGlobals()->getName(assetID);
        pNewScene->setBackgroundBrush( createGradient(QColor::fromHsv(190, 10, 250), QColor::fromHsv(210, 15, 240), sceneWidth(), CONSTANTS::SHOT_LABEL_HEIGHT * groups.count()) );
    }
    pNewScene->setSceneRect(0, 0, sceneWidth(), CONSTANTS::SHOT_LABEL_HEIGHT * groups.count());
    pNewView->setScene(pNewScene);
    pNewView->setFixedWidth(sceneWidth() * m_pShotArea->scaleFactor());
    pNewView->setFixedHeight(CONSTANTS::SHOT_LABEL_HEIGHT * groups.count());
    m_pShotArea->addShotView(pNewView);

    pNewSceneLabel->setSceneRect(0, 0, CONSTANTS::SHOT_LABEL_WIDTH, CONSTANTS::SHOT_LABEL_HEIGHT * groups.count());
    pNewViewLabel->setScene(pNewSceneLabel);
    pNewViewLabel->setFixedSize(CONSTANTS::SHOT_LABEL_WIDTH, CONSTANTS::SHOT_LABEL_HEIGHT * groups.count());

    /* add label rects */
    upn(i, 0, groups.count() - 1) {
        CustomRectItem* labelRect = new CustomRectItem;
        labelRect->setRect(0, 0, CONSTANTS::SHOT_LABEL_WIDTH - CONSTANTS::SHOT_LABEL_PEN_WIDTH, CONSTANTS::SHOT_LABEL_HEIGHT - CONSTANTS::SHOT_LABEL_PEN_WIDTH);
        labelRect->setPos(0, CONSTANTS::SHOT_LABEL_HEIGHT * i);
        labelRect->setBackgroundColor( CONSTANTS::QColorDark(colorForActionType(type_samples[i]).hsvHue()) );
        labelRect->setPen( QPen(QSvg::honeydew, CONSTANTS::SHOT_LABEL_PEN_WIDTH, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin) );
        labelRect->setTextParamsMain(CONSTANTS::colorDgViewActors[asset_idx % CONSTANTS::colorDgViewActors.count()], 10, "Helvetica", Qt::AlignTop | Qt::AlignHCenter);
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
        m_pAssetByID[assetID]->pView->setSceneRect(0, CONSTANTS::SHOT_LABEL_HEIGHT * groupNum, sceneWidth(), CONSTANTS::SHOT_LABEL_HEIGHT);
        m_pAssetByID[assetID]->pView->setFixedHeight(CONSTANTS::SHOT_LABEL_HEIGHT);
        m_pAssetByID[assetID]->pViewLabel->setSceneRect(0, CONSTANTS::SHOT_LABEL_HEIGHT * groupNum, CONSTANTS::SHOT_LABEL_WIDTH, CONSTANTS::SHOT_LABEL_HEIGHT);
        m_pAssetByID[assetID]->pViewLabel->setFixedHeight(CONSTANTS::SHOT_LABEL_HEIGHT);
    } else {
        m_pAssetByID[assetID]->pView->setSceneRect(0, 0, sceneWidth(), CONSTANTS::SHOT_LABEL_HEIGHT * (m_pAssetByID[assetID]->isProp ? m_groupsPropNumMax : m_groupsActorNumMax));
        m_pAssetByID[assetID]->pView->setFixedHeight(CONSTANTS::SHOT_LABEL_HEIGHT * (m_pAssetByID[assetID]->isProp ? m_groupsPropNumMax : m_groupsActorNumMax));
        m_pAssetByID[assetID]->pViewLabel->setSceneRect(0, 0, sceneWidth(), CONSTANTS::SHOT_LABEL_HEIGHT * (m_pAssetByID[assetID]->isProp ? m_groupsPropNumMax : m_groupsActorNumMax));
        m_pAssetByID[assetID]->pViewLabel->setFixedHeight(CONSTANTS::SHOT_LABEL_HEIGHT * (m_pAssetByID[assetID]->isProp ? m_groupsPropNumMax : m_groupsActorNumMax));
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
            double shotPoint = X_TO_ShotPos(scenePos.x());

            if (m_pAssets[i]->assetID == -1) {
                if (groupNum >= CONSTANTS::GroupNumToEShotActionShared.count()) {
                    qDebug() << "onSceneContextEvent: warning: out of range: groupNum: " << groupNum;
                    return;
                }
                // camera, env
                pAvailableActionsVec = CONSTANTS::GroupNumToEShotActionShared[groupNum];
            } else if (m_pAssets[i]->isProp) {
                if (groupNum >= CONSTANTS::GroupNumToEShotActionProp.count()) {
                    qDebug() << "onSceneContextEvent: warning: out of range: groupNum: " << groupNum;
                    return;
                }
                // prop
                pAvailableActionsVec = CONSTANTS::GroupNumToEShotActionProp[groupNum];
            } else {
                if (groupNum >= CONSTANTS::GroupNumToEShotActionActor.count()) {
                    qDebug() << "onSceneContextEvent: warning: out of range: groupNum: " << groupNum;
                    return;
                }
                // actor
                pAvailableActionsVec = CONSTANTS::GroupNumToEShotActionActor[groupNum];
            }
            if (shotNum < 0) {
                qDebug() << "onSceneContextEvent: warning: out of range: shotNum: " << shotNum;
                return;
            }

            upn(j, 0, pAvailableActionsVec.count() - 1) {
                QAction* pAction = new QAction("Add shot action: " + CONSTANTS::EShotActionToString[ pAvailableActionsVec[j] ]);
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
    if (m_sectionType == ESectionChoice)
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
    upn(i, 0, m_pAssets.count() - 1) {  // ? needed
        if (!m_pAssets[i]->actionRectsByShotName.contains(oldShotName))
            continue;

        m_pAssets[i]->actionRectsByShotName[newShotName] = m_pAssets[i]->actionRectsByShotName[oldShotName];
        m_pAssets[i]->actionRectsByShotName.remove(oldShotName);
        for (CustomRectItem* pRectItem : m_pAssets[i]->actionRectsByShotName[newShotName]) {
            pRectItem->setData("shotName", newShotName);
        }
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
    CustomRectItem* pRect = qobject_cast<CustomRectItem*>(sender());
    // ? onShotActionSelected(pRect);
    //int assetID = rect->data("assetID").toInt();
    QMenu menu;
    QAction *removeAction = menu.addAction("Delete action");
    QAction *selectedAction = menu.exec(screenPos.toPoint());
    if (selectedAction == removeAction) {
        onShotActionRemove(pRect, true);
    }
}

void ShotManager::onShotActionStartChanged(double newShotPos)
{
    QString shotName = m_pSelectedShotAction->data("shotName").toString();
    int shotNum = m_pDialogLink->shotNumByName( shotName );
    //qDebug() << QString("%1, newPos = [%2], shotNum = %3 (%4)").arg(Q_FUNC_INFO).arg(newPos).arg(shotNum).arg(shotName);

    double newX = ShotPos_TO_X(shotNum, newShotPos);
    QPointF scenePos = m_pSelectedShotAction->scenePos();
    scenePos.setX( newX );

    m_pSelectedShotAction->blockSignals(true);
    m_pSelectedShotAction->setPos(scenePos);
    m_pSelectedShotAction->blockSignals(false);
    QSlider* pStartSlider = m_pEditorStackedWidget->currentWidget()->findChild<QSlider*>("m_startSlider");
    if (pStartSlider != nullptr) {
        pStartSlider->blockSignals(true);
        int shotPointInt = newShotPos * 1000.0;
        pStartSlider->setValue(shotPointInt);
        pStartSlider->blockSignals(false);
    }
    onShotActionChanged();
}

void ShotManager::onShotActionStartChanged(int newShotPosStep)
{
    QString shotName = m_pSelectedShotAction->data("shotName").toString();
    int shotNum = m_pDialogLink->shotNumByName( shotName );

    double newShotPos = newShotPosStep * 0.001;
    double newX = ShotPos_TO_X(shotNum, newShotPos);
    QPointF scenePos = m_pSelectedShotAction->scenePos();
    scenePos.setX( newX );

    m_pSelectedShotAction->blockSignals(true);
    m_pSelectedShotAction->setPos(scenePos);
    m_pSelectedShotAction->blockSignals(false);
    QDoubleSpinBox* pStartSpin = m_pEditorStackedWidget->currentWidget()->findChild<QDoubleSpinBox*>("m_startSpin");
    if (pStartSpin != nullptr) {
        pStartSpin->blockSignals(true);
        pStartSpin->setValue(newShotPos);
        pStartSpin->blockSignals(false);
    }
    onShotActionChanged();
}

void ShotManager::onShotActionPositionChanged(QPointF newScenePos)
{
    CustomRectItem* pRect = qobject_cast<CustomRectItem*>(sender());
    onShotActionSelected(pRect);  // ensure selected rect is pRect

    int shotNum = X_TO_ShotNum(newScenePos.x());
    QString shotName = shotNameByNum(shotNum);
    double shotPos = X_TO_ShotPos(newScenePos.x());
    //qDebug() << QString("%1, CALCULATED shotNum = %2 (%3) [%4]").arg(Q_FUNC_INFO).arg(shotNum).arg(shotName).arg(shotPoint);

    QDoubleSpinBox* pStartSpin = m_pEditorStackedWidget->currentWidget()->findChild<QDoubleSpinBox*>("m_startSpin");
    if (pStartSpin != nullptr) {
        pStartSpin->blockSignals(true);
        pStartSpin->setValue(shotPos);
        pStartSpin->blockSignals(false);
    }
    QSlider* pStartSlider = m_pEditorStackedWidget->currentWidget()->findChild<QSlider*>("m_startSlider");
    if (pStartSlider != nullptr) {
        pStartSlider->blockSignals(true);
        int shotPosInt = shotPos * 1000.0;
        pStartSlider->setValue(shotPosInt);
        pStartSlider->blockSignals(false);
    }

    QString oldShotName = m_pSelectedShotAction->data("shotName").toString();
    //qDebug() << QString("%1, oldShotName = %2, newShotName = %3 [%4]").arg(Q_FUNC_INFO).arg(oldShotName).arg(shotName).arg(shotPoint);
    if (shotName != oldShotName) {
        int assetID = m_pSelectedShotAction->data("assetID").toInt();
        m_pAssetByID[assetID]->actionRectsByShotName[oldShotName].remove(pRect);
        //if (!m_pAssetByID[assetID]->actionRectsByShotName.contains(shotName)) TODO remove!
        //    m_pAssetByID[assetID]->actionRectsByShotName[shotName] = QSet<CustomRectItem*>();
        m_pSelectedShotAction->setData("shotName", shotName);
        m_pAssetByID[assetID]->actionRectsByShotName[shotName].insert(pRect);
    }
    onShotActionChanged();
}

void ShotManager::onShotActionChanged()
{
    //qDebug() << QString("%1, sender = [%2] ").arg(Q_FUNC_INFO).arg(sender()->metaObject()->className()) << sender();
    // m_pSelectedItem
    if (m_pSelectedShotAction != nullptr) {
        SA_Base* sa = m_pSelectedShotAction->getShotAction();
        sa->importWidget();
        sa->updateYmlNode();
        QString shotName = m_pSelectedShotAction->data("shotName").toString();
        m_pYmlManager->updateShot(m_sectionName, shotName);
    }
}

void ShotManager::onShotActionSelected(CustomRectItem* pRect)
{
    //qDebug() << QString("%1, sender = ").arg(Q_FUNC_INFO) << sender() << ", pRect = " << qobject_cast<QObject*>(pRect);
    if (m_pSelectedShotAction == pRect)
        return;

    if (m_pSelectedShotAction != nullptr) {
        m_pEditorStackedWidget->currentWidget()->setEnabled(false);
    }
    m_pSelectedShotAction = pRect;
    if (pRect == nullptr)
        return;

    SA_Base* sa = m_pSelectedShotAction->getShotAction();
    m_pEditorStackedWidget->setCurrentIndex(sa->actionType());
    m_pEditorStackedWidget->currentWidget()->setEnabled(true);
    sa->exportWidget( m_pEditorStackedWidget->currentWidget() );
}
