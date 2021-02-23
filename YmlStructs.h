#ifndef YMLSTRUCTS_H
#define YMLSTRUCTS_H

#include <QVector>
#include <QString>
#include <QMap>
#include <QSet>
#define upn(x, init, n) for (int x = init; x <= n; ++x)
#define upiter(x, container) for (auto x = container.begin(); x != container.end(); ++x)
#define dn(x, init, n) for(int x = init; x >= n; --x)
#define diter(x, container) for (auto x = container.rbegin(); x != container.rend(); ++x)
#define pb push_back

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

enum sectionType { nextS, choiceS, randomS, conditionS, scriptS, exitS };

struct sectionLink {
    QVector<QString> names; // [0] - on_true, [1] - on_false if [condition]
	QString sectionName;

	sectionType type = nextS;
    QVector<ymlCond> conditions;
    QVector<QString> choiceLines;
    QVector<choiceAction> choiceActions;
    QVector<bool>    single_use;
    QVector<bool>    emphasize;
    double timeLimit = -1.0;
	bool isStart() {
		return sectionName.startsWith("section_start");
	}
    void addChoice(QString name = "NOT SET", QString choice = QString(), choiceAction action = choiceAction(),
                   ymlCond condition = ymlCond(), bool single = false, bool emphasiz = false) {
		action.action = action.action.toLower();

		names.push_back( name.toLower() );
        choiceLines.push_back(choice);
		choiceActions.push_back( action );
        conditions.push_back(condition);
        single_use.push_back(single);
        emphasize.push_back(emphasiz);
    }
	void reset() {
		names.clear();
		sectionName = QString();
		conditions.clear();
		choiceLines.clear();
		choiceActions.clear();
		single_use.clear();
		emphasize.clear();
		timeLimit = -1.0;
	}
    sectionLink() {}
};


#endif // YMLSTRUCTS_H
