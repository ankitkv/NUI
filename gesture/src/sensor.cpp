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

	if (nuiPoints->getNextData(points, frame) == openni::STATUS_OK) {

		for (std::list<cv::Point3f>::iterator i = points.begin(); i != points.end(); ++i) {
			cv::Point3f point = nuiPoints->getCalibrationMgr()->getCalibratedPoint(*i);
			if (point.x < 0.0) point.x = 0.0;
			if (point.x >= screen->width()) point.x = screen->width() - 1.0;
			if (point.y < 0.0) point.y = 0.0;
			if (point.y >= screen->height()) point.y = screen->height() - 1.0;
			if (point.z < 0.0) point.z = 0.0;

			foreach (CompWindow *w, screen->windows()) {
				GESTURE_WINDOW(w);
				if (!gw->isNormalWindow())
					continue;

				if (point.x >= w->geometry().x() && point.x < w->geometry().x() + w->size().width()
				 && point.y >= w->geometry().y() && point.y < w->geometry().y() + w->size().height()) {

					if ((gw->window->id() == screen->activeWindow() && point.z < 25) || point.z < 10) {
						gw->xbuffer.push_back(point.x);
						gw->ybuffer.push_back(point.y);
					}
				}
			}

			foreach (CompWindow *w, screen->windows()) {
				GESTURE_WINDOW(w);
				if (gw->isNormalWindow() && gw->xbuffer.empty())
					gw->released = true;
			}
		}

		points.clear();
	}
}

