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
#ifndef EMERGELOG_PAINTER_H
#define EMERGELOG_PAINTER_H

#include <QGraphicsTextItem>

class emergelog_painter : public QGraphicsTextItem{
	Q_OBJECT

	public:
		emergelog_painter(QGraphicsItem * parent = 0);
		void setSize(int w,int h);

	protected:
		virtual QRectF boundingRect() const;
	
	private:
		int m_w;
		int m_h;
};

#endif
