#ifndef DIALOGCHANGESECTION_H
#define DIALOGCHANGESECTION_H

#include <QDialog>
#include "SocketSettingsForm.h"
#include "ui_SocketSettingsForm.h"
#include "YmlStructs.h"

namespace Ui {
class DialogChangeSection;
}

class DialogChangeSection : public QDialog
{
    Q_OBJECT

private:
    QRegExpValidator validator;
	QStringList sectionsLst;
    sectionLink* sLink;

public:
	QVector<SocketSettingsForm*> sk;
    explicit DialogChangeSection(QWidget *parent = nullptr);
	~DialogChangeSection() override;
    void updateChoiceForms(sectionLink* link, QStringList sectionsList);
	void fillLinkData();

    Ui::DialogChangeSection *ui;

public slots:
    //void onClicked_Start();
    void accept() override;
    void onClicked_Next();
    void onClicked_Cond(bool enable);
    void onClicked_Script();
    void onClicked_Choice();
    void onClicked_Random();
    void onClicked_Exit();
    void onSectionNameChanged(QString s);
    void onTimeLimitChanged(double tld);
    void onTimeLimitChanged(int tli);
    void onTimeLimitClicked(bool);
};

#endif // DIALOGCHANGESECTION_H
