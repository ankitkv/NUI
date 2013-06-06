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

namespace nui
{

CalibrationMgr::CalibrationMgr(openni::VideoStream *stream) : m_pVideoStream(stream), m_pCalibMatrix(NULL)
{
	uncalibrate();
}

CalibrationMgr::~CalibrationMgr()
{
	if (m_pCalibMatrix != NULL)
	{
		delete m_pCalibMatrix;
		m_pCalibMatrix = NULL;
	}
}

void CalibrationMgr::calibrate(const cv::Point3f& nuiPoint, int X, int Y)
{
	m_pCalibPoints[X][Y] = getWorldPoint(*m_pVideoStream, nuiPoint);
	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 2; ++j) {
			if (m_pCalibPoints[i][j].x == -1 || m_pCalibPoints[i][j].y == -1 || m_pCalibPoints[i][j].z == -1)
				return;
		}
	}

	if (!m_pCalibMatrix)
		m_pCalibMatrix = new cv::Mat();

	float avx[2] = {0.0, 0.0}, avz[2] = {0.0, 0.0};
	std::vector<cv::Point2f> src, dest(4);

	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 2; ++j) {
			avx[j] += m_pCalibPoints[i][j].x;
			avz[j] += m_pCalibPoints[i][j].z;
		}
	}
	for (int i = 0; i < 2; ++i) {
		avx[i] /= 2.0;
		avz[i] /= 2.0;
	}
	float diffx = avx[0] - avx[1];
	float diffz = avz[0] - avz[1];
	double angle = -atan(diffx / diffz) * (180.0 / M_PI);
	cv::Point2f center(0, avz[0] - ((avx[0] * diffz) / diffx));

	m_pRotateMatrix = getRotationMatrix2D(center, angle, 1.0);
	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 2; ++j)
			src.push_back(cv::Point2f(m_pCalibPoints[i][j].x, m_pCalibPoints[i][j].z));
	}
	cv::transform(src, dest, m_pRotateMatrix);

	src.clear();
	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 2; ++j)
			src.push_back(cv::Point2f(dest[i * 2 + j].y, m_pCalibPoints[i][j].y));
	}
	dest.clear();
	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 2; ++j)
			dest.push_back(cv::Point2f(j * SCREEN_WIDTH, i * SCREEN_HEIGHT));
	}

	*m_pCalibMatrix = cv::findHomography(src, dest, CV_RANSAC);
}

void CalibrationMgr::calibrate(cv::Mat& rotateMatrix, cv::Mat& calibMatrix)
{
	if (!m_pCalibMatrix)
		m_pCalibMatrix = new cv::Mat();

	m_pRotateMatrix = rotateMatrix;
	*m_pCalibMatrix = calibMatrix;
}

void CalibrationMgr::uncalibrate()
{
	if (m_pCalibMatrix) {
		delete m_pCalibMatrix;
		m_pCalibMatrix = NULL;
	}
	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 2; ++j)
			m_pCalibPoints[i][j].x = m_pCalibPoints[i][j].y = m_pCalibPoints[i][j].z = -1;
	}
}

cv::Point3f CalibrationMgr::getCalibratedPoint(const cv::Point3f &nuiPoint)
{
	std::vector<cv::Point2f> in, out(1);
	float z;
	cv::Point3f point = getWorldPoint(*m_pVideoStream, nuiPoint);
	in.push_back(cv::Point2f(point.x, point.z));
	cv::transform(in, out, m_pRotateMatrix);
	in.clear();
	z = out.begin()->x;
	in.push_back(cv::Point2f(out.begin()->y, point.y));
	out.clear();
	cv::perspectiveTransform(in, out, *m_pCalibMatrix);
	return cv::Point3f(out.begin()->x, out.begin()->y, z);
}

bool CalibrationMgr::isCalibrated()
{
	return (m_pCalibMatrix != NULL);
}

cv::Point3f CalibrationMgr::getWorldPoint(openni::VideoStream &stream, const cv::Point3f& nuiPoint)
{
	float x, y, z, ix = nuiPoint.x, iy = nuiPoint.y, iz = nuiPoint.z;
	openni::CoordinateConverter::convertDepthToWorld(stream, ix, iy, iz, &x, &y, &z);
	cv::Point3f point;
	point.x = x;
	point.y = y;
	point.z = z;
	return point;
}

}

