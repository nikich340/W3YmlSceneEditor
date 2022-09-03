#include "ShotManager.h"

ShotManager::ShotManager(YmlSceneManager* newYmlManager, QObject *parent) : QObject(parent)
{
	ymlManager = newYmlManager;
}

void ShotManager::setShotScenes(QGraphicsScene* gDgScene, QGraphicsScene* gLabelScene, QGraphicsScene* gShotScene) {
	pDgScene = gDgScene;
	pLabelScene = gLabelScene;
	pShotScene = gShotScene;

	pShotScene->views()[0]->setMouseTracking(true);
	pShotScene->installEventFilter(this);
}

bool ShotManager::eventFilter(QObject *obj, QEvent *event) {
	//qDebug() << "event:" << event->type();
	if (obj == pShotScene && event->type() == QEvent::GraphicsSceneMouseMove) {
		QGraphicsSceneMouseEvent* hoverEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
		if (navigateLine != nullptr)
			navigateLine->setPos( hoverEvent->scenePos().x(), 0 );
	}
	return false;
}

void ShotManager::clearShotEditor() {
	pLabelScene->clear();
	pDgScene->clear();
	pShotScene->clear();

	dgElementByShotname.clear();
	dgLabelByShotname.clear();
	navigateLine = nullptr;
}

void ShotManager::updateDgLabel(QString shotname, double factor) {
	QString labelText = " " + shotname;
	int idx = currentDgLink->getIdx(shotname);
	QString cleanLine = " " + ymlManager->getCleanLine(currentDgLink->lines[idx]);
	QString dur = QString::number( currentDgLink->durations[idx], 'f', 3);

	//QRectF rect = dgElementByShotname[shotname]->rect();
	//QFontMetricsF fm (QFont("Arial", 8) );

	//dgLabelByShotname[shotname]->setHtml(labelText + "<br>" + cleanLine + "<br>");

	// yeah, binary search
	/*int L = 1, R = qMax(cleanLine.length(), labelText.length());
	QString labelText2 = labelText;
	QString cleanLine2 = cleanLine;
	int cnt = 0;
	while (R - L > 0) {
		++cnt;
		int M = (R + L + 1) / 2;
		labelText2 = labelText;
		cleanLine2 = cleanLine;

		if (cleanLine2.length() > M) {
			cleanLine2.chop(cleanLine2.length() - M);
		}
		if (labelText2.length() > M) {
			labelText2.chop(labelText2.length() - M);
		}
		dgLabelByShotname[shotname]->setHtml(labelText2 + "<br>" + cleanLine2 + "<br>");
		double res = dgLabelByShotname[shotname]->boundingRect().width();
		if (res > rect.width() * factor) {
			R = M - 1;
		} else {
			L = M;
		}
	}*/
	dgLabelByShotname[shotname]->setHtml(labelText + "<br><font color=\"#000066\">" + cleanLine + "</font><br><br><font color=\"Purple\">" + dur + "</font>");

	//qDebug() << "|text: " << cleanLine << ", rect label: " << dgLabelByShotname[shotname]->boundingRect();
}

void ShotManager::onScaledView(double factor) {
	/*for (auto sh : currentDgLink->shots) {
		updateDgLabel(sh.shotName, factor);
	}*/
}

void ShotManager::onLoadShots(QString sectionName) {
	clearShotEditor();

	navigateLine = new QGraphicsLineItem;
	navigateLine->setZValue(-1.0);
	navigateLine->setPen( QPen(Qt::red) );
	navigateLine->setPos(0, 0);
	navigateLine->setLine(0, 0, 0, SHOT_SCENE_HEIGHT);
	navigateLine->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	pShotScene->addItem(navigateLine);

	qDebug() << "Shot Manager: loading shots for [" << sectionName << "]";
	currentDgLink = &(ymlManager->dgLinkBySectionName[sectionName]);

	//currentDgLink->totalDuration;
	double dgWidth = qMax(SHOT_SCENE_WIDTH, currentDgLink->totalDuration * 2.0 * SHOT_SECOND);

	pDgScene->setSceneRect(0,0, dgWidth, SHOT_DG_HEIGHT);
	pDgScene->views()[0]->setMaximumWidth(dgWidth);

	pShotScene->setSceneRect(0,0, dgWidth, SHOT_SCENE_HEIGHT);
	pShotScene->views()[0]->setMaximumWidth(dgWidth);

	pLabelScene->setSceneRect(0,0, SHOT_LABEL_WIDTH, SHOT_SCENE_HEIGHT);

	double cur_x = 0;
	QPen pen(Qt::blue, 1, Qt::DashLine, Qt::FlatCap, Qt::BevelJoin);
	pen.setCosmetic(true);
	QFont font("Arial", 9);
	QBrush brush(Qt::darkGreen);

	for (int i = 1; i < currentDgLink->totalDuration * 2.0; ++i) {
		cur_x += SHOT_SECOND;
		pShotScene->addLine(cur_x, 0, cur_x, SHOT_SCENE_HEIGHT, pen);

		QGraphicsSimpleTextItem *textItem = new QGraphicsSimpleTextItem( qn(i) );
		textItem->setPos(cur_x - 1.5, SHOT_DG_HEIGHT * 0.65);
		textItem->setFont(font);
		textItem->setBrush(brush);
		textItem->setFlag(QGraphicsItem::ItemIgnoresTransformations);
		pDgScene->addItem(textItem);
	}

	pDgScene->addLine(0, SHOT_DG_HEIGHT * 0.6, dgWidth, SHOT_DG_HEIGHT * 0.6, pen);

	cur_x = 0;
	QPen pen2(Qt::black, 2.0, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
	QBrush brush2(Qt::yellow);
	pen2.setCosmetic(true);
	for (int i = 0; i < currentDgLink->lines.size(); ++i) {
		double dur = currentDgLink->durations[i];
		QString shotname = currentDgLink->shots[i].shotName;

		QGraphicsRectItem* newElement = new QGraphicsRectItem;
		newElement->setRect(0, 0, dur * SHOT_SECOND, SHOT_DG_HEIGHT * 0.6);
		newElement->setPos(cur_x, 0);
		newElement->setPen( pen2 );
		if (currentDgLink->speakers[i] == ymlManager->SG.getID(SACTORS, "PAUSE")) {
			newElement->setBrush( QBrush(colorDgViewPause) );
		} else if (currentDgLink->speakers[i] == ymlManager->SG.getID(SACTORS, "CHOICE")) {
			newElement->setBrush( QBrush(colorDgViewChoice) );
		} else {
			QString speakerName = ymlManager->SG.getName(currentDgLink->speakers[i]);
			int idx = assignedSpeakers.indexOf( speakerName );
			if (idx == -1) {
				assignedSpeakers.pb( speakerName );
				idx = assignedSpeakers.size() - 1;
			}
			idx = idx % 10; // limit to [0; 9]
			newElement->setBrush( QBrush(colorDgViewActor[idx]) );
		}

		pDgScene->addItem(newElement);
		dgElementByShotname[shotname] = newElement;

		QGraphicsTextItem* textLabel = new QGraphicsTextItem(newElement);
		textLabel->setFlag(QGraphicsItem::ItemIgnoresTransformations);
		textLabel->setPos(0, 0);
		dgLabelByShotname[shotname] = textLabel;
		updateDgLabel(shotname);

		cur_x += dur * SHOT_SECOND;
		pShotScene->addLine(cur_x, 0, cur_x, SHOT_SCENE_HEIGHT, pen2);
	}
}
