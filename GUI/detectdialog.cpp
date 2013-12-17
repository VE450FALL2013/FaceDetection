#include <QtGui>
#include "detectdialog.h"
#include <iostream>
#include <string.h>
#include<iostream>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/shm.h>

QString fileName;
key_t key=ftok("/etc",10);
int shmid;
int *effectCase;
  
DetectDialog::DetectDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
	shmid=shmget(key,4096,IPC_CREAT|IPC_EXCL|0666);
	if(shmid == -1)	{
		shmid = shmget(key,0,0);
	}
	effectCase=(int*)shmat(shmid,NULL,0);
	*effectCase=0;  
    QPushButton *detectButton =
            buttonBox->button(QDialogButtonBox::Ok);
    detectButton->setText(tr("&Detect"));
    detectButton->setEnabled(false);
	browseButton->setEnabled(false);
	


	connect(ImageRadioButton, SIGNAL(clicked()),
            this, SLOT(setBrowse()));
	connect(VideoRadioButton, SIGNAL(clicked()),
            this, SLOT(setBrowse()));
	connect(squareRadioButton, SIGNAL(clicked()),
            this, SLOT(setBrowse()));
	connect(mosaicRadioButton, SIGNAL(clicked()),
            this, SLOT(setBrowse()));
	connect(otherRadioButton, SIGNAL(clicked()),
            this, SLOT(setBrowse()));
	connect(ImageRadioButton, SIGNAL(clicked()), this, SLOT(setOkButton()));	
	connect(sourceFileEdit, SIGNAL(textChanged(QString)), this, SLOT(setOkButton()));
	
	//read effect case
	connect(squareRadioButton,SIGNAL(clicked()),this,SLOT(rdEffectCase()));
	connect(mosaicRadioButton,SIGNAL(clicked()),this,SLOT(rdEffectCase()));
	connect(otherRadioButton,SIGNAL(clicked()),this,SLOT(rdEffectCase()));

    connect(detectButton, SIGNAL(clicked()),
            this, SLOT(detectImage()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
//     connect(&process, SIGNAL(readyReadStandardError()),
//             this, SLOT(updateOutputTextEdit()));
    connect(&process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(processFinished(int, QProcess::ExitStatus)));
    connect(&process, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(processError(QProcess::ProcessError)));
}

void DetectDialog::on_browseButton_clicked()
{
    QString initialName = sourceFileEdit->text();
    if (initialName.isEmpty())
        initialName = QDir::homePath();
    fileName =
            QFileDialog::getOpenFileName(this, tr("Choose File"),
                                         initialName);
    fileName = QDir::toNativeSeparators(fileName);
    if (!fileName.isEmpty()) {
        sourceFileEdit->setText(fileName);
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}

void DetectDialog::setBrowse()
{
	if (ImageRadioButton->isChecked() && (squareRadioButton->isChecked()||mosaicRadioButton->isChecked()||otherRadioButton->isChecked()))	{
		browseButton->setEnabled(true);
	}
	if (VideoRadioButton->isChecked() && (squareRadioButton->isChecked()||mosaicRadioButton->isChecked()||otherRadioButton->isChecked()))	{
		browseButton->setEnabled(false);
		sourceFileEdit->setText(NULL);
		buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
	}
}

void DetectDialog::setOkButton()
{
	QString text = sourceFileEdit->text();

	if (text.isEmpty())	{
		buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	}	
	else	{
		buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
	}
}

void DetectDialog::detectImage()
{
    QString sourceFile = sourceFileEdit->text();
    
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    outputTextEdit->clear();
	
	QStringList args;
	if (VideoRadioButton->isChecked())	{
 			args << "0";	
			process.start("/home/linux789/Desktop/git/FaceDetection/face", args);
			sourceFileEdit->setText(NULL);
	}		
	
	else if (ImageRadioButton->isChecked())	{			
			args << sourceFile;
			process.start("/home/linux789/Desktop/git/FaceDetection/face", args);
	}
	outputTextEdit->append(tr("Press any key to exit"));
}

// void DetectDialog::updateOutputTextEdit(void)
// {
//     QByteArray newData = process.readAllStandardError();
//     QString text = outputTextEdit->toPlainText()
//                    + QString::fromLocal8Bit(newData);
//     outputTextEdit->setPlainText(text);
// }

void DetectDialog::processFinished(int exitCode,
                                    QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit) {
        outputTextEdit->append(tr("Detection program crashed"));
    } else if (exitCode != 0) {
        outputTextEdit->append(tr("Detection failed"));
    } else	{
		outputTextEdit->append(tr("Detection succeed"));
	}
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void DetectDialog::processError(QProcess::ProcessError error)
{
    if (error == QProcess::FailedToStart) {
        outputTextEdit->append(tr("Detection program not found"));
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}

void DetectDialog::rdEffectCase()	{
	effectCase=(int*)shmat(shmid,NULL,0);
	if(squareRadioButton->isChecked())	{
		*effectCase=0;
		shmdt(effectCase);
	}
	else if(mosaicRadioButton->isChecked())	{
		*effectCase=1;
		shmdt(effectCase);
	}
	else if(otherRadioButton->isChecked())	{
		 *effectCase=2;  
		 shmdt(effectCase);
	}
	else{
		*effectCase=0;
		shmdt(effectCase);
	}
}
