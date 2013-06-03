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

#include "libnui.h"
#include "utilities.h"
#include <OpenNI.h>

#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>

class NUIListener : public nui::NUIPoints::Listener
{
public:
	virtual ~NUIListener() {}
	void readyForNextData(nui::NUIPoints* pNUIPoints)
	{
		openni::VideoFrameRef frame;
		cv::Point3f nuiPoint;
		openni::Status rc = pNUIPoints->getNextData(nuiPoint, frame);

		if (rc == openni::STATUS_OK)
		{
			printf("%f, %f, %f\n", nuiPoint.x, nuiPoint.y, nuiPoint.z);
		}
		else
		{
			printf("Update failed\n");
		}
	}
};

class NUIApp {
	nui::NUIPoints *nuiPoints;
	NUIListener *myListener;
	bool mIsAnaglyph;

	void toggle()
	{
		mIsAnaglyph = !mIsAnaglyph;
		if (mIsAnaglyph)
			printf("Enabling...\n");
		else
			printf("Disabling...\n");
	   
	   	if (nuiPoints) {
			nuiPoints->resetListener();
			if (myListener) {
				delete myListener;
				myListener = NULL;
			}
			delete nuiPoints;
			nuiPoints = NULL;
		}

		if (mIsAnaglyph) {
			nuiPoints = new nui::NUIPoints();
			if (!nuiPoints->isValid()) {
				delete nuiPoints;
				nuiPoints = NULL;
			} else {
				myListener = new NUIListener;
				nuiPoints->setListener(*myListener);
			}
		}
	}

public:
	NUIApp() : nuiPoints(NULL), myListener(NULL), mIsAnaglyph(false)
	{}

	void run()
	{
		while (true)
		{
			if (wasKeyboardHit())
				toggle();

			Sleep(1000);
		}
	}
};


int main()
{
	NUIApp app;
	app.run();
	return 0;
}
