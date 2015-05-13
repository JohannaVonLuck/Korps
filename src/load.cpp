/*******************************************************************************
                        Loading Module - Implementation
*******************************************************************************/
#include "main.h"
#include "load.h"
#include "camera.h"
#include "database.h"
#include "effects.h"
#include "fonts.h"
#include "gameloop.h"
#include "misc.h"
#include "object.h"
#include "objhandler.h"
#include "scenery.h"
#include "script.h"
#include "sounds.h"
#include "texture.h"
#include "ui.h"

/*******************************************************************************
    function    :   loader_module::loader_module
    arguments   :   <none>
    purpose     :   Constructor.
    notes       :   Change max_load_value as project progresses, adds more bar.
*******************************************************************************/
loader_module::loader_module()
{
    load_value = 0;                             // Init bar loading at 0
    max_load_value = 130;                       // Max bar loading (changes!)
    strcpy(display_text, "Initializing...");    // Default start up message
    loading = true;                             // We are initially loading
}

/*******************************************************************************
    function    :   loader_module::loadGameSettings
    arguments   :   argc, argv - command line parameters
    purpose     :   Called at program start up to set global program options.
    notes       :   <none>
*******************************************************************************/
void loader_module::loadGameSettings(int argc, char *argv[])
{
    ifstream fin;
    char buffer[128];
    char value[64];
    
    // Open up settings file and load settings
    fin.open("settings.ini", ios::in);
    
    // Check for open
    if(!fin)
    {
        // If error, write error message and exit
        write_error(
            "Loader: FATAL: Failure loading \"settings.ini\" for read.");
        exit(1);
    }
    
    /* Load settings.ini */
    
    // Clean through whitespace
    eatjunk(fin);
    
    // Read in params
    while(!fin.eof())
    {
        // Get next param
        fin.getline(buffer, 120, '\n');
        
        // Parse through params
        if(buffer[0] == '#') ;
        // General Options
        else if(sscanf(buffer, "TREE_COVERAGE = %s", value))
        {
            if(strstr(value, "DENSE") || strstr(value, "dense"))
                game_options.tree_coverage = SC_COVERAGE_DENSE;
            else if(strstr(value, "MODERATE") || strstr(value, "moderate"))
                game_options.tree_coverage = SC_COVERAGE_MODERATE;
            else if(strstr(value, "SPARSE") || strstr(value, "sparse"))
                game_options.tree_coverage = SC_COVERAGE_SPARSE;
            else
                game_options.tree_coverage = SC_COVERAGE_NONE;
        }
        else if(sscanf(buffer, "SCENERY_ELEMENTS = %s", value))
        {
            if(strstr(value, "DENSE") || strstr(value, "dense"))
                game_options.scenery_elements = SC_COVERAGE_DENSE;
            else if(strstr(value, "MODERATE") || strstr(value, "moderate"))
                game_options.scenery_elements = SC_COVERAGE_MODERATE;
            else if(strstr(value, "SPARSE") || strstr(value, "sparse"))
                game_options.scenery_elements = SC_COVERAGE_SPARSE;
            else
                game_options.scenery_elements = SC_COVERAGE_NONE;
        }
        // Control Options
        else if(sscanf(buffer, "SCROLL_SPEED = %i", &game_options.scroll_speed)) ;
        else if(sscanf(buffer, "ROTATE_SPEED = %i", &game_options.rotate_speed)) ;
        // Video Options
        else if(sscanf(buffer, "WEATHER = %s", value))
        {
            if(strstr(value, "ENABLED") || strstr(value, "enabled") ||
               strstr(value, "T") || strstr(value, "t"))
                game_options.weather = true;
            else
                game_options.weather = false;
        }
        else if(sscanf(buffer, "SCREEN_SIZE = %s", value))
           sscanf(value, "%ix%i", &game_setup.screen_width, &game_setup.screen_height);
        else if(sscanf(buffer, "FULLSCREEN = %s", value))
        {
            if(strstr(value, "ENABLED") || strstr(value, "enabled") ||
               strstr(value, "T") || strstr(value, "t"))
                game_setup.full_screen = true;
            else
                game_setup.full_screen = false;
        }
        else if(sscanf(buffer, "FILTERING = %s", value))
        {
            game_setup.anisotropic = false;
            if(strstr(value, "ANISOTROPIC") || strstr(value, "anisotropic"))
            {
                game_setup.filtering = GL_LINEAR_MIPMAP_LINEAR;
                game_setup.anisotropic = true;
            }
            else if(strstr(value, "TRILINEAR") || strstr(value, "trilinear"))
                game_setup.filtering = GL_LINEAR_MIPMAP_LINEAR;
            else if(strstr(value, "LINEAR") || strstr(value, "linear"))
                game_setup.filtering = GL_LINEAR;
            else
                game_setup.filtering = GL_NEAREST;
        }
        else if(sscanf(buffer, "LOD_LEVEL = %s", value))
        {
            if(strstr(value, "HIGHEST") || strstr(value, "highest"))
                game_setup.lod_bias = -1.5;
            else if(strstr(value, "HIGHER") || strstr(value, "higher"))
                game_setup.lod_bias = -1.0;
            else if(strstr(value, "HIGH") || strstr(value, "high"))
                game_setup.lod_bias = -0.5;
            else
                game_setup.lod_bias = 0.0;
        }
        // Sound Options
        else if(sscanf(buffer, "SOUND = %s", value))
        {
            if(strstr(value, "ENABLED") || strstr(value, "enabled") ||
               strstr(value, "T") || strstr(value, "t"))
                game_options.sound = true;
            else
                game_options.sound = false;
        }
        else if(sscanf(buffer, "VOLUME = %i", &game_options.sound_volume)) ;
        else if(sscanf(buffer, "MAX_SOUNDS = %i", &game_options.max_sounds)) ;
        // Realism Options
        else if(sscanf(buffer, "PENETRATION = %s", value))
        {
            if(strstr(value, "US") || strstr(value, "us"))
                game_options.pen_system = CR_PEN_SYSTEM_US;
            else
                game_options.pen_system = CR_PEN_SYSTEM_RU;
        }
        else if(sscanf(buffer, "DIAMETER_FIX = %s", value))
        {
            if(strstr(value, "ENABLED") || strstr(value, "enabled") ||
               strstr(value, "T") || strstr(value, "t"))
                game_options.diameter_fix = true;
            else
                game_options.diameter_fix = false;
        }
        
        // Clean through whitespace
        eatjunk(fin);
    }
    fin.close();
    
    /* Load command line mission parameters */
    
    // Determine which map is being loaded
    if(argc == 1)
    {
        strcpy(game_setup.mission_folder, "Missions/Tutorial");
        game_setup.player_side = PS_AXIS;
        game_setup.player_nation = PN_GERMANY;
    }
    else if(argc == 2)
    {
        strcpy(game_setup.mission_folder, argv[1]);
        game_setup.player_side = PS_AXIS;
        game_setup.player_nation = PN_GERMANY;
    }
    else if(argc == 4)
    {
        strcpy(game_setup.mission_folder, argv[1]);
        if(strstr(argv[2], "AXIS"))
        {
            game_setup.player_side = PS_AXIS;
            game_setup.player_nation = PN_GERMANY;
        }   
        else
        {
            game_setup.player_side = PS_ALLIED;
            
            if(strstr(argv[3], "POLAND"))
                game_setup.player_nation = PN_POLAND;
            else if(strstr(argv[3], "FRANCE"))
                game_setup.player_nation = PN_FRANCE;
            else if(strstr(argv[3], "BRITIAN"))
                game_setup.player_nation = PN_BRITIAN;
            else
                game_setup.player_nation = PN_BELGIUM;
        }
    }
}

/*******************************************************************************
    function    :   loadGame
    arguments   :   <none>
    purpose     :   This function is called after main() initializes the SDL
                    engine and corresponding engines. It is loadGame's respons-
                    ibility to load each individual module of the game up at
                    the loading screen.
    notes       :   The final call to enter the gameLoop is handled by the
                    calling function, which in this case is main().
*******************************************************************************/
void loader_module::loadGame()
{
    char buffer[128];
    
    // Apply game setup and options loaded from settings
    
    // Apply sound settings
    sounds.setSystemEnabled(game_options.sound);
    sounds.setSystemVolume(game_options.sound_volume);
    sounds.setSystemMaxSounds(game_options.max_sounds);
    
    // Apply texturing settings
    textures.setAnisotropicy(game_setup.anisotropic);
    textures.setFiltering(game_setup.filtering);
    textures.setLODBias(game_setup.lod_bias);
    
    // Initialize viewport
    glViewport(0, 0, game_setup.screen_width, game_setup.screen_height);
    
    // Set background color to be a murky dark grey for loading screen
    glClearColor(0.21, 0.18, 0.15, 1.0);
    
    // Turn off the cursor for the time being while loading
    SDL_ShowCursor(SDL_DISABLE);
    
    // Enable OpenGL to do a perspective correction -> nicest
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    
    // Enable alpha transparency blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Load load screen data from disk
    load_screen = loadImage("Misc/base.png", 32,
        load_screen_width, load_screen_height);    
    flipImage(load_screen, 32, load_screen_width, load_screen_height);
    
    sprintf(buffer, "Misc/drop%i.jpg", choose(7));      // Pick random backdrop
    back_drop = loadImage(buffer, 32,
        back_drop_width, back_drop_height);
    flipImage(back_drop, 32, back_drop_width, back_drop_height);
    
    load_bar = loadImage("Misc/bar.bmp", 32,
        load_bar_width, load_bar_height);
    flipImage(load_bar, 32, load_bar_width, load_bar_height);
    
    // Refresh display before loading anything
    load_value = 0;
    display();
    
    /*  LOADING PROCESS BEGINS HERE  */
    
    strcpy(display_text, "Loading Reference Data");         // Load DB
    display();
    db.loadDirectory("Reference");
    load_value += 10;
    
    strcpy(display_text, "Loading Special Effects");        // Load SE
    display();
    effects.loadEffects();
    load_value += 10;
    
    strcpy(display_text, "Loading Sounds");                 // Load sound
    display();
    sounds.loadSounds();
    load_value += 10;
    
    strcpy(display_text, "Loading Scenery");                // Load scenery
    display();
    map.loadScenery(game_setup.mission_folder);
    
    strcpy(display_text, "Building Scenery");               // Build scenery
    display();
    map.buildScenery();
    
    strcpy(display_text, "Loading Objects");                // Load objects
    display();
    objects.loadMission(game_setup.mission_folder);
    load_value += 10;
    
    strcpy(display_text, "Loading Scripts");                // Load script
    display();
    script.load();
    load_value += 5;
    
    strcpy(display_text, "Loading User Interface");         // Load UI
    display();
    ui.loadUI();
    load_value += 10;
    
    /*  LOADING PROCESS ENDS HERE  */
    
    // Check for correct load value at end (good debugging tool)
    if(load_value != max_load_value)
    {
        sprintf(buffer, "Load: Did not hit max load value %i, hit %i instead.",
            max_load_value, load_value);
        write_error(buffer);
    }
    
    // Refresh display again before doing final steps
    load_value = max_load_value;
    display();
    
    // Kill loading screen
    loading = false;
    delete load_screen;
    delete back_drop;
    delete load_bar;
    
    // Setup OpenGL and SDL to further behave like we want it to before
    // jumping into main game loop.
    
    // Enable Z-Buffer for hidden surface removal
    glEnable(GL_DEPTH_TEST);                  // Enables Depth Testing
    glDepthFunc(GL_LEQUAL);                   // The Type Of Depth Testing To Do

    // Enable Alpha Test which will force alpha values less then delta to not be
    // written to the depth buffer. This is very important so that our objects
    // display correctly.
    glEnable(GL_ALPHA_TEST);
    
    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // Disable color material mapping initially
    glDisable(GL_COLOR_MATERIAL);
    
    // Setup Weather/Fog/Light0
    GLfloat diffuse[4] =  {0.40, 0.40, 0.40, 1.0};
    GLfloat ambient[4] =  {0.60, 0.60, 0.60, 1.0};
    GLfloat specular[4] = {0.25, 0.25, 0.25, 1.0};
    GLfloat fog[4] = {1.0, 1.0, 1.0, 1.0};
    glFogi(GL_FOG_MODE, GL_LINEAR);
    switch(map.getWeather())
    {
        case SC_WEATHER_CLEAR:
            glFogf(GL_FOG_START, 0.0);
            glFogf(GL_FOG_END, 10000.0);
            break;
        
        case SC_WEATHER_OVERCAST:
            diffuse[0] *= 0.70; diffuse[1] *= 0.70; diffuse[2] *= 0.70;
            ambient[0] *= 0.70; ambient[1] *= 0.70; ambient[2] *= 0.70;
            specular[0] *= 0.70; specular[1] *= 0.70; specular[2] *= 0.70;
            fog[0] = fog[1] = fog[2] = 0.75;
            glFogf(GL_FOG_START, 0.0);
            glFogf(GL_FOG_END, 3500.0);
            break;
        
        case SC_WEATHER_FOGGY:
            diffuse[0] *= 0.65; diffuse[1] *= 0.65; diffuse[2] *= 0.65;
            ambient[0] *= 0.65; ambient[1] *= 0.65; ambient[2] *= 0.65;
            specular[0] *= 0.65; specular[1] *= 0.65; specular[2] *= 0.65;
            fog[0] = fog[1] = fog[2] = 0.5;
            glFogf(GL_FOG_START, 0.0);
            glFogf(GL_FOG_END, 750.0);
            break;
        
        case SC_WEATHER_RAINING:
            diffuse[0] *= 0.70; diffuse[1] *= 0.70; diffuse[2] *= 0.70;
            ambient[0] *= 0.70; ambient[1] *= 0.70; ambient[2] *= 0.70;
            specular[0] *= 0.70; specular[1] *= 0.70; specular[2] *= 0.70;
            fog[0] = fog[1] = fog[2] = 0.65;
            glFogf(GL_FOG_START, 0.0);
            glFogf(GL_FOG_END, 2500.0);
            if(game_options.weather)
                effects.addEffect(SE_RAIN, kVector(0.0, 0.0, 0.0));
            break;
        
        case SC_WEATHER_DUSKDAWN:
            diffuse[0] *= 1.25; diffuse[1] *= 1.0; diffuse[2] *= 0.75;
            ambient[0] *= 1.25; ambient[1] *= 1.0; ambient[2] *= 0.75;
            specular[0] *= 1.25; specular[1] *= 1.0; specular[2] *= 0.75;
            glFogf(GL_FOG_START, 0.0);
            glFogf(GL_FOG_END, 10000.0);
            break;
        
        case SC_WEATHER_NIGHT:
            diffuse[0] *= 0.1; diffuse[1] *= 0.1; diffuse[2] *= 0.15;
            ambient[0] *= 0.1; ambient[1] *= 0.1; ambient[2] *= 0.15;
            specular[0] *= 0.1; specular[1] *= 0.1; specular[2] *= 0.15;
            fog[0] = fog[1] = fog[2] = 0.25;
            glFogf(GL_FOG_START, 0.0);
            glFogf(GL_FOG_END, 10000.0);
            break;
    }
    glFogfv(GL_FOG_COLOR, fog);
    if(game_options.weather)
        glEnable(GL_FOG);
    
    // Set sun lighting
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    
    // Move mouse to center of screen (will cause a mouse_motion event, but
    // will be discarded since gameLoop discards all pending events before
    // entering into the main game loop).
    SDL_WarpMouse(game_setup.screen_width / 2, game_setup.screen_height / 2);
    
    // Turn on the mouse cursor
    SDL_ShowCursor(SDL_ENABLE);
    
    // Set background color to be sky blue
    glClearColor(107.0 / 255.0, 153.0 / 255.0, 212.0 / 255.0, 1.0);
    
    // Finally, reset camera to some default parameters.
    camera.zoomTo(0);
    camera.elevateTo(10);
    camera.forceValues();
    camera.elevateTo(4);
}

/*******************************************************************************
    function    :   display
    arguments   :   <none>
    purpose     :   This specific function controls how to redraw the loading
                    screen and is also called specifically by other functions
                    as the game loads to refresh the screen buffer.
    notes       :   <none>
*******************************************************************************/
void loader_module::display()
{
    static int i;
    static unsigned int memory_ptr;
    SDL_Event event;                // Our event storage variable for ESC cap
    
    // Protect against calls which are done when loading is not in progress
    if(!loading)
        return;
    
    // Check for ESC key press and exit if ESC is ever pressed
    while(SDL_PollEvent(&event))
        if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
            exit(0);
    
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Setup GL for 2D overlay rendering
    glMatrixMode(GL_PROJECTION);    // Set to PROJECTION matrix mode
    glLoadIdentity();               // Load identity matrix
    gluOrtho2D(0.0, game_setup.screen_width,    // Define a parallel projection
        game_setup.screen_height, 0.0);

    glMatrixMode(GL_MODELVIEW);     // Set to MODELVIEW matrix mode
    glLoadIdentity();               // Load identity matrix
    
    glPushMatrix();

    // Translate coordinates
    glTranslatef(
        (game_setup.screen_width - load_screen_width) / 2.0,
        (game_setup.screen_height - load_screen_height) / 2.0,
        0.0);
        
    // Display base load_screen image using OpenGL BLIT.
    glRasterPos2i(0, load_screen_height);
    glDrawPixels(
        load_screen_width,
        load_screen_height,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        load_screen);
    
    // Display back drop image using OpenGL BLIT.
    glRasterPos2i(0, 125 + back_drop_height);
    glDrawPixels(
        back_drop_width,
        back_drop_height,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        back_drop);
        
    // Display bar using a per-row OpenGL BLIT, this way we do not have to do
    // any scaling which, in practice, made the load bar look kinda wierd.
    memory_ptr = (unsigned int)(load_bar);
    
    for(i = 0; i < load_bar_height; i++)
    {
        glRasterPos2i(50, 526 + i);
        glDrawPixels(
            (int)(((float)load_value / (float)max_load_value) * (float)load_bar_width),
            1,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            (void*)memory_ptr);
            
        memory_ptr += load_bar_width * 4;
    }
    
    // Write some display text ontop of the loading screen for the user to see
    // whats currently being loaded/performed.
    fonts.renderText("Press [ESC] To Abort", 325, 650);    // Abort message
    fonts.renderText("Loading: ", 50, 495);
    fonts.renderText(game_setup.mission_folder, 125, 495);     // Map (directory) being loaded
    fonts.renderText(display_text, 50, 560);       // Display loading text
    fonts.renderText("Loading", FONT_GOTHIC_28, 345, 555);
    fonts.renderText("Beta v0.6.0", 700, 560);     // Version number
    
    glPopMatrix();
    
    // Swap buffers for output display
    SDL_GL_SwapBuffers();
}
