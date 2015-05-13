/*******************************************************************************
                        Tank Object Model - Definition
*******************************************************************************/
#ifndef TANK_H
#define TANK_H

#include "metrics.h"
#include "object.h"
#include "objunit.h"

#define TANK_TRACK_MOVEMENT             0.11    // Multiplier for UV offsetting

/*******************************************************************************
    struct      :   tank_object
    purpose     :   The base tank object module which controls and takes care
                    of the primary functions of the tank object. This object
                    is the centerpiece of the game and is a very complicated
                    structure with many facets.
    notes       :   
*******************************************************************************/
struct tank_object : public moving_object, public turreted_object
{
    /* Attributes */
    float angular_offset;                   // Turning circle speed (+b) offset value
    int cupola_attach;                      // Attachment for cupola
    
    // Track UV Attributes
    int track_left_id;
    float track_left_s_texel;
    int track_right_id;
    float track_right_s_texel;
    
    /* Building Routine */
    void build_tank();                      // Builds Display Lists
    
    /* Functions */
    tank_object();                          // Constructor
    ~tank_object();                         // Deconstructor
    
    /* Initialization Routine */
    void initTank();
    
    /* Metrics */
    kVector transform(kVector position, int attachLevelFrom = OBJ_ATTACH_HULL, int attachLevelTo = OBJ_ATTACH_WORLD);
    
    /* Base Update & Display Routines */
    void update(float deltaT);
    void display();
    void displayWaypoints();
};

#endif
