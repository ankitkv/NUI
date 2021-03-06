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

#include "calib.h"
#include "utilities.h"

#include <GL/glut.h>
#include <vector>
#include <cstdio>
#include <pwd.h>

#define GL_WIN_SIZE_X	1280
#define GL_WIN_SIZE_Y	1024
#define TEXTURE_SIZE	512

#define DEFAULT_DISPLAY_MODE	DISPLAY_MODE_DEPTH

#define MIN_NUM_CHUNKS(data_size, chunk_size)	((((data_size)-1) / (chunk_size) + 1))
#define MIN_CHUNKS_SIZE(data_size, chunk_size)	(MIN_NUM_CHUNKS(data_size, chunk_size) * (chunk_size))

#define SAMPLES 6

NUICalib* NUICalib::ms_self = NULL;

void NUICalib::glutIdle()
{
	glutPostRedisplay();
}
void NUICalib::glutDisplay()
{
	NUICalib::ms_self->display();
}
void NUICalib::glutKeyboard(unsigned char key, int x, int y)
{
	NUICalib::ms_self->onKey(key, x, y);
}

NUICalib::NUICalib(const char* strNUIName, const char* deviceUri, bool debug) :
	m_debug(debug), m_pNUIPoints(NULL), m_pNUIPointsListener(NULL)

{
	ms_self = this;
	strncpy(m_strNUIName, strNUIName, ONI_MAX_STR);

	m_xDisplay = XOpenDisplay(NULL);
	Screen *s = DefaultScreenOfDisplay(m_xDisplay);
	s_width = WidthOfScreen(s);
	s_height = HeightOfScreen(s);
	m_pNUIPoints = new nui::NUIPoints(s_width, s_height);
}
NUICalib::~NUICalib()
{
	finalize();

	delete[] m_pTexMap;

	ms_self = NULL;
}

void NUICalib::finalize()
{
	if (m_pNUIPoints != NULL)
	{
		m_pNUIPoints->resetListener();
		delete m_pNUIPoints;
		m_pNUIPoints = NULL;
	}
	if (m_pNUIPointsListener != NULL)
	{
		delete m_pNUIPointsListener;
		m_pNUIPointsListener = NULL;
	}
}

int NUICalib::init(int argc, char **argv)
{
	m_pTexMap = NULL;

	if (!m_pNUIPoints->isValid())
	{
		return openni::STATUS_ERROR;
	}

	m_pNUIPointsListener = new NUIListener;
	m_pNUIPoints->setListener(*m_pNUIPointsListener);

	m_xScreenRoot = RootWindow(m_xDisplay, 0);

	passwd *pw = getpwuid(getuid());
	cv::FileStorage ifs(std::string(pw->pw_dir) + "/.nuicalib.xml", cv::FileStorage::READ);
	if (ifs.isOpened()) {
		cv::Mat rotateMatrix, calibMatrix;
		ifs["rotation"] >> rotateMatrix;
		ifs["calibration"] >> calibMatrix;
		ifs.release();
		m_pNUIPoints->getCalibrationMgr()->calibrate(rotateMatrix, calibMatrix);
	}

	return initOpenGL(argc, argv);

}
int NUICalib::run()	//Does not return
{
	glutMainLoop();

	return openni::STATUS_OK;
}
void NUICalib::display()
{
	if (!m_pNUIPointsListener->isAvailable())
	{
		return;
	}

	openni::VideoFrameRef depthFrame = m_pNUIPointsListener->getFrame();
	const std::list<cv::Point3f>& nuiPoints = m_pNUIPointsListener->getNUIPoints();
	m_pNUIPointsListener->setUnavailable();

	if (m_pTexMap == NULL)
	{
		// Texture map init
		m_nTexMapX = MIN_CHUNKS_SIZE(depthFrame.getWidth(), TEXTURE_SIZE);
		m_nTexMapY = MIN_CHUNKS_SIZE(depthFrame.getHeight(), TEXTURE_SIZE);
		m_pTexMap = new openni::RGB888Pixel[m_nTexMapX * m_nTexMapY];
	}



	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, GL_WIN_SIZE_X, GL_WIN_SIZE_Y, 0, -1.0, 1.0);

	if (depthFrame.isValid())
	{
		calculateHistogram(m_pDepthHist, MAX_DEPTH, depthFrame);
	}

	memset(m_pTexMap, 0, m_nTexMapX*m_nTexMapY*sizeof(openni::RGB888Pixel));

	// check if we need to draw depth frame to texture
	if (depthFrame.isValid())
	{
		const openni::DepthPixel* pDepthRow = (const openni::DepthPixel*)depthFrame.getData();
		openni::RGB888Pixel* pTexRow = m_pTexMap + depthFrame.getCropOriginY() * m_nTexMapX;
		int rowSize = depthFrame.getStrideInBytes() / sizeof(openni::DepthPixel);
		int width = depthFrame.getWidth();
		int height = depthFrame.getHeight();

		for (int y = 0; y < height; ++y)
		{
			const openni::DepthPixel* pDepth = pDepthRow;
			openni::RGB888Pixel* pTex = pTexRow + depthFrame.getCropOriginX();

			for (int x = 0; x < width; ++x, ++pDepth, ++pTex)
			{
				if (*pDepth != 0)
					pTex->r = pTex->g = pTex->b = m_pDepthHist[*pDepth];
			}

			pDepthRow += rowSize;
			pTexRow += m_nTexMapX;
		}
	}

	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_nTexMapX, m_nTexMapY, 0, GL_RGB, GL_UNSIGNED_BYTE, m_pTexMap);

	// Display the OpenGL texture map
	glColor4f(1,1,1,1);

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);

	int nXRes = depthFrame.getWidth();
	int nYRes = depthFrame.getHeight();

	// upper left
	glTexCoord2f(0, 0);
	glVertex2f(0, 0);
	// upper right
	glTexCoord2f((float)nXRes/(float)m_nTexMapX, 0);
	glVertex2f(GL_WIN_SIZE_X, 0);
	// bottom right
	glTexCoord2f((float)nXRes/(float)m_nTexMapX, (float)nYRes/(float)m_nTexMapY);
	glVertex2f(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
	// bottom left
	glTexCoord2f(0, (float)nYRes/(float)m_nTexMapY);
	glVertex2f(0, GL_WIN_SIZE_Y);

	glEnd();
	glDisable(GL_TEXTURE_2D);

	if (!nuiPoints.empty()) {
		float nuiPointCoordinates[m_debug ? nuiPoints.size() * 3 : 3];
		int index = 0;

		if (m_debug) {
			for (std::list<cv::Point3f>::const_iterator i = nuiPoints.begin(); i != nuiPoints.end(); ++i) {
				nuiPointCoordinates[index++] = i->x * GL_WIN_SIZE_X / (float) depthFrame.getWidth();
				nuiPointCoordinates[index++] = i->y * GL_WIN_SIZE_Y / (float) depthFrame.getHeight();
				nuiPointCoordinates[index++] = 0;
			}
		} else {
			nuiPointCoordinates[index++] = nuiPoints.begin()->x * GL_WIN_SIZE_X / (float) depthFrame.getWidth();
			nuiPointCoordinates[index++] = nuiPoints.begin()->y * GL_WIN_SIZE_Y / (float) depthFrame.getHeight();
			nuiPointCoordinates[index++] = 0;
		}

		glVertexPointer(3, GL_FLOAT, 0, nuiPointCoordinates);
		glColor3f(1.f, 0.f, 0.f);
		glPointSize(4.5);
		glDrawArrays(GL_POINTS, 0, index);
		glFlush();
	}

	// Swap the OpenGL display buffers
	glutSwapBuffers();

	if (!nuiPoints.empty())
		doMouseMove(*(nuiPoints.begin()));
}

void NUICalib::doMouseMove(const cv::Point3f& nuiPoint)
{
	if (!m_pNUIPoints->getCalibrationMgr()->isCalibrated())
		return;

	cv::Point3f point = m_pNUIPoints->getCalibrationMgr()->getCalibratedPoint(nuiPoint);
	if (point.x < 0.0) point.x = 0.0;
	if (point.x >= s_width) point.x = s_width - 1.0;
	if (point.y < 0.0) point.y = 0.0;
	if (point.y >= s_height) point.y = s_height - 1.0;
	if (point.z < 0.0) point.z = 0.0;
	printf("point: (%f, %f)\tAbove screen: %f\n", point.x, point.y, point.z);

	XWarpPointer(m_xDisplay, None, m_xScreenRoot, 0, 0, 0, 0, point.x, point.y);
	XFlush(m_xDisplay);
}

void NUICalib::onKey(unsigned char key, int /*x*/, int /*y*/)
{
	switch (key)
	{
	case 'q':
		m_pNUIPoints->getCalibrationMgr()->calibrate(*(m_pNUIPointsListener->getNUIPoints().begin()), 0, 0);
		break;
	case 'w':
		m_pNUIPoints->getCalibrationMgr()->calibrate(*(m_pNUIPointsListener->getNUIPoints().begin()), 0, 1);
		break;
	case 'a':
		m_pNUIPoints->getCalibrationMgr()->calibrate(*(m_pNUIPointsListener->getNUIPoints().begin()), 1, 0);
		break;
	case 's':
		m_pNUIPoints->getCalibrationMgr()->calibrate(*(m_pNUIPointsListener->getNUIPoints().begin()), 1, 1);
		break;
	case 'd': {
		if (m_pNUIPoints->getCalibrationMgr()->isCalibrated())
			m_pNUIPoints->getCalibrationMgr()->uncalibrate();

		passwd *pw = getpwuid(getuid());
		remove((std::string(pw->pw_dir) + "/.nuicalib.xml").c_str());
		}
		break;
	case 27:
		finalize();
		exit (1);
	}

	if (m_pNUIPoints->getCalibrationMgr()->isCalibrated()) {
		passwd *pw = getpwuid(getuid());
		cv::FileStorage ofs(std::string(pw->pw_dir) + "/.nuicalib.xml", cv::FileStorage::WRITE);
		if (!ofs.isOpened()) {
			printf("could not write calibration file ~/.nuicalib.xml\n");
			exit(1);
		}

		ofs << "rotation" << m_pNUIPoints->getCalibrationMgr()->getRotateMatrix();
		ofs << "calibration" << *(m_pNUIPoints->getCalibrationMgr()->getCalibMatrix());
		ofs.release();
	}
}

int NUICalib::initOpenGL(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
	glutCreateWindow (m_strNUIName);
	// 	glutFullScreen();
	glutSetCursor(GLUT_CURSOR_NONE);

	initOpenGLHooks();

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	return openni::STATUS_OK;

}
void NUICalib::initOpenGLHooks()
{
	glutKeyboardFunc(glutKeyboard);
	glutDisplayFunc(glutDisplay);
	glutIdleFunc(glutIdle);
}
