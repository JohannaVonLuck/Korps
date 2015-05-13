/*******************************************************************************

    Licensing Information:
    
Korps License Agreement
Revision IIa
January 1st, 2005
Korps Project Group

Preample:

   This license is based on the notion of taking the best of both worlds of both the open-source and proprietary licensing models. There are some things which an open-source model is good for, and some things which a proprietary model is good for. This license is based on the concept of open-source code but with proprietary authoring. Only those who gain permission first may make derivative works, but the code remains open to everybody to look at and learn from. This makes sure that the authors do not get ripped off from their long hours of work, but also entitles everybody to look at how the code was programmed by the authors, and to allow modification for personal use.

TERMS AND CONDITIONS

1. Definitions

   The source code for a work means the preferred form of the work for making modifications to it. For an executable work, complete source code means all the source code for all modules it contains, plus any associated interface definition files, plus the scripts used to control compilation and installation of the executable. However, as a special exception, the source code distributed need not include anything that is normally distributed (in either source or binary form) with the major components (compiler, kernel, and so on) of the operating system on which the executable runs, unless that component itself accompanies the executable.

   Korps is the PC software product that is being licensed through this license agreement.

   The Korps Project Group is the organization of programmers, modelers, and staff members (excluding any outside contributors) that make up the online group known as the Korps Project Group. They are the primary authors and copyright holders of Korps.

2. General

   This license applies to all software code which encompases the "Korps" PC software product, including any derived works.

   This program is presented as free software; you may redistribute it in its original packaged form (in .zip, self-extracting .exe, or .tar.gz format), and/or modify it solely for your own personal use. Any modifications or derived works of this software MAY NOT be redistributed without first obtaining written permission from Korps Project Group.

   This program, it's source code, related data files, and any derived works MAY NOT at any time be charged for in any way shape or form, including, but not limited to, licensing, distributing, and modifying.

3. Derived Works

   Derived works of this software must adhere to the following stipulations:
a) Derived works for personal use only is fully permitted in accordance with the terms and conditions set forth in this license.
b) Persons must obtain written permission from Korps Project Group in order to make derived works that are to be distributed publically.
c) Authors of derived works may not grant other persons permission to derive works in any fashion.
d) Each person who works on a derived work is represented by the person who was given permission, and must also adhere to the same stipulations as set forth.
e) Derived works must maintain this copyright notice verbatim.
f) Derived works must maintain the original authors' and contributors names.
g) Derived works must have all derived source code freely available and included with any derived packages, at no charge to the general public, except in the case where the derived work is for personal use only.
h) Authors of derived works must cause the modified files to carry prominent notices stating changes made and date of any such change, except in the case where the derived work is for personal use only.
i) No authors of derived works may apply for software patents that relate to any portion of the original or derived work.

4. No Warranty

   BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED    OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,    REPAIR OR CORRECTION. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES, INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

5. End User License Agreement

   BY USING THIS SOFTWARE, MODIFYING IT, OR OTHERWISE, YOU AGREE TO ABIDE BY THESE TERMS AND CONDITIONS SET FORTH, AND AGREE TO ALL OTHER LICENSING TERMS AND CONDITIONS SET FORTH BY WORKS USED HEREIN, INCLUDING 3D MODELS, ARTWORK, AND OTHER COPYRIGHTED MATERIAL WHICH MAY NOT BE THE SOLE WORKS OF KORPS PROJECT GROUP.

END TERMS AND CONDITIONS

	Project:		"Korps", (C) 2004, 2005 Korps Project Group
 
    Description:    Korps is a single player WWII combat simulator, similiar to
                    CM, and is a full 3D combat environment where historical
                    accuracy is the key idea behind everything. It is aimed at
                    the early war years of Poland 1939 and France 1940.
                    
    Authors:
                Programming:
                    Johanna Wolf                <johanna.a.wolf@gmail.com>
                    Jerod Houghtelling          <jerodh@swnebr.net>
                    Joel Pavek                  <jpavekshs@yahoo.com>
                    Lance Simunek               <stfuwtf@gmail.com>
                
                3D Modelling:
                    Stefan "LA" Siverud         <legalassassin@gmail.com>
                    Beau "Texan" Hall           <beau.hall@gmail.com>
                    Greg "Geo" Thompson         <geo@geo-sphere.net>
                
                Graphic Artists:
                    Nils Eikelenboom            <eikelenboom@home.nl>
                
                Scale Modelling:
                    Gray Appleton               <applehobbys@aol.com>
                
                Technical Consultantcy:
                    David Lehmann               <david.lehmann@infonie.fr>
                    Michael Rausch              <m.rausch@t-online.de>
                    Wojciech Piotrowicz         <wojtekp@gnu.univ.gda.pl>
                    Ryan Gill                   <rmgill@mindspring.com>
                
                Beta Testing:
                    Tony Schaefer               <tonyscha@yahoo.com>
                    
                Outside Contributors:
                    - Base artwork from the "Combat Mission" series, and is
                      used with permission. http://www.battlefront.com
                    * From CMMODS.com:
                        - Limey
                        - Bruce "Pud" Richardson
                        - Thomas Klimisch
                        - johnnymo3
                        - Gurra
                        - Marco Bergman
                        - Gordon Molek
                        - Ed "Tanks-a-lot"
                    
    Notes:          This project relys heavily on the OpenGL graphics API, and
                    as such must be ran on a capable system using 3D-accelerated
                    video hardware that supports OpenGL (imbeded onto video
                    hardware). The newest drivers should also be used.
                    
                    This project relys heavily on the OpenAL sound API, and
                    as such must be ran on a capable system which supports
                    the usage of OpenAL. The newest drivers should also be used.
                    
                    This project relys heavily on the SDL API, of which controls
                    the underlaying portion of cross-platform capabilities of
                    the project. This package can be found at:
                        http://www.libsdl.org/index.php
    
                    This project relys on an external library called LIB3DS
                    which loads 3D Studio Max objects into memory and into our
                    game engine. This package can be found at:
                        http://lib3ds.sourceforge.net/
*******************************************************************************/
/*  Dependencies - Loaded from Common Include  */
#include "main.h"

/*******************************************************************************
                             Header Includes
*******************************************************************************/
#include "texture.h"            // Texture Library Module
#include "model.h"              // 3D Model Library Module
#include "database.h"           // Database Module
#include "effects.h"            // Special Effects Module
#include "sounds.h"             // Sound Module
#include "objhandler.h"         // Object Handler Module
#include "fonts.h"              // SDL TTF Font Wrapper
#include "console.h"            // Command Console/Comm Module
#include "misc.h"               // Misc. Functions
#include "load.h"               // Game Loading Screen Module
#include "camera.h"             // Camera Control Module
#include "scenery.h"            // Scenery Module
#include "collision.h"          // Collision Detection & Response Module
#include "ui.h"                 // User Interface Module
#include "script.h"             // Scripting Module
#include "gameloop.h"           // Game Execution Loop Base

/*******************************************************************************
                     Global Object Module Declarations
*******************************************************************************/
camera_module camera;           // Camera Control Module
ui_module ui;                   // User Interface Module
console_module console;         // Command Console/Comm Module
font_module fonts;              // SDL TTF Font Module
loader_module loader;           // Game Loading Screen Module
scenery_module map;             // Scenery Module
texture_library textures;       // Texture Library Module
model_library models;           // 3D Model Library Module
database_module db;             // Database Module
object_handler objects;         // Object Handler Module
collision_module cdr;           // Collision Detection & Response Module
se_module effects;              // Special Effects Module
sound_module sounds;            // Sound Module
script_module script;           // Scripting Module

/*******************************************************************************
                       Global Variable Declarations
*******************************************************************************/
korps_options game_options;     // Game Options
korps_setup game_setup;         // Game Setup
int frameCount;                 // Frame Counter
ALCcontext* ALContext;
ALCdevice* ALDevice;

/*******************************************************************************
    function    :   cleanup
    arguments   :   <none>
    purpose     :   Is called upon a call to exit(), which exits out of all
                    currently running engines (such as the TTF engine and SDL).
    notes       :   <none>
*******************************************************************************/
void cleanup()
{
    TTF_Quit();         // Quit True-Type Font engine
    
	// Disable context
	alcMakeContextCurrent(NULL);
	// Release context(s)
	alcDestroyContext(ALContext);
	// Close device
	alcCloseDevice(ALDevice);

    SDL_Quit();         // Quit SDL engine
}

/*******************************************************************************
    function    :   int main
    arguments   :   int argc, char *argv[]
    purpose     :   Main execution starting point.
    notes       :   Other game initializations are handled in it's own init()
                    function.
*******************************************************************************/
int main(int argc, char *argv[])
{
    SDL_Surface* temp;      // Temporary holder of SDL_Surface data
    char buffer[128];

    srand(time(NULL));      // Init random number generator
    
    // Load game options from settings.ini file
    loader.loadGameSettings(argc, argv);
    
    // Initialize audio and check for failure
    //alutInit(&argc, argv);	old code
	
	/*ALDevice = alcOpenDevice(NULL); // select the "preferred device"
	if(ALDevice == NULL)
	{
		write_error( "Sound: Could not initilize audio card." );
        exit(1);
	}
	
	// Create context(s)
	ALContext = alcCreateContext(ALDevice, NULL);
	if(ALContext == NULL)
	{
		write_error( "Sound: Could not initilize audio card." );
        exit(1);
	}
	
	// Set active context
	alcGetError(ALDevice);
	alcMakeContextCurrent(ALContext);
	if(alcGetError(ALDevice) != ALC_NO_ERROR)
	{
		write_error( "Sound: Could not initilize audio card." );
        exit(1);
	}
	
    sounds.init();*/
    
    // Initialize video and check for failure
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        // Write error and exit
        sprintf(buffer, "Error: FATAL: Unable to initialize video: %s",
            SDL_GetError());
        write_error(buffer);
        exit(1);
    }
    
    // Set up SDL to use OpenGL double buffering
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    // Set OpenGL to at *least* use 16 bit Hi-Color in the form
    // of a minimal of 5 bits Red, 6 bits Green, and 5 bits Blue.
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    
    // Set current video mode to the desired WidthxHeight, bpp, and
    // pass in the desired video flags & check for success.
    if(!(SDL_SetVideoMode(game_setup.screen_width, game_setup.screen_height, 24,
        (game_setup.full_screen ? SDL_FULLSCREEN : 0) | SDL_OPENGL)))
    {
        // Write error and exit
        sprintf(buffer, "Error: FATAL: Unable to set video mode: %s",
            SDL_GetError());
        write_error(buffer);
        SDL_Quit();
        exit(1);
    }
    
    // Set up the "professional"-looking options like the icon and
    // the window name. These are just for looks mostly... mostly.. :P
    SDL_WM_SetCaption("Korps Beta v0.6.0", "Korps");
    
    // To set our icon we need to load the icon file from disk - be
    // warned that this is a common memory leak if the loaded surface
    // is not freed afterwords.
    SDL_WM_SetIcon(temp = IMG_Load("korps.xpm"), NULL);
    SDL_FreeSurface(temp);
    
    // Initialize SDL's true type font engine
    TTF_Init();
    
    // Enable SDL keypress->unicode translation (for command console)
    SDL_EnableUNICODE(1);
    
    // And finally, make sure the program calls our cleanup function
    // upon exiting, which will cleanup anything else which needs to
    // be cleaned up before calling TTF_Quit and finally SDL_Quit.
    atexit(cleanup);
    
    // Initialize console
    console.addSysMessage(
        "                                      -= Korps Command Console =-");
    console.addSysMessage(" ");
    console.addSysMessage("System: Korps v0.6.0 Initialized Successfully.");
    
    // Start loader module and begin loading game
    loader.loadGame();
    
    // And finally kick into our main loop, never to return
    gameMainLoop(argc, argv);
    
    return 1;
}
