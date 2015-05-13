/*******************************************************************************
                           UI Module - Implementation
*******************************************************************************/
#include "main.h"
#include "ui.h"
#include "atg.h"
#include "camera.h"
#include "console.h"
#include "database.h"
#include "effects.h"
#include "fonts.h"
#include "gameloop.h"
#include "misc.h"
#include "model.h"
#include "object.h"
#include "objhandler.h"
#include "objlist.h"
#include "objmodules.h"
#include "scenery.h"
#include "sounds.h"
#include "tank.h"
#include "texture.h"

/*******************************************************************************
    function    :   ui_module::ui_module
    arguments   :   <none>
    purpose     :   Constructor.
    notes       :   <none>
*******************************************************************************/
ui_module::ui_module()
{
    usr_interface_base = NULL;
    usr_interface = NULL;
    usr_interface_width = usr_interface_height = usr_interface_size = 0;
    usr_interface_trans = 0.0;
    usr_interface_updated = false;
    
    mouse_down = false;
    mouse_dragging = false;
    mouse_coord_start_x = 0;
    mouse_coord_start_y = 0;
    mouse_coord_end_x = 0;
    mouse_coord_end_y = 0;
    
    picture_bg = NULL;
    picture_bg_width = picture_bg_height = 0;
}

/*******************************************************************************
    function    :   ui_module::~ui_module
    arguments   :   <none>
    purpose     :   Deconstructor.
    notes       :   <none>
*******************************************************************************/
ui_module::~ui_module()
{
    if(usr_interface_base)
        delete usr_interface_base;
    if(usr_interface)
        delete usr_interface;
    if(picture_bg)
        delete picture_bg;
    selection.empty();
}

/*******************************************************************************
    function    :   ui_module::loadUI
    arguments   :   <none>
    purpose     :   Loads the corresponding files into the user usr_interface.
    notes       :   <none>
*******************************************************************************/
void ui_module::loadUI()
{
    // Load base interfacing panel
    usr_interface_base = loadImage(
        "UI/usr_interface.png", 32, usr_interface_width, usr_interface_height);
    
    // See if usr_interface base loaded
    if(!usr_interface_base)
    {
        write_error("UI: FATAL: Could not load user usr_interface graphic.");
        exit(1);
    }
    
    // note: usr_interface panel is not flipped for OpenGL
    
    // Set memory size of usr_interface
    usr_interface_size = usr_interface_width * usr_interface_height * 4;
    
    // Create memory area for usr_interface
    usr_interface = new GLubyte[usr_interface_size];
    
    // Load picture backgrounding
    picture_bg = loadImage(
        "UI/picture_bg.png", 32, picture_bg_width, picture_bg_height);
    
    // Check to make sure we loaded it
    if(!usr_interface_base)
    {
        write_error("UI: FATAL: Could not load UI picture background.");
        exit(1);
    }
}

/*******************************************************************************
    Keyboard Event Handler Routines
*******************************************************************************/

/*******************************************************************************
    function    :   ui_module::keyboardDown
    arguments   :   key - key event structure
    purpose     :   Key press event handler.
    notes       :   <none>
*******************************************************************************/
void ui_module::keystrokeDown(SDL_KeyboardEvent &key)
{
    char buffer[128];
    int i;
    
    // Command console keyboard input override - re-route input to command
    // console if it is currently in the "pulled down" state.
    if(console.isPulledDown())
    {
        // Send character to the command console for handling. Note that unicode
        // is a 16 bit character, of which, we chop off the leading 9 bits.
        console.cmdEnter((char)(key.keysym.unicode & 0x7F));
        
        // Special input handler. These inputs only apply to when the command
        // console is in command entering mode or exiting command entering mode.
        // Note that since we enter/exit using the ENTER key, we will need to
        // not want to send the ENTER key event beyond the command console.
        // However, when we exit command entering mode, isEnteringCmd() is false
        // and thus we special case it to ensure a return; is hit.
        if(console.isEnteringCmd() || key.keysym.sym == SDLK_RETURN)
        {
            switch(key.keysym.sym)
            {
                // Grab a previous input
                case SDLK_UP:
                    console.cmdSpecial(CON_CMD_PREVIOUS);
                    break;
                
                // Grab next input (blank if none), basically the 180 of prev.
                case SDLK_DOWN:
                    console.cmdSpecial(CON_CMD_NEXT);
                    break;
                
                // Command console command abort (ESC key)
                case SDLK_ESCAPE:
                    console.cmdSpecial(CON_CMD_ABORT);
                    break;
                
                // Command console toggle override (~ tilde)
                case SDLK_BACKQUOTE:
                    console.togglePullDown();
                    break;
                
                default:
                    break;
            }
            
            // All of our commands that were re-routed to the command handler
            // must now NOT be sent to the game handler, otherwise this could
            // make other things happen while in console command entering mode.
            return;
        }
    }
    
    switch(key.keysym.sym)
    {
        case SDLK_ESCAPE:
            exit(0);
            break;            
        
        case SDLK_BACKQUOTE:
            console.togglePullDown();
            break;
        
        case SDLK_LEFTBRACKET:
            game_speed--;
            if(game_speed < -10)
                game_speed = -10;
            if(game_speed >= 0)
                sprintf(buffer, "Game Speed: %1.1fx", powf(2.0, (float)game_speed));
            else
                sprintf(buffer, "Game Speed: 1/%1.0fx", powf(2.0, fabs((float)game_speed)));
            console.addMessage(buffer);
            break;
        
        case SDLK_RIGHTBRACKET:
            game_speed++;
            if(game_speed > 4)
                game_speed = 4;
            if(game_speed >= 0)
                sprintf(buffer, "Game Speed: %1.1fx", powf(2.0, (float)game_speed));
            else
                sprintf(buffer, "Game Speed: 1/%1.0fx", powf(2.0, fabs((float)game_speed)));
            console.addMessage(buffer);
            break;
        
        case SDLK_QUOTE:
            game_speed = 0;
            console.addMessage("Game Speed: 1.0x");
            break;
        
        case SDLK_r:
            if(!selection.isEmpty() && selection.containsAttached())
            {
                ol_node* curr = selection.getHeadPtr();
                
                while(curr)
                {
                    if(curr->obj_ptr->obj_type == OBJ_TYPE_TANK ||
                       curr->obj_ptr->obj_type == OBJ_TYPE_VEHICLE ||
                       curr->obj_ptr->obj_type == OBJ_TYPE_ATG ||
                       curr->obj_ptr->obj_type == OBJ_TYPE_ATR)
                    {
                        for(i = 0; i < (dynamic_cast<firing_object*>(curr->obj_ptr))->sight_count; i++)
                            (dynamic_cast<firing_object*>(curr->obj_ptr))->sight[i].relieveTarget();
                    }
                    
                    curr = curr->next;
                }
            }
            break;
        
        case SDLK_s:
            if(!selection.isEmpty() && selection.containsAttached())
            {
                ol_node* curr = selection.getHeadPtr();
                
                while(curr)
                {
                    if(curr->obj_ptr->obj_type == OBJ_TYPE_TANK ||
                        curr->obj_ptr->obj_type == OBJ_TYPE_VEHICLE ||
                        curr->obj_ptr->obj_type == OBJ_TYPE_ATG ||
                        curr->obj_ptr->obj_type == OBJ_TYPE_ATR)
                    {
                        for(i = 0; i < (dynamic_cast<firing_object*>(curr->obj_ptr))->sight_count; i++)
                            (dynamic_cast<firing_object*>(curr->obj_ptr))->sight[i].relieveTarget();
                    }
                    if(curr->obj_ptr->obj_type == OBJ_TYPE_TANK ||
                        curr->obj_ptr->obj_type == OBJ_TYPE_VEHICLE)
                    {
                        (dynamic_cast<moving_object*>(curr->obj_ptr))->killWaypoints();
                    }
                    
                    curr = curr->next;
                }
            }
            break;
        
        case SDLK_d:
            debugMode = !debugMode;
            break;
        
        case SDLK_k:
            if(!selection.isEmpty())
            {
                if((selection.getHeadPtr())->obj_ptr->obj_type == OBJ_TYPE_TANK)
                {
                    if(SDL_GetModState() & KMOD_SHIFT)
                        (dynamic_cast<moving_object*>(selection.getHeadPtr()->obj_ptr))->motor.reduceMotorLife(1.0f);
                    else
                        (dynamic_cast<moving_object*>(selection.getHeadPtr()->obj_ptr))->motor.reduceMotorLife(0.1f);
                }
            }
            break;
        
        case SDLK_F9:
            if(game_options.tree_coverage == SC_COVERAGE_DENSE)
            {
                game_options.tree_coverage = SC_COVERAGE_NONE;
                console.addMessage("Tree coverage changed to: None");
            }
            else if(game_options.tree_coverage == SC_COVERAGE_MODERATE)
            {
                game_options.tree_coverage = SC_COVERAGE_DENSE;
                console.addMessage("Tree coverage changed to: Dense");
            }
            else if(game_options.tree_coverage == SC_COVERAGE_SPARSE)
            {
                game_options.tree_coverage = SC_COVERAGE_MODERATE;
                console.addMessage("Tree coverage changed to: Moderate");
            }
            else
            {
                game_options.tree_coverage = SC_COVERAGE_SPARSE;
                console.addMessage("Tree coverage changed to: Sparse");
            }
            break;
        
        case SDLK_F10:
            if(game_options.scenery_elements == SC_COVERAGE_DENSE)
            {
                game_options.scenery_elements = SC_COVERAGE_NONE;
                console.addMessage("Scenery elements changed to: None");
            }
            else if(game_options.scenery_elements == SC_COVERAGE_MODERATE)
            {
                game_options.scenery_elements = SC_COVERAGE_DENSE;
                console.addMessage("Scenery elements changed to: Dense");
            }
            else if(game_options.scenery_elements == SC_COVERAGE_SPARSE)
            {
                game_options.scenery_elements = SC_COVERAGE_MODERATE;
                console.addMessage("Scenery elements changed to: Moderate");
            }
            else
            {
                game_options.scenery_elements = SC_COVERAGE_SPARSE;
                console.addMessage("Scenery elements changed to: Sparse");
            }
            break;
        
        case SDLK_EQUALS:
        case SDLK_PLUS:
            if(camera.playerControlActive())
                camera.zoom(1);
            break;
        
        case SDLK_MINUS:
        case SDLK_UNDERSCORE:
            if(camera.playerControlActive())
                camera.zoom(-1);
            break;

        case SDLK_HOME:
            if(camera.playerControlActive())
                camera.elevate(1);
            break;
            
        case SDLK_END:
            if(camera.playerControlActive())
                camera.elevate(-1);
            break;
        
        case SDLK_a:
            if(SDL_GetModState() & KMOD_CTRL)
            {
                kVector pos = kVector(0.0, 0.0, 0.0);
                object_list* obj_list = objects.getUnitsNear(pos, 123456789);
                obj_list->filterUnattached();
                selection.setSelected(false);
                selection.empty();
                selection.fastAdd(obj_list);
                selection.setSelected(true);
                delete obj_list;
                updateInterfacingObject();
            }
            break;
        
        case SDLK_F1:
            {
                kVector pos = kVector(0.0, 0.0, 0.0);
                object_list* obj_list = objects.getUnitsNear(pos, 99999999);
                obj_list->keepPlatoon(1);
                selection.setSelected(false);
                selection.empty();
                selection.fastAdd(obj_list);
                selection.setSelected(true);
                delete obj_list;
                updateInterfacingObject();
            }
            break;
        
        case SDLK_F2:
            {
                kVector pos = kVector(0.0, 0.0, 0.0);
                object_list* obj_list = objects.getUnitsNear(pos, 99999999);
                obj_list->keepPlatoon(2);
                selection.setSelected(false);
                selection.empty();
                selection.fastAdd(obj_list);
                selection.setSelected(true);
                delete obj_list;
                updateInterfacingObject();
            }
            break;
        
        case SDLK_F3:
            {
                kVector pos = kVector(0.0, 0.0, 0.0);
                object_list* obj_list = objects.getUnitsNear(pos, 99999999);
                obj_list->keepPlatoon(3);
                selection.setSelected(false);
                selection.empty();
                selection.fastAdd(obj_list);
                selection.setSelected(true);
                delete obj_list;
                updateInterfacingObject();
            }
            break;
        
        case SDLK_F4:
            {
                kVector pos = kVector(0.0, 0.0, 0.0);
                object_list* obj_list = objects.getUnitsNear(pos, 99999999);
                obj_list->keepPlatoon(4);
                selection.setSelected(false);
                selection.empty();
                selection.fastAdd(obj_list);
                selection.setSelected(true);
                delete obj_list;
                updateInterfacingObject();
            }
            break;
        
        case SDLK_F12: // Take a screen capture and save it to disk
            {
                GLubyte* screen_shot_gl;
                SDL_Surface* screen_shot_sdl;
                char buffer[128];
                static int curr_cap = 1;
                ifstream fin;
                
                // Allocate memory for the screen capture
                screen_shot_gl = new GLubyte[game_setup.screen_width * game_setup.screen_height * 3];
                
                // Grab screen capture from OpenGL
                glReadPixels(0, 0, game_setup.screen_width, game_setup.screen_height, GL_BGR,
                    GL_UNSIGNED_BYTE, screen_shot_gl);
                
                // Flip image (since it will be in OpenGL 0,0 and not SDL 0,0)
                flipImage(screen_shot_gl, 24, game_setup.screen_width, game_setup.screen_height);
                
                // Create SDL surface from pixels
                screen_shot_sdl = SDL_CreateRGBSurfaceFrom(screen_shot_gl,
                    game_setup.screen_width, game_setup.screen_height, 24, game_setup.screen_width * 3,
                    0, 0, 0, 0);
                
                // Create filename based on capture number
                sprintf(buffer, "capture%03i.bmp", curr_cap++);
                
                // Work around to figure out how many screen caps are currently
                // residing in the directory without having to do OS-dependent
                // coding to figure out the capture number.
                fin.open(buffer);
                while(fin)
                {
                    // While file exists, try next file until we hit one that
                    // doesn't, and then use that one for the capture.
                    fin.close();
                    fin.clear();
                    sprintf(buffer, "capture%03i.bmp", curr_cap++);
                    fin.open(buffer);
                }
                fin.close();
                
                // Save it as a .bmp
                SDL_SaveBMP(screen_shot_sdl, buffer);
                
                // Cleanup used memory
                SDL_FreeSurface(screen_shot_sdl);
                delete screen_shot_gl;
            }
            break;
        
        default:
            break;
    }
}

/*******************************************************************************
    function    :   ui_module::keystrokeUp
    arguments   :   key - key event structure
    purpose     :   Key release event handler.
    notes       :   <none>
*******************************************************************************/
void ui_module::keystrokeUp(SDL_KeyboardEvent &key)
{
}

/*******************************************************************************
    Mouse Event Handler Routines
*******************************************************************************/

/*******************************************************************************
    function    :   ui_module::mouseMove
    arguments   :   motion - motion event structure
    purpose     :   Mouse motion event handler.
    notes       :   Mouse events still register xrel and yrel values even if
                    the mouse x,y position is against the side of the screen.
*******************************************************************************/
void ui_module::mouseMove(SDL_MouseMotionEvent &motion)
{
    static int offset = (int)((float)game_setup.screen_height * 0.65);
    
    // Check for player control
    if(!camera.playerControlActive())
    {
        camera.disable(CAM_MOVE_FORWARD);
        camera.disable(CAM_MOVE_BACKWARD);
        camera.disable(CAM_MOVE_LEFT);
        camera.disable(CAM_MOVE_RIGHT);
        camera.disable(CAM_ROT_LEFT);
        camera.disable(CAM_ROT_RIGHT);
        mouse_down = false;
        mouse_dragging = false;
        return;
    }
    
    // Handle mouse down and mouse drag
    if(mouse_down)
    {
        mouse_coord_end_x = motion.x;
        mouse_coord_end_y = motion.y;
        
        if(!mouse_dragging && (fabsf(mouse_coord_start_x - mouse_coord_end_x) >= 3 ||
            fabsf(mouse_coord_start_y - mouse_coord_end_y) >= 3))
            mouse_dragging = true;
        
        if(mouse_dragging)
        {
            camera.disable(CAM_MOVE_FORWARD);
            camera.disable(CAM_MOVE_BACKWARD);
            camera.disable(CAM_MOVE_LEFT);
            camera.disable(CAM_MOVE_RIGHT);
            camera.disable(CAM_ROT_LEFT);
            camera.disable(CAM_ROT_RIGHT);
            return;
        }
    }
    
    if(motion.y <= 5)
        camera.enable(CAM_MOVE_FORWARD);
    else
        camera.disable(CAM_MOVE_FORWARD);
    
    if(motion.y >= game_setup.screen_height - 6)
        camera.enable(CAM_MOVE_BACKWARD);
    else
        camera.disable(CAM_MOVE_BACKWARD);
    
    if(motion.x <= 5 && motion.y > offset)
        camera.enable(CAM_MOVE_LEFT);
    else
        camera.disable(CAM_MOVE_LEFT);
        
    if(motion.x >= game_setup.screen_width - 6 && motion.y > offset)
        camera.enable(CAM_MOVE_RIGHT);
    else
        camera.disable(CAM_MOVE_RIGHT);

    if(motion.x <= 5 && motion.y <= offset)
        camera.enable(CAM_ROT_LEFT);
    else
        camera.disable(CAM_ROT_LEFT);
        
    if(motion.x >= game_setup.screen_width - 6 && motion.y <= offset)
        camera.enable(CAM_ROT_RIGHT);
    else
        camera.disable(CAM_ROT_RIGHT);
}

/*******************************************************************************
    function    :   ui_module::mouseButtonDown()
    arguments   :   button - SDL Mouse Button Event
    purpose     :   register when and where the mouse was pushed down
    notes       :   <none>
*******************************************************************************/
void ui_module::mouseButtonDown(SDL_MouseButtonEvent &button)
{
    if(!camera.playerControlActive())
        return;
    
    switch(button.button)
    {
        case SDL_BUTTON_LEFT:
            mouse_down = true;
            mouse_coord_start_x = button.x;
            mouse_coord_start_y = button.y;
            break;
        
        case SDL_BUTTON_WHEELUP:
            camera.elevate(-1);
            break;
        
        case SDL_BUTTON_WHEELDOWN:
            camera.elevate(1);
            break;
    }
}

/*******************************************************************************
    function    :   ui_module::mouseButtonUp()
    arguments   :   button - SDL Mouse Button Event
    purpose     :   register when and where the mouse was let up
    notes       :   <none>
*******************************************************************************/
void ui_module::mouseButtonUp(SDL_MouseButtonEvent &button)
{
    if(!camera.playerControlActive())
        return;
    
    // Unselect ALL objects as to allow proper modifications to selection list
    // without having to worry about consistency.
    selection.setSelected(false);
    
    switch(button.button)
    {
        case SDL_BUTTON_LEFT:
            // Set mouse coordinates of release
            mouse_coord_end_x = button.x;
            mouse_coord_end_y = button.y;
            
            // Handler (nasty one too)
            if(mouse_down)
            {
                // Check against ammo selectors
                if(!mouse_dragging && !selection.isEmpty() &&
                    button.x >= 860 && button.x <= 882 &&
                    button.y >= 736 && button.y <= 749)
                {
                    int i, j;
                    firing_object* obj_ptr = dynamic_cast<firing_object*>(selection.getHeadPtr()->obj_ptr);
                    
                    if(obj_ptr && !obj_ptr->gun[0].isOutOfAmmo())
                    {
                        i = j = obj_ptr->gun[0].getAmmoInUsage();
                        do
                        {
                            i--;
                            if(i < 0) i = OBJ_MAX_AMMOPOOL - 1;
                            if(obj_ptr->gun[0].isFireable(i) && obj_ptr->ammo_pool[i] > 0)
                            {
                                obj_ptr->gun[0].setAmmoUsage(i);
                                obj_ptr->gun[0].unloadBreech();
                                break;
                            }
                        } while(i != j);
                    }
                }
                else if(!mouse_dragging && !selection.isEmpty() &&
                    button.x >= 960 && button.x <= 982 &&
                    button.y >= 736 && button.y <= 749)
                {
                    int i, j;
                    firing_object* obj_ptr = dynamic_cast<firing_object*>(selection.getHeadPtr()->obj_ptr);
                    
                    if(obj_ptr && !obj_ptr->gun[0].isOutOfAmmo())
                    {
                        i = j = obj_ptr->gun[0].getAmmoInUsage();
                        do
                        {
                            i++;
                            if(i >= OBJ_MAX_AMMOPOOL) i = 0;
                            if(obj_ptr->gun[0].isFireable(i) && obj_ptr->ammo_pool[i] > 0)
                            {
                                obj_ptr->gun[0].setAmmoUsage(i);
                                obj_ptr->gun[0].unloadBreech();
                                break;
                            }
                        } while(i != j);
                    }
                }
                else if(mouse_dragging)
                {
                    object_list* obj_list;
                    
                    // Get the list of objects we highlighted
                    obj_list = objects.getUnitsAt(
                        mouse_coord_start_x, mouse_coord_start_y, 
                        mouse_coord_end_x, mouse_coord_end_y);
                    
                    // Handle empty list
                    if(obj_list->isEmpty() && !(SDL_GetModState() & KMOD_SHIFT || SDL_GetModState() & KMOD_ALT))
                        selection.empty();
                    else if(selection.isEmpty())
                    {
                        if(!(SDL_GetModState() & KMOD_ALT))
                        {
                            if(obj_list->containsAttached())
                            {
                                // Filter unattached & add all objects
                                obj_list->filterUnattached();
                                selection.fastAdd(obj_list);
                            }
                            else if(obj_list->containsUnattached())
                            {
                                // Filter attached & add head object
                                obj_list->filterAttached();
                                if(!obj_list->isEmpty())
                                    selection.fastAdd(obj_list->getHeadPtr()->obj_ptr);
                            }
                        }
                    }
                    else if(selection.containsAttached())
                    {
                        if(obj_list->containsAttached())
                        {
                            // Filter unattached
                            obj_list->filterUnattached();
                            
                            if(SDL_GetModState() & KMOD_SHIFT)
                            {
                                // Add to current selection (if not already present)
                                selection.add(obj_list);
                            }
                            else if(SDL_GetModState() & KMOD_ALT)
                            {
                                // Remove from selection (if present)
                                selection.remove(obj_list);
                            }
                            else
                            {
                                // Select new objects
                                selection.empty();
                                selection.fastAdd(obj_list);
                            }
                        }
                        else if(obj_list->containsUnattached())
                        {
                            if(!(SDL_GetModState() & KMOD_ALT))
                            {
                                obj_list->filterAttached();
                                if(!obj_list->isEmpty())
                                {
                                    selection.empty();
                                    selection.fastAdd(obj_list->getHeadPtr()->obj_ptr);
                                }
                            }
                        }
                    }
                    else if(selection.containsUnattached())
                    {
                        if(obj_list->containsAttached())
                        {
                            if(!(SDL_GetModState() & KMOD_ALT))
                            {
                                obj_list->filterUnattached();
                                selection.empty();
                                selection.fastAdd(obj_list);
                            }
                        }
                        else if(obj_list->containsUnattached())
                        {
                            obj_list->filterAttached();
                            
                            if(SDL_GetModState() & KMOD_ALT)
                            {
                                // Remove from selection (if present)
                                selection.remove(obj_list);
                            }
                            else
                            {
                                if(!obj_list->isEmpty())
                                {
                                    selection.empty();
                                    selection.fastAdd(obj_list->getHeadPtr()->obj_ptr);
                                }
                            }
                        }
                    }
                    
                    delete obj_list;
                }
                else
                {
                    object* obj_ptr;
                    ol_node* curr;
                    kVector pos;
                    int i;
                    
                    // Grab object we clicked on
                    obj_ptr = objects.getUnitAt(button.x, button.y);
                    
                    if(selection.isEmpty())
                    {
                        if(obj_ptr && !(SDL_GetModState() & KMOD_ALT))
                            selection.fastAdd(obj_ptr);
                    }
                    else
                    {
                        // Note: From this point forward, it can be safely assumed that
                        // there exists at least one object in the selection list.
                        if(!obj_ptr)
                        {
                            if(selection.containsAttached())
                            {
                                // Get position vector existing on top of map from the
                                // intersection of AHM with the direction vector formed
                                // where mouse was clicked on the screen.
                                pos = normalized(camera.vectorAt(button.x, button.y));
                                pos = map.rayIntersect(camera.getCamPosV(), pos);
                                pos[1] = map.getOverlayHeight(pos[0], pos[2]);
                                
                                if(SDL_GetModState() & KMOD_CTRL)
                                {
                                    // Attack Ground Command
                                    curr = selection.getHeadPtr();
                                    while(curr)
                                    {
                                        // Note: Temporary targeting code.
                                        if(curr->obj_ptr->obj_type == OBJ_TYPE_TANK)
                                        {
                                            for(i = 0; i < (dynamic_cast<tank_object*>(curr->obj_ptr))->sight_count; i++)
                                                (dynamic_cast<tank_object*>(curr->obj_ptr))->sight[i].assignTarget(pos());
                                        }
                                        else if(curr->obj_ptr->obj_type == OBJ_TYPE_ATG)
                                        {
                                            (dynamic_cast<atg_object*>(curr->obj_ptr))->sight[0].assignTarget(pos());
                                        }
                                        curr = curr->next;
                                    }
                                }
                                else
                                {
                                    // Move Command
                                     
                                    // note: Obviously formation running code would go
                                    // here, but since I don't know how to approach that
                                    // just yet I just have code here which moves the
                                    // selected units based on offseted position from
                                    // the head object (obviously which would be the
                                    // commander - if selection is sorted that is).
                                    
                                    // Set obj_ptr to always point to the head object.
                                    curr = selection.getHeadPtr();
                                    obj_ptr = curr->obj_ptr;
                                    
                                    // Direct head object to move to this point.
                                    if(obj_ptr->obj_type == OBJ_TYPE_TANK ||
                                       obj_ptr->obj_type == OBJ_TYPE_VEHICLE)
                                    {
                                        if(!(SDL_GetModState() & KMOD_SHIFT))
                                            (dynamic_cast<moving_object*>(obj_ptr))->killWaypoints();
                                        if(!(SDL_GetModState() & KMOD_ALT))
                                            (dynamic_cast<moving_object*>(obj_ptr))->addWaypoint(pos);
                                        else
                                            (dynamic_cast<moving_object*>(obj_ptr))->addWaypoint(pos, WP_MOD_REVERSE);
                                    }
                                    
                                    curr = curr->next;
                                    
                                    // Direct all further objects to move to an offseted
                                    // position to this point based on their offset from
                                    // the head object.
                                    while(curr)
                                    {
                                        if(curr->obj_ptr->obj_type == OBJ_TYPE_TANK ||
                                           curr->obj_ptr->obj_type == OBJ_TYPE_VEHICLE)
                                        {
                                            if(!(SDL_GetModState() & KMOD_SHIFT))
                                                (dynamic_cast<moving_object*>(curr->obj_ptr))->killWaypoints();
                                            if(!(SDL_GetModState() & KMOD_ALT))
                                                (dynamic_cast<moving_object*>(curr->obj_ptr))->addWaypoint(
                                                    (curr->obj_ptr->pos - obj_ptr->pos) + pos);
                                            else
                                                (dynamic_cast<moving_object*>(curr->obj_ptr))->addWaypoint(
                                                    (curr->obj_ptr->pos - obj_ptr->pos) + pos, WP_MOD_REVERSE);
                                        }
                                        
                                        curr = curr->next;
                                    }
                                }
                            }
                            else
                            {
                                if(selection.containsSide(game_setup.player_side))
                                    // Invalid Move/Attack Ground Command
                                    console.addComMessage("Cannot comply. Selected unit is not under your command.");
                            }
                        }
                        else // an object was selected
                        {
                            // Note: From this point forward, it can be assumed that
                            // an object was clicked on.
                            if(selection.containsAttached())
                            {
                                if(!isUnitSide(obj_ptr, game_setup.player_side) ||
                                   SDL_GetModState() & KMOD_CTRL)
                                {
                                    // Attack Command
                                    curr = selection.getHeadPtr();
                                    while(curr)
                                    {
                                        // Note: Temporary targeting code.
                                        if(curr->obj_ptr->obj_type == OBJ_TYPE_TANK)
                                        {
                                            for(i = 0; i < (dynamic_cast<tank_object*>(curr->obj_ptr))->sight_count; i++)
                                                (dynamic_cast<tank_object*>(curr->obj_ptr))->sight[i].assignTarget(obj_ptr);
                                        }
                                        else if(curr->obj_ptr->obj_type == OBJ_TYPE_ATG)
                                        {
                                            (dynamic_cast<atg_object*>(curr->obj_ptr))->sight[0].assignTarget(obj_ptr);
                                        }
                                        curr = curr->next;
                                    }
                                }
                                else
                                {
                                    // Object Selection Command
                                    if(isUnitAttached(obj_ptr))
                                    {
                                        if(SDL_GetModState() & KMOD_SHIFT)
                                        {
                                            // Add to current selection (if not already present)
                                            selection.add(obj_ptr);
                                        }
                                        else if(SDL_GetModState() & KMOD_ALT)
                                        {
                                            // Remove from selection (if present)
                                            selection.remove(obj_ptr);
                                        }
                                        else
                                        {
                                            // Select new object
                                            selection.empty();
                                            selection.fastAdd(obj_ptr);
                                        }
                                    }
                                    else
                                    {
                                        if(isUnitSide(obj_ptr, game_setup.player_side) &&
                                           SDL_GetModState() & KMOD_SHIFT)
                                            // Invalid Object Selection Command
                                            console.addComMessage("Cannot comply. Selected unit is not under your command.");
                                        else if(!(SDL_GetModState() & KMOD_ALT))
                                        {
                                            // Select new object
                                            selection.empty();
                                            selection.fastAdd(obj_ptr);
                                        }
                                    }
                                }
                            }
                            else // selection does not contain attached
                            {
                                if(selection.containsSide(game_setup.player_side) &&
                                   !isUnitSide(obj_ptr, game_setup.player_side))
                                    // Invalid Attack Command
                                    console.addComMessage("Cannot comply. Selected unit is not under your command.");
                                else
                                {
                                    // Object Selection Command
                                    selection.empty();
                                    selection.fastAdd(obj_ptr);
                                }
                            }
                        }
                    }
                }
                
                // Reset mouse parameters to default, handler action complete.
                mouse_down = false;
                mouse_dragging = false;
            }
            break;
        
        case SDL_BUTTON_MIDDLE:
            if(!mouse_down && !mouse_dragging)
            {
                object_list* obj_list;
                object* obj_ptr;
                
                // Get the object underneath the mouse
                obj_ptr = objects.getUnitAt(button.x, button.y);
                
                // Check to see if object is attached
                if(obj_ptr && isUnitAttached(obj_ptr))
                {
                    // Get entire screen of objects
                    obj_list = objects.getUnitsAt(0, 0, game_setup.screen_width, game_setup.screen_height);
                    
                    // Filter unwanted
                    obj_list->filterUnattached();
                    obj_list->keepModel(obj_ptr->obj_model);
                    
                    // Check for existance
                    if(!obj_list->isEmpty())
                    {
                        if(SDL_GetModState() & KMOD_SHIFT)
                        {
                            // Add to current selection (if not already present)
                            selection.add(obj_list);
                        }
                        else if(SDL_GetModState() & KMOD_ALT)
                        {
                            // Remove from selection (if present)
                            selection.remove(obj_list);
                        }
                        else
                        {
                            // Select new object
                            selection.empty();
                            selection.fastAdd(obj_list);
                        }
                    }
                    
                    delete obj_list;
                }
            }
            break;
        
        case SDL_BUTTON_RIGHT:
            if(!mouse_down)
            {
                // Empty selection list.
                selection.empty();
            }
            break;
        
        default:
            break;
    }
    
    // Enable selection list
    selection.setSelected(true);
    
    // Sort selection list
    if(!selection.isEmpty())
        selection.sort();
    
    // Set usr_interface update flag
    updateInterfacingObject();
}

/*******************************************************************************
    Base Update & Display Routines
*******************************************************************************/

/*******************************************************************************
    function    :   ui_module::update
    arguments   :   deltaT - time elapsed since last call (in seconds)
    purpose     :   display
    notes       :   <none>
*******************************************************************************/
void ui_module::update(float deltaT)
{
    bool update_alpha_values = false;
    static float timer = 1.0;
    
    timer -= deltaT;
    if(timer <= 0.0)
    {
        updateInterfacingObject();
        timer = 1.0;
    }
    
    // Update the usr_interface alpha value
    if(!selection.isEmpty() && usr_interface_trans < 1.0)
    {
        usr_interface_trans += 2.5 * deltaT;
        
        if(usr_interface_trans >= 1.0 - FP_ERROR)
            usr_interface_trans = 1.0;
        
        // Set flag to update alpha values in image
        update_alpha_values = true;
    }
    else if(selection.isEmpty() && usr_interface_trans > 0.0)
    {
        usr_interface_trans -= 2.5 * deltaT;
        
        if(usr_interface_trans <= FP_ERROR)
        {
            usr_interface_trans = 0.0;
            updateInterfacingObject();
        }
        
        // Set flag to update alpha values in image
        update_alpha_values = true;
    }
    
    // Update alpha values of usr_interface graphic.
    if(update_alpha_values)
    {
        int i;
        GLubyte alpha_value = (int)(usr_interface_trans * 255.0f);
        for(i = 3; i < usr_interface_size; i += 4)
            usr_interface[i] = alpha_value;
    }
}

/*******************************************************************************
    function    :   ui_module::display
    arguments   :   <none>
    purpose     :   Displays the user usr_interface.
    notes       :   <none>
*******************************************************************************/
void ui_module::display()
{
    int i, j;
    char* temp;
    char buffer[128];
    GLubyte* text;
    int text_width;
    int text_height;
    GLubyte alpha_value;
    
    // Display selection box for object selection.
    if(mouse_dragging)
    {
        glEnable(GL_COLOR_MATERIAL);
        glEnable(GL_LINE_SMOOTH);
        glLineWidth(2.0);
        glColor4f(1.0, 1.0, 1.0, 0.9);
        glBegin(GL_LINE_LOOP);
            glVertex2i(mouse_coord_start_x, mouse_coord_start_y);
            glVertex2i(mouse_coord_end_x, mouse_coord_start_y);
            glVertex2i(mouse_coord_end_x, mouse_coord_end_y);
            glVertex2i(mouse_coord_start_x, mouse_coord_end_y);
        glEnd();
        glLineWidth(1.0);
        glDisable(GL_LINE_SMOOTH);
        glDisable(GL_COLOR_MATERIAL);        
    }
    
    // Display usr_interface.
    if(usr_interface_trans > 0.0)
    {
        // See if the usr_interface needs updation.
        if(!usr_interface_updated && !selection.isEmpty())
        {
            object* obj_ptr = selection.getHeadPtr()->obj_ptr;
            unit_object* uobj_ptr = NULL;
            firing_object* fobj_ptr = NULL;
            
            if(obj_ptr->obj_type == OBJ_TYPE_TANK ||
               obj_ptr->obj_type == OBJ_TYPE_VEHICLE ||
               obj_ptr->obj_type == OBJ_TYPE_ATG ||
               obj_ptr->obj_type == OBJ_TYPE_ATR)
            {
                uobj_ptr = dynamic_cast<unit_object*>(obj_ptr);
                fobj_ptr = dynamic_cast<firing_object*>(obj_ptr);
            }
            
            // Check for successful casting & go into drawing data elements.
            if(obj_ptr && uobj_ptr)
            {
                // Copy base back overtop of usr_interface for redraw.
                for(i = 0; i < usr_interface_size; i++)
                    usr_interface[i] = usr_interface_base[i];
                
                // Draw picture background
                blitImage(picture_bg, usr_interface, 32, picture_bg_width, picture_bg_height,
                    usr_interface_width, usr_interface_height, 5, 25);
                
                // Draw overriding picture
                blitImage(uobj_ptr->picture, usr_interface, 32, uobj_ptr->picture_width, uobj_ptr->picture_height,
                    usr_interface_width, usr_interface_height, 10, 30);
                
                text = fonts.generateText("__Unit__________________", FONT_ARIAL_14, text_width, text_height);
                if(text)
                {
                    blitImage(text, usr_interface, 32, text_width, text_height,
                        usr_interface_width, usr_interface_height, 140, 25);
                    delete text;
                }
                
                // Draw long display name
                temp = db.query(obj_ptr->obj_model, "LONG_NAME");
                if(temp)
                {
                    text = fonts.generateText(temp, FONT_ARIAL_14, text_width, text_height);
                    if(text)
                    {
                        blitImage(text, usr_interface, 32, text_width, text_height,
                            usr_interface_width, usr_interface_height, 150, 45);
                        delete text;
                    }
                }
                
                // Draw owner
                buffer[0] = '\0';
                if(obj_ptr->obj_modifiers & OBJ_MOD_GERMAN)
                    strcpy(buffer, "German");
                else if(obj_ptr->obj_modifiers & OBJ_MOD_BELGIAN)
                    strcpy(buffer, "Belgian");
                else if(obj_ptr->obj_modifiers & OBJ_MOD_BRITISH)
                    strcpy(buffer, "British");
                else if(obj_ptr->obj_modifiers & OBJ_MOD_FRENCH)
                    strcpy(buffer, "French");
                else
                    strcpy(buffer, "Polish");
                text = fonts.generateText(buffer, FONT_ARIAL_14, text_width, text_height);
                if(text)
                {
                    blitImage(text, usr_interface, 32, text_width, text_height,
                        usr_interface_width, usr_interface_height, 150, 65);
                    delete text;
                }
                
                // Draw rank
                buffer[0] = '\0';
                if(obj_ptr->obj_modifiers & OBJ_MOD_ELITE)
                    strcpy(buffer, "Elite");
                else if(obj_ptr->obj_modifiers & OBJ_MOD_VETERAN)
                    strcpy(buffer, "Veteran");
                else if(obj_ptr->obj_modifiers & OBJ_MOD_EXPERIENCED)
                    strcpy(buffer, "Experienced");
                else if(obj_ptr->obj_modifiers & OBJ_MOD_REGULAR)
                    strcpy(buffer, "Regular");
                else if(obj_ptr->obj_modifiers & OBJ_MOD_GREEN)
                    strcpy(buffer, "Green");
                else
                    strcpy(buffer, "Militia");
                text = fonts.generateText(buffer, FONT_ARIAL_14, text_width, text_height);
                if(text)
                {
                    blitImage(text, usr_interface, 32, text_width, text_height,
                        usr_interface_width, usr_interface_height, 230, 65);
                    delete text;
                }
                
                // Draw organization tag
                if(uobj_ptr->organization)
                {
                    text = fonts.generateText(uobj_ptr->organization, FONT_ARIAL_14, text_width, text_height);
                    if(text)
                    {
                        blitImage(text, usr_interface, 32, text_width, text_height,
                            usr_interface_width, usr_interface_height, 150, 85);
                        delete text;
                    }
                }
                
                // Draw attachment flag
                if(obj_ptr->obj_modifiers & OBJ_MOD_ATTACHED)
                {
                    text = fonts.generateText("<Attached>" , FONT_ARIAL_12, text_width, text_height);
                    if(text)
                    {
                        blitImage(text, usr_interface, 32, text_width, text_height,
                            usr_interface_width, usr_interface_height, 190, 105);
                        delete text;
                    }
                }
                
                // Draw crew information
                text = fonts.generateText("__Crew_____________________________________", FONT_ARIAL_14, text_width, text_height);
                if(text)
                {
                    blitImage(text, usr_interface, 32, text_width, text_height,
                        usr_interface_width, usr_interface_height, 340, 25);
                    delete text;
                }
                for(i = 0; i < uobj_ptr->crew.getCrewmanCount() && i < 5; i++)
                {
                    text = fonts.generateText(uobj_ptr->crew.getCrewmanName(i), FONT_ARIAL_12, text_width, text_height);
                    if(text)
                    {
                        blitImage(text, usr_interface, 32, text_width, text_height,
                            usr_interface_width, usr_interface_height, 350, 45 + (15 * i));
                        delete text;
                    }
                    
                    text = fonts.generateText(uobj_ptr->crew.getCrewmanHealthStatus(i), FONT_ARIAL_12, text_width, text_height);
                    if(text)
                    {
                        blitImage(text, usr_interface, 32, text_width, text_height,
                            usr_interface_width, usr_interface_height, 450, 45 + (15 * i));
                        delete text;
                    }
                    
                    text = fonts.generateText(uobj_ptr->crew.getCrewmanMoraleStatus(i), FONT_ARIAL_12, text_width, text_height);
                    if(text)
                    {
                        blitImage(text, usr_interface, 32, text_width, text_height,
                            usr_interface_width, usr_interface_height, 525, 45 + (15 * i));
                        delete text;
                    }
                    
                    text = fonts.generateText(uobj_ptr->crew.getCurrentJobText(i), FONT_ARIAL_12, text_width, text_height);
                    if(text)
                    {
                        blitImage(text, usr_interface, 32, text_width, text_height,
                            usr_interface_width, usr_interface_height, 575, 45 + (15 * i));
                        delete text;
                    }
                }
                
                // Draw gun information
                text = fonts.generateText("__Weapons_____________________________", FONT_ARIAL_14, text_width, text_height);
                if(text)
                {
                    blitImage(text, usr_interface, 32, text_width, text_height,
                        usr_interface_width, usr_interface_height, 700, 25);
                    delete text;
                }
                
                // Draw ammo pool
                text = fonts.generateText("Ammo Pool", FONT_ARIAL_12, text_width, text_height);
                if(text)
                {
                    blitImage(text, usr_interface, 32, text_width, text_height,
                        usr_interface_width, usr_interface_height, 710, 45);
                    delete text;
                }
                buffer[0] = '\0';
                for(i = 0; i < OBJ_MAX_AMMOPOOL; i++)
                {
                    if(fobj_ptr->ammo_pool_type[i] != NULL && fobj_ptr->ammo_pool[i] > 0)
                    {
                        if(i != OBJ_AMMOPOOL_MG)
                        {
                            temp = db.query(fobj_ptr->ammo_pool_type[i], "TYPE");
                            if(temp)
                                sprintf(&buffer[64], "%i %s", fobj_ptr->ammo_pool[i], temp);
                            else
                                sprintf(&buffer[64], "%i ?", fobj_ptr->ammo_pool[i]);
                        }
                        else
                            sprintf(&buffer[64], "%i MG", fobj_ptr->ammo_pool[i]);
                        
                        if(buffer[0] == '\0')
                            strcpy(buffer, &buffer[64]);
                        else
                        {
                            strcat(buffer, ", ");
                            strcat(buffer, &buffer[64]);
                        }
                    }
                }
                if(buffer[0] != '\0')
                    text = fonts.generateText(buffer, FONT_ARIAL_12, text_width, text_height);
                else
                    text = fonts.generateText("Out of Ammo", FONT_ARIAL_12, text_width, text_height);
                if(text)
                {
                    blitImage(text, usr_interface, 32, text_width, text_height,
                        usr_interface_width, usr_interface_height, 720, 60);
                    delete text;
                }
                
                // Draw guns
                text = fonts.generateText("Main Guns", FONT_ARIAL_12, text_width, text_height);
                if(text)
                {
                    blitImage(text, usr_interface, 32, text_width, text_height,
                        usr_interface_width, usr_interface_height, 710, 80);
                    delete text;
                }
                
                for(i = 0; i < fobj_ptr->gun_count && i < 5; i++)
                {
                    if(fobj_ptr->gun[i].isMainGun())
                    {
                        temp = db.query(fobj_ptr->gun[i].getGunType(), "DESIGNATION");
                        if(temp)
                        {
                            text = fonts.generateText(temp, FONT_ARIAL_12, text_width, text_height);
                            if(text)
                            {
                                blitImage(text, usr_interface, 32, text_width, text_height,
                                    usr_interface_width, usr_interface_height, 720, 95 + (15 * i));
                                delete text;
                            }
                        }
                        
                        if(!(fobj_ptr->gun[i].isOutOfAmmo()))
                        {
                            // Display left and right changers
                            text = fonts.generateText("[<<]", FONT_ARIAL_12, text_width, text_height);
                            if(text)
                            {
                                blitImage(text, usr_interface, 32, text_width, text_height,
                                    usr_interface_width, usr_interface_height, 860, 95 + (15 * i));
                                delete text;
                            }
                            text = fonts.generateText("[>>]", FONT_ARIAL_12, text_width, text_height);
                            if(text)
                            {
                                blitImage(text, usr_interface, 32, text_width, text_height,
                                    usr_interface_width, usr_interface_height, 960, 95 + (15 * i));
                                delete text;
                            }
                            // Draw current usage type
                            temp = db.query(fobj_ptr->ammo_pool_type[fobj_ptr->gun[i].getAmmoInUsage()], "TYPE");
                            if(temp)
                            {
                                text = fonts.generateText(temp, FONT_ARIAL_12, text_width, text_height);
                                if(text)
                                {
                                    blitImage(text, usr_interface, 32, text_width, text_height,
                                        usr_interface_width, usr_interface_height, 900, 95 + (15 * i));
                                    delete text;
                                }
                            }
                        }
                        else
                        {
                            // Display out of ammo message
                            text = fonts.generateText("Out of Ammo", FONT_ARIAL_12, text_width, text_height);
                            if(text)
                            {
                                blitImage(text, usr_interface, 32, text_width, text_height,
                                    usr_interface_width, usr_interface_height, 875, 95 + (15 * i));
                                delete text;
                            }
                        }
                    }
                }
                
                // Yay! Finished.
                usr_interface_updated = true;
                
                // Update alpha value for usr_interface.
                alpha_value = (int)(usr_interface_trans * 255.0f);
                for(i = 3; i < usr_interface_size; i += 4)
                    usr_interface[i] = alpha_value;
            }
        }
        
        // BLIT usr_interface to screen (1 line at a time)
        i = 0;
        j = (int)(usr_interface_trans * (float)usr_interface_height) - 1;
        while(j >= 0 && i < usr_interface_height)
        {
            glRasterPos2i(0, game_setup.screen_height - j);
            glDrawPixels(usr_interface_width, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                (GLvoid*)(usr_interface + ((usr_interface_width * i) * 4)));
            i++; j--;
        }
    }
}
