/*******************************************************************************
                        Special Effects - Definition
*******************************************************************************/
#ifndef EFFECTS_H
#define EFFECTS_H
#include "metrics.h"

// Explosion effects
#define SE_EXPLOSION            0       // Base HE explosion
#define SE_BASE_EXPLOSION       1       // Base flash for Big explosion
#define SE_BIG_EXPLOSION        2       // HUGE explosion
#define SE_SUB_EXPLOSION        3       // Sub explosion in HUGE explosion
#define SE_MG_GROUND            4       // Effect for when bullet hits the ground

// Firing effects
#define SE_CANNON_FIRING        10      // Base cannon firing
#define SE_MG_FIRING            11      // MG firing
#define SE_FIRING_BLAST         12      // Cannon blast (ext)
#define SE_FIRING_SMOKE         13      // Cannon smoke (ext)

// Fountain-like effects
#define SE_DIRT                 20      // Dirt fragments fountain
#define SE_MG_DIRT              21      // Dirt used in MG_GROUND effect
#define SE_LARGE_DIRT           22      // Large dirt fragments fountain
#define SE_SHRAPNEL             23      // Shrapnel fragments fountain
#define SE_DEBRIS               24      // Debris fragments fountain
#define SE_FIRE_DEBRIS          25      // Debris fragments fountain that emit smoked
#define SE_SMOKE_DEBRIS         26      // Debris fragments fountain that emit fire
#define SE_SPARKS               27      // Spark fountain
#define SE_FIRE                 28      // Fire fountain

// Smoke/Clouding effects
#define SE_WH_DISPENSER_SMOKE   30      // White smoke from dispenser
#define SE_WH_BILLOWING_SMOKE   31      // White billowing smoke (engine damage)
#define SE_BK_BILLOWING_SMOKE   32      // Black billowing smoke (on fire)
#define SE_QF_WH_SMOKE          33      // Quick fading white smoke
#define SE_QF_BR_SMOKE          34      // Quick fading brown smoke
#define SE_DUST_CLOUD           35      // Dust cloud
#define SE_DUST_TRAIL           36      // Dust emitted from tank tracks
#define SE_BL_DEBRIS_SMOKE      37      // Black smoke left in wake of flying debris
#define SE_FIRE_DEBRIS_SMOKE    38      // Black smoke left in wake of flying fire debris
#define SE_MG_GROUND_SMOKE      39      // Emitted when mg fire hits the ground(ext)

// Weather effects
#define SE_RAIN                 50      // Rain weather effect
#define SE_SNOW                 51      // Snow weather effect

// Frame count
// Since all images in an animation are stored linearly in one image
// File there is only one texture ID for the whole effect. 
// I have left the apply for anyone to simply change SE_MAX_TEX_FRAMES
// To some other number besides one which will allow them to display each
// Frame of an animation from a single image file once again.
#define SE_MAX_TEX_FRAMES       1       // Max frame count for animated effects

// Currently using an extremely high number of buckets, if korps segfaults on load
// try reducing this number
#define SE_NUM_BUCKETS          1001     // Number of buckets used in particle sort

// Class prototype
class effect;

struct particle
{
    kVector pos;                // Position of partilce
    kVector dir;                // Direction of particle
    float roll;                 // Roll of particle
    
    float time_left;            // Time left for particle
    float life_time;            // The total life time of particle
    
    float size;                 // Scaling factor (size) of particle
    
    // Used in debris to determine when to emit smoke particle
    float last_spawn;
    
    int tex_num;                // Texture num of particle
    float tex_cycle;            // Cycle time left for this texture num
    
    float distance_from_camera; // Particles distance from camera
    
    particle* next;             // Pointer to next particle in list
    particle* next_in_bucket;   // Pointer to next particle in bucket
    effect* parent;
};

class effect
{
    private:
        int effect_id;          // Effect ID tag
        int effect_type;        // Effect type tag
        bool effect_done;       // Effect done boolean
        
        particle* pl_head;      // Particle list head pointer
        
        int emit_count;         // # of particles left to emit
        float emit_rate;		// Partile emit rate in seconds left until add
        float emit_timer;       // Timer for particle emmission
        
        float life_time;        // Total life of each particle
        float run_time;         // Running time of this effect
        float effect_life;      // Total lifetime of this effect

        float mod;              // Used to store the modifier passed into effect
        
        kVector start_pos;      // Center of effect/emission
        kVector start_dir;      // Direction of effect/emission
        float start_size;       // Starting size for particles
        float start_speed;      // Starting speed for particles
        
        kVector system_dir;     // Direction vector of system
        
        float gravity;          // Gravity value for this effect system
        float roll_speed;       // Angular velocity for this effect system
        float resize_rate;      // Resizing value (value per second)
        
        float size_variance;    // Variance in size (offset) from size
        float dispersion;       // Dispersion factor (in radians)
        float dispersion_area;  // Origin dispersion factor (used in rain and snow)
        
        // Since multiple frames are contained in a single image file
        // Image_slice represents the percentage of the width that a frame
        // Takes up in that image. Basically 1 / tex_frame_count
        float image_slice;
        
        GLuint tex_frame;       // Texture ID for animation frames
        int tex_frame_count;    // Frame count for animated textures
        float tex_cycle_time;   // Cycle time between frames
        bool tex_cycle_repeat;  // Repeat frames (ex: fire)
        
        bool draw;              // Culling control

        
        
        void emit_particle();

        friend class se_module;
        
    public:
        
        effect(int effectType, kVector position, kVector direction,
            float modifier, kVector systemDirection);
        ~effect();
        
        /* Accessors */
        bool isFinished()
            { return effect_done; }
        void getClosestAndFarthest( float &closest, float &farthest );
        
        /* Mutators */
        void sortIntoBuckets();
        void stopEffect()
            { emit_count = 0; tex_cycle_repeat = false; }
        void setStartPosition(kVector position)
            { start_pos = position; }
        void setStartDirection(kVector direction)
            { start_dir = direction; }
        
        /* Base Update & Display Routines */
        void update(float deltaT);
        
        effect* next;
};

/*******************************************************************************
   class       :   se_module
   purpose     :   Special effects module which handles and maintains all
                   effects inside of the game engine.
   notes       :   1) Add effect is split into many sub overloaded functions
                      for ease of use.
                   2) Modifier values vary per effect type.
                   3) ID returned should never, under any circumstance, be
                      edited, ever. At their heart, they are pointers.
*******************************************************************************/
class se_module
{
    private:
        effect* el_head;
        
    public:
        se_module();                    // Constructor
        ~se_module();                   // Deconstructor
        
        /* Base Rountines */
        void loadEffects();
        
        int addEffect(int effectType, kVector position, kVector direction,
            float modifier, kVector systemDirection);
            
        // Overloaded addEffect routines
        inline int addEffect(int effectType, kVector position, kVector direction, int modifier, kVector systemDirection)
            { return addEffect(effectType, position, direction, (float)modifier, systemDirection); }
        inline int addEffect(int effectType, kVector position, kVector direction, int modifier)
            { return addEffect(effectType, position, direction, (float)modifier, kVector(0.0,0.0,0.0)); }
        inline int addEffect(int effectType, kVector position, kVector direction, double modifier, kVector systemDirection)
            { return addEffect(effectType, position, direction, (float)modifier, systemDirection); }
        inline int addEffect(int effectType, kVector position, kVector direction, double modifier)
            { return addEffect(effectType, position, direction, (float)modifier, kVector(0.0,0.0,0.0)); }
        inline int addEffect(int effectType, kVector position, float modifier, kVector systemDirection)
            { return addEffect(effectType, position, kVector(0.0,1.0,0.0), modifier, systemDirection); }
        inline int addEffect(int effectType, kVector position, float modifier)
            { return addEffect(effectType, position, kVector(0.0,1.0,0.0), modifier, kVector(0.0,0.0,0.0)); }
        inline int addEffect(int effectType, kVector position, int modifier, kVector systemDirection)
            { return addEffect(effectType, position, kVector(0.0,1.0,0.0), (float)modifier, systemDirection); }
        inline int addEffect(int effectType, kVector position, int modifier)
            { return addEffect(effectType, position, kVector(0.0,1.0,0.0), (float)modifier, kVector(0.0,0.0,0.0)); }
        inline int addEffect(int effectType, kVector position, double modifier, kVector systemDirection)
            { return addEffect(effectType, position, kVector(0.0,1.0,0.0), (float)modifier, systemDirection); }
        inline int addEffect(int effectType, kVector position, double modifier)
            { return addEffect(effectType, position, kVector(0.0,1.0,0.0), (float)modifier, kVector(0.0,0.0,0.0)); }
        inline int addEffect(int effectType, kVector position, kVector direction)
            { return addEffect(effectType, position, direction, 1.0f, kVector(0.0,0.0,0.0)); }
        inline int addEffect(int effectType, kVector position, kVector direction, kVector systemDirection)
            { return addEffect(effectType, position, direction, 1.0f, systemDirection); }
        inline int addEffect(int effectType, kVector position)
            { return addEffect(effectType, position, kVector(0.0,1.0,0.0), 1.0f, kVector(0.0,0.0,0.0)); }
        inline int addEffect(int effectType)
            { return addEffect(effectType, kVector(0.0,0.0,0.0), kVector(0.0,1.0,0.0), 1.0f, kVector(0.0,0.0,0.0)); }
        
        /* Mutators */
        void stopEffect(int id);
        void killEffect(int id);
	
	// Sets variables farthest and closest
        void set_bucket_parameters();
        
        void setEffectPosition(int id, kVector position);
        void setEffectDirection(int id, kVector direction);
        
        /* Base Update and Display Routines */
        void update(float deltaT);
        void display();

        // Used in Rain and Snow effects to detect change in camera location
        kVector prevCamPos;
        
        // Buckets
        particle* buckets[SE_NUM_BUCKETS];
        
	// Farthest particle from the camera position
        float farthest;
	
	// Closest particle from the camera position
        float closest;
        
};

extern se_module effects;

#endif
