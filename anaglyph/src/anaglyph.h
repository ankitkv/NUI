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

#include <core/core.h>
#include <composite/composite.h>
#include <opengl/opengl.h>
#include <deque>

#include "anaglyph_options.h"

#define SAMPLES 6

class AnaglyphScreen :
    public PluginClassHandler <AnaglyphScreen, CompScreen>,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public AnaglyphOptions
{
	void createNUIPoints(AnaglyphScreen *);
	void destroyNUIPoints();

    public:
	
	class NUIListener : public nui_points::NUIPoints::Listener
	{
		std::deque<float> xpoints, ypoints;
		Display *dpy;
		Window screen_root;
		AnaglyphScreen *screen;

		//void mousemove(int x, int y);

	public:
		NUIListener(AnaglyphScreen *);
		virtual ~NUIListener() {}
		void readyForNextData(nui_points::NUIPoints *);
	} *myListener;

	AnaglyphScreen (CompScreen *);
	~AnaglyphScreen ();

	CompositeScreen *cScreen;
	GLScreen	*gScreen;
	
	bool		mIsAnaglyph;
	bool		mIsDamage;
	nui_points::NUIPoints *nuiPoints;

	void toggle ();

	bool anaglyphWindow (CompOption::Vector options);
	bool anaglyphScreen ();

	void
	optionChanged (CompOption *, Options);
	
	bool
	glPaintOutput (const GLScreenPaintAttrib &,
		       const GLMatrix		 &,
		       const CompRegion		 &,
		       CompOutput		 *,
		       unsigned int);
};

#define ANAGLYPH_SCREEN(s) \
    AnaglyphScreen *as = AnaglyphScreen::get (s);


class AnaglyphWindow :
    public PluginClassHandler <AnaglyphWindow, CompWindow>,
    public CompositeWindowInterface,
    public GLWindowInterface
{
    public:
    
	AnaglyphWindow (CompWindow *w);
	~AnaglyphWindow ();
	
    public:

	CompWindow *window;
	GLWindow   *gWindow;
	CompositeWindow *cWindow;
	
	bool mIsAnaglyph;

	void toggle ();
	
	bool
	glPaint (const GLWindowPaintAttrib &,
		 const GLMatrix		   &,
		 const CompRegion	   &,
		 unsigned int		    );
	
	bool
	damageRect (bool, const CompRect &);
	
};

#define ANAGLYPH_WINDOW(w) \
    AnaglyphWindow *aw = AnaglyphWindow::get (w);

class AnaglyphPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <AnaglyphScreen, AnaglyphWindow>
{
    public:

	bool init ();
};