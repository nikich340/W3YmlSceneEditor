#include "GraphicsSectionItem.h"
#include "constants.h"
#include "DialogChangeSection.h"

GraphicsSectionItem::GraphicsSectionItem(QGraphicsItem *parent) : QGraphicsRectItem(parent)
{
    setDefaults();
    this->setData(0, "section");
}
void GraphicsSectionItem::setDefaults() {
    setRect( QRectF(0, 0, WIDTH, HEIGHT) );
    setPen( QPen(Qt::black) );
    //setBrush( QBrush(QColor(128, 255, 170)) );
    setBrush( QBrush(QColor(255, 255, 128)) );
    setFlags( QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable );
}
void GraphicsSectionItem::setLabel(QString text) {
    if (label == nullptr) {
        label = new QGraphicsTextItem("Name not set");
        label->setDefaultTextColor( QColor(0, 0, 0) );
        QFont f("Arial");
        f.setPixelSize(14);
		label->setData(0, "inputSocket"); // hack for edge drag event
        label->setFont(f);
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
        if (cPos[i + 1] - last > 16) {
            insPos.push_back(cPos[i]);
            last = cPos[i];
        }
    }
    last = 0;
    for (int i = 0; i < text.length(); ++i) {
        newText.push_back(text[i]);
        if (last < insPos.size() && i == insPos[last]) {
			if (last > 2) {
				newText.push_back("...");
				break;
			}
            newText.push_back('\n');
            ++last;
        }
    }

	qreal labelH = 0;
	if (last == 0)
		labelH = HEIGHT / 3.5;
	else if (last == 1)
		labelH = HEIGHT / 6;

	label->setPos( WIDTH / 20, labelH );
	label->setPlainText(newText);
}
SocketItem* GraphicsSectionItem::addCleanOutput() {
    hasCleanOutput = true;
	SocketItem* output = new SocketItem(0, 0, DIAMETER, DIAMETER, this);
	//output->setParentItem(this);
    output->setPen(QPen(Qt::black));
    output->setBrush(QBrush(Qt::cyan));
    output->isInputSocket = false;

    outputs.push_back(output);
    if (outputs.size() > sLink->names.size()) {
        sLink->addChoice();
		if ( sLink->type != choiceS || sLink->type != randomS )
            output->hide();
    }
    updateOutputs();

    return output;
}
void GraphicsSectionItem::setYmlManager(YmlSceneManager *newManager) {
    ymlManager = newManager;
}
void GraphicsSectionItem::setSectionLink(sectionLink *link) {
    sLink = link;
}
bool GraphicsSectionItem::isAllowedNewOutput() {
    int cnt = outputs.size();

	if (sLink->type == choiceS || sLink->type == randomS) {
        return (cnt < 7);
    }
	if (sLink->type == conditionS) {
        return (cnt < 2);
    }
	return (cnt < 1 && sLink->type != exitS);
}
void GraphicsSectionItem::updateState() {
    int hasOutput = 0;
    int outputCnt = 0;
    for (auto out : outputs) {
        if ( out->isVisible() ) {
            ++outputCnt;
        }
        if ( out->hasEdges() ) {
            ++hasOutput;
        }
    }

    bool ok = true;
	if (input != nullptr)
		ok &= ( (input->hasEdges() > 0) ^ sLink->isStart() );
	ok &= ( (outputCnt > 0 && hasOutput > 0) || sLink->type == exitS );
	ok &= ( (outputCnt == 2 && hasOutput == 2) || sLink->type != conditionS );

    if (ok) {
        state = SectionState::normal;
        setBrush( QBrush(QColor(255, 255, 128)) );
    } else {
        state = SectionState::incomplete;
        setBrush( QBrush(QColor(255, 102, 102)) );
    }
}
void GraphicsSectionItem::createInputSocket() {
    if (input == nullptr) {
		input = new SocketItem(0, 0, DIAMETER, DIAMETER, this);
        input->setPen(QPen(Qt::black));
        input->setBrush(QBrush(Qt::cyan));
        input->setPos( -DIAMETER/2, (HEIGHT - DIAMETER)/2 );
        input->setData(0, "inputSocket");
		//input->setParentItem(this);
        input->isInputSocket = true;
    }
}
void GraphicsSectionItem::fillCleanSockets() {
    while ( outputs.size() < 7 ) {
        addCleanOutput();
    }
    if (input == nullptr) {
        createInputSocket();
        input->hide();
    }
    updateState();
}
bool GraphicsSectionItem::removeInputEdge(EdgeItem *edge) {
    if (edge == nullptr || input == nullptr)
        return false;

    bool ret = input->removeEdge(edge);
    updateState();
    return ret;
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
            sLink->names[i] = "NOT SET";
            updateState();

            return true;
        }
    }

    return false;
}
void GraphicsSectionItem::addInputEdge(EdgeItem *edge) {
    if (input == nullptr) {
        createInputSocket();
    }
    input->addEdge( edge );
    updateState();

    edge->setEndPoint(input);
    edge->draw();
}
bool GraphicsSectionItem::addOutputEdge(GraphicsSectionItem *next, bool skipUpdates, EdgeItem *edge) {
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
    }

    int socketIdx = outputs.indexOf(output);
    if (socketIdx != -1) {
        //qDebug() << "socketIdx: " << socketIdx << ", choice size: " << sLink->choiceLines.size();
		if (sLink->type == choiceS && socketIdx < sLink->choiceLines.size()) {
			QString s = sLink->choiceLines[socketIdx];
			bool needN = false;
			for (int i = 1; i < s.length(); ++i) {
				if (i % 25 == 0) {
					needN = true;
				}
				if (needN && s[i] == ' ') {
					needN = false;
					s[i] = '\n';
				}
			}
			output->setLabel( s );
		} else if (sLink->type == conditionS && !sLink->conditions[0].condFact.isEmpty()) {
            ymlCond cond = sLink->conditions[0];
            if (socketIdx == 0)
				output->setLabel(cond.condFact + "\n" + cond.condOperand + QString::number(cond.condValue) + ": [on_true]");
            else if (socketIdx == 1)
				output->setLabel(cond.condFact + "\n" + cond.condOperand + QString::number(cond.condValue) + ": [on_false]");
		} else if (sLink->type == randomS) {
            output->setLabel("[random #" + QString::number(socketIdx + 1) + "]");
        } else {
            output->setLabel("NEXT");
        }
    }
    if (socketIdx != -1 && !skipUpdates) {
        // Simple update without changing order - lifehack
        // PUSH new section to yml storage
        sLink->names[socketIdx] = next->sLink->sectionName;
        qDebug() << "set name [" << socketIdx << "] = " << next->sLink->sectionName;
        ymlManager->updateSectionLink(sLink->sectionName);
    }

	next->addInputEdge( edge );
    updateState();
    return true;
}
void GraphicsSectionItem::updateOutputs() {
    int cnt = 1;
    for (int i = 0; i < outputs.size(); ++i) {
        if ( outputs[i]->isVisible() ) {
            ++cnt;
         }
    }
    qreal h = 0;
    qreal h_diff = HEIGHT / cnt;
    for (int i = 0; i < outputs.size(); ++i) {
        if (outputs[i]->isVisible())
            ++cnt;
    }
    for (int i = 0; i < outputs.size(); ++i) {
        if ( outputs[i]->isVisible() ) {
            h += h_diff;
            outputs[i]->setPos( WIDTH - DIAMETER/2, h - DIAMETER/2 );
         }
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
        changeMe();
    } else {
        QGraphicsRectItem::mouseDoubleClickEvent(event);
    }
}

void GraphicsSectionItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsRectItem::mouseMoveEvent(event);
    updateInputEdges();
    updateOutputEdges();
}

void GraphicsSectionItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    QMenu menu;
	QAction *editAction = menu.addAction("Edit");
	QAction *deleteAction = menu.addAction("Delete");
    QAction *selectedAction = menu.exec(event->screenPos());
    if (selectedAction == nullptr) {
        event->ignore();
	} else if (selectedAction == editAction) {
        event->accept();
        changeMe();
	} else if (selectedAction == deleteAction) {
        event->accept();
        removeMe();
    }
}

void GraphicsSectionItem::changeMe() {
    DialogChangeSection dialog;
    dialog.setModal(true);
    dialog.updateChoiceForms( this->sLink, ymlManager->getSectionNames() );

    int ret = dialog.exec();
    if (ret == QDialog::Accepted) {
        qDebug() << "accepted!";
		sectionLink oldData = *sLink;
		sLink->reset();
		dialog.fillLink();
		/*
		 * 1) RESET ALL
		 * 2) PUSH all data from dialog
		 * 3) UPDATE everything (check if outputs/inputs were changed -> update edges)
		 */


		// choice -> any except cond = reset all conditions
		// cond -> any except choice = reset condition [0]

		// any -> random/choice = show all outputs
    }
}
void GraphicsSectionItem::removeMe() {
	QMessageBox msgBox;
	msgBox.setText("Do you want to delete section [" + sLink->sectionName + "] ?");
	msgBox.setInformativeText("It will remove all it's connection and shots.");
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::Yes);
	int ret = msgBox.exec();

	if ( ret != QMessageBox::Yes ) {
		return;
	}

	qDebug() << "Deleting section [" + sLink->sectionName + "]..";
	if (input != nullptr) {
		while ( input->hasEdges() ) {
			EdgeItem* edge = input->getLastEdge();
			GraphicsSectionItem* startSection = qgraphicsitem_cast<GraphicsSectionItem*>( edge->socketStart->parentItem() );
			startSection->removeOutputEdge(edge);
		}
		scene()->removeItem(input);
		delete input;
		input = nullptr;
	}
	for (auto out : outputs) {
		if (out != nullptr) {
			EdgeItem* edge = out->getLastEdge();
			removeOutputEdge(edge);
		}
		scene()->removeItem(out);
		delete out;
		out = nullptr;
	}
	outputs.clear();

	// I'll be back
	ymlManager->deleteSection( sLink->sectionName );
}
