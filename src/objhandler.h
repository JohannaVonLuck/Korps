/*******************************************************************************
                         Object Handler - Definition
*******************************************************************************/
#ifndef OBJHANDLER_H
#define OBJHANDLER_H

#include "object.h"
#include "objunit.h"
#include "objlist.h"

/* Object Handler Defines */

// Object Count Limits
#define OBJ_MAX_TANK            35      // Max number of Tank objects
#define OBJ_MAX_VEHICLE         20      // Max number of Vehicle objects
#define OBJ_MAX_ATG             15      // Max number of ATG objects
#define OBJ_MAX_ATR             10      // Max number of ATR objects
#define OBJ_MAX_STATIC          100     // Max number of Static objects
#define OBJ_MAX_PROJECTILE      300     // Max number of Projectile objects

// Collision Detection Testing List Structure
struct cdtl_node
{
    object* obj_ptr;                    // Pointer to object
    float init_yaw;                     // Initial yaw
    
    cdtl_node* next;
};

/*******************************************************************************
    class       :   object_handler
    purpose     :   Manages and controls all objects currently being used in
                    the game engine.
    notes       :   
*******************************************************************************/
class object_handler
{
    private:
        object** objects[10];       // Object pointers
        int obj_max[10];            // Max objects per level
        int obj_count[10];          // Object count per level
    
    public:
        object_handler();           // Constructor
        ~object_handler();          // Deconstructor
        
        /* Base Routines */
        void loadMission(char* directory);
        
        object* addObject(int objType);
        void removeObject(object* objPtr)
            { if(objPtr) objPtr->obj_status = OBJ_STATUS_REMOVE; }
        
        /* Unit Grabbing Routines */
        object* getUnitAt(int x, int y);
        object* getUnitNear(kVector pos, float distanceTolerance);
        object* getUnitWithID(char* idTag);
        
        object_list* getUnitsAt(int x_min, int y_min, int x_max, int y_max);
        object_list* getUnitsNear(kVector pos, float distanceTolerance);
        object_list* getUnitsWithID(char* idTag);
        
        /* Collision Detection Routines */
        cdtl_node* createCDTL(object* objPtr, float angleTolerance,
            float AABBTolerance, object* excludeObjPtr);
        void killCDTL(object* objPtr);
        
        void cdObjPass(int startList, int endList);
        void cdProjPass();
        
        /* Base Update and Display Routines */
        void update(float deltaT);
        void displayFirstPass();
        void displaySecondPass();
};

extern object_handler objects;

#endif
