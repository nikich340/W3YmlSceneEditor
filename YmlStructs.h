#ifndef YMLSTRUCTS_H
#define YMLSTRUCTS_H

#include <QVector>
#include <QString>
#include <QMap>
#include <QSet>

struct ymlCond { // [moddlg_facts_dan_storytime, "<", 1]
    QString condFact;
    QString condOperand;
    int condValue;
    ymlCond(QString fact = QString(), QString operand = QString(), int value = 0) {
        condFact = fact;
        condOperand = operand;
        condValue = value;
    }
};
struct choiceAction {
    QString action;
    int amount;
    bool grantExp; // - [ "pay 100, add experience?", section_pay, pay, 100, true ]
    choiceAction(QString actionn = QString(), int amountt = -1, bool grantExpp = false) {
        action = actionn;
        amount = amountt;
        grantExp = grantExpp;
    }
};

struct sectionLink {
    QVector<QString> names; // [0] - on_true, [1] - on_false if [condition]
    QString sectionName;

    bool isChoice     = false;
    bool isRandomizer = false;
    bool isCondition  = false;
    bool isExit       = false;
    bool isStart       = false;
    bool isScript       = false;
    QVector<ymlCond> conditions;
    QVector<QString> choiceLines;
    QVector<choiceAction> choiceActions;
    QVector<bool>    single_use;
    QVector<bool>    emphasize;
    double timeLimit = -1.0;
    void addChoice(QString name = "NOT SET", QString choice = QString(), choiceAction action = choiceAction(),
                   ymlCond condition = ymlCond(), bool single = false, bool emphasiz = false) {
        names.push_back(name);
        choiceLines.push_back(choice);
        choiceActions.push_back(action);
        conditions.push_back(condition);
        single_use.push_back(single);
        emphasize.push_back(emphasiz);
    }
    sectionLink() {}
};


#endif // YMLSTRUCTS_H
