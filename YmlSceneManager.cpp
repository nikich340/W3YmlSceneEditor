#include "YmlSceneManager.h"
#include "GraphicsSectionItem.h"
#include <yaml-cpp/exceptions.h>

#include <typeinfo>
#include <QDebug>
#include <fstream>
#include <QFile>
#include <QDir>
#include <QTextStream>

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
    m_pScene = gScene;
    qDebug() << "YmlSceneManager()";

    /* Init SceneGlobals */
    SG.init();
    //SG.getID(SACTORS, "PAUSE");
    //SG.getID(SACTORS, "CHOICE");
}

void YmlSceneManager::loadCsvLines()
{
    /* read vanilla lines info TODO on release */
    //QDir w3dataDir(QCoreApplication::applicationDirPath() + "/w3.data");
    QDir w3dataDir("C:/DATA/QT_projects/YmlSceneEditor/w3.data");
    QStringList csvFilesList = w3dataDir.entryList({ "*.lines.csv" }, QDir::Files | QDir::Readable);
    for (const QString& w3LinesCsv : csvFilesList) {
        QFile w3LinesFile(w3dataDir.path() + "/" + w3LinesCsv);
        if (w3LinesFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
           QTextStream in(&w3LinesFile);
           dialogLine newLine;
           int cnt = 0;

           while (!in.atEnd())
           {
              QString line = in.readLine();
              if (line.startsWith(";"))
                  continue;
              QStringList lst = line.split("|");
              if (lst.size() < 4) {
                  warning(QString("Parse csv: Incorrect format in line #%1: %2").arg(cnt).arg(line));
                  continue;
              }
              ++cnt;
              bool convertOK = true;
              newLine.id = lst[0].toUInt(&convertOK, 10);
              if (!convertOK) {
                  error(QString("Parse csv: Incorrect id format in line #%1: %2 (%3)").arg(cnt).arg(lst[0]).arg(line));
                  continue;
              }

              convertOK = true;
              newLine.key_hex = lst[1].toUInt(&convertOK, 16);
              if (!convertOK) {
                  warning(QString("Parse csv: Incorrect hex key format in line #%1: %2 (%3)").arg(cnt).arg(lst[1]).arg(line));
                  newLine.key_hex = 0;
              }

              convertOK = true;
              newLine.duration = lst[2].toDouble(&convertOK);
              if (!convertOK) {
                  qw << QString("Parse csv: Incorrect duration format in line #%1: %2 (%3)").arg(cnt).arg(lst[2]).arg(line);
                  newLine.duration = -1.0;
              }
              newLine.text = lst[3];
              if (newLine.text.isEmpty()) {
                  error(QString("Parse csv: No text in line #%1: %2").arg(cnt).arg(line));
                  continue;
              }
              m_lineByID[newLine.id] = newLine;
              m_linesStorage.addStringUnsafe(newLine.text, newLine.id);
           }

           w3LinesFile.close();
           info(QString("Loaded %1 dialog lines from csv: %2").arg(cnt).arg(w3LinesCsv));
        } else {
            error(QString("Failed to read dialoglines csv: %1").arg(w3LinesCsv));
        }
    }
    m_linesStorage.sort();
}

void YmlSceneManager::loadEnvPaths()
{
    //QDir w3dataDir(QCoreApplication::applicationDirPath() + "/w3.data");
    QDir w3dataDir("C:/DATA/QT_projects/YmlSceneEditor/w3.data");
    QStringList csvFilesList = w3dataDir.entryList({ "*.envs.csv" }, QDir::Files | QDir::Readable);
    for (const QString& w3EnvCsv : csvFilesList) {
        QFile w3CsvFile(w3dataDir.path() + "/" + w3EnvCsv);
        int cnt = 0;
        if (w3CsvFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
           QTextStream in(&w3CsvFile);
           while (!in.atEnd())
           {
              QString line = in.readLine();
              if (line.startsWith(";"))
                  continue;

              if (!line.endsWith(".env")) {
                  warning(QString("%1: incorrect env path?: %2").arg(Q_FUNC_INFO).arg(line));
              }
              cnt += 1;
              m_envStorage.addStringUnsafe(line);
           }
           m_envStorage.sort();
           w3CsvFile.close();
           info(QString("Loaded %1 env paths from csv: %2").arg(cnt).arg(w3EnvCsv));
        } else {
            error(QString("Failed to read env csv: %1").arg(w3EnvCsv));
        }
    }
}

YmlSceneManager::~YmlSceneManager()
{
    //m_pScene->clear();
	for(dialogLink* pLink : qAsConst(m_pDialogLinkBySectionName)) {
		for (shot sh : pLink->shots) {
			deletePointersFromContainer( sh.actions );
		}
	}
    deletePointersFromContainer( m_pSectionLinkBySectionName );
    deletePointersFromContainer( m_pDialogLinkBySectionName );
    qDebug() << "~YmlSceneManager()";
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
    m_pScene->clear();
	// clear old (sections graph)
    deletePointersFromContainer( m_pSectionLinkBySectionName );
    m_pSectionLinkBySectionName.clear();
    m_startSections.clear();
    m_wasDrawn.clear();
    m_pItemBySectionName.clear();
    m_sectionNamesByDepth.clear();

	// clear old (dialogscript + storyboard shots)
    // deleted by scene -> deletePointersFromContainer( m_pItemBySectionName );
    for(dialogLink* pLink : qAsConst(m_pDialogLinkBySectionName)) {
        for (shot sh : pLink->shots) {
            deletePointersFromContainer( sh.actions );
        }
    }
    deletePointersFromContainer( m_pDialogLinkBySectionName );
    m_pDialogLinkBySectionName.clear();
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
}
bool YmlSceneManager::loadYmlRepo(QString path) {
	readingYmlRepo = true;
	// load yml
	try {
        m_root.reset();
        m_root = YAML::LoadFile(path.toStdString());
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
    QElapsedTimer timer;
    timer.start();
    m_filePath = path;
	qi << "Parsing " + path + "...";

    // load yml
    try {
        m_root.reset();
        m_root = YAML::LoadFile(path.toStdString());
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
    info(QString("YML loaded in %1 ms: %2").arg(timer.elapsed()).arg(path));

    emit ymlFileLoaded(path);
    return true;
}
bool YmlSceneManager::saveYmlFile() {
    QFile ymlFile(m_filePath);

    if (ymlFile.exists())
        ymlFile.copy(m_filePath + ".backup");
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
        if ( m_pItemBySectionName[name]->state == GraphicsSectionItem::incomplete ) {
			QMessageBox msgBox;
			msgBox.setText("Save is not allowed: section [" + name + "] is incomplete!");
			msgBox.setInformativeText("Make double click on it to edit settings.");
			msgBox.setStandardButtons(QMessageBox::Ok);
			msgBox.setDefaultButton(QMessageBox::Ok);
			int ret = msgBox.exec();

			return false;
		}
	}

    std::ofstream out(m_filePath.toStdString());
	out.clear();

    if (out.good())
        out << m_root;
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
    if (m_root["dialogscript"]) {
        for(YAML::const_iterator it = m_root["dialogscript"].begin(); it != m_root["dialogscript"].end(); ++it) {
            QString sectionName = it->XKey.as<QString>().toLower(); // make all names toLower!
			qDebug() << "sectionName: " << sectionName;

               // section
            if ( sectionName.startsWith("section_") || sectionName.startsWith("script_") ) {
                if (m_pSectionLinkBySectionName.contains(sectionName)) {
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
                sectionLink* tmpLink = new sectionLink(sectionName);

                   // NEXT, (RANDOM, CHOICE)
                if (lastSectionNode.IsMap()) {
                    QString lastSectionCue = lastSectionNode.begin()->XKey.as<QString>();
					if (lastSectionCue.toUpper() == "NEXT") {
                            // simple NEXT
                        if (lastSectionNode.begin()->YValue.IsScalar()) {
                            tmpLink->addChoice( lastSectionNode.begin()->YValue.as<QString>() );
                            tmpLink->type = ESectionNext;
                            // advanced NEXT with condition
                        } else if (lastSectionNode.begin()->YValue.IsMap()
                                   && lastSectionNode.begin()->YValue["condition"]
                                   && lastSectionNode.begin()->YValue["condition"].IsSequence()
                                   && lastSectionNode.begin()->YValue["condition"].size() == 3
                                   && lastSectionNode.begin()->YValue["on_true"]
                                   && lastSectionNode.begin()->YValue["on_false"]) {
                            tmpLink->type = ESectionCondition;
                            tmpLink->addChoice( lastSectionNode.begin()->YValue["on_true"].as<QString>(), QString(), choiceAction(), lastSectionNode.begin()->YValue["condition"].as<ymlCond>() );
                            tmpLink->addChoice( lastSectionNode.begin()->YValue["on_false"].as<QString>() );
                        } else {
							showError("Error: [" + sectionName + "]: incorrect last element");
                            return false;
                        }
					} else if (lastSectionCue.toUpper() == "CHOICE"
                               && lastSectionNode.begin()->YValue.IsSequence()) {
                        tmpLink->type = ESectionChoice;
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
                        tmpLink->type = ESectionRandom;

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
                    tmpLink->type = ESectionExit;
                   // usual randomizer section
                } else if (sectionNode.IsSequence()
                           && sectionName.startsWith("section_randomizer")) {
                    tmpLink->type = ESectionRandom;

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
                    tmpLink->type = ESectionScript;
                }

                if ( sectionName.startsWith("section_start") ) {
                    m_startSections.push_back( sectionName );
                }

                // all is fine, add to graph
                m_sectionNames.append(sectionName);
                m_pSectionLinkBySectionName.insert(sectionName, tmpLink);
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

    info("Loaded " + qn(m_pSectionLinkBySectionName.size()) + " sections");
    return true;
}

bool YmlSceneManager::loadShotActions(const YAML::Node actsNode, shot& sh) {
	upn(k, 0, (int) actsNode.size() - 1) {
		YAML::Node actionNode = actsNode[k];
		QString actionName = actionNode.begin()->XKey.as<QString>().toLower();
        EShotActionType actionType = CONSTANTS::stringToEShotAction.value(actionName, EShotUnknown);

        SA_Base* pNewAction = SA_Base::createShotAction(actionType, sceneGlobals());
        QString errorMsg;
        if ( !pNewAction->importYmlNode(actionNode.begin()->YValue, errorMsg) ) {
            error(errorMsg);
            continue;
        }
        pNewAction->updateYmlNode();
        qd << QString("%1: Created SA with type: %2").arg(Q_FUNC_INFO).arg(pNewAction->actionTypeStr());
        sh.actions.pb(pNewAction);
    }
	return true;
}

bool YmlSceneManager::loadSceneRepository() {
    if (!m_root["repository"])	{
		warning("loadSceneRepository()>: No repository found!");
		return true;
	}

	/* LOAD ACTORS FROM REPO */
    if (m_root["repository"]["actors"]) {
        qd << "loadSceneRepository()>: Loading " << m_root["repository"]["actors"].size() << " actors..";
		int cnt_new = 0;

        for (auto it = m_root["repository"]["actors"].begin(); it != m_root["repository"]["actors"].end(); ++it) {
            QString mapName = it->XKey.as<QString>() + CONSTANTS::REPO_TEMP;
            if ( SG.hasName(ERepoAssets, mapName) )
			{
				qw << ("loadSceneRepository()>: actor " + mapName + " already exist! Overwriting.");
				//continue;
			}
            int nameID = SG.getID(ERepoAssets, mapName);

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
    if (m_root["repository"]["props"]) {
        qd << "loadSceneRepository()>: Loading " << m_root["repository"]["props"].size() << " props..";
		int cnt_new = 0;

        for (auto it = m_root["repository"]["props"].begin(); it != m_root["repository"]["props"].end(); ++it) {
            QString mapName = it->XKey.as<QString>() + CONSTANTS::REPO_TEMP;
            if ( SG.hasName(ERepoAssets, mapName) )
			{
				qw << ("loadSceneRepository()>: prop " + mapName + " already exist! Overwriting.");
				//continue;
			}
            int nameID = SG.getID(ERepoAssets, mapName);

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
    if (m_root["repository"]["cameras"]) {
        qd << "loadSceneRepository()>: Loading " << m_root["repository"]["cameras"].size() << " cameras..";
		int cnt_new = 0;

        for (auto it = m_root["repository"]["cameras"].begin(); it != m_root["repository"]["cameras"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();
            if ( SG.hasName(ERepoAssets, mapName) )
			{
				qw << ("loadSceneRepository()>: camera " + mapName + " already exist! Overwriting.");
				//continue;
			}
            int nameID = SG.getID(ERepoCameras, mapName);

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
    if (m_root["repository"]["animations"]) {
        qd << "loadSceneRepository()>: Loading " << m_root["repository"]["animations"].size() << " animations..";
		int cnt_new = 0;

        for (auto it = m_root["repository"]["animations"].begin(); it != m_root["repository"]["animations"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();
            if ( SG.hasName(ERepoAnims, mapName) )
			{
				qw << ("loadSceneRepository()>: anim " + mapName + " already exist! Overwriting.");
				//continue;
			}
            int nameID = SG.getID(ERepoAnims, mapName);

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
    if (m_root["repository"]["animations.mimic"]) {
        qd << "loadSceneRepository()>: Loading " << m_root["repository"]["animations.mimic"].size() << " mimic anims..";
		int cnt_new = 0;

        for (auto it = m_root["repository"]["animations.mimic"].begin(); it != m_root["repository"]["animations.mimic"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();
            if ( SG.hasName(ERepoMimicAnims, mapName) )
			{
                qw << ("loadSceneRepository()>: mimic anim " + mapName + " already exist! Overwriting.");
				//continue;
			}
            int nameID = SG.getID(ERepoMimicAnims, mapName);

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
    if (m_root["repository"]["actor.poses"]) {
        qd << "loadSceneRepository()>: Loading " << m_root["repository"]["actor.poses"].size() << " poses..";
		int cnt_new = 0;

        for (auto it = m_root["repository"]["actor.poses"].begin(); it != m_root["repository"]["actor.poses"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();
            if ( SG.hasName(ERepoAnimPoses, mapName) )
			{
				qw << ("loadSceneRepository()>: pose " + mapName + " already exist! Overwriting.");
				//continue;
			}
            int nameID = SG.getID(ERepoAnimPoses, mapName);

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
    if (m_root["repository"]["mimics"]) {
        qd << "loadSceneRepository()>: Loading " << m_root["repository"]["mimics"].size() << " mimics..";
		int cnt_new = 0;

        for (auto it = m_root["repository"]["mimics"].begin(); it != m_root["repository"]["mimics"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();
            if ( SG.hasName(ERepoMimicPoses, mapName) )
			{
                qw << ("loadSceneRepository(): mimic pose " + mapName + " already exist! Overwriting.");
				//continue;
			}
            int nameID = SG.getID(ERepoMimicPoses, mapName);

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
    if (m_root["repository"]["soundbanks"]) {
        qd << "loadSceneRepository()>: Loading " << m_root["repository"]["soundbanks"].size() << " soundbanks..";
		int cnt_new = 0;

        for (auto it = m_root["repository"]["soundbanks"].begin(); it != m_root["repository"]["soundbanks"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();
            if ( SG.hasName(ERepoSoundbanks, mapName) )
			{
				qw << ("loadSceneRepository(): soundbank " + mapName + " already exists!");
				//continue;
			}
            int nameID = SG.getID(ERepoSoundbanks, mapName);

            if ( !it->YValue.IsSequence() ) {
				warning("loadSceneRepository()>: soundbank " + mapName + ": expected sound events sequence!");
				continue;
			}
			QSet<QString> soundbankSet;
            upn(j, 0, it->YValue.size() - 1) {
                soundbankSet.insert( it->YValue[j].as<QString>() );
			}
            //qd << "SOUNBANK ADD: " << mapName << " [" << qn(soundbankSet.size()) << "]";

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
        if ( name.endsWith(CONSTANTS::REPO_TEMP) ) {
            qd << "cleanupTempRepository(): CLEANED";
            SG.actors.remove(nameID);
            SG.removeName(ERepoAssets, name);
        }
    }

    QVector<int> propIDs = SG.props.keys().toVector();
    dn(i, propIDs.count() - 1, 0) {
        int nameID = propIDs[i];
        QString name = SG.getName(nameID);
        qd << "cleanupTempRepository(): " << name;
        if ( name.endsWith(CONSTANTS::REPO_TEMP) ) {
            qd << "cleanupTempRepository(): CLEANED";
            SG.props.remove(nameID);
            SG.removeName(ERepoAssets, name);
        }
    }
}

bool YmlSceneManager::loadSceneProduction() {
    if (!m_root["production"]["assets"])	{
		warning("loadSceneProduction()>: No production assets!");
		return true;
	}

	/* LOAD ACTORS FROM PRODUCTION */
    if (m_root["production"]["assets"]["actors"]) {
        qd << "loadSceneProduction()>: Loading " << m_root["production"]["assets"]["actors"].size() << " actors..";
		//QSet<QString> dontRemoveKeys; // for renamed (mapName -> repoName)

        for (auto it = m_root["production"]["assets"]["actors"].begin(); it != m_root["production"]["assets"]["actors"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();

            if ( !it->YValue["repo"] ) {
				error("loadSceneProduction()>: No repo value found for actor " + mapName);
				continue;
			}
            QString repoName = it->YValue["repo"].as<QString>();
            if ( !SG.hasName(ERepoAssets, repoName + CONSTANTS::REPO_TEMP) ) {
				error("loadSceneProduction()>: Actor " + repoName + " not found in repository!");
				continue;
			}
            int repoID = SG.getID(ERepoAssets, repoName + CONSTANTS::REPO_TEMP);
            int prodID = SG.getID(ERepoAssets, mapName);

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
    if (m_root["production"]["assets"]["props"]) {
        qd << "loadSceneProduction()>: Loading " << m_root["production"]["assets"]["props"].size() << " props..";

        for (auto it = m_root["production"]["assets"]["props"].begin(); it != m_root["production"]["assets"]["props"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();

            if ( !it->YValue["repo"] ) {
				error("loadSceneProduction()>: No repo value found for prop " + mapName);
				continue;
			}
            QString repoName = it->YValue["repo"].as<QString>();
            if ( !SG.hasName(ERepoAssets, repoName + CONSTANTS::REPO_TEMP) ) {
				error("loadSceneProduction()>: Prop " + repoName + " not found in repository!");
				continue;
			}

            int repoID = SG.getID(ERepoAssets, repoName + CONSTANTS::REPO_TEMP);
            int prodID = SG.getID(ERepoAssets, mapName);

			asset prodProp = SG.props[repoID];
            prodProp.is_prop = true;
            prodProp.nameID = prodID;

			if (readingYmlRepo)
				prodProp.fromRepo = true;
			SG.props[prodID] = prodProp;
		}
	}

	/* LOAD CAMERAS FROM PRODUCTION */
    if (m_root["production"]["assets"]["cameras"]) {
        qd << "loadSceneProduction()>: Loading " << m_root["production"]["assets"]["cameras"].size() << " cameras..";
		//QSet<QString> dontRemoveKeys; // for renamed (mapName -> repoName)

        for (auto it = m_root["production"]["assets"]["cameras"].begin(); it != m_root["production"]["assets"]["cameras"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();

            if ( !it->YValue["repo"] ) {
				error("loadSceneProduction()>: No repo value found for camera " + mapName);
				continue;
			}
            QString repoName = it->YValue["repo"].as<QString>();
            if ( !SG.hasName(ERepoCameras, repoName) ) {
				error("loadSceneProduction()>: Camera " + repoName + " not found in repository!");
				continue;
			}

            int repoID = SG.getID(ERepoCameras, repoName);
            int prodID = SG.getID(ERepoCameras, mapName);

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
    if (m_root["production"]["assets"]["animations"]) {
        qd << "loadSceneProduction()>: Loading " << m_root["production"]["assets"]["animations"].size() << " animations..";

        for (auto it = m_root["production"]["assets"]["animations"].begin(); it != m_root["production"]["assets"]["animations"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();

            if ( !it->YValue["repo"] ) {
				error("loadSceneProduction()>: No repo value found for animation " + mapName);
				continue;
			}
            QString repoName = it->YValue["repo"].as<QString>();
            if ( !SG.hasName(ERepoAnims, repoName) ) {
				error("loadSceneProduction()>: Animation " + repoName + " not found in repository!");
				continue;
			}

            int repoID = SG.getID(ERepoAnims, repoName);
            int prodID = SG.getID(ERepoAnims, mapName);

			animation prodAnim = SG.anims[repoID];
			prodAnim.nameID = prodID;

            if ( it->YValue["actor"] ) {
                QString actorName = it->YValue["actor"].as<QString>();
                if (!SG.hasName(ERepoAssets, actorName)) {
					error("loadSceneProduction()>: Animation actor " + actorName + " not found in repository!");
				} else {
                    int actorID = SG.getID(ERepoAssets, actorName);
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
    if (m_root["production"]["assets"]["animations.mimic"]) {
        qd << "loadSceneProduction()>: Loading " << m_root["production"]["assets"]["animations.mimic"].size() << " mimic anims..";

        for (auto it = m_root["production"]["assets"]["animations.mimic"].begin(); it != m_root["production"]["assets"]["animations.mimic"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();

            if ( !it->YValue["repo"] ) {
				error("loadSceneProduction()>: No repo value found for mimic animation " + mapName);
				continue;
			}
            QString repoName = it->YValue["repo"].as<QString>();
            if ( !SG.hasName(ERepoMimicAnims, repoName) ) {
				error("loadSceneProduction()>: Mimic animation " + repoName + " not found in repository!");
				continue;
			}

            int repoID = SG.getID(ERepoMimicAnims, repoName);
            int prodID = SG.getID(ERepoMimicAnims, mapName);

			animation prodMimicAnim = SG.mimics[repoID];
			prodMimicAnim.nameID = prodID;

            if ( it->YValue["actor"] ) {
                QString actorName = it->YValue["actor"].as<QString>();
                if (!SG.hasName(ERepoAssets, actorName)) {
					error("loadSceneProduction()>: Animation Mimic actor " + actorName + " not found in repository!");
				} else {
                    int actorID = SG.getID(ERepoAssets, actorName);
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
    if (m_root["production"]["assets"]["actor.poses"]) {
        qd << "loadSceneProduction()>: Loading " << m_root["production"]["assets"]["actor.poses"].size() << " mimic anims..";

        for (auto it = m_root["production"]["assets"]["actor.poses"].begin(); it != m_root["production"]["assets"]["actor.poses"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();

            if ( !it->YValue["repo"] ) {
				error("loadSceneProduction()>: No repo value found for actor.pose " + mapName);
				continue;
			}
            QString repoName = it->YValue["repo"].as<QString>();
            if ( !SG.hasName(ERepoAnimPoses, repoName) ) {
				error("loadSceneProduction()>: Actor.pose " + repoName + " not found in repository!");
				continue;
			}

            int repoID = SG.getID(ERepoAnimPoses, repoName);
            int prodID = SG.getID(ERepoAnimPoses, mapName);

			anim_pose prodPose = SG.poses[repoID];
			prodPose.nameID = prodID;

            if ( it->YValue["actor"] ) {
                QString actorName = it->YValue["actor"].as<QString>();
                if (!SG.hasName(ERepoAssets, actorName)) {
					error("loadSceneProduction()>: Pose actor " + actorName + " not found in repository!");
				} else {
                    int actorID = SG.getID(ERepoAssets, actorName);
					prodPose.actorID = actorID;
				}
			}

			if (readingYmlRepo)
				prodPose.fromRepo = true;
			SG.poses[prodID] = prodPose;
		}
	}

	/* LOAD MIMICS (MIMIC POSES) FROM PRODUCTION */
    if (m_root["production"]["assets"]["mimics"]) {
        qd << "loadSceneProduction()>: Loading " << m_root["production"]["assets"]["mimics"].size() << " mimics (mimic poses)..";

        for (auto it = m_root["production"]["assets"]["mimics"].begin(); it != m_root["production"]["assets"]["mimics"].end(); ++it) {
            QString mapName = it->XKey.as<QString>();

            if ( !it->YValue["repo"] ) {
				error("loadSceneProduction()>: No repo value found for mimics " + mapName);
				continue;
			}
            QString repoName = it->YValue["repo"].as<QString>();
            if ( !SG.hasName(ERepoMimicPoses, repoName) ) {
				error("loadSceneProduction()>: Mimics (mimic pose) " + repoName + " not found in repository!");
				continue;
			}

            int repoID = SG.getID(ERepoMimicPoses, repoName);
            int prodID = SG.getID(ERepoMimicPoses, mapName);

			mimic_pose prodMimicPose = SG.mimic_poses[repoID];
			prodMimicPose.nameID = prodID;

            if ( it->YValue["actor"] ) {
                QString actorName = it->YValue["actor"].as<QString>();
                if (!SG.hasName(ERepoAssets, actorName)) {
					error("loadSceneProduction()>: Mimic Pose actor " + actorName + " not found in repository!");
				} else {
                    int actorID = SG.getID(ERepoAssets, actorName);
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
    if (m_root["production"]["assets"]["soundbanks"]) {
        qd << "loadSceneProduction()>: Loading " << m_root["production"]["assets"]["soundbanks"].size() << " soundbanks..";

        upn(j, 0, m_root["production"]["assets"]["soundbanks"].size() - 1) {
            QString bankName = m_root["production"]["assets"]["soundbanks"][j].as<QString>();
            if ( !SG.hasName(ERepoSoundbanks, bankName) ) {
				error("loadSceneProduction()>: Soundbank " + bankName + " not found in repository!");
				continue;
			}
            int bankID = SG.getID(ERepoSoundbanks, bankName);

			/*for (auto jt : SG.soundbanks[bankID]) {
				SG.addNameFor("availableSoundEvents", jt);
			}*/
		}
	}
	return true;
}

bool YmlSceneManager::loadSceneDefaults() {
	/* general settings */
    if (m_root["production"]["settings"]) {
        YAML::Node settingsNode = m_root["production"]["settings"];
		if (settingsNode["sceneid"])
			SG.sceneid = settingsNode["sceneid"].as<int>();
		if (settingsNode["strings-idstart"])
			SG.idstart = settingsNode["strings-idstart"].as<int>();
		if (settingsNode["strings-idspace"])
			SG.idspace = settingsNode["strings-idspace"].as<QString>();
	}
    if (m_root["production"]["placement"])
        SG.placementTag = m_root["production"]["placement"].as<QString>();
    if (m_root["production"]["gameplay"])
        SG.gameplay = m_root["production"]["gameplay"].as<bool>();
    if (m_root["production"]["cinematic_subtitles"])
        SG.cinematic_subtitles = m_root["production"]["cinematic_subtitles"].as<bool>();

	/* actors defaults */
    YAML::Node placeNode = m_root["storyboard"]["defaults"]["placement"];
    YAML::Node poseNode = m_root["storyboard"]["defaults"]["actor.pose"];
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
            if ( !SG.hasName(ERepoAnimPoses, poseName) ) {
				error("loadSceneDefaults(): Default pose " + poseName + " was not defined in repo/production (actor: " + actorName + ")");
			} else {
                int poseID = SG.getID(ERepoAnimPoses, poseName);
				SG.defaultPose[actorID] = poseID;
				qd << "loadSceneDefaults(): Set def pose for " << actorName << ": " << poseName;
			}
		}
		// mimic (special case - defined in actor map in: (1) repo, (2) production)
		mimic_pose defMimic;
		// from repository (barely)
        YAML::Node mimicRNode = m_root["repository"]["actors"][actorName]["mimic"];
		if (mimicRNode && mimicRNode.IsMap()) {
            defMimic.nameID = SG.getID(ERepoMimicPoses, actorName + "_default_mimic"); // not really needed
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
        YAML::Node mimicPNode = m_root["production"]["assets"]["actors"][actorName]["mimic"];
		if (mimicPNode && mimicPNode.IsMap()) {
            defMimic.nameID = SG.getID(ERepoMimicPoses, actorName + "_default_mimic"); // not really needed

			if ( mimicPNode["repo"] ) {
				QString mimicPNodeRepo = mimicPNode["repo"].as<QString>();
                if ( !SG.hasName(ERepoMimicPoses, mimicPNodeRepo) ) {
					warning("loadSceneDefaults()>: Mimics (mimic pose) " + mimicPNodeRepo + " for actor " + actorName + " not found in repository!");
				} else {
					// load repo params first
                    defMimic = SG.mimic_poses[ SG.getID(ERepoMimicPoses, mimicPNodeRepo) ];
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
        if (!m_root["dialogscript"][sectionName])	{
			qDebug() << "Exception0: no dialogscript found for section [" << sectionName << "]";
			continue;
		}

        YAML::Node sNode = m_root["dialogscript"][sectionName];

        dialogLink *newDgLink = new dialogLink();
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
                newDgLink->lines.pb( key.toUpper() );
                int actorID = SG.getID(ERepoAssets, key.toUpper());
                newDgLink->speakers.pb( actorID );
				double dur = -1.0;
				if (key.toUpper() == "PAUSE") {
                    dur = it->begin()->YValue.as<double>();
				} else {
                    dur = m_pSectionLinkBySectionName[sectionName]->timeLimit;
                    if (dur < 0.0)
                        dur = 10.0;
				}

                newDgLink->durations.pb( dur );

				shot newShot;
				if ( prevShotName.isEmpty() ) {
					prevShotName = key.toUpper() + "_";
					int idx = 1;
                    while ( newDgLink->shotNames.contains( prevShotName + qn(idx) ) ) {
						++idx;
					}
					prevShotName += qn(idx);
				}
				newShot.shotName = prevShotName;
                newDgLink->shotNames.insert(prevShotName);
                newDgLink->shots.pb( newShot );

				prevShotName = QString();

			} else if ( dgActors.contains(key) ) {
                if ( !it->begin()->YValue.IsScalar() ) {
					qCritical() << "Exception3/2: non-scalar YY (actor)";
					break;
				}
				//qDebug() << "ACTOR key";
                QString line = it->begin()->YValue.as<QString>();
                newDgLink->lines.pb( line );
                if ( !SG.hasName(ERepoAssets, key) ) {
					error(QString(Q_FUNC_INFO) + ": section " + sectionName + ": actor " + key + " not found in repository!");
				}
                int actorID = SG.getID(ERepoAssets, key);
				warning("TEST actor: " + SG.getName(actorID));
                newDgLink->speakers.pb( actorID );
                newDgLink->durations.pb( getTextDuration(line) ); // TODO: try extract duration

				shot newShot;
				if ( prevShotName.isEmpty() ) {
					prevShotName = key;
					int idx = 1;
                    while ( newDgLink->shotNames.contains( prevShotName + qn(idx) ) ) {
						++idx;
					}
					prevShotName += qn(idx);
				}
				newShot.shotName = prevShotName;
                newDgLink->shotNames.insert(prevShotName);
                newDgLink->shots.pb( newShot );

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
        newDgLink->calculateTotalDuration();
        m_pDialogLinkBySectionName[sectionName]= newDgLink;
	}

    if (m_root["storyboard"]) {
        for (auto it = m_root["storyboard"].begin(); it != m_root["storyboard"].end(); ++it) {
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
                int shotIdx = m_pDialogLinkBySectionName[sectionName]->shotNumByName(shotName);
				if (shotIdx == -1) {
					qCritical() << "loadShotsInfo()>: Exception6: shot was not found in dialogscript!";
					continue;
				}

                if ( !loadShotActions(jt->YValue, m_pDialogLinkBySectionName[sectionName]->shots[shotIdx]) ) {
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
    for (auto s : m_startSections) {
        if ( !dfsPrepareGraph(s, 0) )
            return false;
    }

    qreal x = 50;
    qreal x_offset = qMin( CONSTANTS::WIDTH * 2.5, (CONSTANTS::SCENE_WIDTH * 0.95) / m_sectionNamesByDepth.size() );
    qreal y_max = -1, y_min = CONSTANTS::SCENE_HEIGHT + 1;
    for (int i = 0; i < m_sectionNamesByDepth.size(); ++i) {
		qreal y = 0;
        qreal y_offset = (CONSTANTS::SCENE_HEIGHT * 0.95) / (m_sectionNamesByDepth[i].size() + 1);

		bool wasStep = true;
		qreal x_offset2 = 0;
        if (y_offset < CONSTANTS::HEIGHT) {
            x_offset2 = CONSTANTS::WIDTH / 1.25;
		}
        for (int j = 0; j < m_sectionNamesByDepth[i].size(); ++j) {
			y += y_offset;
            GraphicsSectionItem* pItem = m_pItemBySectionName[m_sectionNamesByDepth[i][j]];
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

    m_wasDrawn.clear();
    for (auto s : m_startSections) {
        if ( !dfsDrawGraph(s) ) {
            return false;
        }
    }

    // fill sockets
    for (auto it : m_pItemBySectionName) {
        it->fillCleanSockets();
        it->updateState();
    }
    m_pScene->views().first()->fitInView(0, y_min, x, y_max, Qt::KeepAspectRatio);

	hasChanges = false;
    return true;

}

bool YmlSceneManager::dfsPrepareGraph(QString sectionName, int depth) {
    if ( depth > 0 && m_startSections.contains(sectionName) ) {
        // TRYING TO ADD section_start as NEXT, incorrect yml!
        return false;
    }
    if ( !m_pSectionLinkBySectionName.contains(sectionName) ) {
		qDebug() << "Non-existing section: " << sectionName;
		return false;
	}

    if ( m_wasDrawn.contains(sectionName) ) {
        return true;
    }

    m_wasDrawn.insert(sectionName);
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
    m_pScene->addItem( newSect );
    m_pItemBySectionName[sectionName] = newSect;

    if ( m_sectionNamesByDepth.size() < depth + 1) {
        m_sectionNamesByDepth.push_back(QVector<QString>());
    }
    m_sectionNamesByDepth[depth].push_back(sectionName);

    for (auto next : m_pSectionLinkBySectionName[sectionName]->names) {
        if ( !dfsPrepareGraph(next, depth + 1) ) {
            return false;
        }
    }

    return true;
}

bool YmlSceneManager::dfsDrawGraph(QString sectionName) {
    if ( m_wasDrawn.contains(sectionName) ) {
        return true;
    }

    m_wasDrawn.insert(sectionName);
    GraphicsSectionItem* pSect = m_pItemBySectionName[sectionName];

    for (auto next : m_pSectionLinkBySectionName[sectionName]->names) {
        if ( !dfsDrawGraph(next) ) {
            return false;
        }
        //qDebug() << "Dfs draw graph: [" << sectionName << "]->[" << next << "]";
        if ( !pSect->addOutputEdge(m_pItemBySectionName[next]) ) {
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
    sectionLink* tmpLink = new sectionLink(sectionName, ESectionNext);
    m_pSectionLinkBySectionName.insert(sectionName, tmpLink);

    m_pDialogLinkBySectionName[sectionName] = new dialogLink();

	GraphicsSectionItem* newSect = new GraphicsSectionItem;
	newSect->setLabel(sectionName);
	newSect->setSectionLink(tmpLink);
	newSect->setYmlManager(this);
	newSect->setPos(pos);
    m_pScene->addItem( newSect );
	newSect->fillCleanSockets();

    m_pItemBySectionName[sectionName] = newSect;

    m_root["dialogscript"][sectionName] = YAML::Load("- NEXT: invalid_link");

	qInfo() << "ADD: " << sectionName;
}

void YmlSceneManager::renameSectionLink(QString sectionName, QString oldName) {
	// rename all entries!
	if (oldName != sectionName) {
        m_pSectionLinkBySectionName[sectionName] = m_pSectionLinkBySectionName[oldName];
        m_pSectionLinkBySectionName.remove(oldName);
        m_pItemBySectionName[sectionName] = m_pItemBySectionName[oldName];
        m_pItemBySectionName.remove(oldName);
        m_sectionNames.removeOne(oldName);
        m_sectionNames.append(sectionName);
        m_pDialogLinkBySectionName[sectionName] = m_pDialogLinkBySectionName[oldName];
        m_pDialogLinkBySectionName.remove(oldName);

        YAML::Node tempNode = m_root["dialogscript"];
		for (auto it = tempNode.begin(); it != tempNode.end(); ++it) {
			if (it->first.as<QString>() == oldName) {
				it->first = sectionName;
				break;
			}
		}

        if (m_root["dialogscript"][oldName]) {
            m_root["dialogscript"][sectionName] = YAML::Clone(m_root["dialogscript"][oldName]);
            m_root["dialogscript"].remove(oldName);
        } else if (m_root["storyboard"][oldName]) {
            m_root["storyboard"][sectionName] = YAML::Clone(m_root["storyboard"][oldName]);
            m_root["storyboard"].remove(oldName);
        }

        emit sectionNameChanged(oldName, sectionName);
	}
}

void YmlSceneManager::updateSectionLink(QString sectionName) {
    sectionLink* pLink = m_pSectionLinkBySectionName[sectionName];

    if (m_root["dialogscript"][sectionName]) {
        YAML::Node sNode = m_root["dialogscript"][sectionName];
        YAML::Node sNode2;
        switch ( pLink->type ) {
            case ESectionExit: {
				// simple EXIT scalar
				sNode2 = "EXIT";
				break;
			}
            case ESectionChoice: {
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
            case ESectionCondition: {
				YAML::Node tempMap;
                tempMap["condition"] = pLink->conditions[0];
				tempMap["condition"].SetStyle(YAML::EmitterStyle::Flow);
                tempMap["on_true"] = pLink->names[0];
                tempMap["on_false"] = pLink->names[1];
				sNode2["NEXT"] = tempMap;
				break;
			}
            case ESectionRandom: {
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
    delete m_pSectionLinkBySectionName[sectionName];
    m_pSectionLinkBySectionName.remove( sectionName );
    m_sectionNames.removeOne( sectionName );

    delete m_pDialogLinkBySectionName[sectionName];
    m_pDialogLinkBySectionName.remove(sectionName);

    GraphicsSectionItem* pItem = m_pItemBySectionName[sectionName];
    m_pScene->removeItem( pItem );
    m_pItemBySectionName.remove( sectionName );
    pItem->deleteLater();

    if (m_root["dialogscript"][sectionName]) {
        m_root["dialogscript"].remove( sectionName );
	}
    if (m_root["storyboard"][sectionName]) {
        m_root["storyboard"].remove( sectionName );
	}
    emit sectionDeleted(sectionName);
}

sectionLink* YmlSceneManager::getSectionLink(QString sectionName) {
    if ( m_pSectionLinkBySectionName.contains(sectionName) )
        return m_pSectionLinkBySectionName[sectionName];
	else
		return nullptr;
}

GraphicsSectionItem* YmlSceneManager::getSectionItem(QString sectionName) {
    if ( m_pItemBySectionName.contains(sectionName) )
        return m_pItemBySectionName[sectionName];
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

    int id = -1;
	cnt = 0;
	bool ok_id = false;
	upn(i, 0, line.length() - 1) {
		QChar c = line[i];
		if (c.isDigit()) {
            id = id * 10 + c.digitValue();
			++cnt;
			if (cnt == 10 && i+1 < line.length() && line[i + 1] == '|') {
				ok_id = true;
				break;
			}
		} else {
			cnt = 0;
            id = -1;
		}
	}
    if (ok_id && m_lineByID.contains(id) && m_lineByID[id].duration > 0.0) {
        return m_lineByID[id].duration;
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
    if (m_root["dialogscript"][sectionName]) {
        YAML::Node sectionSeq = YAML::Node(YAML::NodeType::Sequence);

        upn(i, 0, m_pDialogLinkBySectionName[sectionName]->lines.count() - 1) {
            dialogLink* pDialogLink = m_pDialogLinkBySectionName[sectionName];
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
        sectionSeq.push_back( lastCloned(m_root["dialogscript"][sectionName]) );
        m_root["dialogscript"][sectionName] = sectionSeq;
    } else {
        error("ERROR: section [" + sectionName + "] not found for updateDialogscriptSection!");
    }
}

void YmlSceneManager::updateShot(QString sectionName, QString shotName)
{
    int shotNum = m_pDialogLinkBySectionName[sectionName]->shotNumByName(shotName);
    updateShot(sectionName, shotNum);
}

void YmlSceneManager::updateShot(QString sectionName, int shotNum)
{
    QString shotName = m_pDialogLinkBySectionName[sectionName]->shots[shotNum].shotName;
    int actionsCount = m_pDialogLinkBySectionName[sectionName]->shots[shotNum].actions.count();

    // remove shot if no actions, remove sb section if no shots
    if (actionsCount == 0 && m_root["storyboard"][sectionName][shotName]) {
        m_root["storyboard"][sectionName].remove(shotName);
        if (m_root["storyboard"][sectionName].size() == 0) {
            m_root["storyboard"].remove(sectionName);
        }
        // update dg section to remove CUE for empty shots
        updateDialogscriptSection(sectionName);
    }

    // create shot if actions are added
    if (actionsCount > 0) {
        // create sb section if shot is going to be added
        if (!m_root["storyboard"][sectionName]) {
            m_root["storyboard"][sectionName] = YAML::Node(YAML::NodeType::Map);
        }
        // create sb shot if actions are added
        if (!m_root["storyboard"][sectionName][shotName]) {
            m_root["storyboard"][sectionName][shotName] = YAML::Node(YAML::NodeType::Map);
            // update dg section to add CUE for shot
            updateDialogscriptSection(sectionName);
        }
        YAML::Node shotSeq(YAML::NodeType::Sequence);
        shotSeq.SetStyle(YAML::EmitterStyle::Block);
        m_pDialogLinkBySectionName[sectionName]->shots[shotNum].sortActionsByStart();
        upn(i, 0, actionsCount - 1) {
            SA_Base* sa = m_pDialogLinkBySectionName[sectionName]->shots[shotNum].actions[i];
            shotSeq.push_back( sa->ymlNode() );
        }
        m_root["storyboard"][sectionName][shotName] = shotSeq;
    }
}

void YmlSceneManager::removeShot(QString sectionName, QString shotName)
{
    int shotNum = m_pDialogLinkBySectionName[sectionName]->shotNumByName(shotName);

    // remove dialog info
    m_pDialogLinkBySectionName[sectionName]->shots.removeAt(shotNum);
    m_pDialogLinkBySectionName[sectionName]->durations.removeAt(shotNum);
    m_pDialogLinkBySectionName[sectionName]->lines.removeAt(shotNum);
    m_pDialogLinkBySectionName[sectionName]->speakers.removeAt(shotNum);

    // remove shot from storyboard section
    if (m_root["storyboard"][sectionName][shotName]) {
        m_root["storyboard"][sectionName].remove(shotName);
        // remove section if empty
        if (m_root["storyboard"][sectionName].size() == 0) {
            m_root["storyboard"].remove(sectionName);
        }
    }

    // update dg section to remove actor line and CUE
    updateDialogscriptSection(sectionName);
}

void YmlSceneManager::addShot(QString sectionName, int shotNum)
{
    dialogLink* pDialogLink = m_pDialogLinkBySectionName[sectionName];

    info( QString("onShotAdd: in %1, at [%2]").arg(sectionName).arg(shotNum) );
    int shotNameNumber = 0;
    while (pDialogLink->shotNames.contains("shot_" + qn(shotNameNumber) )) {
        ++shotNameNumber;
    }
    QString shotName = "shot_" + qn(shotNameNumber);

    pDialogLink->shotNames.insert( shotName );

    pDialogLink->durations.insert( shotNum, 5.0 );
    pDialogLink->lines.insert( shotNum, "PAUSE" );
    pDialogLink->speakers.insert( shotNum, sceneGlobals()->getID(ERepoAssets, "PAUSE") );
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

template<typename Container>
void YmlSceneManager::deletePointersFromContainer(Container& container)
{
    for (auto it : container) {
        //qDebug() << "deletePointersFromContainer: " << typeid(it).name();
        delete it;
    }
}
