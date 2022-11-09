#include "DialogSelectEnv.h"
#include "ui_DialogSelectEnv.h"

DialogSelectEnv::DialogSelectEnv(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSelectEnv)
{
    ui->setupUi(this);
    connect(ui->lineEdit, SIGNAL(textEdited(QString)), this, SLOT(onFilterChanged(QString)));
}

DialogSelectEnv::~DialogSelectEnv()
{
    delete ui;
}

void DialogSelectEnv::accept()
{
    if (ui->listWidget->currentRow() >= 0)
        emit envSelected(ui->listWidget->currentItem()->text());
    super::accept();
}

void DialogSelectEnv::setStorage(StringStorage* newStorage)
{
    m_pStorage = newStorage;
    ui->listWidget->addItems(m_pStorage->allStrings(FILTER_MAX));
    ui->labelInfoFilter->setText(QString("%1 of %2 envs shown").arg(std::min(FILTER_MAX, m_pStorage->count())).arg(m_pStorage->count()));
}

void DialogSelectEnv::onFilterChanged(QString text)
{
    ui->listWidget->clear();
    QStringList newItems = m_pStorage->stringsByPrefix(text, FILTER_MAX);
    ui->labelInfoFilter->setText(QString("%1 of %2 envs shown").arg(newItems.count()).arg(m_pStorage->count()));
    ui->listWidget->addItems(newItems);
}
