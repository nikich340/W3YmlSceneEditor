#ifndef INPUTMIMICPOSE_H
#define INPUTMIMICPOSE_H

#include <QWidget>
#include "YmlStructs.h"
#include "constants.h"

namespace Ui {
class InputMimicPose;
}

class InputMimicPose : public QWidget
{
	Q_OBJECT

public:
	explicit InputMimicPose(QWidget *parent = nullptr);
	~InputMimicPose();
	void setPose(mimic_pose _pose, QString _name);
	mimic_pose getPose();
	QString getName();
	void setNameLock(bool lock);
	void setNameStyleSheet(QString styleSheet);

private:
	Ui::InputMimicPose *ui;
	bool nameChangeLocked = false;

signals:
	void nameChanged(QString newName);
	void mimicPoseChanged(mimic_pose newPose);

public slots:
	void signalChangeName();
	void signalChangeMimicPose();
};

#endif // INPUTMIMICPOSE_H
