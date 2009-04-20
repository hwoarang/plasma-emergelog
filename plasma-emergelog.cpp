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
#include <QFileSystemWatcher>
#include <QTextDocument>
#include <QTextCursor>
#include <QSizeF>
#include <plasma/theme.h>
#include "plasma-emergelog-painter.h"

#define LOG "/var/log/emerge.log"

emergelog::emergelog(QObject *parent, const QVariantList &args) : Plasma::Applet(parent,args)
{
	setAspectRatioMode(Plasma::IgnoreAspectRatio);
	setBackgroundHints(TranslucentBackground);
	resize(500, 200);
}

emergelog::~emergelog(){
	delete this;
}

void emergelog::init()
{
	painter = new emergelog_painter(this);
	calculate_size();
	document=painter->document();
	stream = 0;
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
	painter->moveBy(contentsRect().x(),contentsRect().y());
	painter->setSize((int)contentsRect().width(),(int)contentsRect().height());
}

void emergelog::display()
{
	/* close previous instances */
	delete stream;
	file->close();
	document->clear();
	file->setFileName(LOG);
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
		tmp.replace(QRegExp("^\\d{1,10}:  "), " ");
		tmp.replace(QRegExp("::.*"), " ");
		cursor.insertText(tmp);
	}
	cursor.endEditBlock();
	painter->update();
}

#include "plasma-emergelog.moc"
