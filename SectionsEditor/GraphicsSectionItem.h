#ifndef GRAPHICSSECTIONITEM_H
#define GRAPHICSSECTIONITEM_H
#include <QObject>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include "SocketItem.h"
#include "YmlSceneManager.h"

class GraphicsSectionItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
    using super = QGraphicsRectItem;
private:
    YmlSceneManager* ymlManager; // <- here
    sectionLink* sLink; // PUSH UPDATE TO YML ON EVERY CHANGE!
    bool hasCleanOutput = false;
	bool isUpdating = false;
public:
    GraphicsSectionItem(QGraphicsItem *parent = nullptr);
    ~GraphicsSectionItem();

	enum SectionState { normal, incomplete };
    SectionState state;

	const QString sName() { return sLink->sectionName; }
    void updateState();
    void setSectionLink(sectionLink* link);
    void setYmlManager(YmlSceneManager* newManager);
    void setDefaults();
    void setLabel(QString text);
	void updateSocketLabel(SocketItem* socket);
    SocketItem* addCleanOutput();
    void addInputEdge(EdgeItem* edge); // auto-create PathItem between sections
    bool isAllowedNewOutput();
    void fillCleanSockets();
    void createInputSocket();
    bool removeInputEdge(EdgeItem* edge);
    bool removeOutputEdge(EdgeItem* edge);
    bool addOutputEdge(GraphicsSectionItem *next, bool skipUpdates = true, EdgeItem* edge = nullptr);
    void updateOutputs();     // repos outputs
    void updateOutputEdges(); // repaint EdgeItems
    void updateInputEdges(); // repaint EdgeItems
	void putOutputEdgesOnTop();

    enum { Type = QGraphicsRectItem::UserType + 1 };
    int type() const override { return Type; }

    void removeMe();
    void changeMe();

private:
    QGraphicsTextItem* label = nullptr;
    SocketItem* m_inputSocket = nullptr;
    //QVector< EdgeItem* > inputEdges; // multiple node->line-> for [input]

    QVector<SocketItem*> m_outputSockets;
    //QVector< EdgeItem* > outputEdges; // one node->line-> for [output]

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
	QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;
};

#endif // GRAPHICSSECTIONITEM_H
