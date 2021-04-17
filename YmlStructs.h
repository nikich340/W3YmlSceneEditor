#ifndef YMLSTRUCTS_H
#define YMLSTRUCTS_H

#include <QVector>
#include <QString>
#include <QMap>
#include <QSet>
#include <QString>
#include <QVariant>

#define upn(x, init, n) for (int x = init; x <= n; ++x)
#define ups(x, init, n) for (size_t x = init; x <= n; ++x)
#define upiter(x, container) for (auto x = container.begin(); x != container.end(); ++x)
#define dn(x, init, n) for(int x = init; x >= n; --x)
#define diter(x, container) for (auto x = container.rbegin(); x != container.rend(); ++x)
#define pb push_back
#define qn(x) QString::number(x)

struct dialogLine {
	QString text;
	QString id;
	QString hex;
	double duration;
};

struct shotAction {
	QString actionName;
	double start;
	QHash<QString, QVariant> values;  // super abstract, but perfomance ??
	// variant.setData(QVariant::fromValue<QVector3D>(vec));
	// vec = variant.value<QVector3D>();
	shotAction(QString _actionName = QString(), double _start = -1.0) {
		actionName = _actionName;
		start = _start;
	}
};

struct shot {
	QVector<shotAction> actions;
	QString shotName;
};

struct dialogLink {
	QVector< shot > shots;
	QVector< QString > speakers;
	QVector< QString > lines;
	QVector< double > durations;

	double totalDuration = -1.0;

	void calculateTotalDuration() {
		totalDuration = 0.0;
		for (auto d : durations) {
			totalDuration += d;
		}
	}
	int getIdx(QString shotName) {
		upn(i, 0, shots.size() - 1) {
			if (shots[i].shotName == shotName)
				return i;
		}
		return -1;
	}

	void clear() {
		shots.clear();
		speakers.clear();
		lines.clear();
		durations.clear();
	}
};

struct ymlCond { // [moddlg_facts_dan_storytime, "<", 1]
    QString condFact;
    QString condOperand;
    int condValue;
    ymlCond(QString fact = QString(), QString operand = QString(), int value = 0) {
        condFact = fact;
        condOperand = operand;
        condValue = value;
    }
	friend bool operator==(const ymlCond& lhs, const ymlCond& rhs) {
		bool ret = true;
		ret = ret & (lhs.condFact == rhs.condFact);
		ret = ret & (lhs.condOperand == rhs.condOperand);
		ret = ret & (lhs.condValue == rhs.condValue);
		return ret;
	}
	friend bool operator!=(const ymlCond& lhs, const ymlCond& rhs) {
		return !(lhs == rhs);
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
	friend bool operator==(const choiceAction& lhs, const choiceAction& rhs) {
		bool ret = true;
		ret = ret & (lhs.action == rhs.action);
		ret = ret & (lhs.amount == rhs.amount);
		ret = ret & (lhs.grantExp == rhs.grantExp);
		return ret;
	}
	friend bool operator!=(const choiceAction& lhs, const choiceAction& rhs) {
		return !(lhs == rhs);
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
	void addChoice(QString name = QString(), QString choice = QString(), choiceAction action = choiceAction(),
                   ymlCond condition = ymlCond(), bool single = false, bool emphasiz = false) {
		if (name == "--NOT SET--") {
			name = QString();
		}
		if (action.action == "--NOT SET--") {
			action.action = QString();
		}
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
	friend bool operator==(const sectionLink& lhs, const sectionLink& rhs) {
		bool ret = true;
		ret = ret & (lhs.type == rhs.type);
		ret = ret & (lhs.sectionName == rhs.sectionName);
		ret = ret & (lhs.names == rhs.names);
		ret = ret & (lhs.choiceLines == rhs.choiceLines);
		ret = ret & (lhs.choiceActions == rhs.choiceActions);
		ret = ret & (lhs.conditions == rhs.conditions);
		ret = ret & (lhs.single_use == rhs.single_use);
		ret = ret & (lhs.emphasize == rhs.emphasize);
		ret = ret & ( abs(lhs.timeLimit - rhs.timeLimit) < 1e-5 );
		return ret;
	}
	friend bool operator!=(const sectionLink& lhs, const sectionLink& rhs) {
		return !(lhs == rhs);
	}
    sectionLink() {}
};

#endif // YMLSTRUCTS_H
