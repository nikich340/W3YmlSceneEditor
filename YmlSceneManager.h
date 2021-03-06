#ifndef YMLSCENEMANAGER_H
#define YMLSCENEMANAGER_H
#include <QtWidgets>
//#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>
#include "YmlStructs.h"
#include "constants.h"

class GraphicsSectionItem;

class YmlSceneManager : public QObject
{
    Q_OBJECT
private:
	QVector<GraphicsSectionItem*> unusedItems;
	QGraphicsScene* pScene = nullptr;
	QGraphicsScene* pDgScene = nullptr;
	QGraphicsScene* pLabelScene = nullptr;
	QGraphicsScene* pShotScene = nullptr;
    QString filePath;
    QStringList sectionNames;
    YAML::Node root;
	QMap<QString, sectionLink*> sectionGraph;
    QVector<QString> startSections;              // for init drawing
    QVector< QVector<QString> > sectionsByDepth; // for init drawing
    QMap<QString, GraphicsSectionItem*> itemBySectionName;
    QSet<QString> wasDrawn;                      // for init drawing

public:
	/* shotEditor */
	QMap<QString, dialogLine> lineById;
	QMap<QString, dialogLink> dgLinkBySectionName;
	QSet<QString> dgActors, dgProps;

	bool hasChanges = false;
	bool hasShotChanges = false;
    YmlSceneManager(QObject *parent = nullptr, QGraphicsScene* gScene = nullptr);
    bool loadYmlFile(QString path);
    bool saveYmlFile();

	/* section editor */
	bool loadSectionsInfo();
	bool loadShotActions(const YAML::Node actsNode, shot& sh);
	bool loadShotsInfo();
    bool drawSectionsGraph();
    bool dfsPrepareGraph(QString sectionName, int depth);
    bool dfsDrawGraph(QString sectionName);
	bool requestSave();
	void addSectionLink(QPointF pos);
	void updateSectionLink(QString sectionName);
	void renameSectionLink(QString sectionName, QString oldName);
    void removeSectionLink(QString sectionName);
    sectionLink* getSectionLink(QString sectionName);
    GraphicsSectionItem* getSectionItem(QString sectionName);
    QStringList getSectionNames();

	/* shot editor */
	QString getCleanLine(QString text);
	double getTextDuration(QString text);
	void setShotScenes(QGraphicsScene* gDgScene, QGraphicsScene* gLabelScene, QGraphicsScene* gShotScene);
	void loadShotEditor(QString sectionName);

    void error(QString s);
    void info(QString s);

public slots:
    void deleteSection(QString sectionName);

signals:
    void print_info(QString msg);
    void print_error(QString msg);
	void loadShots(QString sectionName);
};

#endif // YMLSCENEMANAGER_H
