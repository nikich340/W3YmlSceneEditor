#include "DialogChangeSection.h"
#include "ui_DialogChangeSection.h"
#include <QDebug>
#include <QMessageBox>

DialogChangeSection::DialogChangeSection(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogChangeSection)
{
    ui->setupUi(this);
    connect(ui->choice_1->ui->conditionCheck, SIGNAL(clicked(bool)), this, SLOT(onClicked_Cond(bool)));
}

void DialogChangeSection::updateChoiceForms(sectionLink* link, QStringList sectionsList) {
    sLink = link;
    sectionsLst = sectionsList;

    ui->lineName->setText(sLink->sectionName);

    sk.push_back(ui->choice_1);
    sk.push_back(ui->choice_2);
    sk.push_back(ui->choice_3);
    sk.push_back(ui->choice_4);
    sk.push_back(ui->choice_5);
    sk.push_back(ui->choice_6);
    sk.push_back(ui->choice_7);

    // filter out start sections
    QStringList filteredList = sectionsList;
    for (int i = 0; i < sectionsList.size(); ++i) {
        if (sectionsList[i].startsWith("section_start")) {
            filteredList.removeAt(i);
        }
    }

    for (int i = 0; i < 7; ++i) {
        sk[i]->setupMe( filteredList );

        if (i < sLink->names.size()) {
            // choice line
            sk[i]->ui->choiceLine->setText( sLink->choiceLines[i] );
            // next section name
            sk[i]->ui->nextSectionBox->setCurrentText( sLink->names[i] );

            sk[i]->ui->emphasizeCheck->setChecked( sLink->emphasize[i] );
            sk[i]->ui->singleCheck->setChecked( sLink->single_use[i] );

            // action
            if ( !sLink->choiceActions[i].action.isEmpty() ) {
                sk[i]->ui->actionBox->setCurrentText( sLink->choiceActions[i].action );
                sk[i]->ui->actionPaySpin->setValue( sLink->choiceActions[i].amount );
                sk[i]->ui->actionGrantCheck->setChecked( sLink->choiceActions[i].grantExp );
            }

            // condition
            if ( !sLink->conditions[i].condFact.isEmpty() ) {
                sk[i]->ui->conditionCheck->setChecked(true);
                sk[i]->ui->factNameLine->setText( sLink->conditions[i].condFact );
                sk[i]->ui->factOperandBox->setCurrentText( sLink->conditions[i].condOperand );
                sk[i]->ui->factValSpin->setValue( sLink->conditions[i].condValue );
            } else {
                sk[i]->onClickedCondCheck(false);
            }
        } else {
            qDebug() << "Error setting choice! " << i;
        }
    }

    if ( sLink->timeLimit > 0.0 ) {
        ui->timeLimitCheck->setChecked( true );
        ui->timeLimitSpin->setValue(sLink->timeLimit);
        ui->timeLimitSlider->setValue(sLink->timeLimit * 10.0);
    } else {
        ui->timeLimitSpin->setEnabled(false);
        ui->timeLimitSlider->setEnabled(false);
    }

    ui->timeLimitWidget->setEnabled(false);
    if ( sLink->isChoice ) {
        ui->timeLimitWidget->setEnabled(true);
        ui->choiceButton->setChecked(true);
        onClicked_Choice();

    } else if ( sLink->isScript ) {
        ui->scriptButton->setChecked( true );
        onClicked_Script();

    } else if ( sLink->isRandomizer ) {
        ui->randomButton->setChecked( true );
        onClicked_Random();

    } else if ( sLink->isExit ) {
        ui->exitButton->setChecked( true );
        onClicked_Exit();

    } else {
        ui->nextButton->setChecked( true );
        onClicked_Next();
    }
}

void DialogChangeSection::onClicked_Next() {
    ui->lineName->setValidator( new QRegExpValidator(QRegExp("^section_(start)?(\\S)*$")) );
    ui->lineName->setPlaceholderText("section_story");

    // choice-related
    ui->timeLimitWidget->setEnabled(false);

    sk[0]->setEnabled( true );
    sk[1]->setEnabled( true );
    sk[0]->ui->widgetAction->setEnabled(false);
    sk[1]->ui->widgetAction->setEnabled(false);

    sk[0]->ui->widgetCond->setEnabled( true );
    sk[0]->ui->singleCheck->setEnabled(false);
    sk[0]->ui->emphasizeCheck->setEnabled(false);

    sk[1]->ui->widgetCond->setEnabled(false);

    for (int i = 2; i < 7; ++i) {
        sk[i]->ui->choiceLabel->setText( "-" );
        sk[i]->setEnabled(false);
    }

    onClicked_Cond( sk[0]->ui->conditionCheck->isChecked() );
}

void DialogChangeSection::onClicked_Cond(bool enable) {
    // check for another section
    if ( !ui->nextButton->isChecked() )
        return;
    if (enable) {
        sk[0]->ui->choiceLabel->setText( "on_true" );
        sk[1]->ui->choiceLabel->setText( "on_false" );
        sk[1]->setEnabled( true );
    } else {
        sk[0]->ui->choiceLabel->setText( "NEXT" );
        sk[1]->ui->choiceLabel->setText( "-" );
        sk[1]->setEnabled( false );
    }
}

void DialogChangeSection::onClicked_Script() {
    ui->lineName->setValidator( new QRegExpValidator(QRegExp("^script_(\\S)*$")) );
    ui->lineName->setPlaceholderText("script_story");

    sk[0]->ui->choiceLabel->setText( "NEXT" );
    sk[0]->setEnabled(true);

    // choice-related
    ui->timeLimitWidget->setEnabled( false );
    sk[0]->ui->widgetAction->setEnabled( false );
    sk[0]->ui->widgetCond->setEnabled( false );

    for (int i = 1; i < 7; ++i) {
        sk[i]->ui->choiceLabel->setText( "-" );
        sk[i]->setEnabled(false);
    }
}

void DialogChangeSection::onClicked_Choice() {
    ui->lineName->setValidator( new QRegExpValidator(QRegExp("^section_choice(_(\\S)*)?$")) );
    ui->lineName->setPlaceholderText("section_choice_story");

    ui->timeLimitWidget->setEnabled(true);

    for (int i = 0; i < 7; ++i) {
        sk[i]->ui->choiceLabel->setText( "choice #" + QString::number(i + 1) );
        sk[i]->setEnabled(true);
        sk[i]->ui->widgetAction->setEnabled( true );
        sk[i]->ui->widgetCond->setEnabled( true );
        sk[i]->onChoseAction( sk[i]->ui->actionBox->currentText() );
    }
    //sk[0]->ui->conditionCheck->setEnabled( true );
    sk[0]->ui->singleCheck->setEnabled( true );
    sk[0]->ui->emphasizeCheck->setEnabled( true );
}

void DialogChangeSection::onClicked_Random() {
    ui->lineName->setValidator( new QRegExpValidator(QRegExp("^section_(\\S)*$")) );
    ui->lineName->setPlaceholderText("section_story");

    // choice-related
    ui->timeLimitWidget->setEnabled( false );

    for (int i = 0; i < 7; ++i) {
        sk[i]->ui->choiceLabel->setText( "random #" + QString::number(i + 1) );
        sk[i]->setEnabled(true);

        // choice-related
        sk[i]->ui->widgetAction->setEnabled( false );
        sk[i]->ui->widgetCond->setEnabled( false );
    }
}

void DialogChangeSection::onClicked_Exit() {
    ui->lineName->setValidator( new QRegExpValidator(QRegExp("^section_exit(_(\\S)*)?$")) );
    ui->lineName->setPlaceholderText("section_exit_story");

    // choice-related
    ui->timeLimitWidget->setEnabled(false);

    for (int i = 0; i < 7; ++i) {
        sk[i]->ui->choiceLabel->setText( "-" );
        sk[i]->setEnabled(false);
    }
}

void DialogChangeSection::onTimeLimitChanged(double tld) {
    int tli = (tld * 10.0);
    ui->timeLimitSlider->setValue(tli);
}
void DialogChangeSection::onTimeLimitChanged(int tli) {
    double tld = tli / 10.0;
    ui->timeLimitSpin->setValue(tld);
}

void DialogChangeSection::onTimeLimitClicked(bool enabled) {
    ui->timeLimitSpin->setEnabled(enabled);
    ui->timeLimitSlider->setEnabled(enabled);
}

void DialogChangeSection::onSectionNameChanged(QString str) {
    if ( !ui->lineName->hasAcceptableInput() ) {
        ui->lineName->setStyleSheet("color: red");
    } else {
        ui->lineName->setStyleSheet("color: black");
    }
}

void DialogChangeSection::accept() {
    qDebug()<<"internal accept";

    QMessageBox* msg = nullptr;

    if ( !ui->lineName->hasAcceptableInput() ) {
        QMessageBox msg(QMessageBox::Warning, "Incorrect settings", "Wrong section name format!", QMessageBox::Ok, this);
        msg.setModal(true);
        int ret = msg.exec(); // QMessageBox::Ok
        return;
    }
    if ( ui->lineName->text() != sLink->sectionName && sectionsLst.contains(ui->lineName->text()) ) {
        QMessageBox msg(QMessageBox::Warning, "Incorrect settings", "Section with the same name already exists!", QMessageBox::Ok, this);
        msg.setModal(true);
        int ret = msg.exec(); // QMessageBox::Ok
        return;
    }
    int cnt = 0;
    for (int i = 0; i < 7; ++i) {
        if ( !sk[i]->isEnabled() || sk[i]->ui->nextSectionBox->currentText() == "NOT SET" )
            continue;
        cnt += 1;
        if ( ui->choiceButton->isChecked() && sk[i]->ui->choiceLine->text().isEmpty() ) {
            QMessageBox msg(QMessageBox::Warning, "Incorrect settings", "Dialog line #" + QString::number(i+1) + " should not be empty!", QMessageBox::Ok, this);
            msg.setModal(true);
            int ret = msg.exec(); // QMessageBox::Ok
            return;
        }
        if ( ui->choiceButton->isChecked() && sk[i]->ui->conditionCheck && sk[i]->ui->factNameLine->text() == "NOT SET" ) {
            QMessageBox msg(QMessageBox::Warning, "Incorrect settings", "Condition #" + QString::number(i+1) + " fact name should not be empty!", QMessageBox::Ok, this);
            msg.setModal(true);
            int ret = msg.exec(); // QMessageBox::Ok
            return;
        }
    }
    if ( (ui->choiceButton->isChecked() && cnt < 1) ||
         (ui->randomButton->isChecked() && cnt < 1) ||
         (ui->nextButton->isChecked() && cnt < 1) ) {
        QMessageBox msg(QMessageBox::Warning, "Incorrect settings", "At least one output must be linked to existing section!", QMessageBox::Ok, this);
        msg.setModal(true);
        int ret = msg.exec(); // QMessageBox::Ok
        return;
    }
    if ( (ui->nextButton->isChecked() && sk[0]->ui->conditionCheck->isChecked() && cnt < 2) ) {
        QMessageBox msg(QMessageBox::Warning, "Incorrect settings", "Both of outputs must be linked to existing sections!", QMessageBox::Ok, this);
        msg.setModal(true);
        int ret = msg.exec(); // QMessageBox::Ok
        return;
    }

    QDialog::accept();
}

DialogChangeSection::~DialogChangeSection()
{
    delete ui;
}
