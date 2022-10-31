#include "InputMimicPose.h"
#include "ui_InputMimicPose.h"

InputMimicPose::InputMimicPose(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InputMimicPose)
{
	ui->setupUi(this);
    ui->comboAnim->addItems(CONSTANTS::MIMICS_LIST);
    ui->comboEyes->addItems(CONSTANTS::MIMICS_LIST);
    ui->comboPose->addItems(CONSTANTS::MIMICS_LIST);
    ui->comboState->addItems(CONSTANTS::MIMICS_LIST);

	connect(ui->lineName, SIGNAL(textEdited(QString)), this, SLOT(signalChangeName()));

	connect(ui->comboAnim, SIGNAL(currentTextChanged(QString)), this, SLOT(signalChangeMimicPose()));
	connect(ui->comboEyes, SIGNAL(currentTextChanged(QString)), this, SLOT(signalChangeMimicPose()));
	connect(ui->comboPose, SIGNAL(currentTextChanged(QString)), this, SLOT(signalChangeMimicPose()));
	connect(ui->comboState, SIGNAL(currentTextChanged(QString)), this, SLOT(signalChangeMimicPose()));
	connect(ui->spinWeight, SIGNAL(valueChanged(double)), this, SLOT(signalChangeMimicPose()));
	connect(ui->spinDuration, SIGNAL(valueChanged(double)), this, SLOT(signalChangeMimicPose()));
}

void InputMimicPose::setPose(mimic_pose _pose, QString _name) {
	ui->lineName->setText(_name);

	setProperty("nameID", _pose.nameID);
	setProperty("actorID", _pose.actorID);
	ui->comboAnim->setCurrentText(_pose.anim.isEmpty() ? "Determined" : _pose.anim);
	ui->comboEyes->setCurrentText(_pose.eyes.isEmpty() ? "Determined" : _pose.eyes);
	ui->comboPose->setCurrentText(_pose.pose.isEmpty() ? "Determined" : _pose.pose);
	ui->comboState->setCurrentText(_pose.emotional_state.isEmpty() ? "Determined" : _pose.emotional_state);

	ui->spinWeight->setValue(_pose.weight);
	ui->spinDuration->setValue(_pose.duration);
}


QString InputMimicPose::getName() {
	return ui->lineName->text();
}

mimic_pose InputMimicPose::getPose() {
	mimic_pose ret;
	ret.anim = ui->comboAnim->currentText();
	ret.eyes = ui->comboEyes->currentText();
	ret.pose = ui->comboPose->currentText();
	ret.weight = ui->spinWeight->value();
	ret.duration = ui->spinDuration->value();
	ret.nameID = property("nameID").toInt();
	ret.actorID = property("actorID").toInt();
	return ret;
}

void InputMimicPose::signalChangeName() {
	emit nameChanged( ui->lineName->text() );
}

void InputMimicPose::signalChangeMimicPose() {
	emit mimicPoseChanged( getPose() );
}

void InputMimicPose::setNameStyleSheet(QString styleSheet) {
	ui->lineName->setStyleSheet(styleSheet);
}

void InputMimicPose::setNameLock(bool lock) {
	nameChangeLocked = lock;
	if (lock) {
		ui->lineName->setEnabled(false);
		disconnect(ui->lineName, SIGNAL(textEdited(QString)), this, nullptr);
	} else {
		ui->lineName->setEnabled(true);
		connect(ui->lineName, SIGNAL(textEdited(QString)), this, SLOT(signalChangeName()));
	}
}

InputMimicPose::~InputMimicPose()
{
	delete ui;
}
