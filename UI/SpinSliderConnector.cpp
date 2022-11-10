#include "SpinSliderConnector.h"

void SpinSliderConnector::updateValues(double valueDouble, int valueInt)
{
    if (m_pSlider != nullptr) {
        m_pSlider->blockSignals(true);
        m_pSlider->setValue(valueInt);
        m_pSlider->blockSignals(false);
    }
    if (m_pDoubleSpin != nullptr) {
        m_pDoubleSpin->blockSignals(true);
        m_pDoubleSpin->setValue(valueDouble);
        m_pDoubleSpin->blockSignals(false);
    }
    if (m_pSpin != nullptr) {
        m_pSpin->blockSignals(true);
        m_pSpin->setValue(valueInt);
        m_pSpin->blockSignals(false);
    }
    emit valueChanged(valueDouble);
    emit valueChanged(valueInt);
}

SpinSliderConnector::SpinSliderConnector(QObject *parent)
    : QObject{parent}
{

}

void SpinSliderConnector::setDoubleSpinSlider(QDoubleSpinBox* pDoubleSpin, QSlider* pSlider, int multiply)
{
    m_pDoubleSpin = pDoubleSpin;
    m_pSlider = pSlider;
    m_multiply = multiply;
    connect(m_pDoubleSpin, SIGNAL(valueChanged(double)), this, SLOT(onDoubleValueChanged(double)));
    connect(m_pSlider, SIGNAL(valueChanged(int)), this, SLOT(onIntValueChanged(int)));
}

void SpinSliderConnector::setSpinSlider(QSpinBox* m_pSpin, QSlider* pSlider, int multiply)
{
    m_pSpin = m_pSpin;
    m_pSlider = pSlider;
    m_multiply = multiply;
    connect(m_pSpin, SIGNAL(valueChanged(int)), this, SLOT(onIntValueChanged(int)));
    connect(m_pSlider, SIGNAL(valueChanged(int)), this, SLOT(onIntValueChanged(int)));
}

void SpinSliderConnector::onDoubleValueChanged(double valueDouble)
{
    int valueInt = valueDouble * m_multiply;
    updateValues(valueDouble, valueInt);
}

void SpinSliderConnector::onIntValueChanged(int valueInt)
{
    double valueDouble = valueInt * 1.0 / m_multiply;
    updateValues(valueDouble, valueInt);
}
