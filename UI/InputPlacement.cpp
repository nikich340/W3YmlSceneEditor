#include "InputPlacement.h"
#include "ui_InputPlacement.h"

InputPlacement::InputPlacement(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InputPlacement)
{
	ui->setupUi(this);
	connect(ui->spinX, SIGNAL(valueChanged(double)), this, SLOT(signalChange()));
	connect(ui->spinY, SIGNAL(valueChanged(double)), this, SLOT(signalChange()));
	connect(ui->spinZ, SIGNAL(valueChanged(double)), this, SLOT(signalChange()));
	connect(ui->spinPitch, SIGNAL(valueChanged(double)), this, SLOT(signalChange()));
	connect(ui->spinYaw, SIGNAL(valueChanged(double)), this, SLOT(signalChange()));
	connect(ui->spinRoll, SIGNAL(valueChanged(double)), this, SLOT(signalChange()));
}

void InputPlacement::setPlacement(transform _placement) {
	ui->spinX->setValue( _placement.pos.x() );
	ui->spinY->setValue( _placement.pos.y() );
	ui->spinZ->setValue( _placement.pos.z() );
	ui->spinPitch->setValue( _placement.rot.x() );
	ui->spinYaw->setValue( _placement.rot.y() );
	ui->spinRoll->setValue( _placement.rot.z() );
}

transform InputPlacement::getPlacement() {
	transform ret;
	ret.pos.setX( ui->spinX->value() );
	ret.pos.setY( ui->spinY->value() );
	ret.pos.setZ( ui->spinZ->value() );
	ret.rot.setX( ui->spinPitch->value() );
	ret.rot.setY( ui->spinYaw->value() );
	ret.rot.setZ( ui->spinRoll->value() );
	return ret;
}

void InputPlacement::signalChange() {
	emit placementChanged( getPlacement() );
}

InputPlacement::~InputPlacement()
{
	delete ui;
}
