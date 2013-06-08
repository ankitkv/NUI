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

bool GestureScreen::isNormalWindow(CompWindow *w)
{
	if (w->type () & (CompWindowTypeDesktopMask |
	    CompWindowTypeDockMask                  |
	    CompWindowTypeFullscreenMask))
		return false;

	if (w->overrideRedirect ())
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

		GESTURE_SCREEN(screen);

		for (std::list<cv::Point3f>::iterator i = points.begin(); i != points.end(); ++i) {
			cv::Point3f point = nuiPoints->getCalibrationMgr()->getCalibratedPoint(*i);
			if (point.x < 0.0) point.x = 0.0;
			if (point.x >= screen->width()) point.x = screen->width() - 1.0;
			if (point.y < 0.0) point.y = 0.0;
			if (point.y >= screen->height()) point.y = screen->height() - 1.0;
			if (point.z < 0.0) point.z = 0.0;

			if (ms->grab) {
				foundWindow = 0;
				if (point.x >= ms->w->geometry().x() && point.x < ms->w->geometry().x() + ms->w->size().width()
				 && point.y >= ms->w->geometry().y() && point.y < ms->w->geometry().y() + ms->w->size().height()) {

					if (point.z < 25) {
						xbuffer.push_back(point.x);
						ybuffer.push_back(point.y);
					}
				}
			} else {
				foreach (CompWindow *w, screen->windows()) {
					if (!isNormalWindow(w))
						continue;

					if ((!foundWindow || (foundWindow == w->id())) && point.x >= w->geometry().x() && point.x < w->geometry().x() + w->size().width()
					 && point.y >= w->geometry().y() && point.y < w->geometry().y() + w->size().height()) {

						if (point.z < 25) {
							foundWindow = w->id();
							xbuffer.push_back(point.x);
							ybuffer.push_back(point.y);
						}
					}
				}
			}
		}

		if (!xbuffer.empty()) {
			float avgx = 0, avgy = 0;
			for (std::list<float>::iterator i = ms->xbuffer.begin(); i != ms->xbuffer.end(); ++i)
				avgx += *i;
			for (std::list<float>::iterator i = ms->ybuffer.begin(); i != ms->ybuffer.end(); ++i)
				avgy += *i;
			avgx /= ms->xbuffer.size();
			avgy /= ms->ybuffer.size();

			if (!ms->grab) {
				CompOption::Vector o;

				o.push_back (CompOption ("window", CompOption::TypeInt));
				o[0].value ().set ((int) foundWindow);

				o.push_back (CompOption ("x", CompOption::TypeInt));
				o[1].value ().set ((int) avgx);

				o.push_back (CompOption ("y", CompOption::TypeInt));
				o[2].value ().set ((int) avgy);

				moveInitiate(o);
			}

			moveHandleMotionEvent((int) avgx, (int) avgy);
			ms->xbuffer.clear();
			ms->ybuffer.clear();
		} else if (ms->grab) {
			moveTerminate();
		}

		points.clear();
	}
}

