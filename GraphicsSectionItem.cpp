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
	setBrush( QBrush(colorSectionNormal) );
	setFlags( QGraphicsItem::ItemIsMovable );
}
void GraphicsSectionItem::setLabel(QString text) {
    if (label == nullptr) {
        label = new QGraphicsTextItem("Name not set");
		label->setDefaultTextColor( Qt::black );
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
	output->setBrush(QBrush(colorSocketNormal));
    output->isInputSocket = false;

    outputs.push_back(output);
    if (outputs.size() > sLink->names.size()) {
        sLink->addChoice();
		qDebug() << "add clean output {" << qn(sLink->names.size() - 1) << "}, type: " << sLink->type;
		if ( (sLink->type != choiceS && sLink->type != randomS) && (outputs.size() > 1 || sLink->type == exitS) )
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

	if (sLink->type == choiceS) {
		for (int i = 0; i < sLink->names.size(); ++i) {
			if (!sLink->names[i].isEmpty() && sLink->choiceLines[i].isEmpty()) {
				ok = false;
				break;
			}
		}
	}

    if (ok) {
        state = SectionState::normal;
		setBrush( QBrush(colorSectionNormal) );
    } else {
        state = SectionState::incomplete;
		setBrush( QBrush(colorSectionIncomplete) );
    }
}
void GraphicsSectionItem::updateSocketLabel(SocketItem* socket) {
	int socketIdx = outputs.indexOf(socket);
	if (socketIdx != -1) {
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
			socket->setLabel( s );
		} else if (sLink->type == conditionS && !sLink->conditions[0].condFact.isEmpty()) {
			ymlCond cond = sLink->conditions[0];
			if (socketIdx == 0)
				socket->setLabel(cond.condFact + "\n" + cond.condOperand + QString::number(cond.condValue) + ": [on_true]");
			else if (socketIdx == 1)
				socket->setLabel(cond.condFact + "\n" + cond.condOperand + QString::number(cond.condValue) + ": [on_false]");
		} else if (sLink->type == randomS) {
			socket->setLabel("[random #" + QString::number(socketIdx + 1) + "]");
		} else {
			socket->setLabel("NEXT");
		}
	}
}
void GraphicsSectionItem::createInputSocket() {
    if (input == nullptr) {
		input = new SocketItem(0, 0, DIAMETER, DIAMETER, this);
        input->setPen(QPen(Qt::black));
		input->setBrush(QBrush(colorSocketNormal));
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
		if (sLink->isStart())
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
	if ( nextSection != nullptr && !nextSection->removeInputEdge(edge) )
        return false;

    for (int i = 0; i < outputs.size(); ++i) {
		if ( outputs[i] != nullptr && outputs[i]->removeEdge(edge) ) {
            scene()->removeItem(edge);
            delete edge;
			if (!isUpdating) {
				sLink->names[i] = "";
				updateState();
			}

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

	updateSocketLabel(output);
    int socketIdx = outputs.indexOf(output);
	if (socketIdx != -1 && !skipUpdates) {
        // Simple update without changing order - lifehack
        // PUSH new section to yml storage
        sLink->names[socketIdx] = next->sName();
        qDebug() << "addOutputEdge: set name [" << socketIdx << "] = " << next->sName();
		ymlManager->updateSectionLink(sName());
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
		QElapsedTimer timer;
		timer.start();
		qInfo() << "Editing section [" + sName() + "]..";
		isUpdating = true;

		sectionLink oldData = *sLink;
		sLink->reset();
		dialog.fillLinkData();

		if (oldData == *sLink) {
			qInfo() << "Nothing changed! Skip any updates.";
			return;
		}

		setLabel(sName());
		ymlManager->renameSectionLink(sName(), oldData.sectionName);

		// upd output edges
		upn(i, 0, 6) {
			// if changed
			if ( sLink->names[i] != oldData.names[i] ) {
				if ( outputs[i]->hasEdges() ) {
					qDebug() << "Was changed: remove output {" << qn(i) << "}";
					EdgeItem* edge = outputs[i]->getLastEdge();
					removeOutputEdge(edge);
				}
				if ( outputs[i]->hasEdges() ) {
					qCritical() << "Still has edge after removing!";
					continue;
				}
				if ( sLink->names[i].isEmpty() ) {
					qDebug() << "Was changed: new empty";
					continue;
				}
				qDebug() << "Was changed: add output {" << qn(i) << "} to [" << sLink->names[i] << "]";
				GraphicsSectionItem* nextSection = ymlManager->getSectionItem(sLink->names[i]);
				EdgeItem* newEdge = new EdgeItem;
				newEdge->setStartPoint(outputs[i]);
				outputs[i]->addEdge( newEdge );
				scene()->addItem(newEdge);
				addOutputEdge(nextSection, true, newEdge);
			}
			updateSocketLabel(outputs[i]);
		}

		// handle START change
		if (sLink->isStart() && !oldData.isStart()) {
			while ( input->hasEdges() ) {
				EdgeItem* edge = input->getLastEdge();
				GraphicsSectionItem* startSection = qgraphicsitem_cast<GraphicsSectionItem*>( edge->socketStart->parentItem() );
				qDebug() << "Start section now: remove input edge from [" << startSection->sName() << "]";
				startSection->removeOutputEdge(edge); // auto-remove inputEdges from this
				ymlManager->updateSectionLink(startSection->sName());
			}
			input->hide();
			qInfo() << "Start section now: hide input socket";
		}
		if (!sLink->isStart() && oldData.isStart()) {
			input->show();
			qInfo() << "Not-start section now: show input socket";
		}

		if (sLink->type != oldData.type) {
			// handle type changes
			switch (sLink->type) {
				case choiceS:
				case randomS: {
					qDebug() << "Choice/Random now: show all outputs";
					upn(i, 0, 6) {
						outputs[i]->show();
					}
					break;
				}
				case conditionS: {
					qDebug() << "Condition now: show sockets 0,1 and hide other";
					upn(i, 0, 1) {
						outputs[i]->show();
					}
					upn(i, 2, 6) {
						outputs[i]->hide();
					}
					break;
				}
				case nextS:
				case scriptS: {
					qDebug() << "Next/Script now: show socket 0 and hide other";
					upn(i, 0, 0) {
						outputs[i]->show();
					}
					upn(i, 1, 6) {
						outputs[i]->hide();
					}
					break;
				}
				case exitS: {
					qDebug() << "Exit now: hide all sockets";
					upn(i, 0, 6) {
						outputs[i]->hide();
					}
					break;
				}
			}
		}
		// upd instances in prev sections
		if (sName() != oldData.sectionName) {
			qInfo() << "Change section name from [" << oldData.sectionName << "] to [" << sName() << "]";
			QVector<EdgeItem*> inputEdges = input->getEges();
			upn(i, 0, inputEdges.size() - 1) {
				GraphicsSectionItem* startSection = qgraphicsitem_cast<GraphicsSectionItem*>( inputEdges[i]->socketStart->parentItem() );
				if (startSection == nullptr) {
					qCritical() << "null start section for input edge {" << qn(i) << "}";
					continue;
				}
				upn(j, 0, startSection->sLink->names.size() - 1) {
					if (startSection->sLink->names[j] == oldData.sectionName) {
						qDebug() << "rename entry {" << qn(j) << "} in start section";
						startSection->sLink->names[j] = sName();
						ymlManager->updateSectionLink(startSection->sName());
						break;
					}
				}
			}
		}
		updateOutputs();

		ymlManager->updateSectionLink(sName());
		ymlManager->info("Section [" + sName() + "] was updated in " + QString::number(timer.elapsed()) + " ms");
		isUpdating = false;
		updateState();
		/*
		 * 1) RESET ALL
		 * 2) PUSH all data from dialog
		 * 3) UPDATE everything (check if outputs/inputs were changed -> update edges)
		 */
    }
}
void GraphicsSectionItem::removeMe() {
	QMessageBox msgBox;
	msgBox.setText("Do you want to delete section [" + sName() + "] ?");
	msgBox.setInformativeText("It will remove all it's connection and shots.");
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::Yes);
	int ret = msgBox.exec();

	if ( ret != QMessageBox::Yes ) {
		return;
	}

	QElapsedTimer timer;
	timer.start();
	isUpdating = true;
	qInfo() << "Deleting section [" + sName() + "]..";

	upn(i, 0, outputs.size() - 1) {
		if ( outputs[i] == nullptr )
			continue;
		// if changed
		if ( outputs[i]->hasEdges() ) {
			qDebug() << "Was removed: remove output {" << qn(i) << "}";
			EdgeItem* edge = outputs[i]->getLastEdge();
			removeOutputEdge(edge);
		}
		if ( outputs[i]->hasEdges() ) {
			qCritical() << "Still has edge after removing!";
			continue;
		}
	}

	upn(i, 0, outputs.size() - 1) {
		if ( outputs[i] == nullptr )
			continue;
		scene()->removeItem(outputs[i]);
		delete outputs[i];
		outputs[i] = nullptr;
	}
	outputs.clear();

	if (input != nullptr) {
		while ( input->hasEdges() ) {
			EdgeItem* edge = input->getLastEdge();
			GraphicsSectionItem* startSection = qgraphicsitem_cast<GraphicsSectionItem*>( edge->socketStart->parentItem() );
			if ( !startSection->removeOutputEdge(edge) ) {
				qCritical() << "Removing input edge failed!";
			}
			// edge is not more valid!
			ymlManager->updateSectionLink(startSection->sName());
			qDebug() << "removed input edge: upd [" << startSection->sName() << "]";
		}
		scene()->removeItem(input);
		delete input;
		input = nullptr;
	}

	ymlManager->deleteSection( sName() );
	ymlManager->info("Section was deleted in " + QString::number(timer.elapsed()) + " ms");
}
