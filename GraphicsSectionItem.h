#pragma once
#ifndef GRAPHICSSECTIONITEM_H
#define GRAPHICSSECTIONITEM_H
#include <QtWidgets>
#include "SocketItem.h"
#include "YmlSceneManager.h"

class GraphicsSectionItem : public QGraphicsRectItem
{
private:
    YmlSceneManager* ymlManager; // <- here
    sectionLink* sLink; // PUSH UPDATE TO YML ON EVERY CHANGE!
    bool hasCleanOutput = false;
public:
    GraphicsSectionItem(QGraphicsItem *parent = nullptr);
    void setSectionLink(sectionLink* link);
    void setDefaults();
    void setLabel(QString text);
    SocketItem* addCleanOutput();
    void addInputEdge(EdgeItem* edge); // auto-create PathItem between sections
    bool isAllowedNewOutput();
    void fillCleanOutputs();
    bool removeInputEdge(EdgeItem* edge);
    bool removeOutputEdge(EdgeItem* edge);
    bool addOutputEdge(GraphicsSectionItem *next, EdgeItem* edge = nullptr);
    void updateOutputs();     // repos outputs
    void updateOutputEdges(); // repaint EdgeItems
    void updateInputEdges(); // repaint EdgeItems

    enum { Type = QGraphicsRectItem::UserType + 1 };
    int type() const override { return Type; }

private:
    QGraphicsTextItem* label = nullptr;
    SocketItem* input = nullptr;
    //QVector< EdgeItem* > inputEdges; // multiple node->line-> for [input]

    QVector<SocketItem*> outputs;
    //QVector< EdgeItem* > outputEdges; // one node->line-> for [output]

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif // GRAPHICSSECTIONITEM_H
