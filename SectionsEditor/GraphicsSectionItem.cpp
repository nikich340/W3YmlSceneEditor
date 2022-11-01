#include "GraphicsSectionItem.h"
#include "constants.h"
#include "DialogChangeSection.h"

GraphicsSectionItem::GraphicsSectionItem(QGraphicsItem *parent) : super(parent)
{
    setDefaults();
    this->setData(0, "section");
}

GraphicsSectionItem::~GraphicsSectionItem()
{
    if (m_inputSocket != nullptr) {
        m_inputSocket->scene()->removeItem(m_inputSocket);
        delete m_inputSocket;
    }
    upn(i, 0, m_outputSockets.count() - 1) {
        m_outputSockets[i]->scene()->removeItem(m_outputSockets[i]);
        delete m_outputSockets[i];
    }
}
void GraphicsSectionItem::setDefaults() {
    setRect( QRectF(0, 0, CONSTANTS::WIDTH, CONSTANTS::HEIGHT) );
    setPen( QPen(Qt::black) );
    //setBrush( QBrush(QColor(128, 255, 170)) );
    setBrush( QBrush(CONSTANTS::colorSectionNormal) );
	setFlags( QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable );
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
        labelH = CONSTANTS::HEIGHT / 3.5;
	else if (last == 1)
        labelH = CONSTANTS::HEIGHT / 6;

    label->setPos( CONSTANTS::WIDTH / 20, labelH );
	label->setPlainText(newText);
}
SocketItem* GraphicsSectionItem::addCleanOutput() {
    hasCleanOutput = true;
    SocketItem* output = new SocketItem(0, 0, CONSTANTS::DIAMETER, CONSTANTS::DIAMETER, this);
	//output->setParentItem(this);
    output->setPen(QPen(Qt::black));
    output->setBrush(QBrush(CONSTANTS::colorSocketNormal));
    output->isInputSocket = false;

    m_outputSockets.push_back(output);
    if (m_outputSockets.size() > sLink->names.size()) {
        sLink->addChoice();
		//qDebug() << "add clean output {" << qn(sLink->names.size() - 1) << "}, type: " << sLink->type;
        if ( (sLink->type != ESectionChoice && sLink->type != ESectionRandom) && (m_outputSockets.size() > 1 || sLink->type == ESectionExit) )
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
    int cnt = m_outputSockets.size();

	if (sLink->type == ESectionChoice || sLink->type == ESectionRandom) {
        return (cnt < 7);
    }
	if (sLink->type == ESectionCondition) {
        return (cnt < 2);
    }
	return (cnt < 1 && sLink->type != ESectionExit);
}
void GraphicsSectionItem::updateState() {
    int hasOutput = 0;
    int outputCnt = 0;
    for (auto out : m_outputSockets) {
        if ( out->isVisible() ) {
            ++outputCnt;
        }
        if ( out->hasEdges() ) {
            ++hasOutput;
        }
    }

    bool ok = true;
    if (m_inputSocket != nullptr)
        ok &= ( (m_inputSocket->hasEdges() > 0) ^ sLink->isStart() );
	ok &= ( (outputCnt > 0 && hasOutput > 0) || sLink->type == ESectionExit );
	ok &= ( (outputCnt == 2 && hasOutput == 2) || sLink->type != ESectionCondition );

	if (sLink->type == ESectionChoice) {
		for (int i = 0; i < sLink->names.size(); ++i) {
			if (!sLink->names[i].isEmpty() && sLink->choiceLines[i].isEmpty()) {
				ok = false;
				break;
			}
		}
	}

    if (ok) {
        state = SectionState::normal;
        setBrush( QBrush(CONSTANTS::colorSectionNormal) );
    } else {
        state = SectionState::incomplete;
        setBrush( QBrush(CONSTANTS::colorSectionIncomplete) );
    }
	ymlManager->hasChanges = true;
}
void GraphicsSectionItem::updateSocketLabel(SocketItem* socket) {
    int socketIdx = m_outputSockets.indexOf(socket);
	if (socketIdx != -1) {
		if (sLink->type == ESectionChoice && socketIdx < sLink->choiceLines.size()) {
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
		} else if (sLink->type == ESectionCondition && !sLink->conditions[0].condFact.isEmpty()) {
			ymlCond cond = sLink->conditions[0];
			if (socketIdx == 0)
				socket->setLabel(cond.condFact + "\n" + cond.condOperand + QString::number(cond.condValue) + ": [on_true]");
			else if (socketIdx == 1)
				socket->setLabel(cond.condFact + "\n" + cond.condOperand + QString::number(cond.condValue) + ": [on_false]");
		} else if (sLink->type == ESectionRandom) {
			socket->setLabel("[random #" + QString::number(socketIdx + 1) + "]");
		} else {
			socket->setLabel("NEXT");
		}
	}
}
void GraphicsSectionItem::createInputSocket() {
    if (m_inputSocket == nullptr) {
        m_inputSocket = new SocketItem(0, 0, CONSTANTS::DIAMETER, CONSTANTS::DIAMETER, this);
        m_inputSocket->setPen(QPen(Qt::black));
        m_inputSocket->setBrush(QBrush(CONSTANTS::colorSocketNormal));
        m_inputSocket->setPos( -CONSTANTS::DIAMETER/2, (CONSTANTS::HEIGHT - CONSTANTS::DIAMETER)/2 );
        m_inputSocket->setData(0, "inputSocket");
		//input->setParentItem(this);
        m_inputSocket->isInputSocket = true;
    }
}
void GraphicsSectionItem::fillCleanSockets() {
    while ( m_outputSockets.size() < 7 ) {
        addCleanOutput();
    }
    if (m_inputSocket == nullptr) {
        createInputSocket();
		if (sLink->isStart())
            m_inputSocket->hide();
    }
    updateState();
}
bool GraphicsSectionItem::removeInputEdge(EdgeItem *edge) {
    if (edge == nullptr || m_inputSocket == nullptr)
        return false;

    bool ret = m_inputSocket->removeEdge(edge);
    updateState();
    return ret;
}
bool GraphicsSectionItem::removeOutputEdge(EdgeItem *edge) {
    if (edge == nullptr || m_outputSockets.isEmpty())
        return false;

    GraphicsSectionItem* nextSection = static_cast<GraphicsSectionItem*>( edge->socketEnd->parentItem() );
	if ( nextSection != nullptr && !nextSection->removeInputEdge(edge) )
        return false;

    for (int i = 0; i < m_outputSockets.size(); ++i) {
        if ( m_outputSockets[i] != nullptr && m_outputSockets[i]->removeEdge(edge) ) {
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
    if (m_inputSocket == nullptr) {
        createInputSocket();
    }
    m_inputSocket->addEdge( edge );
    updateState();

    edge->setEndPoint(m_inputSocket);
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
        output = static_cast<SocketItem*>(edge->socketStart);
    }

	updateSocketLabel(output);
    int socketIdx = m_outputSockets.indexOf(output);
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
    for (int i = 0; i < m_outputSockets.size(); ++i) {
        if ( m_outputSockets[i]->isVisible() ) {
            ++cnt;
         }
    }
    qreal h = 0;
    qreal h_diff = CONSTANTS::HEIGHT / cnt;
    for (int i = 0; i < m_outputSockets.size(); ++i) {
        if (m_outputSockets[i]->isVisible())
            ++cnt;
    }
    for (int i = 0; i < m_outputSockets.size(); ++i) {
        if ( m_outputSockets[i]->isVisible() ) {
            h += h_diff;
            m_outputSockets[i]->setPos( CONSTANTS::WIDTH - CONSTANTS::DIAMETER/2, h - CONSTANTS::DIAMETER/2 );
         }
    }
    updateOutputEdges();
}
void GraphicsSectionItem::updateOutputEdges() {
    for (int i = 0; i < m_outputSockets.size(); ++i) {
        m_outputSockets[i]->redrawAllEdges();
    }
}
void GraphicsSectionItem::updateInputEdges() {
    if (m_inputSocket != nullptr)
        m_inputSocket->redrawAllEdges();
}

void GraphicsSectionItem::putOutputEdgesOnTop() {
    for (int i = 0; i < m_outputSockets.size(); ++i) {
        if ( m_outputSockets[i]->hasEdges() ) {
            EdgeItem* edge = m_outputSockets[i]->getLastEdge();
			SocketItem* outputSocket = static_cast<SocketItem*>(edge->socketEnd);
			if ( outputSocket->makeTop(edge) == false ) {
				qCritical() << "GraphicsSectionItem::putOutputEdgesOnTop(): failed to put on top!";
			}
		}
	}
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
	if (selectedAction == editAction) {
        event->accept();
        changeMe();
	} else if (selectedAction == deleteAction) {
        event->accept();
        removeMe();
	} else {
		event->ignore();
	}
}

QVariant GraphicsSectionItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) {
	if ( change == QGraphicsItem::ItemSelectedHasChanged ) {
		if ( value.toBool() ) {
            emit ymlManager->sectionLoaded(sLink->sectionName);

			// push output edges on top
			putOutputEdgesOnTop();
		} else {
			qDebug() << "Section de-selected.";
		}
	}
	return QGraphicsItem::itemChange(change, value);
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
                if ( m_outputSockets[i]->hasEdges() ) {
					qDebug() << "Was changed: remove output {" << qn(i) << "}";
                    EdgeItem* edge = m_outputSockets[i]->getLastEdge();
					removeOutputEdge(edge);
				}
                if ( m_outputSockets[i]->hasEdges() ) {
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
                newEdge->setStartPoint(m_outputSockets[i]);
                m_outputSockets[i]->addEdge( newEdge );
				scene()->addItem(newEdge);
				addOutputEdge(nextSection, true, newEdge);
			}
            updateSocketLabel(m_outputSockets[i]);
		}

		// handle START change
		if (sLink->isStart() && !oldData.isStart()) {
            while ( m_inputSocket->hasEdges() ) {
                EdgeItem* edge = m_inputSocket->getLastEdge();
				GraphicsSectionItem* startSection = qgraphicsitem_cast<GraphicsSectionItem*>( edge->socketStart->parentItem() );
				qDebug() << "Start section now: remove input edge from [" << startSection->sName() << "]";
				startSection->removeOutputEdge(edge); // auto-remove inputEdges from this
				ymlManager->updateSectionLink(startSection->sName());
			}
            m_inputSocket->hide();
			qInfo() << "Start section now: hide input socket";
		}
		if (!sLink->isStart() && oldData.isStart()) {
            m_inputSocket->show();
			qInfo() << "Not-start section now: show input socket";
		}

		if (sLink->type != oldData.type) {
			// handle type changes
			switch (sLink->type) {
				case ESectionChoice:
				case ESectionRandom: {
					qDebug() << "Choice/Random now: show all outputs";
					upn(i, 0, 6) {
                        m_outputSockets[i]->show();
					}
					break;
				}
				case ESectionCondition: {
					qDebug() << "Condition now: show sockets 0,1 and hide other";
					upn(i, 0, 1) {
                        m_outputSockets[i]->show();
					}
					upn(i, 2, 6) {
                        m_outputSockets[i]->hide();
					}
					break;
				}
				case ESectionNext:
				case ESectionScript: {
					qDebug() << "Next/Script now: show socket 0 and hide other";
					upn(i, 0, 0) {
                        m_outputSockets[i]->show();
					}
					upn(i, 1, 6) {
                        m_outputSockets[i]->hide();
					}
					break;
				}
				case ESectionExit: {
					qDebug() << "Exit now: hide all sockets";
					upn(i, 0, 6) {
                        m_outputSockets[i]->hide();
					}
					break;
				}
			}
            emit ymlManager->sectionTypeChanged(sName(), sLink->type);
		}
		// upd instances in prev sections
		if (sName() != oldData.sectionName) {
			qInfo() << "Change section name from [" << oldData.sectionName << "] to [" << sName() << "]";
            QVector<EdgeItem*> inputEdges = m_inputSocket->getEges();
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

    upn(i, 0, m_outputSockets.size() - 1) {
        if ( m_outputSockets[i] == nullptr )
			continue;
		// if changed
        if ( m_outputSockets[i]->hasEdges() ) {
			qDebug() << "Was removed: remove output {" << qn(i) << "}";
            EdgeItem* edge = m_outputSockets[i]->getLastEdge();
			removeOutputEdge(edge);
		}
        if ( m_outputSockets[i]->hasEdges() ) {
			qCritical() << "Still has edge after removing!";
			continue;
		}
	}

    upn(i, 0, m_outputSockets.size() - 1) {
        if ( m_outputSockets[i] == nullptr )
			continue;
        scene()->removeItem(m_outputSockets[i]);
        delete m_outputSockets[i];
	}
    m_outputSockets.clear();

    if (m_inputSocket != nullptr) {
        while ( m_inputSocket->hasEdges() ) {
            EdgeItem* edge = m_inputSocket->getLastEdge();
			GraphicsSectionItem* startSection = qgraphicsitem_cast<GraphicsSectionItem*>( edge->socketStart->parentItem() );
			if ( !startSection->removeOutputEdge(edge) ) {
				qCritical() << "Removing input edge failed!";
			}
			// edge is not more valid!
			ymlManager->updateSectionLink(startSection->sName());
			qDebug() << "removed input edge: upd [" << startSection->sName() << "]";
		}
        scene()->removeItem(m_inputSocket);
        delete m_inputSocket;
        m_inputSocket = nullptr;
	}

	ymlManager->deleteSection( sName() );
	ymlManager->info("Section was deleted in " + QString::number(timer.elapsed()) + " ms");
}
