/*******************************************************************************
                         Object Modules - Definition
*******************************************************************************/
#ifndef OBJMODULES_H
#define OBJMODULES_H

#include "object.h"

/* Object Module Defines */

/* Crew Module Defines */
#define FEED_ROUTINE_MAX        10

#define FEED_PRIORITY_HIGH      0
#define FEED_PRIORITY_MID       1
#define FEED_PRIORITY_LOW       2

#define FEED_PRIORITY_HIGHER    0
#define FEED_PRIORITY_NORMAL    1
#define FEED_PRIORITY_LOWER     2

#define JOB_TIME_UNLIMITED      600.0f

/*******************************************************************************
    class       :   crew_module
    purpose     :   
    notes       :   
*******************************************************************************/
class crew_module
{
    private:
        struct job
        {
            char* job_text;                 // Text to display for job
            float job_time;                 // Time left for job
            short feed_routine;             // Feed routine to use
            bool handled;                   // Is a crew member handling this?
            bool suspended;                 // Is job currently suspended?
        };
        
        struct job_node
        {
            job* job_ptr;                   // Ptr to job
            float priority;                 // Priority setting
            bool unique;                    // Unique allocation for job_ptr
            
            job_node* next;
        };
        
        // Base
        object* parent;                     // Pointer to parent object
        
        int crew_count;
        char* crew_name[OBJ_CREW_MAX];
        float crew_pos[OBJ_CREW_MAX][3];
        int crew_attach[OBJ_CREW_MAX];
        float crew_health[OBJ_CREW_MAX];
        float crew_morale[OBJ_CREW_MAX];
        
        job_node* jl_head[OBJ_CREW_MAX];
        job_node* jl_derelict;
        job* curr_job[OBJ_CREW_MAX];
        
        int feedr_count;
        char feedr[FEED_ROUTINE_MAX][3][OBJ_CREW_MAX][2];
        
        bool job_feed(job* job_ptr);
        void job_assign(int crewman);
        
    public:
        crew_module();                      // Constructor
        ~crew_module();                     // Deconstructor
        
        /* Initialization Routine */
        void initCrew(object* parentPtr);
        
        /* Base Routines */
        unsigned int enqueJob(char* jobText, float jobTime, int feedRoutine);
        
        bool isJobFinished(unsigned int &jobID);
        void finishJob(unsigned int &jobID);
        
        void suspendJob(unsigned int jobID);
        void resumeJob(unsigned int jobID);
        
        /* Accessors */
        bool isCrewmanBusy(int crewMember)
            { return (curr_job[crewMember] != NULL); }
        bool isCrewmanActive(int crewMember)
            { return (crew_health[crewMember] >= 0.40 && crew_morale[crewMember] >= 0.40); }
        
        char* getCurrentJobText(int crewMember)
            { return (curr_job[crewMember]) ? curr_job[crewMember]->job_text : (char*)"Idle"; }
        float getCurrentJobTime(int crewMember)
            { return (curr_job[crewMember]) ? curr_job[crewMember]->job_time : 0.0; }
        int getCurrentJobFeedRoutine(int crewMember)
            { return (curr_job[crewMember]) ? curr_job[crewMember]->feed_routine : 0; }
        int getCurrentJobHandlement(int crewMember)
            { return (curr_job[crewMember]) ? curr_job[crewMember]->handled : false; }
        
        char* getJobText(unsigned int jobID)
            { return (jobID != JOB_NULL) ? ((job*)jobID)->job_text : (char*)"Idle"; }
        float getJobTime(unsigned int jobID)
            { return (jobID != JOB_NULL) ? ((job*)jobID)->job_time : 0.0; }
        int getJobFeedRoutine(unsigned int jobID)
            { return (jobID != JOB_NULL) ? (int)(((job*)jobID)->feed_routine) : 0; }
        bool getJobHandlement(unsigned int jobID)
            { return (jobID != JOB_NULL && ((job*)jobID)->handled); }
        
        int getCrewmanCount() { return crew_count; }
        char* getCrewmanName(int crewMember) { return crew_name[crewMember]; }
        float* getCrewmanPosition(int crewMember) { return crew_pos[crewMember]; }
        int getCrewmanAttach(int crewMember) { return crew_attach[crewMember]; }
        float getCrewmanHealth(int crewMember) { return crew_health[crewMember]; }
        float getCrewmanMorale(int crewMember) { return crew_morale[crewMember]; }
        
        char* getCrewmanHealthStatus(int crewMember);
        char* getCrewmanMoraleStatus(int crewMember);
        
        /* Mutators */
        void reduceHealth(int crewMember, float amount);
        void reduceMorale(int crewMember, float amount);
        void reduceCrewHealth(float amount)
            { for(int i = 0; i < crew_count; i++) reduceHealth(i, amount); }
        void reduceCrewMorale(float amount)
            { for(int i = 0; i < crew_count; i++) reduceMorale(i, amount); }
        
        void raiseMorale(int crewMember, float amount) { ; }
        void raiseCrewMorale(float amount)
            { for(int i = 0; i < crew_count; i++) raiseMorale(i, amount); }
        
        /* Base Update Routine */
        void update(float deltaT);
};

/*******************************************************************************
    class       :   motor_device
    purpose     :   Controls the engine and movement speed of the vehicle. Uses
                    an array of speed and time to attain speed attributes.
                    Plug-in to moveable objects.
    notes       :   1) Linear interpolation is used between gear speeds, There
                       exists no regression which can model all gears as one
                       function short of 100% accurate engine representation.
                    2) The name of the module is slightly misleading - the
                       motor module also controls the transmission.
                    3) Update must be passed the blockmapValue to use in order
                       to appropriately set tank speed.
*******************************************************************************/
class motor_device
{
    private:
        // Base
        object* parent;                     // Pointer to parent object
        int motor_sound_id;                 // Sound ID number (for engine)
        
        // Throttle Control
        float throttle;                     // Throttle setting (0.0 - 1.0)
        float throttle_speed;               // Limitation speed for throttle
        
        // Blockmap/Terrain Control
        int curr_blockmap_value;            // Current blockmap on terrain
        float blockmap_max_speed;           // Limitation speed on terrain
        float blockmap_multiplier;          // Blockmap multiplier for TIME
        float suspension_multiplier;        // Suspension efficiency multiplier
        
        // Tracking Control
        int curr_gear;                      // Current gear
        float curr_time;                    // Current time position
        float curr_speed;                   // Computed current speed (m/s)
        float curr_rpm;                     // Current RPM
        
        // Gear Control
        float* gear_slope;                  // Slope values for each gear
        float* gear_speed;                  // Speed limits for each gear
        float* gear_time;                   // Time to attain values for gears
        int gear_count;                     // # of gears
        
        // Time Control
        bool time_recompute;                // curr_time recomputation yes/no
        void recompute_time();              // Recomputes time based on speed
        
        // Damage Control
        float motor_life;                   // Motor life (0.0 to 1.0)
        kVector motor_pos;                  // Position of motor
        int damage_effect_id[2];            // Current effect IDs
        
        // Other Motor Values
        float max_rpm;                      // Max RPM value
        float min_rpm;                      // Min RPM value (computed)
        float idle_rpm;                     // Idle RPM value (computed)
        
    public:
        motor_device();                     // Constructor
        ~motor_device();                    // Deconstructor
        
        /* Initialization Routine */
        void initMotor(object* parentPtr);
        
        /* Accessors */
        int getCurrentGear() { return curr_gear; }
        float getCurrentSpeed() { return curr_speed; }
        float getCurrentRPM() { return curr_rpm; }
        float getMotorLife() { return motor_life; }
        kVector getMotorPosV() { return motor_pos; }
        float* getMotorPos() { return motor_pos(); }
        
        /* Mutators */
        void setThrottle(float percent);
        void setOutputSpeed(float speed) { curr_speed = speed; recompute_time(); }
        
        void reduceMotorLife(float amount);
        
        /* Base Update Routine */
        void update(float deltaT);
};

#define TARGET_BASE             0
#define TARGET_HULL             0
#define TARGET_TURRET1          1
#define TARGET_TURRET2          2
#define TARGET_TURRET3          3
#define TARGET_CREW1            11
#define TARGET_CREW2            12
#define TARGET_CREW3            13
#define TARGET_CREW4            14
#define TARGET_CREW5            15
#define TARGET_CREW6            16
#define TARGET_LEFT_TRACK       21
#define TARGET_RIGHT_TRACK      22
#define TARGET_LOWER_HULL       23
#define TARGET_UPPER_HULL       24
#define TARGET_ENGINE           25
#define TARGET_SPECIAL1         31
#define TARGET_SPECIAL2         32
#define TARGET_SPECIAL3         33
#define TARGET_SPECIAL4         34
#define TARGET_SPECIAL5         35

#define SIGHT_AWAITING          0
#define SIGHT_AQUIRING          1
#define SIGHT_AIMING            2
#define SIGHT_TRACKING          3

/*******************************************************************************
    class       :   sight_device
    purpose     :   Sighting device which simulates targeting of objects. All
                    guns are attached to a sighting mechanism which does the
                    aiming, the gun mechanism just handling the gun/breech
                    itself and nothing more. Targets are always assigned to
                    a sighting mechanism, but some guns may not be set to fire.
    notes       :   1) Uses a queue to determine which device is currently
                       allowed to fired. Devices get onto the queue via
                       enqueDevice and are assigned an ID number to all further
                       referencing of the module. Devices must then, after
                       firing, remove themselves from the queue with the
                       dequeueDevice and their ID number. ID numbers do not
                       transfer over from one enque to another.
                    2) Devices which have the same shell velocity are both
                       considered allowed to fire.
                    3) Devices can determine if they are in the state of being
                       allowed to fire via the isDeviceOkayed function.
                    4) Targets are assigned either in terms of an firing_object* or
                       a static float[3]. Targets that exist as firing_object* may
                       be further assigned a specific targeting spot, which
                       makes a referencing call to the DB of that object.
                    5) Guns which are currently not aimed properly at a target
                       will not currently release the target automatically.
*******************************************************************************/
class sight_device
{
    private:
        /* Base Attributes */
        object* parent;                     // Parent object pointer
        
        // Sight Attributes
        short sight_num;                    // Sight number
        char* sight_type;                   // Type of sight
        kVector sight_pivot;                // Pivot point for sight
        int sight_attach;                   // Attachment for sight
        
        // Sight Ext. Attributes
        float field_of_view;                // FOV (in radians)
        short status;                       // Sight status (SIGHT_STAT_X)
        
        // Job Attributes
        crew_module* crew;                  // Pointer to crew module
        unsigned int job_id;                // Current job ID tag
        short aim_routine;                  // Aiming routine to use
        
        // Sight Assignment Queue Attributes
        short queue_id[OBJ_GUN_MAX];        // Device queue (id)
        float queue_velocity[OBJ_GUN_MAX];  // Device adjustment velocity
        float previous_velocity;            // Last used adjustment velocity
        short head_pos;                     // Head position
        short curr_pos;                     // Current position
        
        // Targeting Data Attributes
        object* target_obj_ptr;             // Target (as an object pointer)
        float target_position[3];           // Target spot position (or offset)
        short target_spot;                  // Target specific spot (TARGET_X)
        bool target_assigned;               // Defines if target is assigned
        bool target_isa_object;             // Controls usage of target_obj_ptr
        float previous_theta;               // Previously used theta (PE solver)
        
        // Elevation Data Attributes
        float elevate_min;                  // Minimum elevation
        float elevate_std;                  // Standard (no target) elevation
        float elevate_max;                  // Maximum elevation
        float elevate_speed;                // Maximum elevation speed
        float elevate_error;                // Error in elevation to apply
        float desired_elevate;              // Desired elevation
        float elevate;                      // Current elevation
        
        // Transverse Data Attributes
        float trans_min;                    // Minimum transverse
        float trans_std;                    // Standard (no target) transverse
        float trans_max;                    // Maximum transverse
        float trans_speed;                  // Maximum transverse speed
        float trans_error;                  // Error in transverse to apply
        float desired_trans;                // Desired transverse
        float transverse;                   // Current transverse
        
    public:
        sight_device();                     // Constructor
        ~sight_device() { return; }         // Deconstructor
        
        /* Initialization Routine */
        void initSight(object* parentPtr, int sightNum);
        
        /* Target Assignment */
        void assignTarget(object* objPtr, short targetSpot = TARGET_BASE);
        void assignTarget(float* position);
        inline void assignTarget(float* position, short targetSpot)
            { assignTarget(position); }
        void relieveTarget();
        
        /* Error Offseting */
        void assignError(float elevateError, float transverseError)
            { elevate_error = elevateError; trans_error = transverseError; }
        
        /* Device Queing */
        int enqueueDevice(short deviceID, float adjustmentVelocity);
        void dequeueDevice(short sightID);
        
        bool isDeviceOkayed(short sightID);
        
        /* Accessors */
        bool isTargetAssigned()
            { return target_assigned; }
        
        int getSightNum()
            { return sight_num; }
        char* getSightType()
            { return sight_type; }
        
        kVector getSightPivotV()
            { return sight_pivot; }
        float* getSightPivot()
            { return sight_pivot(); }
        int getSightAttach()
            { return sight_attach; }
        
        int getSightStatus()
            { return status; }
        char* getSightStatusStr();
        
        unsigned int getJobID() { return job_id; }
        
        short getDeviceCurrentID()
            { return curr_pos; }
        float getDeviceAdjustmentVelocity()
            { return queue_velocity[curr_pos]; }
        
        float getTransverse()
            { return transverse; }
        float getElevate()
            { return elevate; }
        
        /* Base Update Routine */
        void update(float deltaT);
};

#define BREECH_EMPTY            0
#define BREECH_LOADED           1
#define BREECH_SPENT            2
#define BREECH_EJECTING         3
#define BREECH_CYCLING          4
#define BREECH_LOADING          5
#define BREECH_UNLOADING        6

/*******************************************************************************
    class       :   gun_device
    purpose     :   Controls the gun/breech of a gun. Handles current status of
                    breech and interfaces directly with the sighting mechanism
                    and firing functions of the parent object.
    notes       :   1) Does not control aiming of the gun - this is done via
                       the attached sighting mechanism.
                    2) Currently does not have any interface with the crew, so
                       automatically loads new shell, aims, and fires.
*******************************************************************************/
class gun_device
{
    private:
        /* Base Attributes */
        object* parent;                     // Parent object pointer
        
        // Gun Attributes
        int gun_num;                        // Gun number
        char* gun_type;                     // Type of gun
        kVector gun_pivot;                  // Pivot point for gun
        int gun_attach;                     // Attachment for pivot
        
        // Gun Ext. Attributes
        float gun_recoil;                   // Current recoil on gun
        float gun_max_recoil;               // Max recoil used on gun
        float gun_length;                   // Length of gun (on 3D model)
        
        // Sight Attachment
        sight_device* sight;                // Pointer to sighting device
        short sight_id;                     // Sight controler assigned id
        
        // Breech Attributes
        short status;                       // Breech status
        float tracking_time;                // Elapsed time tracking target
        float breech_timer;                 // Basic breech timer for module
        
        // Job Attributes
        crew_module* crew;                  // Pointer to crew module
        unsigned int job_id;                // Current job ID tag
        short load_routine;                 // Load routine to use
        
        // Ammoclip Attributes
        short clip_size;                    // Clip Size
        short clip_left;                    // Number of rounds left in clip
        short clip_burst;                   // # of rounds to fire per burst
        float clip_fire_time;               // Firing rate (time between rounds)
        
        // Ammo Attributes
        short ammo_in_breech;               // Current ammo (pool) in breech
        short ammo_in_usage;                // Future ammo (pool) to work with
        float ammo_load_time;               // Loading time for ammo
        
        bool is_fireable[OBJ_MAX_AMMOPOOL]; // Determines if can fire said pool
        
        /* Misc Boolean Controls */
        bool enabled;                       // Is gun enabled to fire?
        bool automatic_eject;               // Can gun auto-eject spent shells
        bool out_of_ammo;                   // Is this gun out of ammo
        bool flash_supressor;               // Does gun have a flash supressor?
        
    public:
        gun_device();                       // Constructor
        ~gun_device() { return; }           // Deconstructor
        
        /* Initialization Routine */
        void initGun(object* parentPtr, int gunNum);
        
        /* Accessors */
        bool isFireEnabled() { return enabled; }
        bool isFireable(int poolNum) { return is_fireable[poolNum]; }
        bool isOutOfAmmo() { return out_of_ammo; }
        bool isMainGun() { return !is_fireable[OBJ_AMMOPOOL_MG]; }
        bool isMachineGun() { return is_fireable[OBJ_AMMOPOOL_MG]; }
        bool isFlashSupressed() { return flash_supressor; }
        
        short getGunNum() { return gun_num; }
        char* getGunType() { return gun_type; }
        
        kVector getGunPivotV() { return gun_pivot; }
        float* getGunPivot() { return gun_pivot(); }
        int getGunAttach() { return gun_attach; }
        
        float getGunRecoil() { return gun_recoil; }
        float getGunLength() { return gun_length; }
        
        int getSightNum() { if(sight) return sight->getSightNum(); return -1; }
        
        int getBreechStatus() { return status; }
        char* getBreechStatusStr();
        float getBreechTime()
            { if(crew && job_id != JOB_NULL) return crew->getJobTime(job_id); return breech_timer; }
        
        unsigned int getJobID() { return job_id; }
        
        int getAmmoInBreech() { return ammo_in_breech; }
        int getAmmoInUsage() { return ammo_in_usage; }
        
        int getClipLeft() { return clip_left; }
        
        float getTransverse() { if(sight) return sight->getTransverse(); return 0.0; }
        float getElevate() { if(sight) return sight->getElevate(); return 0.0; }
        
        /* Mutators */
        void enableFire() { enabled = true; }
        void disableFire() { enabled = false; }
        void recoilGun() { gun_recoil = gun_max_recoil; }
        
        // Breech Routines
        void loadBreech();
        void unloadBreech();
        
        // Ammo Routines
        void setAmmoUsage(int poolNum);
        void setClipBurst(int burstCount)
            { if(burstCount >= 1 && burstCount <= clip_size) clip_burst = burstCount; }
        
        /* Base Update Routine */
        void update(float deltaT);
};

#endif
