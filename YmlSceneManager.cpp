#include "YmlSceneManager.h"
#include "GraphicsSectionItem.h"
#include <yaml-cpp/exceptions.h>

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
	struct convert<QVector3D> {
	  static Node encode(const QVector3D& rhs) {
		Node node;
		node.push_back(rhs.x());
		node.push_back(rhs.y());
		node.push_back(rhs.z());
		return node;
	  }

	  static bool decode(const Node& node, QVector3D& rhs) {
		if (!node.IsSequence())
			return false;
		rhs.setX( node[0].as<float>() );
		rhs.setY( node[1].as<float>() );
		rhs.setZ( node[2].as<float>() );
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

YmlSceneManager::YmlSceneManager(QObject *parent, QGraphicsScene* gScene) : QObject(parent)
{
	pScene = gScene;
    qDebug() << "YmlSceneManager()";

	/* read vanilla lines info */
	QFile inputFile(":/lines.en.csv");
	if (inputFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
	   QTextStream in(&inputFile);
	   dialogLine newLine;
	   QStringList lst;
	   int cnt = 0;

	   while (!in.atEnd())
	   {
		  lst = in.readLine().split("|");
		  if (lst.size() < 4) {
			  qWarning() << "Bad line: " << lst;
			  continue;
		  }
		  ++cnt;
		  newLine.id = lst[0];
		  newLine.hex = lst[1];
		  newLine.duration = lst[2].toDouble();
		  newLine.text = lst[3];

		  lineById[newLine.id] = newLine;
	   }
	   inputFile.close();
	   qInfo() << "Loaded {" << lineById.size() << "} dg lines of total {" << cnt << "}.";
	} else {
		qCritical() << "Failed to read lines.en.csv!";
	}
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
	itemBySectionName.clear();
    sectionsByDepth.clear();
	for (auto item : unusedItems) {
		delete item;
	}
	unusedItems.clear();

    // load extra info
    if (!loadSectionsInfo()) {
        return false;
    }

	if (!loadShotsInfo()) {
		return false;
	}

	hasChanges = false;
    return true;
}
bool YmlSceneManager::saveYmlFile() {
    QFile ymlFile(filePath);

    if (ymlFile.exists())
        ymlFile.copy(filePath + ".backup");
    else
        return false;
    ymlFile.close();

	if ( sectionNames.empty() ) {
		QMessageBox msgBox;
		msgBox.setText("Save is not allowed: no sections found!");
		msgBox.setInformativeText("Scene should contain at least start and exit sections.");
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		int ret = msgBox.exec();

		return false;
	}
	for (auto name : sectionNames) {
		if ( itemBySectionName[name]->state == GraphicsSectionItem::incomplete ) {
			QMessageBox msgBox;
			msgBox.setText("Save is not allowed: section [" + name + "] is incomplete!");
			msgBox.setInformativeText("Make double click on it to edit settings.");
			msgBox.setStandardButtons(QMessageBox::Ok);
			msgBox.setDefaultButton(QMessageBox::Ok);
			int ret = msgBox.exec();

			return false;
		}
	}

	std::ofstream out(filePath.toStdString());
	out.clear();

    if (out.good())
        out << root;
    else
        return false;

    out.close();
	hasChanges = false;
    return true;
}

bool YmlSceneManager::requestSave() {
	QMessageBox msgBox;
	msgBox.setText("WARNING: Current scene contains unsaved changes.");
	msgBox.setInformativeText("Do you want to proceed anyway?");
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::No);
	int ret = msgBox.exec();

	return (ret == QMessageBox::No);
}

bool YmlSceneManager::loadSectionsInfo() {
    if (root["dialogscript"]) {
        for(YAML::const_iterator it = root["dialogscript"].begin(); it != root["dialogscript"].end(); ++it) {
			QString sectionName = it->XX.as<QString>().toLower(); // make all names toLower!
			qDebug() << "sectionName: " << sectionName;

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
					if (lastSectionCue.toUpper() == "NEXT") {
                            // simple NEXT
                        if (lastSectionNode.begin()->YY.IsScalar()) {
                            tmpLink->addChoice( lastSectionNode.begin()->YY.as<QString>() );
							tmpLink->type = nextS;
                            // advanced NEXT with condition
                        } else if (lastSectionNode.begin()->YY.IsMap()
                                   && lastSectionNode.begin()->YY["condition"]
                                   && lastSectionNode.begin()->YY["condition"].IsSequence()
                                   && lastSectionNode.begin()->YY["condition"].size() == 3
                                   && lastSectionNode.begin()->YY["on_true"]
                                   && lastSectionNode.begin()->YY["on_false"]) {
							tmpLink->type = conditionS;
                            tmpLink->addChoice( lastSectionNode.begin()->YY["on_true"].as<QString>(), QString(), choiceAction(), lastSectionNode.begin()->YY["condition"].as<ymlCond>() );
                            tmpLink->addChoice( lastSectionNode.begin()->YY["on_false"].as<QString>() );
                        } else {
                            error("Error: [" + sectionName + "]: incorrect last element");
                            return false;
                        }
					} else if (lastSectionCue.toUpper() == "CHOICE"
                               && lastSectionNode.begin()->YY.IsSequence()) {
						tmpLink->type = choiceS;
						qDebug() << "CHoice!";

                        // iterate through all choices
                        for (auto jt = lastSectionNode.begin()->YY.begin(); jt != lastSectionNode.begin()->YY.end(); ++jt) {
                            YAML::Node tmpNode = *jt;
                            if (tmpNode.IsSequence()) {
                                choiceAction act = choiceAction();
								if ( tmpNode.size() > 2 ) {
									act.action = tmpNode[2].as<QString>();
									if ( tmpNode.size() > 3 ) {
										act.amount = tmpNode[3].as<int>();
										if ( tmpNode.size() > 4 ) {
											act.grantExp = tmpNode[4].as<bool>();
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
									tmpLink->timeLimit = tmpNode["TIME_LIMIT"].as<double>();
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
					} else if (lastSectionCue.toUpper() == "RANDOM"
                               && lastSectionNode.begin()->YY.IsSequence()) {
						tmpLink->type = randomS;

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
						   && lastSectionNode.as<QString>().toUpper() == "EXIT"
                           && sectionName.startsWith("section_exit")) {
					tmpLink->type = exitS;
                   // usual randomizer section
                } else if (sectionNode.IsSequence()
                           && sectionName.startsWith("section_randomizer")) {
					tmpLink->type = randomS;

                    // iterate through all randoms
					for (auto jt = sectionNode.begin(); jt != sectionNode.end(); ++jt) {
                        YAML::Node tmpNode = *jt;
                        if (tmpNode.IsScalar()) {
                            tmpLink->addChoice( tmpNode.as<QString>() );
							qDebug() << "Add random: " << tmpNode.as<QString>();
                        } else {
                            error("Error: [" + sectionName + "]: incorrect randomizer syntax");
                            return false;
                        }
                    }
                }

                if ( sectionName.startsWith("script_") ) {
					tmpLink->type = scriptS;
                }

                if ( sectionName.startsWith("section_start") ) {
                    startSections.push_back( sectionName );
                }

                // all is fine, add to graph
                sectionNames.append(sectionName);
                sectionGraph.insert(sectionName, tmpLink);
				qInfo() << "ADD: " << sectionName;
			} else if ( sectionName == "actors" ) {
				YAML::Node actorsNode(it->YY);
				if (!actorsNode.IsSequence()) {
					error("Error: actors list [" + sectionName + "] not a sequence");
					return false;
				}
				for (size_t j = 0; j < actorsNode.size(); ++j) {
					dgActors.insert( actorsNode[j].as<QString>() );
				}
			} else if ( sectionName == "props" ) {
				YAML::Node propsNode(it->YY);
				if (!propsNode.IsSequence()) {
					error("Error: actors list [" + sectionName + "] not a sequence");
					return false;
				}
				for (size_t j = 0; j < propsNode.size(); ++j) {
					dgProps.insert( propsNode[j].as<QString>() );
				}
			}
        }
    } else {
        error("Error: no dialogscript sections found!");
        return false;
    }

    qDebug() << "Loaded " << sectionGraph.size() << " sections";
    return true;
}

bool YmlSceneManager::loadShotActions(const YAML::Node actsNode, shot& sh) {
	upn(k, 0, (int) actsNode.size() - 1) {
		YAML::Node actionNode = actsNode[k];
		shotAction newAction;
		newAction.actionName = actionNode.begin()->XX.as<QString>().toLower();

		QStringList keys = newAction.actionName.split(".");
		YAML::Node paramNode = actionNode.begin()->YY;
		bool isExtended = paramNode.IsMap();
		qDebug() << "** " << keys;

		if ( keys.isEmpty() ) {
			qCritical() << "Empty shot action keys! Skipped";
			continue;
		}

		if ( keys[0] == "cam" ) {
			newAction.start = paramNode[0].as<double>();
			newAction.values["cam_name"] = paramNode[1].as<QString>();

			// rapid, smooth
			if ( keys.size() > 2 && (keys[2] == "start" || keys[2] == "end") && paramNode.size() > 2 ) {
				newAction.values["cam_ease"] = paramNode[2].as<QString>();
			}
		} else if ( keys[0] == "actor" || keys[0] == "prop" ) {
			if ( keys[1] == "anim" )
			{
				if (isExtended) {
					newAction.start = paramNode[".@pos"][0].as<double>();
					newAction.values["animation"] = paramNode[".@pos"][1].as<QString>();
					if (paramNode["actor"])
						newAction.values["actor"] = paramNode["actor"].as<QString>();

					if (paramNode["blendin"])
						newAction.values["blendin"] = paramNode["blendin"].as<double>();

					if (paramNode["blendout"])
						newAction.values["blendout"] = paramNode["blendout"].as<double>();

					if (paramNode["clipfront"])
						newAction.values["clipfront"] = paramNode["clipfront"].as<double>();

					if (paramNode["clipend"])
						newAction.values["clipend"] = paramNode["clipend"].as<double>();

					if (paramNode["weight"])
						newAction.values["weight"] = paramNode["weight"].as<double>();

					if (paramNode["stretch"])
						newAction.values["stretch"] = paramNode["stretch"].as<double>();
				} else {
					newAction.start = paramNode[0].as<double>();
					newAction.values["animation"] = paramNode[1].as<QString>();
				}
			}
			else if ( keys[1] == "mimic" )
			{
				if (isExtended) {
					newAction.start = paramNode[".@pos"][0].as<double>();
					newAction.values["mimic"] = paramNode[".@pos"][1].as<QString>();
					if (paramNode["actor"])
						newAction.values["actor"] = paramNode["actor"].as<QString>();

					if (paramNode["emotional_state"])
						newAction.values["emotional_state"] = paramNode["emotional_state"].as<QString>();

					if (paramNode["pose"])
						newAction.values["pose"] = paramNode["pose"].as<QString>();

					if (paramNode["eyes"])
						newAction.values["eyes"] = paramNode["eyes"].as<QString>();

					if (paramNode["anim"])
						newAction.values["anim"] = paramNode["anim"].as<QString>();

					if (paramNode["weight"])
						newAction.values["weight"] = paramNode["weight"].as<double>();

					if (paramNode["duration"])
						newAction.values["duration"] = paramNode["duration"].as<double>();
				} else {
					newAction.start = paramNode[0].as<double>();
					newAction.values["mimic"] = paramNode[1].as<QString>();
				}
			}
			else if ( keys[1] == "gamestate" )
			{
				if (isExtended) {
					newAction.start = paramNode[".@pos"][0].as<double>();
					newAction.values["actor"] = paramNode[".@pos"][1].as<QString>();
					if (paramNode["action"])
						newAction.values["action"] = paramNode["action"].as<QString>();
					if (paramNode["behavior"])
						newAction.values["behavior"] = paramNode["behavior"].as<QString>();
				} else {
					newAction.start = paramNode[0].as<double>();
					newAction.values["actor"] = paramNode[1].as<QString>();
				}
			}
			else if ( keys[1] == "placement" )
			{
				newAction.start = paramNode[0].as<double>();
				newAction.values["actor"] = paramNode[1].as<QString>();
				newAction.values["pos"] = paramNode[2].as<QVector3D>();
				newAction.values["rot"] = paramNode[3].as<QVector3D>();

				if ( keys.size() > 2 && (keys[2] == "start" || keys[2] == "end") && paramNode.size() > 4 ) {
					newAction.values["ease"] = paramNode[4].as<QString>();
				}
			}
			else if ( keys[1] == "pose" )
			{
				newAction.start = paramNode[0].as<double>();
				newAction.values["pose"] = paramNode[1].as<QString>();
			}
			else if ( keys[1] == "lookat" )
			{
				if (isExtended) {
					newAction.start = paramNode[".@pos"][0].as<double>();
					newAction.values["actor"] = paramNode[".@pos"][1].as<QString>();
					if (paramNode[".@pos"][2].IsSequence()) {
						newAction.values["lookat_pos"] = paramNode[".@pos"][2].as<QVector3D>();
					} else {
						newAction.values["lookat_actor"] = paramNode[".@pos"][2].as<QString>();
					}

					if (paramNode["turn"])
						newAction.values["turn"] = paramNode["turn"].as<QString>();

					if (paramNode["speed"])
						newAction.values["speed"] = paramNode["speed"].as<double>();
				} else {
					newAction.start = paramNode[0].as<double>();
					newAction.values["actor"] = paramNode[1].as<QString>();
					if (paramNode[2].IsSequence()) {
						newAction.values["lookat_pos"] = paramNode[2].as<QVector3D>();
					} else {
						newAction.values["lookat_actor"] = paramNode[2].as<QString>();
					}
				}
			}
			else if ( keys[1] == "show" || keys[1] == "hide" || keys[1] == "scabbard" )
			{
				newAction.start = paramNode[0].as<double>();
				newAction.values["actor"] = paramNode[1].as<QString>();
			}
			else if ( keys[1] == "effect" )
			{
				newAction.start = paramNode[0].as<double>();
				newAction.values["actor"] = paramNode[1].as<QString>();
				newAction.values["effect"] = paramNode[2].as<QString>();
			}
			else if ( keys[1] == "sound" )
			{
				newAction.start = paramNode[0].as<double>();
				newAction.values["actor"] = paramNode[1].as<QString>();
				newAction.values["soundEvent"] = paramNode[2].as<QString>();
			}
			else if ( keys[1] == "appearance" )
			{
				newAction.start = paramNode[0].as<double>();
				newAction.values["actor"] = paramNode[1].as<QString>();
				newAction.values["appearance"] = paramNode[2].as<QString>();
			}
			else if ( keys[1] == "equip" )
			{
				newAction.start = paramNode[0].as<double>();
				newAction.values["actor"] = paramNode[1].as<QString>();
				newAction.values["item"] = paramNode[2].as<QString>();
			}
		} else if ( keys[0] == "world" ) {
			if ( keys[1] == "addfact" )
			{
				newAction.start = paramNode[0].as<double>();
				newAction.values["fact"] = paramNode[1].as<QString>();
				newAction.values["value"] = paramNode[2].as<int>();

				if ( paramNode.size() > 3 ) {
					newAction.values["validFor"] = paramNode[3].as<int>();
				}
			}
			else if ( keys[1] == "weather" )
			{
				newAction.start = paramNode[0].as<double>();
				newAction.values["weatherId"] = paramNode[1].as<QString>();

				if ( paramNode.size() > 2 ) {
					newAction.values["blendTime"] = paramNode[2].as<int>();
				}
			}
			else if ( keys[1] == "effect" )
			{
				newAction.start = paramNode[0].as<double>();
				newAction.values["tag"] = paramNode[1].as<QString>();
				newAction.values["effect"] = paramNode[2].as<QString>();
			}
		} else if ( keys[0] == "env" ) {
			if (isExtended) {
				newAction.start = paramNode[".@pos"][0].as<double>();
				newAction.values["envPath"] = paramNode[".@pos"][1].as<QString>();

				if (paramNode[".@pos"].size() > 2)
					newAction.values["blendTime"] = paramNode[".@pos"][2].as<double>();

				if (paramNode["priority"])
					newAction.values["priority"] = paramNode["priority"].as<int>();

				if (paramNode["blendFactor"])
					newAction.values["blendFactor"] = paramNode["blendFactor"].as<double>();
			} else {
				newAction.start = paramNode[0].as<double>();
				newAction.values["envPath"] = paramNode[1].as<QString>();

				if (paramNode.size() > 2)
					newAction.values["blendTime"] = paramNode[2].as<double>();
			}
		} else if ( keys[0] == "fade" ) {
			newAction.start = paramNode[0].as<double>();
			newAction.values["duration"] = paramNode[1].as<double>();

			if (paramNode.size() > 2) {
				newAction.values["color"] = QVector4D(paramNode[2][0].as<int>(), paramNode[2][1].as<int>(),
													  paramNode[2][2].as<int>(), paramNode[2][3].as<int>());
			}
		} else {
			qCritical() << "Unknown shot action!!!";
			continue;
		}

		// load
		sh.actions.pb( newAction );
	}
	return true;
}
bool YmlSceneManager::loadShotsInfo() {
	QSet<QString> nextKeys, cueKeys;
	nextKeys = { "NEXT", "SCRIPT", "RANDOM", "EXIT", "OUTPUT", "BLACKSCREEN", "CAMERA_BLEND" };
	cueKeys = { "CUE", "HINT", "REFERENCE" };

	for (auto sectionName : sectionNames) {
		if (!root["dialogscript"][sectionName])	{
			qDebug() << "Exception0: no dialogscript found for section [" << sectionName << "]";
			continue;
		}

		YAML::Node sNode = root["dialogscript"][sectionName];

		dialogLink newDgLink;
		QSet<QString> usedShotNames;
		QString prevShotName = QString();
		qInfo() << "--- Loading dialog for section [" << sectionName << "]";

		for (auto it = sNode.begin(); it != sNode.end(); ++it) {			
			if ( !it->IsMap() ) {
				qWarning() << "Warning: non-map type (exit?).";
				break;
			}
			if ( it->size() > 1 ) {
				qCritical() << "Exception2: Map has > 1 elements?";
				break;
			}
			if ( !it->begin()->XX.IsScalar() ) {
				qCritical() << "Exception3/1: non-scalar!";
				break;
			}
			QString key = it->begin()->XX.as<QString>();

			if ( nextKeys.contains(key.toUpper()) ) {
				//qDebug() << "NEXT key, continue";
				prevShotName = QString();
				continue;
			}
			if ( cueKeys.contains(key.toUpper()) ) {
				//qDebug() << "CUE key";
				prevShotName = it->begin()->YY.as<QString>();
				continue;

			} else if ( key.toUpper() == "PAUSE" || key.toUpper() == "CHOICE" ) {
				//qDebug() << "CHOICE/PAUSE key";
				newDgLink.lines.pb( key.toUpper() );
				newDgLink.speakers.pb( key.toUpper() );
				double dur;
				if (key.toUpper() == "PAUSE") {
					dur = it->begin()->YY.as<double>();
				} else {
					dur = sectionGraph[sectionName]->timeLimit;
					if (dur < 0.0)
						dur = 10.0;
				}
				newDgLink.durations.pb( dur );

				shot newShot;
				if ( prevShotName.isEmpty() ) {
					prevShotName = key.toUpper() + "_";
					int idx = 1;
					while ( usedShotNames.contains( prevShotName + qn(idx) ) ) {
						++idx;
					}
					prevShotName += qn(idx);
				}
				newShot.shotName = prevShotName;
				usedShotNames.insert(prevShotName);
				newDgLink.shots.pb( newShot );

				prevShotName = QString();

			} else if ( dgActors.contains(key) ) {
				if ( !it->begin()->YY.IsScalar() ) {
					qCritical() << "Exception3/2: non-scalar YY (actor)";
					break;
				}
				//qDebug() << "ACTOR key";
				QString line = it->begin()->YY.as<QString>();
				newDgLink.lines.pb( line );
				newDgLink.speakers.pb( key );
				newDgLink.durations.pb( getTextDuration(line) ); // TODO: try extract duration

				shot newShot;
				if ( prevShotName.isEmpty() ) {
					prevShotName = key;
					int idx = 1;
					while ( usedShotNames.contains( prevShotName + qn(idx) ) ) {
						++idx;
					}
					prevShotName += qn(idx);
				}
				newShot.shotName = prevShotName;
				usedShotNames.insert(prevShotName);
				newDgLink.shots.pb( newShot );

				prevShotName = QString();

			} else {
				qCritical() << "UNKNOWN KEY?? " << key;
				prevShotName = QString();
				continue;
			}
		}
		/*upn(i, 0, newDgLink.lines.size() - 1) {
			qDebug() << "Speaker: [" << newDgLink.speakers[i] << "], line: [" << newDgLink.lines[i] << "], shot alias: ["
					 << newDgLink.shots[i].shotName << "], duration: [" << newDgLink.durations[i] << "]";
		}*/
		newDgLink.calculateTotalDuration();
		dgLinkBySectionName[sectionName]= newDgLink;
	}

	if (root["storyboard"]) {
		for (auto it = root["storyboard"].begin(); it != root["storyboard"].end(); ++it) {
			QString sectionName = it->XX.as<QString>();
			qInfo() << "Loading storyboard for section: [" << sectionName << "]";

			if ( !it->YY.IsMap() ) {
				qCritical() << "Exception4: sbui section not a map!";
				continue;
			}
			if ( !sectionNames.contains(sectionName) ) {
				qInfo() << "Skipping: not a real section (defaults?)";
				continue;
			}

			for (auto jt = it->YY.begin(); jt != it->YY.end(); ++jt) {
				QString shotName = jt->XX.as<QString>();
				qInfo() << "Loading shot: [" << shotName << "]";

				if ( !jt->YY.IsSequence() ) {
					qCritical() << "Exception5: shot actions are not a list!";
					continue;
				}
				int shotIdx = dgLinkBySectionName[sectionName].getIdx(shotName);
				if (shotIdx == -1) {
					qCritical() << "Exception6: shot was not found in dialogscript!";
					continue;
				}

				if ( !loadShotActions(jt->YY, dgLinkBySectionName[sectionName].shots[shotIdx]) ) {
					return false;
				}
			}
		}

	} else {
		qWarning() << "No sbui shots found!";
		return false;
	}
	return true;
}

bool YmlSceneManager::drawSectionsGraph() {
    for (auto s : startSections) {
        if ( !dfsPrepareGraph(s, 0) )
            return false;
    }

    qreal x = 50;
	qreal x_offset = qMin( WIDTH * 2.5, (SCENE_WIDTH * 0.95) / sectionsByDepth.size() );
	qreal y_max = -1, y_min = SCENE_HEIGHT + 1;
    for (int i = 0; i < sectionsByDepth.size(); ++i) {
		qreal y = 0;
		qreal y_offset = (SCENE_HEIGHT * 0.95) / (sectionsByDepth[i].size() + 1);

		bool wasStep = true;
		qreal x_offset2 = 0;
		if (y_offset < HEIGHT) {
			x_offset2 = WIDTH / 1.25;
		}
        for (int j = 0; j < sectionsByDepth[i].size(); ++j) {
			y += y_offset;
			GraphicsSectionItem* pItem = itemBySectionName[sectionsByDepth[i][j]];
			if (wasStep) {
				pItem->setPos(x, y);
				wasStep = false;
			} else {
				pItem->setPos(x + x_offset2, y);
				wasStep = true;
			}
			y_max = qMax(y_max, y);
			y_min = qMin(y_min, y);
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
	for (auto it : itemBySectionName) {
        it->fillCleanSockets();
        it->updateState();
    }
	pScene->views().first()->fitInView(0, y_min, x, y_max, Qt::KeepAspectRatio);

	hasChanges = false;
    return true;

}

bool YmlSceneManager::dfsPrepareGraph(QString sectionName, int depth) {
    if ( depth > 0 && startSections.contains(sectionName) ) {
        // TRYING TO ADD section_start as NEXT, incorrect yml!
        return false;
    }
	if ( !sectionGraph.contains(sectionName) ) {
		qDebug() << "Non-existing section: " << sectionName;
		return false;
	}

    if ( wasDrawn.contains(sectionName) ) {
        return true;
    }

    wasDrawn.insert(sectionName);
	GraphicsSectionItem* newSect = new GraphicsSectionItem;
	/*? if ( unusedItems.isEmpty() ) {
		newSect = new GraphicsSectionItem;
	} else {
		// Reuse deleted items!
		newSect = unusedItems.last();
		unusedItems.pop_back();
	}*/
    newSect->setLabel(sectionName);
    newSect->setSectionLink(getSectionLink(sectionName));
    newSect->setYmlManager(this);
    pScene->addItem( newSect );
	itemBySectionName[sectionName] = newSect;

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
	GraphicsSectionItem* pSect = itemBySectionName[sectionName];

    for (auto next : sectionGraph[sectionName]->names) {
        if ( !dfsDrawGraph(next) ) {
            return false;
        }
        //qDebug() << "Dfs draw graph: [" << sectionName << "]->[" << next << "]";
		if ( !pSect->addOutputEdge(itemBySectionName[next]) ) {
            qDebug() << "FAILED to add new socket, incorrect yml!";
            return false;
        }
    }
    return true;
}

void YmlSceneManager::addSectionLink(QPointF pos) {
	QString sectionName = "section_new_";
	int j = 1;
	while ( sectionNames.contains(sectionName + qn(j)) ) {
		++j;
	}
	sectionName = sectionName + qn(j);
	sectionNames.append(sectionName);
	sectionLink* tmpLink = new sectionLink;
	tmpLink->type = nextS;
	tmpLink->sectionName = sectionName;
	sectionGraph.insert(sectionName, tmpLink);

	GraphicsSectionItem* newSect = new GraphicsSectionItem;
	newSect->setLabel(sectionName);
	newSect->setSectionLink(tmpLink);
	newSect->setYmlManager(this);
	newSect->setPos(pos);
	pScene->addItem( newSect );
	newSect->fillCleanSockets();

	itemBySectionName[sectionName] = newSect;

	root["dialogscript"][sectionName] = YAML::Load("- NEXT: invalid_link");

	qInfo() << "ADD: " << sectionName;
}

void YmlSceneManager::renameSectionLink(QString sectionName, QString oldName) {
	sectionLink* link = sectionGraph[oldName];
	GraphicsSectionItem* item = itemBySectionName[oldName];

	// rename all entries!
	if (oldName != sectionName) {
		sectionGraph.remove(oldName);
		sectionGraph[sectionName] = link;
		itemBySectionName.remove(oldName);
		itemBySectionName[sectionName] = item;
		sectionNames.removeAll(oldName);
		sectionNames << sectionName;

		YAML::Node tempNode = root["dialogscript"];
		for (auto it = tempNode.begin(); it != tempNode.end(); ++it) {
			if (it->first.as<QString>() == oldName) {
				it->first = sectionName;
				break;
			}
		}
	}
}

void YmlSceneManager::updateSectionLink(QString sectionName) {
	sectionLink* link = sectionGraph[sectionName];

    if (root["dialogscript"][sectionName]) {
        YAML::Node sNode = root["dialogscript"][sectionName];
        YAML::Node sNode2;
		switch ( link->type ) {
			case exitS: {
				// simple EXIT scalar
				sNode2 = "EXIT";
				break;
			}
			case choiceS: {
				YAML::Node tempSeq;
				YAML::Node tempMap;
				for (int i = 0; i < link->names.size(); ++i) {
					if ( link->names[i].isEmpty() )
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
				break;
			}
			case conditionS: {
				YAML::Node tempMap;
				tempMap["condition"] = link->conditions[0];
				tempMap["condition"].SetStyle(YAML::EmitterStyle::Flow);
				tempMap["on_true"] = link->names[0];
				tempMap["on_false"] = link->names[1];
				sNode2["NEXT"] = tempMap;
				break;
			}
			case randomS: {
				YAML::Node tempSeq;
				for (auto s : link->names) {
					if ( s.isEmpty() )
						continue;
					tempSeq.push_back(s);
				}
				sNode2["RANDOM"] = tempSeq;
				break;
			}
			default: {
				// { scriptS/nextS }
				// simple block NEXT: section_next
				sNode2["NEXT"] = link->names[0];
				break;
			}
		}

        // update last element in section node
        sNode[sNode.size() - 1] = sNode2;
    } else {
        error("ERROR: section [" + sectionName + "] not found during update!");
    }
}

void YmlSceneManager::deleteSection(QString sectionName) {
    delete sectionGraph[sectionName];
    sectionGraph.remove( sectionName );
	sectionNames.removeAll( sectionName );

	GraphicsSectionItem* pItem = itemBySectionName[sectionName];
	pScene->removeItem( pItem );
	itemBySectionName.remove( sectionName );
	unusedItems.push_back( pItem ); // instead of unsafe(?) deleting from heap

	if (root["dialogscript"][sectionName]) {
		root["dialogscript"].remove( sectionName );
	}
	if (root["storyboard"][sectionName]) {
		root["storyboard"].remove( sectionName );
	}
}
sectionLink* YmlSceneManager::getSectionLink(QString sectionName) {
	if ( sectionGraph.contains(sectionName) )
		return sectionGraph[sectionName];
	else
		return nullptr;
}
GraphicsSectionItem* YmlSceneManager::getSectionItem(QString sectionName) {
	if ( itemBySectionName.contains(sectionName) )
		return itemBySectionName[sectionName];
	else
		return nullptr;
}

QStringList YmlSceneManager::getSectionNames() {
    return sectionNames;
}

QString YmlSceneManager::getCleanLine(QString text) {
	int text_pos = qMax( text.lastIndexOf(']'), text.lastIndexOf('|') );
	if (text_pos > 0)
		++text_pos;
	return text.mid(text_pos);
}
double YmlSceneManager::getTextDuration(QString line) {
	QString duration;
	int cnt = 0;
	bool ok_dur = true;
	for (auto c : line) {
		if (c == ']')
			++cnt;
		if (cnt == 1) {
			duration.pb(c);
			if (!c.isDigit() && c != '.')
				ok_dur = false;
		}
		if (c == '[')
			++cnt;
	}
	if (ok_dur && duration.toDouble() > 0.0)
		return duration.toDouble();

	QString id;
	cnt = 0;
	bool ok_id = false;
	upn(i, 0, line.length() - 1) {
		QChar c = line[i];
		if (c.isDigit()) {
			id.pb(c);
			++cnt;
			if (cnt == 10 && i+1 < line.length() && line[i + 1] == '|') {
				ok_id = true;
				break;
			}
		} else {
			cnt = 0;
			id.clear();
		}
	}
	if (ok_id && lineById.contains(id) && lineById[id].duration > 0.0) {
		return lineById[id].duration;
	}

	int text_pos = qMax( line.lastIndexOf(']'), line.lastIndexOf('|') );
	if (text_pos > 0)
		++text_pos;

	int length = line.length() - text_pos;
	double factor;

	/* @rmemr */
	if (length < 20) {
		factor = 0.11;
	} else if (length < 50) {
		factor = 0.08;
	} else if (length < 75) {
		factor = 0.075;
	} else {
		factor = 0.073;
	}

	return qMax(1.0, factor * length);
		// heuristic "approved duration": letters * lengthDependentFactor sec
		// based on some voiceline duration statistics:
		//     <20: 0.1084             0.11
		//     <50: 0.0799             0.08
		//     <75: 0.07404            0.075
		//     <100: 0.0733            0.073
		//     <150: 0.0720            0.073
		//     <200: 0.0725            0.073
}

void YmlSceneManager::setShotScenes(QGraphicsScene* gDgScene, QGraphicsScene* gLabelScene, QGraphicsScene* gShotScene) {
	pDgScene = gDgScene;
	pLabelScene = gLabelScene;
	pShotScene = gShotScene;
}

void YmlSceneManager::loadShotEditor(QString sectionName) {
	emit loadShots(sectionName);
}
