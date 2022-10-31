#include "InputPose.h"
#include "ui_InputPose.h"

InputPose::InputPose(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InputPose)
{
	ui->setupUi(this);
    ui->comboState->addItems(CONSTANTS::MIMICS_LIST);
	ui->comboStatus->addItems({"Low", "High"});

	connect(ui->lineName, SIGNAL(textEdited(QString)), this, SLOT(signalChangeName()));

	connect(ui->lineAnim, SIGNAL(textEdited(QString)), this, SLOT(signalChangeAnimPose()));
	connect(ui->lineNameParam, SIGNAL(textEdited(QString)), this, SLOT(signalChangeAnimPose()));
	connect(ui->comboState, SIGNAL(currentTextChanged(QString)), this, SLOT(signalChangeAnimPose()));
	connect(ui->comboStatus, SIGNAL(currentTextChanged(QString)), this, SLOT(signalChangeAnimPose()));
}

void InputPose::setPose(anim_pose _pose, QString _name) {
	setProperty("nameID", _pose.nameID);
	setProperty("actorID", _pose.actorID);
	ui->lineName->setText( _name );
	ui->lineAnim->setText( _pose.idle_anim );
	ui->lineNameParam->setText( _pose.nameParam );
	ui->comboState->setCurrentText( _pose.emotional_state.isEmpty() ? "Determined" : _pose.emotional_state );
	ui->comboStatus->setCurrentText( _pose.status.isEmpty() ? "High" : _pose.status );
}

QString InputPose::getName() {
	return ui->lineName->text();
}

anim_pose InputPose::getPose() {
	anim_pose ret;
	ret.status = ui->comboStatus->currentText();
	ret.idle_anim = ui->lineAnim->text();
	ret.nameParam = ui->lineNameParam->text();
	ret.emotional_state = ui->comboState->currentText();
	ret.nameID = property("nameID").toInt();
	ret.actorID = property("actorID").toInt();
	return ret;
}

void InputPose::setNameStyleSheet(QString styleSheet) {
	ui->lineName->setStyleSheet(styleSheet);
}

void InputPose::signalChangeName() {
	emit nameChanged( ui->lineName->text() );
}

void InputPose::signalChangeAnimPose() {
	emit animPoseChanged( getPose() );
}

InputPose::~InputPose()
{
	delete ui;
}
