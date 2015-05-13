/*******************************************************************************
                        ATG Object Model - Definition
*******************************************************************************/
#ifndef ATG_H
#define ATG_H

#include "metrics.h"
#include "object.h"
#include "objunit.h"

/*******************************************************************************
    struct      :   atg_object
    purpose     :   Anti-tank gun object.
    notes       :   
*******************************************************************************/
struct atg_object : public firing_object
{
    /* Attributes */
    float hull_yaw;
    bool hull_yaw_override;
    
    /* Building Routine */
    void build_atg();               // Builds dspLists
    
    /* Functions */
    atg_object();                   // Constructor
    ~atg_object();                  // Deconstructor
    
    /* Initialization Routine */
    void initATG();
    
    /* Metrics */
    kVector transform(kVector position, int attachLevelFrom = OBJ_ATTACH_HULL, int attachLevelTo = OBJ_ATTACH_WORLD);
    
    /* Base Update & Display Routines */
    void update(float deltaT);
    void display();
};

#endif
