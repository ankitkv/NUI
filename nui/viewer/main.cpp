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

#include "viewer.h"

int main(int argc, char** argv)
{
	int rc = openni::STATUS_OK;
	bool debug = false;

	const char* deviceURI = openni::ANY_DEVICE;
	if (argc > 1 && !strcmp(argv[1], "debug"))
		debug = true;

	NUIViewer nuiViewer("NUIPoints Viewer", deviceURI, debug);

	rc = nuiViewer.init(argc, argv);
	if (rc != openni::STATUS_OK)
	{
		return 1;
	}
	nuiViewer.run();
}
