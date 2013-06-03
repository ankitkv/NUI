/*****************************************************************************
*                                                                            *
*  3D Natural User Interface for Operating Systems                           *
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

#ifndef _LIBNUI_H_
#define _LIBNUI_H_

#include "calibration.h"

#include <list>

#define FAR_LIMIT 1000
#define SCREEN_AREA 35
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

namespace nui
{

struct NUIPointsInternal;

class ONI_API_EXPORT NUIPoints
{
public:
	class Listener
	{
	public:
		virtual void readyForNextData(NUIPoints*) = 0;
	};

	NUIPoints(const char* uri = NULL);
	NUIPoints(openni::Device* pDevice);
	~NUIPoints();

	bool isValid() const;

	openni::Status setListener(Listener& listener);
	void resetListener();

	openni::Status getNextData(std::list<cv::Point3f>& nuiPoints, openni::VideoFrameRef& rawFrame);
	openni::VideoStream* getStream();

	CalibrationMgr *getCalibrationMgr() { return m_pCalibrationMgr; }

private:
	void initialize();

	NUIPointsInternal* m_pInternal;
	CalibrationMgr *m_pCalibrationMgr;
};

}

#endif // _LIBNUI_H_

