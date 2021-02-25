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
	QVector<GraphicsSectionItem*> unusedItems;
    QGraphicsScene* pScene = nullptr;
    QString filePath;
    QStringList sectionNames;
    YAML::Node root;
    QMap<QString, sectionLink*> sectionGraph;
    QVector<QString> startSections;              // for init drawing
    QVector< QVector<QString> > sectionsByDepth; // for init drawing
    QMap<QString, GraphicsSectionItem*> itemBySectionName;
    QSet<QString> wasDrawn;                      // for init drawing

public:
    YmlSceneManager(QObject *parent = nullptr, QGraphicsScene* gScene = nullptr);
    bool loadYmlFile(QString path);
    bool saveYmlFile();
    bool loadSectionsInfo();
    bool drawSectionsGraph();
    bool dfsPrepareGraph(QString sectionName, int depth);
    bool dfsDrawGraph(QString sectionName);
	void updateSectionLink(QString sectionName);
	void renameSectionLink(QString sectionName, QString oldName);
    void removeSectionLink(QString sectionName);
    sectionLink* getSectionLink(QString sectionName);
    GraphicsSectionItem* getSectionItem(QString sectionName);
    QStringList getSectionNames();
    void error(QString s);
    void info(QString s);

public slots:
    void deleteSection(QString sectionName);

signals:
    void print_info(QString msg);
    void print_error(QString msg);
};

#endif // YMLSCENEMANAGER_H
