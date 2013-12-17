#ifndef DETECTDIALOG_H
#define DETECTDIALOG_H

#include <QDialog>
#include <QProcess>

#include "ui_detectdialog.h"


class DetectDialog : public QDialog, private Ui::DetectDialog
{
    Q_OBJECT

public:
    DetectDialog(QWidget *parent = 0);

private slots:
    void on_browseButton_clicked();
    void detectImage();
//     void updateOutputTextEdit();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processError(QProcess::ProcessError error);
	void setBrowse();
	void setOkButton();
	void rdEffectCase();

private:
    QProcess process;
    QString targetFile;
};

#endif
