/*******************************************************************************
                        Projectile Object - Definition
*******************************************************************************/
#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "metrics.h"
#include "object.h"
#include "objhandler.h"

#define AMMO_TYPE_AP            0       // Armor Piercing
#define AMMO_TYPE_APC           1       // Armor Piercing Capped
#define AMMO_TYPE_APBC          2       // Armor Piercing /w Ballistic Cap
#define AMMO_TYPE_APCBC         3       // Armor Piercing Capped /w Ball. Cap
#define AMMO_TYPE_APCR          4       // Armor Piercing Composite Rigit (Tung)
#define AMMO_TYPE_API           5       // Armor Piercing Incendinary
#define AMMO_TYPE_HE            6       // High Explosive
#define AMMO_TYPE_HEAT          7       // High Explosive Anti Tank
#define AMMO_TYPE_HESH          8       // High Explosive Squash Head
#define AMMO_TYPE_SMOKE         9       // Smoke
#define AMMO_TYPE_UNKNOWN       -1      // Invalid

#define AMMO_MOD_STANDARD       0x00    // No modifiers
#define AMMO_MOD_HE_BURSTER     0x01    // Round /w HE-Burster (not for HE!)
#define AMMO_MOD_HE_BURSTER_ARMED 0x02  // Round's HE Burster armed!

#define AMMO_MOD_YELLOW_TRACER  0x1000  // Yellow tracer
#define AMMO_MOD_WHITE_TRACER   0x2000  // White tracer
#define AMMO_MOD_RED_TRACER     0x4000  // Red tracer
#define AMMO_MOD_GREEN_TRACER   0x8000  // Green tracer
#define AMMO_MOD_TRACER         0xF000  // Contains tracer (MUST match tracers)

#define PROJ_VEL_MULTIPLIER     0.472   // Universal projectile slow-down

#define PROJ_MAX_TRACER_TAIL    10      // Tracer tail length - DO NOT CHANGE!
#define PROJ_TRACER_TIMER       0.015   // Tracer tail time per position

#define PROJ_FIRE_DISPERSION    0.06    // Dispersion angle (for firing/init)
#define PROJ_INT_DISPERSION     15.0    // Dispersion angle (for interrupt)

/* String Parsing Helper Functions */
short int ammoType(char* typeStr);

/*******************************************************************************
    struct      :   proj_object
    purpose     :   Object container for all projectiles in-game, including
                    basic shells as well as machine gun bullets.
    notes       :   1) Maintains the same header as that of the object.
                    2) All vectors are in cartesian, never spherical (even for
                       direction).
                    3) Direction vector is scaled by velocity to save an extra
                       multiply.
                    4) Gravity usage is -9.81 m/s^2.
*******************************************************************************/
struct proj_object : public object 
{
    /* Base Attributes */
    cdtl_node* cdtl_head;           // Collision Detection Testing List
    float travel_distance;          // Distance traveled (in m)
    float distance_offset;          // TD offset (e.g. damage)
    float remove_timer;             // Time left before removal (in s)
    bool projectile_flight;         // Projectile is in flight/moving
    bool projectile_damaged;        // Projectile has been damaged
    short cdr_passes;               // # of CDR passes being performed
    
    /* Projectile Data */
    short type;                     // Projectile type (AMMO_TYPE_xxxx)
    float velocity;                 // Velocity (in m/s)
    float diameter;                 // Caliber (in cm)
    float explosive;                // Explosive content in shell (in kg)
    
    /*/ Tracer/Smoke Trail Extension */
    float tracer_timer;
    int tracer_depth;
    float tracer_tail_pos[PROJ_MAX_TRACER_TAIL][3];
    
    /* Functions */
    proj_object();                  // Constructor
    ~proj_object();                 // Deconstructor
    
    /* Initialize */
    void initProj(object* parentPtr, char* roundType, kVector position,
        kVector direction);
    
    /* Projectile Extensions */
    void armDevice();
    void interupt(kVector newPos, kVector newDir, float newVelocity,
        float timeOver, bool damageShell);
    
    /* Mutators */
    void killProj()
        { projectile_flight = false; }
    void addModifier(int modifier)
        { obj_modifiers = obj_modifiers | modifier; }
    void removeModifier(int modifier)
        { obj_modifiers = obj_modifiers - (obj_modifiers & modifier); }
    
    /* Base Update and Display Routines */
    void update(float deltaT);
    void display();
};

#endif
