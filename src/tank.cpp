/*******************************************************************************
                     Tank Object Model - Implementation
*******************************************************************************/
#include "main.h"
#include "tank.h"
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
    function    :   tank_object::tank_object
    arguments   :   <none>
    purpose     :   Constructor
    notes       :   <none>
*******************************************************************************/
tank_object::tank_object()
{
    // Set object type to tank (important!)
    obj_type = OBJ_TYPE_TANK;
    
    // Initialize variables
    angular_offset = 0.0;
    cupola_attach = OBJ_ATTACH_HULL;
    
    track_left_id = track_right_id = -1;
    track_left_s_texel = track_right_s_texel = 0.0;
}

/*******************************************************************************
    function    :   tank_object::~tank_object
    arguments   :   <none>
    purpose     :   Deconstructor
    notes       :   <none>
*******************************************************************************/
tank_object::~tank_object()
{
    return;
}

/*******************************************************************************
    function    :   tank_object::build_tank
    arguments   :   <none>
    purpose     :   Builds the entire tank object model.
    notes       :   <none>
*******************************************************************************/
void tank_object::build_tank()
{
    GLuint dspList;
    char element[32];
    char buffer[128];
    int i, j = 0;
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
    models.drawMesh(model_id, "FR_LW_HULL", MDL_DRW_IMMEDIATE);
    models.drawMesh(model_id, "LF_LW_HULL", MDL_DRW_IMMEDIATE);
    models.drawMesh(model_id, "RG_LW_HULL", MDL_DRW_IMMEDIATE);
    models.drawMesh(model_id, "RR_LW_HULL", MDL_DRW_IMMEDIATE);
    models.drawMesh(model_id, "FR_UP_HULL", MDL_DRW_IMMEDIATE);
    models.drawMesh(model_id, "LF_UP_HULL", MDL_DRW_IMMEDIATE);
    models.drawMesh(model_id, "RG_UP_HULL", MDL_DRW_IMMEDIATE);
    models.drawMesh(model_id, "RR_UP_HULL", MDL_DRW_IMMEDIATE);
    models.drawMesh(model_id, "TP_HULL", MDL_DRW_IMMEDIATE);
    models.drawMesh(model_id, "GLACIS", MDL_DRW_IMMEDIATE);
    models.drawMesh(model_id, "FR_HL_NOSE", MDL_DRW_IMMEDIATE);
    if(cupola_attach == OBJ_ATTACH_HULL)
        models.drawMesh(model_id, "CUPOLA", MDL_DRW_IMMEDIATE);
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
    
    // Build Turrets
    for(i = 1; i <= turret_count; i++)
    {
        float turret_pivot[3] = {0.0, 0.0, 0.0};
        
        sprintf(buffer, "TURRET%i_PIVOT", i);
        temp = db.query(obj_model, buffer);
        if(temp)
            sscanf(temp, "%f %f %f", &turret_pivot[0], &turret_pivot[1], &turret_pivot[2]);
        else
        {
            sprintf(buffer, "Tank: Turret %i pivot not defined for \"%s\".",
                i, obj_model);
            write_error(buffer);
            obj_status = OBJ_STATUS_REMOVE;
            return;
        }
        
        sprintf(buffer, "FR_TURRET%i", i);
        models.setMeshPolyOffset(model_id, buffer, turret_pivot);
        sprintf(buffer, "LF_TURRET%i", i);
        models.setMeshPolyOffset(model_id, buffer, turret_pivot);
        sprintf(buffer, "RG_TURRET%i", i);
        models.setMeshPolyOffset(model_id, buffer, turret_pivot);
        sprintf(buffer, "RR_TURRET%i", i);
        models.setMeshPolyOffset(model_id, buffer, turret_pivot);
        sprintf(buffer, "TP_TURRET%i", i);
        models.setMeshPolyOffset(model_id, buffer, turret_pivot);
        if(cupola_attach - OBJ_ATTACH_TURRET_OFF + 1 == i)
            models.setMeshPolyOffset(model_id, "CUPOLA", turret_pivot);
        if(ext_crew_attach - OBJ_ATTACH_TURRET_OFF + 1 == i)
            models.setMeshPolyOffset(model_id, "EXT_CREW", turret_pivot);
        if(ext_ammo_attach - OBJ_ATTACH_TURRET_OFF + 1 == i)
            models.setMeshPolyOffset(model_id, "EXT_AMMO", turret_pivot);
        
        sprintf(element, "TURRET%i_DSPLIST", i);
        dspList = glGenLists(1);
        glNewList(dspList, GL_COMPILE);
        sprintf(buffer, "FR_TURRET%i", i);
        models.drawMesh(model_id, buffer, MDL_DRW_IMMEDIATE);
        sprintf(buffer, "LF_TURRET%i", i);
        models.drawMesh(model_id, buffer, MDL_DRW_IMMEDIATE);
        sprintf(buffer, "RG_TURRET%i", i);
        models.drawMesh(model_id, buffer, MDL_DRW_IMMEDIATE);
        sprintf(buffer, "RR_TURRET%i", i);
        models.drawMesh(model_id, buffer, MDL_DRW_IMMEDIATE);
        sprintf(buffer, "TP_TURRET%i", i);
        models.drawMesh(model_id, buffer, MDL_DRW_IMMEDIATE);
        if(cupola_attach - OBJ_ATTACH_TURRET_OFF + 1 == i)
            models.drawMesh(model_id, "CUPOLA", MDL_DRW_IMMEDIATE);
        // Draw misc attachments
        for(j = models.getMeshCount(model_id) - 1; j >= 0; j--)
            if(strstr(models.getMeshName(model_id, j), "MISC") != NULL)
            {
                sprintf(buffer, "%s_ATTACH", models.getMeshName(model_id, j));
                temp = db.query(obj_model, buffer);
                if(temp)
                {
                    sprintf(buffer, "TURRET%i", i);
                    if(strcmp(temp, buffer) == 0)
                    {
                        models.setMeshPolyOffset(model_id, j, turret_pivot);
                        models.drawMesh(model_id, j, MDL_DRW_IMMEDIATE);
                    }
                }
            }
        glEndList();
        sprintf(buffer, "%i", (int)dspList);       // Store into DB
        db.insert(obj_model, element, buffer);
    }
    
    // Build Gun Mantlets & Guns
    for(i = 1; i <= gun_count; i++)
    {
        float gun_pivot[3] = {0.0, 0.0, 0.0};
        
        sprintf(buffer, "GUN%i_PIVOT", i);
        temp = db.query(obj_model, buffer);
        if(temp)
            sscanf(temp, "%f %f %f", &gun_pivot[0], &gun_pivot[1], &gun_pivot[2]);
        else
        {
            sprintf(buffer, "Tank: Gun %i pivot not defined for \"%s\".",
                i, obj_model);
            write_error(buffer);
            obj_status = OBJ_STATUS_REMOVE;
            return;
        }
        
        sprintf(buffer, "GUN_MANT%i", i);
        models.setMeshPolyOffset(model_id, buffer, gun_pivot);
        sprintf(buffer, "GUN%i", i);
        models.setMeshPolyOffset(model_id, buffer, gun_pivot);
        
        sprintf(element, "GUN_MANT%i_DSPLIST", i);
        dspList = glGenLists(1);
        glNewList(dspList, GL_COMPILE);
        sprintf(buffer, "GUN_MANT%i", i);
        models.drawMesh(model_id, buffer, MDL_DRW_IMMEDIATE);
        // Draw misc attachments
        for(j = models.getMeshCount(model_id) - 1; j >= 0; j--)
            if(strstr(models.getMeshName(model_id, j), "MISC") != NULL)
            {
                sprintf(buffer, "%s_ATTACH", models.getMeshName(model_id, j));
                temp = db.query(obj_model, buffer);
                if(temp)
                {
                    sprintf(buffer, "GUN_MANT%i", i);
                    if(strcmp(temp, buffer) == 0)
                    {
                        models.setMeshPolyOffset(model_id, j, gun_pivot);
                        models.drawMesh(model_id, j, MDL_DRW_IMMEDIATE);
                    }
                }
            }
        glEndList();
        sprintf(buffer, "%i", (int)dspList);       // Store into DB
        db.insert(obj_model, element, buffer);
        
        sprintf(element, "GUN%i_DSPLIST", i);
        dspList = glGenLists(1);
        glNewList(dspList, GL_COMPILE);
        sprintf(buffer, "GUN%i", i);
        models.drawMesh(model_id, buffer, MDL_DRW_IMMEDIATE);
        // Draw misc attachments
        for(j = models.getMeshCount(model_id) - 1; j >= 0; j--)
            if(strstr(models.getMeshName(model_id, j), "MISC") != NULL)
            {
                sprintf(buffer, "%s_ATTACH", models.getMeshName(model_id, j));
                temp = db.query(obj_model, buffer);
                if(temp)
                {
                    sprintf(buffer, "GUN%i", i);
                    if(strcmp(temp, buffer) == 0)
                    {
                        models.setMeshPolyOffset(model_id, j, gun_pivot);
                        models.drawMesh(model_id, j, MDL_DRW_IMMEDIATE);
                    }
                }
            }
        glEndList();
        sprintf(buffer, "%i", (int)dspList);       // Store into DB
        db.insert(obj_model, element, buffer);
    }
    
    // Build values for model (library lib controlled)
    models.buildDistanceValues(model_id);
    models.buildMeshMinMaxValues(model_id);
    
    // Add identifier to DB
    db.insert(obj_model, "BUILT", "T");
}

/*******************************************************************************
    function    :   tank_object::initTank
    arguments   :   <none>
    purpose     :   Initialization function which initializes the tank object.
    notes       :   <none>
*******************************************************************************/
void tank_object::initTank()
{
    int i;
    char* temp;
    char buffer[128];
    
    // Initialize modules
    initTurrets();
    initWeapons();
    initMovement();
    initUnit();
    
    // Grab modlib mesh IDs for tracks
    track_left_id = models.getMeshID(model_id, "TRACK_L");
    track_right_id = models.getMeshID(model_id, "TRACK_R");
    
    // Grab cupola attach
    temp = db.query(obj_model, "CUPOLA_ATTACH");
    if(temp)
    {
        if(strstr(temp, "HULL"))
            cupola_attach = OBJ_ATTACH_HULL;
        else if(strstr(temp, "TURRET"))
        {
            sscanf(temp, "TURRET%i", &cupola_attach);
            cupola_attach += OBJ_ATTACH_TURRET_OFF - 1;    // 1-1? ;)
        }
        else
        {
            sprintf(buffer, "Tank: Invalid cupola attachment level \"%s\".",
                temp);
            write_error(buffer);
            obj_status = OBJ_STATUS_REMOVE;
            return;
        }
    }
    else
    {
        sprintf(buffer, "Tank: Cupola attachment not defined for \"%s\".",
            obj_model);
        write_error(buffer);
        obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // Set turning offset (+b) for the turning circle value
    temp = db.query(obj_model, "TURNING_CIRCLE");
    if(temp)
        angular_offset = (3.333336 / atof(temp)) - 0.0194267;
    else
    {
        sprintf(buffer, "Tank: Turning circle not defined for \"%s\".",
            obj_model);
        write_error(buffer);
        obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // Build our tank object (3D display lists wise).
    build_tank();
    
    // Set our display lists
    selected_dspList = (temp = db.query(obj_model, "SELECTED_DSPLIST")) != NULL ?
        (GLuint)atol(temp) : DISPLAY_NULL;
    hull_dspList = (temp = db.query(obj_model, "HULL_DSPLIST")) != NULL ?
        (GLuint)atol(temp) : DISPLAY_NULL;
    for(i = 0; i < turret_count; i++)
    {
        sprintf(buffer, "TURRET%i_DSPLIST", i+1);
        turret_dspList[i] = (temp = db.query(obj_model, buffer)) != NULL ?
            (GLuint)atol(temp) : DISPLAY_NULL;
    }
    for(i = 0; i < gun_count; i++)
    {
        sprintf(buffer, "GUN_MANT%i_DSPLIST", i + 1);
        gun_mant_dspList[i] = (temp = db.query(obj_model, buffer)) != NULL ?
            (GLuint)atol(temp) : DISPLAY_NULL;
        sprintf(buffer, "GUN%i_DSPLIST", i + 1);
        gun_dspList[i] = (temp = db.query(obj_model, buffer)) != NULL ?
            (GLuint)atol(temp) : DISPLAY_NULL;
    }
}

/*******************************************************************************
    function    :   tank_object::transform
    arguments   :   position - coordinate to transform from a level to another
                    attachmentLevelFrom - conversion from this attachment level
                    attachmentLevelTo - conversion to this attachment level
    purpose     :   Function that perform transformations based on attachment
                    levels for 3D coordinate transforms.
    notes       :   <none>
*******************************************************************************/
kVector tank_object::transform(kVector position, int attachLevelFrom, int attachLevelTo)
{
    int i, j;
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
                
                case OBJ_ATTACH_TURRET1:
                case OBJ_ATTACH_TURRET2:
                case OBJ_ATTACH_TURRET3:
                    return transformed(position, (float*)turret_matrix[attachLevelFrom - OBJ_ATTACH_TURRET_OFF]);
                    break;
                
                case OBJ_ATTACH_GUNMNT1:
                case OBJ_ATTACH_GUNMNT2:
                case OBJ_ATTACH_GUNMNT3:
                case OBJ_ATTACH_GUNMNT4:
                case OBJ_ATTACH_GUNMNT5:
                    return transformed(position, (float*)gun_matrix[attachLevelFrom - OBJ_ATTACH_GUNMNT_OFF]);
                    break;
                
                case OBJ_ATTACH_GUN1:
                case OBJ_ATTACH_GUN2:
                case OBJ_ATTACH_GUN3:
                case OBJ_ATTACH_GUN4:
                case OBJ_ATTACH_GUN5:
                    i = attachLevelFrom - OBJ_ATTACH_GUN_OFF;
                    glMatrixMode(GL_MODELVIEW);
                    glPushMatrix();
                    glLoadMatrixf(gun_matrix[i]);
                    glTranslatef(0.0, 0.0, -gun[i].getGunRecoil());
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
                case OBJ_ATTACH_TURRET1:
                case OBJ_ATTACH_TURRET2:
                case OBJ_ATTACH_TURRET3:
                    i = attachLevelFrom - OBJ_ATTACH_TURRET_OFF;
                    glMatrixMode(GL_MODELVIEW);
                    glPushMatrix();
                    glLoadIdentity();
                    glTranslatef(turret_pivot[i][0], turret_pivot[i][1],
                         turret_pivot[i][2]);
                    glRotatef(turret_rotation[i] * radToDeg, 0.0, 1.0, 0.0);
                    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
                    glPopMatrix();
                    return transformed(position, (float*)matrix);
                    break;
                
                case OBJ_ATTACH_GUNMNT1:
                case OBJ_ATTACH_GUNMNT2:
                case OBJ_ATTACH_GUNMNT3:
                case OBJ_ATTACH_GUNMNT4:
                case OBJ_ATTACH_GUNMNT5:
                    i = attachLevelFrom - OBJ_ATTACH_GUNMNT_OFF;
                    glMatrixMode(GL_MODELVIEW);
                    glPushMatrix();
                    glLoadIdentity();
                    if(gun[i].getGunAttach() == OBJ_ATTACH_HULL)
                    {
                        // Gun is attached to main hull (enabled gun transverse)
                        glTranslatef(gun[i].getGunPivotV()[0],
                                     gun[i].getGunPivotV()[1],
                                     gun[i].getGunPivotV()[2]);
                        glRotatef(gun[i].getTransverse() * radToDeg, 0.0, 1.0, 0.0);
                    }
                    else
                    {
                        // Gun is attached to turret (disabled gun transverse)
                        glLoadMatrixf(turret_matrix[gun[i].getGunAttach() - OBJ_ATTACH_TURRET_OFF]);
                        glTranslatef(gun[i].getGunPivotV()[0],
                                     gun[i].getGunPivotV()[1],
                                     gun[i].getGunPivotV()[2]);
                    }
                    glRotatef((gun[i].getElevate() * radToDeg) - 90.0, 1.0, 0.0, 0.0);
                    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
                    glPopMatrix();
                    return transformed(position, (float*)matrix);
                    break;
                
                case OBJ_ATTACH_GUN1:
                case OBJ_ATTACH_GUN2:
                case OBJ_ATTACH_GUN3:
                case OBJ_ATTACH_GUN4:
                case OBJ_ATTACH_GUN5:
                    i = attachLevelFrom - OBJ_ATTACH_GUN_OFF;
                    glMatrixMode(GL_MODELVIEW);
                    glPushMatrix();
                    glLoadIdentity();
                    if(gun[i].getGunAttach() == OBJ_ATTACH_HULL)
                    {
                        // Gun is attached to main hull (enabled gun transverse)
                        glTranslatef(gun[i].getGunPivotV()[0],
                                     gun[i].getGunPivotV()[1],
                                     gun[i].getGunPivotV()[2]);
                        glRotatef(gun[i].getTransverse() * radToDeg, 0.0, 1.0, 0.0);
                    }
                    else
                    {
                        // Gun is attached to turret (disabled gun transverse)
                        glLoadMatrixf(turret_matrix[gun[i].getGunAttach() - OBJ_ATTACH_TURRET_OFF]);
                        glTranslatef(gun[i].getGunPivotV()[0],
                                     gun[i].getGunPivotV()[1],
                                     gun[i].getGunPivotV()[2]);
                    }
                    glRotatef((gun[i].getElevate() * radToDeg) - 90.0, 1.0, 0.0, 0.0);
                    glTranslatef(0.0, 0.0, -gun[i].getGunRecoil());     // Recoil gun
                    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
                    glPopMatrix();
                    return transformed(position, (float*)matrix);
                    break;
                
                default:
                    return position;    // Default action - return position passed
                    break;
            }
            break;
        
        case OBJ_ATTACH_TURRET1:
        case OBJ_ATTACH_TURRET2:
        case OBJ_ATTACH_TURRET3:
            i = attachLevelTo - OBJ_ATTACH_TURRET_OFF;
            switch(attachLevelFrom)
            {
                case OBJ_ATTACH_HULL:
                    glMatrixMode(GL_MODELVIEW);
                    glPushMatrix();
                    glLoadIdentity();
                    glRotatef(-turret_rotation[i] * radToDeg, 0.0, 1.0, 0.0);
                    glTranslatef(-turret_pivot[i][0], -turret_pivot[i][1],
                         -turret_pivot[i][2]);
                    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
                    glPopMatrix();
                    return transformed(position, (float*)matrix);
                    break;
                
                case OBJ_ATTACH_TURRET1:
                case OBJ_ATTACH_TURRET2:
                case OBJ_ATTACH_TURRET3:
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
        
        case OBJ_ATTACH_GUNMNT1:
        case OBJ_ATTACH_GUNMNT2:
        case OBJ_ATTACH_GUNMNT3:
        case OBJ_ATTACH_GUNMNT4:
        case OBJ_ATTACH_GUNMNT5:
            i = attachLevelTo - OBJ_ATTACH_GUNMNT_OFF;
            switch(attachLevelFrom)
            {
                case OBJ_ATTACH_HULL:
                    glMatrixMode(GL_MODELVIEW);
                    glPushMatrix();
                    glLoadIdentity();
                    glRotatef(-((gun[i].getElevate() * radToDeg) - 90.0), 1.0, 0.0, 0.0);
                    // Handle if gun is attached to turret
                    if(gun[i].getGunAttach() == OBJ_ATTACH_HULL)
                    {
                        // Gun is attached to main hull (enabled gun transverse)
                        glRotatef(-gun[i].getTransverse() * radToDeg, 0.0, 1.0, 0.0);
                        glTranslatef(-gun[i].getGunPivotV()[0],
                                     -gun[i].getGunPivotV()[1],
                                     -gun[i].getGunPivotV()[2]);
                    }
                    else
                    {
                        // Gun is attached to turret (disabled gun transverse)
                        glTranslatef(-gun[i].getGunPivotV()[0],
                                     -gun[i].getGunPivotV()[1],
                                     -gun[i].getGunPivotV()[2]);
                        j = gun[i].getGunAttach() - OBJ_ATTACH_TURRET_OFF;
                        glRotatef(-turret_rotation[j] * radToDeg, 0.0, 1.0, 0.0);
                        glTranslatef(-turret_pivot[j][0], -turret_pivot[j][1],
                            -turret_pivot[j][2]);
                    }
                    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
                    glPopMatrix();
                    return transformed(position, (float*)matrix);
                    break;
                
                case OBJ_ATTACH_TURRET1:
                case OBJ_ATTACH_TURRET2:
                case OBJ_ATTACH_TURRET3:
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
            i = attachLevelTo - OBJ_ATTACH_GUN_OFF;
            switch(attachLevelFrom)
            {
                case OBJ_ATTACH_HULL:
                    glMatrixMode(GL_MODELVIEW);
                    glPushMatrix();
                    glLoadIdentity();
                    glTranslatef(0.0, 0.0, gun[i].getGunRecoil());
                    glRotatef(-((gun[i].getElevate() * radToDeg) - 90.0), 1.0, 0.0, 0.0);
                    // Handle if gun is attached to turret
                    if(gun[i].getGunAttach() == OBJ_ATTACH_HULL)
                    {
                        // Gun is attached to main hull (enabled gun transverse)
                        glRotatef(-gun[i].getTransverse() * radToDeg, 0.0, 1.0, 0.0);
                        glTranslatef(-gun[i].getGunPivotV()[0],
                                     -gun[i].getGunPivotV()[1],
                                     -gun[i].getGunPivotV()[2]);
                    }
                    else
                    {
                        // Gun is attached to turret (disabled gun transverse)
                        glTranslatef(-gun[i].getGunPivotV()[0],
                                     -gun[i].getGunPivotV()[1],
                                     -gun[i].getGunPivotV()[2]);
                        j = gun[i].getGunAttach() - OBJ_ATTACH_TURRET_OFF;
                        glRotatef(-turret_rotation[j] * radToDeg, 0.0, 1.0, 0.0);
                        glTranslatef(-turret_pivot[j][0], -turret_pivot[j][1],
                            -turret_pivot[j][2]);
                    }
                    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
                    glPopMatrix();
                    return transformed(position, (float*)matrix);
                    break;
                
                case OBJ_ATTACH_TURRET1:
                case OBJ_ATTACH_TURRET2:
                case OBJ_ATTACH_TURRET3:
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
    function    :   tank_object::update
    arguments   :   deltaT - number of seconds elapsed since last update
    purpose     :   Updates the entire tank object model.
    notes       :   <none>
*******************************************************************************/
void tank_object::update(float deltaT)
{
    int i;
    
    // Recompute maximum angular velocity based on S-35 test data.
    max_angular_vel = 0.011656 * fabsf(linear_vel) + angular_offset;
    // Limit turning max speed for slower movement (clamping)
    if(max_angular_vel >= fabsf(linear_vel) * 0.274)
        max_angular_vel = fabsf(linear_vel) * 0.274;
    
    // Update unit modules
    updateUnit(deltaT);
    updateMovement(deltaT);
    updateWeapons(deltaT);
    updateTurrets(deltaT);
    
    // Update track UV offset (for track texture rotate)
    if(fabsf(linear_vel) > 0.0)
    {
        float turn_offset;
        
        if(angular_vel == 0.0)
        {
            // Not Turning - Full speed for both tracks.
            track_left_s_texel += TANK_TRACK_MOVEMENT * linear_vel * deltaT;
            track_right_s_texel += TANK_TRACK_MOVEMENT * linear_vel * deltaT;
        }
        else if((linear_vel > 0.0 && angular_vel > 0.0) ||
                (linear_vel < 0.0 && angular_vel < 0.0))
        {
            // Turning Left - Decrease speed of inner track, increase speed of
            // outer track. Tweaked values (not 100% accurate).
            turn_offset = 1.0 - (1.724 * fabsf(angular_vel));
            track_left_s_texel += TANK_TRACK_MOVEMENT * linear_vel *
                (turn_offset < 0.15 ? 0.15 : turn_offset) * deltaT;
            turn_offset = 1.0 + (0.427 * fabsf(angular_vel));
            track_right_s_texel += TANK_TRACK_MOVEMENT * linear_vel *
                (turn_offset > 2.0 ? 2.0 : turn_offset) * deltaT;
        }
        else if((linear_vel > 0.0 && angular_vel < 0.0) ||
                (linear_vel < 0.0 && angular_vel > 0.0))
        {
            // Turning Right - Decrease speed of inner track, increase speed of
            // outer track. Tweaked values (not 100% accurate).
            turn_offset = 1.0 - (1.724 * fabsf(angular_vel));
            track_right_s_texel += TANK_TRACK_MOVEMENT * linear_vel *
                (turn_offset < 0.15 ? 0.15 : turn_offset) * deltaT;
            turn_offset = 1.0 + (0.427 * fabsf(angular_vel));
            track_left_s_texel += TANK_TRACK_MOVEMENT * linear_vel *
                (turn_offset > 2.0 ? 2.0 : turn_offset) * deltaT;
        }
        
        // Normalize values to be between 0.0 and 1.0. These are offsets, so
        // values of 1.0 are normalized to 0.0 without any ill effect.
        track_left_s_texel -= floor(track_left_s_texel);
        track_right_s_texel -= floor(track_right_s_texel);
    }
    
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
    
    // Update Turret Matricies
    for(i = 0; i < turret_count; i++)
    {
        glPushMatrix();
        
        glTranslatef(turret_pivot[i][0],
                     turret_pivot[i][1],
                     turret_pivot[i][2]);
        glRotatef(turret_rotation[i] * radToDeg, 0.0, 1.0, 0.0);
        
        glGetFloatv(GL_MODELVIEW_MATRIX, turret_matrix[i]);
        
        glPopMatrix();
    }
    
    // Update Gun Matricies
    for(i = 0; i < gun_count; i++)
    {
        glPushMatrix();
        
        // Handle if gun is attached to turret
        if(gun[i].getGunAttach() == OBJ_ATTACH_HULL)
        {
            // Gun is attached to main hull (enabled gun transverse)
            glTranslatef(gun[i].getGunPivotV()[0],
                         gun[i].getGunPivotV()[1],
                         gun[i].getGunPivotV()[2]);
            glRotatef(gun[i].getTransverse() * radToDeg, 0.0, 1.0, 0.0);
        }
        else
        {
            // Gun is attached to turret (disabled gun transverse)
            glLoadMatrixf(turret_matrix[gun[i].getGunAttach() - OBJ_ATTACH_TURRET_OFF]);
            glTranslatef(gun[i].getGunPivotV()[0],
                         gun[i].getGunPivotV()[1],
                         gun[i].getGunPivotV()[2]);
        }
        
        glRotatef((gun[i].getElevate() * radToDeg) - 90.0, 1.0, 0.0, 0.0);
        
        glGetFloatv(GL_MODELVIEW_MATRIX, gun_matrix[i]);
        
        glPopMatrix();
    }
    
    glPopMatrix();      // Restore original matrix
    
    // Camera culling
    draw = camera.sphereInView(pos(), radius);
}

/*******************************************************************************
    function    :   tank_object::display
    arguments   :   <none>
    purpose     :   Base display function which displays our tank object.
    notes       :   <none>
*******************************************************************************/
void tank_object::display()
{
    int i;
    float texelOffset[2] = {0.0, 0.0};
    
    // If we are inside of drawing view (via frustum culling) then draw.
    // Otherwise we don't waste time sending the info down to the GPU.
    if(draw)
    {
        glPushMatrix();
        
        glMultMatrixf(hull_matrix);
        
        glCallList(hull_dspList);
        
        texelOffset[0] = track_left_s_texel;
        models.setMeshTexelOffset(model_id, track_left_id, texelOffset);
        texelOffset[0] = track_right_s_texel;
        models.setMeshTexelOffset(model_id, track_right_id, texelOffset);
        
        models.drawMesh(model_id, track_left_id, MDL_DRW_VERTEXARRAY);
        models.drawMesh(model_id, track_right_id, MDL_DRW_VERTEXARRAY);
        
        // If object is selected, display the "selected" visual
        if(selected)
            glCallList(selected_dspList);
        
        glPopMatrix();
        
        // Draw turret (using its matrix)
        for(i = 0; i < turret_count; i++)
        {
            glPushMatrix();
            
            glMultMatrixf(turret_matrix[i]);
            
            glCallList(turret_dspList[i]);
            
            glPopMatrix();
        }
        
        // Draw gun mantlets and their gun (using their matricies)
        for(i = 0; i < gun_count; i++)
        {
            glPushMatrix();
            
            glMultMatrixf(gun_matrix[i]);
            
            glCallList(gun_mant_dspList[i]);
            
            glTranslatef(0.0, 0.0, -gun[i].getGunRecoil());     // Recoil gun
            glCallList(gun_dspList[i]);
                          
            glPopMatrix();
        }
    }
    
    // Draw waypoints if the object is selected
    if(selected)
        displayWaypoints();
}

/*******************************************************************************
    function    :   tank_object::displayWaypoints
    arguments   :   <none>
    purpose     :   Displays the waypoints of the tank object,
    notes       :   <none>
*******************************************************************************/
void tank_object::displayWaypoints()
{
    kVector wp_dir;
    kVector wp_pos;
    kVector previous_wp;
    waypoint_node* curr;
    float step;
    float t;
    
    // POSSIBILITY OF TEMPORARY CODE
    
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_COLOR_MATERIAL);
    
    glAlphaFunc(GL_GEQUAL, 0.0);
    
    glEnable(GL_LINE_SMOOTH);           // Enable nice 2px lines for waypoints
    glLineWidth(2.0);
    
    glPointSize(10.0);                  // Enable nice 10px points for WPs
    glEnable(GL_POINT_SMOOTH);
    
    glColor3f(0.8, 0.8, 0.8);           // Draw lines first - set to silver

    curr = wl_head;
    previous_wp = pos;
    while(curr)
    {
      glBegin(GL_LINE_STRIP);
        wp_dir = previous_wp - curr->waypoint;
        step = 15.0 / magnitude(wp_dir);
        t = 0.0;
        while(t < 1.0)
        {
            wp_pos = curr->waypoint + (wp_dir * t);
            wp_pos[1] = map.getOverlayHeight(wp_pos[0], wp_pos[2]);
            glVertex3f(wp_pos[0], wp_pos[1] + 1.0, wp_pos[2]);
            t += step;
        }
        
        glVertex3f(previous_wp[0], previous_wp[1] + 1.0, previous_wp[2]);
        previous_wp = curr->waypoint;
        curr = curr->next;
        glEnd();
    }
    
    glColor3f(0.0, 0.25, 0.8);
    glBegin(GL_POINTS);
        curr = wl_head;
        while(curr)
        {
            glVertex3f(curr->waypoint[0], curr->waypoint[1] + 1.0, curr->waypoint[2]);
            curr = curr->next;
        }
    glEnd();
    
    glDisable(GL_COLOR_MATERIAL);
    glLineWidth(1.0);
    glDisable(GL_LINE_SMOOTH);
    glEnable(GL_LIGHTING);
    
    glAlphaFunc(GL_GEQUAL, ALPHA_PASS);
}
