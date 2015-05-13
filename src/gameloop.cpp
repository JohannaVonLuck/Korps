/*******************************************************************************
                        Main Game Loop Implementation
*******************************************************************************/
#include "main.h"
#include "gameloop.h"
#include "atg.h"
#include "camera.h"
#include "collision.h"
#include "console.h"
#include "effects.h"
#include "fonts.h"
#include "metrics.h"
#include "model.h"
#include "objhandler.h"
#include "scenery.h"
#include "script.h"
#include "sounds.h"
#include "tank.h"
#include "texture.h"
#include "ui.h"

bool debugMode;
int game_speed = 0;

/*******************************************************************************
    Screen Update Routines
*******************************************************************************/

/*******************************************************************************
    function    :   <inline> reshape
    arguments   :   w - New width of screen
                    h - New height of screen
    purpose     :   Window resize event handler.
    notes       :   <none>
*******************************************************************************/
inline void reshape(int w, int h)
{
    game_setup.screen_width = w;                // Reset width and height to new values
    game_setup.screen_height = h;
    
    glViewport(0, 0, w, h);         // Set viewport for Projection Matrix
}

/*******************************************************************************
    function    :   display_3d
    arguments   :   <none>
    purpose     :   Sets up OpenGL to display 3D objects & then displays those
                    such objects.
    notes       :   <none>
*******************************************************************************/
inline void display_3d()
{
    static GLfloat position[4] = {0.0, 5000.0, 0.0, 1.0};
    
    // Setup GL for 3D rendering
    glMatrixMode(GL_PROJECTION);    // Select The Projection Matrix
    glLoadIdentity();               // Reset The Projection Matrix
    glViewport(0, 0,                // Set viewport
        game_setup.screen_width, game_setup.screen_height);
    glMatrixMode(GL_MODELVIEW);     // Select The Modelview Matrix
    glLoadIdentity();               // Reset The Modelview Matrix
    
    glEnable(GL_DEPTH_TEST);        // Enable depth testing
    glEnable(GL_LIGHTING);          // Enable lighting

    // Orient the camera
    camera.orient();
    
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    
    glAlphaFunc(GL_GEQUAL, ALPHA_PASS);
    
    // Display the map (first pass)
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    map.displayFirstPass();
    glPopAttrib();
    
    // Display objects from object handler (first pass)
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    objects.displayFirstPass();
    glPopAttrib();
    
    // Display scenery elements of map (second pass)
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    map.displaySecondPass();
    glPopAttrib();
    
    glAlphaFunc(GL_GREATER, 0.0);
    
    // Display objects from object handler (second pass)
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    objects.displaySecondPass();
    glPopAttrib();
    
    // Display objects from special effects
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    effects.display();
    glPopAttrib();
}

/*******************************************************************************
    function    :   display_2d
    arguments   :   <none>
    purpose     :   Sets up OpenGL to display 2D objects & then displays those
                    such objects.
    notes       :   <none>
*******************************************************************************/
inline void display_2d()
{
    // Setup GL for 2D overlay rendering
    glMatrixMode(GL_PROJECTION);    // Set to PROJECTION matrix mode
    glLoadIdentity();               // Load identity matrix
    gluOrtho2D(0.0, game_setup.screen_width,    // Define a parallel projection
        game_setup.screen_height, 0.0);
    
    glMatrixMode(GL_MODELVIEW);     // Set to MODELVIEW matrix mode
    glLoadIdentity();               // Load identity matrix
    
    glDisable(GL_DEPTH_TEST);       // Disable depth testing
    glDisable(GL_LIGHTING);         // Disable lighting
    glAlphaFunc(GL_GREATER, 0.0);
    
    // Perform 2D overlay rendering tasks
    
    // Display the user interface
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    ui.display();
    glPopAttrib();
    
    // Display text messaging console
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    console.display();
    glPopAttrib();
    
    if(debugMode)
    {
        // All temporary debug code here on out
        if(ui.isSelectionEmpty())
        {
            fonts.renderText("Object Selected: <NULL>", FONT_COURIER_12, 20, 75);
        }
        else
        {
            char buffer[128];
            int y = 75, by;
            sprintf(buffer, "Object Selected: %s", (dynamic_cast<unit_object*>(ui.getSelection()->getHeadPtr()->obj_ptr))->organization);
            fonts.renderText(buffer, FONT_COURIER_12, 20, y); y += 12;
            sprintf(buffer, "  ID       : %s", (dynamic_cast<unit_object*>(ui.getSelection()->getHeadPtr()->obj_ptr))->obj_id);
            fonts.renderText(buffer, FONT_COURIER_12, 20, y); y += 12;
            sprintf(buffer, "  Model    : %s", ui.getSelection()->getHeadPtr()->obj_ptr->obj_model);
            fonts.renderText(buffer, FONT_COURIER_12, 20, y); y += 12;
            sprintf(buffer, "  Type     : %04hi", ui.getSelection()->getHeadPtr()->obj_ptr->obj_type);
            fonts.renderText(buffer, FONT_COURIER_12, 20, y); y += 12;
            sprintf(buffer, "  Status   : %04hi", ui.getSelection()->getHeadPtr()->obj_ptr->obj_status);
            fonts.renderText(buffer, FONT_COURIER_12, 20, y); y += 12;
            sprintf(buffer, "  Modifiers: %04hX", ui.getSelection()->getHeadPtr()->obj_ptr->obj_modifiers);
            fonts.renderText(buffer, FONT_COURIER_12, 20, y); y += 12;
            sprintf(buffer, "  Organized: %04hX", (dynamic_cast<unit_object*>(ui.getSelection()->getHeadPtr()->obj_ptr))->obj_organization);
            fonts.renderText(buffer, FONT_COURIER_12, 20, y); y += 12;
            sprintf(buffer, "  Model ID : %i", ui.getSelection()->getHeadPtr()->obj_ptr->model_id);
            fonts.renderText(buffer, FONT_COURIER_12, 20, y); y += 12;
            sprintf(buffer, "  Position : <%.2f, %.2f, %.2f>", ui.getSelection()->getHeadPtr()->obj_ptr->pos[0], ui.getSelection()->getHeadPtr()->obj_ptr->pos[1], ui.getSelection()->getHeadPtr()->obj_ptr->pos[2]);
            fonts.renderText(buffer, FONT_COURIER_12, 20, y); y += 12;
            sprintf(buffer, "  Direction: <%.2f, %.2f, %.2f>", ui.getSelection()->getHeadPtr()->obj_ptr->dir[0], ui.getSelection()->getHeadPtr()->obj_ptr->dir[1], ui.getSelection()->getHeadPtr()->obj_ptr->dir[2]);
            fonts.renderText(buffer, FONT_COURIER_12, 20, y); y += 12;
            sprintf(buffer, "  Roll     : %.2f", ui.getSelection()->getHeadPtr()->obj_ptr->roll);
            fonts.renderText(buffer, FONT_COURIER_12, 20, y); y += 12;
            sprintf(buffer, "  Draw     : %i", (int)(ui.getSelection()->getHeadPtr()->obj_ptr->draw));
            fonts.renderText(buffer, FONT_COURIER_12, 20, y); y += 12;
            
            if(ui.getSelection()->getHeadPtr()->obj_ptr->obj_type == OBJ_TYPE_TANK)
            {
                tank_object* tank_obj_ptr = dynamic_cast<tank_object*>(ui.getSelection()->getHeadPtr()->obj_ptr);
                
                sprintf(buffer, "  LinearVel: %.2fms  %.2fkmh  %.2fmph", tank_obj_ptr->linear_vel, tank_obj_ptr->linear_vel * 3.6, tank_obj_ptr->linear_vel * 2.23693629);
                fonts.renderText(buffer, FONT_COURIER_12, 20, y); y += 12;
                sprintf(buffer, "  CurrGear : %i", tank_obj_ptr->motor.getCurrentGear());
                fonts.renderText(buffer, FONT_COURIER_12, 20, y); y += 12;
                sprintf(buffer, "  AngulrVel: %.2f", tank_obj_ptr->angular_vel);
                fonts.renderText(buffer, FONT_COURIER_12, 20, y); y += 12;
                sprintf(buffer, "  MotorLife: %.2f", tank_obj_ptr->motor.getMotorLife());
                fonts.renderText(buffer, FONT_COURIER_12, 20, y); y += 12;
                
                if(tank_obj_ptr->wl_head != NULL)
                {
                    y += 12;
                    sprintf(buffer, "  NextWPPos: <%.2f, %.2f, %.2f>", tank_obj_ptr->wl_head->waypoint[0], tank_obj_ptr->wl_head->waypoint[1], tank_obj_ptr->wl_head->waypoint[2]);
                    fonts.renderText(buffer, FONT_COURIER_12, 20, y); y += 12;
                    kVector dir = tank_obj_ptr->wl_head->waypoint - tank_obj_ptr->pos;
                    dir.convertTo(CS_SPHERICAL);
                    sprintf(buffer, "  NextWPDir: <%.2f, %.2f, %.2f>", dir[0], dir[1], dir[2]);
                    fonts.renderText(buffer, FONT_COURIER_12, 20, y); y += 12;
                }
                else
                {
                    y += 12;
                    fonts.renderText("  NextWPPos: <NULL>", FONT_COURIER_12, 20, y); y += 12;
                    fonts.renderText("  NextWPDir: <NULL>", FONT_COURIER_12, 20, y); y += 12;
                }
            }
            if(ui.getSelection()->getHeadPtr()->obj_ptr->obj_type == OBJ_TYPE_TANK ||
               ui.getSelection()->getHeadPtr()->obj_ptr->obj_type == OBJ_TYPE_ATG)
            {
                int i;
                firing_object* fr_obj_ptr = dynamic_cast<firing_object*>(ui.getSelection()->getHeadPtr()->obj_ptr);
                unit_object* unit_obj_ptr = dynamic_cast<unit_object*>(ui.getSelection()->getHeadPtr()->obj_ptr);
                
                y += 12;
                sprintf(buffer, "  AmmoPool : %i %i %i %i %i %i", fr_obj_ptr->ammo_pool[0], fr_obj_ptr->ammo_pool[1], fr_obj_ptr->ammo_pool[2], fr_obj_ptr->ammo_pool[3], fr_obj_ptr->ammo_pool[4], fr_obj_ptr->ammo_pool[5]);
                fonts.renderText(buffer, FONT_COURIER_12, 20, y); y += 12;
                
                by = y;
                
                for(i = 0; i < fr_obj_ptr->sight_count; i++)
                {
                    y += 12;
                    sprintf(buffer, "  SightDev : %i", i + 1);
                    fonts.renderText(buffer, FONT_COURIER_12, 20, y); y += 12;
                    sprintf(buffer, "  Elevate  : %.2f", fr_obj_ptr->sight[i].getElevate());
                    fonts.renderText(buffer, FONT_COURIER_12, 20, y); y += 12;
                    sprintf(buffer, "  Trans    : %.2f", fr_obj_ptr->sight[i].getTransverse());
                    fonts.renderText(buffer, FONT_COURIER_12, 20, y); y += 12;
                    sprintf(buffer, "  Status   : %i", fr_obj_ptr->sight[i].getSightStatus());
                    fonts.renderText(buffer, FONT_COURIER_12, 20, y); y += 12;
                }
                
                y = by;
                
                for(i = 0; i < fr_obj_ptr->gun_count; i++)
                {
                    y += 12;
                    sprintf(buffer, "  GunDev   : %i", i + 1);
                    fonts.renderText(buffer, FONT_COURIER_12, 200, y); y += 12;
                    sprintf(buffer, "  Breech   : %i", fr_obj_ptr->gun[i].getBreechStatus());
                    fonts.renderText(buffer, FONT_COURIER_12, 200, y); y += 12;
                    sprintf(buffer, "  BreechJob: %.2f", fr_obj_ptr->gun[i].getBreechTime());
                    fonts.renderText(buffer, FONT_COURIER_12, 200, y); y += 12;
                    sprintf(buffer, "  ClipLeft : %i", fr_obj_ptr->gun[i].getClipLeft());
                    fonts.renderText(buffer, FONT_COURIER_12, 200, y); y += 12;
                }
                
                y = 75 + 12;
                
                for(i = 0; i < unit_obj_ptr->crew.getCrewmanCount(); i++)
                {
                    sprintf(buffer, "  Crewman %i: %s %s %f %s %f", i+1,
                        unit_obj_ptr->crew.getCrewmanName(i),
                        unit_obj_ptr->crew.getCrewmanHealthStatus(i),
                        unit_obj_ptr->crew.getCrewmanHealth(i),
                        unit_obj_ptr->crew.getCrewmanMoraleStatus(i),
                        unit_obj_ptr->crew.getCrewmanMorale(i));
                    fonts.renderText(buffer, FONT_COURIER_12, 350, y); y += 12;
                    sprintf(buffer, "   Curr Job: %s %.2f",
                        unit_obj_ptr->crew.getCurrentJobText(i),
                        unit_obj_ptr->crew.getCurrentJobTime(i));
                    fonts.renderText(buffer, FONT_COURIER_12, 350, y); y += 12;
                    y += 6;
                }
            }
        }
    }
}

/*******************************************************************************
    function    :   <inline> display
    arguments   :   <none>
    purpose     :   Base display function which handles screen redrawing.
    notes       :   <none>
*******************************************************************************/
inline void display()
{
    // Keep track of frame count
    frameCount++;
    
    // Clear the display
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Display 3D objects
    display_3d();

    // Display 2D overlay objects
    display_2d();

    // Swap buffers for output display
    SDL_GL_SwapBuffers();
}

/*******************************************************************************
    Timer Routine
*******************************************************************************/

/*******************************************************************************
    function    :   <inline> timer
    arguments   :   unsigned int time_elapsed
    purpose     :   Timer function. Runs the update calls to the different
                    modules.
    notes       :   <none>
*******************************************************************************/
inline void timer(unsigned int time_elapsed)
{
    static int timers[] = {0,15000,5,10,20,50};
    static int interval[] = {0,15000,5,10,20,50};
    static int previous = time_elapsed;
    static int start_index = 0;
    int time_passed;
    
    int i;
    float deltaT;
    float speed;
    
    time_passed = time_elapsed - previous;
    previous = time_elapsed;
    
    for(i = start_index; i < 6; i++)
    {
        timers[i] -= time_passed;
        
        if(timers[i] <= FP_ERROR)
        {
            // Calculate deltaT
            deltaT = (float)(interval[i] - timers[i]) / 1000.0;
            
            switch(i)
            {
                // Case 0 is our timer initialize function
                case 0:
                    start_index = 1;    // Disable timer 0 from firing again
                    
                    // Run an initial update on the scenery module and
                    // through the objects
                    map.update(0.0);
                    objects.update(0.0);
                    
                    return;
                    break;
                
                // Case 1 is our Frames per Second counter
                case 1:
                    {
                        char buffer[128];
                        
                        // Display FPS message
                        sprintf(buffer, "Frames Per Second: %.2f",
                            ((float)frameCount / deltaT));
                        
                        console.addMessage(buffer);     // Add message to console
                        
                        frameCount = 0;                 // Reset for next FPS count
                    }
                    break;
                
                // Case 2 is our 5ms update control
                case 2:
                    if(game_speed == 0)
                        speed = 1.0;
                    else
                    {
                        if(game_speed > 0)
                            speed = powf(2.0, (float)game_speed);
                        else
                            speed = 1.0 / powf(2.0, fabs((float)game_speed));
                    }
                    
                    // Update camera
                    camera.update(deltaT);
                    
                    deltaT *= speed;
                    
                    // Update objects
                    objects.update(deltaT);
                    
                    break;
                
                // Case 3 is our 10ms update control
                case 3:
                    if(game_speed == 0)
                        speed = 1.0;
                    else
                    {
                        if(game_speed > 0)
                            speed = powf(2.0, (float)game_speed);
                        else
                            speed = 1.0 / powf(2.0, fabs((float)game_speed));
                    }
                    
                    // Update text console
                    console.update(deltaT);
                    
                    // Update user interface
                    ui.update(deltaT);
                    
                    // Update scenery
                    map.update(deltaT);
                    
                    deltaT *= speed;
                    
                    // Update collision detection/response engine
                    cdr.update(deltaT);
                    
                    // Update effects
                    effects.update(deltaT);
                    
                    // Update sounds
                    sounds.update();
                    
                    break;
                
                // Case 4 is our 20ms update control
                case 4:
                    if(game_speed == 0)
                        speed = 1.0;
                    else
                    {
                        if(game_speed > 0)
                            speed = powf(2.0, (float)game_speed);
                        else
                            speed = 1.0 / powf(2.0, fabs((float)game_speed));
                    }
                    
                    // Perform CD for projectiles
                    objects.cdProjPass();
                    
                    deltaT *= speed;
                    
                    script.update(deltaT);
                    break;
                
                // Case 4 is our 50ms update control
                case 5:
                    // Perform CD for objects
                    objects.cdObjPass(OBJ_TYPE_TANK, OBJ_TYPE_VEHICLE);
                    break;
                
                default:
                    break;
            }
            
            timers[i] = interval[i];        // Reset timer
        }
    }
}

/*******************************************************************************
    Game Loop Routine
*******************************************************************************/

/*******************************************************************************
    function    :   gameMainLoop
    arguments   :   argc - # of cmd line arguments
                    argv - string of cmd line arguments
    purpose     :   Main game execution loop.
    notes       :   All event handlers are all inline functions, which makes
                    this function look a whole lot nicer.
*******************************************************************************/
void gameMainLoop(int argc, char* argv[])
{
    SDL_Event event;    // Our event storage variable
    
    // Initialize timer events (first call to this function will essentially
    // do such).
    timer(SDL_GetTicks());
    
    // Before entering our game loop, kill all currently pending events
    while(SDL_PollEvent(&event))
        ;
    
    // Enter our main game loop event handler - this is not that
    // complicated of a structure, but it serves it's purpose.
    while(1)
    {
        // Process all events on our event queue
        while(SDL_PollEvent(&event))
        {
            // switch to corresponding event handler
            switch(event.type)
            {
                /* ----- Keyboard Events ----- */
            
                /*  Keystroke Press  */
                case SDL_KEYDOWN:
                    ui.keystrokeDown(event.key);
                    break;
                
                /*  Keystroke Release  */
                case SDL_KEYUP:
                    ui.keystrokeUp(event.key);
                    break;

                /* ----- Mouse Events ----- */
            
                /*  Mouse Motion  */
                case SDL_MOUSEMOTION:
                    ui.mouseMove(event.motion);
                    break;
                
                /*  Mouse Button Press  */
                case SDL_MOUSEBUTTONDOWN:
                    ui.mouseButtonDown(event.button);
                    break;
                
                /*  Mouse Button Release  */
                case SDL_MOUSEBUTTONUP:
                    ui.mouseButtonUp(event.button);
                    break;

                /* ----- Window Events ----- */
            
                /*  Portion of Window Unhidden  */
                case SDL_VIDEOEXPOSE:
                    break;
                
                /*  Window Resized  */
                case SDL_VIDEORESIZE:
                    reshape(event.resize.w, event.resize.h);
                    break;
                
                /*  Window Focus Lost/Gained  */
                case SDL_ACTIVEEVENT:
                    break;

                /*  OS Quit Message  */
                case SDL_QUIT:
                    exit(0);
                    break;
            }
        }
        
        // Perform timer functions
        timer(SDL_GetTicks());
        
        // Perform redisplay functions
        display();
    }
}
