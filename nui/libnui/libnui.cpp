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

NUIPoints::NUIPoints(const char* uri) : m_pCalibrationMgr(NULL)
{
	m_pInternal = new NUIPointsInternal(this);

	m_pInternal->m_pDevice = new Device;
	m_pInternal->m_oniOwner = true;

	OpenNI::initialize();
	Status rc = m_pInternal->m_pDevice->open(uri);
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
	m_pCalibrationMgr = new CalibrationMgr(getStream());
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

Status NUIPoints::getNextData(cv::Point3f& nuiPoint, VideoFrameRef& rawFrame)
{
	Status rc = m_pInternal->m_pDepthStream->readFrame(&rawFrame);
	if (rc != STATUS_OK)
	{
		printf("readFrame failed\n%s\n", OpenNI::getExtendedError());
	}

	DepthPixel* pDepth = (DepthPixel*)rawFrame.getData();
	bool found = false;
	nuiPoint.z = 0xffff;
	int width = rawFrame.getWidth();
	int height = rawFrame.getHeight();

	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height - (height / 4.5); ++y)
		{
			if (pDepth[y * width + x] < FAR_LIMIT && pDepth[y * width + x] != 0)
			{
				nuiPoint.x = x;
				nuiPoint.y = y;
				nuiPoint.z = pDepth[y * width + x];

				found = true;
				if (x < SCREEN_AREA) {
					for (int i = x; i < x + SCREEN_AREA; ++i) {
						if (pDepth[y * width + i] == 0 || pDepth[y * width + i] >= FAR_LIMIT) {
							found = false;
							break;
						}
					}
				}
				if (found)
					break;
			}
		}
		if (found)
			break;
	}

	if (!found)
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

