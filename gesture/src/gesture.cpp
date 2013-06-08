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
	clicked(false),
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
    gScreen (GLScreen::get (screen)),
    nuiPoints(NULL)
{
    if (cScreen)
		CompositeScreenInterface::setHandler (cScreen);
	if (gScreen)
		GLScreenInterface::setHandler (gScreen);

	optionSetScreenToggleKeyInitiate(boost::bind(&GestureScreen::toggleGesture, this));
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
