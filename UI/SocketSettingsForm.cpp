#include "SocketSettingsForm.h"
#include "ui_SocketSettingsForm.h"

SocketSettingsForm::SocketSettingsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SocketSettingsForm)
{
    ui->setupUi(this);
}

void SocketSettingsForm::setupMe(QStringList sectionsList) {
    QStringList operandLst;
    operandLst << "<" << "<=" << "=" << "!=" << ">" << ">=";
    ui->factOperandBox->addItems( operandLst );

    QStringList actionLst;
    actionLst << "NOT SET" << "shop" << "blacksmith" << "betting" << "gwent"
              << "axii" << "exit" << "pay" << "shave" << "haircut";
    actionLst.sort();
    ui->actionBox->addItems( actionLst );

    ui->nextSectionBox->addItem("NOT SET");
    sectionsList.sort();
    ui->nextSectionBox->addItems( sectionsList );
}

void SocketSettingsForm::onChoseAction(QString action) {
    if (action == "pay") {
        ui->actionPayLabel->setEnabled(true);
        ui->actionPaySpin->setEnabled(true);
        ui->actionGrantCheck->setEnabled(true);
    } else if (action == "haircut" || action == "shave") {
        ui->actionPayLabel->setEnabled(true);
        ui->actionPaySpin->setEnabled(true);
        ui->actionGrantCheck->setEnabled(false);
    } else {
        ui->actionPayLabel->setEnabled(false);
        ui->actionPaySpin->setEnabled(false);
        ui->actionGrantCheck->setEnabled(false);
    }
}
void SocketSettingsForm::onClickedCondCheck(bool enabled) {
    ui->factLabel->setEnabled( enabled );
    ui->factOperandBox->setEnabled( enabled );
    ui->factValSpin->setEnabled( enabled );
    ui->factNameLine->setEnabled( enabled );
}
SocketSettingsForm::~SocketSettingsForm()
{
    delete ui;
}
