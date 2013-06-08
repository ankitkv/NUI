/*****************************************************************************
*                                                                            *
*  3D Natural User Interface for Operating Systems                           *
*                                                                            *
*  Anaglyph plugin                                                           *
*  Copyright (c) 2007 Patryk Kowalczyk <wodor@wodor.org>                     *
*  Ported to Compiz 0.9.x                                                    *
*  Copyright (c) 2010 Sam Spilsbury <smspillaz@gmail.com>                    *
*  Copyright (c) 2010 Scott Moreau                                           *
*  Copyright (c) 2013 Ankit Vani,                                            *
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
 
#include <core/core.h>
#include <composite/composite.h>
#include <opengl/opengl.h>

#include "anaglyph_options.h"

class AnaglyphScreen :
    public PluginClassHandler <AnaglyphScreen, CompScreen>,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public AnaglyphOptions
{
    public:
	
	AnaglyphScreen (CompScreen *);
	~AnaglyphScreen ();
    
    public:
    
	CompositeScreen *cScreen;
	GLScreen	*gScreen;
	
	bool		mIsAnaglyph;
	bool		mIsDamage;

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
