/*******************************************************************************
                         Base Object - Implementation
*******************************************************************************/
#include "main.h"
#include "object.h"
#include "atg.h"
#include "camera.h"
#include "database.h"
#include "metrics.h"
#include "misc.h"
#include "model.h"
#include "scenery.h"
#include "tank.h"

/*******************************************************************************
    Helper Functions
*******************************************************************************/

/*******************************************************************************
    function    :   objType
    arguments   :   designationStr - designation string
    purpose     :   Object helper function. Determines the type of object,
                    such as tank or anti-tank gun, based on a designation
                    such as "Tank" or "ATG".
    notes       :   <none>
*******************************************************************************/
unsigned short objType(char* designationStr)
{
    // Check valid string
    if(designationStr == NULL || designationStr[0] == '\0')
        return OBJ_TYPE_STATIC;
    
    // Determine object type based on designation
    if(strstr(designationStr, "Tank Gun") ||
       strstr(designationStr, "ATG"))
        return OBJ_TYPE_ATG;
    else if(strstr(designationStr, "Tank Rifle") ||
            strstr(designationStr, "ATR"))
        return OBJ_TYPE_ATR;
    else if(strstr(designationStr, "Vehicle") ||
            strstr(designationStr, "Car"))
        return OBJ_TYPE_VEHICLE;
    else if(strstr(designationStr, "Tank") ||
            strstr(designationStr, "Assault"))
        return OBJ_TYPE_TANK;
    
    return OBJ_TYPE_STATIC;
}

/*******************************************************************************
    function    :   objStatus
    arguments   :   statusStr - status string
    purpose     :   Object helper function. Determines and returns any modifiers
                    for the object based on the passed string.
    notes       :   <none>
*******************************************************************************/
unsigned short objStatus(char* statusStr)
{
    // Check for valid string
    if(statusStr == NULL || statusStr[0] == '\0')
        return OBJ_STATUS_OK;
    
    // Determine status based on string
    if(strstr(statusStr, "IMMOBILE"))
        return OBJ_STATUS_IMMOBILE;
    else if(strstr(statusStr, "ABANDONED"))
        return OBJ_STATUS_ABANDONED;
    else if(strstr(statusStr, "DESTROYED"))
        return OBJ_STATUS_DESTROYED;
    else if(strstr(statusStr, "KNOCKEDOUT"))
        return OBJ_STATUS_KNOCKEDOUT;
    else if(strstr(statusStr, "DEAD"))
        return OBJ_STATUS_DEAD;
    else if(strstr(statusStr, "RETREAT"))
        return OBJ_STATUS_RETREAT;
    
    return OBJ_STATUS_OK;
}

/*******************************************************************************
    function    :   objModifiers
    arguments   :   modifierStr - modifiers of object (based on string)
    purpose     :   Object helper function. Generates a modifier based integer
                    of which each bit corresponds to a high/low modifier.
    notes       :   <none>
*******************************************************************************/
unsigned short objModifiers(char* modifierStr)
{
    int modifiers = OBJ_MOD_NONE;
    
    // Check for valid string
    if(modifierStr == NULL || modifierStr[0] == '\0')
        return OBJ_MOD_NONE;
    
    // Determine modifiers based on string
    if(strstr(modifierStr, "AXIS") != NULL)
        modifiers = modifiers | OBJ_MOD_AXIS;
    if(strstr(modifierStr, "ALLIED") != NULL)
        modifiers = modifiers | OBJ_MOD_ALLIED;
    if(strstr(modifierStr, "GERMAN") != NULL)
    {
        modifiers = modifiers | OBJ_MOD_GERMAN;
        modifiers = modifiers | OBJ_MOD_AXIS;
    }
    if(strstr(modifierStr, "BELGIAN") != NULL)
    {
        modifiers = modifiers | OBJ_MOD_BELGIAN;
        modifiers = modifiers | OBJ_MOD_ALLIED;
    }
    if(strstr(modifierStr, "BRITISH") != NULL)
    {
        modifiers = modifiers | OBJ_MOD_BRITISH;
        modifiers = modifiers | OBJ_MOD_ALLIED;
    }
    if(strstr(modifierStr, "FRENCH") != NULL)
    {
        modifiers = modifiers | OBJ_MOD_FRENCH;
        modifiers = modifiers | OBJ_MOD_ALLIED;
    }
    if(strstr(modifierStr, "POLISH") != NULL)
    {
        modifiers = modifiers | OBJ_MOD_POLISH;
        modifiers = modifiers | OBJ_MOD_ALLIED;
    }
    if(strstr(modifierStr, "ATTACHED") != NULL)
        modifiers = modifiers | OBJ_MOD_ATTACHED;
    if(strstr(modifierStr, "MILITIA") != NULL)
        modifiers = modifiers | OBJ_MOD_MILITIA;
    if(strstr(modifierStr, "GREEN") != NULL)
        modifiers = modifiers | OBJ_MOD_GREEN;
    if(strstr(modifierStr, "REGULAR") != NULL)
        modifiers = modifiers | OBJ_MOD_REGULAR;
    if(strstr(modifierStr, "EXPERIENCED") != NULL)
        modifiers = modifiers | OBJ_MOD_EXPERIENCED;
    if(strstr(modifierStr, "VETERAN") != NULL)
        modifiers = modifiers | OBJ_MOD_VETERAN;
    if(strstr(modifierStr, "ELITE") != NULL)
        modifiers = modifiers | OBJ_MOD_ELITE;
    
    return modifiers;
}

/*******************************************************************************
    object
*******************************************************************************/

/*******************************************************************************
    function    :   object::object
    arguments   :   <none>
    purpose     :   Constructor.
    notes       :   <none>
*******************************************************************************/
object::object()
{
    // Initialize base attributes
    obj_model = NULL;
    obj_type = OBJ_TYPE_STATIC;
    obj_status = OBJ_STATUS_REMOVE;     // Set to remove unless initObj called
    obj_modifiers = OBJ_MOD_NONE;
    
    // Give 0.0 to orientation
    pos[0] = pos[1] = pos[2] = 0.0;
    dir[0] = dir[1] = dir[2] = 0.0;
    roll = 0.0;
    
    // No model library ID
    model_id = -1;
    
    // Give 1.0 to size
    size[0] = size[1] = size[2] = 1.0;
    
    // Load identity matrix for hull matrix
    hull_matrix[0] = hull_matrix[5] = hull_matrix[10] = hull_matrix[15] = 1.0;
    hull_matrix[1] = hull_matrix[2] = hull_matrix[3] = hull_matrix[4] = 
    hull_matrix[6] = hull_matrix[7] = hull_matrix[8] = hull_matrix[9] = 
    hull_matrix[11] = hull_matrix[12] = hull_matrix[13] = hull_matrix[14] = 0.0;
    
    // No draw, 1.0 culling radius
    draw = false;
    radius = 1.0;
}

/*******************************************************************************
    function    :   object::~object
    arguments   :   <none>
    purpose     :   Deconstructor.
    notes       :   <none>
*******************************************************************************/
object::~object()
{
    // Delete any allocated memory
    if(obj_model)
        delete obj_model;
}

/*******************************************************************************
    function    :   object::initObj
    arguments   :   modelName - name of DB table to associate object with
                    status - initial status of object
                    modifiers - modifiers (16-bit binary) for object
    purpose     :   Initialization function which initializes the object based
                    on the passed attributes. Initializes base attributes.
    notes       :   Does not initialize orientation of object.
*******************************************************************************/
void object::initObj(char* modelName, unsigned short status,
    unsigned short modifiers)
{
    float* min_size;
    float* max_size;
    
    // Copy over object specifics
    obj_model = strdup(modelName);
    obj_type = objType(db.query(obj_model, "DESIGNATION")); // Generate type
    obj_status = status;
    obj_modifiers = modifiers;
    
    // Determine the model library ID number
    model_id = models.getModelID(obj_model);
    
    // Determine culling/CD sphere radius based on model library
    radius = models.getRadius(model_id);
    
    // Determine size based on model library
    min_size = models.getMinSize(model_id);
    max_size = models.getMaxSize(model_id);
    size[0] = fabsf(max_size[0] - min_size[0]);
    size[1] = fabsf(max_size[1] - min_size[1]);
    size[2] = fabsf(max_size[2] - min_size[2]);
}

/*******************************************************************************
    function    :   object::initObj
    arguments   :   xPos - initial x position on map (longitude)
                    zPos - initial z position on map (latitude)
                    heading - initial heading (in degrees)
    purpose     :   Initialization function which initializes the object based
                    on the passed attributes. Initializes object orientation.
    notes       :   Does not initialize base attributes of object.
*******************************************************************************/
void object::initObj(float xPos, float zPos, float heading)
{
    kVector front_left(size[0]/2.0, 0.0, size[2]/2.0);
    kVector front_right(-size[0]/2.0, 0.0, size[2]/2.0);
    kVector rear_left(size[0]/2.0, 0.0, -size[2]/2.0);
    kVector rear_right(-size[0]/2.0, 0.0, -size[2]/2.0);
    char* temp;
    
    // Set up position vector
    pos[0] = xPos;
    pos[1] = map.getOverlayHeight(xPos, zPos) + 0.0247;
    pos[2] = zPos;
    
    // Set up temporary direction vector
    dir[0] = 1.0;
    dir[1] = PIHALF;
    dir[2] = (180.0 - heading) * degToRad;
    
    // Set up temporary roll value
    roll = 0.0;
    
    // Generate initial matrix for the hull_matrix (just position and yaw so
    // we can grab pitch and roll values).
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(pos[0], pos[1], pos[2]);                   // Position
    glRotatef(dir[2] * radToDeg, 0.0, 1.0, 0.0);            // Yaw
    glGetFloatv(GL_MODELVIEW_MATRIX, hull_matrix);
    glPopMatrix();
    
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
    
    // Determine whenever or not this is a special cased object
    temp = db.query(obj_model, "TAG");
    
    // Handle special cases
    if(temp && strcmp(temp, "BLDG") == 0)
    {
        // Special Case Buildings
        pos[1] = lowest(front_left[1], front_right[1], rear_left[1], rear_right[1]);
    }
    else
    {
        // Default Case
        // Pitch Calculation
        dir[1] = PIHALF - (((
            atan((front_left[1] - rear_left[1]) / size[2])
                ) + (
            atan((front_right[1] - rear_right[1]) / size[2])    
                )) / 2.0);
        
        // Roll Calculation
        roll = (atan((front_left[1] - front_right[1]) / size[0]) +
            atan((rear_left[1] - rear_right[1]) / size[0])) / 2.0;
    }
    
    // Finally initialize the orientation matrix with the correct values.
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(pos[0], pos[1], pos[2]);                   // Position
    glRotatef(dir[2] * radToDeg, 0.0, 1.0, 0.0);            // Yaw
    glRotatef((dir[1] * radToDeg) - 90.0, 1.0, 0.0, 0.0);   // Pitch
    glRotatef(roll * radToDeg, 0.0, 0.0, 1.0);              // Roll
    glGetFloatv(GL_MODELVIEW_MATRIX, hull_matrix);
    glPopMatrix();
}

/*******************************************************************************
    function    :   object::initObj
    arguments   :   position - initial position vector
                    direction - initial direction vector
                    roll - initial roll amount
    purpose     :   Initialization function which initializes the object based
                    on the passed attributes. Initializes object orientation.
    notes       :   Does not initialize base attributes of object.
*******************************************************************************/
void object::initObj(float* position, float* direction, float roll)
{
    // Assign postiion vector
    pos[0] = position[0];
    pos[1] = position[1];
    pos[2] = position[2];
    // Assign direction vector
    dir[0] = direction[0];
    dir[1] = direction[1];
    dir[2] = direction[2];
    // Assign roll
    object::roll = roll;
    
    // Generate matrix for the hull_matrix
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(pos[0], pos[1], pos[2]);                   // Position
    glRotatef(dir[2] * radToDeg, 0.0, 1.0, 0.0);            // Yaw
    glRotatef((dir[1] * radToDeg) - 90.0, 1.0, 0.0, 0.0);   // Pitch
    glRotatef(roll * radToDeg, 0.0, 0.0, 1.0);              // Roll
    glGetFloatv(GL_MODELVIEW_MATRIX, hull_matrix);
    glPopMatrix();
}

/*******************************************************************************
    function    :   object::transform
    arguments   :   position - coordinate to transform from a level to another
                    attachmentLevelFrom - conversion from this attachment level
                    attachmentLevelTo - conversion to this attachment level
    purpose     :   Function that perform transformations based on attachment
                    levels for 3D coordinate transforms.
    notes       :   <none>
*******************************************************************************/
kVector object::transform(kVector position, int attachLevelFrom, int attachLevelTo)
{
    switch(obj_type)
    {
        case OBJ_TYPE_TANK:
            return (dynamic_cast<tank_object*>(this))->transform(position, attachLevelFrom, attachLevelTo);
            break;
        
        case OBJ_TYPE_ATG:
            return (dynamic_cast<atg_object*>(this))->transform(position, attachLevelFrom, attachLevelTo);
            break;
        
        case OBJ_TYPE_VEHICLE:
            // Do later
        case OBJ_TYPE_INFANTRY:
            // Do later
        case OBJ_TYPE_ATR:
            // Do later
        case OBJ_TYPE_BUNKER:
            // Do later
        case OBJ_TYPE_SPECIAL:
            // Do later
        case OBJ_TYPE_STATIC:
        case OBJ_TYPE_PROJECTILE:
        default:
            return transformed(position, (float*)hull_matrix);
    }
}

/*******************************************************************************
    function    :   object::update
    arguments   :   deltaT - number of seconds elapsed since last update.
    purpose     :   Base object update function.
    notes       :   <none>
*******************************************************************************/
void object::update(float deltaT)
{
    // Camera culling
    draw = camera.sphereInView(pos(), radius);
}

/*******************************************************************************
    function    :   object::display
    arguments   :   <none>
    purpose     :   Base display function.
    notes       :   <none>
*******************************************************************************/
void object::display()
{
    // Draw object if in view.
    if(draw)
    {
        glPushMatrix();
        
        // Orient object
        glMultMatrixf(hull_matrix);
        
        // Draw model (from model library)
        models.drawModel(model_id, MDL_DRW_VERTEXARRAY);
        
        glPopMatrix();
    }
}
