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

#ifndef _VIEWER_H_
#define _VIEWER_H_

#include "libnui.h"

#include <X11/Xlib.h>
#include <list>

#define MAX_DEPTH 10000

class NUIListener : public nui::NUIPoints::Listener
{
public:
	NUIListener() : m_ready(false) {}
	virtual ~NUIListener() {}
	void readyForNextData(nui::NUIPoints* pNUIPoints)
	{
		m_nuiPoints.clear();
		int rc = pNUIPoints->getNextData(m_nuiPoints, m_frame);
		m_ready = true;
	}

	const openni::VideoFrameRef& getFrame() {return m_frame;}
	const std::list<cv::Point3f>& getNUIPoints() {return m_nuiPoints;}
	bool isAvailable() const {return m_ready;}
	void setUnavailable() {m_ready = false;}
private:
	openni::VideoFrameRef m_frame;
	std::list<cv::Point3f> m_nuiPoints;
	bool m_ready;
};


class NUICalib
{
public:
	NUICalib(const char* strNUIName, const char* deviceUri, bool debug);
	virtual ~NUICalib();

	virtual int init(int argc, char **argv);
	virtual int run();	//Does not return

	void doMouseMove(const cv::Point3f& nuiPoint);

protected:
	virtual void display();
	virtual void displayPostDraw(){};	// Overload to draw over the screen image

	virtual void onKey(unsigned char key, int x, int y);

	virtual int initOpenGL(int argc, char **argv);
	void initOpenGLHooks();

	void finalize();

private:
	NUICalib(const NUICalib&);
	NUICalib& operator=(NUICalib&);

	static NUICalib* ms_self;
	static void glutIdle();
	static void glutDisplay();
	static void glutKeyboard(unsigned char key, int x, int y);

	bool					m_debug;
	float					m_pDepthHist[MAX_DEPTH];
	char					m_strNUIName[ONI_MAX_STR];
	openni::RGB888Pixel*	m_pTexMap;
	unsigned int			m_nTexMapX;
	unsigned int			m_nTexMapY;

	Display *m_xDisplay;
	Window m_xScreenRoot;
	int s_width;
	int s_height;

	nui::NUIPoints* m_pNUIPoints;
	NUIListener* m_pNUIPointsListener;
};


#endif // _VIEWER_H_
