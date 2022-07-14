//
// opbrowser.cpp - Jaguar Object Processor browser
//
// by James Hammons
// (C) 2012 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JLH  12/01/2012  Created this file
//

// STILL TO DO:
//

#include "opbrowser.h"
#include "jaguar.h"
#include "memory.h"
#include "op.h"


OPBrowserWindow::OPBrowserWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog),
	layout(new QVBoxLayout), text(new QLabel),
	refresh(new QPushButton(tr("Refresh")))
{
	setWindowTitle(tr("OP Browser"));

	// Need to set the size as well...
//	resize(560, 480);

	QFont fixedFont("Lucida Console", 8, QFont::Normal);
//	QFont fixedFont("", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);
	text->setFont(fixedFont);
////	layout->setSizeConstraint(QLayout::SetFixedSize);
	setLayout(layout);

	QScrollArea * scrollArea = new QScrollArea;
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(text);
	layout->addWidget(scrollArea);
	layout->addWidget(refresh);

	connect(refresh, SIGNAL(clicked()), this, SLOT(RefreshContents()));
}


void OPBrowserWindow::RefreshContents(void)
{
	char string[1024];//, buf[64];
	QString opDump;

	if (isVisible())
	{
		uint32_t olp = OPGetListPointer();
		sprintf(string, "OLP = $%X<br>", olp);
		opDump += QString(string);

		numberOfObjects = 0;
		DiscoverObjects(olp);
		DumpObjectList(opDump);

		text->clear();
		text->setText(opDump);
	}
}


void OPBrowserWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
		hide();
	else if (e->key() == Qt::Key_Enter)
		RefreshContents();
}


bool OPBrowserWindow::ObjectExists(uint32_t address)
{
	// Yes, we really do a linear search, every time. :-/
	for(uint32_t i=0; i<numberOfObjects; i++)
	{
		if (address == object[i])
			return true;
	}

	return false;
}


void OPBrowserWindow::DiscoverObjects(uint32_t address)
{
	uint8_t objectType = 0;

	do
	{
		// If we've seen this object already, bail out!
		// Otherwise, add it to the list
		if (ObjectExists(address))
			return;

		object[numberOfObjects++] = address;

		// Get the object & decode its type, link address
		uint32_t hi = JaguarReadLong(address + 0, OP);
		uint32_t lo = JaguarReadLong(address + 4, OP);
		objectType = lo & 0x07;
		uint32_t link = ((hi << 11) | (lo >> 21)) & 0x3FFFF8;

		if (objectType == 3)
		{
			// Branch if YPOS < 2047 (or YPOS > 0) can be treated as a GOTO, so
			// don't do any discovery in that case. Otherwise, have at it:
			if (((lo & 0xFFFF) != 0x7FFB) && ((lo & 0xFFFF) != 0x8003))
				// Recursion needed to follow all links! This does depth-first
				// recursion on the not-taken objects
				DiscoverObjects(address + 8);
		}
		if (objectType == 2 || objectType == 4) {
			address += 8;
		}
		else {
			// Get the next object...
			address = link;
		}
	}
	while (objectType != 4);
}


void OPBrowserWindow::DumpObjectList(QString & list)
{
	const char * opType[8] = {
		"(BITMAP)", "(SCALED BITMAP)", "(GPU INT)", "(BRANCH)",
		"(STOP)", "???", "???", "???"
	};
	const char * ccType[8] = {
		"==", "&lt;", "&gt;", "(opflag set)",
		"(second half line)", "?", "?", "?"
	};
	char buf[512];

	for(uint32_t i=0; i<numberOfObjects; i++)
	{
		uint32_t address = object[i];

		uint32_t hi = JaguarReadLong(address + 0, OP);
		uint32_t lo = JaguarReadLong(address + 4, OP);
		uint8_t objectType = lo & 0x07;
		uint32_t link = ((hi << 11) | (lo >> 21)) & 0x3FFFF8;
		if (objectType != 2 && objectType != 4) {
			sprintf(buf, "<br>%06X: %08X %08X %s -> %06X", address, hi, lo, opType[objectType], link);
		}
		else {
			sprintf(buf, "<br>%06X: %08X %08X %s", address, hi, lo, opType[objectType]);
		}
		list += QString(buf);

		if (objectType == 3)
		{
			uint16_t ypos = (lo >> 3) & 0x7FF;
			uint8_t  cc   = (lo >> 14) & 0x07;	// Proper # of bits == 3
			sprintf(buf, " YPOS %s %u", ccType[cc], ypos);
			list += QString(buf);
		}

		list += "<br>";

		// Yes, the OP really determines bitmap/scaled bitmap address for the
		// following phrases this way...!
		if (objectType == 0)
			DumpFixedObject(list, OPLoadPhrase(address + 0),
				OPLoadPhrase(address | 0x08));

		if (objectType == 1)
			DumpScaledObject(list, OPLoadPhrase(address + 0),
				OPLoadPhrase(address | 0x08), OPLoadPhrase(address | 0x10));

		if (address == link)	// Ruh roh...
		{
			// Runaway recursive link is bad!
			sprintf(buf, "***** SELF REFERENTIAL LINK *****<br>");
			list += QString(buf);
		}
	}

	list += "<br>";
}


void OPBrowserWindow::DumpScaledObject(QString & list, uint64_t p0, uint64_t p1, uint64_t p2)
{
	char buf[512];

	sprintf(buf, "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;%08X %08X<br>", (uint32_t)(p1 >> 32), (uint32_t)(p1 & 0xFFFFFFFF));
	list += QString(buf);
	sprintf(buf, "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;%08X %08X<br>", (uint32_t)(p2 >> 32), (uint32_t)(p2 & 0xFFFFFFFF));
	list += QString(buf);
	DumpBitmapCore(list, p0, p1);
	uint32_t hscale = p2 & 0xFF;
	uint32_t vscale = (p2 >> 8) & 0xFF;
	uint32_t remainder = (p2 >> 16) & 0xFF;
	sprintf(buf, "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[hsc: %02X, vsc: %02X, rem: %02X]<br>", hscale, vscale, remainder);
	list += QString(buf);
}


void OPBrowserWindow::DumpFixedObject(QString & list, uint64_t p0, uint64_t p1)
{
	char buf[512];

	sprintf(buf, "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;%08X %08X<br>", (uint32_t)(p1 >> 32), (uint32_t)(p1 & 0xFFFFFFFF));
	list += QString(buf);
	DumpBitmapCore(list, p0, p1);
}


void OPBrowserWindow::DumpBitmapCore(QString & list, uint64_t p0, uint64_t p1)
{
	char buf[512];
	uint8_t op_bitmap_bit_depth[8] = { 1, 2, 4, 8, 16, 24, 32, 0 };

	uint32_t bdMultiplier[8] = { 64, 32, 16, 8, 4, 2, 1, 1 };
	uint8_t bitdepth = (p1 >> 12) & 0x07;
//WAS:	int16 ypos = ((p0 >> 3) & 0x3FF);			// ??? What if not interlaced (/2)?
	int16_t ypos = ((p0 >> 3) & 0x7FF);			// ??? What if not interlaced (/2)?
	int32_t xpos = p1 & 0xFFF;
	xpos = (xpos & 0x800 ? xpos | 0xFFFFF000 : xpos);	// Sign extend that mutha!
	uint32_t iwidth = ((p1 >> 28) & 0x3FF);
	uint32_t dwidth = ((p1 >> 18) & 0x3FF);		// Unsigned!
	uint16_t height = ((p0 >> 14) & 0x3FF);
	uint32_t link = ((p0 >> 24) & 0x7FFFF) << 3;
	uint32_t ptr = ((p0 >> 43) & 0x1FFFFF) << 3;
	uint32_t firstPix = (p1 >> 49) & 0x3F;
	uint8_t flags = (p1 >> 45) & 0x0F;
	uint8_t idx = (p1 >> 38) & 0x7F;
	uint32_t pitch = (p1 >> 15) & 0x07;

	sprintf(buf, "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[%u x %u @ (%i, %u) (iw:%u, dw:%u) (%u bpp), p:%06X fp:%02X, fl:%s%s%s%s, idx:%02X, pt:%02X]<br>",
		iwidth * bdMultiplier[bitdepth],
		height, xpos, ypos, iwidth, dwidth, op_bitmap_bit_depth[bitdepth],
		ptr, firstPix, (flags & OPFLAG_REFLECT ? "REFLECT " : ""),
		(flags & OPFLAG_RMW ? "RMW " : ""), (flags & OPFLAG_TRANS ? "TRANS " : ""),
		(flags & OPFLAG_RELEASE ? "RELEASE" : ""), idx, pitch);
	list += QString(buf);
}

