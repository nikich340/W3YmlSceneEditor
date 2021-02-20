#include "SocketSettingsForm.h"
#include "ui_SocketSettingsForm.h"

SocketSettingsForm::SocketSettingsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SocketSettingsForm)
{
    ui->setupUi(this);
}

SocketSettingsForm::~SocketSettingsForm()
{
    delete ui;
}
