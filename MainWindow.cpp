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
    gScene->setSceneRect(0,0, 5000, 1000);
	ymlManager = new YmlSceneManager(this, gScene);

    QLinearGradient gradient(0, 0, 0, 1000);
	gradient.setColorAt(0, colorSceneGradient0);
	gradient.setColorAt(1.0, colorSceneGradient1);
    gScene->setBackgroundBrush(gradient);

    ui->gView->setScene(gScene);
	ui->gView->setYmlManager(ymlManager);

    connect(ymlManager, SIGNAL(print_info(QString)), this, SLOT(print_info(QString)));
    connect(ymlManager, SIGNAL(print_error(QString)), this, SLOT(print_error(QString)));
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
    QString fileName = QFileDialog::getOpenFileName(this,
                       tr("Open scene yml"), "", tr("Yaml Files (*.yml *.yaml)"));

	// TODO: ymlManager->requestSave();
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

