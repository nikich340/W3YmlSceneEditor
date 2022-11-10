#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QtWidgets>
#include "UI/SpinSliderConnector.h"
#include "UI/WidgetsCheckController.h"
#include "constants.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QElapsedTimer timer;
    timer.start();
    ui->setupUi(this);
	ui->actionShowhideLog->setChecked( readSetting("logFieldVisible", true).toBool() );
	ui->infoField->setVisible( readSetting("logFieldVisible", true).toBool() );

    m_pSectionsScene = new QGraphicsScene(this);
    m_pSectionsScene->setSceneRect(0,0, CONSTANTS::SCENE_WIDTH, CONSTANTS::SCENE_HEIGHT);
    m_pSectionsScene->setItemIndexMethod(QGraphicsScene::NoIndex);

    m_pDialogsScene = new QGraphicsScene(this);
    ui->gViewShotDialogs->setScene(m_pDialogsScene);
    ui->gViewShotDialogs->viewport()->setMouseTracking(true);
    m_pDialogsScene->installEventFilter(ui->gScrollShot);
    m_pDialogsScene->setItemIndexMethod(QGraphicsScene::NoIndex);

    m_pYmlManager = new YmlSceneManager(this, m_pSectionsScene);
    ui->gView->setScene(m_pSectionsScene);
    ui->gView->setYmlManager(m_pYmlManager);
    ui->gScrollShot->setChildViews(ui->gScrollShotLabel, ui->gViewShotDialogs);

    m_pShotManager = new ShotManager(m_pYmlManager, this);
    m_pShotManager->setWidgets(m_pDialogsScene, ui->shotEditorStackedWidget, ui->gScrollShotLabel, ui->gScrollShot);

    QLinearGradient gradient(0, 0, CONSTANTS::SCENE_WIDTH, CONSTANTS::SCENE_HEIGHT);
    gradient.setColorAt(0, CONSTANTS::colorSceneGradient0);
    gradient.setColorAt(1.0, CONSTANTS::colorSceneGradient1);
    m_pSectionsScene->setBackgroundBrush(gradient);
    m_pDialogsScene->setBackgroundBrush(gradient);

    m_pDialogSelectEnv = new DialogSelectEnv();
    m_pDialogSelectEnv->setModal(true);
    for (int SA_Num = EShotUnknown; ; ++SA_Num) {
        EShotActionType SA_Enum = static_cast<EShotActionType>(SA_Num);
        QWidget* pWidget = new QWidget();
        ui->shotEditorStackedWidget->addWidget(pWidget);
        setupSA_PageWidget(SA_Enum, pWidget);
        pWidget->setEnabled(false);
        if (SA_Num == EShotMAX)
            break;
    }
    ui->shotEditorStackedWidget->setCurrentIndex((int)EShotCamBlendStart);

    connect(m_pYmlManager, SIGNAL(print_info(QString)), this, SLOT(print_info(QString)));
    connect(m_pYmlManager, SIGNAL(print_error(QString)), this, SLOT(print_error(QString)));
    connect(m_pYmlManager, SIGNAL(print_warning(QString)), this, SLOT(print_warning(QString)));
    connect(m_pYmlManager, SIGNAL(sectionTypeChanged(QString,int)), m_pShotManager, SLOT(onUpdateSectionType(QString,int)));
    connect(m_pYmlManager, SIGNAL(sectionNameChanged(QString,QString)), m_pShotManager, SLOT(onUpdateSectionName(QString,QString)));
    connect(m_pYmlManager, SIGNAL(sectionLoaded(QString)), m_pShotManager, SLOT(onLoadSectionShots(QString)));
    connect(m_pYmlManager, SIGNAL(sectionDeleted(QString)), m_pShotManager, SLOT(onClearEditor()));
    connect(m_pYmlManager, SIGNAL(ymlFileLoaded(QString)), m_pShotManager, SLOT(onClearEditor()));
    //connect(ui->gViewShotEditor, SIGNAL(wasScaled(double)), shotManager, SLOT(onScaledView(double)));
    //connect(ui->gViewShotEditor, SIGNAL(addActionIntent(QPointF p)), shotManager, SLOT(OnAddActionIntent(QPointF)));
    connect(ui->gScrollShot, SIGNAL(lineMoveEvent(QPointF)), m_pShotManager, SLOT(onNavigationLineMove(QPointF)));
    connect(ui->gScrollShot, SIGNAL(contextEvent(QGraphicsScene*,QPoint,QPointF)), m_pShotManager, SLOT(onSceneContextEvent(QGraphicsScene*,QPoint,QPointF)));
    connect(ui->repoActorsButton, SIGNAL(clicked()), this, SLOT(onClicked_RepoActors()));

	connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(onClicked_Quit()));
    connect(ui->actionLoad, SIGNAL(triggered()), this, SLOT(onClicked_Load()));
	connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(onClicked_Save()));
	connect(ui->actionShowhideLog, SIGNAL(triggered()), this, SLOT(onClicked_ShowhideLog()));
	connect(ui->actionSetRepoPath, SIGNAL(triggered()), this, SLOT(onClicked_SetRepoPath()));

    print_info(QString("MainWindow(): UI Loaded in %1 ms.").arg(timer.elapsed()));
}

void MainWindow::loadYmlRepos() {
	QString dirPath = readSetting("sceneReposPath").toString();
	if (dirPath.isEmpty()) {
		print_warning("readSceneRepos(): Radish scene.repos path is not set!");
		return;
	}

	QDir dir(dirPath);
	if (!dir.exists()) {
		print_error("readSceneRepos(): Folder " + dirPath + " does not exist!");
        return;
	}
    QElapsedTimer timer;
    timer.start();
	QStringList files = dir.entryList(QStringList() << "*.yml" << "*.yaml" << "*.YML" << "*.YAML", QDir::Files);

	int ok_cnt = 0;
	for (QString filePath : files) {
		/*QMessageBox box(QMessageBox::Warning, "Test", "Loading: " + filePath, QMessageBox::Ok);
		box.setModal(true);
		box.exec();*/
        if ( !m_pYmlManager->loadYmlRepo(dirPath + "/" + filePath) ) {
			print_warning("readSceneRepos(): Can't read repo yml file: " + dirPath + "/" + filePath);
		} else {
			++ok_cnt;
		}
	}
	if (ok_cnt > 0) {
        print_info(QString("readSceneRepos(): Successfully loaded %1 yml repo files in %2 ms.").arg(ok_cnt).arg(timer.elapsed()));
    }
}

void MainWindow::loadCsvFiles()
{
    /* check w3.data dir */
    QDir w3dataDir(QCoreApplication::applicationDirPath() + "/w3.data");
    if (!w3dataDir.exists()) {
        print_error(QCoreApplication::applicationDirPath() + "/w3.data folder does not exist!");
        return;
    }
    m_pYmlManager->loadCsvLines();
    m_pYmlManager->loadEnvPaths();
    m_pDialogSelectEnv->setStorage(&m_pYmlManager->m_envStorage);
}

void MainWindow::addPrintLog(QString lineColor, QString line)
{
    ++m_logLinesCnt;
    if (m_logLinesCnt > 500) {
        m_logLinesCnt = 100;
        m_log = m_log.split("\n").mid(0, 100).join("\n");
    }
    m_log += QString("<font color=\"%2\">%3</font><br>").arg(lineColor).arg(line);
    ui->infoField->setHtml(m_log);
    ui->infoField->verticalScrollBar()->setValue( ui->infoField->verticalScrollBar()->maximum() );
}

void MainWindow::print_info(QString s) {
    qi << s;
    addPrintLog("DarkBlue", s);
}

void MainWindow::print_warning(QString s) {
    qw << s;
    addPrintLog("OrangeRed", s);
}

void MainWindow::print_error(QString s) {
    qc << s;
    addPrintLog("Crimson", s);
}

void MainWindow::onClicked_Quit()
{
    QApplication::quit();
}

void MainWindow::onClicked_Load()
{
    if ( m_pYmlManager->hasChanges && m_pYmlManager->requestSave() ) {
		return;
	}

    QString fileName = QFileDialog::getOpenFileName(this,
                       tr("Open scene yml"), "", tr("Yaml Files (*.yml *.yaml)"));
	if (fileName.isEmpty()) {
		qInfo() << "Loading file canceled by user";
		return;
	}

	QElapsedTimer timer;
	timer.start();
    if ( m_pYmlManager->loadYmlFile(fileName) ) {
        setWindowTitle("Radish YML Scene Editor [" + fileName + "]");
    } else {
        print_error("Failed to load YML!");
    }


    if ( m_pYmlManager->drawSectionsGraph() ) {
        print_info("Sections graph painted in: " + QString::number(timer.elapsed()) + " ms.");
    } else {
        print_error("Failed to draw sections: INCORRECT YML!");
    }
}

void MainWindow::onClicked_Save()
{
	QElapsedTimer timer;
	timer.start();
    if ( m_pYmlManager->saveYmlFile() ) {
		print_info("Successfully saved in " + qn(timer.elapsed()) + " ms.");
    } else {
		print_error("Failed to save yml!");
    }
}

void MainWindow::onClicked_ShowhideLog() {
	bool isVisible = !ui->infoField->isVisible();
	writeSetting("logFieldVisible", isVisible);
	ui->infoField->setVisible( isVisible );
	ui->actionShowhideLog->setChecked( isVisible );
}

void MainWindow::onClicked_SetRepoPath() {
	QString folderName = QFileDialog::getExistingDirectory( this, tr("Select radish repo.scenes folder") );
	if (folderName.isEmpty()) {
		qi << "Selecting repo.scenes canceled by user";
		return;
	} else {
		/*QDir directory(folderName);
		if ( !directory.exists() ) {

		}
		QStringList ymls = directory.entryList(QStringList() << "*.jpg" << "*.JPG",QDir::Files);*/
		writeSetting("sceneReposPath", folderName);
		print_info("Selected radish repo.scenes folder: " + folderName);
        //QMessageBox box(QMessageBox::Warning, "Warning", "Repository files will be loaded on next program launch.", QMessageBox::Ok);
        //box.setModal(true);
        //box.exec();
        m_pYmlManager->clearData(true);
        loadYmlRepos();
    }
}

void MainWindow::onClicked_RepoActors() {
    if (m_pYmlManager->hasChanges) {
        m_pYmlManager->showError("Can't change actors repository: first save current section changes!");
		return;
	}
    RepoActorsDialog actorsRepo(m_pYmlManager);
	actorsRepo.setModal(true);
	int result = actorsRepo.exec();
	qd << result;
}

void MainWindow::writeSetting(QString name, QVariant value) {
	QSettings settings("Radish YML Scene Editor");
	settings.setValue(name, value);
}
QVariant MainWindow::readSetting(QString name, QVariant defaultValue) {
	QSettings settings("Radish YML Scene Editor");
	if (settings.contains(name))
		return settings.value(name);
	else
        return defaultValue;
}

void MainWindow::setupSA_PageWidget(const EShotActionType SA_Type, QWidget* pWidget)
{
    if (pWidget == nullptr) {
        qc << QString("%1: pWidget is NULL.").arg(Q_FUNC_INFO);
        return;
    }
    if (m_pShotManager == nullptr) {
        qc << QString("%1: m_pShotManager is NULL.").arg(Q_FUNC_INFO);
        return;
    }
    QString typeName = CONSTANTS::EShotActionToString[SA_Type];
    QGridLayout* pLayout = new QGridLayout(pWidget);
    QLabel* pDescriptionLabel = new QLabel();
    pDescriptionLabel->setText( QString("<b>Description</b>: <i>%1</i>").arg(SA_Base::shortDescription(SA_Type)) );
    //pDescriptionLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    //pDescriptionLabel->setWordWrap(true);
    pLayout->setContentsMargins(6, 6, 6, 6);
    pLayout->addWidget(new QLabel( QString("<font color=%1>[%2]</font>").arg(ShotManager::colorForActionType(SA_Type).darker(175).name()).arg(typeName) ), pLayout->rowCount()-1,0, 1,1);
    pLayout->addWidget(pDescriptionLabel, pLayout->rowCount()-1,1, 1,2);
    typeName.replace(".", "_");
    pWidget->setObjectName("page_" + typeName);

    QDoubleSpinBox* pStartSpin = new QDoubleSpinBox();
    pStartSpin->setMinimum(0.0);
    pStartSpin->setMaximum(0.999);
    pStartSpin->setDecimals(3);
    pStartSpin->setSingleStep(0.001);
    pStartSpin->setObjectName("m_startSpin");
    QSlider* pStartSlider = new QSlider(Qt::Horizontal);
    pStartSlider->setObjectName("m_startSlider");
    pStartSlider->setRange(0, 999);
    pStartSlider->setTickPosition(QSlider::TicksBelow);
    pStartSlider->setTickInterval(100);
    pLayout->addWidget(new QLabel("Action start: "), pLayout->rowCount(),0, 1,1);
    pLayout->addWidget(pStartSlider, pLayout->rowCount()-1,1, 1,1);
    pLayout->addWidget(pStartSpin, pLayout->rowCount()-1,2, 1,1);
    pLayout->setColumnStretch(1, 100); // alows value thing to expand
    SpinSliderConnector* pStartTimeConnector = new SpinSliderConnector(pWidget);
    pStartTimeConnector->setDoubleSpinSlider(pStartSpin, pStartSlider, 1000);
    connect(pStartTimeConnector, SIGNAL(valueChanged(double)), m_pShotManager, SLOT(onShotActionStartChanged(double)));
    switch (SA_Type) {
        // SHARED
        case EShotCam:
        case EShotCamBlendKey:
        case EShotCamBlendTogame:
        {
            QLabel* pCamLabel = new QLabel("Camera name: ");
            QComboBox* pCamNamesBox = new QComboBox();
            QPushButton* pCamButton = new QPushButton("Edit");
            pCamNamesBox->setObjectName("v_camName");
            pLayout->addWidget(pCamLabel, pLayout->rowCount(),0, 1,1);
            pLayout->addWidget(pCamNamesBox, pLayout->rowCount()-1,1, 1,1);
            pLayout->addWidget(pCamButton, pLayout->rowCount()-1,2, 1,1);
            // TODO! connect(pCamButton, SIGNAL(clicked(bool)), m_pDialogEditCamera, SLOT(open()));
            // TODO! m_pDialogEditCamera->addBoxForUpdates(pCamNamesBox); -> fills camera names
            connect(pCamNamesBox, SIGNAL(currentIndexChanged(int)), m_pShotManager, SLOT(onShotActionChanged()));
            break;
        }
        case EShotCamBlendStart:
        case EShotCamBlendEnd:
        {
            QLabel* pCamLabel = new QLabel("Camera name: ");
            QComboBox* pCamNamesBox = new QComboBox();
            QPushButton* pCamButton = new QPushButton("Edit");
            pCamNamesBox->setObjectName("v_camName");
            pLayout->addWidget(pCamLabel, pLayout->rowCount(),0, 1,1);
            pLayout->addWidget(pCamNamesBox, pLayout->rowCount()-1,1, 1,1);
            pLayout->addWidget(pCamButton, pLayout->rowCount()-1,2, 1,1);
            // TODO! connect(pCamButton, SIGNAL(clicked(bool)), m_pDialogEditCamera, SLOT(open()));
            // TODO! m_pDialogEditCamera->addBoxForUpdates(pCamNamesBox); -> fills camera names
            connect(pCamNamesBox, SIGNAL(currentIndexChanged(int)), m_pShotManager, SLOT(onShotActionChanged()));

            QCheckBox* pEaseCheck = new QCheckBox("Camera ease: ");
            QComboBox* pEaseBox = new QComboBox();
            pEaseBox->setObjectName("v_camEase");
            pEaseBox->addItems({"smooth", "rapid"});
            pLayout->addWidget(pEaseCheck, pLayout->rowCount(),0, 1,1);
            pLayout->addWidget(pEaseBox, pLayout->rowCount()-1,1, 1,1);
            WidgetsCheckController* pEaseController = new WidgetsCheckController(pWidget);
            pEaseController->setObjectName("v_camEaseCheck");
            pEaseController->setCheckbox(pEaseCheck);
            pEaseController->addWidget(pEaseBox);
            connect(pEaseController, SIGNAL(clicked(bool)), m_pShotManager, SLOT(onShotActionChanged()));
            connect(pEaseBox, SIGNAL(currentIndexChanged(int)), m_pShotManager, SLOT(onShotActionChanged()));
            break;
        }
        case EShotEnvBlendIn:
        case EShotEnvBlendOut:
        {
            QLabel* pEnvLabel = new QLabel("Env path: ");
            QLineEdit* pEnvPathLine = new QLineEdit();
            QPushButton* pEnvPathButton = new QPushButton("Select");
            pEnvPathLine->setObjectName("v_envPath");
            pLayout->addWidget(pEnvLabel, pLayout->rowCount(),0, 1,1);
            pLayout->addWidget(pEnvPathLine, pLayout->rowCount()-1,1, 1,1);
            pLayout->addWidget(pEnvPathButton, pLayout->rowCount()-1,2, 1,1);
            connect(pEnvPathLine, SIGNAL(textChanged(QString)), m_pShotManager, SLOT(onShotActionChanged()));
            connect(pEnvPathButton, SIGNAL(clicked(bool)), m_pDialogSelectEnv, SLOT(exec()));
            connect(m_pDialogSelectEnv, SIGNAL(envSelected(QString)), pEnvPathLine, SLOT(setText(QString)));
            // button click -> env dialog open -> click OK -> emit new path to line -> onShotActionChanged()

            QCheckBox* pBlendTimeCheck = new QCheckBox("Blend time: ");
            QSlider* pBlendTimeSlider = new QSlider(Qt::Horizontal);
            pBlendTimeSlider->setRange(0, 10 * 10);
            QDoubleSpinBox* pBlendTimeSpin = new QDoubleSpinBox();
            pBlendTimeSpin->setObjectName("v_blendTime");
            pBlendTimeSpin->setDecimals(3);
            pBlendTimeSpin->setRange(0.0, 1000.0);
            pBlendTimeSpin->setSingleStep(0.001);
            pLayout->addWidget(pBlendTimeCheck, pLayout->rowCount(),0, 1,1);
            pLayout->addWidget(pBlendTimeSlider, pLayout->rowCount()-1,1, 1,1);
            pLayout->addWidget(pBlendTimeSpin, pLayout->rowCount()-1,2, 1,1);
            SpinSliderConnector* pBlendTimeConnector = new SpinSliderConnector(pWidget);
            pBlendTimeConnector->setDoubleSpinSlider(pBlendTimeSpin, pBlendTimeSlider, 10);
            connect(pBlendTimeConnector, SIGNAL(valueChanged(double)), m_pShotManager, SLOT(onShotActionChanged()));
            WidgetsCheckController* pBlendTimeController = new WidgetsCheckController(pWidget);
            pBlendTimeController->setObjectName("v_blendTimeCheck");
            pBlendTimeController->setCheckbox(pBlendTimeCheck);
            pBlendTimeController->addWidget(pBlendTimeSpin);
            connect(pBlendTimeController, SIGNAL(clicked(bool)), m_pShotManager, SLOT(onShotActionChanged()));

            QCheckBox* pBlendFactorCheck = new QCheckBox("Blend factor: ");
            QSlider* pBlendFactorSlider = new QSlider(Qt::Horizontal);
            pBlendFactorSlider->setRange(0, 1 * 100);
            QDoubleSpinBox* pBlendFactorSpin = new QDoubleSpinBox();
            pBlendFactorSpin->setObjectName("v_blendFactor");
            pBlendFactorSpin->setDecimals(3);
            pBlendFactorSpin->setRange(0.0, 1.0);
            pBlendFactorSpin->setSingleStep(0.001);
            pLayout->addWidget(pBlendFactorCheck, pLayout->rowCount(),0, 1,1);
            pLayout->addWidget(pBlendFactorSlider, pLayout->rowCount()-1,1, 1,1);
            pLayout->addWidget(pBlendFactorSpin, pLayout->rowCount()-1,2, 1,1);
            SpinSliderConnector* pBlendFactorConnector = new SpinSliderConnector(pWidget);
            pBlendFactorConnector->setDoubleSpinSlider(pBlendFactorSpin, pBlendFactorSlider, 100);
            connect(pBlendFactorConnector, SIGNAL(valueChanged(double)), m_pShotManager, SLOT(onShotActionChanged()));
            WidgetsCheckController* pBlendFactorController = new WidgetsCheckController(pWidget);
            pBlendFactorController->setObjectName("v_blendFactorCheck");
            pBlendFactorController->setCheckbox(pBlendFactorCheck);
            pBlendFactorController->addWidget(pBlendFactorSpin);
            connect(pBlendFactorController, SIGNAL(clicked(bool)), m_pShotManager, SLOT(onShotActionChanged()));

            QCheckBox* pPriorityCheck = new QCheckBox("Env priority: ");
            QSpinBox* pPrioritySpin = new QSpinBox();
            pPrioritySpin->setObjectName("v_priority");
            pPrioritySpin->setRange(0, 1000000);
            pLayout->addWidget(pPriorityCheck, pLayout->rowCount(),0, 1,1);
            pLayout->addWidget(pPrioritySpin, pLayout->rowCount()-1,2, 1,1);
            connect(pPrioritySpin, SIGNAL(valueChanged(int)), m_pShotManager, SLOT(onShotActionChanged()));
            WidgetsCheckController* pPriorityController = new WidgetsCheckController(pWidget);
            pPriorityController->setObjectName("v_priorityCheck");
            pPriorityController->setCheckbox(pPriorityCheck);
            pPriorityController->addWidget(pPrioritySpin);
            connect(pPriorityController, SIGNAL(clicked(bool)), m_pShotManager, SLOT(onShotActionChanged()));
            break;
        }
        case EShotFadeIn:
        case EShotFadeOut: {
            QLabel* pDurationLabel = new QLabel("Fade duration: ");
            QSlider* pDurationSlider = new QSlider(Qt::Horizontal);
            pDurationSlider->setRange(0, 10 * 10);
            QDoubleSpinBox* pDurationSpin = new QDoubleSpinBox();
            pDurationSpin->setObjectName("v_duration");
            pDurationSpin->setRange(0.0, 1000.0);
            pDurationSpin->setDecimals(3);
            pDurationSpin->setSingleStep(0.001);
            pLayout->addWidget(pDurationLabel, pLayout->rowCount(),0, 1,1);
            pLayout->addWidget(pDurationSlider, pLayout->rowCount()-1,1, 1,1);
            pLayout->addWidget(pDurationSpin, pLayout->rowCount()-1,2, 1,1);
            SpinSliderConnector* pDurationConnector = new SpinSliderConnector(pWidget);
            pDurationConnector->setDoubleSpinSlider(pDurationSpin, pDurationSlider, 10);
            connect(pDurationConnector, SIGNAL(valueChanged(double)), m_pShotManager, SLOT(onShotActionChanged()));

            QCheckBox* pColorCheck = new QCheckBox("Screen color: ");
            QLabel* pColorLabel = new QLabel();
            pColorLabel->setObjectName("v_color");
            pColorLabel->setAutoFillBackground(true);
            QPushButton* pColorButton = new QPushButton("Select");
            pLayout->addWidget(pColorCheck, pLayout->rowCount(),0, 1,1);
            pLayout->addWidget(pColorLabel, pLayout->rowCount()-1,1, 1,1);
            pLayout->addWidget(pColorButton, pLayout->rowCount()-1,2, 1,1);
            WidgetsCheckController* pColorController = new WidgetsCheckController(pWidget);
            pColorController->setObjectName("v_colorCheck");
            pColorController->setCheckbox(pColorCheck);
            pColorController->addWidget(pColorLabel);
            pColorController->addWidget(pColorButton);
            connect(pColorController, SIGNAL(clicked(bool)), m_pShotManager, SLOT(onShotActionChanged()));
            connect(pColorButton, &QAbstractButton::clicked, this, [pColorLabel, this]() {
                QColor color = QColorDialog::getColor(Qt::black, this, "Select screen color", QColorDialog::ShowAlphaChannel);
                if (color.isValid()) {
                    QPalette palette = pColorLabel->palette();
                    palette.setColor(QPalette::Window, color);
                    pColorLabel->setPalette(palette);
                    m_pShotManager->onShotActionChanged();
                }
            });
            break;
        }
        case EShotWorldAddfact:{
            break;
        }
        case EShotWorldWeather:

            break;
        case EShotWorldEffectStart:

            break;
        case EShotWorldEffectStop:

            break;
        // ACTOR
        case EShotActorAnim:

            break;
        case EShotActorAnimAdditive:

            break;
        case EShotActorAnimPose:

            break;
        case EShotActorMimicAnim:

            break;
        case EShotActorMimicPose:

            break;
        case EShotActorPlacement:

            break;
        case EShotActorPlacementStart:

            break;
        case EShotActorPlacementKey:

            break;
        case EShotActorPlacementEnd:

            break;
        case EShotActorGamestate:

            break;
        case EShotActorLookat:

            break;
        case EShotActorScabbardShow:

            break;
        case EShotActorScabbardHide:

            break;
        case EShotActorEffectStart:

            break;
        case EShotActorEffectStop:

            break;
        case EShotActorSound:

            break;
        case EShotActorAppearance:

            break;
        case EShotActorEquipRight:

            break;
        case EShotActorEquipLeft:

            break;
        case EShotActorUnequipRight:

            break;
        case EShotActorUnequipLeft:

            break;
        // PROP
        case EShotPropShow:

            break;
        case EShotPropHide:

            break;
        case EShotPropPlacement:

            break;
        case EShotPropPlacementStart:

            break;
        case EShotPropPlacementKey:

            break;
        case EShotPropPlacementEnd:

            break;
        case EShotPropEffectStart:

            break;
        case EShotPropEffectStop:

            break;
        default:
           qc << QString("%1: unknown type = %2").arg(Q_FUNC_INFO).arg(CONSTANTS::EShotActionToString[SA_Type]);
           break;
    }
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    //ui->gView->setMinimumSize(QSize(event->size().width() / 3, 0));
    QMainWindow::resizeEvent(event);
}

MainWindow::~MainWindow()
{
    delete ui;
}

