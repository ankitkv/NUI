/*****************************************************************************
*                                                                            *
*  3D Natural User Interface for Operating Systems                           *
*                                                                            *
*  3D Gesture plugin                                                         *
*  Copyright (C) 2013 Ankit Vani,                                            *
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

#include <core/core.h>
#include <composite/composite.h>
#include <opengl/opengl.h>

#include "gesture_options.h"

class GestureScreen :
    public PluginClassHandler <GestureScreen, CompScreen>,
    public CompositeScreenInterface,
    public GestureOptions,
    public GLScreenInterface,
    public nui::NUIPoints::Listener
{
    public:
	GestureScreen (CompScreen *screen);
	~GestureScreen ();

	CompositeScreen *cScreen;
	GLScreen        *gScreen;

	nui::NUIPoints *nuiPoints;
	void createNUIPoints();
	void destroyNUIPoints();

	void readyForNextData(nui::NUIPoints *);
	std::list<cv::Point3f> points;

	bool registerPaintHandler (compiz::composite::PaintHandler *pHnd);
	void unregisterPaintHandler ();

	bool toggleGesture();
	bool glPaintOutput (const GLScreenPaintAttrib &,
		       const GLMatrix		 &,
		       const CompRegion		 &,
		       CompOutput		 *,
		       unsigned int);
};

class GestureWindow :
    public GLWindowInterface,
    public CompositeWindowInterface,
    public PluginClassHandler<GestureWindow, CompWindow>
{
    public:
	GestureWindow (CompWindow *window);

	bool glPaint (const GLWindowPaintAttrib &, const GLMatrix &,
		      const CompRegion &, unsigned int);

	float        savedX;
	float        savedY;
	std::list<float> xbuffer[3];
	std::list<float> ybuffer[3];
	bool clicked;

	bool isNormalWindow();
	bool released;

	XEvent event;
	Window pw;

	CompWindow      *window;
	GLWindow        *gWindow;
	CompositeWindow *cWindow;
};

#define GESTURE_SCREEN(s) \
    GestureScreen *gs = GestureScreen::get (s)

#define GESTURE_WINDOW(w) \
    GestureWindow *gw = GestureWindow::get (w)


class GesturePluginVTable :
    public CompPlugin::VTableForScreenAndWindow<GestureScreen, GestureWindow>
{
    public:

	bool init ();
};
