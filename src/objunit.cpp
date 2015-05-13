/*******************************************************************************
                       Object Units - Implementation
*******************************************************************************/
#include "main.h"
#include "objunit.h"
#include "camera.h"
#include "database.h"
#include "effects.h"
#include "metrics.h"
#include "misc.h"
#include "model.h"
#include "object.h"
#include "projectile.h"
#include "scenery.h"
#include "sounds.h"
#include "texture.h"

/*******************************************************************************
    Helper Functions
*******************************************************************************/

/*******************************************************************************
    function    :   isUnitNation
    arguments   :   objPtr - Pointer to an object
                    nation - nation ID number
    purpose     :   Determines if said object is a part of the given nation.
    notes       :   <none>
*******************************************************************************/
bool isUnitNation(object* objPtr, int nation)
{
    if((nation == PN_GERMANY && (objPtr->obj_modifiers & OBJ_MOD_GERMAN)) ||
       (nation == PN_POLAND && (objPtr->obj_modifiers & OBJ_MOD_POLISH)) ||
       (nation == PN_FRANCE && (objPtr->obj_modifiers & OBJ_MOD_FRENCH)) ||
       (nation == PN_BRITIAN && (objPtr->obj_modifiers & OBJ_MOD_BRITISH)) ||
       (nation == PN_BELGIUM && (objPtr->obj_modifiers & OBJ_MOD_BELGIAN)))
        return true;
    return false;
}

/*******************************************************************************
    function    :   isUnitSide
    arguments   :   objPtr - Pointer to an object
                    side - side ID number
    purpose     :   Determines if said object is a part of the given side.
    notes       :   <none>
*******************************************************************************/
bool isUnitSide(object* objPtr, int side)
{
    if((side == PS_AXIS && (objPtr->obj_modifiers & OBJ_MOD_AXIS)) ||
       (side == PS_ALLIED && (objPtr->obj_modifiers & OBJ_MOD_ALLIED)))
        return true;
    return false;
}

/*******************************************************************************
    function    :   isUnitAttached
    arguments   :   objPtr - Pointer to an object
    purpose     :   Determines if said object is a part of player command
                    and control (e.g. attached to the player command tree).
    notes       :   <none>
*******************************************************************************/
bool isUnitAttached(object* objPtr)
{
    if(((game_setup.player_side == PS_AXIS &&
            (objPtr->obj_modifiers & OBJ_MOD_AXIS)) ||
       (game_setup.player_side == PS_ALLIED &&
            (objPtr->obj_modifiers & OBJ_MOD_ALLIED))) &&
       (objPtr->obj_modifiers & OBJ_MOD_ATTACHED))
        return true;
    return false;
}

/*******************************************************************************
    unit_object
*******************************************************************************/

/*******************************************************************************
    function    :   unit_object::unit_object
    arguments   :   <none>
    purpose     :   Constructor.
    notes       :   <none>
*******************************************************************************/
unit_object::unit_object()
{
    // Initialize base attributes
    obj_id = NULL;
    obj_routines = 0x0000;
    obj_organization = 0x0000;
    
    // No picture data
    picture = NULL;
    picture_width = picture_height = 0;
    
    // No organizational string
    organization = NULL;
    
    // Not selected
    selected = false;
    
    // Did not load picture
    picture_load = false;
    
    // No display lists
    hull_dspList = DSPLIST_NULL;
    selected_dspList = DSPLIST_NULL;
}

/*******************************************************************************
    function    :   unit_object::~unit_object
    arguments   :   <none>
    purpose     :   Deconstructor.
    notes       :   <none>
*******************************************************************************/
unit_object::~unit_object()
{
    // Delete any allocated memory
    if(obj_id)
        delete obj_id;
    
    if(organization)
        delete organization;
    
    if(picture_load)
        delete picture;
}

/*******************************************************************************
    function    :   unit_object::initUnit
    arguments   :   idTagStr - ID tag string
                    organizationStr - Organizational string construct (must be
                        in the form of B#-C#-P#-U# or I-#).
    purpose     :   Initializes a unit object based on the passed parameters.
    notes       :   <none>
*******************************************************************************/
void unit_object::initUnit(char* idTagStr, char* organizationStr)
{
    char* temp;
    int section[4] = {0,0,0,0};
    char buffer[128] = {'\0'};
    
    // Copy over ID tag
    if(idTagStr)
        obj_id = strdup(idTagStr);
    
    /* Build Order Feed Routines */
    temp = db.query(obj_model, "ORDER_RECIEVED_BY");
    if(temp)
    {
        sscanf(temp, "FEED_ROUTINE_%i", &section[3]);
        obj_routines = obj_routines | (((--section[3]) & 0x000F) << 12);
    }
    temp = db.query(obj_model, "ORDER_COMMANDED_BY");
    if(temp)
    {
        sscanf(temp, "FEED_ROUTINE_%i", &section[2]);
        obj_routines = obj_routines | (((--section[2]) & 0x000F) << 8);
    }
    temp = db.query(obj_model, "SPOTTING_BY");
    if(temp)
    {
        sscanf(temp, "FEED_ROUTINE_%i", &section[1]);
        obj_routines = obj_routines | (((--section[1]) & 0x000F) << 4);
    }
    temp = db.query(obj_model, "DRIVING_BY");
    if(temp)
    {
        sscanf(temp, "FEED_ROUTINE_%i", &section[0]);
        obj_routines = obj_routines | ((--section[0]) & 0x000F);
    }
    
    /* Build Organization String */
    if(!organizationStr)
    {
        organization = strdup("Undefined");
    }
    else if(sscanf(organizationStr, "%i-%i-%i-%i",
        &section[3], &section[2], &section[1], &section[0]) == 4)
    {
        obj_organization = (unsigned short)
            (((section[3] & 0x000F) << 12) | ((section[2] & 0x000F) << 8) |
             ((section[1] & 0x000F) << 4) | (section[0] & 0x000F));
        
        if(obj_modifiers & OBJ_MOD_GERMAN)
        {
            // German Naming Template
            if(section[0] > 0)
            {
                if(obj_type == OBJ_TYPE_TANK)
                {
                    if(section[0] == 1)
                        strcpy(buffer, "PzBef");
                    else
                        sprintf(buffer, "%i.Pz", section[0]);
                }
                else if(obj_type == OBJ_TYPE_VEHICLE)
                {
                    if(section[0] == 1)
                        strcpy(buffer, "BefWg");
                    else
                        sprintf(buffer, "%i.Wg", section[0]);
                }
                else
                {
                    if(section[0] == 1)
                        strcpy(buffer, "Bef");
                    else
                        sprintf(buffer, "%i.", section[0]);
                }
                                
                if(section[1] > 0 || section[2] > 0 || section[3] > 0)
                    strcat(buffer, ", ");
            }
            
            if(section[1] > 0)
            {
                sprintf(&buffer[strlen(buffer)], "%i.Zug", section[1]);
                
                if(section[2] > 0 || section[3] > 0)
                    strcat(buffer, ", ");
            }
            
            if(section[2] > 0)
            {
                sprintf(&buffer[strlen(buffer)], "%i.Kompanie", section[2]);
                
                if(section[3] > 0)
                    strcat(buffer, ", ");
            }
            
            if(section[3] > 0)
            {
                if(section[3] == 1)
                    strcat(buffer, "I.Bataillon");
                else if(section[3] == 2)
                    strcat(buffer, "II.Bataillon");
                else if(section[3] == 3)
                    strcat(buffer, "III.Bataillon");
                else if(section[3] == 4)
                    strcat(buffer, "IV.Bataillon");
                else if(section[3] == 5)
                    strcat(buffer, "V.Bataillon");
                else
                    sprintf(&buffer[strlen(buffer)], "%i.Bataillon", section[3]);
            }
        }
        /*else if(obj_modifiers & OBJ_MOD_BELGIAN)
        {
            // Dutch Naming Template
            ;   // Do later
        }
        else if(obj_modifiers & OBJ_MOD_BRITISH)
        {
            // British Naming Template
            ;   // Do later
        }
        else if(obj_modifiers & OBJ_MOD_FRENCH)
        {
            // French Naming Template
            ;
        }
        else if(obj_modifiers & OBJ_MOD_POLISH)
        {
            // Polish Naming Template
            ;
        }*/
        else
        {
            // Standard Naming Template
            if(section[0] > 0)
            {
                if(section[0] == 1)
                    strcpy(buffer, "Commander");
                else
                {
                    if(obj_type == OBJ_TYPE_TANK)
                    {
                        if(section[0] == 2)
                            strcat(buffer, "2nd Tank");
                        else if(section[0] == 3)
                            strcat(buffer, "3rd Tank");
                        else
                            sprintf(buffer, "%ith Tank", section[0]);
                    }
                    else if(obj_type == OBJ_TYPE_VEHICLE)
                    {
                        if(section[0] == 2)
                            strcat(buffer, "2nd Vehicle");
                        else if(section[0] == 3)
                            strcat(buffer, "3rd Vehicle");
                        else
                            sprintf(buffer, "%ith Vehicle", section[0]);
                    }
                    else
                    {
                        if(section[0] == 2)
                            strcat(buffer, "2nd Unit");
                        else if(section[0] == 3)
                            strcat(buffer, "3rd Unit");
                        else
                            sprintf(buffer, "%ith Unit", section[0]);
                    }
                }
                
                if(section[1] > 0 || section[2] > 0 || section[3] > 0)
                    strcat(buffer, ", ");
            }
            
            if(section[1] > 0)
            {
                if(section[1] == 1)
                    strcat(buffer, "1st Platoon");
                else if(section[1] == 2)
                    strcat(buffer, "2nd Platoon");
                else if(section[1] == 3)
                    strcat(buffer, "3rd Platoon");
                else
                    sprintf(&buffer[strlen(buffer)], "%ith Platoon", section[1]);
                
                if(section[2] > 0 || section[3] > 0)
                    strcat(buffer, ", ");
            }
            
            if(section[2] > 0)
            {
                if(section[2] == 1)
                    strcat(buffer, "Alpha Company");
                else if(section[2] == 2)
                    strcat(buffer, "Bravo Company");
                else if(section[3] == 3)
                    strcat(buffer, "Charlie Company");
                else if(section[3] == 4)
                    strcat(buffer, "Delta Company");
                else if(section[3] == 5)
                    strcat(buffer, "Echo Company");
                else
                {
                    strcat(buffer, "_ Company");
                    buffer[strlen(buffer) - 9] = 'F' + (section[2] - 1);
                }
                
                if(section[3] > 0)
                    strcat(buffer, ", ");
            }
            
            if(section[3] > 0)
            {
                if(section[3] == 1)
                    strcat(buffer, "1st Battalion");
                else if(section[3] == 2)
                    strcat(buffer, "2nd Battalion");
                else if(section[3] == 3)
                    strcat(buffer, "3rd Battalion");
                else
                    sprintf(&buffer[strlen(buffer)], "%ith Battalion", section[3]);
            }
        }
        
        organization = strdup(buffer);   // Copy over new string
    }
    else if(sscanf(organizationStr, "I-%i", &section[0]) == 1)
    {
        obj_organization = (unsigned short)(0xF000 | (section[0] & 0x0FFF));
        
        if(obj_modifiers & OBJ_MOD_GERMAN)
        {
            // German Naming Template
            sprintf(buffer, "%i. des Unabhangig", section[0]);
        }
        else if(obj_modifiers & OBJ_MOD_BELGIAN)
        {
            // Dutch Naming Template
            sprintf(buffer, "%i.Eenheid, Onafhankelijk", section[0]);
        }
        else if(obj_modifiers & OBJ_MOD_BRITISH)
        {
            // British Naming Template
            sprintf(buffer, "%i.Unit, Independent", section[0]);
        }
        else if(obj_modifiers & OBJ_MOD_FRENCH)
        {
            // French Naming Template
            sprintf(buffer, "%i.Unite de Independant", section[0]);
        }
        else if(obj_modifiers & OBJ_MOD_POLISH)
        {
            // Polish Naming Template
            sprintf(buffer, "%i.Jednostka, Niezalezny", section[0]);
        }
        else
        {
            // Standard Naming Template
            if(section[0] == 1)
                strcpy(buffer, "1st Unit, Independent");
            else if(section[0] == 2)
                strcpy(buffer, "2nd Unit, Independent");
            else if(section[0] == 3)
                strcpy(buffer, "3rd Unit, Independent");
            else
                sprintf(buffer, "%ith Unit, Independent", section[0]);
        }
        
        organization = strdup(buffer);   // Copy over new string
    }
    else
    {
        organization = strdup("Undefined");
    }
}

/*******************************************************************************
    function    :   unit_object::initUnit
    arguments   :   <none>
    purpose     :   Initializes a unit object (and its crew).
    notes       :   Done as a seperate function since ordering of inits matters.
*******************************************************************************/
void unit_object::initUnit()
{
    char* temp;
    char buffer[128];
    
    // Handle loading of picture
    temp = db.query(obj_model, "PICTURE_PTR");
    if(temp)
    {
        // Grab from DB
        picture = (GLubyte*)atoi(temp);
        temp = db.query(obj_model, "PICTURE_DIM");
        if(temp)
            sscanf(temp, "%i %i", &picture_width, &picture_height);
        else
            picture = NULL;
    }
    else
    {
        // Load picture
        sprintf(buffer, "Models/%s/%s.png", obj_model, obj_model);
        picture = loadImage(buffer, 32, picture_width, picture_height);
        
        // Insert into DB for later retrevial
        sprintf(buffer, "%i", (int)(picture));
        db.insert(obj_model, "PICTURE_PTR", buffer);
        sprintf(buffer, "%i %i", picture_width, picture_height);
        db.insert(obj_model, "PICTURE_DIM", buffer);
        
        // Set load flag
        picture_load = true;
    }
    
    // Grab EXT_CREW attach
    temp = db.query(obj_model, "EXT_CREW_ATTACH");
    if(temp)
    {
        if(strstr(temp, "HULL"))
            ext_crew_attach = OBJ_ATTACH_HULL;
        else if(strstr(temp, "TURRET"))
        {
            sscanf(temp, "TURRET%i", &ext_crew_attach);
            ext_crew_attach += OBJ_ATTACH_TURRET_OFF - 1;    // 1-1? ;)
        }
        else
        {
            sprintf(buffer, "Unit: Invalid attachment level \"%s\" for \"%s\".",
                temp, obj_model);
            write_error(buffer);
            obj_status = OBJ_STATUS_REMOVE;
            return;
        }
    }
    else
        ext_crew_attach = OBJ_ATTACH_HULL;
    
    // Initialize crew
    crew.initCrew((object*)this);
}

/*******************************************************************************
    function    :   object::displayForSelection
    arguments   :   <none>
    purpose     :   Displays the object as a rectangular box composing the
                    object based on its size, & setting up a glPushName for the
                    OpenGL selection buffer.
    notes       :   1) The display is based on drawing the sides of the
                       rectangular box of an object based on its size.
                    2) The glPushName command pushes an unsigned integer which
                       turns out to be just a 32 bit pointer to the object.
*******************************************************************************/
void unit_object::displayForSelection()
{
    float size_half[3];
    
    // Only display if in view
    if(draw)
    {
        // For ease
        size_half[0] = size[0] / 2.0;
        size_half[2] = size[2] / 2.0;
        
        // Push an integer tag (as a pointer to this object)
        glPushName((GLuint)this);
        
        // Orient object
        glPushMatrix();
        glMultMatrixf(hull_matrix);
        
        // Draw sides of rectangular box based on size property
        glBegin(GL_QUADS);
            // Front facing
            glVertex3f(-size_half[0], 0.0, size_half[2]);
            glVertex3f(size_half[0], 0.0, size_half[2]);
            glVertex3f(size_half[0], size[1], size_half[2]);
            glVertex3f(-size_half[0], size[1], size_half[2]);
            
            // Rear facing
            glVertex3f(-size_half[0], 0.0, -size_half[2]);
            glVertex3f(size_half[0], 0.0, -size_half[2]);
            glVertex3f(size_half[0], size[1], -size_half[2]);
            glVertex3f(-size_half[0], size[1], -size_half[2]);
            
            // Upward facing
            glVertex3f(-size_half[0], size[1], -size_half[2]);
            glVertex3f(size_half[0], size[1], -size_half[2]);
            glVertex3f(size_half[0], size[1], size_half[2]);
            glVertex3f(-size_half[0], size[1], size_half[2]);
            
            // Downward facing
            glVertex3f(-size_half[0], 0.0, -size_half[2]);
            glVertex3f(size_half[0], 0.0, -size_half[2]);
            glVertex3f(size_half[0], 0.0, size_half[2]);
            glVertex3f(-size_half[0], 0.0, size_half[2]);
            
            // Left facing
            glVertex3f(-size_half[0], 0.0, -size_half[2]);
            glVertex3f(-size_half[0], 0.0, size_half[2]);
            glVertex3f(-size_half[0], size[1], size_half[2]);
            glVertex3f(-size_half[0], size[1], -size_half[2]);
            
            // Right facing
            glVertex3f(size_half[0], 0.0, -size_half[2]);
            glVertex3f(size_half[0], 0.0, size_half[2]);
            glVertex3f(size_half[0], size[1], size_half[2]);
            glVertex3f(size_half[0], size[1], -size_half[2]);
        glEnd();
        
        glPopMatrix();
        
        glPopName();
    }
}

/*******************************************************************************
    moving_object
*******************************************************************************/

/*******************************************************************************
    function    :   moving_object::moving_object
    arguments   :   <none>
    purpose     :   Constructor.
    notes       :   <none>
*******************************************************************************/
moving_object::moving_object()
{
    linear_vel = 0.0;
    angular_vel = 0.0;
    max_angular_vel = 0.5;
    wl_head = NULL;
    wl_tail = NULL;
}

/*******************************************************************************
    function    :   moving_object::~moving_object
    arguments   :   <none>
    purpose     :   Deconstructor.
    notes       :   <none>
*******************************************************************************/
moving_object::~moving_object()
{
    killWaypoints();
}

/*******************************************************************************
    function    :   moving_object::addWaypoint
    arguments   :   waypoint - position vector of waypoint
                    modifiers - supplied modifiers for waypoint
    purpose     :   Adds a waypoint to the waypoint queue of the object.
    notes       :   <none>
*******************************************************************************/
void moving_object::addWaypoint(kVector waypoint, unsigned int modifiers)
{
    float job_time;
    
    // Make sure the waypoint's y value is that of the heightmap
    waypoint[1] = map.getOverlayHeight(waypoint[0], waypoint[2]);
    
    // Add waypoint to our linked list queue
    if(wl_head == NULL)
    {
        wl_head = new waypoint_node;
        wl_tail = wl_head;
    }
    else
    {
        wl_tail->next = new waypoint_node;
        wl_tail = wl_tail->next;
    }
    
    // Set waypoint
    wl_tail->waypoint = waypoint;
    
    // Set to passed modifiers
    wl_tail->modifiers = WP_MOD_NONE | modifiers;
    
    if(wl_tail->modifiers & WP_MOD_TRANSMITTED)
        wl_tail->jobID = JOB_NULL;      // Set no job ID
    else
    {
        // Add a job ID for this node since not already transmitted
        switch(obj_modifiers & OBJ_MOD_RANK)
        {
            case OBJ_MOD_ELITE:
                job_time = 2.0;
                break;
            
            case OBJ_MOD_VETERAN:
                job_time = 3.0;
                break;
            
            case OBJ_MOD_EXPERIENCED:
                job_time = 4.0;
                break;
            
            case OBJ_MOD_REGULAR:
                job_time = 5.0;
                break;
            
            case OBJ_MOD_GREEN:
                job_time = 6.0;
                break;
            
            case OBJ_MOD_MILITIA:
            default:
                job_time = 7.0;
                break;
        }
        wl_tail->jobID = crew.enqueJob("Receiving Order", job_time, (int)((obj_routines & 0xF000) >> 12));
    }
    
    // Set tail NULL end
    wl_tail->next = NULL;
}

/*******************************************************************************
    function    :   moving_object::killWaypoints
    arguments   :   <none>
    purpose     :   Removes all waypoints from the object. Useful for when
                    we need to remove all waypoints.
    notes       :   <none>
*******************************************************************************/
void moving_object::killWaypoints()
{
    waypoint_node* wl_curr;
    
    // Go through our linked list queue and delete allocated memory for WPs
    while((wl_curr = wl_head) != NULL)
    {
        if(wl_curr->jobID != JOB_NULL)          // Be sure to kill any jobs
            crew.finishJob(wl_curr->jobID);
        wl_head = wl_head->next;
        delete wl_curr;
    }
    wl_head = NULL;       // Important
    wl_tail = NULL;       // Important
}

/*******************************************************************************
    function    :   moving_object::updateMovement
    arguments   :   deltaT - number of seconds elapsed since last update.
    purpose     :   Updates the movement of a moving object. This involves
                    two phases. The first phase updates the waypoint control
                    system. The second phase updates the object orientation.
    notes       :   <none>
*******************************************************************************/
void moving_object::updateMovement(float deltaT)
{
    waypoint_node* wl_curr;
    kVector wp_dir;
    float yaw_offset;
    kVector front_left;
    kVector front_right;
    kVector rear_left;
    kVector rear_right;
    float desired_pitch;
    float desired_roll;
    float desired_height;
    
    // Normalize direction vector
    if(dir[2] >= TWOPI)
        dir[2] -= TWOPI;
    else if(dir[2] < 0.0)
        dir[2] += TWOPI;
    
    /* Phase 1: Update Waypoint System */
    
    // Movement is controled via waypoints.
    if(wl_head)
    {
        float tolerance;
        bool clean_up;
        
        // Clean-up any waypoints currently on the waypoint list and assign
        // an initial wp_dir vector to work with.
        do
        {
            clean_up = false;
            
            // Calculate waypoint direction and convert to spherical for our
            // pitch and yaw values (as well as distance value).
            wp_dir = (wl_head->waypoint - pos);
            wp_dir.convertTo(CS_SPHERICAL);
            
            // Calculate distance tolerance needed in order to remove waypoint
            // based on the yaw offset.
            if(wp_dir[0] > 10.0 || wp_dir[0] < 5.0)
                tolerance = 5.0;        // Set default distance tolerance
            else
            {
                // Calculate yaw offseting based on direction
                if(wl_head->modifiers & WP_MOD_FORWARD)
                    // Move forward towards waypoint
                    yaw_offset = wp_dir[2] - dir[2];
                else
                    // Move backward towards waypoint
                    yaw_offset = (wp_dir[2] + PI) - dir[2];
                
                // Normalize offset direction between -PI and PI
                if(yaw_offset < -PI)
                    yaw_offset += TWOPI;
                else if(yaw_offset > PI)
                    yaw_offset -= TWOPI;
                
                // Set distance tolerance
                tolerance = (fabsf(yaw_offset) * 0.0277778) + 5.0;
            }
            
            // Check to see if we have hit a waypoint (within 5.0 meters).
            if(wp_dir[0] <= tolerance)
            {
                // Cycle to next waypoint, deleting current
                if(wl_head->jobID != JOB_NULL)
                    crew.finishJob(wl_head->jobID);
                wl_curr = wl_head;
                wl_head = wl_head->next;
                delete wl_curr;
                
                // Check to see if we hit the end of the list.
                if(wl_head == NULL)
                    wl_tail = NULL;   // Make sure tail pointer is correct.
                
                clean_up = true;
            }
        } while(clean_up && wl_head);
    }
    
    // Update order transmissions
    wl_curr = wl_head;
    while(wl_curr)
    {
        if(!(wl_curr->modifiers & WP_MOD_TRANSMITTED) && crew.isJobFinished(wl_curr->jobID))
            wl_curr->modifiers = wl_curr->modifiers | WP_MOD_TRANSMITTED;
        wl_curr = wl_curr->next;
    }
    
    // Update movement of object
    if(wl_head && (wl_head->modifiers & WP_MOD_TRANSMITTED))
    {
        // Make sure head has a driving job
        if(wl_head->jobID == JOB_NULL)
            wl_head->jobID = crew.enqueJob("Driving...", JOB_TIME_UNLIMITED, (int)(obj_routines & 0x000F));
        
        // Check for handling of driving job
        if(!crew.getJobHandlement(wl_head->jobID))
        {
            // If not being driven, then no angular velocity or throttle.
            angular_vel = 0.0;
            motor.setThrottle(0.0);
        }
        else
        {
            // Normalize waypoint direction to be between -PI and PI, giving us
            // the ability to work in a 2PI system that specifically has a LEFT
            // and RIGHT side, otherwise it makes the problem to be solved up
            // ahead, namely figuring out which way to turn, a whole lot harder.
            
            // Calculate yaw offseting based on direction
            if(wl_head->modifiers & WP_MOD_FORWARD)
                // Move forward towards waypoint
                yaw_offset = wp_dir[2] - dir[2];
            else
                // Move backward towards waypoint
                yaw_offset = (wp_dir[2] + PI) - dir[2];
            
            // Normalize offset direction between -PI and PI
            if(yaw_offset < -PI)
                yaw_offset += TWOPI;
            else if(yaw_offset > PI)
                yaw_offset -= TWOPI;
            
            // Check to see if the object needs angular velocity to get it
            // to be 'on course'.
            if(fabsf(yaw_offset) < 0.001)
            {
                // If 0.001 tolerance, the we are right on course
                angular_vel = 0.0;
                if(wl_head->modifiers & WP_MOD_FORWARD)
                    dir[2] = wp_dir[2];
                else
                    dir[2] = wp_dir[2] + PI;
                yaw_offset = 0.0;
            }
            else
            {
                // Turn the object to get it 'on course'.
                if(yaw_offset < 0.0)
                {
                    // Turn Right
                    if(angular_vel > -max_angular_vel)
                    {
                        angular_vel -= 0.35 * deltaT;
                        
                        // Check for overdo
                        if(angular_vel < -max_angular_vel)
                            angular_vel = -max_angular_vel;
                    }                    
                    if(angular_vel < (yaw_offset + yaw_offset))
                        angular_vel = (yaw_offset + yaw_offset);
                    
                    // Update yaw of vehicle with new direction
                    dir[2] += angular_vel * deltaT;
                    
                    // Check for overdo
                    if(fabsf(yaw_offset) - (fabsf(angular_vel) * deltaT) <= FP_ERROR)
                    {
                        angular_vel = 0.0;
                        if(wl_head->modifiers & WP_MOD_FORWARD)
                            dir[2] = wp_dir[2];
                        else
                            dir[2] = wp_dir[2] + PI;
                    }
                }
                else
                {
                    // Turn Left
                    if(angular_vel < max_angular_vel)
                    {
                        angular_vel += 0.35 * deltaT;
                        
                        // Check for overdo
                        if(angular_vel > max_angular_vel)
                            angular_vel = max_angular_vel;
                    }
                    if(angular_vel > (yaw_offset + yaw_offset))
                        angular_vel = (yaw_offset + yaw_offset);
                    
                    // Update yaw of vehicle with new direction
                    dir[2] += angular_vel * deltaT;
                    
                    // Check for overdo
                    if(fabsf(yaw_offset) - (fabsf(angular_vel) * deltaT) <= FP_ERROR)
                    {
                        angular_vel = 0.0;
                        if(wl_head->modifiers & WP_MOD_FORWARD)
                            dir[2] = wp_dir[2];
                        else
                            dir[2] = wp_dir[2] + PI;
                    }
                }
            }
            
            // Update movement speed
            if(linear_vel < 0.0 && wl_head->modifiers & WP_MOD_FORWARD ||
               linear_vel > 0.0 && wl_head->modifiers & WP_MOD_REVERSE)
                motor.setThrottle(0.0);     // Moving wrong way!
            else if(wl_head->modifiers & WP_MOD_FORWARD)
            {
                // Check to see if we need to slow down to make a waypoint.
                if(wp_dir[0] <= 15.0)
                {
                    // Base our amount of slow down on the angle we are from WP.
                    if(fabsf(yaw_offset) < 15.0 * degToRad)
                        motor.setThrottle(100.0);
                    else if(fabsf(yaw_offset) > 90.0 * degToRad)
                        motor.setThrottle(10.0);
                    else
                        motor.setThrottle(145.326f * powf(0.968799f,
                            (fabsf(yaw_offset) * radToDeg) - 3.2f));
                }
                else
                    motor.setThrottle(100.0);      // Otherwise - just go
            }
            else if(wl_head->modifiers & WP_MOD_REVERSE)
                // Moving backward - just set throttle to 15%
                motor.setThrottle(15.0);
        }
    }
    else
    {
        // No waypoints - no angular velocity or throttle.
        angular_vel = 0.0;
        motor.setThrottle(0.0);
    }
    
    // Call our motor update to compute new speed value.
    motor.update(deltaT);
    
    // Get our speed value from the motor
    if((linear_vel >= 0.0 && !wl_head) ||
       (linear_vel > 0.0 && wl_head) ||
       (linear_vel == 0.0 && wl_head && wl_head->modifiers & WP_MOD_FORWARD))
        linear_vel = motor.getCurrentSpeed();
    else
        linear_vel = -motor.getCurrentSpeed();
    
    /* Phase 2: Update Orientation of Object */
    
    // Update position using current direction vector
    pos += vectorIn(dir, CS_CARTESIAN) * linear_vel * deltaT;
    
    // Set up values for basis vectors
    front_left[0] = rear_left[0] = size[0] / 2.0;
    front_right[0] = rear_right[0] = -front_left[0];
    front_left[2] = front_right[2] = size[2] / 2.0;
    rear_left[2] = rear_right[2] = -front_left[2];
    front_left[1] = front_right[1] = rear_left[1] = rear_right[1] = 0.0;
    
    // Transforms vectors based on orientation matrix
    front_left.transform((float*)hull_matrix);
    front_right.transform((float*)hull_matrix);
    rear_left.transform((float*)hull_matrix);
    rear_right.transform((float*)hull_matrix);
    
    // Get new height values for transformed values
    front_left[1] = map.getOverlayHeight(front_left[0], front_left[2]);
    front_right[1] = map.getOverlayHeight(front_right[0], front_right[2]);
    rear_left[1] = map.getOverlayHeight(rear_left[0], rear_left[2]);
    rear_right[1] = map.getOverlayHeight(rear_right[0], rear_right[2]);
    
    // Perform a quick collision test with scenery and heightmap
    while(
        (linear_vel > 0.0 && (
            map.getBlockmap(front_left[0], front_left[2]) == 255 ||
            map.getBlockmap(front_right[0], front_right[2]) == 255 ||
            fabsf(front_left[1] - pos[1]) > 2.5 ||
            fabsf(front_right[1] - pos[1]) > 2.5
        )) ||
        (linear_vel < 0.0 && (
            map.getBlockmap(rear_left[0], rear_left[2]) == 255 ||
            map.getBlockmap(rear_right[0], rear_right[2]) == 255 ||
            fabsf(rear_left[1] - pos[1]) > 2.5 ||
            fabsf(rear_right[1] - pos[1]) > 2.5
        )))
    {
        // If so - push back the object a bit, stop it, and kill all waypoints.
        pos += vectorIn(dir, CS_CARTESIAN) * -0.15f;
        angular_vel = 0.0;
        motor.setOutputSpeed(0.0);
        killWaypoints();
    }
    
    // Pitch Recalculation
    desired_pitch = PIHALF - (((
        atan((front_left[1] - rear_left[1]) / size[2])
            ) + (
        atan((front_right[1] - rear_right[1]) / size[2])    
            )) / 2.0);
    
    if(!(fabsf(desired_pitch - dir[1]) < 0.0001))
    {
        if(desired_pitch < dir[1])
        {
            dir[1] -= 0.22 * deltaT;
     
            // Check for overdo       
            if(dir[1] < desired_pitch)
                dir[1] = desired_pitch;
        }
        else if(desired_pitch > dir[1])
        {
            dir[1] += 0.22 * deltaT;
            
            // Check for overdo
            if(dir[1] > desired_pitch)
                dir[1] = desired_pitch;
        }
    }
    
    // Roll Recalculation
    desired_roll = (atan((front_left[1] - front_right[1]) / size[0]) +
        atan((rear_left[1] - rear_right[1]) / size[0])) / 2.0;
    
    if(!(fabsf(desired_roll - roll) < 0.0001))
    {
        if(desired_roll < roll)
        {
            roll -= 0.22 * deltaT;
            
            // Check for overdo
            if(roll < desired_roll)
                roll = desired_roll;
        }
        else if(desired_roll > roll)
        {
            roll += 0.22 * deltaT;
            
            // Check for overdo
            if(roll > desired_roll)
                roll = desired_roll;
        }
    }
    
    // Position Recalculation
    desired_height = map.getOverlayHeight(pos[0], pos[2]) + 0.0247;
    
    if(!(fabsf(desired_height - pos[1]) < 0.0001))
    {
        pos[1] += (desired_height - pos[1]) * 0.25 * (deltaT > 4.0 ?  4.0 : deltaT);
    }
    
    // Check to make sure we are not imbeded into the scenery too much
    if(pos[1] < desired_height - 1.25)
        pos[1] = desired_height - 1.25;
}

/*******************************************************************************
    firing_object
*******************************************************************************/

/*******************************************************************************
    function    :   firing_object::firing_object
    arguments   :   <none>
    purpose     :   Constructor.
    notes       :   <none>
*******************************************************************************/
firing_object::firing_object()
{
    int i;
    
    // Initialize variables
    for(i = 0; i < OBJ_MAX_AMMOPOOL; i++)
    {
        ammo_pool[i] = 0;
        ammo_pool_type[i] = NULL;
    }
    
    gun_matrix = NULL;
    
    gun_mant_dspList = NULL;
    gun_dspList = NULL;
    
    sight_count = -1;
    gun_count = -1;
    sight = NULL;
    gun = NULL;
}

/*******************************************************************************
    function    :   firing_object::~firing_object
    arguments   :   <none>
    purpose     :   Deconstructor.
    notes       :   <none>
*******************************************************************************/
firing_object::~firing_object()
{
    // Deallocate any allocated memory
    if(gun_matrix)
    {
        delete *gun_matrix;
        delete gun_matrix;
    }
    if(gun_mant_dspList)
        delete gun_mant_dspList;
    if(gun_dspList)
        delete gun_dspList;
    if(sight)
        delete [] sight;
    if(gun)
        delete [] gun;
}

/*******************************************************************************
    function    :   firing_object::initWeapons
    arguments   :   <none>
    purpose     :   Initializes weapontry on firing object.
    notes       :   <none>
*******************************************************************************/
void firing_object::initWeapons()
{
    int i;
    char* temp;
    char buffer[128];
    void* temp_ptr;
    
    // Grab sight count
    temp = db.query(obj_model, "SIGHT_COUNT");
    if(temp)
    {
        sight_count = atoi(temp);
        
        if(sight_count <= 0 || sight_count > OBJ_SIGHT_MAX)
        {
            sprintf(buffer, "Unit: Invalid sight count defined for \"%s\".", obj_model);
            write_error(buffer);
            obj_status = OBJ_STATUS_REMOVE;
            return;
        }
    }
    else
    {
        sprintf(buffer, "Unit: Sight count not defined for \"%s\".", obj_model);
        write_error(buffer);
        obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // Grab gun count
    temp = db.query(obj_model, "GUN_COUNT");
    if(temp)
    {
        gun_count = atoi(temp);
        
        if(gun_count <= 0 || gun_count > OBJ_GUN_MAX)
        {
            sprintf(buffer, "Unit: Invalid gun count defined for \"%s\".", obj_model);
            write_error(buffer);
            obj_status = OBJ_STATUS_REMOVE;
            return;
        }
    }
    else
    {
        sprintf(buffer, "Unit: Gun count not defined for \"%s\".", obj_model);
        write_error(buffer);
        obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // Allocate memory for gun matrix
    gun_matrix = new GLfloat* [gun_count];
    temp_ptr = (void*)(new GLfloat [gun_count * 16]);
    for(i = 0; i < gun_count; i++)
        gun_matrix[i] = (GLfloat*)temp_ptr + (i * 16);
    
    // Allocate memory for display lists
    gun_mant_dspList = new GLuint[gun_count];
    gun_dspList = new GLuint[gun_count];
    
    // Allocate memory for sight and gun devices
    sight = new sight_device[sight_count];
    gun = new gun_device[gun_count];
    
    // Initialize sight devices
    for(i = 0; i < sight_count; i++)
        sight[i].initSight((object*)this, i);
    
    // Initialize gun devices
    for(i = 0; i < gun_count; i++)
        gun[i].initGun((object*)this, i);
}

/*******************************************************************************
    function    :   firing_object::initAmmo
    arguments   :   ammoLoadStr - String that defines ammo pool load out
    purpose     :   Assigns the ammo pool based on passed string.
    notes       :   1) Pass NULL as load string for standard load out values.
                    2) ammoLoadStr better not be anything important, cause
                       this function is gonna be destructive to it.
*******************************************************************************/
void firing_object::initAmmo(char* ammoLoadStr)
{
    int i;
    char element[64];
    char* token;
    int ammo_max;
    
    // If no string is passed, assumption is that to use standard load out.
    if(ammoLoadStr == NULL || ammoLoadStr[0] == '\0' ||
       strcmp(ammoLoadStr, "STD") == 0)
    {
        // Easy enough - DB query the standard load out
        for(i = 0; i < OBJ_MAX_AMMOPOOL; i++)
        {
            sprintf(element, "AMMO_STDLOAD_A%i", i + 1);
            token = db.query(obj_model, element);
            if(token)
                ammo_pool[i] = abs((short)atoi(token));
        }
        return;
    }
    
    // If a string is passed, then we have to parse it and figure out what
    // ammo load out we're getting at.
    i = 0;
    token = strtok(ammoLoadStr, " ,");           // Tokenize that bad boy
    while(i < OBJ_MAX_AMMOPOOL && token != NULL)
    {
        // Standard load out comes in as "STD"
        if(strcmp(token, "STD") == 0)
        {
            // DB query for standard load out
            sprintf(element, "AMMO_STDLOAD_A%i", i + 1);
            token = db.query(obj_model, element);
            if(token)
                ammo_pool[i] = abs((short)atoi(token));
        }
        else
            ammo_pool[i] = abs((short)atoi(token));     // Assign load
        
        // Next token
        i++;
        token = strtok(NULL, " ,");
    }
    
    // For any remaining pools, DB query for a standard load out
    for(i = i; i < OBJ_MAX_AMMOPOOL; i++)
    {
        sprintf(element, "AMMO_STDLOAD_A%i", i + 1);
        token = db.query(obj_model, element);
        if(token)
            ammo_pool[i] = abs((short)atoi(token));
    }
    
    // Check ammo shell loadout to not be over the maximum allowable.
    token = db.query(obj_model, "AMMO_MAX_SHELL_LOAD");
    if(token)
    {
        ammo_max = atoi(token);
        
        // Decrement ammo max over each pool
        for(i = 0; i < OBJ_MAX_AMMOPOOL - 1; i++)
        {
            // Check for too much ammo
            if(ammo_max <= 0)
                ammo_pool[i] = 0;
            if(ammo_max - ammo_pool[i] < 0)
            {
                ammo_pool[i] -= (ammo_pool[i] - ammo_max);
            }
            
            ammo_max -= ammo_pool[i];   // Decrement counter
        }
    }
    
    // Check ammo MG loadout to not be over the maximum allowable.
    token = db.query(obj_model, "AMMO_MAX_MG_LOAD");
    if(token)
    {
        ammo_max = atoi(token);
        
        // Check ammo over MG pool only
        if(ammo_pool[OBJ_MAX_AMMOPOOL - 1] > ammo_max)
            ammo_pool[OBJ_MAX_AMMOPOOL - 1] = ammo_max;
    }
    
    // Grab EXT_AMMO attachment
    token = db.query(obj_model, "EXT_AMMO_ATTACH");
    if(token)
    {
        if(strstr(token, "HULL"))
            ext_ammo_attach = OBJ_ATTACH_HULL;
        else if(strstr(token, "TURRET"))
        {
            sscanf(token, "TURRET%i", &ext_ammo_attach);
            ext_ammo_attach += OBJ_ATTACH_TURRET_OFF - 1;    // 1-1? ;)
        }
        else
        {
            char buffer[128];
            sprintf(buffer, "Unit: Invalid attachment level \"%s\" for \"%s\".",
                token, obj_model);
            write_error(buffer);
            obj_status = OBJ_STATUS_REMOVE;
            return;
        }
    }
    else
        ext_ammo_attach = OBJ_ATTACH_HULL;
}

/*******************************************************************************
    function    :   firing_object::fireGun
    arguments   :   gunNum - gun number
                    fromAmmoPool - ammo pool number to fire from
    purpose     :   Fires the weapon system given using the given ammo type.
    notes       :   <none>
*******************************************************************************/
object* firing_object::fireGun(int gunNum, int fromAmmoPool)
{
    kVector proj_pos;
    kVector proj_dir;
    proj_object* proj_ptr;
    int snd_id;
    char buffer[128];
    
    if(gunNum < 0 || gunNum >= gun_count || fromAmmoPool < 0 ||
       fromAmmoPool >= OBJ_MAX_AMMOPOOL)
    {
        // Check for valid passed argument
        sprintf(buffer, "Unit: Out of bounds: gun %i using ammo from pool %i.",
            gunNum + 1,
            fromAmmoPool + 1);
        write_error(buffer);
        return NULL;
    }
    
    if(ammo_pool[fromAmmoPool] < 0 || gun[gunNum].isOutOfAmmo())
    {
        // Check for valid passed argument
        sprintf(buffer,
            "Unit: Gun %i is out of ammo from pool %i yet was told to fire.",
            gunNum + 1,
            fromAmmoPool + 1);
        write_error(buffer);
        return NULL;
    }
    
    // Add new projectile object
    proj_ptr = (proj_object*)objects.addObject(OBJ_TYPE_PROJECTILE);
    
    // Check for allocation
    if(proj_ptr == NULL)
    {
        write_error("Unit: Unable to allocate a new projectile.");
        return NULL;
    }
    
    // Instruct gun to recoil
    gun[gunNum].recoilGun();
    
    // Determine position and direction vector for projectile
    proj_pos = kVector(0.0, 0.0,
        gun[gunNum].getGunLength());
    proj_dir = kVector(0.0, 0.0,
        gun[gunNum].getGunLength() + 1.0);
    proj_pos.transform((float*)gun_matrix[gunNum]);
    proj_dir.transform((float*)gun_matrix[gunNum]);
    proj_dir = proj_dir - proj_pos;
    
    // Initialize projectile
    proj_ptr->initProj((object*)this, ammo_pool_type[fromAmmoPool],
        proj_pos, proj_dir);
    
    // Add firing specular effect
    if(gun[gunNum].isMainGun())
    {
        // Add firing specular effects for a main gun
        effects.addEffect(SE_CANNON_FIRING, proj_pos, proj_dir,
            proj_ptr->diameter *
                (gun[gunNum].isFlashSupressed() ? 0.25f : 1.0f),
            (obj_type == OBJ_TYPE_TANK || obj_type == OBJ_TYPE_VEHICLE) ?
                vectorIn(dir, CS_CARTESIAN) * (dynamic_cast<moving_object*>
                (this))->linear_vel : kVector(0.0, 0.0, 0.0));
        snd_id = sounds.addSound(SOUND_CANNON, SOUND_HIGH_PRIORITY,
            proj_pos(), SOUND_PLAY_ONCE);
        sounds.setSoundRolloff(snd_id, 0.01f);
        sounds.auxModOnCaliber(snd_id, proj_ptr->diameter);
    }
    else
    {
        // Add firing specular effects for a machine gun
        effects.addEffect(SE_MG_FIRING, proj_pos, proj_dir,
            (obj_type == OBJ_TYPE_TANK || obj_type == OBJ_TYPE_VEHICLE) ?
                vectorIn(dir, CS_CARTESIAN) * (dynamic_cast<moving_object*>
                (this))->linear_vel : kVector(0.0, 0.0, 0.0));
        snd_id = sounds.addSound(SOUND_MG, SOUND_MID_PRIORITY,
            proj_pos(), SOUND_PLAY_ONCE);
        sounds.setSoundRolloff(snd_id, 0.02f);
    }
    
    return proj_ptr;
}

/*******************************************************************************
    function    :   firing_object::updateWeapons
    arguments   :   deltaT - seconds elapsed since last update
    purpose     :   Updates weapons.
    notes       :   <none>
*******************************************************************************/
void firing_object::updateWeapons(float deltaT)
{
    int i;
    
    // Update sighting mechanisms
    for(i = 0; i < sight_count; i++)
        sight[i].update(deltaT);
    
    // Update gun mechanisms
    for(i = 0; i < gun_count; i++)
        gun[i].update(deltaT);
}

/*******************************************************************************
    turreted_object
*******************************************************************************/

/*******************************************************************************
    function    :   turreted_object::turreted_object
    arguments   :   <none>
    purpose     :   Constructor.
    notes       :   <none>
*******************************************************************************/
turreted_object::turreted_object()
{
    // Initialize variables
    turret_count = 0;
    turret_rotation = NULL;
    turret_pivot = NULL;
    turret_matrix = NULL;
    turret_dspList = NULL;
}

/*******************************************************************************
    function    :   turreted_object::~turreted_object
    arguments   :   <none>
    purpose     :   Deconstructor.
    notes       :   <none>
*******************************************************************************/
turreted_object::~turreted_object()
{
    // Deallocate any allocated memory
    if(turret_rotation)
        delete turret_rotation;
    if(turret_pivot)
        delete [] turret_pivot;
    if(turret_matrix)
    {
        delete *turret_matrix;
        delete turret_matrix;
    }
    if(turret_dspList)
        delete turret_dspList;
}

/*******************************************************************************
    function    :   turreted_object::initTurrets
    arguments   :   <none>
    purpose     :   Initializes turrets on turreted object.
    notes       :   <none>
*******************************************************************************/
void turreted_object::initTurrets()
{
    int i;
    char* temp;
    char buffer[128];
    void* temp_ptr;
    
    // Grab turret count
    temp = db.query(obj_model, "TURRET_COUNT");
    if(temp)
        turret_count = atoi(temp);
    else
    {
        sprintf(buffer, "Unit: Turret count not defined for \"%s\".", obj_model);
        write_error(buffer);
        obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // If turret count is zero, then there are no turrets to keep track of.
    if(turret_count == 0)
        return;
    
    // Allocate memory for turret rotation
    turret_rotation = new float[turret_count];
    
    // Allocate memory for pivot points
    turret_pivot = new kVector[turret_count];
    
    // Allocate memory for turret matrix
    turret_matrix = new GLfloat* [turret_count];
    temp_ptr = (void*)(new GLfloat [turret_count * 16]);
    for(i = 0; i < turret_count; i++)
        turret_matrix[i] = (GLfloat*)temp_ptr + (i * 16);
    
    // Allocate memory for display lists
    turret_dspList = new GLuint[turret_count];
    
    // Grab turret pivot points
    for(i = 0; i < turret_count; i++)
    {
        sprintf(buffer, "TURRET%i_PIVOT", i+1);
        temp = db.query(obj_model, buffer);
        if(temp)
            sscanf(temp, "%f %f %f", &turret_pivot[i][0], &turret_pivot[i][1],
                &turret_pivot[i][2]);
        else
        {
            sprintf(buffer, "Unit: Turret %i pivot not defined for \"%s\".",
                i+1, obj_model);
            write_error(buffer);
            obj_status = OBJ_STATUS_REMOVE;
            return;
        }
    }
}

/*******************************************************************************
    function    :   turreted_object::updateTurrets
    arguments   :   deltaT - number of seconds elapsed since last update
    purpose     :   Updates turret rotation.
    notes       :   <none>
*******************************************************************************/
void turreted_object::updateTurrets(float deltaT)
{
    int i;
    
    // Do not update objects with no turrets
    if(turret_count == 0)
        return;
    
    // Update transverse of turret mechanism based on gun attachments.
    for(i = gun_count - 1; i >= 0; i--)
    {
        if(gun[i].getGunAttach() != OBJ_ATTACH_HULL)
            turret_rotation[gun[i].getGunAttach() - OBJ_ATTACH_TURRET_OFF] =
                gun[i].getTransverse();
    }
}
