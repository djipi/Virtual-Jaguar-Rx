//
// imagedelegate.h: Class definition
//

#ifndef __IMAGEDELEGATE_H__
#define __IMAGEDELEGATE_H__

#include <QtWidgets>

class ImageDelegate: public QAbstractItemDelegate
{
//	Q_OBJECT

	public:
//		ImageDelegate(QObject * parent = 0);
		ImageDelegate();

		void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
		QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

//	public slots:
//		void setPixelSize(int size);

	private:
//		int pixelSize;
		QImage cartSmall;
};

#endif	// __IMAGEDELEGATE_H__
