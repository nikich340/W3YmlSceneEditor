#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <YmlSceneManager.h>
#include "GraphicsSectionItem.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    YmlSceneManager *ymlManager;
    QGraphicsScene *gScene;
    void resizeEvent(QResizeEvent* event);

public slots:
    void print_info(QString s);
    void print_error(QString s);
    void onClicked_Quit();
    void onClicked_Load();
    void onClicked_Save();
};
#endif // MAINWINDOW_H
