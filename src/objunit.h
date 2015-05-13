/*******************************************************************************
                          Object Units - Definition
*******************************************************************************/
#ifndef OBJUNIT_H
#define OBJUNIT_H

#include "metrics.h"
#include "object.h"
#include "objmodules.h"

// Waypoint Control Structure
struct waypoint_node
{
    kVector waypoint;                   // Waypoint position
    unsigned int modifiers;             // Modifiers for waypoint
    unsigned int jobID;                 // Job ID for waypoint/crew control
    
    waypoint_node* next;
};

/* Helper Functions */
bool isUnitNation(object* objPtr, int nation);
bool isUnitSide(object* objPtr, int side);
bool isUnitAttached(object* objPtr);

/*******************************************************************************
    struct      :   unit_object
    purpose     :   
    notes       :   
*******************************************************************************/
struct unit_object : public object
{
    /* Attributes */
    // Base Attributes
    char* obj_id;                       // Object identification tag
    unsigned short obj_routines;        // Object order feed routines (4b RCSD)
    unsigned short obj_organization;    // Object organization (4b BCPU)
    
    // Misc. Attributes
    GLubyte* picture;                   // Picture associated with object
    int picture_width;                  // Width of UI picture
    int picture_height;                 // Height of UI picture
    char* organization;                 // Object organization string
    bool selected;                      // Object is selected or not
    bool picture_load;                  // Object loaded this picture
    
    /* Display Lists */
    GLuint hull_dspList;                // Hull display list
    GLuint selected_dspList;            // Drawing to display when selected
    
    /* Object Modules */
    crew_module crew;                   // Crew module job controller
    int ext_crew_attach;                // EXT_CREW attachment
    
    /* Functions */
    unit_object();                      // Constructor
    ~unit_object();                     // Deconstructor
    
    /* Initialization Routines*/
    void initUnit(char* idTagStr, char* organizationStr);
    void initUnit();
    
    /* Base Update & Display Routine */
    inline void updateUnit(float deltaT) { crew.update(deltaT); }
    void displayForSelection();         // Display for OpenGL selection buffer
};

/*******************************************************************************
    struct      :   moving_object
    purpose     :   
    notes       :   
*******************************************************************************/
struct moving_object : virtual public unit_object
{
    /* Attributes */
    float linear_vel;                       // Current movement speed
    float angular_vel;                      // Current turning speed
    float max_angular_vel;                  // Maximum turning speed
    
    waypoint_node* wl_head;                 // Head of waypoint list
    waypoint_node* wl_tail;                 // Tail of waypoint list
    
    /* Object Modules */
    motor_device motor;                     // Motor (speed) device controler
    
    /* Functions */
    moving_object();                        // Constructor
    ~moving_object();                       // Deconstructor
    
    /* Initialization Routines*/
    inline void initMovement() { motor.initMotor((object*)this); }
    
    /* Movement Routines */
    void addWaypoint(kVector waypoint, unsigned int modifiers = WP_MOD_FORWARD);
    void killWaypoints();
    
    /* Base Update Routine */
    void updateMovement(float deltaT);
};

/*******************************************************************************
    struct      :   firing_object
    purpose     :   
    notes       :   
*******************************************************************************/
struct firing_object : virtual public unit_object
{
    /* Attributes */
    short ammo_pool[OBJ_MAX_AMMOPOOL];      // Ammo pool (shell load-out)
    char* ammo_pool_type[OBJ_MAX_AMMOPOOL]; // Ammo type (shell type per pool)
    int ext_ammo_attach;                    // Attachment for EXT_AMMO
    
    /* Orientation Matricies */
    GLfloat** gun_matrix;                   // Gun matricies
    
    /* Display Lists */
    GLuint* gun_mant_dspList;               // Gun mantlet display list
    GLuint* gun_dspList;                    // Gun display list
    
    /* Object Modules */
    short sight_count;                      // # of sighting devices (scopes)
    short gun_count;                        // # of gun devices (breeches)
    sight_device* sight;                    // Sight (scope) device controlers
    gun_device* gun;                        // Gun (breech) device controlers
    
    /* Functions */
    firing_object();                        // Constructor
    ~firing_object();                       // Deconstructor
    
    /* Initialization Routines*/
    void initWeapons();
    void initAmmo(char* ammoLoadStr);
    
    /* Firing Routines */
    virtual object* fireGun(int gunNum, int fromAmmoPool);
    
    /* Base Update Routine */
    void updateWeapons(float deltaT);
};

/*******************************************************************************
    struct      :   turreted_object
    purpose     :   
    notes       :   
*******************************************************************************/
struct turreted_object : virtual public unit_object, virtual public firing_object
{
    /* Attributes */
    int turret_count;                       // # of turrets
    float* turret_rotation;                 // Turret rotation
    kVector* turret_pivot;                  // Pivot points for turrets
    
    /* Orientation Matricies */
    GLfloat** turret_matrix;                // Turret matricies
    
    /* Display List */
    GLuint* turret_dspList;                 // Turret display list
    
    /* Functions */
    turreted_object();                      // Constructor
    ~turreted_object();                     // Deconstructor
    
    /* Initialization Routine */
    void initTurrets();
    
    /* Base Update Routine */
    void updateTurrets(float deltaT);
};

#endif
