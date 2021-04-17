#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QObject>
#include <QColor>

static QColor colorSceneGradient0(235, 235, 224), colorSceneGradient1(195, 195, 162);
static QColor colorSceneGradient00(235, 235, 224), colorSceneGradient11(195, 20, 162);
static QColor colorSectionNormal(240, 255, 112), colorSectionIncomplete(255, 122, 122);
static QColor colorSocketNormal(0, 220, 220), colorSocketLabel(255, 51, 133);
static QColor colorEdgeNormal(0, 0, 100), colorEdgeChange(128, 0, 50), colorEdgeHighlight(0, 90, 20);
static QColor colorDgViewPause(178, 178, 255), colorDgViewChoice(255, 255, 160);
static QColor colorDgViewActor[10] = {
	QColor(153,255,153), QColor(192,153,255), QColor(255,210,153), QColor(153,246,255), QColor(255,173,230),
	QColor(0,0,0), QColor(0,0,0), QColor(0,0,0), QColor(0,0,0), QColor(0,0,0)
};

const qreal DIAMETER = 6.0;
const qreal HEIGHT = 60.0;
const qreal WIDTH = 125.0;
const qreal SCENE_HEIGHT = 1250.0;
const qreal SCENE_WIDTH = 5000.0;

const qreal SHOT_SCENE_HEIGHT = 2000.0;
const qreal SHOT_SCENE_WIDTH = 3000.0;
const qreal SHOT_SECOND = 50;
const qreal SHOT_ROW = 25;
const qreal SHOT_DG_HEIGHT = 70.0;
const qreal SHOT_LABEL_WIDTH = 150.0;

#endif // CONSTANTS_H
