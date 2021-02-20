#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ymlManager = new YmlSceneManager(this);
    gScene = new QGraphicsScene(this);
    gScene->setSceneRect(0,0, 5000, 1000);

    QLinearGradient gradient(0, 0, 0, 1000);
    gradient.setColorAt(0, QColor(194, 194, 163));
    gradient.setColorAt(1.0, QColor(122, 122, 82));
    gScene->setBackgroundBrush(gradient);

    ui->gView->setScene(gScene);

    qDebug() << "Main window size1: " << this->size();
    connect(ymlManager, SIGNAL(debugInfo(QString)), ui->infoLabel, SLOT(setText(QString)));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(onClicked_Quit()));
    connect(ui->actionLoad, SIGNAL(triggered()), this, SLOT(onClicked_Load()));
}
void MainWindow::onClicked_Quit()
{
    qDebug() << "Main window size2: " << this->size();
    QApplication::quit();
}
void MainWindow::onClicked_Load()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                       tr("Open scene yml"), "", tr("Yaml Files (*.yml *.yaml)"));

    gScene->clear();
    // TODO: ymlManager->requestSave();

    if ( ymlManager->loadYmlFile(fileName) ) {
        setWindowTitle("W3 YML Scene Editor [" + fileName + "]");
    }

    if ( ymlManager->drawSectionsGraph(gScene) ) {
        ui->infoLabel->setText("Sections graph was successfully drawn.");
    } else {
        ui->infoLabel->setText("Failed to draw sections: INCORRECT YML!");
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

