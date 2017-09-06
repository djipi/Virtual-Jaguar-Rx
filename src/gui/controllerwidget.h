#ifndef __CONTROLLERWIDGET_H__
#define __CONTROLLERWIDGET_H__

#include <QtWidgets>
#include <stdint.h>

class ControllerWidget: public QWidget
{
	Q_OBJECT

	public:
		ControllerWidget(QWidget * parent = 0);
		~ControllerWidget();
		QSize sizeHint(void) const;
		QSizePolicy sizePolicy(void) const;

	protected:
		void paintEvent(QPaintEvent *);
		void mousePressEvent(QMouseEvent *);
		void mouseReleaseEvent(QMouseEvent *);
		void mouseMoveEvent(QMouseEvent *);
		void leaveEvent(QEvent *);

	private:
		void DrawBorderedText(QPainter &, int, int, QString);

	signals:
		void KeyDefined(int, uint32_t);

	public:
		uint32_t keys[21];

	private:
		QImage controllerPic;
		QSize widgetSize;
		int keyToHighlight;
		bool mouseDown;

		// Class data
		static char keyName1[96][16];
		static char keyName2[64][16];
		static char hatName[4][16];
		static char axisName[2][8];
		static int buttonPos[21][2];
};

#endif	// __CONTROLLERWIDGET_H__
