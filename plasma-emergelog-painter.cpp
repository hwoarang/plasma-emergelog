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
#include "plasma-emergelog-painter.h"

#include <QRectF>

emergelog_painter::emergelog_painter(QGraphicsItem * parent) : QGraphicsTextItem(parent){}

QRectF emergelog_painter::boundingRect() const
{
	return QRectF(0, 0, m_w, m_h);
}

void emergelog_painter::setSize(int w,int h){
	m_w=w;
	m_h=h;
	prepareGeometryChange();
}
