#include "YmlSceneManager.h"
#include "GraphicsSectionItem.h"
#include <yaml-cpp/exceptions.h>
#include "constants.h"

#include <QDebug>
#include <fstream>
#include <QFile>
#include <QTextStream>
#define XX first
#define YY second
#define to_qstr(x) QString::fromStdString(x)

namespace YAML {
    template<>
    struct convert<QString> {
      static Node encode(const QString& rhs) {
        Node node(rhs.toStdString());
        return node;
      }

      static bool decode(const Node& node, QString& rhs) {
        if (!node.IsScalar())
            return false;
        rhs = to_qstr(node.Scalar());
        return true;
      }
     };

      template<>
      struct convert<ymlCond> {
        static Node encode(const ymlCond& rhs) {
            Node node;
            node.push_back(rhs.condFact);
            node.push_back(rhs.condOperand);
            node.push_back(rhs.condValue);
            node[0].SetTag("!");
            node[1].SetTag("!");
            return node;
        }

        static bool decode(const Node& node, ymlCond& rhs) {
          if (!node.IsSequence())
              return false;
          rhs.condFact = node[0].as<QString>();
          rhs.condOperand = node[1].as<QString>();
          rhs.condValue = node[2].as<int>();
          return true;
        }
      };
      Emitter& operator <<(Emitter& out, const QString& s) {
          out << s.toStdString();
          return out;
      }
      Emitter& operator <<(Emitter& out, const ymlCond& cond) {
          out << Flow;
          out << BeginSeq << cond.condFact << cond.condOperand << cond.condValue << EndSeq;
          return out;
      }
}

YmlSceneManager::YmlSceneManager(QObject *parent) : QObject(parent)
{
    qDebug() << "YmlSceneManager()";
}

void debugNode(YAML::Node n) {
    switch (n.Type()) {
      case YAML::NodeType::Null:      qDebug() << "Null"; break;
      case YAML::NodeType::Scalar:    qDebug() << "Scalar"; break;
      case YAML::NodeType::Sequence:  qDebug() << "Sequence"; break;
      case YAML::NodeType::Map:       qDebug() << "Map"; break;
      case YAML::NodeType::Undefined: qDebug() << "Undefined"; break;
    }
}
void YmlSceneManager::error(QString s) {
    emit print_error(s);
}
void YmlSceneManager::info(QString s) {
    emit print_info(s);
}

bool YmlSceneManager::loadYmlFile(QString path) {
    filePath = path;
    qDebug() << "Parsing " + path + "...";

    // load yml
    try {
        root.reset();
        root = YAML::LoadFile(path.toStdString());
        error("Yml loaded: OK! " + path);
    } catch (const YAML::BadFile& e) {
        error(to_qstr("Error: ") + e.what());
        return false;
    } catch (const YAML::ParserException& e) {
        error(to_qstr("Error: ") + e.what());
        return false;
    }

    // clear old
    sectionGraph.clear();
    startSections.clear();
    wasDrawn.clear();
    itemBySection.clear();
    sectionsByDepth.clear();

    // load extra info
    if (!loadSectionsInfo()) {
        return false;
    }

    return true;
}
bool YmlSceneManager::saveYmlFile() {
    QFile ymlFile(filePath);

    if (ymlFile.exists())
        ymlFile.copy(filePath + ".backup");
    else
        return false;
    ymlFile.close();

    std::ofstream out(filePath.toStdString() + ".yml");
    out.clear();
    if (out.good())
        out << root;
    else
        return false;

    out.close();
    return true;
}

bool YmlSceneManager::loadSectionsInfo() {

    if (root["dialogscript"]) {
        for(YAML::const_iterator it = root["dialogscript"].begin(); it != root["dialogscript"].end(); ++it) {
            QString sectionName = it->XX.as<QString>();

               // section
            if ( sectionName.startsWith("section_") || sectionName.startsWith("script_") ) {
                if (sectionGraph.contains(sectionName)) {
                    error("Error: duplicated section name: " + sectionName);
                    return false;
                }

                // analyze for linked sections
                YAML::Node sectionNode(it->YY);
                if (!sectionNode.IsSequence()) {
                    error("Error: section [" + sectionName + "] not a sequence");
                    return false;
                }

                  // CHECK: only last element?
                YAML::Node lastSectionNode(sectionNode[sectionNode.size() - 1]);
                sectionLink* tmpLink = new sectionLink;
                tmpLink->sectionName = sectionName;

                   // NEXT, (RANDOM, CHOICE)
                if (lastSectionNode.IsMap()) {
                    QString lastSectionCue = lastSectionNode.begin()->XX.as<QString>();
                    if (lastSectionCue == "NEXT") {
                            // simple NEXT
                        if (lastSectionNode.begin()->YY.IsScalar()) {
                            tmpLink->addChoice( lastSectionNode.begin()->YY.as<QString>() );
                            // advanced NEXT with condition
                        } else if (lastSectionNode.begin()->YY.IsMap()
                                   && lastSectionNode.begin()->YY["condition"]
                                   && lastSectionNode.begin()->YY["condition"].IsSequence()
                                   && lastSectionNode.begin()->YY["condition"].size() == 3
                                   && lastSectionNode.begin()->YY["on_true"]
                                   && lastSectionNode.begin()->YY["on_false"]) {
                            tmpLink->isCondition = true;
                            tmpLink->addChoice( lastSectionNode.begin()->YY["on_true"].as<QString>(), QString(), choiceAction(), lastSectionNode.begin()->YY["condition"].as<ymlCond>() );
                            tmpLink->addChoice( lastSectionNode.begin()->YY["on_false"].as<QString>() );
                        } else {
                            error("Error: [" + sectionName + "]: incorrect last element");
                            return false;
                        }
                    } else if (lastSectionCue == "CHOICE"
                               && lastSectionNode.begin()->YY.IsSequence()) {
                        tmpLink->isChoice = true;

                        // iterate through all choices
                        for (auto jt = lastSectionNode.begin()->YY.begin(); jt != lastSectionNode.begin()->YY.end(); ++jt) {
                            YAML::Node tmpNode = *jt;
                            if (tmpNode.IsSequence()) {
                                choiceAction act = choiceAction();
                                if ( tmpNode["choice"].size() > 2 ) {
                                    act.action = tmpNode["choice"][2].as<QString>();
                                    if ( tmpNode["choice"].size() > 3 ) {
                                        act.amount = tmpNode["choice"][3].as<int>();
                                        if ( tmpNode["choice"].size() > 4 ) {
                                            act.grantExp = tmpNode["choice"][4].as<bool>();
                                        }
                                    }
                                }

                                tmpLink->addChoice( tmpNode[1].as<QString>(), tmpNode[0].as<QString>(), act );
                            } else if (tmpNode.IsMap()) {
                                if ( tmpNode["choice"] ) {
                                    bool emphasize = false, single_use = false;
                                    ymlCond condition;
                                    choiceAction act = choiceAction();
                                    if ( tmpNode["choice"].size() > 2 ) {
                                        act.action = tmpNode["choice"][2].as<QString>();
                                        if ( tmpNode["choice"].size() > 3 ) {
                                            act.amount = tmpNode["choice"][3].as<int>();
                                            if ( tmpNode["choice"].size() > 4 ) {
                                                act.grantExp = tmpNode["choice"][4].as<bool>();
                                            }
                                        }
                                    }
                                    if ( tmpNode["emphasize"] ) {
                                        emphasize = tmpNode["emphasize"].as<bool>();
                                    }
                                    if ( tmpNode["single_use"] ) {
                                        single_use = tmpNode["single_use"].as<bool>();
                                    }
                                    if ( tmpNode["condition"] ) {
                                        condition = tmpNode["condition"].as<ymlCond>();
                                    }

                                    tmpLink->addChoice( tmpNode["choice"][1].as<QString>(), tmpNode["choice"][0].as<QString>(),
                                                        act, condition, single_use, emphasize );
                                } else if ( tmpNode["TIME_LIMIT"] ) {
                                    tmpLink->timeLimit = tmpNode["TIME_LIMIT"].as<float>();
                                }
                            } else {
                                error("Error: [" + sectionName + "]: incorrect choices syntax");
                            }
                            /*if (!jt->YY["choice"] || jt->YY["choice"].size() < 2) {
                                emit debugInfo("Error: [" + sectionName + "]: incorrect choice element");
                                return;
                            }*/
                            //tmpLink.choiceLines.push_back(jt["choice"][0].as<QString>());
                        }
                    } else if (lastSectionCue == "RANDOM"
                               && lastSectionNode.begin()->YY.IsSequence()) {
                        tmpLink->isRandomizer = true;

                        // iterate through all randoms
                        for (auto jt = lastSectionNode.begin()->YY.begin(); jt != lastSectionNode.begin()->YY.end(); ++jt) {
                            YAML::Node tmpNode = *jt;
                            if (tmpNode.IsScalar()) {
                                tmpLink->addChoice( tmpNode.as<QString>() );
                            } else {
                                error("Error: [" + sectionName + "]: incorrect RANDOM syntax");
                                return false;
                            }
                        }
                    }

                    // EXIT
                } else if (lastSectionNode.IsScalar()
                           && lastSectionNode.as<QString>() == "EXIT"
                           && sectionName.startsWith("section_exit")) {
                    tmpLink->isExit = true;
                   // usual randomizer section
                } else if (sectionNode.IsSequence()
                           && sectionName.startsWith("section_randomizer")) {
                    tmpLink->isRandomizer = true;

                    // iterate through all randoms
                    for (auto jt = sectionNode.begin()->YY.begin(); jt != sectionNode.begin()->YY.end(); ++jt) {
                        YAML::Node tmpNode = *jt;
                        if (tmpNode.IsScalar()) {
                            tmpLink->addChoice( tmpNode.as<QString>() );
                        } else {
                            error("Error: [" + sectionName + "]: incorrect randomizer syntax");
                            return false;
                        }
                    }
                }

                if ( sectionName.startsWith("script_") ) {
                    tmpLink->isScript = true;
                }

                if ( sectionName.startsWith("section_start") ) {
                    startSections.push_back( sectionName );
                    tmpLink->isStart = true;
                }

                // all is fine, add to graph
                sectionNames.append(sectionName);
                sectionGraph.insert(sectionName, tmpLink);
                qDebug() << "ADD: " << sectionName;
            }
        }
    } else {
        error("Error: no dialogscript sections found!");
        return false;
    }

    qDebug() << "Loaded " << sectionGraph.size() << " sections";
    return true;
}

bool YmlSceneManager::drawSectionsGraph(QGraphicsScene* gScene) {
    pScene = gScene;

    for (auto s : startSections) {
        if ( !dfsPrepareGraph(s, 0) )
            return false;
    }

    qreal x = 50;
    qreal x_offset = qMin( WIDTH * 2.0, 4900.0 / sectionsByDepth.size() );
    for (int i = 0; i < sectionsByDepth.size(); ++i) {
        qreal y = 0;
        qreal y_offset = 1000.0 / (sectionsByDepth[i].size() + 1);
        for (int j = 0; j < sectionsByDepth[i].size(); ++j) {
            y += y_offset;
            GraphicsSectionItem* pItem = itemBySection[sectionsByDepth[i][j]];
            pItem->setPos(x, y);
        }
        x += x_offset;
    }

    wasDrawn.clear();
    for (auto s : startSections) {
        if ( !dfsDrawGraph(s) ) {
            return false;
        }
    }

    // fill sockets
    for (auto it : itemBySection) {
        it->fillCleanSockets();
        it->updateState();
    }
    return true;

}

bool YmlSceneManager::dfsPrepareGraph(QString sectionName, int depth) {
    if ( depth > 0 && startSections.contains(sectionName) ) {
        // TRYING TO ADD section_start as NEXT, incorrect yml!
        return false;
    }

    if ( wasDrawn.contains(sectionName) ) {
        return true;
    }

    wasDrawn.insert(sectionName);
    GraphicsSectionItem* newSect = new GraphicsSectionItem;
    newSect->setLabel(sectionName);
    newSect->setSectionLink(getSectionLink(sectionName));
    newSect->setYmlManager(this);
    pScene->addItem( newSect );
    itemBySection[sectionName] = newSect;

    if ( sectionsByDepth.size() < depth + 1) {
        sectionsByDepth.push_back(QVector<QString>());
    }
    sectionsByDepth[depth].push_back(sectionName);

    for (auto next : sectionGraph[sectionName]->names) {
        if ( !dfsPrepareGraph(next, depth + 1) ) {
            return false;
        }
    }

    return true;
}

bool YmlSceneManager::dfsDrawGraph(QString sectionName) {
    if ( wasDrawn.contains(sectionName) ) {
        return true;
    }

    wasDrawn.insert(sectionName);
    GraphicsSectionItem* pSect = itemBySection[sectionName];

    for (auto next : sectionGraph[sectionName]->names) {
        if ( !dfsDrawGraph(next) ) {
            return false;
        }
        //qDebug() << "Dfs draw graph: [" << sectionName << "]->[" << next << "]";
        if ( !pSect->addOutputEdge(itemBySection[next]) ) {
            qDebug() << "FAILED to add new socket, incorrect yml!";
            return false;
        }
    }
    return true;
}

void YmlSceneManager::updateSectionLink(QString sectionName) {
    sectionLink* link = sectionGraph[sectionName];
    if (root["dialogscript"][sectionName]) {
        YAML::Node sNode = root["dialogscript"][sectionName];
        YAML::Node sNode2;
        if ( link->isExit ) {
            // simple EXIT scalar
            sNode2 = "EXIT";
        } else if ( link->isRandomizer ) {
            YAML::Node tempSeq;
            for (auto s : link->names) {
                tempSeq.push_back(s);
            }
            sNode2["RANDOM"] = tempSeq;
        } else if ( link->isChoice ) {
            YAML::Node tempSeq;
            YAML::Node tempMap;
            for (int i = 0; i < link->names.size(); ++i) {
                if (link->names[i] == "NOT SET" || link->names[i].isEmpty())
                    continue;
                tempMap.SetStyle(YAML::EmitterStyle::Block);
                tempMap["choice"].SetStyle(YAML::EmitterStyle::Flow);

                // dialog line
                tempMap["choice"].push_back( link->choiceLines[i] );
                // next section name
                tempMap["choice"].push_back( link->names[i] );
                // add action values
                if ( !link->choiceActions[i].action.isEmpty() ) {
                    tempMap["choice"].push_back( link->choiceActions[i].action );
                    if ( link->choiceActions[i].amount != -1 ) {
                        tempMap["choice"].push_back( link->choiceActions[i].amount );
                        if ( link->choiceActions[i].grantExp ) {
                            tempMap["choice"].push_back( true );
                        }
                    }
                }
                // add conition block
                if ( !link->conditions[i].condFact.isEmpty() ) {
                    tempMap["condition"] = link->conditions[i];
                    tempMap["condition"].SetStyle(YAML::EmitterStyle::Flow);
                }
                tempMap["choice"][0].SetTag("!"); // hack to add quotes
                tempMap["choice"][1].SetTag("!"); // hack to add quotes
                // single_use block
                if ( link->single_use[i] )
                    tempMap["single_use"] = true;
                // emphasize block
                if ( link->emphasize[i] )
                    tempMap["emphasize"] = true;

                // push -choice to CHOICE block
                tempSeq.push_back(tempMap);
                tempMap.reset();
            }
            // add TIME_LIMIT if exists
            if ( link->timeLimit > 0.0 ) {
                tempMap["TIME_LIMIT"] = link->timeLimit;
                tempSeq.push_back(tempMap);
            }
            sNode2["CHOICE"] = tempSeq;

        } else if ( link->isCondition ) {
            YAML::Node tempMap;
            tempMap["condition"] = link->conditions[0];
            tempMap["condition"].SetStyle(YAML::EmitterStyle::Flow);
            tempMap["on_true"] = link->names[0];
            tempMap["on_false"] = link->names[1];
            sNode2["NEXT"] = tempMap;
        } else {
            // simple block NEXT: section_next
            sNode2["NEXT"] = link->names[0];
        }
        // update last element in section node
        sNode[sNode.size() - 1] = sNode2;
    } else {
        error("ERROR: section [" + sectionName + "] not found during update!");
    }
}
void YmlSceneManager::removeSectionLink(QString sectionName) {
    delete sectionGraph[sectionName];
    sectionGraph.remove( sectionName );
    sectionNames.removeOne( sectionName );
    // TODO
}
sectionLink* YmlSceneManager::getSectionLink(QString sectionName) {
     return sectionGraph[sectionName];
}

QStringList YmlSceneManager::getSectionNames() {
    return sectionNames;
}
