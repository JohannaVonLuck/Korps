/*******************************************************************************
                      Projectile Object - Implementation
*******************************************************************************/
#include "main.h"
#include "projectile.h"
#include "camera.h"
#include "console.h"
#include "database.h"
#include "effects.h"
#include "metrics.h"
#include "misc.h"
#include "model.h"
#include "object.h"
#include "objhandler.h"
#include "scenery.h"
#include "sounds.h"
#include "texture.h"

/*******************************************************************************
    function    :   short int ammoType
    arguments   :   typeStr - text string tag for type of round
    purpose     :   Determines what projectile type a given shell is based on
                    it's text string round type tag.
    notes       :   1) Returns -1 if in error.
                    2) typeStr must not be padded, and must be in uppercase.
*******************************************************************************/
short int ammoType(char* typeStr)
{
    // Make sure it's a valid string, otherwise return error.
    if(typeStr == NULL || typeStr[0] == '\0')
        return AMMO_TYPE_UNKNOWN;
    
    // Determine ammo type based on string. Fairly simple opp. Done in a
    // fashion for efficiency based on the most common of rounds first.
    if(strcmp(typeStr, "AP") == 0)
        return AMMO_TYPE_AP;
    else if(strcmp(typeStr, "HE") == 0)
        return AMMO_TYPE_HE;
    else if(strcmp(typeStr, "APC") == 0)
        return AMMO_TYPE_APC;
    else if(strcmp(typeStr, "HEAT") == 0)
        return AMMO_TYPE_HEAT;
    else if(strcmp(typeStr, "APCBC") == 0)
        return AMMO_TYPE_APCBC;
    else if(strcmp(typeStr, "SMOKE") == 0)
        return AMMO_TYPE_SMOKE;
    else if(strcmp(typeStr, "APCR") == 0)
        return AMMO_TYPE_APCR;
    else if(strcmp(typeStr, "APBC") == 0)
        return AMMO_TYPE_APBC;
    else if(strcmp(typeStr, "HESH") == 0)
        return AMMO_TYPE_HESH;
    else if(strcmp(typeStr, "API") == 0)
        return AMMO_TYPE_API;
    
    // Otherwise, we have no idea what kind of ammo type it is.    
    return AMMO_TYPE_UNKNOWN;
}

/*******************************************************************************
    function    :   proj_object::proj_object
    arguments   :   <none>
    purpose     :   Constructor.
    notes       :   <none>
*******************************************************************************/
proj_object::proj_object()
{
    travel_distance = 0.0;
    distance_offset = 0.0;
    remove_timer = 6.0;
    projectile_flight = true;
    projectile_damaged = false;
    cdr_passes = 0;
    
    type = AMMO_TYPE_UNKNOWN;
    velocity = 0.0;
    diameter = 0.0;
    explosive = 0.0;
    
    tracer_depth = PROJ_MAX_TRACER_TAIL;
    tracer_timer = 0.0;
}

/*******************************************************************************
    function    :   proj_object::~proj_object
    arguments   :   <none>
    purpose     :   Deconstructor.
    notes       :   <none>
*******************************************************************************/
proj_object::~proj_object()
{
    if(cdtl_head)
    {
        cdtl_node* curr = cdtl_head;
        
        while(curr)
        {
            cdtl_head = cdtl_head->next;
            delete curr;
            curr = cdtl_head;
        }
    }
}

/*******************************************************************************
    function    :   proj_object::initProj
    arguments   :   parentPtr - Pointer to parent object (for CDTL exclusion)
                    roundType - Type of round (DB)
                    position - Start position of round
                    direction - Starting direction of round (cart.)
    purpose     :   Initializes a projectile object given the passed arguments.
    notes       :   Both position and direction passed must be in cartesian.
*******************************************************************************/
void proj_object::initProj(object* parentPtr, char* roundType, kVector position,
    kVector direction)
{
    int i;
    char* temp;
    char buffer[128];
    
    //cout << SDL_GetTicks() << ": [" << (unsigned int)this << "]: Initializing Projectile: " << roundType << " from parent " << (dynamic_cast<unit_object*>(parentPtr))->obj_id << endl;
    
    // Copy over specifics about round and init all that fun stuff
    obj_model = strdup(roundType);
    obj_type = OBJ_TYPE_PROJECTILE;
    obj_modifiers = AMMO_MOD_STANDARD;
    
    // Grab type of projectile
    temp = db.query(obj_model, "TYPE");
    if(temp)
    {
        type = ammoType(temp);
        
        if(type == AMMO_TYPE_UNKNOWN)
        {
            // Check for valid shell type
            sprintf(buffer, "Proj: Shell type for round \"%s\" invalid.",
                roundType);
            write_error(buffer);
            return;
        }
    }
    else
    {
        // Check for valid entry in DB
        sprintf(buffer, "Proj: Shell type for round \"%s\" not defined.",
            roundType);
        write_error(buffer);
        return;
    }
    
    // Grab velocity (in m/s) of projectile
    temp = db.query(obj_model, "VELOCITY");
    if(temp)
        velocity = atof(temp);
    else
    {
        // Check for valid entry in DB
        sprintf(buffer, "Proj: Velocity for round \"%s\" not defined.",
            roundType);
        write_error(buffer);
        return;
    }
    
    // Grab diameter (in cm) of projectile
    temp = db.query(obj_model, "CALIBER");
    if(temp)
        diameter = atof(temp);
    else
    {
        // Check for valid entry in DB
        sprintf(buffer, "Proj: Caliber for round \"%s\" not defined.",
            roundType);
        write_error(buffer);
        return;
    }
    
    // Grab explosive content of projectile
    temp = db.query(obj_model, "EXPLOSIVE");
    if(temp)
        explosive = atof(temp);
    
    // Grab the projectile modifiers
    temp = db.query(obj_model, "MODIFIERS");
    if(temp)
    {
        // Assign a tracer
        if(strstr(temp, "TRACER"))
        {
            if(strstr(temp, "YELLOW"))
                obj_modifiers = obj_modifiers | AMMO_MOD_YELLOW_TRACER;
            else if(strstr(temp, "WHITE"))
                obj_modifiers = obj_modifiers | AMMO_MOD_WHITE_TRACER;
            else if(strstr(temp, "RED"))
                obj_modifiers = obj_modifiers | AMMO_MOD_RED_TRACER;
            else if(strstr(temp, "GREEN"))
                obj_modifiers = obj_modifiers | AMMO_MOD_GREEN_TRACER;
        }
        
        // And handle any other modifiers
        if(strstr(temp, "HE_BURSTER"))
            obj_modifiers = obj_modifiers | AMMO_MOD_HE_BURSTER;
    }
    else
    {
        // Check for valid entry in DB
        sprintf(buffer, "Proj: Modifiers for round \"%s\" not defined.",
            roundType);
        write_error(buffer);
        return;
    }
    
    // Copy over position and direction vectors
    pos = position;
    dir = direction;
    
    // Initialize all tracer tail positions to starting position
    for(i = 0; i < PROJ_MAX_TRACER_TAIL; i++)
    {
        tracer_tail_pos[i][0] = pos[0];
        tracer_tail_pos[i][1] = pos[1];
        tracer_tail_pos[i][2] = pos[2];
    }
    
    // Assign velocity multiplier to direction vector as well as incorporate
    // the random dispersion effect for new projectiles.
    dir.convertTo(CS_SPHERICAL);
    dir[0] = velocity * PROJ_VEL_MULTIPLIER;    // Set magnitude to velocity
    dir[1] += ((((float)rand() / (float)RAND_MAX) * PROJ_FIRE_DISPERSION) -
                (PROJ_FIRE_DISPERSION / 2.0)) * degToRad;
    dir[2] += ((((float)rand() / (float)RAND_MAX) * PROJ_FIRE_DISPERSION) -
                (PROJ_FIRE_DISPERSION / 2.0)) * degToRad;
    dir.convertTo(CS_CARTESIAN);
    
    // Initialize a CD testing list
    cdtl_head = objects.createCDTL((object*)this, 15.0, 20.0, parentPtr);
    
    // Finally, this object was created successfully
    obj_status = OBJ_STATUS_OK;
}

/*******************************************************************************
    function    :   proj_object::armDevice
    arguments   :   <none>
    purpose     :   Arms any HE bursters or incindenary devices (if present).
    notes       :   Only applies to those shells with such devices present.
*******************************************************************************/
void proj_object::armDevice()
{
    // Check for HE burster
    if((obj_modifiers & AMMO_MOD_HE_BURSTER) &&
       !(obj_modifiers & AMMO_MOD_HE_BURSTER_ARMED))
    {
        obj_modifiers = obj_modifiers | AMMO_MOD_HE_BURSTER_ARMED;
        
        if(remove_timer > 0.002)
            remove_timer = 0.002;
    }
    
    // do the incendinary stuff later
}

/*******************************************************************************
    function    :   proj_object::interupt
    arguments   :   newPos - new position vector to assign to.
                    newDir - new direction vector to assign to (cart.)
                    newVelocity - new velocity scalar to assign to
                    timeOver - Time offset of overshooting new position
                    damageShell - Should we assume we damaged the shell?
    purpose     :   Updates the projectile object's orientation with new values.
    notes       :   Used in junction with CDR module.
*******************************************************************************/
void proj_object::interupt(kVector newPos, kVector newDir, float newVelocity,
    float timeOver, bool damageShell)
{
    kVector direction;
    int i;
    
    // Only need to handle overshoots for projectiles with tracer tails
    if((obj_modifiers & AMMO_MOD_TRACER) || diameter >= 1.5)
    {
        // timeOver should come in as a negative number if we indeed did
        // overshoot the new position by some factor.
        if(timeOver < 0.0)
        {
            // Roll back by timeOver*velocity amount to handle the overshoot.
            float overshot = fabsf(timeOver * velocity * PROJ_VEL_MULTIPLIER);
            
            for(i = 0; i < PROJ_MAX_TRACER_TAIL; i++)
            {
                if(distanceBetween(pos, kVector(
                    tracer_tail_pos[i][0], tracer_tail_pos[i][1], tracer_tail_pos[i][2]))
                   <= overshot)
                {
                    tracer_tail_pos[i][0] = newPos[0];
                    tracer_tail_pos[i][1] = newPos[1];
                    tracer_tail_pos[i][2] = newPos[2];
                }
                else    // Done figuring out which overshoot
                    break;
            }
        }
        
        tracer_tail_pos[0][0] = newPos[0];
        tracer_tail_pos[0][1] = newPos[1];
        tracer_tail_pos[0][2] = newPos[2];
        
        for(i = PROJ_MAX_TRACER_TAIL - 1; i > 0; i--)
        {
            tracer_tail_pos[i][0] = tracer_tail_pos[i - 1][0];
            tracer_tail_pos[i][1] = tracer_tail_pos[i - 1][1];
            tracer_tail_pos[i][2] = tracer_tail_pos[i - 1][2];
        }
    }
    
    // Set new values
    pos = newPos;
    dir = newDir;
    velocity = newVelocity;
    
    // See if the interrupt was done on a projectile that is set to already
    // have been knocked down.
    if(velocity > 0.0 && !projectile_flight)
        projectile_flight = true;
    
    // Add in dispersion factor for interuption
    dir.convertTo(CS_SPHERICAL);
    dir[0] = newVelocity * PROJ_VEL_MULTIPLIER; // Set magnitude to velocity
    dir[1] += ((((float)rand() / (float)RAND_MAX) * PROJ_INT_DISPERSION) -
                (PROJ_INT_DISPERSION / 2.0)) * degToRad;
    dir[2] += ((((float)rand() / (float)RAND_MAX) * PROJ_INT_DISPERSION) -
                (PROJ_INT_DISPERSION / 2.0)) * degToRad;
    direction[1] = dir[1];
    direction[2] = dir[2];
    dir.convertTo(CS_CARTESIAN);
    
    // Do moving forward traceback only if timeOver is negative and the
    // projectile is still in motion.
    if(projectile_flight && timeOver < 0.0)
    {
        pos += dir * fabsf(timeOver);
        
        if((obj_modifiers & AMMO_MOD_TRACER) || diameter > 1.5)
        {
            tracer_tail_pos[0][0] = pos[0];
            tracer_tail_pos[0][1] = pos[1];
            tracer_tail_pos[0][2] = pos[2];
        }
    }
    
    // Update Hull Matrix
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    // Peform orientation
    glTranslatef(pos[0], pos[1], pos[2]);
    glRotatef(direction[2] * radToDeg, 0.0, 1.0, 0.0);
    glRotatef(direction[1] * radToDeg, 1.0, 0.0, 0.0);
    glRotatef(roll * radToDeg, 0.0, 1.0, 0.0);
    glGetFloatv(GL_MODELVIEW_MATRIX, hull_matrix);
    glPopMatrix();
    
    if(damageShell)
    {
        // Set damaged flag and set life left to 1s (since damaged).
        projectile_damaged = true;
        
        if(remove_timer > 1.0)
            remove_timer = 1.0;
    }
}

/*******************************************************************************
    function    :   proj_object::update
    arguments   :   deltaT - number of seconds elapsed since last update
    purpose     :   Updates the projectile object.
    notes       :   Minor collision detection is performed if round penetrates
                    the ground.
*******************************************************************************/
void proj_object::update(float deltaT)
{
    int i;
    kVector direction;
    bool ground_collision = false;
    bool scenery_collision = false;
    int snd_id = 0;
    
    // If projectile is still being drawn (tracer not finished), then update
    // orientation of projectile and everything else that is associated when
    // the projectile is in motion.
    if(projectile_flight)
    {
        // Update position/direction vectors
        dir[1] += (-9.81 * deltaT);
        pos += (dir * deltaT);
        roll += (125.6637 * deltaT);
        
        // Update travel distance value
        travel_distance += (velocity * PROJ_VEL_MULTIPLIER * deltaT);
        
        // Convert to spherical for yaw and pitch values
        direction = vectorIn(dir, CS_SPHERICAL);
        
        // Update CD/Culling radius to reflect distance traveled (for CD jump-fix).
        radius = direction[0] * deltaT;
        if(radius < 3.0)
            radius = 3.0;
        
        // Update Hull Matrix
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        // Peform orientation
        glTranslatef(pos[0], pos[1], pos[2]);
        glRotatef(direction[2] * radToDeg, 0.0, 1.0, 0.0);
        glRotatef(direction[1] * radToDeg, 1.0, 0.0, 0.0);
        glRotatef(roll * radToDeg, 0.0, 1.0, 0.0);
        glGetFloatv(GL_MODELVIEW_MATRIX, hull_matrix);
        glPopMatrix();
        
        // Update the life left for this projectile
        remove_timer -= deltaT;
        
        // Check for stop updating
        if(remove_timer <= 0.0)
        {
            // Check for HE burster arm (to add explosion effect)
            if(obj_modifiers & AMMO_MOD_HE_BURSTER_ARMED)
            {
                effects.addEffect(SE_EXPLOSION, pos, dir, explosive);
                snd_id = sounds.addSound(SOUND_EXPLOSION, SOUND_HIGH_PRIORITY,
                    pos(), SOUND_PLAY_ONCE);
                sounds.setSoundRolloff(snd_id, 0.05);
                sounds.auxModOnExplosive(snd_id, explosive);
            }
            
            // Set to stop projectile flight movement
            projectile_flight = false;
            
            // If an MG bullet with no tracer is defined, then delete object.
            if(diameter < 1.5 && !(obj_modifiers & AMMO_MOD_TRACER))
            {
                obj_status = OBJ_STATUS_REMOVE;
                return;
            }
        }
        
        if((obj_modifiers & AMMO_MOD_TRACER) || diameter >= 1.5)
        {
            // Update tracer timer
            tracer_timer += deltaT;
            
            // Determine whenever we need to advance our tracer tail
            if(tracer_timer > PROJ_TRACER_TIMER)
            {
                while(tracer_timer > PROJ_TRACER_TIMER)
                {
                    for(i = PROJ_MAX_TRACER_TAIL - 1; i > 0; i--)
                    {
                        tracer_tail_pos[i][0] = tracer_tail_pos[i - 1][0];
                        tracer_tail_pos[i][1] = tracer_tail_pos[i - 1][1];
                        tracer_tail_pos[i][2] = tracer_tail_pos[i - 1][2];
                    }
                    
                    tracer_timer -= PROJ_TRACER_TIMER;
                }
            }
            
            // Always update first position
            tracer_tail_pos[0][0] = pos[0];
            tracer_tail_pos[0][1] = pos[1];
            tracer_tail_pos[0][2] = pos[2];
        }
        
        // Check for projectile hitting ground or hitting scenery objects.
        if(map.groundCollision(pos))
            ground_collision = true;
        else if(map.sceneryCollision(pos))
            scenery_collision = true;
    }
    else
    {
        if((obj_modifiers & AMMO_MOD_TRACER) || diameter >= 1.5)
        {
            // Update tracer timer
            tracer_timer += deltaT;
            
            // Determine whenever we need to advance our tracer tail
            if(tracer_timer >= PROJ_TRACER_TIMER)
            {
                while(tracer_timer >= PROJ_TRACER_TIMER)
                {
                    for(i = PROJ_MAX_TRACER_TAIL - 1; i > 0; i--)
                    {
                        tracer_tail_pos[i][0] = tracer_tail_pos[i - 1][0];
                        tracer_tail_pos[i][1] = tracer_tail_pos[i - 1][1];
                        tracer_tail_pos[i][2] = tracer_tail_pos[i - 1][2];
                    }
                    
                    tracer_timer -= PROJ_TRACER_TIMER;
                    tracer_depth--;
                }
            }
            
            // See if tracer tail is done drawing
            if(tracer_depth <= 0)
            {
                obj_status = OBJ_STATUS_REMOVE;
                return;
            }
        }
        else
        {
            obj_status = OBJ_STATUS_REMOVE;
            return;
        }
    }
    
    if(ground_collision || scenery_collision)
    {
        float size;
        projectile_flight = false;
        
        //cout << SDL_GetTicks() << ": [" << (unsigned int)this << "]: G/S Collision Projectile remove" << endl;
        
        if(!projectile_damaged && cdr_passes <= 0)
        {
            // Add effect for shell hitting ground
            switch(type)
            {
                case AMMO_TYPE_AP:
                case AMMO_TYPE_APC:
                case AMMO_TYPE_APBC:
                case AMMO_TYPE_APCBC:
                case AMMO_TYPE_APCR:
                case AMMO_TYPE_SMOKE:
                    if(diameter >= 1.5)
                    {
                        if(obj_modifiers & AMMO_MOD_HE_BURSTER)
                        {
                            size = 10.0 * diameter - 5.0;
                            if(ground_collision) 
                            {
                                // Add some dirt and dust cloud effects
                                effects.addEffect(SE_DIRT, pos, size);
                                effects.addEffect(SE_QF_BR_SMOKE, pos, size / 5.0f);
                                snd_id = sounds.addSound(SOUND_GROUND_EXPLOSION,
                                    SOUND_HIGH_PRIORITY, pos(), SOUND_PLAY_ONCE);
                            }
                            else if(scenery_collision)
                            {
                                // Add some debris and some dust cloud effects
                                effects.addEffect(SE_DEBRIS, pos, normalized(dir) * -1.0f, diameter);
                                effects.addEffect(SE_QF_WH_SMOKE, pos, 2.5f * diameter);
                                snd_id = sounds.addSound(SOUND_EXPLOSION,
                                    SOUND_HIGH_PRIORITY, pos(), SOUND_PLAY_ONCE);
                            }
                            
                            // Explosion effect is universal
                            effects.addEffect(SE_EXPLOSION, pos, explosive);
                            
                            // Do some sound modding
                            sounds.setSoundRolloff(snd_id, 0.05);
                            sounds.auxModOnExplosive(snd_id, explosive);
                        }
                        else
                        {
                            size = 8.16667 * diameter - 11.25;
                            if(ground_collision) 
                            {
                                // Add some dirt and dust cloud effects
                                effects.addEffect(SE_DIRT, pos, size);
                                effects.addEffect(SE_QF_BR_SMOKE, pos, size / 7.0f);
                            }
                            else if(scenery_collision)
                            {
                                // Add some debris and some dust cloud effects
                                effects.addEffect(SE_DEBRIS, pos, normalized(dir) * -1.0f, diameter);
                                effects.addEffect(SE_QF_WH_SMOKE, pos, 2.5f * diameter);
                            }
                            
                            // Add a "thud" sound
                            snd_id = sounds.addSound(
                                    SOUND_SHELL_THUD,
                                    SOUND_MID_PRIORITY, pos(), SOUND_PLAY_ONCE);
                            
                            // Do some sound modding
                            sounds.setSoundRolloff(snd_id, 0.05);
                        }
                    }
                    else
                        if(ground_collision)
                            effects.addEffect(SE_MG_GROUND, pos);
                        else if(scenery_collision)
                            effects.addEffect(SE_MG_GROUND_SMOKE, pos);
                    
                    // Special case SMOKE projectiles
                    if(type == AMMO_TYPE_SMOKE)
                    {
                        char* temp;
                        float weight;
                        temp = db.query(obj_model, "WEIGHT");
                        if(temp)
                        {
                            float amount;
                            weight = atof(temp);
                            amount = (58.823529 * weight) + 4.705882;
                            if(amount > 0)
                                effects.addEffect(SE_WH_DISPENSER_SMOKE, pos, amount);
                        }
                    }
                    break;
                    
                case AMMO_TYPE_API:
                case AMMO_TYPE_HE:
                case AMMO_TYPE_HEAT:
                case AMMO_TYPE_HESH:
                    size = 11.1111 * diameter + 3.33333;
                    if(ground_collision)
                    {
                        effects.addEffect(SE_DIRT, pos, size);
                        effects.addEffect(SE_QF_BR_SMOKE, pos, size / 5.0f);
                        effects.addEffect(SE_EXPLOSION, pos, explosive);
                        snd_id = sounds.addSound(SOUND_GROUND_EXPLOSION,
                                    SOUND_HIGH_PRIORITY, pos(), SOUND_PLAY_ONCE);
                    }
                    else if(scenery_collision)
                    {
                        // Shrapnel to be replaced with debris later
                        effects.addEffect(SE_SHRAPNEL, pos + (dir * (-deltaT/2.0f)), normalized(dir) * -1.0f, size);
                        effects.addEffect(SE_QF_BR_SMOKE, pos + (dir * (-deltaT/2.0f)), normalized(dir) * -1.0f, size / 5.0f);
                        effects.addEffect(SE_EXPLOSION, pos + (dir * (-deltaT/2.0f)), normalized(dir) * -1.0f, explosive);
                        snd_id = sounds.addSound(SOUND_EXPLOSION,
                                    SOUND_HIGH_PRIORITY, pos(), SOUND_PLAY_ONCE);
                    }
                    sounds.setSoundRolloff(snd_id, 0.01);
                    sounds.auxModOnExplosive(snd_id, explosive);
                    break;
                    
                default:
                    break;
            }
        }
    }
    
    // Camera culling
    draw = camera.sphereInView(pos(), 2.5);
    if(!draw && ((obj_modifiers & AMMO_MOD_TRACER) || diameter >= 1.5))
    {
        for(i = 0; i < PROJ_MAX_TRACER_TAIL && !draw; i++)
            draw = camera.pointInView(tracer_tail_pos[i]);
    }
}

/*******************************************************************************
    function    :   proj_object::display
    arguments   :   <none>
    purpose     :   Display routine.
    notes       :   Going into the tracer/trail drawing routine, the matrix
                    has not been poped off the stack so that the tracer blip
                    (a GL_POINT) can be drawn in the correct spot behind the
                    projectile (which requires a transformation).
*******************************************************************************/
void proj_object::display()
{
    //int i;
    static float* cam_pos = camera.getCamPos();
    static int modlib_id = models.getModelID("shell");
    
    // Culling check
    if(!draw)
        return;

    // Orient projectile
    glPushMatrix();
    
    // Orient object
    glMultMatrixf(hull_matrix);
    
    // Scale based on a multipler from accurate so that the round does
    // actually show up on-screen in some fashion.
    glScalef(diameter * 2.0, diameter * 2.0, diameter * 2.0);
    
    // Draw projectile
    if(projectile_flight)
    {
        if(!projectile_damaged)
            models.drawModel(modlib_id, MDL_DRW_VERTEXARRAY);
        else
        {
            glEnable(GL_COLOR_MATERIAL);
            glColor4f(1.0, 1.0, 0.0, 1.0);
            models.drawModel(modlib_id, MDL_DRW_VERTEXARRAY_NO_MATERIAL);
            glDisable(GL_COLOR_MATERIAL);
        }
    }
    
    // Tracer/trail drawing handler. Although the tracers are used primarily
    // for tracer proj., it is also used for non-tracers to give a sorta
    // small smoke trail line.
    if(diameter >= 2.0 || (obj_modifiers & AMMO_MOD_TRACER))
    {
        // Sadly, when working with point sizes, we must compute distance.
        float distance =
            sqrt(((cam_pos[0] - pos[0]) * (cam_pos[0] - pos[0])) +
            ((cam_pos[1] - pos[1]) * (cam_pos[1] - pos[1])) +
            ((cam_pos[2] - pos[2]) * (cam_pos[2] - pos[2])));
        float base_size = distance * -0.0055;   // Change size with distance
        float size;
        
        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_POINT_SMOOTH);
        
        if(obj_modifiers & AMMO_MOD_TRACER)
        {
            // Draw specific tracer line
            switch(obj_modifiers & AMMO_MOD_TRACER)
            {
                case AMMO_MOD_YELLOW_TRACER:    // Draw yellow tracer
                    if(projectile_flight)
                    {
                        // Determine size and clip. Draw smaller blip for MGs.
                        if(diameter >= 2.0)
                        {
                            size = 3.75 + base_size;
                            if(size < 0.5) size = 0.5;
                        }
                        else
                        {
                            size = 2.75 + base_size;
                            if(size < 0.25) size = 0.25;
                        }
                        
                        // Draw tracer blip (using GL_POINT)
                        glPointSize(size);
                        glColor4f(1.0, 1.0, 0.0, 1.0);
                        glBegin(GL_POINTS);
                            glVertex3f(0.0, -0.02, 0.0);
                        glEnd();
                    }
                    
                    // Done with blip - don't forget to pop matrix.
                    glPopMatrix();
                    
                    if(diameter >= 2.0)
                    {
                        // Draw inner tracer blip run-off line for non-MGs
                        size = 2.5 + base_size;
                        if(size < 0.1) size = 0.1;      // Size clip
                        glLineWidth(size);
                        glBegin(GL_LINE_STRIP);
                            glColor4f(1.0, 1.0, 0.2, 0.8);
                            glVertex3fv(tracer_tail_pos[0]);
                            glColor4f(1.0, 1.0, 0.3, 0.6);
                            glVertex3fv(tracer_tail_pos[1]);
                            glColor4f(1.0, 1.0, 0.4, 0.4);
                            glVertex3fv(tracer_tail_pos[2]);
                            glColor4f(1.0, 1.0, 0.5, 0.2);
                            glVertex3fv(tracer_tail_pos[3]);
                        glEnd();
                    }
                    break;
                
                case AMMO_MOD_WHITE_TRACER:    // Draw white tracer
                    if(projectile_flight)
                    {
                        // Determine size and clip. Draw smaller blip for MGs.
                        if(diameter >= 2.0)
                        {
                            size = 3.75 + base_size;
                            if(size < 0.5) size = 0.5;
                        }
                        else
                        {
                            size = 2.75 + base_size;
                            if(size < 0.25) size = 0.25;
                        }
                        
                        // Draw tracer blip (using GL_POINT)
                        glPointSize(size);
                        glColor4f(1.0, 1.0, 1.0, 1.0);
                        glBegin(GL_POINTS);
                            glVertex3f(0.0, -0.02, 0.0);
                        glEnd();
                    }
                    
                    // Done with blip - don't forget to pop matrix.
                    glPopMatrix();
                    
                    if(diameter >= 2.0)
                    {
                        // Draw inner tracer blip run-off line for non-MGs
                        size = 2.5 + base_size;
                        if(size < 0.1) size = 0.1;      // Size clip
                        glLineWidth(size);
                        glBegin(GL_LINE_STRIP);
                            glColor4f(1.0, 1.0, 1.0, 0.8);
                            glVertex3fv(tracer_tail_pos[0]);
                            glColor4f(1.0, 1.0, 1.0, 0.6);
                            glVertex3fv(tracer_tail_pos[1]);
                            glColor4f(1.0, 1.0, 1.0, 0.4);
                            glVertex3fv(tracer_tail_pos[2]);
                            glColor4f(1.0, 1.0, 1.0, 0.2);
                            glVertex3fv(tracer_tail_pos[3]);
                        glEnd();
                    }
                    break;
                    
                case AMMO_MOD_RED_TRACER:    // Draw red/white tracer
                    if(projectile_flight)
                    {
                        // Determine size and clip. Draw smaller blip for MGs.
                        if(diameter >= 2.0)
                        {
                            size = 3.75 + base_size;
                            if(size < 0.5) size = 0.5;
                        }
                        else
                        {
                            size = 2.75 + base_size;
                            if(size < 0.25) size = 0.25;
                        }
                        
                        // Draw tracer blip (using GL_POINT)
                        glPointSize(size);
                        glColor4f(1.0, 0.3, 0.3, 1.0);
                        glBegin(GL_POINTS);
                            glVertex3f(0.0, -0.02, 0.0);
                        glEnd();
                    }
                    
                    // Done with blip - don't forget to pop matrix.
                    glPopMatrix();
                    
                    if(diameter >= 2.0)
                    {
                        // Draw inner tracer blip run-off line for non-MGs
                        size = 2.5 + base_size;
                        if(size < 0.1) size = 0.1;      // Size clip
                        glLineWidth(size);
                        glBegin(GL_LINE_STRIP);
                            glColor4f(1.0, 0.25, 0.25, 0.9);
                            glVertex3fv(tracer_tail_pos[0]);
                            glColor4f(1.0, 0.32, 0.32, 0.7);
                            glVertex3fv(tracer_tail_pos[1]);
                            glColor4f(1.0, 0.4, 0.4, 0.5);
                            glVertex3fv(tracer_tail_pos[2]);
                            glColor4f(1.0, 0.5, 0.5, 0.3);
                            glVertex3fv(tracer_tail_pos[3]);
                        glEnd();
                    }
                    break;
                    
                case AMMO_MOD_GREEN_TRACER:    // Draw green/white tracer
                    if(projectile_flight)
                    {
                        // Determine size and clip. Draw smaller blip for MGs.
                        if(diameter >= 2.0)
                        {
                            size = 3.75 + base_size;
                            if(size < 0.5) size = 0.5;
                        }
                        else
                        {
                            size = 2.75 + base_size;
                            if(size < 0.25) size = 0.25;
                        }
                        
                        // Draw tracer blip (using GL_POINT)
                        glPointSize(size);
                        glColor4f(0.3, 1.0, 0.3, 1.0);
                        glBegin(GL_POINTS);
                            glVertex3f(0.0, -0.02, 0.0);
                        glEnd();
                    }
                    
                    // Done with blip - don't forget to pop matrix.
                    glPopMatrix();
                    
                    if(diameter >= 2.0)
                    {
                        // Draw inner tracer blip run-off line for non-MGs
                        size = 2.5 + base_size;
                        if(size < 0.1) size = 0.1;      // Size clip
                        glLineWidth(size);
                        glBegin(GL_LINE_STRIP);
                            glColor4f(0.25, 1.0, 0.25, 0.9);
                            glVertex3fv(tracer_tail_pos[0]);
                            glColor4f(0.32, 1.0, 0.32, 0.7);
                            glVertex3fv(tracer_tail_pos[1]);
                            glColor4f(0.4, 1.0, 0.4, 0.5);
                            glVertex3fv(tracer_tail_pos[2]);
                            glColor4f(0.5, 1.0, 0.5, 0.3);
                            glVertex3fv(tracer_tail_pos[3]);
                        glEnd();
                    }
                    break;
            }
            
            if(diameter >= 2.0)
            {
                // Draw large smoke trail for non-MGs
                size = 4.0 + base_size;
                if(size < 0.1) size = 0.1;          // Size clip
                glLineWidth(size);
                glBegin(GL_LINE_STRIP);
                    glColor4f(1.0, 1.0, 1.0, 0.25);
                    glVertex3fv(tracer_tail_pos[0]);
                    glVertex3fv(tracer_tail_pos[1]);
                    glColor4f(1.0, 1.0, 1.0, 0.20);
                    glVertex3fv(tracer_tail_pos[2]);
                    glVertex3fv(tracer_tail_pos[3]);
                    glVertex3fv(tracer_tail_pos[4]);
                    glVertex3fv(tracer_tail_pos[5]);
                    glColor4f(1.0, 1.0, 1.0, 0.15);
                    glVertex3fv(tracer_tail_pos[6]);
                    glColor4f(1.0, 1.0, 1.0, 0.10);
                    glVertex3fv(tracer_tail_pos[7]);
                    glColor4f(1.0, 1.0, 1.0, 0.05);
                    glVertex3fv(tracer_tail_pos[8]);
                    glColor4f(1.0, 1.0, 1.0, 0.00);
                    glVertex3fv(tracer_tail_pos[9]);
                glEnd();
            }
            else
            {
                // Draw small smoke trail for MGs
                size = 3.0 + base_size;
                if(size < 0.1) size = 0.1;          // Size clip
                glLineWidth(size);
                glBegin(GL_LINE_STRIP);
                    glColor4f(1.0, 1.0, 1.0, 0.20);
                    glVertex3fv(tracer_tail_pos[0]);
                    glVertex3fv(tracer_tail_pos[1]);
                    glColor4f(1.0, 1.0, 1.0, 0.15);                
                    glVertex3fv(tracer_tail_pos[2]);
                    glVertex3fv(tracer_tail_pos[3]);
                    glColor4f(1.0, 1.0, 1.0, 0.10);
                    glVertex3fv(tracer_tail_pos[4]);
                    glColor4f(1.0, 1.0, 1.0, 0.05);
                    glVertex3fv(tracer_tail_pos[5]);
                    glColor4f(1.0, 1.0, 1.0, 0.00);
                    glVertex3fv(tracer_tail_pos[6]);
                glEnd();
            }
        }
        else
        {
            glPopMatrix();      // Don't forget to pop matrix
            
            // Draw small trail line for non-tracers
            size = 3.0 + base_size;
            if(size < 0.1) size = 0.1;      // Size clip
            glLineWidth(size);
            glBegin(GL_LINE_STRIP);
                glColor4f(0.85, 0.85, 0.85, 0.50);
                glVertex3fv(tracer_tail_pos[0]);
                glColor4f(0.75, 0.75, 0.75, 0.35);
                glVertex3fv(tracer_tail_pos[1]);
                glColor4f(0.65, 0.65, 0.65, 0.25);
                glVertex3fv(tracer_tail_pos[2]);
                glColor4f(0.65, 0.65, 0.65, 0.10);
                glVertex3fv(tracer_tail_pos[3]);
                glColor4f(0.65, 0.65, 0.65, 0.0);
                glVertex3fv(tracer_tail_pos[4]);
            glEnd();
        }
        
        glPointSize(1.0);
        glLineWidth(1.0);
        glDisable(GL_COLOR_MATERIAL);
        glEnable(GL_TEXTURE_2D);    
        glDisable(GL_LINE_SMOOTH);
        glDisable(GL_POINT_SMOOTH);
    }
    else
        glPopMatrix();      // Don't forget to pop matrix
}
