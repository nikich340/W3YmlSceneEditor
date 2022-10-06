#include <QRandomGenerator>
#include "ShotManager.h"

ShotManager::ShotManager(YmlSceneManager* newYmlManager, QObject *parent) : QObject(parent)
{
    m_ymlManager = newYmlManager;
}

void ShotManager::setShotScenes(QGraphicsScene* gDgScene, QGraphicsScene* gLabelScene, QGraphicsScene* gShotScene) {
	pDgScene = gDgScene;
	pLabelScene = gLabelScene;
	pShotScene = gShotScene;

    pShotScene->views()[0]->setMouseTracking(true);
    pShotScene->installEventFilter(this);

    /* one-time constant UI things */

    /* red navigation line */
    m_navigateLine = new QGraphicsLineItem;
    m_navigateLine->setZValue(1.0);
    m_navigateLine->setPen( QPen(Qt::red) );
    m_navigateLine->setPos(0, 0);
    m_navigateLine->setLine(0, 0, 0, SHOT_SCENE_HEIGHT);
    m_navigateLine->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    pShotScene->addItem(m_navigateLine);

    /*addLabel("CAMERAS", EShotCam, SHOT_LABEL_WIDTH / 7, SHOT_LABEL_HEIGHT / 2.8);
    addLabel("ACTOR STATES", EShotActorShow, SHOT_LABEL_WIDTH / 20, SHOT_LABEL_HEIGHT / 2.8);
    addLabel("ACTOR ANIMS", EShotActorAnim, SHOT_LABEL_WIDTH / 20, SHOT_LABEL_HEIGHT / 2.8);
    addLabel("ACTOR MIMICS", EShotActorMimicAnim, SHOT_LABEL_WIDTH / 50, SHOT_LABEL_HEIGHT / 2.8);
    addLabel("ACTOR MISC", EShotActorLookat, SHOT_LABEL_WIDTH / 20, SHOT_LABEL_HEIGHT / 2.8);
    addLabel("PROPS", EShotPropShow, SHOT_LABEL_WIDTH / 5, SHOT_LABEL_HEIGHT / 2.8);
    addLabel("WORLD MISC", EShotWorldWeather, SHOT_LABEL_WIDTH / 20, SHOT_LABEL_HEIGHT /2.8);*/
}

bool ShotManager::eventFilter(QObject *obj, QEvent *event) {
	//qDebug() << "event:" << event->type();
	if (obj == pShotScene && event->type() == QEvent::GraphicsSceneMouseMove) {
		QGraphicsSceneMouseEvent* hoverEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
        if (m_navigateLine != nullptr)
            m_navigateLine->setPos( hoverEvent->scenePos().x(), 0 );
	}
    return false;
}

bool ShotManager::isActorSpecificType(EShotActionType type)
{
    switch (type) {
        case EShotCam:
        case EShotCamBlendStart:
        case EShotCamBlendKey:
        case EShotCamBlendEnd:
        case EShotCamBlendTogame:
        case EShotEnvBlendIn:
        case EShotEnvBlendOut:
        case EShotFadeIn:
        case EShotFadeOut:
        case EShotWorldAddfact:
        case EShotWorldWeather:
        case EShotWorldEffectStart:
        case EShotWorldEffectStop:
            return false;
        default:
            return true;
    }
}

void ShotManager::clearShotEditor() {
    pLabelScene->clear();
	pDgScene->clear();
	pShotScene->clear();

    m_dgCueRectByShotname.clear();
    m_knownActors.clear();
    m_isActorCollapsed.clear();
    m_actorLabels.clear();
    m_navigateLine = nullptr;
}

/* actually update */
void ShotManager::updateCueText(QString shotname) {
	int idx = currentDgLink->getIdx(shotname);
    QString cleanLine = m_ymlManager->getCleanLine(currentDgLink->lines[idx]);
    QString durationStr = QString::number(currentDgLink->durations[idx], 'f', 3);

    m_dgCueRectByShotname[shotname]->setTextMain(cleanLine);
    m_dgCueRectByShotname[shotname]->setTextSecondary(durationStr);
}

/* add labels on the left */
void ShotManager::addLabel(QString text, EShotActionType type, double offsetX, double offsetY) {
    /*double minY = getMinYForType(type);
    QPen penLabel(QColor("#B3FFCF"), SHOT_LABEL_PEN_WIDTH, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);

    QGraphicsRectItem* labelRect = new QGraphicsRectItem;
    labelRect->setRect(0, 0, SHOT_LABEL_WIDTH - SHOT_LABEL_PEN_WIDTH * 2, SHOT_LABEL_HEIGHT - SHOT_LABEL_PEN_WIDTH);
    labelRect->setPos(0, minY);
    labelRect->setBrush( Qt::darkBlue );
    labelRect->setPen( penLabel );
    pLabelScene->addItem(labelRect);

    QGraphicsTextItem* textLabel = new QGraphicsTextItem(labelRect);
    textLabel->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    textLabel->setPos(offsetX, offsetY);
    textLabel->setHtml(QString("<body style=\"font-family:\'%1\'; font-size:%2pt;\"><font color=\"%3\">%4</font>")
                       .arg("Segoe UI")
                       .arg(12)
                       .arg("#000090")
                       .arg(text));
    labelItemByText[text] = textLabel;*/
}

int ShotManager::getGroupNumForType(EShotActionType type) {
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
        /* for every actor*/
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

        /* for every prop */
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
}

double ShotManager::getMinYForAction(shotAction *action)
{
    int actor_id = getActorIdForAction(action);
    qDebug() << QString("getMinYForAction: %1, isActorSpecificType = %2, actor id = %3").arg(EShotActionToString[action->actionType]).arg(isActorSpecificType(action->actionType)).arg(actor_id);
    if ( isActorSpecificType(action->actionType) && actor_id >= 0 ) {
        int actor_idx = m_knownActors.indexOf(actor_id);
        qDebug() << QString("getMinYForAction: actor IDX = %1").arg(actor_idx);
        return SHOT_LABEL_HEIGHT * (2 + actor_idx * m_groupsCount + getGroupNumForType(action->actionType) );
    }
    return SHOT_LABEL_HEIGHT * getGroupNumForType(action->actionType);
    // Y_max = Y_min + SHOT_DG_HEIGHT - SHOT_LABEL_PEN_WIDTH - 1;
}

int ShotManager::getActorIdForAction(shotAction *action)
{
    if ( !isActorSpecificType(action->actionType) )
        return -1;

    if (action->values.contains("actor"))
        return action->values.value("actor").toInt();

    int id;
    switch (action->actionType) {
        case EShotActorAnim:
        case EShotActorAnimAdditive:
            id = action->values["animation"].toInt();
            return m_ymlManager->sceneGlobals()->anims[id].actorID;
            break;
        case EShotActorMimicAnim:
            id = action->values["animation"].toInt();
            return m_ymlManager->sceneGlobals()->mimics[id].actorID;
            break;
        case EShotActorAnimPose:
            id = action->values["pose"].toInt();
            return m_ymlManager->sceneGlobals()->poses[id].actorID;
            break;
        case EShotActorMimicPose:
            id = action->values["mimic"].toInt();
            return m_ymlManager->sceneGlobals()->mimic_poses[id].actorID;
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
            duration = m_ymlManager->sceneGlobals()->anims[id].duration();
            break;
        case EShotActorMimicAnim:
            id = sa->values["animation"].toInt();
            duration = m_ymlManager->sceneGlobals()->mimics[id].duration();
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

    m_ymlManager->info(QString("onLoadShots()>: section [%1]")
                     .arg(sectionName));
    currentDgLink = &(m_ymlManager->dgLinkBySectionName[sectionName]);

    /* set scene sizes */
    double sceneWidth = qMax(SHOT_SCENE_WIDTH, currentDgLink->totalDuration * 2.0 * SHOT_SECOND);
    double sceneHeight = qMax(SHOT_SCENE_HEIGHT, SHOT_LABEL_HEIGHT * (2 + m_ymlManager->sceneGlobals()->actors.count() * m_groupsCount));

    pDgScene->setSceneRect(0,0, sceneWidth, SHOT_DG_HEIGHT);
    pDgScene->views().at(0)->setMaximumWidth(sceneWidth);

    pShotScene->setSceneRect(0,0, sceneWidth, sceneHeight);
    pShotScene->views().at(0)->setMaximumWidth(sceneWidth);
    pShotScene->views().at(0)->setMaximumHeight(sceneHeight);

    pLabelScene->setSceneRect(0,0, SHOT_LABEL_WIDTH, sceneHeight);
    pLabelScene->views().at(0)->setMaximumHeight(sceneHeight);

    /* camera & env groups */
    upn(i, 0, 1) {
        CustomRectItem* labelRect = new CustomRectItem;
        labelRect->setRect(0, 0, SHOT_LABEL_WIDTH - SHOT_LABEL_PEN_WIDTH * 2, SHOT_LABEL_HEIGHT - SHOT_LABEL_PEN_WIDTH);
        labelRect->setPos(0, SHOT_LABEL_HEIGHT * i);
        labelRect->setBackgroundColor( i == 0 ? QSvg::crimson : QSvg::saddlebrown );
        labelRect->setPen( QPen(QSvg::honeydew, SHOT_LABEL_PEN_WIDTH, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin) );
        labelRect->setTextParamsSecondary(QColorConstants::Svg::gold, 10, "Sans Serif", Qt::AlignCenter | Qt::TextWordWrap);
        labelRect->setTextSecondary( i == 0 ? "CAMERAS" : "ENV/WORLD" );
        pLabelScene->addItem( labelRect );
    }
    QPen bigPen(QSvg::darkslateblue, 2.0, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
    QPen mediumPen(QSvg::darkolivegreen, 1.0, Qt::DashLine, Qt::FlatCap, Qt::BevelJoin);
    bigPen.setCosmetic(true);
    mediumPen.setCosmetic(true);
    pShotScene->addLine(0, SHOT_LABEL_HEIGHT * 1, sceneWidth, SHOT_LABEL_HEIGHT * 1, mediumPen);
    pShotScene->addLine(0, SHOT_LABEL_HEIGHT * 2, sceneWidth, SHOT_LABEL_HEIGHT * 2, bigPen);

    /* load actors */
    for (asset actor : m_ymlManager->sceneGlobals()->actors) {
        qDebug() << "actor: " << m_ymlManager->sceneGlobals()->getName( actor.nameID );
        onActorAdd(actor.nameID);
    }

    /* draw seconds and dash lines */
	double cur_x = 0;
    QPen mediumPen2(QSvg::darkslateblue, 1.0, Qt::DashLine, Qt::FlatCap, Qt::BevelJoin);
    mediumPen2.setCosmetic(true);
    QFont fontNum("Arial", 9);
	QBrush brush(Qt::darkGreen);

	for (int i = 1; i < currentDgLink->totalDuration * 2.0; ++i) {
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

	for (int i = 0; i < currentDgLink->lines.size(); ++i) {
        onShotLoad(i);
	}
}

void ShotManager::onShotLoad(QString shotName) {
    upn(i, 0, currentDgLink->shots.size() - 1) {
        if (currentDgLink->shots[i].shotName == shotName) {
            onShotLoad(i);
            return;
        }
    }
}

void ShotManager::onShotLoad(int shotNum) {
    double start_sec = currentDgLink->getStartTimeForLine(shotNum);
    double duration_sec = currentDgLink->durations[shotNum];
    QString shotName = currentDgLink->shots[shotNum].shotName;

    CustomRectItem* actorCueLabel = new CustomRectItem;
    actorCueLabel->setRect(0, 0, duration_sec * SHOT_SECOND, SHOT_DG_HEIGHT * 0.6);
    actorCueLabel->setPos(start_sec * SHOT_SECOND, 0);

    QPen dialogLabelPen(Qt::black, 2.0, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
    dialogLabelPen.setCosmetic(true);
    actorCueLabel->setPen( dialogLabelPen );
    actorCueLabel->setTextParamsMain(QColorConstants::Svg::navy, 9, "Segoe UI", Qt::AlignLeft);
    actorCueLabel->setTextParamsSecondary(QColorConstants::Svg::purple, 9, "DejaVu Sans Mono");

    if (currentDgLink->speakers[shotNum] == m_ymlManager->sceneGlobals()->getID(SACTORS, "PAUSE")) {
        actorCueLabel->setBrush( QBrush(colorDgViewPause) );
    } else if (currentDgLink->speakers[shotNum] == m_ymlManager->sceneGlobals()->getID(SACTORS, "CHOICE")) {
        actorCueLabel->setBrush( QBrush(colorDgViewChoice) );
    } else {
        //QString speakerName = m_ymlManager->sceneGlobals()->getName(currentDgLink->speakers[shotNum]);
        int speakerIdx = m_knownActors.indexOf( currentDgLink->speakers[shotNum] );
        if (speakerIdx == -1) {
            speakerIdx = 0;
            m_ymlManager->error("ShotManager: actor not in m_knownActors: " + m_ymlManager->sceneGlobals()->getName(currentDgLink->speakers[shotNum]));
        }
        speakerIdx = speakerIdx % colorDgViewActors.count();
        actorCueLabel->setBrush( QBrush(colorDgViewActors[speakerIdx]) );
    }

    pDgScene->addItem(actorCueLabel);
    m_dgCueRectByShotname[shotName] = actorCueLabel;
    updateCueText(shotName);

    /* draw action blocks for shot in editor */
    m_blocksByShotName[shotName] = QVector<CustomRectItem*>();
    upn(j, 0, currentDgLink->shots[shotNum].actions.size() - 1) {
        onLoadShotAction(shotNum, j);
    }

    /* vertical line after shot */
    pShotScene->addLine((start_sec + duration_sec) * SHOT_SECOND, 0, (start_sec + duration_sec) * SHOT_SECOND, pShotScene->sceneRect().height(), dialogLabelPen);
}

void ShotManager::onLoadShotAction(int shotNum, int actionNum) {
    double start_sec = currentDgLink->getStartTimeForLine(shotNum);
    double dur_sec = currentDgLink->durations[shotNum];

    shotAction* sa = &currentDgLink->shots[shotNum].actions[actionNum];
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
    actionRect->setBordersRect(QRectF(0, minY, currentDgLink->totalDuration * SHOT_SECOND, maxY - minY));
    actionRect->setPos((start_sec + dur_sec * sa->start) * SHOT_SECOND, QRandomGenerator::global()->bounded((int)minY, (int)maxY + 1));
    actionRect->setDuration(actionDuration);
    actionRect->setFlag(QGraphicsItem::ItemIsMovable);
    actionRect->setFlag(QGraphicsItem::ItemIsSelectable);
    actionRect->setBackgroundColor( actionColors[sa->actionType] );
    actionRect->setTextMain( EShotActionToString[sa->actionType] );
    actionRect->setTextSecondary( secondaryInfo );
    actionRect->setZValue(2.0);
    pShotScene->addItem(actionRect);
    m_blocksByShotName[ currentDgLink->shots[shotNum].shotName ].append(actionRect);
    m_blocksByActorId[ getActorIdForAction(sa) ].append(actionRect);

    m_ymlManager->info(QString("shot: [%1] %2")
                     .arg(sa->start, 0, 'f', 3 )
                       .arg( EShotActionToString[sa->actionType] ));
}

void ShotManager::onActorAdd(int actorID)
{
    m_blocksByActorId[actorID] = QVector<CustomRectItem*>();
    m_actorLabels.append( QVector<CustomRectItem*>() );
    m_knownActors.append( actorID );
    int actor_idx = m_knownActors.count() - 1;
    /* draw labels */

    double startY = SHOT_LABEL_HEIGHT * (2 + actor_idx * m_groupsCount);
    QPen bigPen(QSvg::darkslateblue, 2.0, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
    QPen mediumPen(QSvg::darkolivegreen, 1.0, Qt::DashLine, Qt::FlatCap, Qt::BevelJoin);
    bigPen.setCosmetic(true);
    mediumPen.setCosmetic(true);
    QVector<QString> groups = {"STATE", "ANIMS", "POSE/ADDITIVES", "MIMICS", "LOOKATS", "PLACEMENT", "SFX/VFX"};
    QVector<QColor> colors = {QSvg::dimgray, QSvg::darkslateblue, QSvg::navy, QSvg::darkgreen, QSvg::indigo, QSvg::deeppink, QSvg::maroon};
    upn(i, 0, groups.count() - 1) {
        CustomRectItem* labelRect = new CustomRectItem;
        labelRect->setRect(0, 0, SHOT_LABEL_WIDTH - SHOT_LABEL_PEN_WIDTH * 2, SHOT_LABEL_HEIGHT - SHOT_LABEL_PEN_WIDTH);
        labelRect->setPos(0, startY + SHOT_LABEL_HEIGHT * i);
        labelRect->setBackgroundColor( colors[i] );
        labelRect->setPen( QPen(QSvg::honeydew, SHOT_LABEL_PEN_WIDTH, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin) );
        labelRect->setTextParamsMain(colorDgViewActors[actor_idx % colorDgViewActors.count()], 10, "Helvetica", Qt::AlignTop | Qt::AlignHCenter);
        labelRect->setTextParamsSecondary(QColorConstants::Svg::gold, 10, "Sans Serif", Qt::AlignVCenter | Qt::AlignHCenter | Qt::TextWordWrap);
        labelRect->setTextSecondary( groups[i] );
        //if (i == 0)
        labelRect->setTextMain( m_ymlManager->sceneGlobals()->getName(actorID) );
        pLabelScene->addItem( labelRect );
        m_actorLabels[actor_idx].append( labelRect );
        /* horizontal separator */
        if (i > 0)
            pShotScene->addLine(0, startY + SHOT_LABEL_HEIGHT * i, pShotScene->sceneRect().width(), startY + SHOT_LABEL_HEIGHT * i, mediumPen);
    }

    /* horizontal line before/after actor */
    pShotScene->addLine(0, startY + SHOT_LABEL_HEIGHT * m_groupsCount, pShotScene->sceneRect().width(), startY + SHOT_LABEL_HEIGHT * m_groupsCount, bigPen);
    qDebug() << "onActorAdd [" << m_knownActors.indexOf(actorID) << "]: " << actorID;
    // TODO
}

void ShotManager::onActorChange(int actorID)
{
    // TODO
    m_actorLabels[ m_knownActors.indexOf(actorID) ].first()->setTextMain( "(" + m_ymlManager->sceneGlobals()->getName(actorID) + ")" );
}

void ShotManager::onActorRemove(int actorID)
{
    for (CustomRectItem* block : m_blocksByActorId[actorID]) {
        pShotScene->removeItem(block);
        block->deleteLater();
    }
    m_blocksByActorId.remove(actorID);
    m_actorLabels.remove( m_knownActors.indexOf(actorID) );
    m_knownActors.remove( m_knownActors.indexOf(actorID) );
    // TODO
}

void ShotManager::onActorCollapse(int actorID)
{
    m_isActorCollapsed[ m_knownActors.indexOf(actorID) ] = true;
    // TODO
}

void ShotManager::onActorUncollapse(int actorID)
{
    m_isActorCollapsed[ m_knownActors.indexOf(actorID) ] = false;
    // TODO
}

void ShotManager::onShotRename(QString oldName, QString newName) {
    m_blocksByShotName[newName] = m_blocksByShotName[oldName];
    m_blocksByShotName.remove(oldName);
    // TODO
}
void ShotManager::onShotRemove(QString shotName) {
    for (CustomRectItem* block : m_blocksByShotName[shotName]) {
        pShotScene->removeItem(block);
        block->deleteLater();
    }
    m_blocksByShotName.remove(shotName);
    // TODO m_ymlManager->removeShot(shotName);
}
