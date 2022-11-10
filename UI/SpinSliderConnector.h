#ifndef SPINSLIDERCONNECTOR_H
#define SPINSLIDERCONNECTOR_H

#include <QObject>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QSlider>

class SpinSliderConnector : public QObject
{
    Q_OBJECT
private:
    QDoubleSpinBox* m_pDoubleSpin   = nullptr;
    QSpinBox* m_pSpin               = nullptr;
    QSlider* m_pSlider              = nullptr;
    int m_multiply = 1;
    void updateValues(double valueDouble, int valueInt);
public:
    explicit SpinSliderConnector(QObject *parent = nullptr);
    void setDoubleSpinSlider(QDoubleSpinBox* pDoubleSpin, QSlider* pSlider, int multiply = 1000);
    void setSpinSlider(QSpinBox* m_pSpin, QSlider* pSlider, int multiply = 1);

public slots:
    void onDoubleValueChanged(double valueDouble);
    void onIntValueChanged(int valueInt);

signals:
    void valueChanged(double);
    void valueChanged(int);
};

#endif // SPINSLIDERCONNECTOR_H
