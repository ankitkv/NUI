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

#include <pwd.h>

void GestureScreen::createNUIPoints()
{
	if (!nuiPoints) {
		nuiPoints = new nui::NUIPoints(screen->width(), screen->height());
		if (!nuiPoints->isValid()) {
			delete nuiPoints;
			nuiPoints = NULL;
		} else {
			nuiPoints->setListener(*this);

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

void GestureScreen::destroyNUIPoints()
{
   	if (nuiPoints) {
		nuiPoints->resetListener();
		delete nuiPoints;
		nuiPoints = NULL;
	}
}

bool GestureWindow::isNormalWindow()
{
	if (window->type() & (CompWindowTypeDesktopMask
	    | CompWindowTypeDockMask
	    | CompWindowTypeFullscreenMask))
		return false;

	if (window->state() & (CompWindowStateShadedMask
	    | CompWindowStateMaximizedHorzMask
	    | CompWindowStateMaximizedVertMask
	    | CompWindowStateStickyMask
	    | CompWindowStateBelowMask
	    | CompWindowStateAboveMask))
		return false;

	if (window->overrideRedirect())
		return false;

	return true;
}

bool GestureScreen::toggleGesture()
{
	if (nuiPoints)
		destroyNUIPoints();
	else
		createNUIPoints();
	return true;
}

void GestureScreen::readyForNextData(nui::NUIPoints *nuiPoints)
{
	if (!nuiPoints->getCalibrationMgr()->isCalibrated())
		return;

	openni::VideoFrameRef frame;
	nuiPoints->getNextData(points, frame);
}

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
		avgx /= xbuffer.size();
		xbuffer.clear();

		for (std::list<float>::iterator i = ybuffer.begin(); i != ybuffer.end(); ++i)
			avgy += *i;
		avgy /= ybuffer.size();
		ybuffer.clear();

		if (savedX != -1)
			window->move(avgx - savedX, avgy - savedY, false);

		savedX = avgx;
		savedY = avgy;

	} else if (released) {
		if (savedX != -1) {
			window->syncPosition();
			savedX = savedY = -1;
		}
		released = false;
	}

    status = gWindow->glPaint (sAttrib, transform, region, mask);
    gWindow->glPaintSetEnabled (this, true);

    return status;
}

bool GestureScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
			      const GLMatrix		&transform,
			      const CompRegion		&region,
			      CompOutput		*output,
			      unsigned int		mask)
{
	if (!points.empty()) {

		foreach (CompWindow *w, screen->windows()) {
			GESTURE_WINDOW(w);
			if (!gw->isNormalWindow())
				continue;

			gw->released = true;

			for (std::list<cv::Point3f>::iterator i = points.begin(); i != points.end(); ++i) {
				cv::Point3f point = nuiPoints->getCalibrationMgr()->getCalibratedPoint(*i);
				if (point.x < 0.0) point.x = 0.0;
				if (point.x >= screen->width()) point.x = screen->width() - 1.0;
				if (point.y < 0.0) point.y = 0.0;
				if (point.y >= screen->height()) point.y = screen->height() - 1.0;
				if (point.z < 0.0) point.z = 0.0;

				if (point.x >= w->geometry().x() && point.x < w->geometry().x() + w->size().width()
				 && point.y >= w->geometry().y() && point.y < w->geometry().y() + w->size().height()) {

					if ((gw->window->id() == screen->activeWindow() && point.z < 25) || point.z < 10) {
						gw->released = false;
						gw->xbuffer.push_back(point.x);
						gw->ybuffer.push_back(point.y);
					}
				}
			}
		}

		points.clear();
	}

    return gScreen->glPaintOutput (attrib, transform, region, output, mask);
}
