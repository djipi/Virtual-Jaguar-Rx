//
// controllerwidget.cpp: A widget for changing "Controller" configuration
//
// Part of the Virtual Jaguar Project
// (C) 2011 Underground Software
// See the README and GPLv3 files for licensing and warranty information
//
// JLH = James Hammons <jlhamm@acm.org>
//
// WHO  WHEN        WHAT
// ---  ----------  ------------------------------------------------------------
// JLH  07/20/2011  Created this file
//

#include "controllerwidget.h"

#include "joystick.h"
#include "gamepad.h"
#include "keygrabber.h"


// These tables are used to convert Qt keycodes into human readable form. Note
// that a lot of these are just filler.
char ControllerWidget::keyName1[96][16] = {
	"Space",
	"!", "\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/",
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?",
	"@", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N",
	"O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
	"[", "\\", "]", "^", "_", "`",
	"$61", "$62", "$63", "$64", "$65", "$66", "$67", "$68", "$69", "$6A", "$6B", "$6C", "$6D", 
	"$6E", "$6F", "$70", "$71", "$72", "$73", "$74", "$75", "$76", "$77", "$78", "$79", "$7A", 
	"{", "|", "}", "~"
};

char ControllerWidget::keyName2[64][16] = {
	"Esc", "Tab", "BTab", "BS", "Ret", "Ent", "Ins", "Del", "Pause", "Prt", "SRq", "Clr",
	"$C", "$D", "$E", "$F", "Hm", "End", "Lf", "Up", "Rt", "Dn", "PgU", "PgD", "$18",
	"$19", "$1A", "$1B", "$1C", "$1D", "$1E", "$1F", "Shf", "Ctl", "Mta", "Alt",
	"Cap", "Num", "ScL", "$27", "$28", "$29", "$2A", "$2B", "$2C", "$2D", "$2E", "$2F",
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13",
	"F14", "F15", "F16"
};

char ControllerWidget::hatName[4][16] = { "Up", "Rt", "Dn", "Lf" };

char ControllerWidget::axisName[2][8] = { "+", "-" };

// This is hard-coded crap. It's crap-tastic!
// These are the positions to draw the button names at, ordered by the BUTTON_*
// sequence found in joystick.h.
int ControllerWidget::buttonPos[21][2] = { { 74, 32 }, { 71, 67 }, { 53, 49 }, { 93, 49 },
	{ 110, 200 }, { 110, 175 }, { 110, 151 }, { 110, 126 },
	{ 148, 200 }, { 148, 175 }, { 148, 151 }, { 148, 126 },
	{ 186, 200 }, { 186, 175 }, { 186, 151 }, { 186, 126 },
	{ 234, 31 }, { 216, 51 }, { 199, 71 }, { 164-11, 101-30 }, { 141-11, 108+13-30 }
};


ControllerWidget::ControllerWidget(QWidget * parent/*= 0*/): QWidget(parent),
	controllerPic(":/res/controller.png"), widgetSize(controllerPic.size()),
	keyToHighlight(-1), mouseDown(false)
{
	// Seems we have to pad this stuff, otherwise it clips on the right side
	widgetSize += QSize(4, 4);
	// We want to know when the mouse is moving over our widget...
	setMouseTracking(true);
//nope
//setFixedSize(widgetSize);
}


ControllerWidget::~ControllerWidget()
{
}


QSize ControllerWidget::sizeHint(void) const
{
	return widgetSize;
}


QSizePolicy ControllerWidget::sizePolicy(void) const
{
	return QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}


void ControllerWidget::paintEvent(QPaintEvent * /*event*/)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.drawImage(QPoint(0, 0), controllerPic);

	// Bump up the size of the default font...
	QFont font = painter.font();
	font.setPixelSize(15);
	font.setBold(true);
	painter.setFont(font);
//	painter.setPen(QColor(48, 255, 255, 255));	// This is R,G,B,A
	painter.setPen(QColor(0, 0, 0, 255));		// This is R,G,B,A
	painter.setBrush(QBrush(QColor(48, 255, 255, 255)));

	// First, draw black oversize line, then dot, then colored line
	QPen blackPen(QColor(0, 0, 0, 255));
	blackPen.setWidth(4);
	QPen colorPen(QColor(48, 255, 255, 255));
	colorPen.setWidth(2);
	QLine line(QPoint(141-11, 100-30), QPoint(141-11, 108+5-30));

	painter.setPen(blackPen);
	painter.drawLine(line);
	blackPen.setWidth(1);
	painter.setPen(blackPen);
	painter.drawEllipse(QPoint(141-11, 100-30), 4, 4);
	painter.setPen(colorPen);
	painter.drawLine(line);

//#define DEBUG_CWPAINT
#ifdef DEBUG_CWPAINT
printf("------------------------------\n");
#endif
	for(int i=BUTTON_FIRST; i<=BUTTON_LAST; i++)
	{
		if (keyToHighlight == i)
		{
			painter.setPen(QColor(255, 48, 255, 255));	// This is R,G,B,A
			font.setPixelSize(mouseDown ? 15 : 18);
			painter.setFont(font);
		}
		else
		{
			painter.setPen(QColor(48, 255, 255, 255));	// This is R,G,B,A
			font.setPixelSize(15);
			painter.setFont(font);
		}

#ifdef DEBUG_CWPAINT
printf("key %02i: ", i);
#endif

		if (keys[i] < 0x80)
#ifdef DEBUG_CWPAINT
{
printf("Drawing a key < 0x80 [keys[i]=%X, keyname=%s]...\n", keys[i], keyName1[keys[i] - 0x20]);
#endif
			DrawBorderedText(painter, buttonPos[i][0], buttonPos[i][1],
				QString(keyName1[keys[i] - 0x20]));
#ifdef DEBUG_CWPAINT
}
#endif
		else if ((keys[i] & 0xFFFFFF00) == 0x01000000)
		{
#ifdef DEBUG_CWPAINT
printf("Drawing a key with bit 48 set...\n");
#endif
			DrawBorderedText(painter, buttonPos[i][0], buttonPos[i][1],
				QString(keyName2[keys[i] & 0x3F]));
		}
#if 1
		else if (keys[i] & JOY_BUTTON)
		{
#ifdef DEBUG_CWPAINT
printf("Drawing a joystick button...\n");
#endif
			DrawBorderedText(painter, buttonPos[i][0], buttonPos[i][1],
				QString("JB%1").arg(keys[i] & JOY_BUTTON_MASK));
		}
		else if (keys[i] & JOY_HAT)
		{
#ifdef DEBUG_CWPAINT
printf("Drawing a joystick hat...\n");
#endif
			DrawBorderedText(painter, buttonPos[i][0], buttonPos[i][1],
				QString("j%1").arg(hatName[keys[i] & JOY_BUTTON_MASK]));
		}
		else if (keys[i] & JOY_AXIS)
		{
#ifdef DEBUG_CWPAINT
printf("Drawing a joystick axis...\n");
#endif
			DrawBorderedText(painter, buttonPos[i][0], buttonPos[i][1],
				QString("JA%1%2").arg((keys[i] & JOY_AXISNUM_MASK) >> 1).arg(axisName[keys[i] & JOY_AXISDIR_MASK]));
		}
#endif
		else
#ifdef DEBUG_CWPAINT
{
printf("Drawing ???...\n");
#endif
			DrawBorderedText(painter, buttonPos[i][0], buttonPos[i][1], QString("???"));
#ifdef DEBUG_CWPAINT
}
#endif
	}
}


void ControllerWidget::mousePressEvent(QMouseEvent * /*event*/)
{
	mouseDown = true;
	update();
}


void ControllerWidget::mouseReleaseEvent(QMouseEvent * /*event*/)
{
	mouseDown = false;
	// Spawning the keygrabber causes leaveEvent() to be called, so we need to
	// save this
	int keyToHighlightSave = keyToHighlight;

	KeyGrabber keyGrab(this);
	keyGrab.SetKeyText(keyToHighlightSave);
	keyGrab.exec();
	int key = keyGrab.key;

	if (key != Qt::Key_Escape)
	{
		keys[keyToHighlightSave] = key;
		emit(KeyDefined(keyToHighlightSave, key));
	}

	keyToHighlight = keyToHighlightSave;
	update();
}


void ControllerWidget::mouseMoveEvent(QMouseEvent * event)
{
	if (mouseDown)
		return;

	// Save the current closest item
	int keyToHighlightOld = keyToHighlight;
	// Set up closest distance (this should be large enough)
	double closest = 1e9;

	for(int i=BUTTON_FIRST; i<=BUTTON_LAST; i++)
	{
		// We loop through the button text positions, to see which one is
		// closest.
		double distX = (double)(event->x() - buttonPos[i][0]);
		double distY = (double)(event->y() - buttonPos[i][1]);
		double currentDistance = sqrt((distX * distX) + (distY * distY));

		if (currentDistance < closest)
		{
			closest = currentDistance;
			keyToHighlight = i;
		}
	}

	if (keyToHighlightOld != keyToHighlight)
		update();
}


void ControllerWidget::leaveEvent(QEvent * /*event*/)
{
	keyToHighlight = -1;
	update();
}


void ControllerWidget::DrawBorderedText(QPainter & painter, int x, int y, QString text)
{
	// Text is drawn centered at (x, y) as well, using a bounding rect for the
	// purpose.
	QRect rect(0, 0, 60, 30);
	QPen oldPen = painter.pen();
	painter.setPen(QColor(0, 0, 0, 255));		// This is R,G,B,A

	for(int i=-1; i<=1; i++)
	{
		for(int j=-1; j<=1; j++)
		{
			rect.moveCenter(QPoint(x + i, y + j));
			painter.drawText(rect, Qt::AlignCenter, text);
		}
	}

	painter.setPen(oldPen);
	rect.moveCenter(QPoint(x, y));
	painter.drawText(rect, Qt::AlignCenter, text);
}

