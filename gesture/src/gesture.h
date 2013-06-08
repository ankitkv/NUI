/*****************************************************************************
*                                                                            *
*  3D Natural User Interface for Operating Systems                           *
*                                                                            *
*  3D Gesture plugin, based on Move Window plugin                            *
*  Copyright (c) 2005 Novell, Inc.                                           *
*  Copyright (c) 2013 Ankit Vani,                                            *
*                     Humayun Mulla,                                         *
*                     Ronit Kulkarni,                                        *
*                     Siddharth Kulkarni                                     *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/

#include "libnui.h"

#include <core/screen.h>
#include <core/pluginclasshandler.h>
#include <composite/composite.h>
#include <opengl/opengl.h>

#include "gesture_options.h"

#define NUM_KEYS (sizeof (mKeys) / sizeof (mKeys[0]))

extern const unsigned short KEY_MOVE_INC;

extern const unsigned short SNAP_BACK;
extern const unsigned short SNAP_OFF;

struct _MoveKeys {
    const char *name;
    int        dx;
    int        dy;
} static mKeys[] = {
    { "Left",  -1,  0 },
    { "Right",  1,  0 },
    { "Up",     0, -1 },
    { "Down",   0,  1 }
};

class GestureScreen :
    public ScreenInterface,
    public CompositeScreenInterface,
    public PluginClassHandler<GestureScreen,CompScreen>,
    public GestureOptions,
    public nui::NUIPoints::Listener
{
    public:
	GestureScreen (CompScreen *screen);
	~GestureScreen ();

	CompositeScreen *cScreen;

	nui::NUIPoints *nuiPoints;
	void createNUIPoints();
	void destroyNUIPoints();

	void readyForNextData(nui::NUIPoints *);
	std::list<cv::Point3f> points;

	bool registerPaintHandler (compiz::composite::PaintHandler *pHnd);
	void unregisterPaintHandler ();

	Window     foundWindow;
	CompWindow *w;
	int        savedX;
	int        savedY;
	int        x;
	int        y;
	std::list<float> xbuffer;
	std::list<float> ybuffer;

	Region     region;
	int        status;
	KeyCode    key[NUM_KEYS];

	CompScreen::GrabHandle grab;

	Cursor moveCursor;

	int	snapOffY;
	int	snapBackY;

	bool hasCompositing;

	bool toggleGesture();
	bool moveInitiate(CompOption::Vector &options);
	bool moveTerminate();
	void moveHandleMotionEvent(int xRoot, int yRoot);
};

class GestureWindow :
    public GLWindowInterface,
    public PluginClassHandler<GestureWindow,CompWindow>
{
    public:
	GestureWindow (CompWindow *window) :
	    PluginClassHandler<GestureWindow,CompWindow> (window),
	    window (window),
	    gWindow (GLWindow::get (window)),
	    cWindow (CompositeWindow::get (window))
	{
	    if (gWindow)
		GLWindowInterface::setHandler (gWindow, false);
	};

	bool glPaint (const GLWindowPaintAttrib &, const GLMatrix &,
		      const CompRegion &, unsigned int);

	CompWindow      *window;
	GLWindow        *gWindow;
	CompositeWindow *cWindow;
};

#define GESTURE_SCREEN(s) \
    GestureScreen *ms = GestureScreen::get (s)

#define GESTURE_WINDOW(w) \
    GestureWindow *mw = GestureWindow::get (w)


class GesturePluginVTable :
    public CompPlugin::VTableForScreenAndWindow<GestureScreen, GestureWindow>
{
    public:

	bool init ();
};
