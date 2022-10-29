#include "InputDialogshot.h"
#include "ui_InputDialogshot.h"

InputDialogshot::InputDialogshot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InputDialogshot)
{
    ui->setupUi(this);
}

InputDialogshot::~InputDialogshot()
{
    delete ui;
}
