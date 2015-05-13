/*******************************************************************************
                      Object Handler - Implementation
*******************************************************************************/
#include "main.h"
#include "objhandler.h"
#include "atg.h"
#include "camera.h"
#include "collision.h"
#include "console.h"
#include "database.h"
#include "metrics.h"
#include "misc.h"
#include "model.h"
#include "object.h"
#include "objunit.h"
#include "projectile.h"
#include "scenery.h"
#include "tank.h"

/*******************************************************************************
    function    :   object_handler::object_handler
    arguments   :   <none>
    purpose     :   Constructor.
    notes       :   <none>
*******************************************************************************/
object_handler::object_handler()
{
    int i, j;
    
    // Preset values to zero
    for(i = 0; i < 10; i++)
        obj_max[i] = 0;
    
    // PRESET VALUES
    obj_max[OBJ_TYPE_TANK] = OBJ_MAX_TANK;
    obj_max[OBJ_TYPE_VEHICLE] = OBJ_MAX_VEHICLE;
    obj_max[OBJ_TYPE_ATG] = OBJ_MAX_ATG;
    obj_max[OBJ_TYPE_ATR] = OBJ_MAX_ATR;
    obj_max[OBJ_TYPE_STATIC] = OBJ_MAX_STATIC;
    obj_max[OBJ_TYPE_PROJECTILE] = OBJ_MAX_PROJECTILE;
    
    // Allocate lists
    for(i = 0; i < 10; i++)
    {
        if(obj_max[i] > 0)
        {
            objects[i] = new object* [obj_max[i]];
            for(j = 0; j < obj_max[i]; j++)
                objects[i][j] = NULL;
                
            obj_count[i] = 0;
        }
        else
        {
            objects[i] = NULL;
            obj_count[i] = -1;
        }
    }
}

/*******************************************************************************
    function    :   object_handler::~object_handler
    arguments   :   <none>
    purpose     :   Deconstructor.
    notes       :   <none>
*******************************************************************************/
object_handler::~object_handler()
{
    int i, j, k;
    
    // Deallocate all lists
    for(i = 0; i < 10; i++)
        if(objects[i] != NULL)
        {
            for(j = k = 0; k < obj_count[i]; j++)
            {
                if(objects[i][j])
                {
                    delete objects[i][j];
                    k++;
                }
            }
            
            delete objects[i];
        }
}

/*******************************************************************************
    Base Routines
*******************************************************************************/

/*******************************************************************************
    function    :   object_handler::loadMission
    arguments   :   directory - Mission directory folder
    purpose     :   Loads up the mission.objects file from mission folder.
    notes       :   <none>
*******************************************************************************/
void object_handler::loadMission(char* directory)
{
    char buffer[128];
    char file[128];
    
    char element[32];
    char value[128];
    
    ifstream fin;
    
    char obj_id[32] = {'\0'};
    char obj_model[32] = {'\0'};
    char obj_attributes[128] = {'\0'};
    char organization[32] = {'\0'};
    char ammo_loadout[64] = {'\0'};
    float start_x, start_z;
    float start_heading;
    
    bool obj_id_specified = false;
    bool obj_model_specified = false;
    bool obj_position_specified = false;
    bool obj_heading_specified = false;
    bool obj_attributes_specified = false;
    bool organization_specified = false;
    bool ammo_loadout_specified = false;
    
    object* obj_ptr = NULL;
    
    // Construct file name for mission data
    strcpy(file, directory);
    strcat(file, "/mission.objects");
    
    // Open missions.object file
    fin.open(file);
    
    // Check for open
    if(!fin)
    {
        // If error, write error message and exit
        sprintf(buffer, "Obj: FATAL: Failure loading \"%s\" for read.",
            file);
        write_error(buffer);
        exit(1);
    }
    
    // Set model library to load models on the fly if they are not already
    // loaded into memory (required for initObj).
    models.setLoadOTF(true);
    
    while(1)
    {
        eatjunk(fin);
        if(!fin.eof())
        {
            fin.getline(buffer, 120);
            
            if(buffer[0] == '#')
                continue;
        }
        
        if(fin.eof() || (buffer[0] == '[' && obj_id_specified))
        {
            if(obj_id_specified && obj_model_specified &&
               obj_position_specified && obj_heading_specified)
            {
                obj_ptr = addObject(objType(db.query(obj_model, "DESIGNATION")));
                
                // Make sure object addition was valid before proceeding
                if(obj_ptr != NULL)
                {
                    // Initialize object base
                    if(obj_attributes_specified)
                        obj_ptr->initObj(obj_model, obj_attributes, obj_attributes);
                    else
                        obj_ptr->initObj(obj_model, (char*)NULL, (char*)NULL);
                    
                    // Initialize object orientation
                    obj_ptr->initObj(start_x, start_z, start_heading);
                    
                    // Special case for unit based objects
                    if(obj_ptr->obj_type == OBJ_TYPE_TANK ||
                       obj_ptr->obj_type == OBJ_TYPE_VEHICLE ||
                       obj_ptr->obj_type == OBJ_TYPE_ATG ||
                       obj_ptr->obj_type == OBJ_TYPE_ATR)
                    {
                        // Initialize Unit Properties
                        (dynamic_cast<unit_object*>(obj_ptr))->initUnit(obj_id, organization);
                        
                        // Initialize firing_object Ammo Pool
                        if(ammo_loadout_specified)
                            (dynamic_cast<firing_object*>(obj_ptr))->initAmmo(ammo_loadout);
                        else
                            (dynamic_cast<firing_object*>(obj_ptr))->initAmmo(NULL);
                    }
                    
                    // Seperate call to initialize advanced objects
                    switch(obj_ptr->obj_type)
                    {
                        case OBJ_TYPE_TANK:
                            (dynamic_cast<tank_object*>(obj_ptr))->initTank();
                            break;
                        
                        case OBJ_TYPE_VEHICLE:
                            //(dynamic_cast<vehicle_object*>(obj_ptr))->initVehicle();
                            break;
                        
                        case OBJ_TYPE_ATG:
                            (dynamic_cast<atg_object*>(obj_ptr))->initATG();
                            break;
                        
                        case OBJ_TYPE_ATR:
                            //(dynamic_cast<atr_object*>(obj_ptr))->initATR();
                            break;
                        
                        default:
                            // Nothing else to do but this (actually, this was
                            // a bug I found so here is the fix).
                            models.buildMeshMinMaxValues(obj_ptr->model_id);
                            models.buildDistanceValues(obj_ptr->model_id);
                            break;
                    }
                }
                else
                {
                    sprintf(value, "Obj: Failed adding object: [%s]: \"%s\".",
                        obj_id, obj_model);
                    write_error(value);
                    
                    // Notice: NO CONTINUE - otherwise next object will not
                    // be added correctly.
                }
            }
            else
            {
                sprintf(value, "Obj: Object ignored: [%s]: \"%s\".",
                        obj_id, obj_model);
                write_error(value);
                
                // Notice: NO CONTINUE - otherwise next object will not
                // be added correctly.
            }
            
            // Reset options
            obj_id_specified = false;
            obj_model_specified = false;
            obj_position_specified = false;
            obj_heading_specified = false;
            obj_attributes_specified = false;
            organization_specified = false;
            ammo_loadout_specified = false;
        }
        
        if(fin.eof())
            break;
        
        if(buffer[0] == '[' && sscanf(buffer, "[%s]", value) == 1)
        {
            if(value[strlen(value) - 1] == ']')     // sscanf fix for ]
                value[strlen(value) - 1] = '\0';
            
            strncpy(obj_id, value, 31);
            obj_id_specified = true;
        }
        else if(sscanf(buffer, "%s = %s", element, value) == 2)
        {
            strcpy(value, strstr(strstr(buffer, "="), value));
            
            if(strcmp(element, "MODEL") == 0)
            {
                strncpy(obj_model, value, 31);
                obj_model_specified = true;
            }
            else if(strcmp(element, "POSITION") == 0)
            {
                if(sscanf(value, "%f %f", &start_x, &start_z) == 2)
                    obj_position_specified = true;
            }
            else if(strcmp(element, "HEADING") == 0)
            {
                if(sscanf(value, "%f", &start_heading) == 1)
                    obj_heading_specified = true;
            }
            else if(strcmp(element, "ATTRIBUTES") == 0)
            {
                strcpy(obj_attributes, value);
                obj_attributes_specified = true;
            }
            else if(strcmp(element, "ORGANIZE") == 0)
            {
                strcpy(organization, value);
                organization_specified = true;
            }
            else if(strcmp(element, "AMMOLOAD") == 0)
            {
                strncpy(ammo_loadout, value, 63);
                ammo_loadout_specified = true;
            }
        }
    }
    
    // Close mission.objects file
    fin.close();
}

/*******************************************************************************
    function    :   object* object_handler::addObject
    arguments   :   objType - type of object to add
    purpose     :   Adds an object into the object handler. Returns pointer
                    to newly created object.
    notes       :   If object isn't immediately initialized after creation,
                    the object is removed (e.g. obj_status is automatically
                    initialized to remove). Naturally, the code has until the
                    update call to initialize the object.
*******************************************************************************/
object* object_handler::addObject(int objType)
{
    int j;
    object* obj_ptr = NULL;
    
    // Check for valid object type
    if(objType < 0 || objType > 9)
        return NULL;
    
    // Add new object in appropriate object level
    if(objects[objType] && obj_count[objType] < obj_max[objType])
        for(j = 0; j < obj_max[objType]; j++)
            if(objects[objType][j] == NULL)
            {
                switch(objType)
                {
                    case OBJ_TYPE_TANK:
                        obj_ptr = (object*)(new tank_object);
                        break;
                    
                    case OBJ_TYPE_VEHICLE:
                        //obj_ptr = (object*)(new vehicle_object);
                        break;
                    
                    case OBJ_TYPE_ATG:
                        obj_ptr = (object*)(new atg_object);
                        break;
                    
                    case OBJ_TYPE_ATR:
                        //obj_ptr = (object*)(new atr_object);
                        break;
                    
                    case OBJ_TYPE_STATIC:
                        obj_ptr = new object;
                        break;
                    
                    case OBJ_TYPE_PROJECTILE:
                        obj_ptr = (object*)(new proj_object);
                        break;
                    
                    default:
                        return NULL;
                        break;
                }
                
                // Check for allocate
                if(obj_ptr == NULL)
                    return NULL;
                
                // Place object into list
                objects[objType][j] = obj_ptr;
                obj_count[objType]++;
                
                return obj_ptr;
            }
    
    // Otherwise return NULL (e.g. too many objects, no object level of the
    // type defined, etc.)
    return NULL;
}

/*******************************************************************************
    Object Grabbing Routines
*******************************************************************************/

/*******************************************************************************
    function    :   object_handler::getUnitAt
    arguments   :   x,y - screen coordinates to perform selection on
    purpose     :   Grabs a unit based on the given mouse coordinates on the
                    screen. Returns NULL if no object satisfies selection.
    notes       :   1) Relies on the OpenGL selection buffer.
                    2) Chooses object closest to view camera if multiple
                       objects result from the OpenGL selection buffer.
*******************************************************************************/
object* object_handler::getUnitAt(int x, int y)
{
    GLuint buffer[64];
    GLint viewport[4];
    int hits;
    
    int i, j, k;
    
    GLuint* parsingPtr;
    GLuint name_count;
    GLuint closest_dist = 0xFFFFFFFF;
    object* unit = NULL;
    
    /* Begin OpenGL Selection Mode */
    
    // Initialize a selection buffer
    glSelectBuffer(64, buffer);
    
    // Change rendering mode to the selection mode
    glRenderMode(GL_SELECT);
    
    // Save current modelview and projection matricies
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    
    // Set up new projection matrix for clipping
    glLoadIdentity();
    glGetIntegerv(GL_VIEWPORT, viewport);
    gluPickMatrix(x, viewport[3] - y, 5, 5, viewport);
    camera.orient(false);   // Do NOT update frustum culling
    
    // Initialize OpenGL name stack
    glInitNames();
    
    // Switch back to modelview for displaying
    glMatrixMode(GL_MODELVIEW);
    
    // Draw objects
    for(i = OBJ_TYPE_TANK; i <= OBJ_TYPE_ATR; i++)
        if(objects[i])
            for(j = k = 0; k < obj_count[i]; j++)
                if(objects[i][j] != NULL)
                {
                    (dynamic_cast<unit_object*>(objects[i][j]))->displayForSelection();
                    k++;
                }
    
    // Be sure to complete all rendering commands
    glFlush();
    
    // Restore projection and modelview matricies
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    // Return to normal rendering mode, returning the number of hits recorded
    hits = glRenderMode(GL_RENDER);
    
    /* OpenGL Selection Mode Now Finished */
    
    // Check for no hits - return NULL if so.
    if(hits <= 0)
        return NULL;
    
    // Start our parsing pointer at the top of the buffer
    parsingPtr = buffer;
    
    // Parse buffer
    for(; hits > 0; hits--)
    {
        name_count = *parsingPtr;   // Get name count for this hit record
        
        if(name_count == 0)
            parsingPtr += 3;        // Go to next record
        else
        {
            parsingPtr++;           // Go to min depth for hit
            
            // Check to see for better min depth
            if(*parsingPtr < closest_dist)
            {
                closest_dist = *parsingPtr;     // Copy over min distance
                parsingPtr += 2;                // Skip over min & max
                
                unit = (object*)(*parsingPtr);  // Assign name (a ptr) to unit
                
                parsingPtr += name_count;       // Skip over names
            }
            else
                parsingPtr += 2 + name_count;   // Skip over min, max, & names
        }
    }
    
    return unit;
}

/*******************************************************************************
    function    :   object_handler::getUnitNear
    arguments   :   pos - Position on map
                    distanceTolerance - Tolerance distance used in computation
    purpose     :   Returns the closest unit near the map position given that
                    satisfies the distance tolerance value given, otherwise
                    returns NULL.
    notes       :   <none>
*******************************************************************************/
object* object_handler::getUnitNear(kVector pos, float distanceTolerance)
{
    int i, j, k;
    float closest_dist = (distanceTolerance * distanceTolerance) - FP_ERROR;
    float curr_dist;
    object* unit = NULL;
    
    // Go through each unit level looking for an object
    for(i = OBJ_TYPE_TANK; i <= OBJ_TYPE_ATR; i++)
        if(objects[i])
            for(j = k = 0; k < obj_count[i]; j++)
                if(objects[i][j] != NULL)
                {
                    // Compute manhattan distance to this object
                    curr_dist = ((objects[i][j]->pos[2] - pos[2]) *
                                    (objects[i][j]->pos[2] - pos[2])) +
                                ((objects[i][j]->pos[1] - pos[1]) *
                                    (objects[i][j]->pos[1] - pos[1])) +
                                ((objects[i][j]->pos[0] - pos[0]) *
                                    (objects[i][j]->pos[0] - pos[0]));
                    
                    // See if it is better than what we currently have
                    if(curr_dist < closest_dist)
                    {
                        closest_dist = curr_dist;
                        unit = objects[i][j];
                    }
                    
                    k++;
                }
    
    return unit;
}

/*******************************************************************************
    function    :   object_handler::getUnitWithID
    arguments   :   idTag - id tag string
    purpose     :   Returns the unit with the specified id tag string, other-
                    wise returns NULL.
    notes       :   <none>
*******************************************************************************/
object* object_handler::getUnitWithID(char* idTag)
{
    int i, j, k;
    
    // Go through each unit level looking for the said object
    for(i = OBJ_TYPE_TANK; i <= OBJ_TYPE_ATR; i++)
        if(objects[i])
            for(j = k = 0; k < obj_count[i]; j++)
                if(objects[i][j] != NULL)
                {
                    // See if this is our unit
                    if(strcmp((dynamic_cast<unit_object*>(objects[i][j]))->obj_id, idTag) == 0)
                        return objects[i][j];
                    
                    k++;
                }
    
    // No unit found
    return NULL;
}

/*******************************************************************************
    function    :   object_handler::getUnitsAt
    arguments   :   x_min,y_min - upper left hand corner screen coordinates
                    x_max,y_max - lower right hand corner screen coordinates
    purpose     :   Grabs a list of units based on the given mouse coordinates
                    of a rectangular selection window on the screen. Always
                    will return a newly allocated object list, of which *may*
                    be empty.
    notes       :   1) Relies on the OpenGL selection buffer.
                    2) Returns a NEW allocation, which must be deleted later.
                    3) The object list contains ALL objects in the rectangular
                       selection window, regardless of their type or alignment.
                    4) The object list *may* be empty.
*******************************************************************************/
object_list* object_handler::getUnitsAt(int x_min, int y_min, int x_max, int y_max)
{
    GLuint buffer[64];
    GLint viewport[4];
    int hits;
    
    int i, j, k;
    
    GLuint* parsingPtr;
    GLuint name_count;
    object_list* list = new object_list;
    
    /* Begin OpenGL Selection Mode */
    
    // Initialize a selection buffer
    glSelectBuffer(64, buffer);
    
    // Change rendering mode to the selection mode
    glRenderMode(GL_SELECT);
    
    // Save current modelview and projection matricies
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    
    // Set up new projection matrix for clipping
    glLoadIdentity();
    glGetIntegerv(GL_VIEWPORT, viewport);
    gluPickMatrix((x_min + x_max) / 2,                  // Fun little trick heh
                  viewport[3] - ((y_min + y_max) / 2),
                  abs(x_max - x_min),
                  abs(y_max - y_min),
                  viewport);
    camera.orient(false);   // Do NOT update frustum culling
    
    // Initialize OpenGL name stack
    glInitNames();
    
    // Switch back to modelview for displaying
    glMatrixMode(GL_MODELVIEW);
    
    // Draw objects
    for(i = OBJ_TYPE_TANK; i <= OBJ_TYPE_ATR; i++)
        if(objects[i])
            for(j = k = 0; k < obj_count[i]; j++)
                if(objects[i][j] != NULL)
                {
                    (dynamic_cast<unit_object*>(objects[i][j]))->displayForSelection();
                    k++;
                }
    
    // Be sure to complete all rendering commands
    glFlush();
    
    // Restore projection and modelview matricies
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    // Return to normal rendering mode, returning the number of hits recorded
    hits = glRenderMode(GL_RENDER);
    
    /* OpenGL Selection Mode Now Finished */
    
    // Check for no hits - return if so.
    if(hits <= 0)
        return list;
    
    // Start our parsing pointer at the top of the buffer
    parsingPtr = buffer;
    
    // Parse buffer
    for(; hits > 0; hits--)
    {
        name_count = *parsingPtr;   // Get name count for this hit record
        
        if(name_count == 0)
            parsingPtr += 3;        // Go to next hit record
        else
        {
            parsingPtr += 3;        // Skip over count, min, & max
            
            for(; name_count > 0; name_count--)
            {
                list->add((object*)(*parsingPtr));   // Add names (ptrs) to list
                parsingPtr++;                       // Next element
            }
        }
    }
    
    return list;
}

/*******************************************************************************
    function    :   object_handler::getUnitNear
    arguments   :   pos - Position on map
                    distanceTolerance - Tolerance distance used in computation
    purpose     :   Returns an object list of the units near the map position
                    given that satisfies the distance tolerance value given.
    notes       :   1) Returns a NEW allocation, which must be deleted later.
                    2) The object list contains ALL objects in the specified
                       area, regardless of their type or alignment.
                    3) The object list *may* be empty.
*******************************************************************************/
object_list* object_handler::getUnitsNear(kVector pos, float distanceTolerance)
{
    int i, j, k;
    float tolerance = (distanceTolerance * distanceTolerance) - FP_ERROR;
    object_list* list = new object_list;
    
    // Go through each unit level looking for any objects that satisfy tolerance
    for(i = OBJ_TYPE_TANK; i <= OBJ_TYPE_ATR; i++)
        if(objects[i])
            for(j = k = 0; k < obj_count[i]; j++)
                if(objects[i][j] != NULL)
                {
                    // Compute manhattan distance to this object & check
                    if(((objects[i][j]->pos[2] - pos[2]) *
                            (objects[i][j]->pos[2] - pos[2])) +
                        ((objects[i][j]->pos[1] - pos[1]) *
                            (objects[i][j]->pos[1] - pos[1])) +
                        ((objects[i][j]->pos[0] - pos[0]) *
                            (objects[i][j]->pos[0] - pos[0])) <= tolerance)
                        list->fastAdd(objects[i][j]);
                    
                    k++;
                }
    
    return list;
}

/*******************************************************************************
    function    :   object_handler::getUnitsWithID
    arguments   :   idTag - id tag sub string (partial or full)
    purpose     :   Returns a list of units with a similiar id tag string (using
                    strstr). Always will return a list.
    notes       :   1) Returns a NEW allocation, which must be deleted later.
                    2) The object list contains ALL objects with the specified
                       id tag sub string, regardless of their type or alignment.
                    3) The object list *may* be empty.
*******************************************************************************/
object_list* object_handler::getUnitsWithID(char* idTag)
{
    int i, j, k;
    object_list* list = new object_list;
    
    // Go through each unit level looking for the said object
    for(i = OBJ_TYPE_TANK; i <= OBJ_TYPE_ATR; i++)
        if(objects[i])
            for(j = k = 0; k < obj_count[i]; j++)
                if(objects[i][j] != NULL)
                {
                    // See if this satisfies the id tag sub string
                    if(strstr((dynamic_cast<unit_object*>(objects[i][j]))->obj_id, idTag) != NULL)
                        list->fastAdd(objects[i][j]);
                    
                    k++;
                }
    
    return list;
}

/*******************************************************************************
    CD Routines
*******************************************************************************/

/*******************************************************************************
    function    :   cdtl_node* object_handler::createCDTL
    arguments   :   objPtr - pointer to object
                    angleTolerance - angle tolerance to work off of
                    AABBTolerance - tolerance of AABB to work off of
                    excludeObjPtr - Ptr to object to exclude (otherwise NULL)
    purpose     :   Sets up a collision detection test list for the said object
                    using the provided angle tolerance and AABB area.
    notes       :   Allocates new memory which must be deleted later using the
                    killCDTL function.
*******************************************************************************/
cdtl_node* object_handler::createCDTL(object* objPtr, float angleTolerance = 15.0f,
    float AABBTolerance = 20.0f, object* excludeObjPtr = NULL)
{
    kVector dir;
    cdtl_node* cdtl_head = NULL;
    cdtl_node* cdtl_tail = NULL;
    int list, obj, objcnt;
    float init_yaw;
    float x_diff, z_diff;
    
    angleTolerance *= degToRad * 0.5;
    
    // Go through our objects and figure out which ones are viable to test
    // against.
    for(list = 0; list < OBJ_TYPE_PROJECTILE; list++)
    {
        if(objects[list])
        {
            for(obj = objcnt = 0; objcnt < obj_count[list]; obj++)
            {
                if(objects[list][obj] != NULL)
                {
                    if(objects[list][obj] != excludeObjPtr)
                    {
                        // Gather data about this object (AABB first then angle)
                        dir = objects[list][obj]->pos - objPtr->pos;
                        x_diff = fabsf(dir[0]);
                        z_diff = fabsf(dir[2]);
                        dir.convertTo(CS_YAW_ONLY);
                        init_yaw = dir[2];
                        dir = dir - objPtr->dir.vectorIn(CS_YAW_ONLY);
                        
                        // Normalize direction
                        if(dir[2] < -PI)
                            dir[2] += TWOPI;
                        if(dir[2] > PI)
                            dir[2] -= TWOPI;
                        
                        // Check to see if we should add this to the CDTL
                        if(fabsf(dir[2]) <= angleTolerance ||
                           x_diff <= AABBTolerance || z_diff <= AABBTolerance)
                        {
                            if(cdtl_head == NULL)
                            {
                                cdtl_head = new cdtl_node;
                                cdtl_tail = cdtl_head;
                            }
                            else
                            {
                                cdtl_tail->next = new cdtl_node;
                                cdtl_tail = cdtl_tail->next;
                            }
                            
                            cdtl_tail->obj_ptr = objects[list][obj];
                            cdtl_tail->init_yaw = init_yaw;
                            cdtl_tail->next = NULL;
                        }
                    }
                    objcnt++;
                }
            }
        }
    }
    
    return cdtl_head;
}

/*******************************************************************************
    function    :   cdtl_node* object_handler::killCDTL
    arguments   :   objPtr - pointer to object
    purpose     :   Deallocates memory allocated through usage of the createCDTL
                    function.
    notes       :   <none>
*******************************************************************************/
void object_handler::killCDTL(object* objPtr)
{
    if(objPtr->obj_type == OBJ_TYPE_PROJECTILE)
    {
        cdtl_node* curr = ((proj_object*)objPtr)->cdtl_head;
        
        while(((proj_object*)objPtr)->cdtl_head != NULL)
        {
            ((proj_object*)objPtr)->cdtl_head = ((proj_object*)objPtr)->cdtl_head->next;
            delete curr;
            curr = ((proj_object*)objPtr)->cdtl_head;
        }
    }
}

/*******************************************************************************
    function    :   object_handler::cdObjPass
    arguments   :   startList - array index to start at
                    endList - array index to end at
    purpose     :   Passes all objects currently in system through CDR.
    notes       :   <none>
*******************************************************************************/
void object_handler::cdObjPass(int startList, int endList)
{
    int list1, list2, obj1, objcnt1, obj2, objcnt2;
    
    for(list1 = startList; list1 <= endList; list1++)
    {
      if(objects[list1])
      {
        for(obj1 = objcnt1 = 0; objcnt1 < obj_count[list1]; obj1++)
        {
          if(objects[list1][obj1] != NULL)
          {
            for(list2 = list1; list2 < OBJ_TYPE_PROJECTILE; list2++)
            {
              if(objects[list2])
              {
                if(list1 != list2)
                {
                  obj2 = 0;
                  objcnt2 = 0;
                }
                else
                {
                  obj2 = obj1 + 1;
                  objcnt2 = objcnt1 + 1;
                }
                for(; objcnt2 < obj_count[list2]; obj2++)
                {
                  if(objects[list2][obj2] != NULL)
                  {
                    if(cdr_checkCS(objects[list1][obj1], objects[list2][obj2]))
                    {
                      if(cdr_checkIABB(objects[list1][obj1], objects[list2][obj2]))
                          cdr.handle(objects[list1][obj1], objects[list2][obj2]);
                    }
                    objcnt2++;
                  }
                }
              }
            }
            objcnt1++;
          }
        }
      }
    }
}

/*******************************************************************************
    function    :   object_handler::cdProjPass
    arguments   :   <none>
    purpose     :   Passes all projectiles currently in system through CDR.
    notes       :   <none>
*******************************************************************************/
void object_handler::cdProjPass()
{
    int obj, objcnt;
    cdtl_node* prev;
    cdtl_node* curr;
    ac_node* ACN;
    kVector dir;
    bool remove_node;
    
    if(objects[OBJ_TYPE_PROJECTILE])
    {
        for(obj = objcnt = 0; objcnt < obj_count[OBJ_TYPE_PROJECTILE]; obj++)
        {
            if(objects[OBJ_TYPE_PROJECTILE][obj] != NULL)
            {
                if(((proj_object*)(objects[OBJ_TYPE_PROJECTILE][obj]))->cdtl_head != NULL)
                {
                    curr = ((proj_object*)(objects[OBJ_TYPE_PROJECTILE][obj]))->cdtl_head;
                    prev = NULL;
                    
                    while(curr)
                    {
                        remove_node = false;
                        
                        // Check against the collision sphere of the object
                        if(cdr_checkCS(objects[OBJ_TYPE_PROJECTILE][obj],
                                       (object*)curr->obj_ptr))
                        {
                            // Run a check meshes against this object and get
                            // an ACN back (if collision happens).
                            ACN = cdr.checkMeshes(
                                objects[OBJ_TYPE_PROJECTILE][obj], curr->obj_ptr,
                                CD_T_OFFSET, CD_EXCLUDE_NO_MESHES);
                            
                            // Check for collision
                            if(ACN != NULL)
                            {
                                // Kill testing list before adding in the ACN
                                // so that the ACN handler, if kicking in,
                                // can create a new list.
                                killCDTL(objects[OBJ_TYPE_PROJECTILE][obj]);
                                
                                // Add ACN into CDR engine
                                cdr.addACN(ACN);
                                
                                // Also break out of while loop since we're
                                // done testing for this object on this trip.
                                break;
                            }
                            else
                                // Remove this node (since it didn't hit)
                                remove_node = true;
                        }
                        else
                        {
                            // Take an angle measurement to help determine if
                            // we need to remove any objects from the CDTL.
                            dir = vectorIn(((object*)curr->obj_ptr)->pos -
                                objects[OBJ_TYPE_PROJECTILE][obj]->pos,
                                CS_YAW_ONLY);
                            dir[2] = dir[2] - curr->init_yaw;
                            
                            if(dir[2] < -PI)
                                dir[2] += TWOPI;
                            if(dir[2] > PI)
                                dir[2] -= TWOPI;
                        }
                        
                        // See if this node needs removed from the CDTL.
                        if(remove_node || fabsf(dir[2]) >= PIHALF)
                        {
                            if(prev == NULL)
                            {
                                ((proj_object*)(objects[OBJ_TYPE_PROJECTILE][obj]))->cdtl_head =
                                    ((proj_object*)(objects[OBJ_TYPE_PROJECTILE][obj]))->cdtl_head->next;
                                delete curr;
                                curr = ((proj_object*)(objects[OBJ_TYPE_PROJECTILE][obj]))->cdtl_head;
                                continue;
                            }
                            else
                            {
                                prev->next = curr->next;
                                delete curr;
                                curr = prev->next;
                                continue;
                            }
                        }
                        
                        prev = curr;
                        curr = curr->next;
                    }
                }
                objcnt++;
            }
        }
    }
}

/*******************************************************************************
    function    :   object_handler::update
    arguments   :   deltaT - number of seconds elapsed since last update
    purpose     :   Base object handler update.
    notes       :   <none>
*******************************************************************************/
void object_handler::update(float deltaT)
{
    int i, j, k;
    
    // Make update calls to objects.
    for(i = 0; i < 10; i++)
        if(objects[i])
            for(j = k = 0; k < obj_count[i]; j++)
                if(objects[i][j] != NULL)
                {
                    // Check for special removal tag (member is public).
                    if(objects[i][j]->obj_status == OBJ_STATUS_REMOVE)
                    {
                        //cout << SDL_GetTicks() << ": [" << (unsigned int)objects[i][j] << "]: Object handler CATCH removing." << endl;
                        switch(objects[i][j]->obj_type)
                        {
                            case OBJ_TYPE_TANK:
                                delete dynamic_cast<tank_object*>(objects[i][j]);
                                break;
                            
                            case OBJ_TYPE_VEHICLE:
                                //delete dynamic_cast<vehicle_object*>(objects[i][j]);
                                break;
                            
                            case OBJ_TYPE_ATG:
                                delete dynamic_cast<atg_object*>(objects[i][j]);
                                break;
                            
                            case OBJ_TYPE_ATR:
                                //delete dynamic_cast<atr_object*>(objects[i][j]);
                                break;
                            
                            case OBJ_TYPE_STATIC:
                                delete objects[i][j];
                                break;
                            
                            case OBJ_TYPE_PROJECTILE:
                                // Case: Make sure projectile object is out
                                // of cdr subsystem entirely before deleting.
                                if(((proj_object*)objects[i][j])->cdr_passes <= 0)
                                    delete ((proj_object*)objects[i][j]);
                                else
                                {
                                    // Object still in CD system
                                    //cout << SDL_GetTicks() << ": [" << (unsigned int)objects[i][j] << "]: Object still in CDR, NOT removing." << endl;
                                    //console.addComMessage("Catch");
                                    k++;
                                    continue;
                                }
                                break;
                        }
                        objects[i][j] = NULL;
                        obj_count[i]--;
                    }
                    else
                    {
                        switch(objects[i][j]->obj_type)
                        {
                            case OBJ_TYPE_TANK:
                                (dynamic_cast<tank_object*>(objects[i][j]))->update(deltaT);
                                break;
                            
                            case OBJ_TYPE_VEHICLE:
                                //(dynamic_cast<vehicle_object*>(objects[i][j]))->update(deltaT);
                                break;
                            
                            case OBJ_TYPE_ATG:
                                (dynamic_cast<atg_object*>(objects[i][j]))->update(deltaT);
                                break;
                            
                            case OBJ_TYPE_ATR:
                                //(dynamic_cast<atr_object*>(objects[i][j]))->update(deltaT);
                                break;
                            
                            case OBJ_TYPE_STATIC:
                                objects[i][j]->update(deltaT);
                                break;
                            
                            case OBJ_TYPE_PROJECTILE:
                                ((proj_object*)objects[i][j])->update(deltaT);
                                break;
                        }
                        k++;
                    }
                }
}

/*******************************************************************************
    function    :   object_handler::displayFirstpass
    arguments   :   <none>
    purpose     :   Base object handler display.
    notes       :   <none>
*******************************************************************************/
void object_handler::displayFirstPass()
{
    int i, j, k;
    
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    // Make display calls to objects.
    for(i = 0; i < OBJ_TYPE_PROJECTILE; i++)
        if(objects[i])
            for(j = k = 0; k < obj_count[i]; j++)
                if(objects[i][j] != NULL)
                {
                    objects[i][j]->display();
                    k++;
                }
}

/*******************************************************************************
    function    :   object_handler::displaySecondPass
    arguments   :   <none>
    purpose     :   Base object handler display.
    notes       :   <none>
*******************************************************************************/
void object_handler::displaySecondPass()
{
    int j, k;
    
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    // Make display calls to objects.
    if(objects[OBJ_TYPE_PROJECTILE])
    {
        for(j = k = 0; k < obj_count[OBJ_TYPE_PROJECTILE]; j++)
            if(objects[OBJ_TYPE_PROJECTILE][j] != NULL)
            {
                ((proj_object*)objects[OBJ_TYPE_PROJECTILE][j])->display();
                k++;
            }
    }
}
