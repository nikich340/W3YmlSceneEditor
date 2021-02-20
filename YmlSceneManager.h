#ifndef YMLSCENEMANAGER_H
#define YMLSCENEMANAGER_H
#include <QtWidgets>
#include <QMap>
#include <QSet>
#include <QVector>
#include <QString>
#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>

class GraphicsSectionItem;

struct ymlCond { // [moddlg_facts_dan_storytime, "<", 1]
    QString condFact;
    QString condOperand;
    int condValue;
    ymlCond() {}
};

struct sectionLink {
    QVector<QString> names; // 0 - false, 1 - true if [condition]

    bool isChoice     = false;
    bool isRandomizer = false;
    bool isCondition  = false;
    bool isExit       = false;
    QVector<ymlCond> conditions;
    QVector<QString> choiceLines;
    QVector<QString> actionTypes;
    QVector<bool>    single_use;
    QVector<bool>    emphasize;
    float timeLimit = -1.0;
    sectionLink() {}
};

class YmlSceneManager : public QObject
{
    Q_OBJECT
private:
    QGraphicsScene* pScene;
    QString filePath;
    YAML::Node root;
    QMap<QString, sectionLink*> sectionGraph;
    QVector<QString> startSections;
    QVector< QVector<QString> > sectionsByDepth;
    QMap<QString, GraphicsSectionItem*> itemBySection;
    QSet<QString> wasDrawn;

public:
    YmlSceneManager(QObject *parent = nullptr);
    bool loadYmlFile(QString path);
    bool loadSectionsInfo();
    bool drawSectionsGraph(QGraphicsScene* gScene);
    bool dfsPrepareGraph(QString sectionName, int depth);
    bool dfsDrawGraph(QString sectionName);
    void removeSectionLink(QString sectionName);
    sectionLink* getSectionLink(QString sectionName);
signals:
    void debugInfo(QString msg);
};

#endif // YMLSCENEMANAGER_H
