#include "GraphicsSectionItem.h"
#include "constants.h"
#include "DialogChangeSection.h"

GraphicsSectionItem::GraphicsSectionItem(QGraphicsItem *parent) : QGraphicsRectItem(parent)
{
    setDefaults();
}
void GraphicsSectionItem::setDefaults() {
    setRect(QRectF(0, 0, WIDTH, HEIGHT));
    setPen(QPen(Qt::black));
    setBrush(QBrush(Qt::yellow));
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
}
void GraphicsSectionItem::setLabel(QString text) {
    if (label == nullptr) {
        label = new QGraphicsTextItem("Name not set");
        label->setDefaultTextColor( QColor(0, 0, 0) );
        QFont f("Arial");
        f.setPixelSize(14);
        label->setFont(f);
        label->setPos( 0, HEIGHT / 6 );
        label->setParentItem(this);
    }

    // Wrap text to newlines
    int last = 0;
    QString newText = "";
    QVector<int> cPos, insPos;
    for (int i = 0; i < text.length(); ++i) {
        if (text[i] == '_')
            cPos.push_back(i);
    }
    cPos.push_back(text.length() + 1);
    for (int i = 0; i < cPos.size() - 1; ++i) {
        if (cPos[i + 1] - last > 18) {
            insPos.push_back(cPos[i]);
            last = cPos[i];
        }
    }
    last = 0;
    for (int i = 0; i < text.length(); ++i) {
        newText.push_back(text[i]);
        if (last < insPos.size() && i == insPos[last]) {
            newText.push_back('\n');
            ++last;
        }
    }
    label->setPlainText(newText);
}
void GraphicsSectionItem::addInputEdge(EdgeItem *edge) {
    if (input == nullptr) {
        input = new SocketItem(0, 0, DIAMETER, DIAMETER);
        input->setPen(QPen(Qt::black));
        input->setBrush(QBrush(Qt::cyan));
        input->setPos( -DIAMETER/2, (HEIGHT - DIAMETER)/2 );
        input->setData(0, "inputSocket");
        input->setParentItem(this);
        input->isInputSocket = true;
    }
    input->addEdge( edge );

    edge->setEndPoint(input);
    edge->draw();
}
SocketItem* GraphicsSectionItem::addCleanOutput() {
    hasCleanOutput = true;
    SocketItem* output = new SocketItem(0, 0, DIAMETER, DIAMETER);
    output->setParentItem(this);
    output->setPen(QPen(Qt::black));
    output->setBrush(QBrush(Qt::cyan));
    output->isInputSocket = false;
    outputs.push_back(output);
    updateOutputs();

    return output;
}
void GraphicsSectionItem::setSectionLink(sectionLink *link) {
    sLink = link;
}
bool GraphicsSectionItem::isAllowedNewOutput() {
    int cnt = outputs.size();

    if (sLink->isChoice || sLink->isRandomizer) {
        return (cnt < 7);
    }
    if (sLink->isCondition) {
        return (cnt < 2);
    }
    return (cnt < 1 && !sLink->isExit);
}
void GraphicsSectionItem::fillCleanOutputs() {
    while ( isAllowedNewOutput() ) {
        addCleanOutput();
    }
}
bool GraphicsSectionItem::removeInputEdge(EdgeItem *edge) {
    if (edge == nullptr || input == nullptr)
        return false;
    return input->removeEdge(edge);
}
bool GraphicsSectionItem::removeOutputEdge(EdgeItem *edge) {
    if (edge == nullptr || outputs.isEmpty())
        return false;

    GraphicsSectionItem* nextSection = static_cast<GraphicsSectionItem*>( edge->socketEnd->parentItem() );
    if ( !nextSection->removeInputEdge(edge) )
        return false;

    for (int i = 0; i < outputs.size(); ++i) {
        if ( outputs[i]->removeEdge(edge) ) {
            scene()->removeItem(edge);
            delete edge;

            return true;
        }
    }

    return false;
}
bool GraphicsSectionItem::addOutputEdge(GraphicsSectionItem *next, EdgeItem *edge) {
    SocketItem *output;
    if ( edge == nullptr ) {
        edge = new EdgeItem;
        scene()->addItem(edge);
    }
    edge->setState(EdgeItem::normal);

    if ( !edge->hasStart ) {
        if ( isAllowedNewOutput() ) {
            output = addCleanOutput();
        } else {
            return false;
        }
        edge->setStartPoint(output);
        output->addEdge( edge );
    } else {
        // BAD SMELL
        output = static_cast<SocketItem*>(edge->socketStart);
        // ADD EXTRA FREE OUTPUT
    }

    next->addInputEdge( edge );
    return true;
}

void GraphicsSectionItem::updateOutputs() {
    int cnt = outputs.size() + 1;
    qreal h = 0;
    qreal h_diff = HEIGHT / cnt;
    for (int i = 0; i < outputs.size(); ++i) {
        h += h_diff;
        outputs[i]->setPos( WIDTH - DIAMETER/2, h - DIAMETER/2 );
    }
    updateOutputEdges();
}
void GraphicsSectionItem::updateOutputEdges() {
    for (int i = 0; i < outputs.size(); ++i) {
        outputs[i]->redrawAllEdges();
    }
}
void GraphicsSectionItem::updateInputEdges() {
    if (input != nullptr)
        input->redrawAllEdges();
}

void GraphicsSectionItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        qDebug() << "Double click!";
        event->accept();
        DialogChangeSection* dlg = new DialogChangeSection;
        dlg->setModal(true);
        dlg->exec();
    } else {
        QGraphicsRectItem::mouseDoubleClickEvent(event);
    }
}

void GraphicsSectionItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsRectItem::mouseMoveEvent(event);
    updateInputEdges();
    updateOutputEdges();
}
