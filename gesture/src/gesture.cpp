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

#include "gesture.h"

COMPIZ_PLUGIN_20090315 (gesture, GesturePluginVTable)

bool GestureWindow::glPaint (const GLWindowPaintAttrib &attrib,
		     const GLMatrix            &transform,
		     const CompRegion          &region,
		     unsigned int              mask)
{
    GLWindowPaintAttrib sAttrib = attrib;
    bool                status;

	gWindow->glPaintSetEnabled (this, false);

	if (!xbuffer.empty()) {
		float avgx = 0, avgy = 0;
		for (std::list<float>::iterator i = xbuffer.begin(); i != xbuffer.end(); ++i)
			avgx += *i;
		for (std::list<float>::iterator i = ybuffer.begin(); i != ybuffer.end(); ++i)
			avgy += *i;
		avgx /= xbuffer.size();
		avgy /= ybuffer.size();
		
		if (savedX != -1)
			window->move(avgx - savedX, avgy - savedY, false);

		savedX = avgx;
		savedY = avgy;
		xbuffer.clear();
		ybuffer.clear();

	} else if (released) {
		window->syncPosition();
		savedX = savedY = -1;
		released = false;
	}

    status = gWindow->glPaint (sAttrib, transform, region, mask);
    gWindow->glPaintSetEnabled (this, true);

    return status;
}

bool GestureScreen::registerPaintHandler(compiz::composite::PaintHandler *pHnd)
{
    cScreen->registerPaintHandler (pHnd);
    return true;
}

void GestureScreen::unregisterPaintHandler()
{
    cScreen->unregisterPaintHandler ();
}

GestureWindow::GestureWindow(CompWindow *window) :
	PluginClassHandler<GestureWindow,CompWindow> (window),
	savedX(-1), savedY(-1),
	released(false),
	window (window),
	gWindow (GLWindow::get (window)),
	cWindow (CompositeWindow::get (window))
{
    if (gWindow)
		GLWindowInterface::setHandler(gWindow);
	if (cWindow)
	    CompositeWindowInterface::setHandler(cWindow);
}

GestureScreen::GestureScreen (CompScreen *screen) :
    PluginClassHandler<GestureScreen, CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    nuiPoints(NULL)
{
    if (cScreen)
		CompositeScreenInterface::setHandler (cScreen);

	optionSetScreenToggleKeyInitiate(boost::bind(&GestureScreen::toggleGesture, this));
    ScreenInterface::setHandler (screen);
}

GestureScreen::~GestureScreen ()
{
	destroyNUIPoints();
}

bool GesturePluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
		return false;

    return true;
}
