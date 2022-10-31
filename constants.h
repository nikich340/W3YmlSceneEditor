#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QObject>
#include <QColor>
#define QSvg QColorConstants::Svg

struct CONSTANTS {
    static const int IsNavigationLine;
    static const QColor colorSceneGradient0, colorSceneGradient1;
    static const QColor colorSceneGradient00, colorSceneGradient11;
    static const QColor colorSectionNormal, colorSectionIncomplete;
    static const QColor colorSocketNormal, colorSocketLabel;
    static const QColor colorEdgeNormal, colorEdgeChange, colorEdgeHighlight;
    static const QColor colorDgViewPause, colorDgViewChoice;
    static const QVector<QColor> colorDgViewActors;

    static const QString SREPO_TEMP;
    static const QString SCAMERAS;
    static const QString SASSETS; // for unification: actors + props
    static const QString SANIMS;
    static const QString SMIMICANIMS; // for unification: anim + anim.mimic
    static const QString SANIMPOSES;
    static const QString SMIMICPOSES;
    static const QString SSOUNDBANKS;
    static const QString NOT_SET;
    static const QStringList MIMICS_LIST;

    static const double DIAMETER;
    static const double HEIGHT;
    static const double WIDTH;
    static const double SCENE_HEIGHT;
    static const double SCENE_WIDTH;
    static const double SHOT_SCENE_HEIGHT;
    static const double SHOT_SCENE_WIDTH;
    static const double SHOT_SECOND;
    static const double SHOT_ROW;
    static const double SHOT_DG_HEIGHT;
    static const double SHOT_LABEL_HEIGHT;
    static const double SHOT_LABEL_PEN_WIDTH;
    static const double SHOT_LABEL_WIDTH;

    static QColor QColorLight(int i, int total) {
        return QColor::fromHsv(i * 360 / total, 86, 255);
    }
    static QColor QColorLightGray(int i, int total) {
        return QColor::fromHsv(i * 360 / total, 60, 190);
    }
    static QColor QColorDark(int i, int total) {
        return QColor::fromHsv(i * 360 / total, 128, 90);
    }
    static QColor QColorDark(int h) {
        return QColor::fromHsv(h, 128, 90);
    }
    static QColor QColorVeryDark(int i, int total) {
        return QColor::fromHsv(i * 360 / total, 160, 50);
    }
};

#endif // CONSTANTS_H
