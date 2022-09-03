#ifndef REPOACTORSDIALOG_H
#define REPOACTORSDIALOG_H

#include <QDialog>
#include "YmlSceneManager.h"

namespace Ui {
class RepoActorsDialog;
}

class RepoActorsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit RepoActorsDialog(YmlSceneManager *manager, QWidget *parent = nullptr);
	void setEnabledEditions(bool enabled);
	bool canCloseDialog();
	~RepoActorsDialog();

public slots:
	virtual void accept();
	virtual void reject();
	void onChangedActor(int index);
	void onChangedApps();
	void onRenameActor();
	void onClonedActor();
	void onAddActor();
	void onRemoveActor();
	void onRenameAnimPose(QString newName);
	void onSwitch_DefPlacement();
	void onSwitch_DefAnimPose();
	void onSwitch_DefMimicPose();

private:
	int userMistakes = 0; // 1 - actor name, 2 - pose name
	int prevID = -1;
	bool updateActor = false;
	//QHash<int, QString> nameByID;
	//QHash<int, asset> actorsTemp;
	YmlSceneManager *ymlMan = nullptr;
	Ui::RepoActorsDialog *ui;
};

#endif // REPOACTORSDIALOG_H
