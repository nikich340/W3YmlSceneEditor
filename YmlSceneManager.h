#ifndef YMLSCENEMANAGER_H
#define YMLSCENEMANAGER_H
#include <QtWidgets>
//#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>
#include "YmlStructs.h"
#include "constants.h"
#include "SimpleTrie.h"

class GraphicsSectionItem;

class YmlSceneManager : public QObject
{
    Q_OBJECT
private:
    sceneInfo SG; // = "Scene Globals"

    template<typename HashContainer>
    void removeAssetsFromSG(HashContainer& container, bool clearRepo);

    template<typename Container>
    void deletePointersFromContainer(Container& container);

    QGraphicsScene* m_pScene = nullptr;
    QString m_filePath;
    QStringList m_sectionNames;

    YAML::Node m_root;
    QHash<QString, sectionLink*> m_pSectionLinkBySectionName;
    QVector<QString> m_startSections;              // for init drawing
    QVector< QVector<QString> > m_sectionNamesByDepth; // for init drawing
    QHash<QString, GraphicsSectionItem*> m_pItemBySectionName;
    QSet<QString> m_wasDrawn;                      // for init drawing

public:
    YmlSceneManager(QObject *parent = nullptr, QGraphicsScene* gScene = nullptr);
    ~YmlSceneManager();

	/* scene general */
    QStringList nodeKeys(const YAML::Node& node);
    YAML::Node firstCloned(const YAML::Node& node);
    YAML::Node lastCloned(const YAML::Node& node);
    template<typename T>
    YAML::Node scalarNode(const T& value);
    template<typename T>
    YAML::Node singleMapNode(const QString& key, const T& value);

	bool hasChanges = false;
	bool hasShotChanges = false;
	bool readingYmlRepo = false;

	bool loadYmlRepo(QString path);
	bool loadYmlFile(QString path);
    bool saveYmlFile();
	void clearData(bool clearRepo = false);
	void showError(QString text, QString title = "Error!", QMessageBox::Icon icon = QMessageBox::Critical);

	/* section editor */
	bool loadSectionsInfo();
	bool loadShotActions(const YAML::Node actsNode, shot& sh);
	bool loadSceneRepository();
	bool loadSceneProduction();
	bool loadSceneDefaults();
	void cleanupTempRepository();
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

	/* asset editor */
	void removeActorAsset(int actorID);
    sceneInfo* sceneGlobals() {
        return &SG;
    }

	/* shot editor */
    void updateDialogscriptSection(QString sectionName);
    void updateShot(QString sectionName, QString shotName);
    void updateShot(QString sectionName, int shotNum);
    void removeShot(QString sectionName, QString shotName);
    void addShot(QString sectionName, int shotNum);
    YAML::Node shotActionToNode(shotAction* sa);

    QHash<int, dialogLine> m_lineByID;
    SimpleTrie m_linesTrie;
    QHash<QString, dialogLink*> m_pDialogLinkBySectionName;
    QSet<QString> dgActors, dgProps;

	QString getCleanLine(QString text);
	double getTextDuration(QString text);

    /* global logs */
	void error(QString s);
	void warning(QString s);
    void info(QString s);

public slots:
    void deleteSection(QString sectionName);

signals:
	void print_info(QString msg);
	void print_warning(QString msg);
    void print_error(QString msg);
    void ymlFileLoaded(QString filePath);

    void sectionDeleted(QString sectionName);
    void sectionNameChanged(QString oldSectionName, QString newSectionName);
    void sectionTypeChanged(QString sectionName, int newType);
    void sectionLoaded(QString sectionName);
};

#endif // YMLSCENEMANAGER_H
