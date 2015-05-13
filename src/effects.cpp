/*******************************************************************************
                        Special Effects - Implementation
*******************************************************************************/
#include "main.h"
#include "effects.h"
#include "camera.h"
#include "console.h"
#include "metrics.h"
#include "texture.h"
#include "scenery.h"
#include "script.h"

/*******************************************************************************
    function    :   effect::effect
    arguments   :   effectType - Effect type identifier
                    position - Starting postion for effect
                    direction - Starting direction for effect
                    modifier - Primary modifier value (usage varies per effect)
    purpose     :   Constructor. Builds the effect for immediate use.
    notes       :   <none>
*******************************************************************************/
effect::effect(int effectType, kVector position, kVector direction,
    float modifier, kVector systemDirection)
{
    int i;
    kVector original;
    float exp_size = 0.0;
    
    // Copy over passed values and set default values
    effect_type = effectType;               // Effect basis
    effect_done = false;
    
    pl_head = NULL;                         // Particle linked list
    
    emit_count = 1;                         // Emitting
    emit_rate = 0.0;
    emit_timer = 0.0;

    life_time = 5.0;                        // Timers
    run_time = 0.01;
    effect_life = -1.0;
    
    mod = modifier;
    
    start_pos = position;                   // Starting values
    start_dir = direction;
    start_size =  1.0;
    start_speed = 0.0;
    
    system_dir = systemDirection;
    
    gravity = -9.81;                        // Rates
    roll_speed = 0.0;
    resize_rate = 0.0;
    
    size_variance = 0.0;                    // Variances
    dispersion = 0.0;
    dispersion_area = 0.0;
    
    tex_frame = TEXTURE_NULL;               // Textures
    tex_frame_count = 0;
    tex_cycle_time = 0.0;
    tex_cycle_repeat = false;
    
    // Create type-specific values.
    switch(effect_type)
    {
        case SE_EXPLOSION:
            gravity = 0.0;
            life_time = 0.80;
            resize_rate = -(start_size / 3.0) / life_time;
            start_size = sqrt(modifier) * 8.96302 + 0.054756;
            tex_frame = textures.getTextureID("FX/exp.png");
            tex_frame_count = 10;
            tex_cycle_time = life_time / tex_frame_count;
            break;
            
        case SE_BASE_EXPLOSION:
            emit_count = 1;
            gravity = 0.0;
            life_time = 0.45;
            start_size = float(modifier * 0.777778 + 5.222222);
            start_speed = 2.0;
            tex_frame = textures.getTextureID("FX/base_exp.png");
            tex_frame_count = 1;
            tex_cycle_time = life_time / tex_frame_count;
            break;

        case SE_BIG_EXPLOSION:
            // The big explosion has been tested to take values in the range of {1, ..., 10}
            // but it may also look decent if higher inputs are entered
            effects.addEffect(SE_BASE_EXPLOSION, position, modifier);
            exp_size = sqrt(modifier);

            // Generate a number of sub explosions which is determined by the size of the modifier
            // value
            for(i = 0; i < int((exp_size * exp_size * exp_size) + 20); i++)
            {
                // Disperse sub explosions in a sphere whos size is based on the modifier 
                // value passed in.
                original = kVector(exp_size * (float(rand()) / RAND_MAX), (TWOPI / 2) * 
                    (float(rand()) / RAND_MAX), TWOPI * (float(rand()) / RAND_MAX));
                original.convertTo(CS_CARTESIAN); 
                
                effects.addEffect(SE_SUB_EXPLOSION, position + original, modifier);
            }

            effects.addEffect(SE_SMOKE_DEBRIS, position, modifier);
            effects.addEffect(SE_LARGE_DIRT, position, modifier);
            effects.addEffect(SE_DIRT, position, modifier);
            effects.addEffect(SE_DEBRIS, position, modifier);
            effects.addEffect(SE_SPARKS, position, modifier);
            effect_done = true;     // Done spawning
            break;

        case SE_SUB_EXPLOSION:
            emit_count = 1;
            dispersion = 56 * degToRad;
            gravity = -float(modifier * 0.066667 + 1.233333);
            life_time = float(modifier * 0.075333 + 1.596667);
            resize_rate = (start_size / 0.75) / life_time;
            start_size = float(modifier * 0.2 + 3.3);
            start_speed = float(modifier * 0.188889 + 1.311111);
            tex_frame = textures.getTextureID("FX/sub_exp.png");
            tex_frame_count = 16;
            tex_cycle_time = life_time / tex_frame_count;
            break;
        
        case SE_MG_GROUND:
            effects.addEffect(SE_MG_GROUND_SMOKE,position, direction);
            //effects.addEffect(SE_MG_DIRT,position,direction, 8.0f);
            gravity = 0.0;
            life_time = 0.16;
            start_size = 0.75;
            tex_frame = textures.getTextureID("FX/mg_ground.png");
            tex_frame_count = 1;
            tex_cycle_time = life_time / tex_frame_count;
            break;
            
        case SE_CANNON_FIRING:
            original = direction;
            
            start_size = sqrt(modifier) * 0.16;
            dispersion = 15.0 * degToRad;
            
            // Add smoke first
            effects.addEffect(SE_FIRING_SMOKE, position, direction, start_size * 0.8f, system_dir);
            effects.addEffect(SE_FIRING_SMOKE, position + (direction * 0.4f), direction, start_size * 1.2f, system_dir);
            effects.addEffect(SE_FIRING_SMOKE, position + (direction * 0.8f), direction, start_size * 1.6f, system_dir);
            
            effects.addEffect(SE_FIRING_SMOKE, position, direction, start_size * 0.8f, system_dir * 0.5f);
            effects.addEffect(SE_FIRING_SMOKE, position, direction, start_size * 1.2f, system_dir * 0.5f);
            effects.addEffect(SE_FIRING_SMOKE, position, direction, start_size * 1.6f, system_dir * 0.5f);

            for(i = 0; i < 15; i++)
            {
                direction[0] = ((float)rand() / (float)RAND_MAX);
                direction[1] = ((float)rand() / (float)RAND_MAX);
                direction[2] = ((float)rand() / (float)RAND_MAX);
                direction.normalize();
                direction *= (((float)rand() / (float)RAND_MAX) * 0.25f) + 0.5f;
                effects.addEffect(SE_FIRING_SMOKE, position, direction, start_size * 1.5f);
            }
            
            // Add firing blasts second
            effects.addEffect(SE_FIRING_BLAST, position, direction, start_size, system_dir);
            
            // Account for dispersion factor for the blast
            direction.convertTo(CS_SPHERICAL);
            direction[1] += (((float)rand() / (float)RAND_MAX) * dispersion) - (dispersion / 2.0);
            direction[2] += (((float)rand() / (float)RAND_MAX) * dispersion) - (dispersion / 2.0);
            direction.convertTo(CS_CARTESIAN);
            effects.addEffect(SE_FIRING_BLAST, position + (direction * 0.4f), direction, start_size * 1.4f, system_dir);
            
            // Account for another dispersion for the blast
            direction = original;
            direction.convertTo(CS_SPHERICAL);
            direction[1] += (((float)rand() / (float)RAND_MAX) * dispersion) - (dispersion / 2.0);
            direction[2] += (((float)rand() / (float)RAND_MAX) * dispersion) - (dispersion / 2.0);
            direction.convertTo(CS_CARTESIAN);
            effects.addEffect(SE_FIRING_BLAST, position + (direction * 0.8f), direction, start_size * 1.8f, system_dir);
            effect_done = true;     // Done spawning
            break;

        case SE_MG_FIRING:
            gravity = 0.0;
            life_time = 0.06;
            start_size = 0.18;
            size_variance = 0.025;
            tex_frame = textures.getTextureID("FX/mg_firing.png");
            tex_frame_count = 2;
            break;

        case SE_FIRING_BLAST:
            gravity = 0.0;
            life_time = 0.5;
            resize_rate = (start_size * 2.0) / life_time;
            roll_speed = 0.2;
            start_size = modifier;
            start_speed = 0.5;
            tex_frame = textures.getTextureID("FX/exp.png");
            tex_frame_count = 10;
            tex_cycle_time = life_time / tex_frame_count;
            break;
            
        case SE_FIRING_SMOKE:
            dispersion = 15.0 * degToRad;
            gravity = 1.0;
            life_time = 0.6;
            resize_rate = (3.0 - start_size) / life_time;
            roll_speed = 0.2;
            start_size = modifier;
            start_speed = 4.0;
            tex_frame = textures.getTextureID("FX/smoke_white.png");
            tex_frame_count = 1;
            break;
           
        case SE_DIRT:
            emit_count = (int)modifier;
            life_time = 2.0;
            start_size =  0.05;
            start_speed = 8.0;
            roll_speed = 3.0;
            size_variance = 0.045;
            dispersion = 120.0 * degToRad;
            tex_frame = textures.getTextureID("FX/dirt.png");
            tex_frame_count = 4;
            break;

        case SE_MG_DIRT:
            emit_count = (int)modifier;
            life_time = 0.8;
            start_size =  0.125;
            start_speed = 4.45;
            roll_speed = 2.3;
            size_variance = 0.045;
            dispersion = 105.0 * degToRad;
            tex_frame = textures.getTextureID("FX/dirt.png");
            tex_frame_count = 4;
            break;

        case SE_LARGE_DIRT:
            emit_count = (int)(modifier * 5 + 25);
            life_time = 4.4;
            start_size =  0.23;
            start_speed = modifier * 0.555556 + 8.444444;
            roll_speed = 2.0;
            size_variance = 0.05;
            dispersion = 160.0 * degToRad;
            tex_frame = textures.getTextureID("FX/dirt.png");
            tex_frame_count = 4;
            break;

        case SE_SHRAPNEL:
            dispersion = 90.0 * degToRad;
            emit_count = (int)modifier;
            life_time = 2.0;
            roll_speed = 3.0;
            size_variance = 0.045;
            start_size =  0.05;
            start_speed = 7.0;
            tex_frame = textures.getTextureID("FX/shrapnel.png");
            tex_frame_count = 4;
            break;

        case SE_DEBRIS:
            emit_count = (int)modifier;
            life_time = 4.4;
            start_size =  0.23;
            start_speed = modifier * 0.423729 + 10.0;
            roll_speed = 2.2;
            size_variance = 0.045;
            dispersion = 170.0 * degToRad;
            tex_frame = textures.getTextureID("FX/debris.png");
            tex_frame_count = 1;
            break;
        
        case SE_FIRE_DEBRIS:
            emit_count = 21;
            life_time = float(modifier * 0.022222 + 0.327778);
            start_size =  0.3;
            start_speed = 9.0;
            roll_speed = 3.0;
            size_variance = 0.045;
            dispersion = PI;
            tex_frame = textures.getTextureID("FX/debris.png");
            tex_frame_count = 1;
            break;

        case SE_SMOKE_DEBRIS:
            emit_count = int(modifier * 1.555556 + 4.444444);
            gravity = -7.0;
            life_time = float(modifier * 0.083333 + 1.516667);
            start_size =  0.23;
            start_speed = float(modifier * 0.5 + 9.5);
            roll_speed = 2.2;
            size_variance = 0.045;
            dispersion = 115.0 * degToRad;
            tex_frame = textures.getTextureID("FX/debris.png");
            tex_frame_count = 1;
            break;
            
        case SE_SPARKS:
            emit_count = int(modifier * 6.555556 + 44.444444);
            life_time = 4.9;
            start_size =  float(modifier * 0.008889 + 0.071111);
            start_speed = float(modifier * 0.644444 + 9.055556);
            roll_speed = 2.0;
            size_variance = 0.035;
            dispersion = 110.0 * degToRad;
            tex_frame = textures.getTextureID("FX/spark.png");
            tex_frame_count = 1;
            break;

        case SE_WH_DISPENSER_SMOKE:
            dispersion = PI * degToRad;
            effect_life = float(modifier);
            emit_count = -1;
            emit_rate = 1.0;
            gravity = 0.0;
            life_time = 20.0;
            resize_rate = (start_size * 20) / life_time;
            size_variance = 0.50;
            start_speed = 0.15;
            start_size = 2.6;
            tex_frame = textures.getTextureID("FX/smoke_light_white.png");
            tex_frame_count = 1;
            break;
            
        case SE_WH_BILLOWING_SMOKE:
            dispersion = 30.0 * degToRad;
            effect_life = float(modifier);
            emit_count = -1;
            emit_rate = 0.45;
            gravity = -0.01;
            life_time = 50.0;
            resize_rate = (start_size * 2.5) / life_time;
            roll_speed = 0.1;
            size_variance = 0.50;
            start_size =  3.5;
            start_speed = 0.75;
            tex_frame = textures.getTextureID("FX/smoke_white.png");
            tex_frame_count = 1;
            break;
            
        case SE_BK_BILLOWING_SMOKE:
            dispersion = 30.0 * degToRad;
            effect_life = float(modifier);
            emit_count = -1;
            emit_rate = 0.45;
            gravity = -0.01;
            life_time = 60.0;
            resize_rate = (3.5 * 7.0) / life_time;
            roll_speed = 0.1;
            size_variance = 0.50;
            start_size =  1.5;
            start_speed = 1.25;
            tex_frame = textures.getTextureID("FX/smoke_black.png");
            tex_frame_count = 1;
            break;
            
        case SE_QF_WH_SMOKE:
            dispersion = 140.0 * degToRad;
            emit_count = (int)modifier;
            if((int)modifier == -1)
                emit_rate = 0.15;
            life_time = 3.5;
            gravity = 0.2;
            resize_rate = (start_size * 1.25) / life_time;
            roll_speed = 0.1;
            size_variance = 0.25;
            start_size = 1.5;
            start_speed = 1.75;
            tex_frame = textures.getTextureID("FX/smoke_white.png");
            tex_frame_count = 1;
            break;
            
        case SE_QF_BR_SMOKE:
            dispersion = 140.0 * degToRad;
            emit_count = (int)modifier;
            if((int)modifier == -1)
                emit_rate = 0.15;
            gravity = 0.2;
            life_time = 3.5;
            resize_rate = (start_size * 4.25) / life_time;
            roll_speed = 0.1;
            size_variance = 0.25;
            start_size = 1.5;
            start_speed = 1.75;
            tex_frame = textures.getTextureID("FX/smoke_brown.png");
            tex_frame_count = 1;
            break;
            
        case SE_DUST_CLOUD:
            break;

        case SE_DUST_TRAIL:
            dispersion = PI;
            emit_count = -1;
            emit_rate = 0.2;
            gravity = 0.0;
            life_time = 0.5;
            roll_speed = 0.3 * float(rand())/RAND_MAX - 0.15;
            start_speed = 0.03;
            start_size = 0.3;
            resize_rate =  0.5;
            tex_frame = textures.getTextureID("FX/smoke_brown.png");
            tex_frame_count = 1;
            break;
        
        case SE_BL_DEBRIS_SMOKE:
            emit_count = 1;
            emit_rate = 1.5;
            life_time = float(mod * 0.095918 + 2.104082);
            start_speed = 0.2;
            dispersion = TWOPI;
            gravity = -0.01;
            start_size = 0.25;
            resize_rate = (0.95) / life_time;
            roll_speed = float(rand()) / RAND_MAX - 0.5;
            size_variance = 0.4;
            tex_frame = textures.getTextureID("FX/smoke_black.png");
            tex_frame_count = 1;
            break;

        case SE_FIRE_DEBRIS_SMOKE:
            emit_count = 1;
            emit_rate = 1.5;
            life_time = 0.1;
            start_speed = 2.6;
            dispersion = TWOPI;
            gravity = -0.01;
            start_size = 0.2;
            resize_rate = (0.35) / life_time;
            roll_speed = float(rand()) / RAND_MAX - 0.5;
            size_variance = 0.19;
            tex_frame = textures.getTextureID("FX/fire.png");
            tex_frame_count = 1;
            break;

        case SE_MG_GROUND_SMOKE:
            emit_count = 1;
            life_time = 0.35;
            gravity = 0.0;
            resize_rate = 0.9 / life_time;
            roll_speed = 0.2;
            start_size = 0.1;
            start_speed = 2.7;
            tex_frame = textures.getTextureID("FX/smoke_brown.png");
            tex_frame_count = 1;
            break;
            
        case SE_FIRE:
            dispersion = 50.0 * degToRad;
            effect_life = float(modifier);
            emit_count = -1;
            emit_rate = 0.009;
            gravity = 0.0;
            life_time = 0.67;
            resize_rate = 1.5 * start_size / life_time;
            start_size = 1.3;
            start_speed = 3.5;
            tex_frame = textures.getTextureID("FX/fire.png");
            tex_frame_count = 1;
            tex_cycle_time = life_time / tex_frame_count;
            break;

        case SE_RAIN:
            effects.prevCamPos = camera.getCamPosV();
            gravity = 0.0;
            emit_rate = 0.00175;
            dispersion_area = 120;
            life_time = 4.0;
            size_variance = 0.15;
            start_size =  0.95;
            start_speed = -60.0;
            tex_frame = textures.getTextureID("FX/rain.png");
            tex_frame_count = 2;
            break;
            
        case SE_SNOW:
            effects.prevCamPos = camera.getCamPosV();
            emit_rate = 0.004;
            dispersion_area = 100;
            gravity = 0.0;
            life_time = 15.0;
            roll_speed = 0.8;
            size_variance = 0.15;
            start_size =  0.5;
            start_speed = -4.0;
            start_dir[0] += 1.5 * (float(rand()) / RAND_MAX) - 0.75;
            start_dir[2] += 1.5 * (float(rand()) / RAND_MAX) - 0.75;
            tex_frame = textures.getTextureID("FX/snow.png");
            tex_frame_count = 1;
            break;
            
        default:
            effect_done = true;
            break;
    }
    
    // Image slice
    image_slice = float(1) / tex_frame_count;

    // Culling handling
    draw = true;
}

/*******************************************************************************
    function    :   effect::~effect
    arguments   :   <none>
    purpose     :   Deconstructor.
    notes       :   <none>
*******************************************************************************/
effect::~effect()
{
    particle* curr = pl_head;
    particle* temp;
    
    // Kill all particles
    while(curr)
    {
        temp = curr->next;
        delete curr;
        curr = temp;
    }
}

/*******************************************************************************
    function    :   effect::emit_particle
    arguments   :   <none>
    purpose     :   Emits a new particle based on the effect system and adds it
                    into the particle linked list.
    notes       :   <none>
*******************************************************************************/
void effect::emit_particle()
{
    particle* particle_ptr = new particle;
    
    // Initialize particle with basis information from effect
    particle_ptr->pos = start_pos;
    particle_ptr->dir = start_dir * start_speed;
    particle_ptr->roll = 0.0;
    particle_ptr->time_left = particle_ptr->life_time = life_time;
    particle_ptr->size = start_size;
    particle_ptr->tex_num = 0;
    particle_ptr->tex_cycle = tex_cycle_time;
    particle_ptr->parent = this;
    particle_ptr->distance_from_camera = 99999999;
    
    // Modify head pointer
    particle_ptr->next = pl_head;
    pl_head = particle_ptr;
    
    // Account for size variances
    if(size_variance != 0.0)
        particle_ptr->size += (((float)rand() / (float)RAND_MAX) * size_variance) - (size_variance / 2.0);
    
    // Used in debris to determine when to emit other particles
    particle_ptr->last_spawn = 0.0;
    
    // Do any effect-specific buisness
    switch(effect_type)
    {
        case SE_BASE_EXPLOSION:
        case SE_MG_FIRING:
        case SE_FIRING_SMOKE:
        case SE_FIRING_BLAST:
        case SE_WH_DISPENSER_SMOKE:
        case SE_WH_BILLOWING_SMOKE:
        case SE_BK_BILLOWING_SMOKE:
        case SE_MG_GROUND_SMOKE:
        case SE_QF_WH_SMOKE:
        case SE_QF_BR_SMOKE:
        case SE_DUST_CLOUD:
        case SE_FIRE:
            // Set roll to a random value
            particle_ptr->roll = ((float)rand() / (float)RAND_MAX) * TWOPI;
            
            // Set to random texture number based on available slots
            particle_ptr->tex_num = rand() % tex_frame_count;
            break;
            
        case SE_DUST_TRAIL:
            // Set roll to a random value
            particle_ptr->roll = ((float)rand() / (float)RAND_MAX) * TWOPI;
            
            particle_ptr->dir = start_dir * float((float(rand()) / RAND_MAX) * 0.2); 
            
            // Take ground value, linear regress to something 
            // between 0 - 10 and set lifetime of emitting effect 
            //set life and use linear regression
            //particle_ptr->time_left = particle_ptr->life_time = (map.getTileDustiness(map.getTileType(start_pos[0], start_pos[2])));
            break;
            
        case SE_DIRT:
        case SE_MG_DIRT:
        case SE_DEBRIS:
        case SE_LARGE_DIRT:
        case SE_SHRAPNEL:
        case SE_FIRE_DEBRIS:
        case SE_SMOKE_DEBRIS:
        case SE_SPARKS:
        case SE_SUB_EXPLOSION:
        case SE_BL_DEBRIS_SMOKE:
        case SE_FIRE_DEBRIS_SMOKE:
            // Set roll to a random value
            particle_ptr->roll = ((float)rand() / (float)RAND_MAX) * TWOPI;
            
            // Accounts for the varience in speed of particles
            particle_ptr->dir = start_dir * float(((3.0 * start_speed) / 4.0) + (start_speed / 2) * 
                (float(rand()) / RAND_MAX));

            // Accounts for size varience of particles
            particle_ptr->size = (((3.0 * start_size) / 4.0) + (start_size / 2) * 
                (float(rand()) / RAND_MAX));

            // Set to random texture number based on available slots
            particle_ptr->tex_num = rand() % tex_frame_count;
            break;

        case SE_RAIN:
        case SE_SNOW:
            // Use dispersion to determine emission distance from origin
            particle_ptr->pos[0] = ((float)rand() / (float)RAND_MAX) * dispersion_area - dispersion_area/2;
            particle_ptr->pos[1] = 5;
            particle_ptr->pos[2] = ((float)rand() / (float)RAND_MAX) * dispersion_area - dispersion_area/2;

            // Set roll to a random value
            particle_ptr->roll = ((float)rand() / (float)RAND_MAX) * 2 * TWOPI - TWOPI;

            // Set to random texture number based on available slots
            particle_ptr->tex_num = rand() % tex_frame_count;
            break;
            
        default:
            break;
    }

    // Account for dispersion factor
    if(dispersion != 0.0)
    {
        particle_ptr->dir.convertTo(CS_SPHERICAL);
        particle_ptr->dir[1] += (((float)rand() / (float)RAND_MAX) * dispersion) - (dispersion / 2.0);
        particle_ptr->dir[2] += (((float)rand() / (float)RAND_MAX) * dispersion) - (dispersion / 2.0);
        particle_ptr->dir.convertTo(CS_CARTESIAN);
    }
}

/*******************************************************************************
    function    :   effect::getClosestAndFarthest
    arguments   :   closest    - Closest distance from the camera
                    farthest   - Farthest distance from the camera
    purpose     :   Saves the values of the closest and farthest particles from
                    the camera.
    notes       :   1) Particles are placed in buckets  based on there age.
*******************************************************************************/
void effect::getClosestAndFarthest( float &closest, float &farthest )
{
    particle* curr = pl_head;
    kVector temp;
    // Go through buckets and find the particle that is the farthest 
    // away from the camera and the particle that is closest to the camera
    // We do not consider particles that are being culled
    while( curr != NULL )
    {
        // Calculate distance from camera
        temp = curr->pos - camera.getCamPosV();
        curr->distance_from_camera = sqrt( temp[0] * temp[0] + 
                                           temp[1] * temp[1] + 
                                           temp[2] * temp[2] );

        // Check to see if closest or farthest away from camera
        if( (curr->distance_from_camera < closest) && (draw ==  true) )
            closest = curr->distance_from_camera;
            
        if( (curr->distance_from_camera > farthest) && (draw ==  true) )
            farthest = curr->distance_from_camera;
            
        curr = curr->next;
    }
}

/*******************************************************************************
    function    :   effect::sort_particles
    arguments   :   <none>
    purpose     :   Places each particle in an effect into a bucket. The display
                    function can choose to read the particles from the buckets
                    or from the normal list.  
    notes       :   1) Particles are placed in buckets  based on there age.
*******************************************************************************/
void effect::sortIntoBuckets()
{
    // Create current pointer that is used to traverse the old list
    particle* curr = pl_head;
  
    // Temp distance variable used in inserting particles in buckets
    float perc_distance;
    
    // Place each particle into the correct bucket
    while( curr != NULL )
    {
        // Don't insert particles your not going to draw
        if( draw == false )
        {
            curr = curr->next;
            continue;
        }
        
        perc_distance = curr->distance_from_camera - effects.closest;
        perc_distance = perc_distance / (effects.farthest - effects.closest);
       
        perc_distance *= (SE_NUM_BUCKETS - 1);
        
        if( perc_distance < 0 )
            perc_distance = 0;
        else if( perc_distance > (SE_NUM_BUCKETS - 1)  )
            perc_distance = (SE_NUM_BUCKETS - 1);

        // Place particle at the begining of bucket
        curr->next_in_bucket = effects.buckets[(int)perc_distance];
        effects.buckets[(int)perc_distance] = curr;
        curr = curr->next;
    }
}

/*******************************************************************************
    function    :   effect::update
    arguments   :   deltaT - Time elapsed (relative) since last update call
    purpose     :   Updates effect system and all particles contained within.
    notes       :   <none>
*******************************************************************************/
void effect::update(float deltaT)
{
    kVector temp;
    float temp_array[3];

    particle* curr;
    particle* prev;
    
    if(effect_done)             // Do not update if effect is finished
        return;
    
    // Phase 1: Handle emissions of any new particles
    if(emit_count != 0)
    {
        if(emit_rate == 0.0)    // Emit all particles at once
        {
            for(; emit_count > 0; emit_count--)
                emit_particle();
            emit_count = 0;
        }
        else if( (run_time > effect_life) && (effect_life != -1) )
            emit_count = 0;
        else                    // Emit particles based on emit_timer countdown
        {
            emit_timer -= deltaT;
            
            // Emit particles differently for dispenser smoke
            if( (effect_type == SE_WH_DISPENSER_SMOKE) && (emit_timer < 0.0))
            {
                emit_particle();
                emit_count--;

                // If Dispenser smoke is at the beginning or end of effect life
                // speed up or slow down emit rate
                if( (run_time / effect_life) < 0.15)
                    emit_timer = (2.0 * emit_rate) - run_time * ( -2.0 * emit_rate ) / (effect_life * 0.15);
                else if( (run_time / effect_life) > 0.85)
                    emit_timer = (2.0 * emit_rate) - (effect_life - run_time) * ( 2.0 * emit_rate ) / (effect_life * 0.15);
                else
                    emit_timer = emit_rate;
            }

            // All other systems emit here
            else
            {
                while(emit_timer <= 0.0)   // Emit new particle when timer hits zero
                {
                    emit_particle();
                    emit_count--;

                    emit_timer += emit_rate;
                    if(emit_timer >= 0.0)
                        emit_timer = emit_rate;
                    
                }
            }
        }
    }
    else if( (pl_head == NULL) && (effect_type != SE_DUST_TRAIL) )        // Determine if effect is finished
    {
        effect_done = true;
        return;
    }
    
    // Phase 2: Handle updation of effect system (on a per effect basis)
    run_time += deltaT;     // Update the running time
    switch(effect_type)
    {
        case SE_RAIN:
            // Detect camera movement and adjust
            // If x or y change, move all particles in the opposite direction
            // And rap around particles that jump outside of the dispersion
            // Area
            temp = effects.prevCamPos - camera.getCamPosV();

            // Adjust for change in x and z
            if( (temp[0] != 0.0) || (temp[2] != 0.0) )
            {
                curr = pl_head;
                while(curr)
                {
                    curr->pos[0] += temp[0];
                    curr->pos[2] += temp[2];

                    curr = curr->next;
                }
            }
            effects.prevCamPos = camera.getCamPosV();
            break;
        case SE_SNOW:
            // Detect camera movement and adjust
            // Ifx or y change, move all particles in the opposite direction
            // And rap around particles that jump outside of the dispersion
            // Area
            temp = effects.prevCamPos - camera.getCamPosV();

            // Adjust for change in x and z
            if( (temp[0] != 0.0) || (temp[2] != 0.0) )
            {
                curr = pl_head;
                while(curr)
                {
                    curr->pos[0] += temp[0];
                    
                    if(curr->pos[0] > (dispersion_area/2))
                        curr->pos[0] = -dispersion_area/2;
                    if(curr->pos[0] < (dispersion_area/-2))
                        curr->pos[0] = dispersion_area/2;

                    curr->pos[2] += temp[2];
                    if(curr->pos[2] > (dispersion_area/2))
                        curr->pos[2] = -dispersion_area/2;
                    if(curr->pos[2] < (dispersion_area/-2))
                        curr->pos[2] = dispersion_area/2;

                    curr = curr->next;
                }
            }

            // Adjust for change in y
            if( temp[1] != 0.0)
            {
                curr = pl_head;
                while(curr)
                {
                    curr->pos[1] += temp[1];
                    curr = curr->next;
                } 
            }

            effects.prevCamPos = camera.getCamPosV();
            break;

        case SE_FIRE_DEBRIS:
            curr = pl_head;
            while(curr)
            {
                while(1)
                {
                    if(curr->last_spawn <  0.0)
                    {
                        // Add a smoke graphic to each piece of flying
                        // Debris to give it a nice tracer tail.
                        effects.addEffect(SE_FIRE_DEBRIS_SMOKE, curr->pos, 0.5 + curr->time_left / curr->life_time);
                        curr->last_spawn += 0.003;
                    }
                    else
                    {
                        curr->last_spawn -= deltaT;
                        break;
                    }
                }
                curr = curr->next;
            }
            break;

        case SE_SMOKE_DEBRIS:
            curr = pl_head;
            while(curr)
            {
                while(1)
                {
                    if(curr->last_spawn <  0.0)
                    {
                        // Add a smoke graphic to each piece of flying
                        // debris to give it a nice tracer tail.
                        effects.addEffect(SE_BL_DEBRIS_SMOKE, curr->pos, mod);
                        if(curr->time_left < 0.5)
                            curr->last_spawn += 0.02 + 0.03 * (1 - curr->time_left / (curr->life_time));
                        else
                            curr->last_spawn += 0.02;

                    }
                    else
                    {
                        curr->last_spawn -= deltaT;
                        break;
                    }
                }
                curr = curr->next;
            }
            break;

        default:
            break;
    }
    
    // Phase 3: Handle updation of particles
    curr = pl_head;
    prev = NULL;
    
    draw = false;   // Set culling to false until a particle falls in view
    
    while(curr)
    {
        // Check for camera culling, if any particle is in view then the system
        // must be entirely drawn out.
        if(effect_type == SE_RAIN || effect_type == SE_SNOW )
        {
            temp_array[0] = camera.getCamPos()[0] + curr->pos()[0];
            temp_array[1] = camera.getCamPos()[1] + curr->pos()[1];
            temp_array[2] = camera.getCamPos()[2] + curr->pos()[2];

            if(!draw && camera.sphereInView(temp_array, curr->size))
                draw = true;
        }
        else
        {
            if(!draw && camera.sphereInView(curr->pos(), curr->size))
                draw = true;
        }
            
        // Update the time left for this particle
        curr->time_left -= deltaT;
        
        if(curr->time_left > 0.0)
        {
            // Apply rate values
            curr->pos += system_dir * deltaT;       // System movement
            curr->dir[1] += gravity * deltaT;       // Gravity
            curr->pos += curr->dir * deltaT;        // Direction -> Position
            curr->roll += roll_speed * deltaT;      // Roll
            
            // Apply resizing
            if(resize_rate != 0.0)
                curr->size += resize_rate * deltaT;
            
            // Apply texture frame animation
            if(tex_cycle_time != 0.0)
            {
                curr->tex_cycle -= deltaT;          // Update texture cycle
                
                if(curr->tex_cycle <= 0.0)          // Check for texture cycle
                {
                    // Cycle texture
                    curr->tex_num++;
                    curr->tex_cycle = tex_cycle_time;
                    
                    if(curr->tex_num >= tex_frame_count)
                    {
                        if(tex_cycle_repeat)        // Reset texture to initial
                            curr->tex_num = 0;
                        else                        // Remove particle
                        {
                            if(prev == NULL)
                            {
                                pl_head = pl_head->next;
                                delete curr;
                                curr = pl_head;
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
                    }
                }
            }
        }
        else    // Remove particle
        {
            if(prev == NULL)
            {
                pl_head = pl_head->next;
                delete curr;
                curr = pl_head;
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

/*******************************************************************************
    function    :   se_module::se_module
    arguments   :   <none>
    purpose     :   Constructor.
    notes       :   <none>
*******************************************************************************/
se_module::se_module()
{
    el_head = NULL;
    
    // Initialize all buckets to NULL
    for( int i = 0; i < SE_NUM_BUCKETS; i++ )
        buckets[i] = NULL;
}

/*******************************************************************************
    function    :   se_module::~se_module
    arguments   :   <none>
    purpose     :   Deconstructor.
    notes       :   <none>
*******************************************************************************/
se_module::~se_module()
{
    effect* curr = el_head;
    effect* temp;
    
    // Kill effects list
    while(curr)
    {
        temp = curr->next;
        delete curr;
        curr = temp;
    }
}

/*******************************************************************************
    function    :   se_module::loadEffects
    arguments   :   <none>
    purpose     :   Pre-loads all textures used for special effects.
    notes       :   <none>
*******************************************************************************/
void se_module::loadEffects()
{
    
    // Explosions
    textures.loadTexture("FX/exp.png");
    textures.loadTexture("FX/base_exp.png");
    textures.loadTexture("FX/sub_exp.png");

    // Fire
    textures.loadTexture("FX/fire.png");

    // Fountains
    textures.loadTexture("FX/dirt.png");
    textures.loadTexture("FX/shrapnel.png");
    textures.loadTexture("FX/debris.png");
    textures.loadTexture("FX/spark.png");
    
    // Smoke
    textures.loadTexture("FX/smoke_black.png");
    textures.loadTexture("FX/smoke_brown.png");
    textures.loadTexture("FX/smoke_white.png");
    textures.loadTexture("FX/smoke_light_brown.png");
    textures.loadTexture("FX/smoke_light_white.png");

    // Other
    textures.loadTexture("FX/snow.png");
    textures.loadTexture("FX/rain.png");
    textures.loadTexture("FX/mg_firing.png");
}

/*******************************************************************************
    function    :   se_module::addEffect
    arguments   :   effectType - Type of effect to add
                    position - Position of system to emit particles at
                    direction - Direction of emiting particles
                    modifier - Modifies values on a per-effect basis.
                    systemDirection - Direction of overall system (if it moves)
    purpose     :   Adds and registers an effect into the SE module, returns
                    an ID number that may be referenced on later (except for
                    effects which spawn effects such as SE_BIG_EXPLOSION and
                    SE_CANNON_FIRING).
    notes       :   Modifier values work on a per-effect basis, of which
                    control a specific value, details of which are:
                        Explosion Effects: Controls size of explosion (based on
                            kg of explosive content).
                        Fountain Effects: Controls how many particles to emit.
                        Smoke Effects: Controls how many smoke particles to
                            emit (except for billowing smoke which doesn't use
                            any modifier values).
                        Fire Effects: Not currently used.
                        Weather Effects: Not currently used.
                    Note: An emission number of -1 will make the system last
                        for an infinite amount of time. This is only for
                        modifiers which reflect the # of emissions.
*******************************************************************************/
int se_module::addEffect(int effectType, kVector position, kVector direction,
    float modifier, kVector systemDirection)
{
    effect* effect_ptr;
 
    // Unknown effect type recieved from script
    if( effectType == -1 )
        return 0;
           
    // Create effect
    effect_ptr = new effect(effectType, position, direction, modifier,
        systemDirection);
    
    if(effect_ptr->isFinished())
        delete effect_ptr;          // Handle spawn-off effects
    else
    {
        // Set new head pointer
        effect_ptr->next = el_head;
        el_head = effect_ptr;
        return (int)effect_ptr;
    }
    
    return -1;
}

/*******************************************************************************
    function    :   se_module::stopEffect
    arguments   :   id - ID number returned from addEffect
    purpose     :   Stops an effect to emit any further particles. Does not
                    completely kill the effect (e.g. not immediate).
    notes       :   <none>
*******************************************************************************/
void se_module::stopEffect(int id)
{
    effect* effect_ptr;
    
    effect_ptr = (effect*)id;
    
    if(effect_ptr)
        effect_ptr->stopEffect();
}

/*******************************************************************************
    function    :   se_module::killEffect
    arguments   :   id - ID number returned from addEffect
    purpose     :   Kills an entire effect from existance, immediately removing
                    all particles and ripping the system from the list.
    notes       :   <none>
*******************************************************************************/
void se_module::killEffect(int id)
{
    effect* curr = el_head;
    effect* prev = NULL;
    
    while(curr)
    {
        // Check to see if this effect is the one to kill off
        if(curr == (effect*)id)
        {
            // Kill off effect, maintaining linked list
            if(prev == NULL)
                el_head = el_head->next;
            else
                prev->next = curr->next;
            delete curr;
            return;     // Done here
        }
        
        prev = curr;
        curr = curr->next;
    }
}

/*******************************************************************************
    function    :   se_module::setEffectPosition
    arguments   :   id - ID number returned from addEffect
                    position - New starting postion of effect for emissions.
    purpose     :   Sets a new starting position for the emission of particles.
    notes       :   <none>
*******************************************************************************/
void se_module::setEffectPosition(int id, kVector position)
{
    effect* effect_ptr;
    
    effect_ptr = (effect*)id;
    
    if(effect_ptr)
        effect_ptr->setStartPosition(position);
}

/*******************************************************************************
    function    :   se_module::setEffectDirection
    arguments   :   id - ID number returned from addEffect
                    direction - New direction for emissions.
    purpose     :   Sets a new direction of emissions for particles.
    notes       :   <none>
*******************************************************************************/
void se_module::setEffectDirection(int id, kVector direction)
{
    effect* effect_ptr;
    
    effect_ptr = (effect*)id;
    
    if(effect_ptr)
        effect_ptr->setStartDirection(direction);
}

/*******************************************************************************
    function    :   se_module::update
    arguments   :   deltaT - Time elapsed since last update
    purpose     :   Updates the special effects module's effect listing.
    notes       :   <none>
*******************************************************************************/
void se_module::update(float deltaT)
{

    effect* curr = el_head;
    effect* prev = NULL;
    
    // Reset all buckets to NULL
    for( int i = 0; i < SE_NUM_BUCKETS; i++ )
        buckets[i] = NULL;
    
    set_bucket_parameters();
              
    while(curr)
    {
        // Update effect
        curr->update(deltaT);
                                  
        // Place all particles into global buckets
        curr->sortIntoBuckets();
                        
        // Check for finish/removal
        if(curr->isFinished())
        {
            // Inform sound_effect_handler that effect is over
            script.sound_effect_handler.effectDead( (int)curr );
            
            // Remove effect, maintain linked list
            if(prev == NULL)
            {
                el_head = el_head->next;
                delete curr;
                curr = el_head;
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

/*******************************************************************************
    function    :   se_module::display
    arguments   :   <none>
    purpose     :   Displays all currently running effects.
    notes       :   <none>
*******************************************************************************/
void se_module::display()
{
    particle*  curr;
    kVector orientate;
    kVector temp_vector;
    float temp_size;
    float expand_rate;
    float curr_slice;           // Current image in animation
    
    // Enable textures and color material mapping
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_COLOR_MATERIAL);
    
    

    for( int i = SE_NUM_BUCKETS - 1; i >= 0 ; i-- )
    {    
        curr = buckets[i];
        while( curr != NULL )
        {        
            glAlphaFunc(GL_GEQUAL, 0.1);
        
            // Culling check
            if(!curr->parent->draw)
            {
                curr = curr->next;
                continue;
            }
            
            // Perform type based display routines
            switch(curr->parent->effect_type)
            {
                case SE_DEBRIS:
                case SE_SMOKE_DEBRIS:
                case SE_FIRE_DEBRIS:
                case SE_SPARKS:
                case SE_MG_DIRT:
                case SE_LARGE_DIRT:
        
                    orientate = vectorIn(curr->pos - camera.getCamPosV(), CS_SPHERICAL);
                    orientate[1] *= radToDeg;
                    orientate[2] *= radToDeg;
        
                    glAlphaFunc(GL_GEQUAL, ALPHA_PASS);
                    glColor4f(1.0, 1.0, 1.0, 1.0);

                    // Draw particle
                    glPushMatrix();
                    glTranslatef(curr->pos[0], curr->pos[1], curr->pos[2]);
                    glRotatef(orientate[2], 0.0, 1.0, 0.0);
                    glRotatef(orientate[1], 1.0, 0.0, 0.0);
                    glRotatef(curr->roll * radToDeg, 0.0, 1.0, 0.0);
                    glScalef(curr->size, curr->size, curr->size);
                    
                    // Since all the frames are stored in one image we must break apart
                    // the image and adjust the uv mapping appropriatly
                    curr_slice = curr->parent->image_slice * curr->tex_num;
                    glBindTexture(GL_TEXTURE_2D, curr->parent->tex_frame);
                    glBegin(GL_QUADS);
                        glNormal3f(0.0, 1.0, 0.0);
                        glTexCoord2f( curr_slice , 0.0);
                        glVertex3f(-0.5, 0.0, 0.5);
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 0.0);
                        glVertex3f(0.5, 0.0, 0.5);
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 1.0);
                        glVertex3f(0.5, 0.0, -0.5);
                        glTexCoord2f(curr_slice, 1.0);
                        glVertex3f(-0.5, 0.0, -0.5);
                    glEnd();
                    glPopMatrix();
                    break;
        
                case SE_FIRING_BLAST:
                    orientate = vectorIn(curr->pos - camera.getCamPosV(), CS_SPHERICAL);
                    orientate[1] *= radToDeg;
                    orientate[2] *= radToDeg;

                    if(curr->time_left > 0.5 * curr->parent->life_time)
                    {
                        glAlphaFunc(GL_GEQUAL, ALPHA_PASS);
                        glColor4f(1.0, 1.0, 1.0, 1.0);
                    }
                    else
                    {
                        glAlphaFunc(GL_GEQUAL, ALPHA_PASS * curr->time_left / (0.5 * curr->parent->life_time));
                        glColor4f(1.0, 1.0, 1.0, curr->time_left / (0.5 * curr->parent->life_time));
                    }
                    
                    // Draw particle
                    glPushMatrix();
                    glTranslatef(curr->pos[0], curr->pos[1], curr->pos[2]);
                    glRotatef(orientate[2], 0.0, 1.0, 0.0);
                    glRotatef(orientate[1], 1.0, 0.0, 0.0);
                    glRotatef(curr->roll * radToDeg, 0.0, 1.0, 0.0);
                    glScalef(curr->size, curr->size, curr->size);
                    
                    // Since all the frames are stored in one image we must break apart
                    // the image and adjust the uv mapping appropriatly
                    curr_slice = curr->parent->image_slice * curr->tex_num;
                    glBindTexture(GL_TEXTURE_2D, curr->parent->tex_frame);
                    glBegin(GL_QUADS);
                        glNormal3f(0.0, 1.0, 0.0);
                        glTexCoord2f( curr_slice , 0.0);
                        glVertex3f(-0.5, 0.0, 0.5);
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 0.0);
                        glVertex3f(0.5, 0.0, 0.5);
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 1.0);
                        glVertex3f(0.5, 0.0, -0.5);
                        glTexCoord2f(curr_slice, 1.0);
                        glVertex3f(-0.5, 0.0, -0.5);
                    glEnd();
                    glPopMatrix();
                    break;
            
                case SE_MG_FIRING:
                    orientate = vectorIn(curr->pos - camera.getCamPosV(), CS_SPHERICAL);
                    orientate[1] *= radToDeg;
                    orientate[2] *= radToDeg;

                    if(curr->time_left > 0.5 * curr->parent->life_time)
                    {
                        glAlphaFunc(GL_GEQUAL, ALPHA_PASS);
                        glColor4f(1.0, 1.0, 1.0, 1.0);
                    }
                    else
                    {
                        glAlphaFunc(GL_GEQUAL, ALPHA_PASS * curr->time_left / (0.5 * curr->parent->life_time));
                        glColor4f(1.0, 1.0, 1.0, curr->time_left / (0.5 * curr->parent->life_time));
                    }
                    
                    // Draw particle
                    glPushMatrix();
                    glTranslatef(curr->pos[0], curr->pos[1], curr->pos[2]);
                    glRotatef(orientate[2], 0.0, 1.0, 0.0);
                    glRotatef(orientate[1], 1.0, 0.0, 0.0);
                    glRotatef(curr->roll * radToDeg, 0.0, 1.0, 0.0);
                    glScalef(curr->size, curr->size, curr->size);
                    
                    // Since all the frames are stored in one image we must break apart
                    // the image and adjust the uv mapping appropriatly
                    curr_slice = curr->parent->image_slice * curr->tex_num;
                    glBindTexture(GL_TEXTURE_2D, curr->parent->tex_frame);
                    glBegin(GL_QUADS);
                        glNormal3f(0.0, 1.0, 0.0);
                        glTexCoord2f( curr_slice , 0.0);
                        glVertex3f(-0.5, 0.0, 0.5);
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 0.0);
                        glVertex3f(0.5, 0.0, 0.5);
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 1.0);
                        glVertex3f(0.5, 0.0, -0.5);
                        glTexCoord2f(curr_slice, 1.0);
                        glVertex3f(-0.5, 0.0, -0.5);
                    glEnd();
                    glPopMatrix();
                    break;
        
                case SE_BASE_EXPLOSION:
                    glAlphaFunc(GL_GEQUAL, 0.1);
    
                    if(curr->time_left > 0.5 * curr->parent->life_time)
                    {
                        glColor4f(1.0, 1.0, 1.0, 1.0);
                    }
                    else
                    {
                        glColor4f(1.0, 1.0, 1.0, curr->time_left / (0.5 * curr->parent->life_time));
                    }
                    
                    // Draw particle
                    glPushMatrix();
                    glTranslatef(curr->pos[0], 0.3 + map.getHeight(curr->pos[0], curr->pos[2]), curr->pos[2]);
                    glScalef(curr->size, curr->size, curr->size);
                    
                    // Since all the frames are stored in one image we must break apart
                    // the image and adjust the uv mapping appropriatly
                    curr_slice = curr->parent->image_slice * curr->tex_num;
                    glBindTexture(GL_TEXTURE_2D, curr->parent->tex_frame);
                    glBegin(GL_QUADS);
                        glNormal3f(0.0, 1.0, 0.0);
                        glTexCoord2f( curr_slice , 0.0);
                        glVertex3f(-0.5, 0.0, 0.5);
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 0.0);
                        glVertex3f(0.5, 0.0, 0.5);
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 1.0);
                        glVertex3f(0.5, 0.0, -0.5);
                        glTexCoord2f(curr_slice, 1.0);
                        glVertex3f(-0.5, 0.0, -0.5);
                    glEnd();
                    glPopMatrix();
                    break;
        
                case SE_MG_GROUND:
                    orientate = vectorIn(curr->pos - camera.getCamPosV(), CS_SPHERICAL);
                    orientate[1] *= radToDeg;
                    orientate[2] *= radToDeg;
        
                    glAlphaFunc(GL_GEQUAL, 0.1);
                    glColor4f(1.0, 1.0, 1.0, 1.0);

                    // Draw particle
                    glPushMatrix();
                    glTranslatef(curr->pos[0], curr->pos[1], curr->pos[2]);
                    glRotatef(orientate[2], 0.0, 1.0, 0.0);
                    glRotatef(90, 1.0, 0.0, 0.0);
                    glRotatef(curr->roll * radToDeg, 0.0, 1.0, 0.0);
                    glScalef(curr->size, curr->size, curr->size);
                    
                    // Since all the frames are stored in one image we must break apart
                    // the image and adjust the uv mapping appropriatly
                    curr_slice = curr->parent->image_slice * curr->tex_num;
                    glBindTexture(GL_TEXTURE_2D, curr->parent->tex_frame);
                    glBegin(GL_QUADS);
                        glNormal3f(0.0, 1.0, 0.0);
                        glTexCoord2f( curr_slice , 0.0);
                        glVertex3f(-0.125, 0.0, 0.5);
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 0.0);
                        glVertex3f(0.125, 0.0, 0.5);
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 1.0);
                        glVertex3f(0.125, 0.0, -0.5);
                        glTexCoord2f(curr_slice, 1.0);
                        glVertex3f(-0.125, 0.0, -0.5);
                    glEnd();
                    glPopMatrix();
                    break;
        
                case SE_FIRE:
                    // Display objects from special effects
                    orientate = vectorIn(curr->pos - camera.getCamPosV(), CS_SPHERICAL);
                    orientate[1] *= radToDeg;
                    orientate[2] *= radToDeg;
                    
                    glAlphaFunc(GL_GEQUAL, 0.08);
                        
                    if(curr->time_left > 0.8 * curr->parent->life_time)
                    {
                        glColor4f(1.0, 1.0, 1.0, 1.0);
                    }
                    else
                    {
                        glColor4f(1.0, 1.0, 1.0, curr->time_left / (0.8 * curr->parent->life_time));
                    }
    
                    
                    // Draw particle
                    glPushMatrix();
                    glTranslatef(curr->pos[0], curr->pos[1], curr->pos[2]);
                    glRotatef(orientate[2], 0.0, 1.0, 0.0);
                    glRotatef(orientate[1], 1.0, 0.0, 0.0);
                    glRotatef(curr->roll * radToDeg, 0.0, 1.0, 0.0);
    
                    expand_rate = 0.05;
    
                    if(curr->time_left > (curr->parent->life_time - expand_rate * curr->parent->life_time))
                        temp_size = curr->size * ( 1.0 - curr->time_left / curr->parent->life_time)/expand_rate;
                    else if(curr->time_left > expand_rate * curr->parent->life_time)
                        temp_size = curr->size;
                    else
                        temp_size = curr->size * curr->time_left / (expand_rate * curr->parent->life_time);
                    glScalef(temp_size, temp_size, temp_size);
    
                    // Since all the frames are stored in one image we must break apart
                    // the image and adjust the uv mapping appropriatly
                    curr_slice = curr->parent->image_slice * curr->tex_num;
                    glBindTexture(GL_TEXTURE_2D, curr->parent->tex_frame);
                    glBegin(GL_QUADS);
                        glNormal3f(0.0, 1.0, 0.0);
                        glTexCoord2f( curr_slice , 0.0);
                        glVertex3f(-0.5, 0.0, 0.5);
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 0.0);
                        glVertex3f(0.5, 0.0, 0.5);
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 1.0);
                        glVertex3f(0.5, 0.0, -0.5);
                        glTexCoord2f(curr_slice, 1.0);
                        glVertex3f(-0.5, 0.0, -0.5);
                    glEnd();
                    glPopMatrix();
                    break;
        
                case SE_EXPLOSION:
                case SE_BL_DEBRIS_SMOKE:
                case SE_FIRE_DEBRIS_SMOKE:
                    // Display objects from special effects
                    
                    orientate = vectorIn(curr->pos - camera.getCamPosV(), CS_SPHERICAL);
                    orientate[1] *= radToDeg;
                    orientate[2] *= radToDeg;
                    
                    if(curr->time_left > 0.8 * curr->parent->life_time)
                    {
                        glAlphaFunc(GL_GEQUAL, ALPHA_PASS);
                        glColor4f(1.0, 1.0, 1.0, 1.0);
                    }
                    else
                    {
                        glAlphaFunc(GL_GEQUAL, ALPHA_PASS * curr->time_left / (0.8 * curr->parent->life_time));
                        glColor4f(1.0, 1.0, 1.0, curr->time_left / (0.8 * curr->parent->life_time));
                    }
                    if( curr->parent->effect_type == SE_FIRE )
                        glAlphaFunc(GL_GEQUAL, 0.08);

                    
                    // Draw particle
                    glPushMatrix();
                    glTranslatef(curr->pos[0], curr->pos[1], curr->pos[2]);
                    glRotatef(orientate[2], 0.0, 1.0, 0.0);
                    glRotatef(orientate[1], 1.0, 0.0, 0.0);
                    glRotatef(curr->roll * radToDeg, 0.0, 1.0, 0.0);

                    expand_rate = 0.05;

                    if(curr->time_left > (curr->parent->life_time - expand_rate * curr->parent->life_time))
                        temp_size = curr->size * ( 1.0 - curr->time_left / curr->parent->life_time)/expand_rate;
                    else if(curr->time_left > expand_rate * curr->parent->life_time)
                        temp_size = curr->size;
                    else
                        temp_size = curr->size * curr->time_left / (expand_rate * curr->parent->life_time);
                    glScalef(temp_size, temp_size, temp_size);

                    // Since all the frames are stored in one image we must break apart
                    // the image and adjust the uv mapping appropriatly
                    curr_slice = curr->parent->image_slice * curr->tex_num;
                    glBindTexture(GL_TEXTURE_2D, curr->parent->tex_frame);
                    glBegin(GL_QUADS);
                        glNormal3f(0.0, 1.0, 0.0);
                        glTexCoord2f( curr_slice , 0.0);
                        glVertex3f(-0.5, 0.0, 0.5);
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 0.0);
                        glVertex3f(0.5, 0.0, 0.5);
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 1.0);
                        glVertex3f(0.5, 0.0, -0.5);
                        glTexCoord2f(curr_slice, 1.0);
                        glVertex3f(-0.5, 0.0, -0.5);
                    glEnd();
                    glPopMatrix();
                    break;
        
                case SE_DIRT:
                case SE_SHRAPNEL:
                    orientate = vectorIn(curr->pos - camera.getCamPosV(), CS_SPHERICAL);
                    orientate[1] *= radToDeg;
                    orientate[2] *= radToDeg;
        
                    if(curr->time_left > 0.75)
                    {
                        glAlphaFunc(GL_GEQUAL, ALPHA_PASS * 0.75);
                        glColor4f(1.0, 1.0, 1.0, 0.75);
                    }
                    else
                    {
                        glAlphaFunc(GL_GEQUAL, ALPHA_PASS * curr->time_left);
                        glColor4f(1.0, 1.0, 1.0, curr->time_left);
                    }
                    
                    // Draw particle
                    glPushMatrix();
                    glTranslatef(curr->pos[0], curr->pos[1], curr->pos[2]);
                    glRotatef(orientate[2], 0.0, 1.0, 0.0);
                    glRotatef(orientate[1], 1.0, 0.0, 0.0);
                    glRotatef(curr->roll * radToDeg, 0.0, 1.0, 0.0);
                    glScalef(curr->size, curr->size, curr->size);

                    // Since all the frames are stored in one image we must break apart
                    // the image and adjust the uv mapping appropriatly
                    curr_slice = curr->parent->image_slice * curr->tex_num;
                    glBindTexture(GL_TEXTURE_2D, curr->parent->tex_frame);
                    glBegin(GL_QUADS);
                        glNormal3f(0.0, 1.0, 0.0);
                        glTexCoord2f( curr_slice , 0.0);
                        glVertex3f(-0.5, 0.0, 0.5);
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 0.0);
                        glVertex3f(0.5, 0.0, 0.5);
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 1.0);
                        glVertex3f(0.5, 0.0, -0.5);
                        glTexCoord2f(curr_slice, 1.0);
                        glVertex3f(-0.5, 0.0, -0.5);
                    glEnd();
                    glPopMatrix();
                    break;
                
                case SE_MG_GROUND_SMOKE:
                case SE_FIRING_SMOKE:
                case SE_WH_BILLOWING_SMOKE:
                case SE_BK_BILLOWING_SMOKE:
                case SE_WH_DISPENSER_SMOKE:
                
                    orientate = vectorIn(curr->pos - camera.getCamPosV(), CS_SPHERICAL);
                    
                    if( curr->parent->effect_type == SE_WH_DISPENSER_SMOKE )
                        glAlphaFunc(GL_GEQUAL, 0.0);
                    else
                        glAlphaFunc(GL_GEQUAL, ALPHA_PASS * curr->time_left / curr->parent->life_time);

                    glColor4f(1.0, 1.0, 1.0, curr->time_left / curr->parent->life_time);
                    
                    // Display particle
                    glPushMatrix();
                    glTranslatef(curr->pos[0], curr->pos[1], curr->pos[2]);
                    glRotatef(orientate[2] * radToDeg, 0.0, 1.0, 0.0);
                    glRotatef(orientate[1] * radToDeg, 1.0, 0.0, 0.0);
                    glRotatef(curr->roll * radToDeg, 0.0, 1.0, 0.0);
                    glScalef(curr->size, curr->size, curr->size);
                    
                    // Since all the frames are stored in one image we must break apart
                    // the image and adjust the uv mapping appropriatly
                    curr_slice = curr->parent->image_slice * curr->tex_num;
                    glBindTexture(GL_TEXTURE_2D, curr->parent->tex_frame);
                    glBegin(GL_QUADS);
                        glNormal3f(0.0, 1.0, 0.0);
                        glTexCoord2f( curr_slice , 0.0);
                        glVertex3f(-0.5, 0.0, 0.5);
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 0.0);
                        glVertex3f(0.5, 0.0, 0.5);
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 1.0);
                        glVertex3f(0.5, 0.0, -0.5);
                        glTexCoord2f(curr_slice, 1.0);
                        glVertex3f(-0.5, 0.0, -0.5);
                    glEnd();
                    glPopMatrix();
                    break;
                    
                case SE_QF_WH_SMOKE:
                case SE_QF_BR_SMOKE:
                case SE_DUST_CLOUD:
                case SE_DUST_TRAIL:
        
                    orientate = vectorIn(curr->pos - camera.getCamPosV(), CS_SPHERICAL);
                    
                    glAlphaFunc(GL_GEQUAL, ALPHA_PASS * 0.35 * (curr->time_left / curr->parent->life_time));
                    glColor4f(1.0, 1.0, 1.0, 0.35 * (curr->time_left / curr->parent->life_time));
                    
                    // Display particle
                    glPushMatrix();
                    glTranslatef(curr->pos[0], curr->pos[1], curr->pos[2]);
                    glRotatef(orientate[2] * radToDeg, 0.0, 1.0, 0.0);
                    glRotatef(orientate[1] * radToDeg, 1.0, 0.0, 0.0);
                    glRotatef(curr->roll * radToDeg, 0.0, 1.0, 0.0);
                    glScalef(curr->size, curr->size, curr->size);
                    
                    // Since all the frames are stored in one image we must break apart
                    // the image and adjust the uv mapping appropriatly
                    curr_slice = curr->parent->image_slice * curr->tex_num;
                    glBindTexture(GL_TEXTURE_2D, curr->parent->tex_frame);
                    glBegin(GL_QUADS);
                        glNormal3f(0.0, 1.0, 0.0);
                        glTexCoord2f(curr_slice, 0.0);
                        glVertex3f(-0.5, 0.0, 0.5);
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 0.0);
                        glVertex3f(0.5, 0.0, 0.5);
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 1.0);
                        glVertex3f(0.5, 0.0, -0.5);
                        glTexCoord2f(curr_slice, 1.0);
                        glVertex3f(-0.5, 0.0, -0.5);
                    glEnd();
                    glPopMatrix();
                    break;
                    
                case SE_RAIN:
                    temp_vector = camera.getCamPosV();
                    
                    glColor4f(1.0, 1.0, 1.0, 1.0);
                    glAlphaFunc(GL_GEQUAL, 0.1);
        
                    orientate = vectorIn(curr->pos, CS_SPHERICAL);
                    orientate[1] *= radToDeg;
                    orientate[2] *= radToDeg;
                    
                    // Draw particle
                    glPushMatrix();
                    glTranslatef(temp_vector[0] + curr->pos[0], temp_vector[1] + curr->pos[1], temp_vector[2] + curr->pos[2]);
                    glRotatef(orientate[2] + curr->roll * radToDeg, 0.0, 1.0, 0.0);
                    glRotatef(90, 1.0, 0.0, 0.0);
                    
                    glScalef(curr->size, curr->size, 7);
                    
                    // Since all the frames are stored in one image we must break apart
                    // the image and adjust the uv mapping appropriatly
                    curr_slice = curr->parent->image_slice * curr->tex_num;
                    glBindTexture(GL_TEXTURE_2D, curr->parent->tex_frame);
                    glBegin(GL_TRIANGLES);
                        glNormal3f(0.0, 1.0, 0.0);
    
                        glTexCoord2f( curr_slice + curr->parent->image_slice / 2.0, 0.0);
                        glVertex3f(0.0, 0.0, 0.5);
                        
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 1.0);
                        glVertex3f(0.5, 0.0, -0.5);
                        
                        glTexCoord2f(curr_slice, 1.0);
                        glVertex3f(-0.5, 0.0, -0.5);
                    glEnd();
                    glPopMatrix();
                    
                    break;
        
                case SE_SNOW:
                    temp_vector = camera.getCamPosV();
        
                    // Could change alpha based on distance from camera
                    glColor4f(1.0, 1.0, 1.0, 1.0);
                    glAlphaFunc(GL_GEQUAL, ALPHA_PASS);

                    orientate = vectorIn(curr->pos, CS_SPHERICAL);
                    orientate[1] *= radToDeg;
                    orientate[2] *= radToDeg;
                
                    
                    
                    // Draw particle
                    glPushMatrix();
                    glTranslatef(temp_vector[0] + curr->pos[0], temp_vector[1] + curr->pos[1], temp_vector[2] + curr->pos[2]);
                    glRotatef(orientate[2], 0.0, 1.0, 0.0);
                    glRotatef(90, 1.0, 0.0, 0.0);
                    
                    glScalef(curr->size, curr->size, curr->size);
    
                    // Since all the frames are stored in one image we must break apart
                    // the image and adjust the uv mapping appropriatly
                    curr_slice = curr->parent->image_slice * curr->tex_num;
                    glBindTexture(GL_TEXTURE_2D, curr->parent->tex_frame);
                    glBegin(GL_TRIANGLES);
                        glNormal3f(0.0, 1.0, 0.0);
    
                        glTexCoord2f( curr_slice + curr->parent->image_slice / 2.0, 0.0);
                        glVertex3f(0.0, 0.0, 0.5);
    
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 1.0);
                        glVertex3f(0.5, 0.0, -0.5);
    
                        glTexCoord2f(curr_slice, 1.0);
                        glVertex3f(-0.5, 0.0, -0.5);
                    glEnd();
                    glPopMatrix();
                    break;
        
                case SE_SUB_EXPLOSION:
                    orientate = vectorIn(curr->pos - camera.getCamPosV(), CS_SPHERICAL);
                    orientate[1] *= radToDeg;
                    orientate[2] *= radToDeg;
        
                    if(curr->time_left > 0.3 * curr->parent->life_time)
                    {
                        glColor4f(1.0, 1.0, 1.0, 0.7);
                        glAlphaFunc(GL_GEQUAL, ALPHA_PASS * 0.7);
                    }
                    else
                    {
                        glColor4f(1.0, 1.0, 1.0, 0.7 * curr->time_left / (0.3 * curr->parent->life_time));
                        glAlphaFunc(GL_GEQUAL, ALPHA_PASS * 0.7 * curr->time_left / (0.3 * curr->parent->life_time));
                    }

                    // Draw particle
                    glPushMatrix();
                    glTranslatef(curr->pos[0], curr->pos[1], curr->pos[2]);
                    glRotatef(orientate[2], 0.0, 1.0, 0.0);
                    glRotatef(orientate[1], 1.0, 0.0, 0.0);
                    glRotatef(curr->roll * radToDeg, 0.0, 1.0, 0.0);

                    expand_rate = 0.05;

                    if(curr->time_left > (curr->parent->life_time - expand_rate * curr->parent->life_time))
                        temp_size = curr->size * ( 1.0 - curr->time_left / curr->parent->life_time)/expand_rate;
                    else if(curr->time_left > expand_rate * curr->parent->life_time)
                        temp_size = curr->size;
                    else
                        temp_size = curr->size * curr->time_left / (expand_rate * curr->parent->life_time);
                    glScalef(temp_size, temp_size, temp_size);

                    // Since all the frames are stored in one image we must break apart
                    // the image and adjust the uv mapping appropriatly
                    curr_slice = curr->parent->image_slice * curr->tex_num;
                    glBindTexture(GL_TEXTURE_2D, curr->parent->tex_frame);
                    glBegin(GL_QUADS);
                        glNormal3f(0.0, 1.0, 0.0);
                        glTexCoord2f( curr_slice , 0.0);
                        glVertex3f(-0.5, 0.0, 0.5);
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 0.0);
                        glVertex3f(0.5, 0.0, 0.5);
                        glTexCoord2f(curr_slice + curr->parent->image_slice, 1.0);
                        glVertex3f(0.5, 0.0, -0.5);
                        glTexCoord2f(curr_slice, 1.0);
                        glVertex3f(-0.5, 0.0, -0.5);
                    glEnd();
                    glPopMatrix();
                    break;
        
                default:
                    break;
            }
            
            curr = curr->next_in_bucket;
        }
    }
}
        
/*******************************************************************************
    function    :   se_module::set_bucket_parameters
    arguments   :   <none>
    purpose     :   Sets variables farthest and closest.
    notes       :   <none>
*******************************************************************************/      
void se_module::set_bucket_parameters()
{
    effect* curr = el_head;
    farthest = 0;
    closest = 99999999;  // I just chose a high number to start off with
    
    // Go through buckets and find the particle that is the farthest 
    // away from the camera and the particle that is closest to the camera
    // We do not consider particles that are being culled
    while( curr != NULL )
    {
        curr->getClosestAndFarthest( closest, farthest );
        curr = curr->next;
    }
}

