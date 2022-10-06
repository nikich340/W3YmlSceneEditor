#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QObject>
#include <QColor>
#define QSvg QColorConstants::Svg

const static QColor colorSceneGradient0(235, 235, 224), colorSceneGradient1(195, 195, 162);
const static QColor colorSceneGradient00(235, 235, 224), colorSceneGradient11(195, 20, 162);
const static QColor colorSectionNormal(240, 255, 112), colorSectionIncomplete(255, 122, 122);
const static QColor colorSocketNormal(0, 220, 220), colorSocketLabel(255, 51, 133);
const static QColor colorEdgeNormal(0, 0, 100), colorEdgeChange(128, 0, 50), colorEdgeHighlight(0, 90, 20);
const static QVector<QColor> colorDgViewActors = {
    QSvg::aquamarine, QSvg::lightsalmon, QSvg::lightcyan, QSvg::lightpink, QSvg::lightblue,
    QSvg::lightyellow, QSvg::lightskyblue, QSvg::lightcoral, QSvg::lavender, QSvg::lightsteelblue,
    QSvg::mistyrose, QSvg::honeydew, QSvg::wheat, QSvg::thistle, QSvg::seashell

};
const static QColor colorDgViewPause = QSvg::gold, colorDgViewChoice = QSvg::hotpink;

const static QString SCAMERAS = "cameras";
const static QString SPROPS = "actors"; // not a bug! for unification
const static QString SACTORS = "actors";
const static QString SANIMS = "animations";
const static QString SMIMICANIMS = "animations.mimic";
const static QString SANIMPOSES = "actor.poses";
const static QString SMIMICPOSES = "mimics";
const static QString SSOUNDBANKS = "soundbanks";
const static QString NOT_SET = "---NOT SET---";

const static qreal DIAMETER = 6.0;
const static qreal HEIGHT = 60.0;
const static qreal WIDTH = 125.0;
const static qreal SCENE_HEIGHT = 1250.0;
const static qreal SCENE_WIDTH = 5000.0;

const static qreal SHOT_SCENE_HEIGHT = 2000.0;
const static qreal SHOT_SCENE_WIDTH = 3000.0;
const static qreal SHOT_SECOND = 100;
const static qreal SHOT_ROW = 25;
const static qreal SHOT_DG_HEIGHT = 70.0;
const static qreal SHOT_LABEL_HEIGHT = 80.0;
const static qreal SHOT_LABEL_PEN_WIDTH = 1.5;
const static qreal SHOT_LABEL_WIDTH = 150.0;

const static QStringList MIMICS_LIST = { "Afraid", "Very afraid", "Aggressive", "Bursting_anger", "Confident", "Contempt", "Cry",
								  "Determined", "Disgusted", "Drunk", "Focused", "Happy", "Very happy", "Nervous", "Neutral",
								  "Proud", "Sad", "Sceptic", "Seducing", "Sleeping", "Surprised", "Surprised Shocked", "NO ANIM" };

#endif // CONSTANTS_H
