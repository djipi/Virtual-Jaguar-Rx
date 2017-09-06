// Implementation of OpenGL widget using Qt
//
// by James Hammons
// (C) 2010 Underground Software

#ifndef __GLWIDGET_H__
#define __GLWIDGET_H__

#include <QGLWidget>
#include <stdint.h>

class GLWidget: public QGLWidget
{
	Q_OBJECT

	public:
		GLWidget(QWidget * parent = 0);
		~GLWidget();

		void HandleMouseHiding(void);
		void CheckAndRestoreMouseCursor(void);
//		QSize minimumSizeHint() const;
//		QSize sizeHint() const;

//	signals:
//		void clicked();

	protected:
		void initializeGL(void);
		void paintGL(void);
		void resizeGL(int width, int height);
		void mouseMoveEvent(QMouseEvent *);
//		void mousePressEvent(QMouseEvent * event);
//		void mouseReleaseEvent(QMouseEvent * event);

	private:
		void CreateTextures(void);

	public:
		GLuint texture;
		int textureWidth, textureHeight;

		uint32_t * buffer;
		unsigned rasterWidth, rasterHeight;

		bool synchronize;
		unsigned filter;
		int offset;
		bool fullscreen;
		int outputWidth;
		int32_t hideMouseTimeout;
};

#endif	// __GLWIDGET_H__
