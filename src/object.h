/*******************************************************************************
                          Base Object - Definition
*******************************************************************************/
#ifndef OBJECT_H
#define OBJECT_H

#include "metrics.h"

/* Object Defines */

// Object Types (0-9 supported only)
#define OBJ_TYPE_TANK           0       // Tank object              (unit)
#define OBJ_TYPE_VEHICLE        1       // Vehicle object           (unit)
#define OBJ_TYPE_INFANTRY       2       // Infantry object          (unit)
#define OBJ_TYPE_ATG            3       // Anti-Tank Gun object     (unit)
#define OBJ_TYPE_ATR            4       // Anti-Tank Rifle object   (unit)
#define OBJ_TYPE_BUNKER         5       // Bunker object            (unit)
#define OBJ_TYPE_STATIC         7       // Basic Static object      (non-unit)
#define OBJ_TYPE_SPECIAL        8       // Special case object      (non-unit)
#define OBJ_TYPE_PROJECTILE     9       // Projectile object        (reserved)

// Object Status
#define OBJ_STATUS_OK           0       // Object is fine
#define OBJ_STATUS_IMMOBILE     1       // Object is immobile (tracked/engine)
#define OBJ_STATUS_ABANDONED    2       // Object's crew bailed
#define OBJ_STATUS_KNOCKEDOUT   5       // Object is completely knocked out
#define OBJ_STATUS_DESTROYED    5       // Object is completely destroyed
#define OBJ_STATUS_DEAD         5       // Object is completely dead
#define OBJ_STATUS_RETREAT      8       // Object is in FULL retreat
#define OBJ_STATUS_REMOVE       255     // *Special* Object remove on update

// Object Modifiers
#define OBJ_MOD_NONE            0x0000  // No modifiers
#define OBJ_MOD_AXIS            0x0001  // Object is Axis aligned
#define OBJ_MOD_ALLIED          0x0002  // Object is Allied aligned
#define OBJ_MOD_GERMAN          0x0004  // Object is a German unit
#define OBJ_MOD_BELGIAN         0x0008  // Object is a Belgian unit
#define OBJ_MOD_BRITISH         0x0010  // Object is a British unit
#define OBJ_MOD_FRENCH          0x0020  // Object is a French unit
#define OBJ_MOD_POLISH          0x0040  // Object is a Polish unit
#define OBJ_MOD_ATTACHED        0x0100  // Object is attached to player control
#define OBJ_MOD_PLAYER          0x0200  // Object contains the player
#define OBJ_MOD_MILITIA         0x0400  // Object is militia skilled
#define OBJ_MOD_GREEN           0x0800  // Object is green skilled
#define OBJ_MOD_REGULAR         0x1000  // Object is regular skilled
#define OBJ_MOD_EXPERIENCED     0x2000  // Object is experienced skilled
#define OBJ_MOD_VETERAN         0x4000  // Object is veteran skilled
#define OBJ_MOD_ELITE           0x8000  // Object is elite skilled
#define OBJ_MOD_RANK            0xFC00  // Object rank container

// Attachment Levels
#define OBJ_ATTACH_MAX          14      // Attachment max count
#define OBJ_ATTACH_LEVEL_MAX    4       // Level positions
#define OBJ_ATTACH_LEVEL_WORLD  -1      // WCS level
#define OBJ_ATTACH_LEVEL_HULL   0       // Base level
#define OBJ_ATTACH_LEVEL_TURRET 1       // Turret level
#define OBJ_ATTACH_LEVEL_GUNMNT 2       // Gun Mount level
#define OBJ_ATTACH_LEVEL_GUN    3       // Gun level
#define OBJ_ATTACH_WORLD        -1      // WCS
#define OBJ_ATTACH_HULL         0       // Hull
#define OBJ_ATTACH_TURRET_OFF   1       // Offset starting value for turrets
#define OBJ_ATTACH_TURRET1      1       // Turret1
#define OBJ_ATTACH_TURRET2      2       // Turret2
#define OBJ_ATTACH_TURRET3      3       // Turret3
#define OBJ_ATTACH_GUNMNT_OFF   4       // Offset starting value for gun mounts
#define OBJ_ATTACH_GUNMNT1      4       // GunMount1
#define OBJ_ATTACH_GUNMNT2      5       // GunMount2
#define OBJ_ATTACH_GUNMNT3      6       // GunMount3
#define OBJ_ATTACH_GUNMNT4      7       // GunMount4
#define OBJ_ATTACH_GUNMNT5      8       // GunMount5
#define OBJ_ATTACH_GUN_OFF      9       // Offset starting value for gun
#define OBJ_ATTACH_GUN1         9       // Gun1
#define OBJ_ATTACH_GUN2         10      // Gun2
#define OBJ_ATTACH_GUN3         11      // Gun3
#define OBJ_ATTACH_GUN4         12      // Gun4
#define OBJ_ATTACH_GUN5         13      // Gun5

// Waypoint Modifiers
#define WP_MOD_NONE           0x0000    // Blank path node modifier
#define WP_MOD_FORWARD        0x0001    // WP node to go forward
#define WP_MOD_REVERSE        0x0002    // WP node to go reverse
#define WP_MOD_SPEED_10       0x0004    // WP node to go at 10% throttle
#define WP_MOD_SPEED_20       0x0008    // WP node to go at 20% throttle
#define WP_MOD_SPEED_30       0x0010    // WP node to go at 30% throttle
#define WP_MOD_SPEED_40       0x0020    // WP node to go at 40% throttle
#define WP_MOD_SPEED_50       0x0040    // WP node to go at 50% throttle
#define WP_MOD_SPEED_60       0x0080    // WP node to go at 60% throttle
#define WP_MOD_SPEED_70       0x0100    // WP node to go at 70% throttle
#define WP_MOD_SPEED_80       0x0200    // WP node to go at 80% throttle
#define WP_MOD_SPEED_90       0x0400    // WP node to go at 90% throttle
#define WP_MOD_SPEED_100      0x0800    // WP node to go at 100% throttle
#define WP_MOD_TRANSMITTED    0x1000    // WP node has been transmited
#define WP_MOD_PF_ASSIGNED    0x4000    // WP node was made by PF system
#define WP_MOD_CR_ASSIGNED    0x8000    // WP node made by CR system

// Ammo pool
#define OBJ_MAX_AMMOPOOL        6       // Ammo pool max count
#define OBJ_AMMOPOOL_A1         0       // First pool (primary / AP)
#define OBJ_AMMOPOOL_A2         1       // Second pool (secondary / HE)
#define OBJ_AMMOPOOL_A3         2       // Third pool
#define OBJ_AMMOPOOL_A4         3       // Fourth pool
#define OBJ_AMMOPOOL_A5         4       // Fifth pool
#define OBJ_AMMOPOOL_MG         5       // Machine gun ammo pool    (reserved)

// Max count defines
#define OBJ_CREW_MAX            10      // Crew members         (do not change!)
#define OBJ_TURRET_MAX          3       // Turret devices       (do not change!)
#define OBJ_SIGHT_MAX           3       // Sighting devices     (do not change!)
#define OBJ_GUN_MAX             5       // Gun devices          (do not change!)

/* Helper Functions */
unsigned short objType(char* designationStr);
unsigned short objStatus(char* statusStr);
unsigned short objModifiers(char* modifierStr);

/*******************************************************************************
    struct      :   object
    purpose     :   Base object structure. Inherited into all other objects.
    notes       :   Position vector is in CARTESIAN while direction vector
                    is in SPHERICAL.
*******************************************************************************/
struct object
{
    /* Attributes */
    // Base Object Attributes
    char* obj_model;                // Object model name
    unsigned short obj_type;        // Object type identifier
    unsigned short obj_status;      // Object status identifier
    unsigned int obj_modifiers;     // Object modifiers (32 bit binary)
    
    // Orientation Attributes
    kVector pos;                    // Position vector (CARTESIAN)
    kVector dir;                    // Direction vector (SPHERICAL)
    float roll;                     // Roll
    
    // Orientation Matricies
    GLfloat hull_matrix[16];        // Hull orientation matrix
    
    // Model Attributes
    int model_id;                   // Model library ID for object type
    kVector size;                   // Size of object (WIDTH, HEIGHT, LENGTH)
    
    // Culling Attributes
    bool draw;                      // Object in view (yes/no)
    float radius;                   // Culling radius
    
    /* Functions */
    object();                       // Constructor
    virtual ~object();              // Deconstructor
    
    /* Initialization Routines */
    // Object Base Initializers
    void initObj(char* modelName, unsigned short status, unsigned short modifiers);
    inline void initObj(char* modelName, char* statusStr, char* modifierStr)
        { initObj(modelName, objStatus(statusStr), objModifiers(modifierStr)); }
    // Object Orientation Initializers
    void initObj(float xPos, float zPos, float heading);
    void initObj(float* position, float* direction, float roll);
    
    /* Attachment Level Transformation */
    virtual kVector transform(kVector position, int attachLevelFrom = OBJ_ATTACH_HULL, int attachLevelTo = OBJ_ATTACH_WORLD);
    
    /* Base Update and Display Routines */
    virtual void update(float deltaT);      // Virtual update
    virtual void display();                 // Virtual display
};

#endif
