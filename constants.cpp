#include "constants.h"

const int CONSTANTS::IsNavigationLine = 0;

const QColor CONSTANTS::colorSceneGradient0 = QColor(235, 235, 224), CONSTANTS::colorSceneGradient1(195, 195, 162);
const QColor CONSTANTS::colorSceneGradient00(235, 235, 224), CONSTANTS::colorSceneGradient11(195, 20, 162);
const QColor CONSTANTS::colorSectionNormal(240, 255, 112), CONSTANTS::colorSectionIncomplete(255, 122, 122);
const QColor CONSTANTS::colorSocketNormal(0, 220, 220), CONSTANTS::colorSocketLabel(255, 51, 133);
const QColor CONSTANTS::colorEdgeNormal(0, 0, 100), CONSTANTS::colorEdgeChange(128, 0, 50), CONSTANTS::colorEdgeHighlight(0, 90, 20);

const QVector<QColor> CONSTANTS::colorDgViewActors = {
    // 0 - CHOICE
    // 3 - PAUSE
    CONSTANTS::QColorLight(1, 15), CONSTANTS::QColorLight(4, 15), CONSTANTS::QColorLight(6, 15), CONSTANTS::QColorLight(8, 15),
    CONSTANTS::QColorLight(10, 15), CONSTANTS::QColorLight(12, 15), CONSTANTS::QColorLight(14, 15), CONSTANTS::QColorLight(2, 15),
    CONSTANTS::QColorLight(5, 15), CONSTANTS::QColorLight(7, 15), CONSTANTS::QColorLight(9, 15), CONSTANTS::QColorLight(11, 15),
    CONSTANTS::QColorLight(13, 15)
};

const QColor CONSTANTS::colorDgViewPause = CONSTANTS::QColorLight(3, 15), CONSTANTS::colorDgViewChoice = CONSTANTS::QColorLight(0, 15);

const QString CONSTANTS::SREPO_TEMP = "_repo_temp_";
const QString CONSTANTS::SCAMERAS = "cameras";
const QString CONSTANTS::SASSETS = "assets"; // for unification: actors + props
const QString CONSTANTS::SANIMS = "animations";
const QString CONSTANTS::SMIMICANIMS = "animations"; // for unification: anim + anim.mimic
const QString CONSTANTS::SANIMPOSES = "actor.poses";
const QString CONSTANTS::SMIMICPOSES = "mimics";
const QString CONSTANTS::SSOUNDBANKS = "soundbanks";
const QString CONSTANTS::NOT_SET = "---NOT SET---";

const double CONSTANTS::DIAMETER = 6.0;
const double CONSTANTS::HEIGHT = 60.0;
const double CONSTANTS::WIDTH = 125.0;
const double CONSTANTS::SCENE_HEIGHT = 1250.0;
const double CONSTANTS::SCENE_WIDTH = 5000.0;

const double CONSTANTS::SHOT_SCENE_HEIGHT = 2000.0;
const double CONSTANTS::SHOT_SCENE_WIDTH = 3000.0;
const double CONSTANTS::SHOT_SECOND = 100;
const double CONSTANTS::SHOT_ROW = 25;
const double CONSTANTS::SHOT_DG_HEIGHT = 70.0;
const double CONSTANTS::SHOT_LABEL_HEIGHT = 80.0;
const double CONSTANTS::SHOT_LABEL_PEN_WIDTH = 1.5;
const double CONSTANTS::SHOT_LABEL_WIDTH = 150.0;

const QStringList CONSTANTS::MIMICS_LIST = { "Afraid", "Very afraid", "Aggressive", "Bursting_anger", "Confident", "Contempt", "Cry",
                                  "Determined", "Disgusted", "Drunk", "Focused", "Happy", "Very happy", "Nervous", "Neutral",
                                  "Proud", "Sad", "Sceptic", "Seducing", "Sleeping", "Surprised", "Surprised Shocked", "NO ANIM" };
