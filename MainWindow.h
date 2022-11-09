#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include "YmlSceneManager.h"
#include "GraphicsSectionItem.h"
#include "ShotManager.h"
#include "RepoActorsDialog.h"
#include "DialogSelectEnv.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void loadYmlRepos();
    void loadCsvFiles();

private:
    Ui::MainWindow* ui;
    YmlSceneManager* m_pYmlManager = nullptr;
    ShotManager* m_pShotManager = nullptr;
    QGraphicsScene* m_pSectionsScene = nullptr;
    QGraphicsScene* m_pDialogsScene = nullptr;
    DialogSelectEnv* m_pDialogSelectEnv = nullptr;

    QString m_log;
    int m_logLinesCnt = 0;
    void addPrintLog(QString lineColor, QString line);
    void resizeEvent(QResizeEvent* event);
	void writeSetting(QString name, QVariant value);
	QVariant readSetting(QString name, QVariant defaultValue = QVariant());
    void setupSA_PageWidget(const EShotActionType SA_Type, QWidget* pWidget);

public slots:   
	void print_info(QString s);
	void print_warning(QString s);
    void print_error(QString s);
    void onClicked_Quit();
    void onClicked_Load();
	void onClicked_Save();
	void onClicked_ShowhideLog();
	void onClicked_SetRepoPath();

	// repo things
	void onClicked_RepoActors();
};
#endif // MAINWINDOW_H
