#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QtWidgets>
#include "constants.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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
	ui->gViewShotEditor->setScene(gShotScene);
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
	connect(ymlManager, SIGNAL(loadShots(QString)), shotManager, SLOT(onLoadShots(QString)));
	connect(ui->gViewShotEditor, SIGNAL(wasScaled(double)), shotManager, SLOT(onScaledView(double)));

	connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(onClicked_Quit()));
    connect(ui->actionLoad, SIGNAL(triggered()), this, SLOT(onClicked_Load()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(onClicked_Save()));
}
void MainWindow::print_info(QString s) {
    ui->infoLabel->setText(s);
    ui->infoLabel->setStyleSheet("color: blue");
}
void MainWindow::print_error(QString s) {
    ui->infoLabel->setText(s);
    ui->infoLabel->setStyleSheet("color: red");
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

void MainWindow::resizeEvent(QResizeEvent* event)
{
    //ui->gView->setMinimumSize(QSize(event->size().width() / 3, 0));
    QMainWindow::resizeEvent(event);
}

MainWindow::~MainWindow()
{
    delete ui;
}

