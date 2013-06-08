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

#define ZERO_TO_INFINITY(x) ((x) == 0 ? 9999 : (x))

using namespace openni;

namespace nui
{

class StreamListener;

struct NUIPointsInternal
{
	NUIPointsInternal(NUIPoints* pNUIPoints) :
		m_pDevice(NULL), m_pDepthStream(NULL), m_pListener(NULL), m_pStreamListener(NULL), m_pNUIPoints(pNUIPoints)
		{}

	void Raise()
	{
		if (m_pListener != NULL)
			m_pListener->readyForNextData(m_pNUIPoints);
	}
	bool m_oniOwner;
	Device* m_pDevice;
	VideoStream* m_pDepthStream;

	NUIPoints::Listener* m_pListener;

	StreamListener* m_pStreamListener;

	NUIPoints* m_pNUIPoints;
};

class StreamListener : public VideoStream::NewFrameListener
{
public:
	StreamListener(NUIPointsInternal* pNUIPoints) : m_pNUIPoints(pNUIPoints)
	{}
	virtual ~StreamListener()
	{}
	virtual void onNewFrame(VideoStream& stream)
	{
		m_pNUIPoints->Raise();
	}
private:
	NUIPointsInternal* m_pNUIPoints;
};

NUIPoints::NUIPoints(int width, int height) :
	m_pCalibrationMgr(NULL),
	s_width(width),
	s_height(height)
{
	m_pInternal = new NUIPointsInternal(this);

	m_pInternal->m_pDevice = new Device;
	m_pInternal->m_oniOwner = true;

	OpenNI::initialize();
	Status rc = m_pInternal->m_pDevice->open(openni::ANY_DEVICE);
	if (rc != STATUS_OK)
	{
		printf("Open device failed:\n%s\n", OpenNI::getExtendedError());
		return;
	}
	initialize();
}

NUIPoints::NUIPoints(openni::Device* pDevice) : m_pCalibrationMgr(NULL)
{
	m_pInternal = new NUIPointsInternal(this);

	m_pInternal->m_pDevice = pDevice;
	m_pInternal->m_oniOwner = false;

	OpenNI::initialize();

	if (pDevice != NULL)
	{
		initialize();
	}
}

void NUIPoints::initialize()
{
	m_pInternal->m_pStreamListener = NULL;
	m_pInternal->m_pListener = NULL;

	m_pInternal->m_pDepthStream = new VideoStream;
	Status rc = m_pInternal->m_pDepthStream->create(*m_pInternal->m_pDevice, SENSOR_DEPTH);
	if (rc != STATUS_OK)
	{
		printf("Created failed\n%s\n", OpenNI::getExtendedError());
		return;
	}

	m_pInternal->m_pStreamListener = new StreamListener(m_pInternal);

	rc = m_pInternal->m_pDepthStream->start();
	if (rc != STATUS_OK)
	{
		printf("Start failed:\n%s\n", OpenNI::getExtendedError());
	}

	m_pInternal->m_pDepthStream->addNewFrameListener(m_pInternal->m_pStreamListener);
	m_pCalibrationMgr = new CalibrationMgr(getStream(), s_width, s_height);
}

NUIPoints::~NUIPoints()
{
	if (m_pInternal->m_pDepthStream != NULL)
	{
		m_pInternal->m_pDepthStream->removeNewFrameListener(m_pInternal->m_pStreamListener);

		m_pInternal->m_pDepthStream->stop();
		m_pInternal->m_pDepthStream->destroy();

		delete m_pInternal->m_pDepthStream;
	}

	if (m_pInternal->m_pStreamListener != NULL)
	{
		delete m_pInternal->m_pStreamListener;
	}

	if (m_pInternal->m_oniOwner)
	{
		if (m_pInternal->m_pDevice != NULL)
		{
			m_pInternal->m_pDevice->close();

			delete m_pInternal->m_pDevice;
		}
	}
	
	if (m_pCalibrationMgr != NULL)
	{
		delete m_pCalibrationMgr;
	}

	OpenNI::shutdown();

	delete m_pInternal;
}

bool NUIPoints::isValid() const
{
	if (m_pInternal == NULL)
		return false;
	if (m_pInternal->m_pDevice == NULL)
		return false;
	if (m_pInternal->m_pDepthStream == NULL)
		return false;
	if (!m_pInternal->m_pDepthStream->isValid())
		return false;

	return true;
}

Status NUIPoints::setListener(Listener& listener)
{
	m_pInternal->m_pListener = &listener;
	return STATUS_OK;
}
void NUIPoints::resetListener()
{
	m_pInternal->m_pListener = NULL;
}

Status NUIPoints::getNextData(std::list<cv::Point3f>& nuiPoints, VideoFrameRef& rawFrame)
{
	Status rc = m_pInternal->m_pDepthStream->readFrame(&rawFrame);
	if (rc != STATUS_OK)
	{
		printf("readFrame failed\n%s\n", OpenNI::getExtendedError());
	}

	DepthPixel* pDepth = (DepthPixel*)rawFrame.getData();
	bool found = false;
	cv::Point3f nuiPoint;
	int width = rawFrame.getWidth();
	int height = rawFrame.getHeight();

	for (int x = 1; x < width / 6.0; ++x) {
		for (int y = 1; y < height - (height / 4.5); ++y)
		{
			if (pDepth[y * width + x] < FAR_LIMIT
				&& pDepth[y * width + x] != 0
				&& (ZERO_TO_INFINITY(pDepth[(y + 1) * width + (x + 1)]) - pDepth[y * width + x] > DEPTH_TRESHOLD
				|| ZERO_TO_INFINITY(pDepth[(y - 1) * width + (x + 1)]) - pDepth[y * width + x] > DEPTH_TRESHOLD
				|| ZERO_TO_INFINITY(pDepth[(y + 1) * width + (x - 1)]) - pDepth[y * width + x] > DEPTH_TRESHOLD
				|| ZERO_TO_INFINITY(pDepth[(y - 1) * width + (x - 1)]) - pDepth[y * width + x] > DEPTH_TRESHOLD))
			{
				nuiPoint.x = x;
				nuiPoint.y = y;
				nuiPoint.z = pDepth[y * width + x];
				nuiPoints.push_back(nuiPoint);
			}
		}
	}

	if (nuiPoints.empty())
	{
		return STATUS_ERROR;
	}

	return STATUS_OK;
}

openni::VideoStream* NUIPoints::getStream()
{
	return (m_pInternal ? m_pInternal->m_pDepthStream : NULL);
}

}

