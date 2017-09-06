//
// about.cpp - Credits
//
// by James Hammons
// (C) 2010 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  01/21/2010  Created this file
// JLH  01/22/2010  Fleshed out the credits a bit more
// JLH  01/22/2010  Fixed centering and decorating of window
// JLH  10/08/2011  Updated credits, added Esc & Return as exit keys
// JPM  08/14/2017  Updated credits, added flynn
// JPM  09/06/2017  Updated flynn credit line to be more specific about the work
//

// STILL TO DO:
//

#include "about.h"
#include "version.h"


AboutWindow::AboutWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog)
{
	setWindowTitle(tr("About Virtual Jaguar..."));

	layout = new QVBoxLayout();
	layout->setSizeConstraint(QLayout::SetFixedSize);
	setLayout(layout);

	QString s;
	s.append(tr(
		"<img src=':/res/vj_title_small.png' style='float: right'>"
		"<table>"
		"<tr><td align='right'><b>Version: </b></td><td>"
		VJ_RELEASE_VERSION " (" VJ_RELEASE_SUBVERSION ") Rx"
		"</td></tr>"
		"<tr><td align='right'><b>Coders: </b></td><td>James Hammons (shamus)<br>Niels Wagenaar (nwagenaar)<br>Carwin Jones (Caz)<br>Adam Green</td></tr>"
		"<tr><td align='right'><b>Testers: </b></td><td>Cyrano Jones, LinkoVitch, partycle, ggn,<br>neo-rg, Robert R, TheUMan, Dissection,<br>overridex, geormetal</td></tr>"
		"<tr><td align='right'><b>Build Team: </b></td><td>shamus (win32)<br>goldenegg (MacOS)</td></tr>"
		"<tr><td align='right'><b>Homepage: </b></td><td>http://icculus.org/virtualjaguar/</td></tr>"
		"</table>"
		"<br><br>"
		"<i>Rx version:<i>"
		"<br>"
		"<b>Flynn</b> for the debugger integration, ELF/DWARF format, Win64 & Visual Studio 2015 support"
		"<br><br>"
		"<i>The Virtual Jaguar team would like to express their gratitude to:</i>"
		"<br><br>"
		"<b>Aaron Giles</b> for the original CoJag sources<br>"
		"<b>David Raingeard</b> for the original Virtual Jaguar sources<br>"
		"<b>Bernd Schmidt</b> for his UAE 68K emulator<br>"
		"<b>Sam Lantinga</b> for his amazing SDL libraries<br>"
		"<b>Ryan C. Gordon</b> for Virtual Jaguar's web presence<br>"
		"<b>Curt Vendel</b> for various Jaguar & other goodies<br>"
		"<b>Reboot</b> for reasons too numerous to mention<br>"
		"The guys over at <b>Atari Age</b> :-)<br>"
		"<b>byuu</b> for <s>BSNES</s> Higan and showing us what was possible<br>"
	));
	text = new QLabel(s);
	layout->addWidget(text);
}


void AboutWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape || e->key() == Qt::Key_Return)
		hide();
}
