//
//
// C++ Implementation: Sonogram
//
// Description: 
//
//
// Author: Melchior FRANZ <mfranz@kde.org>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <qpainter.h>
#include <qpixmap.h>
#include <vector>
#include "sonogram.h"

Sonogram::Sonogram(QWidget *parent, const char *name) :
	AnalyzerBase(16, parent, name),
	m_pPixmap(0)
{
}


Sonogram::~Sonogram()
{
	delete m_pPixmap;
}


void Sonogram::init()
{
	m_pPixmap = new QPixmap(width(), height());
	QPainter p(m_pPixmap);
	p.setBackgroundColor(Qt::black);
	p.eraseRect(0, 0, m_pPixmap->width(), m_pPixmap->height());
}


void Sonogram::drawAnalyzer(std::vector<float> *s)
{
	int x = width() - 1;
	QColor c;
	QPainter p(m_pPixmap);

	bitBlt(m_pPixmap, 0, 0, m_pPixmap, 1, 0, x, height());
	std::vector<float>::iterator it = s->begin();
	for (uint y = height() - 1; y && it < s->end(); it++) {
		if (*it < .005)
			c = Qt::black;
		else if (*it < .05)
			c.setHsv(95, 255, 255 - int(*it * 4000.0));
		else if (*it < 1.0)
			c.setHsv(95 - int(*it * 90.0), 255, 255);
		else
			c = Qt::red;

		p.setPen(c);
		p.drawPoint(x, y--);
	}
	bitBlt(this, 0, 0, m_pPixmap);
}

#include "sonogram.moc"
