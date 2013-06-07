/*****************************************************************************
*                                                                            *
*  3D Natural User Interface for Operating Systems                           *
*                                                                            *
*  Anaglyph plugin                                                           *
*  Copyright (c) 2007 Patryk Kowalczyk <wodor@wodor.org>                     *
*  Ported to Compiz 0.9.x                                                    *
*  Copyright (c) 2010 Sam Spilsbury <smspillaz@gmail.com>                    *
*  Copyright (c) 2010 Scott Moreau                                           *
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

#include "anaglyph.h"

#include <list>
#include <pwd.h>

COMPIZ_PLUGIN_20090315(anaglyph, AnaglyphPluginVTable);

void
AnaglyphWindow::toggle()
{
    ANAGLYPH_SCREEN (screen);
    mIsAnaglyph = !mIsAnaglyph;

    if (as->optionGetExcludeMatch ().evaluate (window) ||
			    window->overrideRedirect ()) {
		mIsAnaglyph = false;
    }
}

void
AnaglyphScreen::toggle()
{
    mIsAnaglyph = !mIsAnaglyph;

   	destroyNUIPoints();
    if (mIsAnaglyph)
		createNUIPoints(this);

    foreach (CompWindow *w, screen->windows ()) {
		AnaglyphWindow::get (w)->toggle ();
    }
}

bool
AnaglyphScreen::anaglyphWindow (CompOption::Vector options)
{
    CompWindow *w = screen->findWindow (CompOption::getIntOptionNamed (options,
								       "window",
								       0));
    if (w)
	AnaglyphWindow::get (w)->toggle ();

    return true;
}

bool
AnaglyphScreen::anaglyphScreen ()
{
    toggle ();

    return true;
}

//------------------------------------------ MASTER FUNCTION

bool
AnaglyphWindow::glPaint (const GLWindowPaintAttrib &attrib,
			const GLMatrix		  &transform,
			const CompRegion	  &region,
			unsigned int		  mask)
{
    bool status;
    ANAGLYPH_SCREEN(screen);

    if (mIsAnaglyph && gWindow->textures ().size ())
    {
	//if (as->isAnaglyph)
	//	aw->isAnaglyph = FALSE;

	GLuint oldFilter = as->gScreen->textureFilter ();

	if (as->optionGetMipmaps ())
	    as->gScreen->setTextureFilter (GL_LINEAR_MIPMAP_LINEAR);

	mask |= PAINT_WINDOW_TRANSFORMED_MASK;

	GLMatrix sTransform (transform);
	GLWindowPaintAttrib wAttrib (attrib);

	float offset = 7.5f;
	float desktopOffset = 4.5f;

	if (as->optionGetDesaturate ())
	    wAttrib.saturation = 0.0f;

	gWindow->glPaintSetEnabled (this, false);

	//BLUE and ...
	glColorMask(GL_FALSE,GL_TRUE,GL_TRUE,GL_FALSE);
	if (window->type () & CompWindowTypeDesktopMask) //desktop
	    sTransform.translate (offset * desktopOffset, 0.0f, 0.0f);
	else if (window->state () & CompWindowStateShadedMask)
	    sTransform.translate (0.0f, 0.0f, 0.0f);
	else if ((window->state () & CompWindowStateMaximizedHorzMask) ||
		 (window->state () & CompWindowStateMaximizedVertMask ))
	    sTransform.translate (-offset * 3.5, 0.0f, 0.0f);
	else if (window->type () & CompWindowTypeDockMask) // dock
	    sTransform.translate (0.0f, 0.0f, 0.0f);
	else if (window->state () & CompWindowStateStickyMask) // sticky
	    sTransform.translate (offset * desktopOffset, 0.0f, 0.0f);
	else if (window->state () & CompWindowStateBelowMask) //below
	    sTransform.translate (offset, 0.0f, 0.0f);
	else if (window->state () & CompWindowStateAboveMask) // above
	    sTransform.translate (-offset * 4.0, 0.0f, 0.0f);
	else if (window->id () == screen->activeWindow ()) // active window
	    sTransform.translate (-offset * 3.0, 0.0f, 0.0f);
	else //other windows
	    sTransform.translate (-offset, 0.0f, 0.0f);

	status = gWindow->glPaint (wAttrib, sTransform, region, mask);

	//RED
	glColorMask(GL_TRUE,GL_FALSE,GL_FALSE,GL_FALSE);
	if (window->type () & CompWindowTypeDesktopMask) //desktop
	    sTransform.translate (-offset * 2.0 * desktopOffset, 0.0f, 0.0f);
	else if (window->state () & CompWindowStateShadedMask)
	    sTransform.translate (0.0f, 0.0f, 0.0f);
	else if ((window->state () & CompWindowStateMaximizedHorzMask) ||
		 (window->state () & CompWindowStateMaximizedVertMask ))
	    sTransform.translate (offset *3.5, 0.0f, 0.0f);
	else if (window->type () & CompWindowTypeDockMask)// dock
	    sTransform.translate (0.0f, 0.0f, 0.0f);
	else if (window->state () & CompWindowStateStickyMask) // sticky
	    sTransform.translate (-offset * 2.0 * desktopOffset, 0.0f, 0.0f);
	else if (window->state () & CompWindowStateBelowMask) //below
	    sTransform.translate (-offset * 2.0, 0.0f, 0.0f);
	else if (window->state () & CompWindowStateAboveMask) //above
	    sTransform.translate (offset * 4.0, 0.0f, 0.0f);
	else if (window->id () == screen->activeWindow ()) // active window
	    sTransform.translate (offset * 3.0, 0.0f, 0.0f);
	else //other windows
	    sTransform.translate (offset * 1.5, 0.0f, 0.0f);

	status = gWindow->glPaint (wAttrib, sTransform, region, mask);

	gWindow->glPaintSetEnabled (this, true);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	as->gScreen->setTextureFilter (oldFilter);
    }
    else
    {
	status = gWindow->glPaint (attrib, transform, region, mask);
    }
    return status;
}

//---------------------------------------------------------------DAMAGE FUNCTION

bool
AnaglyphWindow::damageRect (bool initial,
			    const CompRect &rect)
{
    bool status = false;

    ANAGLYPH_SCREEN(screen);

    if (mIsAnaglyph || as->mIsAnaglyph || as->mIsDamage)
    {
	as->mIsDamage = TRUE;
	if (!mIsAnaglyph && !mIsAnaglyph)
	    as->mIsDamage = FALSE;

	as->cScreen->damageScreen ();
	status = TRUE;
    }

    status |= cWindow->damageRect (initial, rect);

    return status;
}

//----------------------------------------------------------------- PAINT OUTPUT

bool AnaglyphScreen::isNormalWindow(CompWindow *w)
{
	if (w->type () & CompWindowTypeDesktopMask
	 || w->state () & CompWindowStateShadedMask
	 || w->state () & CompWindowStateMaximizedHorzMask
	 || w->state () & CompWindowStateMaximizedVertMask
	 || w->type () & CompWindowTypeDockMask
	 || w->state () & CompWindowStateStickyMask
	 || w->state () & CompWindowStateBelowMask
	 || w->state () & CompWindowStateAboveMask)
		return false;
	else
		return true;
}

bool
AnaglyphScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
			      const GLMatrix		&transform,
			      const CompRegion		&region,
			      CompOutput		*output,
			      unsigned int		mask)
{
    mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS_MASK;
    ANAGLYPH_SCREEN(screen);

	if (mIsAnaglyph) {
		if (!as->myListener->points.empty())
		{
			foreach (CompWindow *w, screen->windows()) {
				if (!isNormalWindow(w))
					continue;
				ANAGLYPH_WINDOW(w);
				aw->isTouched = false;
			}

			for (std::list<cv::Point3f>::iterator i = as->myListener->points.begin(); i != as->myListener->points.end(); ++i) {
				cv::Point3f point = as->nuiPoints->getCalibrationMgr()->getCalibratedPoint(*i);
				if (point.x < 0.0) point.x = 0.0;
				if (point.x >= SCREEN_WIDTH) point.x = SCREEN_WIDTH - 1.0;
				if (point.y < 0.0) point.y = 0.0;
				if (point.y >= SCREEN_HEIGHT) point.y = SCREEN_HEIGHT - 1.0;
				if (point.z < 0.0) point.z = 0.0;

				foreach (CompWindow *w, screen->windows()) {
				if (!isNormalWindow(w))
					continue;

					if (point.x >= w->geometry().x() && point.x < w->geometry().x() + w->size().width()
					 && point.y >= w->geometry().y() && point.y < w->geometry().y() + w->size().height()) {
						ANAGLYPH_WINDOW(w);

						if ((aw->window->id() == screen->activeWindow() && point.z < 25) || point.z < 10) {
							aw->isTouched = true;
							aw->xbuffer.push_back(point.x);
							aw->ybuffer.push_back(point.y);
						}
					}
				}
			}

			foreach (CompWindow *w, screen->windows()) {
				if (!isNormalWindow(w))
					continue;

				ANAGLYPH_WINDOW(w);
				if (aw->isTouched) {
					float avgx = 0, avgy = 0, dx, dy;
					for (std::list<float>::iterator i = aw->xbuffer.begin(); i != aw->xbuffer.end(); ++i)
						avgx += *i;
					for (std::list<float>::iterator i = aw->ybuffer.begin(); i != aw->ybuffer.end(); ++i)
						avgy += *i;
					avgx /= aw->xbuffer.size();
					avgy /= aw->ybuffer.size();

					dx = aw->oldx == -1 ? 0 : avgx - aw->oldx;
					dy = aw->oldy == -1 ? 0 : avgy - aw->oldy;
					aw->window->move(dx, dy, true);
					aw->window->syncPosition();

					aw->oldx = avgx;
					aw->oldy = avgy;
					aw->xbuffer.clear();
					aw->ybuffer.clear();
				} else {
					aw->oldx = aw->oldy = -1;
				}
			}

			as->myListener->points.clear();
		}
	}

    return gScreen->glPaintOutput (attrib, transform, region, output, mask);
}

//------------------------------------------------------------------------------

void
AnaglyphScreen::optionChanged (CompOption *option,
			       Options    num)
{
    switch (num)
    {
	case AnaglyphOptions::AnaglyphMatch:
	case AnaglyphOptions::ExcludeMatch:
	{
	    foreach (CompWindow *w, screen->windows ())
	    {
		bool isAnaglyph;

		ANAGLYPH_WINDOW (w);

		isAnaglyph = optionGetAnaglyphMatch ().evaluate (w);
		isAnaglyph = isAnaglyph &&
					!optionGetExcludeMatch ().evaluate (w);

		if (isAnaglyph && mIsAnaglyph && !aw->mIsAnaglyph)
		    aw->toggle ();
	    }
	}
	break;
	default:
	    break;
    }
}


//----------------------------------------------------------------------- SCREEN

AnaglyphScreen::AnaglyphScreen (CompScreen *s) :
    PluginClassHandler <AnaglyphScreen, CompScreen> (s),
    myListener(NULL),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    mIsAnaglyph (false),
    mIsDamage (false),
    nuiPoints(NULL)
{
    CompositeScreenInterface::setHandler (cScreen);
    GLScreenInterface::setHandler (gScreen);

    optionSetWindowToggleKeyInitiate (boost::bind
				    (&AnaglyphScreen::anaglyphWindow, this,
				     _3));
    optionSetScreenToggleKeyInitiate (boost::bind
				    (&AnaglyphScreen::anaglyphScreen, this));
    optionSetWindowToggleKeyInitiate (boost::bind
				     (&AnaglyphScreen::anaglyphWindow, this,
				      _3));
    optionSetScreenToggleKeyInitiate (boost::bind
				      (&AnaglyphScreen::anaglyphScreen, this));

    optionSetAnaglyphMatchNotify (boost::bind (&AnaglyphScreen::optionChanged,
					       this, _1, _2));
    optionSetExcludeMatchNotify (boost::bind (&AnaglyphScreen::optionChanged,
					      this, _1, _2));
}

AnaglyphScreen::~AnaglyphScreen ()
{
	destroyNUIPoints();
}

//----------------------------------------------------------------------- WINDOW

AnaglyphWindow::AnaglyphWindow (CompWindow *w) :
    PluginClassHandler <AnaglyphWindow, CompWindow> (w),
    window (w),
    gWindow (GLWindow::get (w)),
    cWindow (CompositeWindow::get (w)),
    mIsAnaglyph (false),
    oldx(-1), oldy(-1)
{
    ANAGLYPH_SCREEN (screen);

    CompositeWindowInterface::setHandler (cWindow);
    GLWindowInterface::setHandler (gWindow);

    if (as->mIsAnaglyph && as->optionGetAnaglyphMatch ().evaluate (w))
	toggle ();
}

AnaglyphWindow::~AnaglyphWindow ()
{
}

//----------------------------------------------------------------------- OPENNI

void AnaglyphScreen::createNUIPoints(AnaglyphScreen *screen)
{
	if (!nuiPoints) {
		nuiPoints = new nui::NUIPoints();
		if (!nuiPoints->isValid()) {
			delete nuiPoints;
			nuiPoints = NULL;
		} else {
			myListener = new NUIListener(screen);
			nuiPoints->setListener(*myListener);

			passwd *pw = getpwuid(getuid());
			cv::FileStorage ifs(std::string(pw->pw_dir) + "/.nuicalib.xml", cv::FileStorage::READ);
			if (ifs.isOpened()) {
				cv::Mat rotateMatrix, calibMatrix;
				ifs["rotation"] >> rotateMatrix;
				ifs["calibration"] >> calibMatrix;
				ifs.release();
				nuiPoints->getCalibrationMgr()->calibrate(rotateMatrix, calibMatrix);
			}
		}
	}
}

void AnaglyphScreen::destroyNUIPoints()
{
   	if (nuiPoints) {
		nuiPoints->resetListener();
		if (myListener) {
			delete myListener;
			myListener = NULL;
		}
		delete nuiPoints;
		nuiPoints = NULL;
	}
}

AnaglyphScreen::NUIListener::NUIListener(AnaglyphScreen *s) : screen(s)
{ }

void AnaglyphScreen::NUIListener::readyForNextData(nui::NUIPoints* pNUIPoints)
{
	if (!pNUIPoints->getCalibrationMgr()->isCalibrated())
		return;

	openni::VideoFrameRef frame;
	pNUIPoints->getNextData(points, frame);
}

//----------------------------------------------------------------------- PLUGIN

bool
AnaglyphPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
	!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
	!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	return false;

    return true;
}
