#include "RepoActorsDialog.h"
#include "ui_RepoActorsDialog.h"

#define pSG m_ymlManager->sceneGlobals()

RepoActorsDialog::RepoActorsDialog(YmlSceneManager *manager, QWidget *parent) :
	QDialog(parent),
    m_ymlManager(manager),
	ui(new Ui::RepoActorsDialog)
{
	ui->setupUi(this);

	setWindowFlags(Qt::Window);

    //actorsTemp = pSG->actors;

    for (auto actorID : pSG->actors.keys()) {
        QString actorName = pSG->getName(actorID);
		ui->boxActors->addItem(actorName, actorID);

		//nameByID[actorID] = actorName;
	}

	connect( ui->boxActors, SIGNAL(currentIndexChanged(int)), this, SLOT(onChangedActor(int)) );
	connect( ui->textApps, SIGNAL(textChanged()), this, SLOT(onChangedApps()) );
	connect( ui->buttonCloneActor, SIGNAL(clicked()), this, SLOT(onClonedActor()) );
	connect( ui->buttonNewActor, SIGNAL(clicked()), this, SLOT(onAddActor()) );
	connect( ui->buttonRemoveActor, SIGNAL(clicked()), this, SLOT(onRemoveActor()) );
	connect( ui->lineName, SIGNAL(textEdited(QString)), this, SLOT(onRenameActor()) );
	connect( ui->checkPlacement, SIGNAL(clicked()), this, SLOT(onSwitch_DefPlacement()) );
	connect( ui->checkPose, SIGNAL(clicked()), this, SLOT(onSwitch_DefAnimPose()) );
	connect( ui->checkMimicPose, SIGNAL(clicked()), this, SLOT(onSwitch_DefMimicPose()) );

	if (ui->boxActors->count() > 0)
		onChangedActor(0);
}

void RepoActorsDialog::onChangedActor(int index) {
    if (prevID != -1 && !pSG->actors[prevID].fromRepo) {
        //onRenameActor does it //pSG->rename(prevID, ui->lineName->text());
        pSG->actors[prevID].templatePath = ui->linePath->text();

        pSG->actors[prevID].appearances = ui->textApps->toPlainText().split(QRegExp("[\n]"), QString::SkipEmptyParts).toSet();
        pSG->actors[prevID].tags = ui->textTags->toPlainText().split(QRegExp("[\n]"), QString::SkipEmptyParts).toSet();
        pSG->actors[prevID].mainAppearance = ui->boxDefApp->currentText();
        pSG->actors[prevID].by_voicetag = ui->checkVoicetag->isChecked();
		/* PLACEMENT */
		if ( ui->checkPlacement->isChecked() ) {
            pSG->defaultPlacement[prevID] = ui->widgetPlacement->getPlacement();
        } else if ( pSG->defaultPlacement.contains(prevID) ) {
            pSG->defaultPlacement.remove(prevID);
		}

		/* MIMIC POSE */
		if ( ui->checkMimicPose->isChecked() ) {
			mimic_pose mimicPose = ui->widgetMimicPose->getPose();
			mimicPose.actorID = prevID;
            mimicPose.nameID = pSG->getID(SMIMICPOSES, ui->widgetMimicPose->getName());

            pSG->defaultMimic[prevID] = mimicPose;
        } else if ( pSG->defaultMimic.contains(prevID) ) {
            pSG->defaultMimic.remove(prevID);
		}

		/* ANIM POSE */
		if ( ui->checkPose->isChecked() ) {
			anim_pose animPose = ui->widgetPose->getPose();
			animPose.actorID = prevID;
            animPose.nameID = pSG->getID(SANIMPOSES, ui->widgetPose->getName());

            pSG->poses[animPose.nameID] = animPose;
            pSG->defaultPose[prevID] = animPose.nameID;
        } else if ( pSG->defaultPose.contains(prevID) ) {
            pSG->defaultPose.remove(prevID);
		}
	}
	if (index != -1) {
		updateActor = true;
		prevID = ui->boxActors->itemData(index).toInt();

        ui->lineName->setText(pSG->getName(prevID));
        ui->linePath->setText(pSG->actors[prevID].templatePath);
        ui->textApps->setPlainText(pSG->actors[prevID].getAppearances());
        ui->textTags->setPlainText(pSG->actors[prevID].getTags());
        ui->checkVoicetag->setChecked(pSG->actors[prevID].by_voicetag);
		/* PLACEMENT */
        if ( pSG->defaultPlacement.contains(prevID) ) {
            ui->widgetPlacement->setPlacement(pSG->defaultPlacement[prevID]);
			ui->checkPlacement->setChecked(true);
		} else {
			ui->widgetPlacement->setPlacement( transform() );
			ui->checkPlacement->setChecked(false);
		}

		/* MIMIC POSE */
        if ( pSG->defaultMimic.contains(prevID) ) {
            int poseID = pSG->defaultMimic[prevID].nameID;
            ui->widgetMimicPose->setPose(pSG->defaultMimic[prevID], pSG->getName(poseID));
			ui->checkMimicPose->setChecked(true);
		} else {
			int nameID;
			QString name;
            pSG->getNameUnused(SMIMICPOSES, pSG->getName(prevID) + "_default_pose_", name, nameID);
			ui->widgetMimicPose->setPose( mimic_pose(nameID, QString()), name );
			ui->checkMimicPose->setChecked(false);
		}

		/* ANIM POSE */
        if ( pSG->defaultPose.contains(prevID) ) {
            int poseID = pSG->defaultPose[prevID];
            ui->widgetPose->setPose(pSG->poses[poseID], pSG->getName(poseID));
			ui->checkPose->setChecked(true);
		} else {
			int nameID;
			QString name;
            pSG->getNameUnused(SANIMPOSES, pSG->getName(prevID) + "_default_pose_", name, nameID);
			ui->widgetPose->setPose( anim_pose(nameID, QString()), name );
			ui->checkPose->setChecked(false);
		}
	}

    if (index == -1 || pSG->actors[prevID].fromRepo) {
		setEnabledEditions( false );
	} else {
		setEnabledEditions( true );
	}
	onSwitch_DefPlacement();
	onSwitch_DefAnimPose();
	onSwitch_DefMimicPose();
}

void RepoActorsDialog::onSwitch_DefPlacement() {
	ui->widgetPlacement->setEnabled( ui->checkPlacement->isChecked() );
}
void RepoActorsDialog::onSwitch_DefAnimPose() {
	ui->widgetPose->setEnabled( ui->checkPose->isChecked() );
}
void RepoActorsDialog::onSwitch_DefMimicPose() {
	ui->widgetMimicPose->setEnabled( ui->checkMimicPose->isChecked() );
	ui->widgetMimicPose->setNameLock(true);
}

void RepoActorsDialog::onRenameActor() {
	int actorID = ui->boxActors->currentData().toInt();
	QString newName = ui->lineName->text();
    QString oldName = pSG->getName(actorID);
	//qd << "old: " << oldName << " new: " << newName;

	if ( newName == oldName ) {  // not changed
		ui->lineName->setStyleSheet("color: black");
		userMistakes &= ~1;
    } else if ( pSG->hasName(SACTORS, newName) ) { // changed and duplicating, not ok
		ui->lineName->setStyleSheet("color: red");
		userMistakes |= 1;
	} else {  // changed and ok
		ui->lineName->setStyleSheet("color: blue");
        pSG->rename(actorID, newName);
		userMistakes &= ~1;
	}
	ui->boxActors->setEnabled( !userMistakes );
}
void RepoActorsDialog::onRenameAnimPose(QString newName) {
	int nameID = ui->widgetPose->property("nameID").toInt();
    QString oldName = pSG->getName(nameID);
	//qd << "old: " << oldName << " new: " << newName;

	if ( newName == oldName ) {  // not changed
		ui->widgetPose->setNameStyleSheet("color: black");
		userMistakes &= ~2;
    } else if ( pSG->hasName(SACTORS, newName) ) { // changed and duplicating, not ok
		ui->widgetPose->setNameStyleSheet("color: red");
		userMistakes |= 2;
	} else {  // changed and ok
		ui->widgetPose->setNameStyleSheet("color: blue");
        pSG->rename(nameID, newName);
		userMistakes &= ~2;
	}
	ui->boxActors->setEnabled( !userMistakes );
}

void RepoActorsDialog::onClonedActor() {
	int newID;
	QString newName;
    pSG->getNameUnused(SACTORS, ui->boxActors->currentText() + "_cloned_", newName, newID);
	int oldID = ui->boxActors->currentData().toInt();

    pSG->actors[newID] = pSG->actors[oldID];
    pSG->actors[newID].nameID = newID;
    pSG->actors[newID].fromRepo = false;
	// TODO! Add to dialogscript

	int newIndex = ui->boxActors->currentIndex() + 1;
	ui->boxActors->insertItem( newIndex, newName, newID  );
	ui->boxActors->setCurrentIndex( newIndex );
}

void RepoActorsDialog::onAddActor() {
	int newID;
	QString newName;
    pSG->getNameUnused(SACTORS, "new_actor_", newName, newID);

    pSG->actors[newID] = asset(newID, newName);
    pSG->actors[newID].nameID = newID;
	// TODO! Add to dialogscript

	int newIndex = ui->boxActors->currentIndex() + 1;
	ui->boxActors->insertItem( newIndex, newName, newID  );
	ui->boxActors->setCurrentIndex( newIndex );
}

void RepoActorsDialog::onRemoveActor() {
	int actorID = ui->boxActors->currentData().toInt();
	QString actorName = ui->boxActors->currentText();
	QMessageBox sureBox(QMessageBox::Warning, "Warning!", "Do you really want to remove actor [" + actorName + "]?\nIt will remove all shot events and dialog lines linked to this actor.", QMessageBox::Yes | QMessageBox::No);
	sureBox.setModal(true);
	int result = sureBox.exec();
	if (result == QMessageBox::Yes) {

		ui->boxActors->removeItem( ui->boxActors->currentIndex() );

        m_ymlManager->removeActorAsset(actorID);
        //pSG->actors.remove(actorID);
        //pSG->removeID(actorID);
        //pSG->defaultMimic.remove(actorID);
        //pSG->defaultPlacement.remove(actorID);
        // !check shots pSG->defaultPose.remove(actorID);
		// TODO! Remove all related assets
		// TODO! Remove all id references from shots and dialogscript
	}
}

void RepoActorsDialog::setEnabledEditions(bool enabled) {
	ui->lineName->setEnabled( enabled );
	ui->linePath->setEnabled( enabled );
	ui->boxDefApp->setEnabled( enabled );
	ui->textApps->setEnabled( enabled );
	ui->textTags->setEnabled( enabled );
	ui->checkVoicetag->setEnabled( enabled );
	ui->buttonRemoveActor->setEnabled( enabled );

	ui->checkPlacement->setEnabled( enabled );
	ui->widgetPlacement->setEnabled( enabled );

	ui->checkMimicPose->setEnabled( enabled );
	ui->widgetMimicPose->setEnabled( enabled );
	ui->widgetMimicPose->setNameLock( true );

	ui->widgetPose->setEnabled( enabled );
	ui->checkPose->setEnabled( enabled );
}

void RepoActorsDialog::onChangedApps() {
	int oldIndex = ui->boxDefApp->currentIndex();

	ui->boxDefApp->clear();
	QStringList newApps = ui->textApps->toPlainText().split(QRegExp("[\n]"), QString::SkipEmptyParts);
	ui->boxDefApp->addItems( newApps );

	if ( updateActor ) {
        int newIndex = newApps.indexOf( pSG->actors[prevID].mainAppearance );
        if (newIndex == -1 && !pSG->actors[prevID].appearances.isEmpty())
			newIndex = 0;
		ui->boxDefApp->setCurrentIndex( newIndex );
		updateActor = false;
	} else if ( oldIndex >= 0 && ui->boxDefApp->count() > oldIndex ) {
		ui->boxDefApp->setCurrentIndex(oldIndex);
	} else if ( ui->boxDefApp->count() > 0 ) {
		ui->boxDefApp->setCurrentIndex(0);
	}
}

bool RepoActorsDialog::canCloseDialog() {
	qd << "RepoActorsDialog::canCloseDialog()";
	onChangedActor(0);
	if ( !ui->boxActors->isEnabled() ) {
		QMessageBox sureBox(QMessageBox::Critical, "Error", "Actor with the same name already exists.\nRename current actor.", QMessageBox::Ok);
		sureBox.setModal(true);
		int result = sureBox.exec();
		return false;
	}
	return true;
}

void RepoActorsDialog::accept() {
	qd << "RepoActorsDialog::accept()";
	if ( canCloseDialog() )
		QDialog::accept();
}

void RepoActorsDialog::reject() {
	qd << "RepoActorsDialog::reject()";
	if ( canCloseDialog() )
		QDialog::accept();
}

RepoActorsDialog::~RepoActorsDialog()
{
	delete ui;
}
