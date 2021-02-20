#include "DialogChangeSection.h"
#include "ui_DialogChangeSection.h"

DialogChangeSection::DialogChangeSection(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogChangeSection)
{
    ui->setupUi(this);
}

DialogChangeSection::~DialogChangeSection()
{
    delete ui;
}
