#ifndef YMLSCENEMANAGER_H
#define YMLSCENEMANAGER_H
#include <QtWidgets>
//#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>
#include "YmlStructs.h"

class GraphicsSectionItem;

class YmlSceneManager : public QObject
{
    Q_OBJECT
private:
    QGraphicsScene* pScene;
    QString filePath;
    QStringList sectionNames;
    YAML::Node root;
    QMap<QString, sectionLink*> sectionGraph;
    QVector<QString> startSections;
    QVector< QVector<QString> > sectionsByDepth;
    QMap<QString, GraphicsSectionItem*> itemBySection;
    QSet<QString> wasDrawn;

public:
    YmlSceneManager(QObject *parent = nullptr);
    bool loadYmlFile(QString path);
    bool saveYmlFile();
    bool loadSectionsInfo();
    bool drawSectionsGraph(QGraphicsScene* gScene);
    bool dfsPrepareGraph(QString sectionName, int depth);
    bool dfsDrawGraph(QString sectionName);
    void updateSectionLink(QString sectionName);
    void removeSectionLink(QString sectionName);
    sectionLink* getSectionLink(QString sectionName);
    QStringList getSectionNames();
    void error(QString s);
    void info(QString s);
signals:
    void print_info(QString msg);
    void print_error(QString msg);
};

#endif // YMLSCENEMANAGER_H
