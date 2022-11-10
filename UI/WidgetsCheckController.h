#ifndef WIDGETSCHECKCONTROLLER_H
#define WIDGETSCHECKCONTROLLER_H

#include <QObject>
#include <QWidget>
#include <QCheckBox>

class WidgetsCheckController : public QObject
{
    Q_OBJECT
private:
    QVector<QWidget*> m_pControlledWidgets;
    QCheckBox* m_pCheckbox;
public:
    explicit WidgetsCheckController(QObject *parent = nullptr);
    void setCheckbox(QCheckBox* pCheckbox);
    void addWidget(QWidget* pWidget);
    void removeWidget(QWidget* pWidget);
    void setChecked(bool checked);
    bool isChecked();

private slots:
    onCheckboxClicked(bool checked);
signals:
    void clicked(bool);
};


#endif // WIDGETSCHECKCONTROLLER_H
