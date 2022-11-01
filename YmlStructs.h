#ifndef YMLSTRUCTS_H
#define YMLSTRUCTS_H

#include <algorithm>
#include <QVector>
#include <QDebug>
#include <QString>
#include <QMap>
#include <QSet>
#include <QString>
#include <QVariant>
#include <QVector3D>
#include "constants.h"
#include "ShotEditor/ShotActionTypes.h"

struct transform {
	QVector3D pos;
	QVector3D rot;
	transform() {}
	transform(QVector3D _pos, QVector3D _rot) {
		pos = _pos;
		rot = _rot;
	}
};

struct asset_base {
	bool fromRepo = false;
	int nameID = -1;			// = map name from yml
};

struct asset : asset_base {
	QString templatePath;
	QString mainAppearance;
	bool by_voicetag = false;
    bool is_prop = false;
	QSet<QString> appearances;
	QSet<QString> tags;
	/*void debug() {
		qd << "name: " << name << ", tpath: " << templatePath << ", mainApp: " << mainAppearance << ", apps: " << appearances << ", by_voicetag: " << by_voicetag << ", tags: " << tags;
	}*/
	asset() {}
    asset(int _nameID, QString _templatePath) {
		nameID = _nameID;
		templatePath = _templatePath.replace("\\", "/");
	}
	QString getAppearances() {
		QString ret = "";
		for (auto it : appearances) {
			ret += it + "\n";
		}
		return ret;
	}
	QString getTags() {
		QString ret = "";
		for (auto it : tags) {
			ret += it + "\n";
		}
		return ret;
	}
};

struct camera : asset_base {
	float fov = 30.0, dofIntensity = 0.25, zoom = -1.0;
	transform placement;
	QPair<float, float> dofBlur, dofFocus, dofAperture;
	QString plane = "medium";
	QSet<QString> tags = { "ext" };

	camera() {}
	camera(int _nameID, float _fov, transform _placement) {
		nameID = _nameID;
		fov = _fov;
		placement = _placement;
	}
};

/* includding additives, mimic, usual */
struct animation : asset_base {
	int actorID = -1; // special case (PRODUCTION)
	QString animName; // = "animation" param
    int frames = -1;

	// extra
    float blendin = 0.0, blendout = 0.0; // [0.0, duration] in sec
    float clipfront = -1, clipend = -1; // [0.0, duration] in sec
    float weight = 1.0, stretch = 1.0; // (0.0, +inf)

    double raw_duration() {
        return frames / FPS;
    }
    double duration() {
        double ret = raw_duration();
        if (clipend > 0) {
            ret = clipend;
        }
        if (clipfront > 0) {
            ret -= clipfront;
        }
        ret *= stretch;
        return ret;
    }
	animation() {}
	animation(int _nameID, QString _animName, int _frames) {
		nameID = _nameID;
		animName = _animName;
		frames = _frames;
	}
};

struct anim_pose : asset_base {
	int actorID = -1; // special case (PRODUCTION)
	QString nameParam; // = "name" param
	// RMEMR - either (existing) posename + emostate + status or an idle animation (trumps previous settings)
	QString idle_anim;
	QString emotional_state, status;
	anim_pose () {}
	anim_pose(int _nameID, QString _idle_anim) {
		nameID = _nameID;
		idle_anim = _idle_anim;
	}
};

/* Contempt, Disgusted, Aggressive, Bursting_anger, Drunk, Nervous, Afraid, Very afraid, Surprised, Surprised Shocked,
 * Neutral, Happy, Very happy, Determined, Focused, Confident, Proud, Sad, Cry, Sceptic, Seducing, Sleeping
*/
struct mimic_pose : asset_base {
	int actorID = -1; // special case (PRODUCTION)
	QString anim, emotional_state, eyes, pose;
	float weight = 1.0, duration = 0.0;
	mimic_pose() {}
	mimic_pose(int _nameID, QString _pose) {
		nameID = _nameID;
		pose = _pose;
	}
};

/* REPOSITORY + PRODUCTION (main storage for YmlManager) */
struct sceneInfo {
public:
	// key = map name always (QHash for fast lookup)
    QHash<int, asset> actors;            // repository + production
    QHash<int, asset> props;             // repository + production
    QHash<int, camera> cameras;          // repository + production
    QHash<int, animation> anims, mimics; // repository + production
    QHash<int, anim_pose> poses;         // repository + production
    QHash<int, mimic_pose> mimic_poses;  // repository + production
    QHash<int, QSet<QString>> soundbanks;  // repository, soundbanks[banknameID] = set of events
	//QSet<QString> availableSoundEvents; // production (from banks)
	// but load DEFAULT MIMIC from actors repository firstly, and production secondly..

	/* names storage - store unique int IDs instead of real names (easy renaming, less RAM usage) */
    QHash< QString, QHash<QString, int> > usedIDs; // usedNamesFor["type"].contains("name")
												 // usedNamesFor["type"].insert("name")
    QHash< int, QPair<QString, QString> > usedNames;
	int newID = 0;
	bool hasName(QString _type, QString _name) {
		return usedIDs.contains(_type) && usedIDs[_type].contains(_name);
	}
	bool hasID(int _ID) {
		return usedNames.contains(_ID);
	}
	int getID(QString _type, QString _name) {
		if ( !usedIDs.contains(_type) ) {
			usedIDs[_type] = QHash<QString, int>();
		}
		if ( !usedIDs[_type].contains(_name) ) {
			++newID;
			usedIDs[_type][_name] = newID;
			usedNames[newID] = {_type, _name};
		}
		return usedIDs[_type][_name];
	}
	QPair<QString, QString> getTypeName(int _ID) {
		if ( !usedNames.contains(_ID) ) {
			return QPair<QString, QString>();
		}
		return usedNames[_ID];
	}
	QString getName(int _ID) {
        return getTypeName(_ID).YValue;
	}
	void removeName(QString _type, QString _name) {
		if ( usedIDs.contains(_type) &&  usedIDs[_type].contains(_name) ) {
			usedNames.remove( usedIDs[_type][_name] );
			usedIDs[_type].remove(_name);
		}
	}
	void removeID(int _ID) {
		if ( usedNames.contains(_ID) ) {
            usedIDs[usedNames[_ID].XKey].remove( usedNames[_ID].YValue );
			usedNames.remove(_ID);
		}
	}
	void rename(int _ID, QString _newName) {
        if ( usedNames.contains(_ID) && usedNames[_ID].YValue != _newName ) {
            usedIDs[usedNames[_ID].XKey].remove( usedNames[_ID].YValue );
            usedIDs[usedNames[_ID].XKey][_newName] = _ID;
            usedNames[_ID].YValue = _newName;
		}
	}
	void getNameUnused(QString _type, QString _name, QString& outName, int& outID) {
		int i = 0;
		while ( hasName(_type, _name + qn(i)) ) {
			++i;
		}
		outName = _name + qn(i);
		outID = getID(_type, outName);
	}
	void rename(QString _type, QString _name, QString _newName) {
		if ( usedIDs.contains(_type) &&  usedIDs[_type].contains(_name) && _name != _newName ) {
			int ID = usedIDs[_type][_name];
            usedNames[ID].YValue = _newName;
			usedIDs[_type].remove(_name);
			usedIDs[_type][_newName] = ID;
		}
	}
	void removeType(QString _type) {
		if ( usedIDs.contains(_type) ) {
			for (auto it : usedIDs[_type]) {
				usedNames.remove(it);
			}
			usedIDs[_type].clear();
		}
	}

	/* SETTINGS */
	QString placementTag = "NO TAG";
	QString idspace = "9999";	// must be always 4-digit
	int sceneid = 1, idstart = 0;
	bool gameplay = false, cinematic_subtitles = false;

    QHash<int, int> defaultPose; // [actorID] = poseID
    QHash<int, mimic_pose> defaultMimic; // [actorID] = mimic object
    QHash<int, transform> defaultPlacement; // [actorID] = placement object

    /* called ONCE to init all */
    void init() {
        /*actors = QHash<int, asset>();
        props = QHash<int, asset>();
        cameras = QHash<int, camera>();
        anims = QHash<int, animation>();
        mimics = QHash<int, animation>();
        poses = QHash<int, anim_pose>();
        mimic_poses = QHash<int, mimic_pose>();
        soundbanks = QHash<int, QSet<QString>>();

        usedIDs = QHash< QString, QHash<QString, int> >();
        usedNames = QHash< int, QPair<QString, QString> >();
        defaultPose = QHash<int, int>();
        defaultMimic = QHash<int, mimic_pose>();
        defaultPlacement = QHash<int, transform>();*/
    }
	// [actor] = value
};

struct dialogLine {
	QString text;
    uint id;
    uint key_hex;
	double duration;
};

struct shotAction {
    EShotActionType actionType;
    double start;
    QHash<QString, QVariant> values;
    shotAction(EShotActionType _actionType = EShotUnknown, double _start = -1.0) {
        actionType = _actionType;
		start = _start;
	}
};

struct shot {
    QVector<shotAction> actions;
	QString shotName;
    shot(QString _shotName = QString()) {
        shotName = _shotName;
    }
    void sortActionsByStart() {
        std::sort(actions.begin(), actions.end(),
            [](const shotAction& a, const shotAction& b) -> bool
            {
                return a.start < b.start;  // asending
            }
        );
    }
};

struct dialogLink {
    QSet< QString > shotNames;
	QVector< shot > shots;
	QVector< int > speakers;
	QVector< QString > lines;
	QVector< double > durations;

    double totalDuration = 0.0;

	void calculateTotalDuration() {
		totalDuration = 0.0;
		for (auto d : durations) {
			totalDuration += d;
		}
	}
    double getStartTimeForShot(int shotNum) {
        double ret = 0.0;
        upn(i, 0, shotNum - 1) {
            ret += durations[i];
        }
        return ret;
    }
    int shotNumByName(QString shotName) {
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
	choiceAction(QString _action = QString(), int _amount = -1, bool _grantExp = false) {
		action = _action;
		amount = _amount;
		grantExp = _grantExp;
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
    sectionLink(QString _sectionName = QString(), sectionType _type = nextS) {
        type = _type;
        sectionName = _sectionName;
    }
};

#endif // YMLSTRUCTS_H
