#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionLoad_A, SIGNAL(triggered()), this, SLOT(onClicked_QuitBar()));
}
void MainWindow::onClicked_QuitBar()
{
    QApplication::quit();
}

MainWindow::~MainWindow()
{
    delete ui;
}

