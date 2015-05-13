/*******************************************************************************
                      ATG Object Model - Implementation
*******************************************************************************/
#include "main.h"
#include "atg.h"
#include "camera.h"
#include "console.h"
#include "database.h"
#include "effects.h"
#include "misc.h"
#include "metrics.h"
#include "model.h"
#include "object.h"
#include "objhandler.h"
#include "objmodules.h"
#include "projectile.h"
#include "scenery.h"
#include "sounds.h"

/*******************************************************************************
    function    :   atg_object::atg_object
    arguments   :   <none>
    purpose     :   Constructor
    notes       :   <none>
*******************************************************************************/
atg_object::atg_object()
{
    obj_type = OBJ_TYPE_ATG;
    
    // Initialize variables
    hull_yaw = 0.0;
    hull_yaw_override = false;
}

/*******************************************************************************
    function    :   atg_object::~atg_object
    arguments   :   <none>
    purpose     :   Deconstructor
    notes       :   <none>
*******************************************************************************/
atg_object::~atg_object()
{
    return;
}

/*******************************************************************************
    function    :   atg_object::build_atg
    arguments   :   <none>
    purpose     :   Builds the entire ATG object model.
    notes       :   <none>
*******************************************************************************/
void atg_object::build_atg()
{
    GLuint dspList;
    char buffer[128];
    int j = 0;
    char* temp;
    
    // Check to see if we have already built our tank
    if(db.query(obj_model, "BUILT") != NULL)
        return;     // If so we may exit
    
    // Build the display list for object selection
    dspList = glGenLists(1);
    glNewList(dspList, GL_COMPILE);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_COLOR_MATERIAL);
    glColor3f(1.0, 1.0, 1.0);
    
    glBegin(GL_LINES);
    {
        float border = 0.25;
        
        glVertex3f(size[0]/2.0 + 1.25*border, 0.15, size[2]/2.0 + border);
        glVertex3f(size[0]/2.0 - size[0]/4.0, 0.15, size[2]/2.0 + border);
        glVertex3f(size[0]/2.0 + 1.25*border, 0.15, size[2]/2.0 + border);
        glVertex3f(size[0]/2.0 + 1.25*border, 0.15, size[2]/2.0 - size[0]/4.0);
        
        glVertex3f(-size[0]/2.0 - 1.25*border, 0.15, size[2]/2.0 + border);
        glVertex3f(-size[0]/2.0 + size[0]/4.0, 0.15, size[2]/2.0 + border);
        glVertex3f(-size[0]/2.0 - 1.25*border, 0.15, size[2]/2.0 + border);
        glVertex3f(-size[0]/2.0 - 1.25*border, 0.15, size[2]/2.0 - size[0]/4.0);
        
        glVertex3f(-size[0]/2.0 - 1.25*border, 0.15, -size[2]/2.0 - border);
        glVertex3f(-size[0]/2.0 + size[0]/4.0, 0.15, -size[2]/2.0 - border);
        glVertex3f(-size[0]/2.0 - 1.25*border, 0.15, -size[2]/2.0 - border);
        glVertex3f(-size[0]/2.0 - 1.25*border, 0.15, -size[2]/2.0 + size[0]/4.0);
        
        glVertex3f(size[0]/2.0 + 1.25*border, 0.15, -size[2]/2.0 - border);
        glVertex3f(size[0]/2.0 - size[0]/4.0, 0.15, -size[2]/2.0 - border);
        glVertex3f(size[0]/2.0 + 1.25*border, 0.15, -size[2]/2.0 - border);
        glVertex3f(size[0]/2.0 + 1.25*border, 0.15, -size[2]/2.0 + size[0]/4.0);
    }
    glEnd();       
    
    glDisable(GL_COLOR_MATERIAL);
    glEndList();
    sprintf(buffer, "%i", (int)dspList);           // Store into DB
    db.insert(obj_model, "SELECTED_DSPLIST", buffer);
    
    // Build Hull
    dspList = glGenLists(1);
    glNewList(dspList, GL_COMPILE);
    models.drawMesh(model_id, "GUN_SHIELD", MDL_DRW_IMMEDIATE);
    models.drawMesh(model_id, "HULL", MDL_DRW_IMMEDIATE);
    models.drawMesh(model_id, "WHEEL_L1", MDL_DRW_IMMEDIATE);
    models.drawMesh(model_id, "WHEEL_R1", MDL_DRW_IMMEDIATE);
    // Draw misc attachments
    for(j = models.getMeshCount(model_id) - 1; j >= 0; j--)
        if(strstr(models.getMeshName(model_id, j), "MISC") != NULL)
        {
            sprintf(buffer, "%s_ATTACH", models.getMeshName(model_id, j));
            temp = db.query(obj_model, buffer);
            if(!temp || (temp && strcmp(temp, "HULL") == 0))
                models.drawMesh(model_id, j, MDL_DRW_IMMEDIATE);
        }
    glEndList();
    sprintf(buffer, "%i", (int)dspList);           // Store into DB
    db.insert(obj_model, "HULL_DSPLIST", buffer);
    
    // Build Gun Mantlet & Gun
    models.setMeshPolyOffset(model_id, "GUN_MANT1", gun[0].getGunPivot());
    models.setMeshPolyOffset(model_id, "GUN1", gun[0].getGunPivot());
    
    dspList = glGenLists(1);
    glNewList(dspList, GL_COMPILE);
    models.drawMesh(model_id, "GUN_MANT1", MDL_DRW_IMMEDIATE);
    // Draw misc attachments
    for(j = models.getMeshCount(model_id) - 1; j >= 0; j--)
        if(strstr(models.getMeshName(model_id, j), "MISC") != NULL)
        {
            sprintf(buffer, "%s_ATTACH", models.getMeshName(model_id, j));
            temp = db.query(obj_model, buffer);
            if(temp)
            {
                if(strcmp(temp, "GUN_MANT1") == 0)
                {
                    models.setMeshPolyOffset(model_id, j, gun[0].getGunPivot());
                    models.drawMesh(model_id, j, MDL_DRW_IMMEDIATE);
                }
            }
        }
    glEndList();
    sprintf(buffer, "%i", (int)dspList);       // Store into DB
    db.insert(obj_model, "GUN_MANT1_DSPLIST", buffer);
    
    dspList = glGenLists(1);
    glNewList(dspList, GL_COMPILE);
    models.drawMesh(model_id, "GUN1", MDL_DRW_IMMEDIATE);
    // Draw misc attachments
    for(j = models.getMeshCount(model_id) - 1; j >= 0; j--)
        if(strstr(models.getMeshName(model_id, j), "MISC") != NULL)
        {
            sprintf(buffer, "%s_ATTACH", models.getMeshName(model_id, j));
            temp = db.query(obj_model, buffer);
            if(temp)
            {
                if(strcmp(temp, "GUN1") == 0)
                {
                    models.setMeshPolyOffset(model_id, j, gun[0].getGunPivot());
                    models.drawMesh(model_id, j, MDL_DRW_IMMEDIATE);
                }
            }
        }
    glEndList();
    sprintf(buffer, "%i", (int)dspList);       // Store into DB
    db.insert(obj_model, "GUN1_DSPLIST", buffer);
    
    // Build values for model (library lib controlled)
    models.buildDistanceValues(model_id);
    models.buildMeshMinMaxValues(model_id);
    
    // Add identifier to DB
    db.insert(obj_model, "BUILT", "T");
}

/*******************************************************************************
    function    :   atg_object::initATG
    arguments   :   <none>
    purpose     :   Initialization function which initializes the ATG object.
    notes       :   <none>
*******************************************************************************/
void atg_object::initATG()
{
    char* temp;
    
    // Initialize modules
    initWeapons();
    initUnit();
    
    // Build our ATG object (3D display lists wise).
    build_atg();
    
    // Set our display lists
    selected_dspList = (temp = db.query(obj_model, "SELECTED_DSPLIST")) != NULL ?
        (GLuint)atol(temp) : DISPLAY_NULL;
    hull_dspList = (temp = db.query(obj_model, "HULL_DSPLIST")) != NULL ?
        (GLuint)atol(temp) : DISPLAY_NULL;
    gun_mant_dspList[0] = (temp = db.query(obj_model, "GUN_MANT1_DSPLIST")) != NULL ?
        (GLuint)atol(temp) : DISPLAY_NULL;
    gun_dspList[0] = (temp = db.query(obj_model, "GUN1_DSPLIST")) != NULL ?
        (GLuint)atol(temp) : DISPLAY_NULL;
}

/*******************************************************************************
    function    :   atg_object::transform
    arguments   :   position - coordinate to transform from a level to another
                    attachmentLevelFrom - conversion from this attachment level
                    attachmentLevelTo - conversion to this attachment level
    purpose     :   Function that perform transformations based on attachment
                    levels for 3D coordinate transforms.
    notes       :   <none>
*******************************************************************************/
kVector atg_object::transform(kVector position, int attachLevelFrom, int attachLevelTo)
{
    GLfloat matrix[16];
    
    if(attachLevelFrom == attachLevelTo)
        return position;
    
    switch(attachLevelTo)
    {
        case OBJ_ATTACH_WORLD:
            switch(attachLevelFrom)
            {
                case OBJ_ATTACH_HULL:
                    return transformed(position, (float*)hull_matrix);
                    break;
                
                case OBJ_ATTACH_GUNMNT1:
                case OBJ_ATTACH_GUNMNT2:
                case OBJ_ATTACH_GUNMNT3:
                case OBJ_ATTACH_GUNMNT4:
                case OBJ_ATTACH_GUNMNT5:
                    return transformed(position, (float*)gun_matrix[0]);
                    break;
                
                case OBJ_ATTACH_GUN1:
                case OBJ_ATTACH_GUN2:
                case OBJ_ATTACH_GUN3:
                case OBJ_ATTACH_GUN4:
                case OBJ_ATTACH_GUN5:
                    glMatrixMode(GL_MODELVIEW);
                    glPushMatrix();
                    glLoadMatrixf(gun_matrix[0]);
                    glTranslatef(0.0, 0.0, -gun[0].getGunRecoil());
                    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
                    glPopMatrix();
                    return transformed(position, (float*)matrix);
                    break;
                
                default:
                    return position;    // Default action - return position passed
                    break;
            }
            break;
        
        case OBJ_ATTACH_HULL:
            switch(attachLevelFrom)
            {
                case OBJ_ATTACH_GUNMNT1:
                case OBJ_ATTACH_GUNMNT2:
                case OBJ_ATTACH_GUNMNT3:
                case OBJ_ATTACH_GUNMNT4:
                case OBJ_ATTACH_GUNMNT5:
                    glMatrixMode(GL_MODELVIEW);
                    glPushMatrix();
                    glLoadIdentity();
                    // Gun is attached to main hull (enabled gun transverse)
                    glTranslatef(gun[0].getGunPivotV()[0],
                                 gun[0].getGunPivotV()[1],
                                 gun[0].getGunPivotV()[2]);
                    glRotatef(gun[0].getTransverse() * radToDeg, 0.0, 1.0, 0.0);
                    glRotatef((gun[0].getElevate() * radToDeg) - 90.0, 1.0, 0.0, 0.0);
                    glTranslatef(0.0, 0.0, 0.5 * -gun[0].getGunRecoil());  // Recoil gun mantlet
                    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
                    glPopMatrix();
                    return transformed(position, (float*)matrix);
                    break;
                
                case OBJ_ATTACH_GUN1:
                case OBJ_ATTACH_GUN2:
                case OBJ_ATTACH_GUN3:
                case OBJ_ATTACH_GUN4:
                case OBJ_ATTACH_GUN5:
                    glMatrixMode(GL_MODELVIEW);
                    glPushMatrix();
                    glLoadIdentity();
                    // Gun is attached to main hull (enabled gun transverse)
                    glTranslatef(gun[0].getGunPivotV()[0],
                                 gun[0].getGunPivotV()[1],
                                 gun[0].getGunPivotV()[2]);
                    glRotatef(gun[0].getTransverse() * radToDeg, 0.0, 1.0, 0.0);
                    glRotatef((gun[0].getElevate() * radToDeg) - 90.0, 1.0, 0.0, 0.0);
                    glTranslatef(0.0, 0.0, 0.5 * -gun[0].getGunRecoil());  // Recoil gun mantlet
                    glTranslatef(0.0, 0.0, -gun[0].getGunRecoil());     // Recoil gun
                    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
                    glPopMatrix();
                    return transformed(position, (float*)matrix);
                    break;
                
                default:
                    return position;    // Default action - return position passed
                    break;
            }
            break;
        
        case OBJ_ATTACH_GUNMNT1:
        case OBJ_ATTACH_GUNMNT2:
        case OBJ_ATTACH_GUNMNT3:
        case OBJ_ATTACH_GUNMNT4:
        case OBJ_ATTACH_GUNMNT5:
            switch(attachLevelFrom)
            {
                case OBJ_ATTACH_HULL:
                    glMatrixMode(GL_MODELVIEW);
                    glPushMatrix();
                    glLoadIdentity();
                    // Gun is attached to main hull (enabled gun transverse)
                    glTranslatef(0.0, 0.0, 0.5 * gun[0].getGunRecoil());  // Recoil gun mantlet
                    glRotatef(-((gun[0].getElevate() * radToDeg) - 90.0), 1.0, 0.0, 0.0);
                    glRotatef(-gun[0].getTransverse() * radToDeg, 0.0, 1.0, 0.0);
                    glTranslatef(-gun[0].getGunPivotV()[0],
                                 -gun[0].getGunPivotV()[1],
                                 -gun[0].getGunPivotV()[2]);
                    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
                    glPopMatrix();
                    return transformed(position, (float*)matrix);
                    break;
                
                case OBJ_ATTACH_GUNMNT1:
                case OBJ_ATTACH_GUNMNT2:
                case OBJ_ATTACH_GUNMNT3:
                case OBJ_ATTACH_GUNMNT4:
                case OBJ_ATTACH_GUNMNT5:
                case OBJ_ATTACH_GUN1:
                case OBJ_ATTACH_GUN2:
                case OBJ_ATTACH_GUN3:
                case OBJ_ATTACH_GUN4:
                case OBJ_ATTACH_GUN5:
                    return transform(
                        transform(position, attachLevelFrom, OBJ_ATTACH_HULL),
                            OBJ_ATTACH_HULL, attachLevelTo);
                    break;
                
                default:
                    return position;    // Default action - return position passed
                    break;
            }
            break;
        
        case OBJ_ATTACH_GUN1:
        case OBJ_ATTACH_GUN2:
        case OBJ_ATTACH_GUN3:
        case OBJ_ATTACH_GUN4:
        case OBJ_ATTACH_GUN5:
            switch(attachLevelFrom)
            {
                case OBJ_ATTACH_HULL:
                    glMatrixMode(GL_MODELVIEW);
                    glPushMatrix();
                    glLoadIdentity();
                    // Gun is attached to main hull (enabled gun transverse)
                    glTranslatef(0.0, 0.0, gun[0].getGunRecoil());
                    glTranslatef(0.0, 0.0, 0.5 * gun[0].getGunRecoil());  // Recoil gun mantlet
                    glRotatef(-((gun[0].getElevate() * radToDeg) - 90.0), 1.0, 0.0, 0.0);
                    glRotatef(-gun[0].getTransverse() * radToDeg, 0.0, 1.0, 0.0);
                    glTranslatef(-gun[0].getGunPivotV()[0],
                                 -gun[0].getGunPivotV()[1],
                                 -gun[0].getGunPivotV()[2]);
                    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
                    glPopMatrix();
                    return transformed(position, (float*)matrix);
                    break;
                
                case OBJ_ATTACH_GUNMNT1:
                case OBJ_ATTACH_GUNMNT2:
                case OBJ_ATTACH_GUNMNT3:
                case OBJ_ATTACH_GUNMNT4:
                case OBJ_ATTACH_GUNMNT5:
                case OBJ_ATTACH_GUN1:
                case OBJ_ATTACH_GUN2:
                case OBJ_ATTACH_GUN3:
                case OBJ_ATTACH_GUN4:
                case OBJ_ATTACH_GUN5:
                    return transform(
                        transform(position, attachLevelFrom, OBJ_ATTACH_HULL),
                            OBJ_ATTACH_HULL, attachLevelTo);
                    break;
                
                default:
                    return position;    // Default action - return position passed
                    break;
            }       
            break;
        
        default:
            return position;    // Default action - return position passed
            break;
    }
}

/*******************************************************************************
    function    :   atg_object::update
    arguments   :   deltaT - number of seconds elapsed since last update
    purpose     :   Updates the ATG object model.
    notes       :   <none>
*******************************************************************************/
void atg_object::update(float deltaT)
{
    kVector front_left(size[0]/2.0, 0.0, size[2]/2.0);
    kVector front_right(-size[0]/2.0, 0.0, size[2]/2.0);
    kVector rear_left(size[0]/2.0, 0.0, -size[2]/2.0);
    kVector rear_right(-size[0]/2.0, 0.0, -size[2]/2.0);
    float desired_pitch;
    float desired_roll;
    float desired_height;
    
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
        pos[1] += (desired_height - pos[1]) * 0.25 * (deltaT > 1.0 ?  1.0 : deltaT);
    }
    
    // Check to make sure we are not imbeded into the scenery too much
    if(pos[1] < desired_height - 1.25)
        pos[1] = desired_height - 1.25;
        
    if(dir[2] > TWOPI)
        dir[2] -= TWOPI;
    else if(dir[2] < 0.0)
        dir[2] += TWOPI;
    
    // Yaw update
    if(hull_yaw_override)
    {
        float yaw_offset;
        
        yaw_offset = hull_yaw - dir[2];
        
        if(yaw_offset < -PI)
            yaw_offset += TWOPI;
        else if(yaw_offset > PI)
            yaw_offset -= TWOPI;
        
        if(fabs(yaw_offset) <= 0.001)
        {
            dir[2] = hull_yaw;
            hull_yaw_override = false;
        }
        else if(yaw_offset > 0.0)
            dir[2] += 0.1 * deltaT;
        else
            dir[2] -= 0.1 * deltaT;
    }
    
    // Update unit modules
    updateUnit(deltaT);
    updateWeapons(deltaT);
    
    // Update Matricies
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    
    glLoadIdentity();
    
    // Update Hull Matrix
    glTranslatef(pos[0], pos[1], pos[2]);
    glRotatef(dir[2] * radToDeg, 0.0, 1.0, 0.0);
    glRotatef((dir[1] * radToDeg) - 90.0, 1.0, 0.0, 0.0);
    glRotatef(roll * radToDeg, 0.0, 0.0, 1.0);
    
    glGetFloatv(GL_MODELVIEW_MATRIX, hull_matrix);
    
    // Update Gun Matrix
    glPushMatrix();
    
    // Gun is attached to main hull (enabled gun transverse)
    glTranslatef(gun[0].getGunPivotV()[0],
                 gun[0].getGunPivotV()[1],
                 gun[0].getGunPivotV()[2]);
    glRotatef(gun[0].getTransverse() * radToDeg, 0.0, 1.0, 0.0);
    glRotatef((gun[0].getElevate() * radToDeg) - 90.0, 1.0, 0.0, 0.0);
    glTranslatef(0.0, 0.0, 0.5 * -gun[0].getGunRecoil());  // Recoil gun mantlet
    glGetFloatv(GL_MODELVIEW_MATRIX, gun_matrix[0]);
    
    glPopMatrix();
    
    glPopMatrix();
    
    // Camera culling
    draw = camera.sphereInView(pos(), radius);
}

/*******************************************************************************
    function    :   atg_object::display
    arguments   :   <none>
    purpose     :   Base display function which displays our tank object.
    notes       :   <none>
*******************************************************************************/
void atg_object::display()
{
    // If we are inside of drawing view (via frustum culling) then draw.
    // Otherwise we don't waste time sending the info down to the GPU.
    if(draw)
    {
        glPushMatrix();
        
        glMultMatrixf(hull_matrix);
        
        glCallList(hull_dspList);
        
        // If object is selected, display the "selected" visual
        if(selected)
            glCallList(selected_dspList);
        
        glPopMatrix();
        
        glPushMatrix();
        
        glMultMatrixf(gun_matrix[0]);
        
        glCallList(gun_mant_dspList[0]);
        
        glTranslatef(0.0, 0.0, -gun[0].getGunRecoil());        // Recoil gun
        glCallList(gun_dspList[0]);
        
        glPopMatrix();
    }
}
