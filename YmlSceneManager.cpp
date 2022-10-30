#include "YmlSceneManager.h"
#include "GraphicsSectionItem.h"
#include <yaml-cpp/exceptions.h>

#include <QDebug>
#include <fstream>
#include <QFile>
#include <QTextStream>

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
    struct convert<QColor> {
      static Node encode(const QColor& rhs) {
        Node node;
        node.push_back(rhs.red());
        node.push_back(rhs.green());
        node.push_back(rhs.blue());
        node.push_back(rhs.alpha());
        return node;
      }

      static bool decode(const Node& node, QColor& rhs) {
        if (!node.IsSequence())
            return false;
        rhs.setRed( node[0].as<int>() );
        rhs.setGreen( node[1].as<int>() );
        rhs.setBlue( node[2].as<int>() );
        if (node.size() > 3)
            rhs.setAlpha( node[3].as<int>() );
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

QStringList YmlSceneManager::nodeKeys(const YAML::Node &node)
{
    QStringList ret;
    if (node.Type() == YAML::NodeType::Map) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            ret.append(it->XKey.as<QString>());
        }
    }
    return ret;
}

YAML::Node YmlSceneManager::firstCloned(const YAML::Node &node)
{
    if (!node.IsSequence() || node.size() < 1)
        return YAML::Node();
    return YAML::Clone(node[0]);
}

YAML::Node YmlSceneManager::lastCloned(const YAML::Node &node)
{
    if (!node.IsSequence() || node.size() < 1)
        return YAML::Node();
    return YAML::Clone(node[node.size() - 1]);
}

YmlSceneManager::YmlSceneManager(QObject *parent, QGraphicsScene* gScene) : QObject(parent)
{
	pScene = gScene;
    qDebug() << "YmlSceneManager()";

    /* Init SceneGlobals */
    SG.init();
    //SG.getID(SACTORS, "PAUSE");
    //SG.getID(SACTORS, "CHOICE");

	/* read vanilla lines info */
    QFile w3LinesFile(":/lines.en.csv");
    if (w3LinesFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
       QTextStream in(&w3LinesFile);
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
       w3LinesFile.close();
	   qInfo() << "Loaded {" << lineById.size() << "} dg lines of total {" << cnt << "}.";
	} else {
		qCritical() << "Failed to read lines.en.csv!";
	}
}

void YmlSceneManager::showError(QString text, QString title, QMessageBox::Icon icon) {
	qc << title << ": " << text;
	emit error(text);
	QMessageBox box(icon, title, text, QMessageBox::Ok);
	box.setModal(true);
	box.exec();
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
void YmlSceneManager::warning(QString s) {
	qw << "Warning: " << s;
	emit print_warning(s);
}
void YmlSceneManager::info(QString s) {
	qi << "Info: " << s;
	emit print_info(s);
}

void YmlSceneManager::clearData(bool clearRepo) {
	// clear old (sections graph)
	sectionGraph.clear();
	startSections.clear();
	wasDrawn.clear();
	itemBySectionName.clear();
	sectionsByDepth.clear();
	for (auto item : unusedItems) {
		delete item;
	}
	unusedItems.clear();

	// clear old (dialogscript + storyboard shots)
    m_dialogLinkBySectionName.clear();
	dgActors.clear();
	dgProps.clear();

	// clear old (repository + production, except stuff from repo.scenes if clearRepo = false)
	SG.placementTag = "NO TAG";
	SG.sceneid = 1;
	SG.idspace = "9999";
	SG.idstart = 0;
	SG.gameplay = false;
	SG.cinematic_subtitles = false;

    removeAssetsFromSG(SG.actors, clearRepo);
    removeAssetsFromSG(SG.props, clearRepo);
    removeAssetsFromSG(SG.cameras, clearRepo);
    removeAssetsFromSG(SG.anims, clearRepo);
    removeAssetsFromSG(SG.mimics, clearRepo);
    removeAssetsFromSG(SG.poses, clearRepo);
    removeAssetsFromSG(SG.mimic_poses, clearRepo);

	SG.defaultPose.clear();
	SG.defaultMimic.clear();
	SG.defaultPlacement.clear();
	if (clearRepo) {
		SG.removeType(SSOUNDBANKS);
		SG.removeType("availableSoundEvents");
	}
}
bool YmlSceneManager::loadYmlRepo(QString path) {
	readingYmlRepo = true;
	// load yml
	try {
		root.reset();
		root = YAML::LoadFile(path.toStdString());
		info("Yml loaded: OK! " + path);
	} catch (const YAML::BadFile& e) {
		showError(to_qstr("Error: (BadFile) ") + e.what());
		return false;
	} catch (const YAML::ParserException& e) {
		showError(to_qstr("Error: (ParserException) ") + e.what());
		return false;
	}

	if (!loadSceneRepository()) {
		readingYmlRepo = false;
		return false;
	}

	if (!loadSceneProduction()) {
		readingYmlRepo = false;
		return false;
	}

	readingYmlRepo = false;
	return true;
}
bool YmlSceneManager::loadYmlFile(QString path) {
    filePath = path;
	qi << "Parsing " + path + "...";

    // load yml
    try {
        root.reset();
        root = YAML::LoadFile(path.toStdString());
		info("Yml loaded: OK! " + path);
    } catch (const YAML::BadFile& e) {
		showError(to_qstr("Error: (BadFile) ") + e.what());
        return false;
    } catch (const YAML::ParserException& e) {
		showError(to_qstr("Error: (ParserException)") + e.what());
        return false;
    }

	clearData(false);

    // load extra info
    if (!loadSectionsInfo()) {
        return false;
    }
	
	if (!loadSceneRepository()) {
		return false;
	}

	if (!loadSceneProduction()) {
		return false;
	}
	cleanupTempRepository();

	if (!loadSceneDefaults()) {
		return false;
	}

	if (!loadShotsInfo()) {
		return false;
	}

	hasChanges = false;
    emit ymlFileLoaded(path);
    return true;
}
bool YmlSceneManager::saveYmlFile() {
    QFile ymlFile(filePath);

    if (ymlFile.exists())
        ymlFile.copy(filePath + ".backup");
    else
        return false;
    ymlFile.close();

    if ( m_sectionNames.empty() ) {
		QMessageBox msgBox;
		msgBox.setText("Save is not allowed: no sections found!");
		msgBox.setInformativeText("Scene should contain at least start and exit sections.");
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		int ret = msgBox.exec();

		return false;
	}
    for (auto name : m_sectionNames) {
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
            QString sectionName = it->XKey.as<QString>().toLower(); // make all names toLower!
			qDebug() << "sectionName: " << sectionName;

               // section
            if ( sectionName.startsWith("section_") || sectionName.startsWith("script_") ) {
                if (sectionGraph.contains(sectionName)) {
					showError("Error: duplicated section name: " + sectionName);
                    return false;
                }

                // analyze for linked sections
                YAML::Node sectionNode(it->YValue);
                if (!sectionNode.IsSequence()) {
					showError("Error: section [" + sectionName + "] not a sequence");
                    return false;
                }

                  // CHECK: only last element?
                YAML::Node lastSectionNode(sectionNode[sectionNode.size() - 1]);
                sectionLink* tmpLink = new sectionLink;
                tmpLink->sectionName = sectionName;

                   // NEXT, (RANDOM, CHOICE)
                if (lastSectionNode.IsMap()) {
                    QString lastSectionCue = lastSectionNode.begin()->XKey.as<QString>();
					if (lastSectionCue.toUpper() == "NEXT") {
                            // simple NEXT
                        if (lastSectionNode.begin()->YValue.IsScalar()) {
                            tmpLink->addChoice( lastSectionNode.begin()->YValue.as<QString>() );
							tmpLink->type = nextS;
                            // advanced NEXT with condition
                        } else if (lastSectionNode.begin()->YValue.IsMap()
                                   && lastSectionNode.begin()->YValue["condition"]
                                   && lastSectionNode.begin()->YValue["condition"].IsSequence()
                                   && lastSectionNode.begin()->YValue["condition"].size() == 3
                                   && lastSectionNode.begin()->YValue["on_true"]
                                   && lastSectionNode.begin()->YValue["on_false"]) {
							tmpLink->type = conditionS;
                            tmpLink->addChoice( lastSectionNode.begin()->YValue["on_true"].as<QString>(), QString(), choiceAction(), lastSectionNode.begin()->YValue["condition"].as<ymlCond>() );
                            tmpLink->addChoice( lastSectionNode.begin()->YValue["on_false"].as<QString>() );
                        } else {
							showError("Error: [" + sectionName + "]: incorrect last element");
                            return false;
                        }
					} else if (lastSectionCue.toUpper() == "CHOICE"
                               && lastSectionNode.begin()->YValue.IsSequence()) {
						tmpLink->type = choiceS;
						qDebug() << "CHoice!";

                        // iterate through all choices
                        for (auto jt = lastSectionNode.begin()->YValue.begin(); jt != lastSectionNode.begin()->YValue.end(); ++jt) {
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
								showError("Error: [" + sectionName + "]: incorrect choices syntax");
                            }
                            /*if (!jt->YY["choice"] || jt->YY["choice"].size() < 2) {
                                emit debugInfo("Error: [" + sectionName + "]: incorrect choice element");
                                return;
                            }*/
                            //tmpLink.choiceLines.push_back(jt["choice"][0].as<QString>());
                        }
					} else if (lastSectionCue.toUpper() == "RANDOM"
                               && lastSectionNode.begin()->YValue.IsSequence()) {
						tmpLink->type = randomS;

                        // iterate through all randoms
                        for (auto jt = lastSectionNode.begin()->YValue.begin(); jt != lastSectionNode.begin()->YValue.end(); ++jt) {
                            YAML::Node tmpNode = *jt;
                            if (tmpNode.IsScalar()) {
                                tmpLink->addChoice( tmpNode.as<QString>() );
                            } else {
								showError("Error: [" + sectionName + "]: incorrect RANDOM syntax");
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
							showError("Error: [" + sectionName + "]: incorrect randomizer syntax");
                            return false;
                        }
                    }
                } else {
                    showError("Error: [" + sectionName + "]: incorrect section syntax.");
                    return false;
                }

                if ( sectionName.startsWith("script_") ) {
					tmpLink->type = scriptS;
                }

                if ( sectionName.startsWith("section_start") ) {
                    startSections.push_back( sectionName );
                }

                // all is fine, add to graph
                m_sectionNames.append(sectionName);
                sectionGraph.insert(sectionName, tmpLink);
				qd << "ADD: " << sectionName;
			} else if ( sectionName == "actors" ) {
                YAML::Node actorsNode(it->YValue);
				if (!actorsNode.IsSequence()) {
					showError("Error: actors list [" + sectionName + "] not a sequence");
					return false;
				}
				for (size_t j = 0; j < actorsNode.size(); ++j) {
					dgActors.insert( actorsNode[j].as<QString>() );
				}
			} else if ( sectionName == "props" ) {
                YAML::Node propsNode(it->YValue);
				if (!propsNode.IsSequence()) {
					showError("Error: actors list [" + sectionName + "] not a sequence");
					return false;
				}
				for (size_t j = 0; j < propsNode.size(); ++j) {
					dgProps.insert( propsNode[j].as<QString>() );
				}
			}
        }
    } else {
		showError("Error: no dialogscript sections found!");
        return false;
    }

	info("Loaded " + qn(sectionGraph.size()) + " sections");
    return true;
}

bool YmlSceneManager::loadShotActions(const YAML::Node actsNode, shot& sh) {
	upn(k, 0, (int) actsNode.size() - 1) {
		YAML::Node actionNode = actsNode[k];
        shotAction newAction;
        QString actionName = actionNode.begin()->XKey.as<QString>().toLower();
        newAction.actionType = stringToEShotAction.value(actionName, EShotUnknown);

        QStringList keys = actionName.split(".");
        YAML::Node paramNode = actionNode.begin()->YValue;
		bool isExtended = paramNode.IsMap();
		qDebug() << "** " << keys;

		if ( keys.isEmpty() ) {
			error("loadShotActions: Empty shot " + sh.shotName + " action keys! Skipped");
			continue;
		}
		if (keys[0] == "anim") {
			keys.insert(0, "actor"); // anim(.mimic) -> actor.anim(.mimic)
		}

        QString name; // cam: cam_name | actor.anim/actor.anim.mimic: actor_name |
		if ( keys[0] == "cam" ) {
            newAction.start = paramNode[0].as<double>();

			name = paramNode[1].as<QString>();
			if ( !SG.hasName(SCAMERAS, name) ) {
				error("loadShotActions: shot " + sh.shotName + ", camera " + name + " not found in repo!");
				continue;
			}
            newAction.values["cam_name"] = SG.getID(SCAMERAS, name);

			// rapid, smooth
			if ( keys.size() > 2 && (keys[2] == "start" || keys[2] == "end") && paramNode.size() > 2 ) {
                newAction.values["cam_ease"] = paramNode[2].as<QString>();
			}
		} else if ( keys[0] == "actor" || keys[0] == "prop" ) {
			if ( keys[1] == "anim" )
			{
				if (isExtended) {
                    newAction.start = paramNode[".@pos"][0].as<double>();
					name = paramNode[".@pos"][1].as<QString>();
					if (!SG.hasName(SANIMS, name)) {
						error("loadShotActions: shot " + sh.shotName + ", animation " + name + " not found in repo!");
						continue;
					}
                    newAction.values["animation"] = SG.getID(SANIMS, name);

					if (paramNode["actor"]) {
						name = paramNode["actor"].as<QString>();
                        if ( !SG.hasName(SASSETS, name) ) {
							error("loadShotActions: shot " + sh.shotName + ", actor " + name + " not found in repo!");
							continue;
						}
                        newAction.values["actor"] = SG.getID(SASSETS, name);
					}

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
					name = paramNode[1].as<QString>();
					if (!SG.hasName(SANIMS, name)) {
						error("loadShotActions: shot " + sh.shotName + ", animation " + name + " not found in repo!");
						continue;
					}
                    newAction.values["animation"] = SG.getID(SANIMS, name);
				}
			}
			else if ( keys[1] == "mimic" )
			{
				if (isExtended) {
                    newAction.start = paramNode[".@pos"][0].as<double>();
					name = paramNode[".@pos"][1].as<QString>();
					if (!SG.hasName(SMIMICPOSES, name)) {
						error("loadShotActions: shot " + sh.shotName + ", mimic " + name + " not found in repo!");
						continue;
					}
                    newAction.values["mimic"] = SG.getID(SMIMICPOSES, name);

					if (paramNode["actor"]) {
						name = paramNode["actor"].as<QString>();
                        if (!SG.hasName(SASSETS, name)) {
							error("loadShotActions: shot " + sh.shotName + ", actor " + name + " not found in repo!");
							continue;
						}
                        newAction.values["actor"] = SG.getID(SASSETS, name);
					}

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
					name = paramNode[1].as<QString>();
					if (!SG.hasName(SMIMICPOSES, name)) {
						error("loadShotActions: shot " + sh.shotName + ", mimic " + name + " not found in repo!");
						continue;
					}
                    newAction.values["mimic"] = SG.getID(SMIMICPOSES, name);
				}
			}
			else if ( keys[1] == "gamestate" )
			{
				if (isExtended) {
                    newAction.start = paramNode[".@pos"][0].as<double>();
					name = paramNode[".@pos"][1].as<QString>();
                    if (!SG.hasName(SASSETS, name)) {
						error("loadShotActions: shot " + sh.shotName + ", actor " + name + " not found in repo!");
						continue;
					}
                    newAction.values["actor"] = SG.getID(SASSETS, name);

					if (paramNode["action"])
                        newAction.values["action"] = paramNode["action"].as<QString>();
					if (paramNode["behavior"])
                        newAction.values["behavior"] = paramNode["behavior"].as<QString>();
				} else {
                    newAction.start = paramNode[0].as<double>();
					name = paramNode[1].as<QString>();
                    if (!SG.hasName(SASSETS, name)) {
						error("loadShotActions: shot " + sh.shotName + ", actor " + name + " not found in repo!");
						continue;
					}
                    newAction.values["actor"] = SG.getID(SASSETS, name);
				}
			}
			else if ( keys[1] == "placement" )
			{
                newAction.start = paramNode[0].as<double>();
				name = paramNode[1].as<QString>();
                if (!SG.hasName(SASSETS, name)) {
					error("loadShotActions: shot " + sh.shotName + ", actor " + name + " not found in repo!");
					continue;
				}
                newAction.values["actor"] = SG.getID(SASSETS, name);
                newAction.values["pos"] = paramNode[2].as<QVector3D>();
                newAction.values["rot"] = paramNode[3].as<QVector3D>();

				if ( keys.size() > 2 && (keys[2] == "start" || keys[2] == "end") && paramNode.size() > 4 ) {
                    newAction.values["ease"] = paramNode[4].as<QString>();
				}
			}
			else if ( keys[1] == "pose" )
			{
                newAction.start = paramNode[0].as<double>();
				name = paramNode[1].as<QString>();
				if (!SG.hasName(SANIMPOSES, name)) {
					error("loadShotActions: shot " + sh.shotName + ", actor.pose " + name + " not found in repo!");
					continue;
				}
                newAction.values["pose"] = SG.getID(SANIMPOSES, name);
			}
			else if ( keys[1] == "lookat" )
			{
				if (isExtended) {
                    newAction.start = paramNode[".@pos"][0].as<double>();
                    name = paramNode[".@pos"][1].as<QString>();
                    if (!SG.hasName(SASSETS, name)) {
                        error("loadShotActions: shot " + sh.shotName + ", actor " + name + " not found in repo!");
                        continue;
                    }
                    newAction.values["actor"] = SG.getID(SASSETS, name);
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
					name = paramNode[1].as<QString>();
                    if (!SG.hasName(SASSETS, name)) {
						error("loadShotActions: shot " + sh.shotName + ", actor " + name + " not found in repo!");
						continue;
					}
                    newAction.values["actor"] = SG.getID(SASSETS, name);
					if (paramNode[2].IsSequence()) {
                        newAction.values["lookat_pos"] = paramNode[2].as<QVector3D>();
					} else {
						name = paramNode[2].as<QString>();
                        if (!SG.hasName(SASSETS, name)) {
							error("loadShotActions: shot " + sh.shotName + ", actor " + name + " not found in repo!");
							continue;
						}
                        newAction.values["lookat_actor"] = SG.getID(SASSETS, name);
					}
				}
			}
            else if ( keys[1] == "show" || keys[1] == "hide" || keys[1] == "scabbard" || keys[1] == "unequip" )
			{
                newAction.start = paramNode[0].as<double>();
				name = paramNode[1].as<QString>();
                if (!SG.hasName(SASSETS, name)) {
					error("loadShotActions: shot " + sh.shotName + ", actor " + name + " not found in repo!");
					continue;
				}
                newAction.values["actor"] = SG.getID(SASSETS, name);
			}
			else if ( keys[1] == "effect" )
			{
                newAction.start = paramNode[0].as<double>();
				name = paramNode[1].as<QString>();
                if (!SG.hasName(SASSETS, name)) {
					error("loadShotActions: shot " + sh.shotName + ", actor " + name + " not found in repo!");
					continue;
				}
                newAction.values["actor"] = SG.getID(SASSETS, name);
                newAction.values["effect"] = paramNode[2].as<QString>();
			}
			else if ( keys[1] == "sound" )
			{
                newAction.start = paramNode[0].as<double>();
				name = paramNode[1].as<QString>();
                if (!SG.hasName(SASSETS, name)) {
					error("loadShotActions: shot " + sh.shotName + ", actor " + name + " not found in repo!");
					continue;
				}
                newAction.values["actor"] = SG.getID(SASSETS, name);
                newAction.values["effect"] = paramNode[2].as<QString>();
			}
			else if ( keys[1] == "appearance" )
			{
                newAction.start = paramNode[0].as<double>();
				name = paramNode[1].as<QString>();
                if (!SG.hasName(SASSETS, name)) {
					error("loadShotActions: shot " + sh.shotName + ", actor " + name + " not found in repo!");
					continue;
				}
                newAction.values["actor"] = SG.getID(SASSETS, name);
                newAction.values["appearance"] = paramNode[2].as<QString>();
			}
			else if ( keys[1] == "equip" )
			{
                newAction.start = paramNode[0].as<double>();
				name = paramNode[1].as<QString>();
                if (!SG.hasName(SASSETS, name)) {
					error("loadShotActions: shot " + sh.shotName + ", actor " + name + " not found in repo!");
					continue;
				}
                newAction.values["actor"] = SG.getID(SASSETS, name);
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
                newAction.values["weatherName"] = paramNode[1].as<QString>();

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
            /* handles both blendin/blendout ! */
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
            /* handles both blendin/blendout ! */
            newAction.start = paramNode[0].as<double>();
            newAction.values["duration"] = paramNode[1].as<double>();

			if (paramNode.size() > 2) {
                newAction.values["color"] = paramNode[2].as<QColor>();
			}
		} else {
			error("Unknown shot action!!! " + keys[0]);
			continue;
		}

		// load
        sh.actions.pb(newAction);
	}
	return true;
}

bool YmlSceneManager::loadSceneRepository() {
	if (!root["repository"])	{
		warning("loadSceneRepository()>: No repository found!");
		return true;
	}

	/* LOAD ACTORS FROM REPO */
	if (root["repository"]["actors"]) {
		qd << "loadSceneRepository()>: Loading " << root["repository"]["actors"].size() << " actors..";
		int cnt_new = 0;

		for (auto it = root["repository"]["actors"].begin(); it != root["repository"]["actors"].end(); ++it) {
            QString mapName = it->XKey.as<QString>() + SREPO_TEMP;
            if ( SG.hasName(SASSETS, mapName) )
			{
				qw << ("loadSceneRepository()>: actor " + mapName + " already exist! Overwriting.");
				//continue;
			}
            int nameID = SG.getID(SASSETS, mapName);

            if ( !it->YValue["template"] )
			{
				warning("loadSceneRepository():Repository: actor " + mapName + " doesn't have template definition!");
				continue;
			}
            asset actor(nameID, it->YValue["template"].as<QString>());

            if ( it->YValue["appearance"] ) {
                actor.mainAppearance = it->YValue["appearance"][0].as<QString>();
                upn(j, 0, it->YValue["appearance"].size() - 1) {
                    actor.appearances.insert( it->YValue["appearance"][j].as<QString>() );
				}
			}

			if (readingYmlRepo)
				actor.fromRepo = true;
			SG.actors[nameID] = actor;
			++cnt_new;
		}

		info("loadSceneRepository()>: Loaded " + qn(cnt_new) + " actors.");
	} else {
		//warning("loadSceneRepository()>: No actors found!");
		qw << "loadSceneRepository()>: No actors found!";
	}

	/* LOAD PROPS FROM REPO */
	if (root["repository"]["props"]) {
		qd << "loadSceneRepository()>: Loading " << root["repository"]["props"].size() << " props..";
		int cnt_new = 0;

		for (auto it = root["repository"]["props"].begin(); it != root["repository"]["props"].end(); ++it) {
            QString mapName = it->XKey.as<QString>() + SREPO_TEMP;
            if ( SG.hasName(SASSETS, mapName) )
			{
				qw << ("loadSceneRepository()>: prop " + mapName + " already exist! Overwriting.");
				//continue;
			}
            int nameID = SG.getID(SASSETS, mapName);

            if ( !it->YValue["template"] )
			{
				warning("loadSceneRepository():Repository: prop " + mapName + " doesn't have template definition!");
				continue;
			}
            asset prop(nameID, it->YValue["template"].as<QString>());
            prop.is_prop = true;

			if (readingYmlRepo)
				prop.fromRepo = true;
			SG.props[nameID] = prop;
			++cnt_new;
		}
		info("loadSceneRepository()>: Loaded " + qn(cnt_new) + " props.");
	} else {
		//warning("loadSceneRepository()>: No props found!");
		qw << "loadSceneRepository()>: No props found!";
	}

	/* LOAD CAMERAS FROM REPO */
	if (root["repository"]["cameras"]) {
		qd << "loadSceneRepository()>: Loading " << root["repository"]["cameras"].size() << " cameras..";
		int cnt_new = 0;

		for (auto it = root["repository"]["cameras"].begin(); it != root["repository"]["cameras"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();
			if ( SG.hasName(SCAMERAS, mapName) )
			{
				qw << ("loadSceneRepository()>: camera " + mapName + " already exist! Overwriting.");
				//continue;
			}
			int nameID = SG.getID(SCAMERAS, mapName);

            if ( !it->YValue["fov"] || !it->YValue["transform"]["pos"] || !it->YValue["transform"]["rot"] ) {
				warning("loadSceneRepository()>: Can't find fov/transform for cameras: " + mapName);
				continue;
			}
            camera cam(nameID, it->YValue["fov"].as<float>(), transform(it->YValue["transform"]["pos"].as<QVector3D>(), it->YValue["transform"]["rot"].as<QVector3D>()));

            if ( it->YValue["zoom"] )
                cam.zoom = it->YValue["zoom"].as<float>();
            if ( it->YValue["dof"] ) {
                YAML::Node dofNode = it->YValue["dof"];
				if ( dofNode["aperture"] )
					cam.dofAperture = { dofNode["aperture"][0].as<float>(),
										dofNode["aperture"][1].as<float>() };
				if ( dofNode["blur"] )
					cam.dofBlur = { dofNode["blur"][0].as<float>(),
										dofNode["blur"][1].as<float>() };
				if ( dofNode["focus"] )
					cam.dofFocus = { dofNode["focus"][0].as<float>(),
										dofNode["focus"][1].as<float>() };
				if ( dofNode["intensity"] )
					cam.dofIntensity = dofNode["intensity"].as<float>();
			}
            if ( it->YValue["event_generator"]["plane"] )
                cam.plane = it->YValue["event_generator"]["plane"].as<QString>();
            if ( it->YValue["event_generator"]["tags"] ) {
                upn(j, 0, it->YValue["event_generator"]["tags"].size() - 1) {
                    cam.tags.insert( it->YValue["event_generator"]["tags"][j].as<QString>() );
				}
			}

			if (readingYmlRepo)
				cam.fromRepo = true;
			SG.cameras[nameID] = cam;
			++cnt_new;
		}

		info("loadSceneRepository()>: Loaded " + qn(cnt_new) + " cameras.");
	} else {
		//warning("loadSceneRepository()>: No cameras found!");
		qw << "loadSceneRepository()>: No cameras found!";
	}

	/* LOAD ANIMS FROM REPO */
	if (root["repository"]["animations"]) {
		qd << "loadSceneRepository()>: Loading " << root["repository"]["animations"].size() << " animations..";
		int cnt_new = 0;

		for (auto it = root["repository"]["animations"].begin(); it != root["repository"]["animations"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();
			if ( SG.hasName(SANIMS, mapName) )
			{
				qw << ("loadSceneRepository()>: anim " + mapName + " already exist! Overwriting.");
				//continue;
			}
			int nameID = SG.getID(SANIMS, mapName);

            if ( !it->YValue["animation"] || !it->YValue["frames"] ) {
				warning("loadSceneRepository()>: Can't find animation/frames for anim " + mapName);
				continue;
			}
            animation anim(nameID, it->YValue["animation"].as<QString>(), it->YValue["frames"].as<int>());

            if ( it->YValue["blendin"] )
                anim.blendin = it->YValue["blendin"].as<float>();
            if ( it->YValue["blendout"] )
                anim.blendin = it->YValue["blendout"].as<float>();
            if ( it->YValue["weight"] )
                anim.blendin = it->YValue["weight"].as<float>();
            if ( it->YValue["stretch"] )
                anim.blendin = it->YValue["stretch"].as<float>();
            if ( it->YValue["clipfront"] )
                anim.blendin = it->YValue["clipfront"].as<float>();
            if ( it->YValue["clipend"] )
                anim.blendin = it->YValue["clipend"].as<float>();

			if (readingYmlRepo)
				anim.fromRepo = true;
			SG.anims[nameID] = anim;
			++cnt_new;
		}

		info("loadSceneRepository()>: Loaded " + qn(cnt_new) + " anims.");
	} else {
		//warning("loadSceneRepository()>: No animations found!");
		qw << "loadSceneRepository()>: No animations found!";
	}

	/* LOAD MIMIC ANIMS FROM REPO */
	if (root["repository"]["animations.mimic"]) {
		qd << "loadSceneRepository()>: Loading " << root["repository"]["animations.mimic"].size() << " mimic anims..";
		int cnt_new = 0;

		for (auto it = root["repository"]["animations.mimic"].begin(); it != root["repository"]["animations.mimic"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();
			if ( SG.hasName(SMIMICANIMS, mapName) )
			{
                qw << ("loadSceneRepository()>: mimic anim " + mapName + " already exist! Overwriting.");
				//continue;
			}
            int nameID = SG.getID(SMIMICANIMS, mapName);

            if ( !it->YValue["animation"] || !it->YValue["frames"] ) {
				warning("loadSceneRepository()>: Can't find animation/frames for mimic anim: " + mapName);
				continue;
			}
            animation mimicAnim(nameID, it->YValue["animation"].as<QString>(), it->YValue["frames"].as<int>());

            if ( it->YValue["blendin"] )
                mimicAnim.blendin = it->YValue["blendin"].as<float>();
            if ( it->YValue["blendout"] )
                mimicAnim.blendin = it->YValue["blendout"].as<float>();
            if ( it->YValue["weight"] )
                mimicAnim.blendin = it->YValue["weight"].as<float>();
            if ( it->YValue["stretch"] )
                mimicAnim.blendin = it->YValue["stretch"].as<float>();
            if ( it->YValue["clipfront"] )
                mimicAnim.blendin = it->YValue["clipfront"].as<float>();
            if ( it->YValue["clipend"] )
                mimicAnim.blendin = it->YValue["clipend"].as<float>();

			if (readingYmlRepo)
				mimicAnim.fromRepo = true;
			SG.mimics[nameID] = mimicAnim;
			++cnt_new;
		}

		info("loadSceneRepository()>: Loaded " + qn(cnt_new) + " mimic poses.");
	} else {
		//warning("loadSceneRepository()>: No mimic anims found!");
		qw << "loadSceneRepository()>: No mimic anims found!";
	}

	/* LOAD POSES FROM REPO */
	if (root["repository"]["actor.poses"]) {
		qd << "loadSceneRepository()>: Loading " << root["repository"]["actor.poses"].size() << " poses..";
		int cnt_new = 0;

		for (auto it = root["repository"]["actor.poses"].begin(); it != root["repository"]["actor.poses"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();
			if ( SG.hasName(SANIMPOSES, mapName) )
			{
				qw << ("loadSceneRepository()>: pose " + mapName + " already exist! Overwriting.");
				//continue;
			}
			int nameID = SG.getID(SANIMPOSES, mapName);

            if ( !it->YValue["idle_anim"] ) {
				warning("loadSceneRepository()>: Can't find idle_anim for actor.pose: " + mapName);
				continue;
			}
            anim_pose actorPose(nameID, it->YValue["idle_anim"].as<QString>());

            if ( it->YValue["name"] )
                actorPose.nameParam = it->YValue["name"].as<QString>();
            if ( it->YValue["emotional_state"] )
                actorPose.emotional_state = it->YValue["emotional_state"].as<QString>();
            if ( it->YValue["status"] )
                actorPose.status = it->YValue["status"].as<QString>();

			if (readingYmlRepo)
				actorPose.fromRepo = true;
			SG.poses[nameID] = actorPose;
			++cnt_new;
		}

		info("loadSceneRepository()>: Loaded " + qn(cnt_new) + " actor.poses.");
	} else {
		//warning("loadSceneRepository()>: No actor.poses found!");
		qw << "loadSceneRepository()>: No actor.poses found!";
	}

	/* LOAD MIMICS (MIMIC POSES) FROM REPO */
	if (root["repository"]["mimics"]) {
		qd << "loadSceneRepository()>: Loading " << root["repository"]["mimics"].size() << " mimics..";
		int cnt_new = 0;

		for (auto it = root["repository"]["mimics"].begin(); it != root["repository"]["mimics"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();
			if ( SG.hasName(SMIMICPOSES, mapName) )
			{
                qw << ("loadSceneRepository(): mimic pose " + mapName + " already exist! Overwriting.");
				//continue;
			}
			int nameID = SG.getID(SMIMICPOSES, mapName);

            if ( !it->YValue["pose"] ) {
				warning("loadSceneRepository()>: Can't find anim for mimic pose: " + mapName);
				continue;
			}
            mimic_pose mimic(nameID, it->YValue["pose"].as<QString>());

            if ( it->YValue["emotional_state"] )
                mimic.emotional_state = it->YValue["emotional_state"].as<QString>();
            if ( it->YValue["eyes"] )
                mimic.emotional_state = it->YValue["eyes"].as<QString>();
            if ( it->YValue["anim"] )
                mimic.emotional_state = it->YValue["anim"].as<QString>();
            if ( it->YValue["weight"] )
                mimic.weight = it->YValue["weight"].as<float>();
            if ( it->YValue["duration"] )
                mimic.duration = it->YValue["duration"].as<float>();

			if (readingYmlRepo)
				mimic.fromRepo = true;
			SG.mimic_poses[nameID] = mimic;
			++cnt_new;
		}

		info("loadSceneRepository()>: Loaded " + qn(cnt_new) + " mimic poses.");
	} else {
		//warning("loadSceneRepository()>: No mimic_poses found!");
		qw << "loadSceneRepository()>: No mimic_poses found!";
	}

	/* LOAD SOUNDBANKS EVENTS FROM REPO */
	if (root["repository"]["soundbanks"]) {
		qd << "loadSceneRepository()>: Loading " << root["repository"]["soundbanks"].size() << " soundbanks..";
		int cnt_new = 0;

		for (auto it = root["repository"]["soundbanks"].begin(); it != root["repository"]["soundbanks"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();
			if ( SG.hasName(SSOUNDBANKS, mapName) )
			{
				qw << ("loadSceneRepository(): soundbank " + mapName + " already exists!");
				//continue;
			}
			int nameID = SG.getID(SSOUNDBANKS, mapName);

            if ( !it->YValue.IsSequence() ) {
				warning("loadSceneRepository()>: soundbank " + mapName + ": expected sound events sequence!");
				continue;
			}
			QSet<QString> soundbankSet;
            upn(j, 0, it->YValue.size() - 1) {
                soundbankSet.insert( it->YValue[j].as<QString>() );
			}
			qd << "SOUNBANK ADD: " << mapName << " [" << qn(soundbankSet.size()) << "]";

			SG.soundbanks[nameID] = soundbankSet;
			++cnt_new;
		}

		info("loadSceneRepository()>: Loaded " + qn(cnt_new) + " soundbanks.");
	} else {
		//warning("loadSceneRepository()>: No soundbanks found!");
		qw << "loadSceneRepository()>: No soundbanks found!";
	}

	return true;
}

void YmlSceneManager::cleanupTempRepository() {
    QVector<int> actorIDs = SG.actors.keys().toVector();
    dn(i, actorIDs.count() - 1, 0) {
        int nameID = actorIDs[i];
        QString name = SG.getName(nameID);
        qd << "cleanupTempRepository(): " << name;
        if ( name.endsWith(SREPO_TEMP) ) {
            qd << "cleanupTempRepository(): CLEANED";
            SG.actors.remove(nameID);
            SG.removeName(SASSETS, name);
        }
    }

    QVector<int> propIDs = SG.props.keys().toVector();
    dn(i, propIDs.count() - 1, 0) {
        int nameID = propIDs[i];
        QString name = SG.getName(nameID);
        qd << "cleanupTempRepository(): " << name;
        if ( name.endsWith(SREPO_TEMP) ) {
            qd << "cleanupTempRepository(): CLEANED";
            SG.props.remove(nameID);
            SG.removeName(SASSETS, name);
        }
    }
}

bool YmlSceneManager::loadSceneProduction() {
	if (!root["production"]["assets"])	{
		warning("loadSceneProduction()>: No production assets!");
		return true;
	}

	/* LOAD ACTORS FROM PRODUCTION */
	if (root["production"]["assets"]["actors"]) {
		qd << "loadSceneProduction()>: Loading " << root["production"]["assets"]["actors"].size() << " actors..";
		//QSet<QString> dontRemoveKeys; // for renamed (mapName -> repoName)

		for (auto it = root["production"]["assets"]["actors"].begin(); it != root["production"]["assets"]["actors"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();

            if ( !it->YValue["repo"] ) {
				error("loadSceneProduction()>: No repo value found for actor " + mapName);
				continue;
			}
            QString repoName = it->YValue["repo"].as<QString>();
            if ( !SG.hasName(SASSETS, repoName + SREPO_TEMP) ) {
				error("loadSceneProduction()>: Actor " + repoName + " not found in repository!");
				continue;
			}
            int repoID = SG.getID(SASSETS, repoName + SREPO_TEMP);
            int prodID = SG.getID(SASSETS, mapName);

			asset prodActor = SG.actors[repoID];
			prodActor.nameID = prodID;

            if ( it->YValue["appearance"] ) {
                prodActor.mainAppearance = it->YValue["appearance"].as<QString>();
				if ( !prodActor.appearances.contains(prodActor.mainAppearance) ) {
					warning("loadSceneProduction()>: Appearance " + prodActor.mainAppearance + " was not defined in repository");
					prodActor.appearances.insert( prodActor.mainAppearance );
				}
			}
            if ( it->YValue["by_voicetag"] )
                prodActor.by_voicetag = it->YValue["by_voicetag"].as<bool>();
            if ( it->YValue["tags"]) {
                upn(i, 0, it->YValue["tags"].size() - 1) {
                    prodActor.tags.insert( it->YValue["tags"][i].as<QString>() );
				}
			}

			if (readingYmlRepo)
				prodActor.fromRepo = true;
			SG.actors[prodID] = prodActor;
			//dontRemoveKeys.insert(mapName);
		}
		/*for (auto it : SG.actors) {
			if ( !dontRemoveKeys.contains(it.name) ) {
				SG.actors.remove(it.name);
			}
		}*/
		/*for (auto it : SG.actors) {
			qd << "actor name: " << it.name;
		}*/
	}

	/* LOAD PROPS FROM PRODUCTION */
	if (root["production"]["assets"]["props"]) {
		qd << "loadSceneProduction()>: Loading " << root["production"]["assets"]["props"].size() << " props..";

		for (auto it = root["production"]["assets"]["props"].begin(); it != root["production"]["assets"]["props"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();

            if ( !it->YValue["repo"] ) {
				error("loadSceneProduction()>: No repo value found for prop " + mapName);
				continue;
			}
            QString repoName = it->YValue["repo"].as<QString>();
            if ( !SG.hasName(SASSETS, repoName + SREPO_TEMP) ) {
				error("loadSceneProduction()>: Prop " + repoName + " not found in repository!");
				continue;
			}

            int repoID = SG.getID(SASSETS, repoName + SREPO_TEMP);
            int prodID = SG.getID(SASSETS, mapName);

			asset prodProp = SG.props[repoID];
            prodProp.is_prop = true;
            prodProp.nameID = prodID;

			if (readingYmlRepo)
				prodProp.fromRepo = true;
			SG.props[prodID] = prodProp;
		}
	}

	/* LOAD CAMERAS FROM PRODUCTION */
	if (root["production"]["assets"]["cameras"]) {
		qd << "loadSceneProduction()>: Loading " << root["production"]["assets"]["cameras"].size() << " cameras..";
		//QSet<QString> dontRemoveKeys; // for renamed (mapName -> repoName)

		for (auto it = root["production"]["assets"]["cameras"].begin(); it != root["production"]["assets"]["cameras"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();

            if ( !it->YValue["repo"] ) {
				error("loadSceneProduction()>: No repo value found for camera " + mapName);
				continue;
			}
            QString repoName = it->YValue["repo"].as<QString>();
			if ( !SG.hasName(SCAMERAS, repoName) ) {
				error("loadSceneProduction()>: Camera " + repoName + " not found in repository!");
				continue;
			}

			int repoID = SG.getID(SCAMERAS, repoName);
			int prodID = SG.getID(SCAMERAS, mapName);

			camera prodCamera = SG.cameras[repoID];
			prodCamera.nameID = prodID;

			if (readingYmlRepo)
				prodCamera.fromRepo = true;
			SG.cameras[prodID] = prodCamera;
			//dontRemoveKeys.insert(mapName);
		}
		/*for (auto it : SG.cameras) {
			if ( !dontRemoveKeys.contains(it.name) ) {
				SG.cameras.remove(it.name);
			}
		}*/
	}

	/* LOAD ANIMS FROM PRODUCTION */
	if (root["production"]["assets"]["animations"]) {
		qd << "loadSceneProduction()>: Loading " << root["production"]["assets"]["animations"].size() << " animations..";

		for (auto it = root["production"]["assets"]["animations"].begin(); it != root["production"]["assets"]["animations"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();

            if ( !it->YValue["repo"] ) {
				error("loadSceneProduction()>: No repo value found for animation " + mapName);
				continue;
			}
            QString repoName = it->YValue["repo"].as<QString>();
			if ( !SG.hasName(SANIMS, repoName) ) {
				error("loadSceneProduction()>: Animation " + repoName + " not found in repository!");
				continue;
			}

			int repoID = SG.getID(SANIMS, repoName);
			int prodID = SG.getID(SANIMS, mapName);

			animation prodAnim = SG.anims[repoID];
			prodAnim.nameID = prodID;

            if ( it->YValue["actor"] ) {
                QString actorName = it->YValue["actor"].as<QString>();
                if (!SG.hasName(SASSETS, actorName)) {
					error("loadSceneProduction()>: Animation actor " + actorName + " not found in repository!");
				} else {
                    int actorID = SG.getID(SASSETS, actorName);
					prodAnim.actorID = actorID;
				}
			}
            if ( it->YValue["blendin"] )
                prodAnim.blendin = it->YValue["blendin"].as<float>();
            if ( it->YValue["blendout"] )
                prodAnim.blendout = it->YValue["blendout"].as<float>();
            if ( it->YValue["weight"] )
                prodAnim.weight = it->YValue["weight"].as<float>();
            if ( it->YValue["stretch"] )
                prodAnim.stretch = it->YValue["stretch"].as<float>();
            if ( it->YValue["clipfront"] )
                prodAnim.clipfront = it->YValue["clipfront"].as<float>();
            if ( it->YValue["clipend"] )
                prodAnim.clipend = it->YValue["clipend"].as<float>();

			if (readingYmlRepo)
				prodAnim.fromRepo = true;
			SG.anims[prodID] = prodAnim;
		}
	}

	/* LOAD MIMIC ANIMS FROM PRODUCTION */
	if (root["production"]["assets"]["animations.mimic"]) {
		qd << "loadSceneProduction()>: Loading " << root["production"]["assets"]["animations.mimic"].size() << " mimic anims..";

		for (auto it = root["production"]["assets"]["animations.mimic"].begin(); it != root["production"]["assets"]["animations.mimic"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();

            if ( !it->YValue["repo"] ) {
				error("loadSceneProduction()>: No repo value found for mimic animation " + mapName);
				continue;
			}
            QString repoName = it->YValue["repo"].as<QString>();
			if ( !SG.hasName(SMIMICANIMS, repoName) ) {
				error("loadSceneProduction()>: Mimic animation " + repoName + " not found in repository!");
				continue;
			}

			int repoID = SG.getID(SMIMICANIMS, repoName);
			int prodID = SG.getID(SMIMICANIMS, mapName);

			animation prodMimicAnim = SG.mimics[repoID];
			prodMimicAnim.nameID = prodID;

            if ( it->YValue["actor"] ) {
                QString actorName = it->YValue["actor"].as<QString>();
                if (!SG.hasName(SASSETS, actorName)) {
					error("loadSceneProduction()>: Animation Mimic actor " + actorName + " not found in repository!");
				} else {
                    int actorID = SG.getID(SASSETS, actorName);
					prodMimicAnim.actorID = actorID;
				}
			}
            if ( it->YValue["blendin"] )
                prodMimicAnim.blendin = it->YValue["blendin"].as<float>();
            if ( it->YValue["blendout"] )
                prodMimicAnim.blendout = it->YValue["blendout"].as<float>();
            if ( it->YValue["weight"] )
                prodMimicAnim.weight = it->YValue["weight"].as<float>();
            if ( it->YValue["stretch"] )
                prodMimicAnim.stretch = it->YValue["stretch"].as<float>();
            if ( it->YValue["clipfront"] )
                prodMimicAnim.clipfront = it->YValue["clipfront"].as<float>();
            if ( it->YValue["clipend"] )
                prodMimicAnim.clipend = it->YValue["clipend"].as<float>();

			if (readingYmlRepo)
				prodMimicAnim.fromRepo = true;
			SG.mimics[prodID] = prodMimicAnim;
		}
	}

	/* LOAD POSES FROM PRODUCTION */
	if (root["production"]["assets"]["actor.poses"]) {
		qd << "loadSceneProduction()>: Loading " << root["production"]["assets"]["actor.poses"].size() << " mimic anims..";

		for (auto it = root["production"]["assets"]["actor.poses"].begin(); it != root["production"]["assets"]["actor.poses"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();

            if ( !it->YValue["repo"] ) {
				error("loadSceneProduction()>: No repo value found for actor.pose " + mapName);
				continue;
			}
            QString repoName = it->YValue["repo"].as<QString>();
			if ( !SG.hasName(SANIMPOSES, repoName) ) {
				error("loadSceneProduction()>: Actor.pose " + repoName + " not found in repository!");
				continue;
			}

			int repoID = SG.getID(SANIMPOSES, repoName);
			int prodID = SG.getID(SANIMPOSES, mapName);

			anim_pose prodPose = SG.poses[repoID];
			prodPose.nameID = prodID;

            if ( it->YValue["actor"] ) {
                QString actorName = it->YValue["actor"].as<QString>();
                if (!SG.hasName(SASSETS, actorName)) {
					error("loadSceneProduction()>: Pose actor " + actorName + " not found in repository!");
				} else {
                    int actorID = SG.getID(SASSETS, actorName);
					prodPose.actorID = actorID;
				}
			}

			if (readingYmlRepo)
				prodPose.fromRepo = true;
			SG.poses[prodID] = prodPose;
		}
	}

	/* LOAD MIMICS (MIMIC POSES) FROM PRODUCTION */
	if (root["production"]["assets"]["mimics"]) {
		qd << "loadSceneProduction()>: Loading " << root["production"]["assets"]["mimics"].size() << " mimics (mimic poses)..";

		for (auto it = root["production"]["assets"]["mimics"].begin(); it != root["production"]["assets"]["mimics"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();

            if ( !it->YValue["repo"] ) {
				error("loadSceneProduction()>: No repo value found for mimics " + mapName);
				continue;
			}
            QString repoName = it->YValue["repo"].as<QString>();
			if ( !SG.hasName(SMIMICPOSES, repoName) ) {
				error("loadSceneProduction()>: Mimics (mimic pose) " + repoName + " not found in repository!");
				continue;
			}

			int repoID = SG.getID(SMIMICPOSES, repoName);
			int prodID = SG.getID(SMIMICPOSES, mapName);

			mimic_pose prodMimicPose = SG.mimic_poses[repoID];
			prodMimicPose.nameID = prodID;

            if ( it->YValue["actor"] ) {
                QString actorName = it->YValue["actor"].as<QString>();
                if (!SG.hasName(SASSETS, actorName)) {
					error("loadSceneProduction()>: Mimic Pose actor " + actorName + " not found in repository!");
				} else {
                    int actorID = SG.getID(SASSETS, actorName);
					prodMimicPose.actorID = actorID;
				}
			}
            if ( it->YValue["emotional_state"] )
                prodMimicPose.emotional_state = it->YValue["emotional_state"].as<QString>();
            if ( it->YValue["eyes"] )
                prodMimicPose.emotional_state = it->YValue["eyes"].as<QString>();
            if ( it->YValue["anim"] )
                prodMimicPose.emotional_state = it->YValue["anim"].as<QString>();
            if ( it->YValue["weight"] )
                prodMimicPose.weight = it->YValue["weight"].as<float>();
            if ( it->YValue["duration"] )
                prodMimicPose.duration = it->YValue["duration"].as<float>();

			if (readingYmlRepo)
				prodMimicPose.fromRepo = true;
			SG.mimic_poses[prodID] = prodMimicPose;
		}
	}

	/* LOAD SOUNDBANKS FROM PRODUCTION */
	if (root["production"]["assets"]["soundbanks"]) {
		qd << "loadSceneProduction()>: Loading " << root["production"]["assets"]["soundbanks"].size() << " soundbanks..";

		upn(j, 0, root["production"]["assets"]["soundbanks"].size() - 1) {
			QString bankName = root["production"]["assets"]["soundbanks"][j].as<QString>();
			if ( !SG.hasName(SSOUNDBANKS, bankName) ) {
				error("loadSceneProduction()>: Soundbank " + bankName + " not found in repository!");
				continue;
			}
			int bankID = SG.getID(SSOUNDBANKS, bankName);

			/*for (auto jt : SG.soundbanks[bankID]) {
				SG.addNameFor("availableSoundEvents", jt);
			}*/
		}
	}
	return true;
}

bool YmlSceneManager::loadSceneDefaults() {
	/* general settings */
	if (root["production"]["settings"]) {
		YAML::Node settingsNode = root["production"]["settings"];
		if (settingsNode["sceneid"])
			SG.sceneid = settingsNode["sceneid"].as<int>();
		if (settingsNode["strings-idstart"])
			SG.idstart = settingsNode["strings-idstart"].as<int>();
		if (settingsNode["strings-idspace"])
			SG.idspace = settingsNode["strings-idspace"].as<QString>();
	}
	if (root["production"]["placement"])
		SG.placementTag = root["production"]["placement"].as<QString>();
	if (root["production"]["gameplay"])
		SG.gameplay = root["production"]["gameplay"].as<bool>();
	if (root["production"]["cinematic_subtitles"])
		SG.cinematic_subtitles = root["production"]["cinematic_subtitles"].as<bool>();

	/* actors defaults */
	YAML::Node placeNode = root["storyboard"]["defaults"]["placement"];
	YAML::Node poseNode = root["storyboard"]["defaults"]["actor.pose"];
	for (auto actorID : SG.actors.keys()) {
		QString actorName = SG.getName(actorID);
		// placement
		if (placeNode[actorName] && placeNode[actorName].IsSequence() && placeNode[actorName].size() == 2) {
			SG.defaultPlacement[actorID] = transform(placeNode[actorName][0].as<QVector3D>(), placeNode[actorName][1].as<QVector3D>());
			qd << "loadSceneDefaults(): Set def placement for " << actorName << ": " << SG.defaultPlacement[actorID].pos;
		}
		// poses
		if (poseNode[actorName] && poseNode[actorName].IsScalar()) {
			QString poseName = poseNode[actorName].as<QString>();
			if ( !SG.hasName(SANIMPOSES, poseName) ) {
				error("loadSceneDefaults(): Default pose " + poseName + " was not defined in repo/production (actor: " + actorName + ")");
			} else {
				int poseID = SG.getID(SANIMPOSES, poseName);
				SG.defaultPose[actorID] = poseID;
				qd << "loadSceneDefaults(): Set def pose for " << actorName << ": " << poseName;
			}
		}
		// mimic (special case - defined in actor map in: (1) repo, (2) production)
		mimic_pose defMimic;
		// from repository (barely)
		YAML::Node mimicRNode = root["repository"]["actors"][actorName]["mimic"];
		if (mimicRNode && mimicRNode.IsMap()) {
			defMimic.nameID = SG.getID(SMIMICPOSES, actorName + "_default_mimic"); // not really needed
			if ( mimicRNode["emotional_state"] )
				defMimic.emotional_state = mimicRNode["emotional_state"].as<QString>();
			if ( mimicRNode["eyes"] )
				defMimic.emotional_state = mimicRNode["eyes"].as<QString>();
			if ( mimicRNode["anim"] )
				defMimic.emotional_state = mimicRNode["anim"].as<QString>();
			if ( mimicRNode["weight"] )
				defMimic.weight = mimicRNode["weight"].as<float>();
			if ( mimicRNode["duration"] )
				defMimic.duration = mimicRNode["duration"].as<float>();
		}
		// from production (probably)
		YAML::Node mimicPNode = root["production"]["assets"]["actors"][actorName]["mimic"];
		if (mimicPNode && mimicPNode.IsMap()) {
			defMimic.nameID = SG.getID(SMIMICPOSES, actorName + "_default_mimic"); // not really needed

			if ( mimicPNode["repo"] ) {
				QString mimicPNodeRepo = mimicPNode["repo"].as<QString>();
				if ( !SG.hasName(SMIMICPOSES, mimicPNodeRepo) ) {
					warning("loadSceneDefaults()>: Mimics (mimic pose) " + mimicPNodeRepo + " for actor " + actorName + " not found in repository!");
				} else {
					// load repo params first
					defMimic = SG.mimic_poses[ SG.getID(SMIMICPOSES, mimicPNodeRepo) ];
				}
			}
			if ( mimicPNode["emotional_state"] )
				defMimic.emotional_state = mimicPNode["emotional_state"].as<QString>();
			if ( mimicPNode["eyes"] )
				defMimic.emotional_state = mimicPNode["eyes"].as<QString>();
			if ( mimicPNode["anim"] )
				defMimic.emotional_state = mimicPNode["anim"].as<QString>();
			if ( mimicPNode["weight"] )
				defMimic.weight = mimicPNode["weight"].as<float>();
			if ( mimicPNode["duration"] )
				defMimic.duration = mimicPNode["duration"].as<float>();
		}

		// finally check if something was found
		if ( defMimic.nameID != -1 ) {
			defMimic.actorID = actorID;
			SG.defaultMimic[actorID] = defMimic;
			qd << "loadSceneDefaults(): Set def mimic for: " << actorName;
		}
	}

	/* props defaults */
	for (auto propID : SG.props.keys()) {
		// placement
		QString propName = SG.getName(propID);
		if (placeNode[propID] && placeNode[propName].IsSequence() && placeNode[propName].size() == 2) {
			SG.defaultPlacement[propID] = transform(placeNode[propName][0].as<QVector3D>(), placeNode[propName][1].as<QVector3D>());
			qd << "Set def placement for " << propName << ": " << SG.defaultPlacement[propID].pos;
		}
	}
	return true;
}

bool YmlSceneManager::loadShotsInfo() {
	QSet<QString> nextKeys, cueKeys;
	nextKeys = { "NEXT", "SCRIPT", "RANDOM", "EXIT", "OUTPUT", "BLACKSCREEN", "CAMERA_BLEND" };
	cueKeys = { "CUE", "HINT", "REFERENCE" };

    for (auto sectionName : m_sectionNames) {
		if (!root["dialogscript"][sectionName])	{
			qDebug() << "Exception0: no dialogscript found for section [" << sectionName << "]";
			continue;
		}

		YAML::Node sNode = root["dialogscript"][sectionName];

		dialogLink newDgLink;
		QString prevShotName = QString();
		qInfo() << "--- Loading dialog for section [" << sectionName << "]";

		for (auto it = sNode.begin(); it != sNode.end(); ++it) {			
			if ( !it->IsMap() ) {
				qWarning() << "Warning: non-map type (exit?).";
				break;
			}
			if ( it->size() > 1 ) {
                error("loadShotsInfo(): Map " + it->begin()->XKey.as<QString>() + " has > 1 elements?");
				break;
			}
            if ( !it->begin()->XKey.IsScalar() ) {
				qCritical() << "Exception3/1: non-scalar!";
				break;
			}
            QString key = it->begin()->XKey.as<QString>();

			if ( nextKeys.contains(key.toUpper()) ) {
				//qDebug() << "NEXT key, continue";
				prevShotName = QString();
				continue;
			}
			if ( cueKeys.contains(key.toUpper()) ) {
				//qDebug() << "CUE key";
                prevShotName = it->begin()->YValue.as<QString>();
				continue;

			} else if ( key.toUpper() == "PAUSE" || key.toUpper() == "CHOICE" ) {
				//qDebug() << "CHOICE/PAUSE key";
				newDgLink.lines.pb( key.toUpper() );
                int actorID = SG.getID(SASSETS, key.toUpper());
				newDgLink.speakers.pb( actorID );
				double dur = -1.0;
				if (key.toUpper() == "PAUSE") {
                    dur = it->begin()->YValue.as<double>();
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
                    while ( newDgLink.shotNames.contains( prevShotName + qn(idx) ) ) {
						++idx;
					}
					prevShotName += qn(idx);
				}
				newShot.shotName = prevShotName;
                newDgLink.shotNames.insert(prevShotName);
				newDgLink.shots.pb( newShot );

				prevShotName = QString();

			} else if ( dgActors.contains(key) ) {
                if ( !it->begin()->YValue.IsScalar() ) {
					qCritical() << "Exception3/2: non-scalar YY (actor)";
					break;
				}
				//qDebug() << "ACTOR key";
                QString line = it->begin()->YValue.as<QString>();
				newDgLink.lines.pb( line );
                if ( !SG.hasName(SASSETS, key) ) {
					error(QString(Q_FUNC_INFO) + ": section " + sectionName + ": actor " + key + " not found in repository!");
				}
                int actorID = SG.getID(SASSETS, key);
				warning("TEST actor: " + SG.getName(actorID));
				newDgLink.speakers.pb( actorID );
				newDgLink.durations.pb( getTextDuration(line) ); // TODO: try extract duration

				shot newShot;
				if ( prevShotName.isEmpty() ) {
					prevShotName = key;
					int idx = 1;
                    while ( newDgLink.shotNames.contains( prevShotName + qn(idx) ) ) {
						++idx;
					}
					prevShotName += qn(idx);
				}
				newShot.shotName = prevShotName;
                newDgLink.shotNames.insert(prevShotName);
				newDgLink.shots.pb( newShot );

				prevShotName = QString();

			} else {
				error("loadShotsInfo():> dialogscript: UNKNOWN KEY! " + key);
				prevShotName = QString();
				continue;
			}
		}
		/*upn(i, 0, newDgLink.lines.size() - 1) {
			qDebug() << "Speaker: [" << newDgLink.speakers[i] << "], line: [" << newDgLink.lines[i] << "], shot alias: ["
					 << newDgLink.shots[i].shotName << "], duration: [" << newDgLink.durations[i] << "]";
		}*/
		newDgLink.calculateTotalDuration();
        m_dialogLinkBySectionName[sectionName]= newDgLink;
	}

	if (root["storyboard"]) {
		for (auto it = root["storyboard"].begin(); it != root["storyboard"].end(); ++it) {
            QString sectionName = it->XKey.as<QString>();
			qInfo() << "loadShotsInfo()>: Loading storyboard for section: [" << sectionName << "]";

            if ( !it->YValue.IsMap() ) {
				qCritical() << "loadShotsInfo()>: Exception4: sbui section not a map!";
				continue;
			}
            if ( !m_sectionNames.contains(sectionName) ) {
				qInfo() << "loadShotsInfo()>: Skipping " + sectionName + ": not a real section (defaults?)";
				continue;
			}

            for (auto jt = it->YValue.begin(); jt != it->YValue.end(); ++jt) {
                QString shotName = jt->XKey.as<QString>();
				qInfo() << "loadShotsInfo()>: Loading shot: [" << shotName << "]";

                if ( !jt->YValue.IsSequence() ) {
					qCritical() << "loadShotsInfo()>: Exception5: shot actions are not a list!";
					continue;
				}
                int shotIdx = m_dialogLinkBySectionName[sectionName].shotNumByName(shotName);
				if (shotIdx == -1) {
					qCritical() << "loadShotsInfo()>: Exception6: shot was not found in dialogscript!";
					continue;
				}

                if ( !loadShotActions(jt->YValue, m_dialogLinkBySectionName[sectionName].shots[shotIdx]) ) {
					error("loadShotsInfo()>: error loading shot: " + shotName + " (section " + sectionName + ")");
					return false;
				}
			}
		}

	} else {
		qw << "loadShotsInfo()>: No sbui shots found!";
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
			qc << "FAILED to add new socket, incorrect yml!";
            return false;
        }
    }
    return true;
}

void YmlSceneManager::addSectionLink(QPointF pos) {
	QString sectionName = "section_new_";
	int j = 1;
    while ( m_sectionNames.contains(sectionName + qn(j)) ) {
		++j;
	}
	sectionName = sectionName + qn(j);
    m_sectionNames.append(sectionName);
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
        m_sectionNames.removeAll(oldName);
        m_sectionNames << sectionName;

		YAML::Node tempNode = root["dialogscript"];
		for (auto it = tempNode.begin(); it != tempNode.end(); ++it) {
			if (it->first.as<QString>() == oldName) {
				it->first = sectionName;
				break;
			}
		}
        emit sectionNameChanged(oldName, sectionName);
	}
}

void YmlSceneManager::updateSectionLink(QString sectionName) {
    sectionLink* pLink = sectionGraph[sectionName];

    if (root["dialogscript"][sectionName]) {
        YAML::Node sNode = root["dialogscript"][sectionName];
        YAML::Node sNode2;
        switch ( pLink->type ) {
			case exitS: {
				// simple EXIT scalar
				sNode2 = "EXIT";
				break;
			}
			case choiceS: {
				YAML::Node tempSeq;
				YAML::Node tempMap;
                for (int i = 0; i < pLink->names.size(); ++i) {
                    if ( pLink->names[i].isEmpty() )
						continue;
					tempMap.SetStyle(YAML::EmitterStyle::Block);
					tempMap["choice"].SetStyle(YAML::EmitterStyle::Flow);

					// dialog line
                    tempMap["choice"].push_back( pLink->choiceLines[i] );
					// next section name
                    tempMap["choice"].push_back( pLink->names[i] );
					// add action values
                    if ( !pLink->choiceActions[i].action.isEmpty() ) {
                        tempMap["choice"].push_back( pLink->choiceActions[i].action );
                        if ( pLink->choiceActions[i].amount != -1 ) {
                            tempMap["choice"].push_back( pLink->choiceActions[i].amount );
                            if ( pLink->choiceActions[i].grantExp ) {
								tempMap["choice"].push_back( true );
							}
						}
					}
					// add conition block
                    if ( !pLink->conditions[i].condFact.isEmpty() ) {
                        tempMap["condition"] = pLink->conditions[i];
						tempMap["condition"].SetStyle(YAML::EmitterStyle::Flow);
					}
					tempMap["choice"][0].SetTag("!"); // hack to add quotes
					// single_use block
                    if ( pLink->single_use[i] )
						tempMap["single_use"] = true;
					// emphasize block
                    if ( pLink->emphasize[i] )
						tempMap["emphasize"] = true;

					// push -choice to CHOICE block
					tempSeq.push_back(tempMap);
					tempMap.reset();
				}
				// add TIME_LIMIT if exists
                if ( pLink->timeLimit > 0.0 ) {
                    tempMap["TIME_LIMIT"] = pLink->timeLimit;
					tempSeq.push_back(tempMap);
				}
				sNode2["CHOICE"] = tempSeq;
				break;
			}
			case conditionS: {
				YAML::Node tempMap;
                tempMap["condition"] = pLink->conditions[0];
				tempMap["condition"].SetStyle(YAML::EmitterStyle::Flow);
                tempMap["on_true"] = pLink->names[0];
                tempMap["on_false"] = pLink->names[1];
				sNode2["NEXT"] = tempMap;
				break;
			}
			case randomS: {
				YAML::Node tempSeq;
                for (auto s : pLink->names) {
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
                sNode2["NEXT"] = pLink->names[0];
				break;
			}
		}

        // update last element in section node
        sNode[sNode.size() - 1] = sNode2;
    } else {
        error("ERROR: section [" + sectionName + "] not found for updateSection!");
    }
}

void YmlSceneManager::deleteSection(QString sectionName) {
    delete sectionGraph[sectionName];
    sectionGraph.remove( sectionName );
    m_sectionNames.removeAll( sectionName );

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
    emit sectionDeleted(sectionName);
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
    return m_sectionNames;
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

/* ASSET EDITOR */
void YmlSceneManager::removeActorAsset(int actorID) {
	QString actorName = SG.getName(actorID);
	SG.actors.remove(actorID);
	SG.removeID(actorID);
    // TODO!
}

/*
void YmlSceneManager::updateStoryboardSection(QString sectionName)
{
    if (root["storyboard"][sectionName]) {
        YAML::Node sectionMap = root["storyboard"][sectionName];
        sectionMap.reset();
        upn(i, 0, dgLinkBySectionName[sectionName].shots.count() - 1) {
            QString shotName = dgLinkBySectionName[sectionName].shots[i].shotName;
            if (dgLinkBySectionName[sectionName].shots[shotNum].actions.count() > 0) {
                sectionMap[shotName] = YAML::Node();
                updateShot(sectionName, shotName);
            }
        }
        shotSeq = newSeq;
    } else {
        error("ERROR: section [" + sectionName + "][" + shotName + "] not found for updateStoryboardSection!");
    }
}
*/

void YmlSceneManager::updateDialogscriptSection(QString sectionName)
{
    if (root["dialogscript"][sectionName]) {
        YAML::Node sectionSeq = YAML::Node(YAML::NodeType::Sequence);

        upn(i, 0, m_dialogLinkBySectionName[sectionName].lines.count() - 1) {
            dialogLink* pDialogLink = &m_dialogLinkBySectionName[sectionName];
            // handled as section link, shot cue is never defined directly (always called CHOICE_1)
            if (pDialogLink->lines[i] == "CHOICE") {
                break;
            // PAUSE/actor cases
           } else {
                QString actorName = SG.getName(pDialogLink->speakers[i]);
                // add CUE shot reference only if it is not empty
                if (pDialogLink->shots[i].actions.count() > 0) {
                    sectionSeq.push_back( singleMapNode("CUE", pDialogLink->shots[i].shotName) );
                }
                if (actorName == "PAUSE")
                    sectionSeq.push_back( singleMapNode(actorName, pDialogLink->durations[i]) );
                else
                    sectionSeq.push_back( singleMapNode(actorName, pDialogLink->lines[i]) );
            }
        }
        sectionSeq.push_back( lastCloned(root["dialogscript"][sectionName]) );
        root["dialogscript"][sectionName] = sectionSeq;
    } else {
        error("ERROR: section [" + sectionName + "] not found for updateDialogscriptSection!");
    }
}

YAML::Node YmlSceneManager::shotActionToNode(shotAction *sa)
{
    YAML::Node actionMap;
    YAML::Node mainParams;
    mainParams.SetStyle(YAML::EmitterStyle::Flow);
    mainParams.push_back(sa->start);
    YAML::Node extraParams;
    extraParams.SetStyle(YAML::EmitterStyle::Block);

    switch (sa->actionType) {
        case EShotUnknown:
            mainParams.push_back("EShotUnknown!!!");
            break;
        case EShotCam:
        case EShotCamBlendStart:
        case EShotCamBlendKey:
        case EShotCamBlendEnd:
        case EShotCamBlendTogame:
            if (sa->values.contains("cam_name"))
                mainParams.push_back(SG.getName(sa->values["cam_name"].toInt()));
            if (sa->values.contains("cam_ease"))
                mainParams.push_back(sa->values["cam_ease"].toString());
            break;

        case EShotActorAnim:
        case EShotActorAnimAdditive:
        case EShotActorMimicAnim:
            if (sa->values.contains("animation"))
                mainParams.push_back(SG.getName(sa->values["animation"].toInt()));
            if (sa->values.contains("actor")) {
                extraParams["actor"] = SG.getName(sa->values["actor"].toInt());
            }
            if (sa->values.contains("blendin")) {
                extraParams["blendin"] = sa->values["blendin"].toDouble();
            }
            if (sa->values.contains("blendout")) {
                extraParams["blendout"] = sa->values["blendout"].toDouble();
            }
            if (sa->values.contains("clipfront")) {
                extraParams["clipfront"] = sa->values["clipfront"].toDouble();
            }
            if (sa->values.contains("clipend")) {
                extraParams["clipend"] = sa->values["clipend"].toDouble();
            }
            if (sa->values.contains("weight")) {
                extraParams["weight"] = sa->values["weight"].toDouble();
            }
            if (sa->values.contains("stretch")) {
                extraParams["stretch"] = sa->values["stretch"].toDouble();
            }
            break;
        case EShotActorAnimPose:
            if (sa->values.contains("pose"))
                mainParams.push_back(SG.getName(sa->values["pose"].toInt()));
            break;
        case EShotActorMimicPose:
            if (sa->values.contains("mimic"))
                mainParams.push_back(SG.getName(sa->values["mimic"].toInt()));
            if (sa->values.contains("actor")) {
                extraParams["actor"] = SG.getName(sa->values["actor"].toInt());
            }
            if (sa->values.contains("emotional_state")) {
                extraParams["emotional_state"] = sa->values["emotional_state"].toString();
            }
            if (sa->values.contains("pose")) {
                extraParams["pose"] = sa->values["pose"].toString();
            }
            if (sa->values.contains("eyes")) {
                extraParams["eyes"] = sa->values["eyes"].toString();
            }
            if (sa->values.contains("anim")) {
                extraParams["anim"] = sa->values["anim"].toString();
            }
            if (sa->values.contains("weight")) {
                extraParams["weight"] = sa->values["weight"].toDouble();
            }
            if (sa->values.contains("duration")) {
                extraParams["duration"] = sa->values["duration"].toDouble();
            }
            break;
        case EShotActorPlacement:
        case EShotActorPlacementStart:
        case EShotActorPlacementKey:
        case EShotActorPlacementEnd:
        case EShotPropPlacement:
        case EShotPropPlacementStart:
        case EShotPropPlacementKey:
        case EShotPropPlacementEnd:
            if (sa->values.contains("actor"))
                mainParams.push_back(SG.getName(sa->values["actor"].toInt()));
            if (sa->values.contains("pos"))
                mainParams.push_back(sa->values["pos"].value<QVector3D>());
            if (sa->values.contains("rot"))
                mainParams.push_back(sa->values["rot"].value<QVector3D>());
            if (sa->values.contains("ease"))
                mainParams.push_back(sa->values["ease"].toString());
            break;
        case EShotActorGamestate:
            if (sa->values.contains("actor"))
                mainParams.push_back(SG.getName(sa->values["actor"].toInt()));
            if (sa->values.contains("action")) {
                extraParams["action"] = sa->values["action"].toString();
            }
            if (sa->values.contains("behavior")) {
                extraParams["behavior"] = sa->values["behavior"].toDouble();
            }
            break;
        case EShotActorLookat:
            if (sa->values.contains("actor"))
                mainParams.push_back(SG.getName(sa->values["actor"].toInt()));
            if (sa->values.contains("lookat_actor"))
                mainParams.push_back(SG.getName(sa->values["lookat_actor"].toInt()));
            else if (sa->values.contains("lookat_pos"))
                mainParams.push_back(sa->values["lookat_pos"].value<QVector3D>());
            if (sa->values.contains("turn")) {
                extraParams["turn"] = sa->values["turn"].toString();
            }
            if (sa->values.contains("speed")) {
                extraParams["speed"] = sa->values["speed"].toDouble();
            }
            break;
        case EShotActorShow:
        case EShotActorHide:
        case EShotActorScabbardShow:
        case EShotActorScabbardHide:
        case EShotActorUnequipRight:
        case EShotActorUnequipLeft:
        case EShotPropShow:
        case EShotPropHide:
            if (sa->values.contains("actor"))
                mainParams.push_back(SG.getName(sa->values["actor"].toInt()));
            break;
        case EShotActorEffectStart:
        case EShotActorEffectStop:
        case EShotPropEffectStart:
        case EShotPropEffectStop:
        case EShotActorSound:
            if (sa->values.contains("actor"))
                mainParams.push_back(SG.getName(sa->values["actor"].toInt()));
            if (sa->values.contains("effect"))
                mainParams.push_back(sa->values["effect"].toString());
            break;
        case EShotActorAppearance:
            if (sa->values.contains("actor"))
                mainParams.push_back(SG.getName(sa->values["actor"].toInt()));
            if (sa->values.contains("appearance"))
                mainParams.push_back(sa->values["appearance"].toString());
            break;
        case EShotActorEquipRight:
        case EShotActorEquipLeft:
            if (sa->values.contains("actor"))
                mainParams.push_back(SG.getName(sa->values["actor"].toInt()));
            if (sa->values.contains("item"))
                mainParams.push_back(sa->values["item"].toString());
            break;
        case EShotEnvBlendIn:
        case EShotEnvBlendOut:
            if (sa->values.contains("actor"))
                mainParams.push_back(SG.getName(sa->values["actor"].toInt()));
            if (sa->values.contains("envPath"))
                mainParams.push_back(sa->values["envPath"].toString());
            if (sa->values.contains("blendTime"))
                mainParams.push_back(sa->values["blendTime"].toDouble());
            if (sa->values.contains("priority")) {
                extraParams["priority"] = sa->values["priority"].toInt();
            }
            if (sa->values.contains("blendFactor")) {
                extraParams["blendFactor"] = sa->values["blendFactor"].toDouble();
            }
            break;
        case EShotFadeIn:
        case EShotFadeOut:
            if (sa->values.contains("duration"))
                mainParams.push_back(sa->values["duration"].toDouble());
            if (sa->values.contains("color"))
                mainParams.push_back(sa->values["color"].value<QColor>());
            break;
        case EShotWorldAddfact:
            if (sa->values.contains("fact"))
                mainParams.push_back(sa->values["fact"].toString());
            if (sa->values.contains("value"))
                mainParams.push_back(sa->values["value"].toDouble());
            if (sa->values.contains("validFor"))
                mainParams.push_back(sa->values["validFor"].toDouble());
            break;
        case EShotWorldWeather:
            if (sa->values.contains("weatherName"))
                mainParams.push_back(sa->values["weatherName"].toString());
            if (sa->values.contains("blendTime"))
                mainParams.push_back(sa->values["blendTime"].toInt());
            break;
        case EShotWorldEffectStart:
        case EShotWorldEffectStop:
            if (sa->values.contains("tag"))
                mainParams.push_back(sa->values["tag"].toString());
            if (sa->values.contains("effect"))
                mainParams.push_back(sa->values["effect"].toString());
            break;
    }
    if (extraParams.size() == 0) {
        actionMap[ EShotActionToString[sa->actionType] ] = mainParams;
    } else {
        extraParams[".@pos"] = mainParams;
        actionMap[ EShotActionToString[sa->actionType] ] = extraParams;
    }
    return actionMap;
}

void YmlSceneManager::updateShot(QString sectionName, QString shotName)
{
    int shotNum = m_dialogLinkBySectionName[sectionName].shotNumByName(shotName);
    updateShot(sectionName, shotNum);
}

void YmlSceneManager::updateShot(QString sectionName, int shotNum)
{
    QString shotName = m_dialogLinkBySectionName[sectionName].shots[shotNum].shotName;
    int actionsCount = m_dialogLinkBySectionName[sectionName].shots[shotNum].actions.count();

    // remove shot if no actions, remove sb section if no shots
    if (actionsCount == 0 && root["storyboard"][sectionName][shotName]) {
        root["storyboard"][sectionName].remove(shotName);
        if (root["storyboard"][sectionName].size() == 0) {
            root["storyboard"].remove(sectionName);
        }
        // update dg section to remove CUE for empty shots
        updateDialogscriptSection(sectionName);
    }

    // create shot if actions are added
    if (actionsCount > 0) {
        // create sb section if shot is going to be added
        if (!root["storyboard"][sectionName]) {
            root["storyboard"][sectionName] = YAML::Node(YAML::NodeType::Map);
        }
        // create sb shot if actions are added
        if (!root["storyboard"][sectionName][shotName]) {
            root["storyboard"][sectionName][shotName] = YAML::Node(YAML::NodeType::Map);
            // update dg section to add CUE for shot
            updateDialogscriptSection(sectionName);
        }
        YAML::Node shotSeq(YAML::NodeType::Sequence);
        shotSeq.SetStyle(YAML::EmitterStyle::Block);
        upn(i, 0, actionsCount - 1) {
            shotAction* sa = &m_dialogLinkBySectionName[sectionName].shots[shotNum].actions[i];
            shotSeq.push_back( shotActionToNode(sa) );
        }
        root["storyboard"][sectionName][shotName] = shotSeq;
    }
}

void YmlSceneManager::removeShot(QString sectionName, QString shotName)
{
    int shotNum = m_dialogLinkBySectionName[sectionName].shotNumByName(shotName);

    // remove dialog info
    m_dialogLinkBySectionName[sectionName].shots.removeAt(shotNum);
    m_dialogLinkBySectionName[sectionName].durations.removeAt(shotNum);
    m_dialogLinkBySectionName[sectionName].lines.removeAt(shotNum);
    m_dialogLinkBySectionName[sectionName].speakers.removeAt(shotNum);

    // remove shot from storyboard section
    if (root["storyboard"][sectionName][shotName]) {
        root["storyboard"][sectionName].remove(shotName);
        // remove section if empty
        if (root["storyboard"][sectionName].size() == 0) {
            root["storyboard"].remove(sectionName);
        }
    }

    // update dg section to remove actor line and CUE
    updateDialogscriptSection(sectionName);
}

void YmlSceneManager::addShot(QString sectionName, int shotNum)
{
    dialogLink* pDialogLink = &m_dialogLinkBySectionName[sectionName];

    info( QString("onShotAdd: in %1, at [%2]").arg(sectionName).arg(shotNum) );
    int shotNameNumber = 0;
    while (pDialogLink->shotNames.contains("shot_" + qn(shotNameNumber) )) {
        ++shotNameNumber;
    }
    QString shotName = "shot_" + qn(shotNameNumber);

    pDialogLink->shotNames.insert( shotName );

    pDialogLink->durations.insert( shotNum, 5.0 );
    pDialogLink->lines.insert( shotNum, "PAUSE" );
    pDialogLink->speakers.insert( shotNum, sceneGlobals()->getID(SASSETS, "PAUSE") );
    pDialogLink->shots.insert( shotNum, shot(shotName) );
    pDialogLink->calculateTotalDuration();

    updateShot(sectionName, shotName);
    updateDialogscriptSection(sectionName);
}

template<typename HashContainer>
void YmlSceneManager::removeAssetsFromSG(HashContainer& container, bool clearRepo)
{
    auto it = container.begin();
    while (it != container.end()) {
        if (!clearRepo && it->fromRepo)
            ++it;
        else {
            SG.removeID(it->nameID);
            it = container.erase(it);
        }
    }
}


template<typename T>
YAML::Node YmlSceneManager::scalarNode(const T &value)
{
    YAML::Node node(YAML::NodeType::Scalar);
    node = value;
    return node;
}
template<typename T>
YAML::Node YmlSceneManager::singleMapNode(const QString &key, const T &value)
{
    YAML::Node node(YAML::NodeType::Map);
    node[key] = value;
    return node;
}
