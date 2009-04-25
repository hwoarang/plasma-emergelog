//    Copyright (C) 2009  Markos Chandras
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

/* includes */
#include "plasma-emergelog.h"
#include <QPainter>
#include <QFontMetrics>
#include <QFile>
#include <time.h>
#include <QBrush>
#include <QFileSystemWatcher>
#include <QTextDocument>
#include <QTextCharFormat>
#include <QPen>
#include <QRadioButton>
#include <QTextCursor>
#include <QVBoxLayout>
#include <QSizeF>
#include <KConfigDialog>
#include <KConfigGroup>
#include <KMessageBox>
#include <plasma/theme.h>
#include "plasma-emergelog-painter.h"
#include <exception>
#include <iostream>


class myexception: public std::exception
{
	virtual const char* what() const throw()
	{
		return "Cannot read any log files";
	}
} myex;

emergelog::emergelog(QObject *parent, const QVariantList &args) : Plasma::Applet(parent,args)
{
	setAspectRatioMode(Plasma::IgnoreAspectRatio);
	setBackgroundHints(StandardBackground);
	resize(500, 200);
}

emergelog::~emergelog(){
	delete stream;
	delete pmConfig;
	QString cmd = "rm "+log;
	int i=system(cmd.toAscii().constData());
	if(i)perror("Error:");
}

void emergelog::init()
{
	/* Create random file for storring logs */
	int mrand = 0;
	srand( (time(NULL)*rand()));
	mrand= rand()%256;
	log = "/tmp/plasma-emergelog-tmp"+QString::number(mrand);

	KConfigGroup globalCg = globalConfig();
	logFile = globalCg.readEntry("logfile", "/var/log/paludis.log");
	/* Need a fallback code when default value is wrong, otherwise plasma will crash !!!
	 */
	stream = 0;
	file= new QFile(this);
	file->setFileName(logFile);
	if(!file->exists()){
		qDebug("/var/log/paludis.org doesnt exist. Falling back to emerge.log");
		logFile="/var/log/emerge.log";
		file->setFileName(logFile);
		if(!file->exists()){
			qDebug("/var/log/emerge.log doesnt exist. Falling back to pkgcore.log");
			logFile="/var/log/emerge.log";
			file->setFileName(logFile);
			if(!file->exists()){
				// this should never occur on 'working' gentoo systems :)
				KMessageBox::error(pmConfig,i18n("The file %1 doesn't exist.").arg(logFile));
			}
		}
	}
	/* store a small part of the whole emerge.log fail. We dont need to read it all. 
	 * It is huge and we need to respect memory */
	QString cmd = "tail -250 "+logFile+" > "+log;
	int i=system(cmd.toAscii().constData());
	if(i){
		perror("Error:");
		KMessageBox::error(pmConfig,i18n("Permission denied: Cannot open %1. Did you add your self to portage group?").arg(logFile));
	}
	else{
		painter = new emergelog_painter(this);
		/* Measure size */
		calculate_size();
		painter->moveBy(contentsRect().x(),contentsRect().y());
		document=painter->document();
		QBrush *brush = new QBrush();
		brush->setColor(Qt::white);// Change this for different font color
		brush->setStyle(Qt::SolidPattern);
		formater = new QTextCharFormat();
		formater->setForeground(*brush); 
		watcher = new QFileSystemWatcher(this);
		watcher->addPath(logFile);// monitor the logfile (default: /var/log/emerge.log)
	
		/* Blocksize is the height of plasmoid */
		document->setMaximumBlockCount((int) (contentsRect().height()));

		/* 1st slot: when the file change, call display() again to renew contents 
		 * 2nd slot: when the size change, adapt the contents */
		QObject::connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(display()));
		QObject::connect(this, SIGNAL(geometryChanged()), this, SLOT(calculate_size()));
	
		painter->update();
		display();
	}
}

void emergelog::calculate_size(){
	painter->setSize((int)contentsRect().width(),(int)contentsRect().height());
}

void emergelog::display()
{
	/* close previous instances */
	delete stream;
	file->close();
	document->clear();

	/* recreate the file . We have some I/O here but I ll try to avoid this in the future */
	QString cmd = "tail -250 "+logFile+" > "+log;
	int i=system(cmd.toAscii().constData());
	if(i)perror("Error");

	/* open the file */
	file->setFileName(log);
	if(!file->open(QIODevice::ReadOnly | QIODevice::Text)) i18n("Could not open log file");
	stream = new QTextStream(file);
	process_data();
}

void emergelog::process_data(){
	QTextCursor cursor(document);
	cursor.movePosition(QTextCursor::End);
	cursor.beginEditBlock();
	QString tmp;
	QStringList list;

	/* Read all the file. It is not that bad since it is 250 lines so we dont waste much memory */
	QString data= stream->readAll();
	/* Create a list . Each element is a line from that file */
	list = data.split('\n', QString::SkipEmptyParts);
	/* read the block BACKWARDS */
	for (int i=list.size()-1;i>(list.size()-(document->maximumBlockCount()));i--){
		if(cursor.position() != 0){
			cursor.insertBlock();
		}
		/* do some regexp magic here for proper formatting */
		tmp=list.at(i);
		tmp.replace(QRegExp("^\\d{1,10}:\\s{1,3}"), " ");
		tmp.replace(QRegExp("^\\s{1,3}S"),"  S");
		tmp.replace(QRegExp("::/.*"), " ");
		tmp.replace(QRegExp(":::.*")," Finished ;-)");
		tmp.replace(QRegExp("\\*\\*\\* "), " ");
		tmp.replace(QRegExp(">>> "), " ");
		tmp.replace(QRegExp("=== "),"  ");
		tmp.replace(QRegExp("Unmerging.*")," ");
		tmp.replace(QRegExp("terminating.*")," ");
		tmp.replace(QRegExp("exiting.*")," ");
		tmp.replace(QRegExp(".*Merging.*")," ");
		tmp.replace(QRegExp("AUTOCLEAN.*")," ");
		tmp.replace(QRegExp(".*Cleaning.*")," ");
		tmp.replace(QRegExp("unmerge success.*")," ");
		tmp.replace(QRegExp(".*Compiling.*")," ");
		/* Insert the text */
		if(tmp.size()>=10)cursor.insertText(tmp,*formater);
		else {
			if(cursor.position()>0) cursor.setPosition(cursor.position()-1);
		}
	}
	cursor.endEditBlock();
	painter->update();
}

void emergelog::createConfigurationInterface(KConfigDialog *parent)
{
	if(!pmConfig)
		pmConfig = new QWidget();
	
	portageButton = new QRadioButton(i18n("Portage"), pmConfig);
	paludisButton = new QRadioButton(i18n("Paludis"), pmConfig);
	pkgcoreButton = new QRadioButton(i18n("Pkgcore"), pmConfig);
	
	QVBoxLayout *layout = new QVBoxLayout(pmConfig);
	layout->addWidget(portageButton);
	layout->addWidget(paludisButton);
	layout->addWidget(pkgcoreButton);
	
	pmConfig->setLayout(layout);
	parent->addPage(pmConfig, i18n("Package mangler:"));
	
	if(logFile == "/var/log/emerge.log")
		portageButton->setChecked(true);
	else if(logFile == "/var/log/paludis.log")
		paludisButton->setChecked(true);
	else if(logFile == "/var/log/pkcore.log")
		pkgcoreButton->setChecked(true);
	
	connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
	connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void emergelog::configAccepted()
{
	KConfigGroup globalCg = globalConfig();
	watcher->removePath(logFile);
	
	if(portageButton->isChecked()) 
		logFile = "/var/log/emerge.log";
	else if(paludisButton->isChecked())
		logFile = "/var/log/paludis.log";
	else if(pkgcoreButton->isChecked())
		logFile = "/var/log/pkgcore.log";
	
	file->setFileName(logFile); // Use file temporarily so we can prevent the plasmoid from crashing if it doesn't exist
	if(file->exists()) {
		globalCg.writeEntry("logfile", logFile);
	
		emit configNeedsSaving();
	} else {
		KMessageBox::error(pmConfig,i18n("The file %1 doesn't exist.").arg(logFile));
		logFile = globalCg.readEntry("logfile", "/var/log/emerge.log");
	}
	watcher->addPath(logFile);
	painter->update();
	display();
}

#include "plasma-emergelog.moc"
