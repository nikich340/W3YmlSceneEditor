#ifndef INPUTPLACEMENT_H
#define INPUTPLACEMENT_H

#include <QWidget>
#include "YmlStructs.h"

namespace Ui {
class InputPlacement;
}

class InputPlacement : public QWidget
{
	Q_OBJECT

public:
	explicit InputPlacement(QWidget *parent = nullptr);
	void setPlacement(transform _placement);
	transform getPlacement();
	~InputPlacement();

private:
	Ui::InputPlacement *ui;

signals:
	void placementChanged(transform newPlacement);

public slots:
	void signalChange();
};

#endif // INPUTPLACEMENT_H
