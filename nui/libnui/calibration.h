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

#ifndef _CALIBRATION_H_
#define _CALIBRATION_H_

#include <OpenNI.h>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace nui
{

class ONI_API_EXPORT CalibrationMgr
{
	openni::VideoStream *m_pVideoStream;
	cv::Point3f m_pCalibPoints[2][2];
	cv::Mat *m_pCalibMatrix;
	cv::Mat m_pRotateMatrix;

public:
	CalibrationMgr(openni::VideoStream *);
	~CalibrationMgr();

	void calibrate(const cv::Point3f& nuiPoint, int X, int Y);
	void calibrate(cv::Mat& rotateMatrix, cv::Mat& calibMatrix);
	void uncalibrate();
	cv::Point3f getCalibratedPoint(const cv::Point3f&);
	bool isCalibrated();
	static cv::Point3f getWorldPoint(openni::VideoStream &stream, const cv::Point3f&);
	
	cv::Mat& getRotateMatrix() { return m_pRotateMatrix; }
	cv::Mat* getCalibMatrix()  { return m_pCalibMatrix;  }
};

}

#endif // _CALIBRATION_H_

