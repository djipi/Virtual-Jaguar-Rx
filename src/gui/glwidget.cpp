// OpenGL implementation in Qt
// Parts of this are blantantly ripped off from BSNES (thanks Byuu!)
//
// by James Hammons
// (C) 2010 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  01/14/2010  Created this file
// JLH  02/03/2013  Added "centered" fullscreen mode with correct aspect ratio
// JPM  06/06/2016  Visual Studio support
//

#include "glwidget.h"

#include "jaguar.h"
#include "settings.h"
#include "tom.h"

#if defined(__GCCWIN32__) || defined(_MSC_VER)
#if defined(_MSC_VER)
#include <GL/gl.h>
#endif
// Apparently on win32, various OpenGL constants aren't pulled in.
#include <GL/glext.h>
#endif


GLWidget::GLWidget(QWidget * parent/*= 0*/): QGLWidget(parent), texture(0),
	textureWidth(0), textureHeight(0), buffer(0), rasterWidth(326), rasterHeight(240),
	offset(0), hideMouseTimeout(60)
{
	// Screen pitch has to be the texture width (in 32-bit pixels)...
	JaguarSetScreenPitch(1024);
	setMouseTracking(true);
}


GLWidget::~GLWidget()
{
	if (buffer)
		delete[] buffer;
}


void GLWidget::initializeGL()
{
	format().setDoubleBuffer(true);
	resizeGL(rasterWidth, rasterHeight);

	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_POLYGON_SMOOTH);
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_DITHER);
	glEnable(GL_TEXTURE_2D);
	glClearColor(0.0, 0.0, 0.0, 0.0);

	CreateTextures();
}


void GLWidget::paintGL()
{
	// If we're in fullscreen mode, we take the value of the screen width as
	// set by MainWin, since it may be wider than what our aspect ratio allows.
	// In that case, we adjust the viewport over so that it's centered on the
	// screen. Otherwise, we simply take the width from our width() funtion
	// which will always be correct in windowed mode.

	if (!fullscreen)
		outputWidth = width();

	// Bit 0 in VP is interlace flag. 0 = interlace, 1 = non-interlaced
	double multiplier = (TOMGetVP() & 0x0001 ? 1.0 : 2.0);
	unsigned outputHeight = height();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, outputWidth, 0, outputHeight, -1.0, 1.0);
	glViewport(0 + offset, 0, outputWidth, outputHeight);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (vjs.glFilter ? GL_LINEAR : GL_NEAREST));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (vjs.glFilter ? GL_LINEAR : GL_NEAREST));
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, TOMGetVideoModeWidth(), rasterHeight * multiplier, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buffer);

	double w = (double)TOMGetVideoModeWidth()  / (double)textureWidth;
	double h = ((double)rasterHeight * multiplier) / (double)textureHeight;
	unsigned u = outputWidth;
	unsigned v = outputHeight;

	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0, 0); glVertex3i(0, v, 0);
	glTexCoord2f(w, 0); glVertex3i(u, v, 0);
	glTexCoord2f(0, h); glVertex3i(0, 0, 0);
	glTexCoord2f(w, h); glVertex3i(u, 0, 0);
	glEnd();
}


void GLWidget::resizeGL(int /*width*/, int /*height*/)
{
//kludge [No, this is where it belongs!]
	rasterHeight = (vjs.hardwareTypeNTSC ? VIRTUAL_SCREEN_HEIGHT_NTSC : VIRTUAL_SCREEN_HEIGHT_PAL);

	return;
}


// At some point, we'll have to create more than one texture to handle
// cases like Doom. Or have another go at TV type rendering; it will
// require a 2048x512 texture though. (Note that 512 is the correct height for
// interlaced screens; we won't have to change much here to support it.)
void GLWidget::CreateTextures(void)
{
	// Seems that power of 2 sizes are still mandatory...
	textureWidth  = 1024;
	textureHeight = 512;
	buffer = new uint32_t[textureWidth * textureHeight];
	JaguarSetScreenBuffer(buffer);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, textureWidth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, NULL);
}


void GLWidget::HandleMouseHiding(void)
{
	// Mouse watchdog timer handling. Basically, if the timeout value is
	// greater than zero, decrement it. Otherwise, check for zero, if so, then
	// hide the mouse and set the hideMouseTimeout value to -1 to signal that
	// the mouse has been hidden.
	if (hideMouseTimeout > 0)
		hideMouseTimeout--;
	else if (hideMouseTimeout == 0)
	{
		hideMouseTimeout--;
		setCursor(Qt::BlankCursor);
	}
}


// We use this as part of a watchdog system for hiding/unhiding the mouse. This
// part shows the mouse (if hidden) and resets the watchdog timer.
void GLWidget::CheckAndRestoreMouseCursor(void)
{
	// Has the mouse been hidden? (-1 means mouse was hidden)
	if (hideMouseTimeout == -1)
		setCursor(Qt::ArrowCursor);

	hideMouseTimeout = 60;
}


// We check here for mouse movement; if there is any, show the mouse and reset
// the watchdog timer.
void GLWidget::mouseMoveEvent(QMouseEvent * /*event*/)
{
	CheckAndRestoreMouseCursor();
}


#if 0
class RubyGLWidget: public QGLWidget
{
  public:
    GLuint texture;
    unsigned textureWidth, textureHeight;

    uint32_t * buffer;
    unsigned rasterWidth, rasterHeight;

    bool synchronize;
    unsigned filter;

    void updateSynchronization() {
      #ifdef __APPLE__
      makeCurrent();
      CGLContextObj context = CGLGetCurrentContext();
      GLint value = synchronize;  //0 = draw immediately (no vsync), 1 = draw once per frame (vsync)
      CGLSetParameter(context, kCGLCPSwapInterval, &value);
      #endif
    }
} * widget;
#endif
