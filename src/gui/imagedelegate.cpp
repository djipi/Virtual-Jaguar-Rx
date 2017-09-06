//
// imagedelegate.cpp - Qt Model/View rendering class
//
// by James Hammons
// (C) 2010 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>

//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  02/04/2010  Created this file
// JPM  06/06/2016  Visual Studio support
//

// This class takes care of rendering items in our custom model in the ListView
// class utilized in FilePicker.

#include "imagedelegate.h"

#include "filedb.h"
#include "filelistmodel.h"

//ImageDelegate::ImageDelegate(QObject * parent): QAbstractItemDelegate(parent)//, pixelSize(12)
//{
//}

ImageDelegate::ImageDelegate()
{
	QImage cartImg(":/res/cart-blank.png");
	QPainter painter(&cartImg);
	painter.drawPixmap(23, 87, QPixmap(":/res/label-blank.png"));
	painter.end();
	cartSmall = cartImg.scaled(488/4, 395/4, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

/*
Each item is rendered by the delegate's paint() function. The view calls this function with a ready-to-use QPainter object, style information that the delegate should use to correctly draw the item, and an index to the item in the model:
*/

void ImageDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	if (option.state & QStyle::State_Selected)
		painter->fillRect(option.rect, option.palette.highlight());

/*
The first task the delegate has to perform is to draw the item's background correctly. Usually, selected items appear differently to non-selected items, so we begin by testing the state passed in the style option and filling the background if necessary.

The radius of each circle is calculated in the following lines of code:
*/

#if 0
	int size = qMin(option.rect.width(), option.rect.height());
	int brightness = index.model()->data(index, Qt::DisplayRole).toInt();
	double radius = (size/2.0) - (brightness/255.0 * size/2.0);
	if (radius == 0.0)
		return;
#endif

/*
First, the largest possible radius of the circle is determined by taking the smallest dimension of the style option's rect attribute. Using the model index supplied, we obtain a value for the brightness of the relevant pixel in the image. The radius of the circle is calculated by scaling the brightness to fit within the item and subtracting it from the largest possible radius.
*/

	painter->save();
#if 0
	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->setPen(Qt::NoPen);

/*
We save the painter's state, turn on antialiasing (to obtain smoother curves), and turn off the pen.
*/

	if (option.state & QStyle::State_Selected)
		painter->setBrush(option.palette.highlightedText());
	else
		painter->setBrush(QBrush(Qt::black));

/*
The foreground of the item (the circle representing a pixel) must be rendered using an appropriate brush. For unselected items, we will use a solid black brush; selected items are drawn using a predefined brush from the style option's palette.
*/

	painter->drawEllipse(QRectF(option.rect.x() + option.rect.width()/2 - radius,
		option.rect.y() + option.rect.height()/2 - radius, 2*radius, 2*radius));
#else
//	painter->drawPixmap(option.rect.x()+8, option.rect.y()+8, 200, 94, QPixmap(":/res/labels/rayman.jpg"));
//	painter->drawPixmap(option.rect.x()+13, option.rect.y()+51, 433/2, 203/2, QPixmap(":/res/labels/rayman.jpg"));
//	painter->drawPixmap(option.rect.x(), option.rect.y(), 488/2, 395/2, QPixmap(":/res/cart-blank.png"));


	// This is crappy. We really should have a properly scaled image ready to go so we
	// don't get Qt's default ugly looking fast scaling...

#ifdef _MSC_VER
#pragma message("Warning: !!! FIX !!! Need to create properly scaled down cart/label images!")
#else
#warning "!!! FIX !!! Need to create properly scaled down cart/label images!"
#endif // _MSC_VER
//We've got the carts, now just need to do the labels...

	unsigned long i = index.model()->data(index, FLM_INDEX).toUInt();
	QString filename = index.model()->data(index, FLM_FILENAME).toString();
	QImage label = index.model()->data(index, FLM_LABEL).value<QImage>();
	QString nameToDraw;

	if (i == 0xFFFFFFFF)	// Not found...
	{
		int lastSlashPos = filename.lastIndexOf('/');
		nameToDraw = "\"" + filename.mid(lastSlashPos + 1) + "\"";
	}
	else
		nameToDraw = romList[i].name;

	if (label.isNull())
	{
//	painter->drawPixmap(option.rect.x()+14, option.rect.y()+50, 433/2, 203/2, QPixmap(":/res/label-blank.png"));
//		painter->drawPixmap(option.rect.x()+7, option.rect.y()+25, 433/4, 203/4, QPixmap(":/res/label-blank.png"));
		painter->drawImage(option.rect.x() + 2, option.rect.y() + 2, cartSmall);
//Need to query the model for the data we're supposed to draw here...
//	painter->drawText(17, 73, QString(romList[i].name));
//	painter->setPen(Qt::white);
		painter->setPen(QColor(255, 128, 0, 255));
//	painter->drawText(QRect(option.rect.x()+20, option.rect.y()+73, 196, 70), Qt::TextWordWrap | Qt::AlignHCenter, QString(romList[i].name));
		painter->drawText(QRect(option.rect.x()+10, option.rect.y()+36, 196/2, 70/2),
			Qt::TextWordWrap | Qt::AlignHCenter, nameToDraw);
	}
	else
	{
#if 0
		QString filename(romList[i].file);
		filename.prepend("./label/");
		QImage img(filename);
		painter->drawImage(QRect(option.rect.x()+7, option.rect.y()+25, 433/4, 203/4), img);
#else
		painter->drawPixmap(option.rect.x() + 2, option.rect.y() + 2, 488/4, 395/4, QPixmap(":/res/cart-blank.png"));
		painter->drawImage(QRect(option.rect.x()+2+7, option.rect.y()+2+25, 433/4, 203/4), label);
#endif
	}
//26x100
#endif
	painter->restore();
}

/*
Finally, we paint the circle within the rectangle specified by the style option and we call restore() on the painter.

The paint() function does not have to be particularly complicated; it is only necessary to ensure that the state of the painter when the function returns is the same as it was when it was called. This usually means that any transformations applied to the painter must be preceded by a call to QPainter::save() and followed by a call to QPainter::restore().

The delegate's sizeHint() function returns a size for the item based on the predefined pixel size, initially set up in the constructor:
*/

QSize ImageDelegate::sizeHint(const QStyleOptionViewItem & /* option */, const QModelIndex & /* index */) const
{
	// 488x395 --> blank cart (full size)
	// 400x188 --> label (full size) 433x203 <-- (actually, it's this)

	// 200x94 is shrunk dimension...
//	return QSize(100, 47);
//	return QSize(216, 110);
//	return QSize(488/2, 395/2);
	return QSize((488/4) + 4, (395/4) + 4);
}

/*
The delegate's size is updated whenever the pixel size is changed. We provide a custom slot to do this:
*/

//void ImageDelegate::setPixelSize(int size)
//{
//	pixelSize = size;
//}
