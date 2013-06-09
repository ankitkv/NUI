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
	bool moving = false;

	if (!xbuffer[0].empty() || !xbuffer[1].empty()) {
		if (!xbuffer[0].empty() && !xbuffer[1].empty()) {
			float avgx[2] = {0, 0}, avgy[2] = {0, 0};
			for (int index = 0; index < 2; ++index) {
				for (std::list<float>::iterator i = xbuffer[index].begin(); i != xbuffer[index].end(); ++i)
					avgx[index] += *i;
				avgx[index] /= xbuffer[index].size();

				for (std::list<float>::iterator i = ybuffer[index].begin(); i != ybuffer[index].end(); ++i)
					avgy[index] += *i;
				avgy[index] /= ybuffer[index].size();
			}

			float ax = (avgx[0] + avgx[1]) / 2.0;
			float ay = (avgy[0] + avgy[1]) / 2.0;

			if (savedX != -1)
				window->move(ax - savedX, ay - savedY, false);

			savedX = ax;
			savedY = ay;
			moving = true;
		}
		
		for (int i = 0; i < 2; ++i) {
			if (!xbuffer[i].empty()) {
				xbuffer[i].clear();
				ybuffer[i].clear();
			}
		}
	}

	if (!moving && !clicked && !xbuffer[2].empty()) {
		float avgx = 0, avgy = 0;
		for (std::list<float>::iterator i = xbuffer[2].begin(); i != xbuffer[2].end(); ++i)
			avgx += *i;
		avgx /= xbuffer[2].size();
		xbuffer[2].clear();

		for (std::list<float>::iterator i = ybuffer[2].begin(); i != ybuffer[2].end(); ++i)
			avgy += *i;
		avgy /= ybuffer[2].size();
		ybuffer[2].clear();

		memset(&event, 0x00, sizeof(event));
	
		event.type = ButtonPress;
		event.xbutton.button = 1;
		event.xbutton.same_screen = True;
		event.xbutton.root = RootWindow(screen->dpy(), DefaultScreen(screen->dpy()));
		event.xbutton.window = window->id();
	
		CompWindow *r = screen->findWindow(event.xbutton.root);
		event.xbutton.x_root = r->geometry().x();
		event.xbutton.y_root = r->geometry().x();
		event.xbutton.x = avgx - event.xbutton.x_root;
		event.xbutton.y = avgy - event.xbutton.y_root;
		event.xbutton.state = Button1Mask;
		event.xbutton.subwindow = event.xbutton.window;
		
		XSendEvent(screen->dpy(), window->id(), true, 0xfff, &event);
		XFlush(screen->dpy());

		clicked = true;

	} else if (clicked) {
		event.type = ButtonRelease;
		CompWindow *r = screen->findWindow(event.xbutton.root);
		event.xbutton.x_root = r->geometry().x();
		event.xbutton.y_root = r->geometry().x();
		event.xbutton.x = savedX - event.xbutton.x_root;
		event.xbutton.y = savedY - event.xbutton.y_root;
		event.xbutton.state = 0x100;
	
		XSendEvent(screen->dpy(), window->id(), true, 0xfff, &event);	
		XFlush(screen->dpy());

		clicked = false;
	}

	if (released) {
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
	bool erased;

	if (!points.empty()) {

		foreach (CompWindow *w, screen->windows()) {
			GESTURE_WINDOW(w);
			if (!gw->isNormalWindow())
				continue;

			gw->released = true;

			for (std::list<cv::Point3f>::iterator i = points.begin(); i != points.end(); ) {
				cv::Point3f point = nuiPoints->getCalibrationMgr()->getCalibratedPoint(*i);
				if (point.x < 0.0) point.x = 0.0;
				if (point.x >= screen->width()) point.x = screen->width() - 1.0;
				if (point.y < 0.0) point.y = 0.0;
				if (point.y >= screen->height()) point.y = screen->height() - 1.0;
				if (point.z < 0.0) point.z = 0.0;
				erased = false;

				if (point.x >= w->geometry().x() && point.x < w->geometry().x() + w->size().width()) {
					if (point.x >= w->geometry().x() && point.x < w->geometry().x() + w->size().width()
						&& point.y >= w->geometry().y() && point.y < w->geometry().y() + w->size().height()
						&& ((gw->window->id() == screen->activeWindow() && point.z < 25) || point.z < 10)) {
							gw->xbuffer[2].push_back(point.x);
							gw->ybuffer[2].push_back(point.y);
					}

					if (point.y > w->geometry().y() - (w->size().height() / 3) && point.y < w->geometry().y() + (w->size().height() / 3)) {
						// upper edge
						if ((gw->window->id() == screen->activeWindow() && point.z < 25) || point.z < 10) {
							gw->released = false;
							gw->xbuffer[0].push_back(point.x);
							gw->ybuffer[0].push_back(point.y);
							i = points.erase(i);
							erased = true;
						}
					} else if (point.y > w->geometry().y() + w->size().height() - (w->size().height() / 3)
					           && point.y < w->geometry().y() + w->size().height() + (w->size().height() / 3)) {
						// lower edge
						if ((gw->window->id() == screen->activeWindow() && point.z < 25) || point.z < 10) {
							gw->released = false;
							gw->xbuffer[1].push_back(point.x);
							gw->ybuffer[1].push_back(point.y);
							i = points.erase(i);
							erased = true;
						}
					}
				}

				if (!erased)
					++i;
			}
		}

		points.clear();
	}

    return gScreen->glPaintOutput (attrib, transform, region, output, mask);
}
