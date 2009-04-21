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
#include <QTextCursor>
#include <QSizeF>
#include <plasma/theme.h>
#include "plasma-emergelog-painter.h"

#define LOG "/var/log/emerge.log"

emergelog::emergelog(QObject *parent, const QVariantList &args) : Plasma::Applet(parent,args)
{
	setAspectRatioMode(Plasma::IgnoreAspectRatio);
	setBackgroundHints(StandardBackground);
	resize(500, 200);
}

emergelog::~emergelog(){
	delete stream;
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

	/* store a small part of the whole emerge.log fail. We dont need to read it all. 
	 * It is huge and we need to respect memory */
	QString cmd = "tail -250 /var/log/emerge.log > "+log;
	int i=system(cmd.toAscii().constData());
	if(i)perror("Error:");

	painter = new emergelog_painter(this);
	/* Measure size */
	calculate_size();
	painter->moveBy(contentsRect().x(),contentsRect().y());
	document=painter->document();
	stream = 0;
	QBrush *brush = new QBrush();
	brush->setColor(Qt::white);// Change this for different font color
	brush->setStyle(Qt::SolidPattern);
	formater = new QTextCharFormat();
	formater->setForeground(*brush);
	watcher = new QFileSystemWatcher(this);
	watcher->addPath(LOG);
	file = new QFile(this);
	document->setMaximumBlockCount((int) (contentsRect().height()));
	QObject::connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(display()));
	QObject::connect(this, SIGNAL(geometryChanged()), this, SLOT(calculate_size()));
	
	painter->update();
	display();
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
	QString cmd = "tail -250 /var/log/emerge.log > "+log;
	int i=system(cmd.toAscii().constData());
	if(i)perror("Error");
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
	QString data= stream->readAll();
	list = data.split('\n', QString::SkipEmptyParts);
	/* read the block */
	for (int i=list.size()-1;i>(list.size()-(document->maximumBlockCount()));i--){
		if(cursor.position() != 0){
			cursor.insertBlock();
		}
		/* do some regexp magic here for proper formatting */
		tmp=list.at(i);
		tmp.replace(QRegExp("^\\d{1,10}:\\s{1,3}"), " ");
		tmp.replace(QRegExp("^\\s{1,3}S"),"  S");
		tmp.replace(QRegExp("::.*"), " ");
		tmp.replace(QRegExp("\\*\\*\\* "), " ");
		tmp.replace(QRegExp(">>> "), " ");
		tmp.replace(QRegExp("=== "),"  ");
		cursor.insertText(tmp,*formater);
	}
	cursor.endEditBlock();
	painter->update();
}

#include "plasma-emergelog.moc"
