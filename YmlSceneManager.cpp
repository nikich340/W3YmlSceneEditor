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
        return Node(rhs);
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

bool YmlSceneManager::loadYmlFile(QString path) {
    filePath = path;
    qDebug() << "Parsing " + path + "...";

    // load yml
    try {
        root.reset();
        root = YAML::LoadFile(path.toStdString());
        emit debugInfo("Yml loaded: OK! " + path);
    } catch (const YAML::BadFile& e) {
        emit debugInfo(to_qstr("Error: ") + e.what());
        return false;
    } catch (const YAML::ParserException& e) {
        emit debugInfo(to_qstr("Error: ") + e.what());
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

    // dump back to yml for check
    std::ofstream fout("D:/QT_projects/build-YmlSceneEditor-Desktop_Qt_5_12_6_MinGW_64_bit-Debug/test_write.yml");
    fout << root;

    return true;
}

bool YmlSceneManager::loadSectionsInfo() {

    if (root["dialogscript"]) {
        for(YAML::const_iterator it = root["dialogscript"].begin(); it != root["dialogscript"].end(); ++it) {
            QString sectionName = it->XX.as<QString>();

               // section
            if ( sectionName.startsWith("section_") || sectionName.startsWith("script_") ) {
                if (sectionGraph.contains(sectionName)) {
                    emit debugInfo("Error: duplicated section name: " + sectionName);
                    return false;
                }

                // analyze for linked sections
                YAML::Node sectionNode(it->YY);
                if (!sectionNode.IsSequence()) {
                    emit debugInfo("Error: section [" + sectionName + "] not a sequence");
                    return false;
                }

                  // CHECK: only last element?
                YAML::Node lastSectionNode(sectionNode[sectionNode.size() - 1]);
                sectionLink* tmpLink = new sectionLink;

                   // NEXT, (RANDOM, CHOICE)
                if (lastSectionNode.IsMap()) {
                    QString lastSectionCue = lastSectionNode.begin()->XX.as<QString>();
                    if (lastSectionCue == "NEXT") {
                            // simple NEXT
                        if (lastSectionNode.begin()->YY.IsScalar()) {
                            tmpLink->names.push_back( lastSectionNode.begin()->YY.as<QString>() );
                            // advanced NEXT with condition
                        } else if (lastSectionNode.begin()->YY.IsMap()
                                   && lastSectionNode.begin()->YY["condition"]
                                   && lastSectionNode.begin()->YY["condition"].IsSequence()
                                   && lastSectionNode.begin()->YY["condition"].size() == 3
                                   && lastSectionNode.begin()->YY["on_true"]
                                   && lastSectionNode.begin()->YY["on_false"]) {
                            tmpLink->isCondition = true;
                            tmpLink->names.push_back( lastSectionNode.begin()->YY["on_true"].as<QString>() );
                            tmpLink->names.push_back( lastSectionNode.begin()->YY["on_false"].as<QString>() );
                            tmpLink->conditions.push_back( lastSectionNode.begin()->YY["condition"].as<ymlCond>() );
                        } else {
                            emit debugInfo("Error: [" + sectionName + "]: incorrect last element");
                            return false;
                        }
                    } else if (lastSectionCue == "CHOICE"
                               && lastSectionNode.begin()->YY.IsSequence()) {
                        tmpLink->isChoice = true;

                        // iterate through all choices
                        for (auto jt = lastSectionNode.begin()->YY.begin(); jt != lastSectionNode.begin()->YY.end(); ++jt) {
                            YAML::Node tmpNode = *jt;
                            if (tmpNode.IsSequence()) {
                                tmpLink->choiceLines.push_back( tmpNode[0].as<QString>() );
                                tmpLink->names.push_back( tmpNode[1].as<QString>() );

                                if ( tmpNode.size() > 2 ) {
                                    tmpLink->actionTypes.push_back( tmpNode[2].as<QString>() );
                                } else {
                                    tmpLink->actionTypes.push_back( QString() );
                                }
                            } else if (tmpNode.IsMap()) {
                                if ( tmpNode["choice"] ) {
                                    tmpLink->choiceLines.push_back( tmpNode["choice"][0].as<QString>() );
                                    tmpLink->names.push_back( tmpNode["choice"][1].as<QString>() );

                                    if ( tmpNode["choice"].size() > 2 ) {
                                        tmpLink->actionTypes.push_back( tmpNode["choice"][2].as<QString>() );
                                    } else {
                                        tmpLink->actionTypes.push_back( QString() );
                                    }

                                    if ( tmpNode["emphasize"] ) {
                                        tmpLink->emphasize.push_back( tmpNode["emphasize"].as<bool>() );
                                    } else {
                                        tmpLink->emphasize.push_back( false );
                                    }

                                    if ( tmpNode["single_use"] ) {
                                        tmpLink->emphasize.push_back( tmpNode["single_use"].as<bool>() );
                                    } else {
                                        tmpLink->emphasize.push_back( false );
                                    }

                                    if ( tmpNode["condition"] ) {
                                        tmpLink->conditions.push_back( tmpNode["condition"].as<ymlCond>() );
                                    } else {
                                        tmpLink->conditions.push_back( ymlCond() );
                                    }
                                } else if ( tmpNode["TIME_LIMIT"] ) {
                                    tmpLink->timeLimit = tmpNode["TIME_LIMIT"].as<float>();
                                }
                            } else {
                                emit debugInfo("Error: [" + sectionName + "]: incorrect choices syntax");
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
                                tmpLink->names.push_back(tmpNode.as<QString>());
                            } else {
                                emit debugInfo("Error: [" + sectionName + "]: incorrect RANDOM syntax");
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
                            tmpLink->names.push_back(tmpNode.as<QString>());
                        } else {
                            emit debugInfo("Error: [" + sectionName + "]: incorrect randomizer syntax");
                            return false;
                        }
                    }
                }

                // all is fine, add to graph
                sectionGraph.insert(sectionName, tmpLink);
                if ( sectionName.startsWith("section_start") ) {
                    startSections.push_back( sectionName );
                }
                qDebug() << "ADD: " << sectionName;
            }
        }
    } else {
        emit debugInfo("Error: no dialogscript sections found!");
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
        it->fillCleanOutputs();
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
        if ( !pSect->addOutputEdge(itemBySection[next]) ) {
            // FAILED to add new socket, incorrect yml!
            return false;
        }
    }
    return true;
}


void YmlSceneManager::removeSectionLink(QString sectionName) {
    delete sectionGraph[sectionName];
    sectionGraph.remove(sectionName);
}
sectionLink* YmlSceneManager::getSectionLink(QString sectionName) {
     return sectionGraph[sectionName];
}
