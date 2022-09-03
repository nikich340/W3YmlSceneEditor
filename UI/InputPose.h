#ifndef INPUTPOSE_H
#define INPUTPOSE_H

#include <QWidget>
#include "YmlStructs.h"
#include "constants.h"

namespace Ui {
class InputPose;
}

class InputPose : public QWidget
{
	Q_OBJECT

public:
	explicit InputPose(QWidget *parent = nullptr);
	void setPose(anim_pose _pose, QString _name);
	anim_pose getPose();
	QString getName();
	void setNameStyleSheet(QString styleSheet);
	~InputPose();

signals:
	void nameChanged(QString newName);
	void animPoseChanged(anim_pose newPose);

public slots:
	void signalChangeName();
	void signalChangeAnimPose();

private:
	Ui::InputPose *ui;
};

#endif // INPUTPOSE_H
