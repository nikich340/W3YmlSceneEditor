#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QtWidgets>
#include "constants.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	ui->actionShowhideLog->setChecked( readSetting("logFieldVisible", true).toBool() );
	ui->infoField->setVisible( readSetting("logFieldVisible", true).toBool() );

    gScene = new QGraphicsScene(this);
	gScene->setSceneRect(0,0, SCENE_WIDTH, SCENE_HEIGHT);

	gDgScene = new QGraphicsScene(this);
	gDgScene->setSceneRect(0,0, SHOT_SCENE_WIDTH, SHOT_DG_HEIGHT);

	gShotScene = new QGraphicsScene(this);
	gShotScene->setSceneRect(0,0, SHOT_SCENE_WIDTH, SHOT_SCENE_HEIGHT);

	gLabelScene = new QGraphicsScene(this);
	gLabelScene->setSceneRect(0,0, SHOT_LABEL_WIDTH, SHOT_SCENE_HEIGHT);

	ymlManager = new YmlSceneManager(this, gScene);
	ymlManager->setShotScenes(gDgScene, gLabelScene, gShotScene);

	ui->gView->setScene(gScene);
	ui->gViewDgEditor->setScene(gDgScene);
	ui->gViewDgEditor->setMaximumWidth(SHOT_SCENE_WIDTH);
	ui->gViewShotEditor->setScene(gShotScene);
	ui->gViewShotEditor->setMaximumWidth(SHOT_SCENE_WIDTH);
	ui->gViewLabel->setScene(gLabelScene);
	ui->gView->setYmlManager(ymlManager);
	//ui->gViewShotEditor->setYmlManager(ymlManager);
	ui->gViewShotEditor->setChildViews(ui->gViewLabel, ui->gViewDgEditor);

	shotManager = new ShotManager(ymlManager, this);
	shotManager->setShotScenes(gDgScene, gLabelScene, gShotScene);

	QLinearGradient gradient(0, 0, 987, SCENE_HEIGHT);
	gradient.setColorAt(0, colorSceneGradient0);
	gradient.setColorAt(1.0, colorSceneGradient1);
    gScene->setBackgroundBrush(gradient);
	gDgScene->setBackgroundBrush(gradient);
	gShotScene->setBackgroundBrush(gradient);
	gLabelScene->setBackgroundBrush(gradient);

    connect(ymlManager, SIGNAL(print_info(QString)), this, SLOT(print_info(QString)));
	connect(ymlManager, SIGNAL(print_error(QString)), this, SLOT(print_error(QString)));
	connect(ymlManager, SIGNAL(print_warning(QString)), this, SLOT(print_warning(QString)));
	connect(ymlManager, SIGNAL(loadShots(QString)), shotManager, SLOT(onLoadShots(QString)));
    //connect(ui->gViewShotEditor, SIGNAL(wasScaled(double)), shotManager, SLOT(onScaledView(double)));
    connect(ui->gViewShotEditor, SIGNAL(addActionIntent(QPointF p)), shotManager, SLOT(OnAddActionIntent(QPointF)));

	connect(ui->repoActorsButton, SIGNAL(clicked()), this, SLOT(onClicked_RepoActors()));

	connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(onClicked_Quit()));
    connect(ui->actionLoad, SIGNAL(triggered()), this, SLOT(onClicked_Load()));
	connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(onClicked_Save()));
	connect(ui->actionShowhideLog, SIGNAL(triggered()), this, SLOT(onClicked_ShowhideLog()));
	connect(ui->actionSetRepoPath, SIGNAL(triggered()), this, SLOT(onClicked_SetRepoPath()));

	readSceneRepos();
}

void MainWindow::readSceneRepos() {
	QString dirPath = readSetting("sceneReposPath").toString();
	if (dirPath.isEmpty()) {
		print_warning("readSceneRepos(): Radish scene.repos path is not set!");
		return;
	}

	QDir dir(dirPath);
	if (!dir.exists()) {
		print_error("readSceneRepos(): Folder " + dirPath + " does not exist!");
	}
	QStringList files = dir.entryList(QStringList() << "*.yml" << "*.yaml" << "*.YML" << "*.YAML", QDir::Files);

	int ok_cnt = 0;
	for (QString filePath : files) {
		/*QMessageBox box(QMessageBox::Warning, "Test", "Loading: " + filePath, QMessageBox::Ok);
		box.setModal(true);
		box.exec();*/
		if ( !ymlManager->loadYmlRepo(dirPath + "/" + filePath) ) {
			print_warning("readSceneRepos(): Can't read repo yml file: " + dirPath + "/" + filePath);
		} else {
			++ok_cnt;
		}
	}
	if (ok_cnt > 0) {
		print_info("readSceneRepos(): Successfully loaded " + qn(ok_cnt) + " yml repo files.");
	}
}

void MainWindow::print_info(QString s) {
	QString curS = ui->infoField->toHtml();
	ui->infoField->setHtml(curS + "<font color=\"DarkBlue\">" + s + "</font>\n");
	ui->infoField->verticalScrollBar()->setValue( ui->infoField->verticalScrollBar()->maximum() );
}
void MainWindow::print_warning(QString s) {
	QString curS = ui->infoField->toHtml();
	ui->infoField->setHtml(curS + "<font color=\"OrangeRed\">" + s + "</font>\n");
	ui->infoField->verticalScrollBar()->setValue( ui->infoField->verticalScrollBar()->maximum() );
}
void MainWindow::print_error(QString s) {
	QString curS = ui->infoField->toHtml();
	ui->infoField->setHtml(curS + "<font color=\"Crimson\"><b>" + s + "</b></font>\n");
	ui->infoField->verticalScrollBar()->setValue( ui->infoField->verticalScrollBar()->maximum() );
}
void MainWindow::onClicked_Quit()
{
    QApplication::quit();
}
void MainWindow::onClicked_Load()
{
	if ( ymlManager->hasChanges && ymlManager->requestSave() ) {
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
	gScene->clear();
    if ( ymlManager->loadYmlFile(fileName) ) {
        setWindowTitle("Radish YML Scene Editor [" + fileName + "]");
    } else {
        print_error("Failed to load YML!");
    }


	if ( ymlManager->drawSectionsGraph() ) {
		print_info("Sections graph was successfully drawn in " + QString::number(timer.elapsed()) + " ms.");
    } else {
        print_error("Failed to draw sections: INCORRECT YML!");
    }
}
void MainWindow::onClicked_Save()
{
	QElapsedTimer timer;
	timer.start();
    if ( ymlManager->saveYmlFile() ) {
		print_info("Successfully saved in " + qn(timer.elapsed()) + " ms.");
    } else {
		print_error("Failed to save yml!");
    }
}
void MainWindow::onClicked_ShowhideLog() {
	/*if (ui->infoField->isHidden()) {
		ui->infoField->show();
	} else {
		ui->infoField->hide();
	}*/
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
		QMessageBox box(QMessageBox::Warning, "Warning", "Repository files will be loaded on next program launch.", QMessageBox::Ok);
		box.setModal(true);
		box.exec();
	}
}

void MainWindow::onClicked_RepoActors() {
	if (ymlManager->hasChanges || ymlManager->hasShotChanges) {
		ymlManager->showError("Can't change actors repository: firstly save current shot/section changes!");
		return;
	}
	RepoActorsDialog actorsRepo(ymlManager);
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

void MainWindow::resizeEvent(QResizeEvent* event)
{
    //ui->gView->setMinimumSize(QSize(event->size().width() / 3, 0));
    QMainWindow::resizeEvent(event);
}

MainWindow::~MainWindow()
{
    delete ui;
}

