#include "WidgetsCheckController.h"

WidgetsCheckController::WidgetsCheckController(QObject *parent)
    : QObject{parent}
{

}

void WidgetsCheckController::setCheckbox(QCheckBox* pCheckbox)
{
    m_pCheckbox = pCheckbox;
    connect(m_pCheckbox, SIGNAL(clicked(bool)), this, SLOT(onCheckboxClicked(bool)));
}

void WidgetsCheckController::addWidget(QWidget* pWidget)
{
    m_pControlledWidgets.append(pWidget);
}

void WidgetsCheckController::removeWidget(QWidget* pWidget)
{
    m_pControlledWidgets.removeOne(pWidget);
}

void WidgetsCheckController::setChecked(bool checked)
{
    m_pCheckbox->setChecked(checked);
    for (int i = 0; i < m_pControlledWidgets.count(); i += 1) {
        m_pControlledWidgets[i]->setEnabled(checked);
    }
}

WidgetsCheckController::onCheckboxClicked(bool checked)
{
    for (int i = 0; i < m_pControlledWidgets.count(); i += 1) {
        m_pControlledWidgets[i]->setEnabled(checked);
    }
    emit clicked(checked);
}
