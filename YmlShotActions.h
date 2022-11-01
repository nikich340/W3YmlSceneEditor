#ifndef YMLSHOTACTIONS_H
#define YMLSHOTACTIONS_H
#include <QHash>
#include <QString>
#include <QVariant>
#include "constants.h"

class ShotActionBase {
public:
    EShotActionType actionType = EShotUnknown;
    double start = -1.0;
    QHash<QString, QVariant> values;
    ShotActionBase(EShotActionType _actionType = EShotUnknown, double _start = -1.0) {
        actionType = _actionType;
        start = _start;
    }
};

#endif // YMLSHOTACTIONS_H
