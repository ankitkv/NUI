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

#include <X11/cursorfont.h>
#include <core/atoms.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

COMPIZ_PLUGIN_20090315 (gesture, GesturePluginVTable)

bool
GestureScreen::moveInitiate(CompOption::Vector &options)
{
    CompWindow *w;
    Window     xid;

    GESTURE_SCREEN (screen);

    xid = CompOption::getIntOptionNamed (options, "window");

    w = screen->findWindow (xid);
    if (w && (w->actions () & CompWindowActionMoveMask))
    {
	CompRect     workArea;
	int          x, y;

	CompScreen *s = screen;

	x = CompOption::getIntOptionNamed (options, "x", w->geometry ().x () +
					   (w->size ().width () / 2));
	y = CompOption::getIntOptionNamed (options, "y", w->geometry ().y () +
					   (w->size ().height () / 2));

	if (s->otherGrabExist ("gesture", NULL))
	    return false;

	if (ms->w)
	    return false;

	if (ms->region)
	{
	    XDestroyRegion (ms->region);
	    ms->region = NULL;
	}

	ms->status = RectangleOut;

	ms->savedX = w->serverGeometry ().x ();
	ms->savedY = w->serverGeometry ().y ();

	ms->x = 0;
	ms->y = 0;

	lastPointerX = x;
	lastPointerY = y;

	workArea = s->getWorkareaForOutput (w->outputDevice ());

	if (!ms->grab)
	    ms->grab = s->pushGrab (ms->moveCursor, "gesture");

	if (ms->grab)
	{
	    unsigned int grabMask = CompWindowGrabMoveMask |
				    CompWindowGrabButtonMask;

		grabMask |= CompWindowGrabExternalAppMask;

	    ms->w = w;

	    w->grabNotify (x, y, 0, grabMask);
	}
    }

    return false;
}

bool
GestureScreen::moveTerminate()
{
    GESTURE_SCREEN (screen);

    if (ms->w)
    {
	ms->w->syncPosition ();

	/* update window attributes as window constraints may have
	   changed - needed e.g. if a maximized window was moved
	   to another output device */
	ms->w->updateAttributes (CompStackingUpdateModeNone);

	ms->w->ungrabNotify ();

	if (ms->grab)
	{
	    screen->removeGrab (ms->grab, NULL);
	    ms->grab = NULL;
	}

	ms->w             = 0;
    }

    return false;
}

/* creates a region containing top and bottom struts. only struts that are
   outside the screen workarea are considered. */
static Region
moveGetYConstrainRegion (CompScreen *s)
{
    CompWindow   *w;
    Region       region;
    REGION       r;
    CompRect     workArea;
    BoxRec       extents;
    unsigned int i;

    region = XCreateRegion ();
    if (!region)
	return NULL;

    r.rects    = &r.extents;
    r.numRects = r.size = 1;

    r.extents.x1 = MINSHORT;
    r.extents.y1 = 0;
    r.extents.x2 = 0;
    r.extents.y2 = s->height ();

    XUnionRegion (&r, region, region);

    r.extents.x1 = s->width ();
    r.extents.x2 = MAXSHORT;

    XUnionRegion (&r, region, region);

    for (i = 0; i < s->outputDevs ().size (); i++)
    {
	XUnionRegion (s->outputDevs ()[i].region (), region, region);

	workArea = s->getWorkareaForOutput (i);
	extents = s->outputDevs ()[i].region ()->extents;

	foreach (w, s->windows ())
	{
	    if (!w->mapNum ())
		continue;

	    if (w->struts ())
	    {
		r.extents.x1 = w->struts ()->top.x;
		r.extents.y1 = w->struts ()->top.y;
		r.extents.x2 = r.extents.x1 + w->struts ()->top.width;
		r.extents.y2 = r.extents.y1 + w->struts ()->top.height;

		if (r.extents.x1 < extents.x1)
		    r.extents.x1 = extents.x1;
		if (r.extents.x2 > extents.x2)
		    r.extents.x2 = extents.x2;
		if (r.extents.y1 < extents.y1)
		    r.extents.y1 = extents.y1;
		if (r.extents.y2 > extents.y2)
		    r.extents.y2 = extents.y2;

		if (r.extents.x1 < r.extents.x2 && r.extents.y1 < r.extents.y2)
		{
		    if (r.extents.y2 <= workArea.y ())
			XSubtractRegion (region, &r, region);
		}

		r.extents.x1 = w->struts ()->bottom.x;
		r.extents.y1 = w->struts ()->bottom.y;
		r.extents.x2 = r.extents.x1 + w->struts ()->bottom.width;
		r.extents.y2 = r.extents.y1 + w->struts ()->bottom.height;

		if (r.extents.x1 < extents.x1)
		    r.extents.x1 = extents.x1;
		if (r.extents.x2 > extents.x2)
		    r.extents.x2 = extents.x2;
		if (r.extents.y1 < extents.y1)
		    r.extents.y1 = extents.y1;
		if (r.extents.y2 > extents.y2)
		    r.extents.y2 = extents.y2;

		if (r.extents.x1 < r.extents.x2 && r.extents.y1 < r.extents.y2)
		{
		    if (r.extents.y1 >= workArea.bottom ())
			XSubtractRegion (region, &r, region);
		}
	    }
	}
    }

    return region;
}

void
GestureScreen::moveHandleMotionEvent(int xRoot, int yRoot)
{
    GESTURE_SCREEN (screen);

    if (ms->grab)
    {
	int	     dx, dy;
	int	     wX, wY;
	int          wWidth, wHeight;
	CompWindow   *w;

	w = ms->w;

	wX      = w->geometry ().x ();
	wY      = w->geometry ().y ();
	wWidth  = w->geometry ().widthIncBorders ();
	wHeight = w->geometry ().heightIncBorders ();

	ms->x += xRoot - lastPointerX;
	ms->y += yRoot - lastPointerY;

    CompRect workArea;
    int	     min, max;

    dx = ms->x;
    dy = ms->y;

    workArea = screen->getWorkareaForOutput (w->outputDevice ());

	if (!ms->region)
	    ms->region = moveGetYConstrainRegion (screen);

	/* make sure that the top border extents or the top row of
	   pixels are within what is currently our valid screen
	   region */
	if (ms->region)
	{
	    int x, y, width, height;
	    int status;

	    x	   = wX + dx - w->border ().left;
	    y	   = wY + dy - w->border ().top;
	    width  = wWidth + w->border ().left + w->border ().right;
	    height = w->border ().top ? w->border ().top : 1;

	    status = XRectInRegion (ms->region, x, y,
				    (unsigned int) width,
				    (unsigned int) height);

	    /* only constrain movement if previous position was valid */
	    if (ms->status == RectangleIn)
	    {
			int xStatus = status;

			while (dx && xStatus != RectangleIn)
			{
			    xStatus = XRectInRegion (ms->region,
						     x, y - dy,
						     (unsigned int) width,
						     (unsigned int) height);

			    if (xStatus != RectangleIn)
				dx += (dx < 0) ? 1 : -1;

			    x = wX + dx - w->border ().left;
			}

			while (dy && status != RectangleIn)
			{
			    status = XRectInRegion (ms->region,
						    x, y,
						    (unsigned int) width,
						    (unsigned int) height);

			    if (status != RectangleIn)
				dy += (dy < 0) ? 1 : -1;

			    y = wY + dy - w->border ().top;
			}
		    }
		    else
		    {
			ms->status = status;
		    }
		}

	    if (w->state () & CompWindowStateMaximizedVertMask)
	    {
		min = workArea.y () + w->border ().top;
		max = workArea.bottom () - w->border ().bottom - wHeight;

		if (wY + dy < min)
		    dy = min - wY;
		else if (wY + dy > max)
		    dy = max - wY;
	    }

	    if (w->state () & CompWindowStateMaximizedHorzMask)
	    {
		if (wX > (int) screen->width () ||
		    wX + w->size ().width () < 0)
		    return;

		if (wX + wWidth < 0)
		    return;

		min = workArea.x () + w->border ().left;
		max = workArea.right () - w->border ().right - wWidth;

		if (wX + dx < min)
		    dx = min - wX;
		else if (wX + dx > max)
		    dx = max - wX;
    }

	if (dx || dy)
	{
	    w->move (wX + dx - w->geometry ().x (),
		     wY + dy - w->geometry ().y (), false);

	    ms->x -= dx;
	    ms->y -= dy;
	}
    }
}

bool
GestureWindow::glPaint (const GLWindowPaintAttrib &attrib,
		     const GLMatrix            &transform,
		     const CompRegion          &region,
		     unsigned int              mask)
{
    GLWindowPaintAttrib sAttrib = attrib;
    bool                status;

    GESTURE_SCREEN (screen);

    status = gWindow->glPaint (sAttrib, transform, region, mask);

    return status;
}

bool
GestureScreen::registerPaintHandler(compiz::composite::PaintHandler *pHnd)
{
    hasCompositing = true;
    cScreen->registerPaintHandler (pHnd);
    return true;
}

void
GestureScreen::unregisterPaintHandler()
{
    hasCompositing = false;
    cScreen->unregisterPaintHandler ();
}

GestureScreen::GestureScreen (CompScreen *screen) :
    PluginClassHandler<GestureScreen,CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    nuiPoints(NULL),
    foundWindow(0),
    w(0),
    region (NULL),
    status (RectangleOut),
    grab (NULL),
    hasCompositing (false)
{
    moveCursor = XCreateFontCursor (screen->dpy (), XC_fleur);
    if (cScreen)
    {
	CompositeScreenInterface::setHandler (cScreen);
	hasCompositing =
	    cScreen->compositingActive ();
    }

    ScreenInterface::setHandler (screen);

	optionSetScreenToggleKeyInitiate (boost::bind (&GestureScreen::toggleGesture, this));
}

GestureScreen::~GestureScreen ()
{
	destroyNUIPoints();

    if (region)
	XDestroyRegion (region);

    if (moveCursor)
	XFreeCursor (screen->dpy (), moveCursor);
}

bool
GesturePluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
		return false;

    return true;
}
