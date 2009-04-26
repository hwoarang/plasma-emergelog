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
#ifndef EMERGELOG_H
#define EMERGELOG_H

/* basic includes */

#include <QtCore/QPointer>
#include <Plasma/Applet>

class KConfigDialog;
class QSizeF;
class QFile;
class QFileSystemWatcher;
class QRadioButton;
class QTextDocument;
class emergelog_painter;
class QTextCharFormat;
class QPen;
/* Main Class */

class emergelog : public Plasma::Applet
{
	Q_OBJECT
	public:
		emergelog(QObject *parent, const QVariantList &args);
		~emergelog();

		//void paintInterface(QPainter *painer, const QStyleOptionGraphicsItem *option, const QRect& contentsRect);
		void init();
		void process_data();
		bool valid;
        
	protected:
		void createConfigurationInterface(KConfigDialog *parent);

	public slots:
		void display(bool valid);
	private slots:
		void calculate_size();
		void configAccepted();

	private:
		QFileSystemWatcher *watcher;
		QFile *file;
		QTextStream *stream;
		QTextDocument *document;
		emergelog_painter *painter;
		QPen *pen;
		QPointer<QWidget> pmConfig;
		QTextCharFormat *formater;
		QRadioButton *portageButton;
		QRadioButton *paludisButton;
		QRadioButton *pkgcoreButton;
		QString log;
		QString logFile;
};
K_EXPORT_PLASMA_APPLET(emergelog, emergelog)
#endif
