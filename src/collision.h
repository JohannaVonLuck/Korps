/*******************************************************************************
               Collision Detection & Response Module - Definition
*******************************************************************************/

#ifndef COLLISION_H
#define COLLISION_H

#include "metrics.h"
#include "model.h"
#include "object.h"
#include "projectile.h"
#include "scenery.h"

// T offsets (for polygon based collision detection)
#define CD_T_START                      12345.0 // Start value for T for PBCD
#define CD_T_OFFSET                     32.0    // T offseting value for PBCD

#define CD_EXCLUDE_NO_MESHES            -1

// Slope Effect Fixes (to slope formulas for small-caliber ops)
#define CR_SLOPE_ADDFIX                 0.06    // Slope multiplier fix
#define CR_SLOPE_DIAFIX_MULTIPLIER      0.7     // Diameter fix multiplier

// Armor Types
#define ARMOR_TYPE_RHA                  0       // Rolled Homogenous Armor
#define ARMOR_TYPE_FHS                  1       // Face Hardened Steel
#define ARMOR_TYPE_CAST                 2       // Cast Armor

// Collision Detection Data Report Modifiers
#define CD_MOD_NONE                 0x0000      // No modifiers
#define CD_MOD_EXT_CREW_HIT         0x0001      // EXT_CREW mesh hit
#define CD_MOD_EXT_AMMO_HIT         0x0002      // EXT_AMMO mesh hit
#define CD_MOD_EXT_ENGINE_HIT       0x0004      // EXT_ENGINE mesh hit
#define CD_MOD_EXT_FUEL_HIT         0x0008      // EXT_FUEL mesh hit
#define CD_MOD_CK_HULL_FRONT        0x0010      // Front of hull checked
#define CD_MOD_CK_HULL_LEFT         0x0020      // Left of hull checked
#define CD_MOD_CK_HULL_RIGHT        0x0040      // Right of hull checked
#define CD_MOD_CK_HULL_REAR         0x0080      // Rear of hull checked
#define CD_MOD_CK_TURRET_FRONT      0x0100      // Front of a turret checked
#define CD_MOD_CK_TURRET_LEFT       0x0200      // Left of a turret checked
#define CD_MOD_CK_TURRET_RIGHT      0x0400      // Right of a turret checked
#define CD_MOD_CK_TURRET_REAR       0x0800      // Rear of a turret checked

// Collision Response Data Report Modifiers
#define CR_MOD_NONE                 0x0000      // No modifiers
#define CR_MOD_FULL_PEN             0x0001      // Full penetration
#define CR_MOD_PARTIAL_PEN          0x0002      // Partial penetration
#define CR_MOD_PENETRATION          0x0003      // Penetration combine
#define CR_MOD_MAJOR_SPALLING       0x0004      // Major spalling
#define CR_MOD_MINOR_SPALLING       0x0008      // Minor spalling
#define CR_MOD_SPALLING             0x000C      // Spalling combine
#define CR_MOD_PEN_OR_SPALL         0x000F      // Pen/Spal combine
#define CR_MOD_NO_PENETRATION       0x0010      // No penetration/damage
#define CR_MOD_RICOCHET             0x0080      // Shell richochets
#define CR_MOD_EXPLODE              0x0100      // Shell explodes (outside only)
#define CR_MOD_SHATTER              0x0200      // Shell shatters (shatter gap)
#define CR_MOD_DISAPEAR             0x0400      // Shell forcefully disapears/removed (not pen!)
#define CR_MOD_ARM_DEVICE           0x2000      // Shell HE burster/incd. device armed

// Collision Detection Data Report
struct cd_data
{
    kVector rayPos;
    kVector rayDir;
    
    short id;
    short mesh;
    short attachment;
    unsigned short modifiers;
    
    float t;
    
    kVector impactPoint;
    float impactAngle;
    kVector reflection;
};

// Collision Response Data Report
struct cr_data
{
    unsigned short modifiers;           // Base Modifiers
    float impact_angle;                 // Angle of impact (degrees)
    float impact_velocity;              // Impact velocity (m/s)
    float impact_ke;                    // Impact kinetic energy (kg(m/s)^2)
    float td_ratio;                     // Thickness/Diameter ratio (mm/mm)
    float slope_effect;                 // Slope effect multiplier
    float multipliers;                  // Other multipliers
    float resistance;                   // Equivalent resistance (mm)
    float penetration;                  // Impact penetration (mm)
    float pr_ratio;                     // Penetration/Resistance ratio (mm/mm)
    float probability;                  // Penetration probability (0.0 - 1.0)
    float result_velocity;              // Resulting velocity afterwords
    float result_ke;                    // Resulting kinetic energy afterwords
    float distance_offset;              // KE loss mimicing - richochets only
};

// Anticipated Collision Node (ACN)
struct ac_node
{
    object* objOnePtr;
    object* objTwoPtr;
    cd_data* cdDataReport;
    float timeToCollision;
    
    ac_node* next;
};

/*******************************************************************************
    class       :   collision_module
    purpose     :   The all important collision detection and response module.
                    Responsible for providing functionality to check for object
                    collisions on the basis of object vs. object and projectile
                    vs. object.
    notes       :   1) Since object control and storage is done in the object
                       handler, the collision module does not handle the supply
                       of object pointers - that is the job of the handler.
*******************************************************************************/
class collision_module
{
    private:
        /* Anticipated Impact List */
        ac_node* acl_head;
        ac_node* acl_pending;
        
        bool pen_log;                       // Armor penetration logging
        
        /* CD Routines */
        void buildLCSIM(object* obj_one_ptr, object* obj_two_ptr,
            int attachment, GLfloat* matrix);
        
        /* CR Routines */
        void stop_object(object* obj, object* obj_against);
        void go_around_object(object* obj, object* obj_against);
        
        /* Routines */
        cr_data* penetration_calculator(proj_object* proj_ptr, object* obj_ptr, 
            char* slab, float impact_angle);
        float slope_effect(int shell_type, float shell_diameter,
            float td_ratio, float impact_angle);
        float penetration_probability(float pr_ratio);
    
    public:
        collision_module();             // Constructor
        ~collision_module();            // Deconstructor
        
        /* CD Routines */
        bool checkMeshAABB(kVector rayPos, kVector rayDir, int id, int mesh, float t_min, float t_max);
        cd_data* checkMeshPB(kVector rayPos, kVector rayDir, int id, int mesh, float t_min, float t_max);
        ac_node* checkMeshes(object* objOnePtr, object* objTwoPtr, float moveBack, int meshExclude);
        
        /* CR Routines */
        void handle(object* objOnePtr, object* objTwoPtr);
        void handle(ac_node* ACN);
        
        /* Functionality */
        ac_node* buildACN(object* objOnePtr, object* objTwoPtr, cd_data* cddReport);
        void addACN(ac_node* ACN);
        
        /* Mutators */
        void setPenLog(bool enable = true)
            { pen_log = enable; }
        
        /* Base Update Routine */
        void update(float deltaT);
};

extern collision_module cdr;

/* Inline Functionality */

/*******************************************************************************
    function    :   bool cdr_checkCS
    arguments   :   objOnePtr - Pointer to first object
                    objTwoPtr - Pointer to second object
    purpose     :   Collision sphere inline function check. Checks to see if
                    two objects collide using a collision sphere (obj->radius).
    notes       :   Uses the dimensional speed-up hierarchy.
*******************************************************************************/
inline bool cdr_checkCS(object* objOnePtr, object* objTwoPtr)
{
    float distance;
    float dis_x;
    float dis_z;
    float sum_radii;
    
    // Check object one and two are unique
    if(objOnePtr == objTwoPtr)
        return false;
    
    // Compute x and z offset
    dis_x = fabsf(objOnePtr->pos[0] - objTwoPtr->pos[0]);
    dis_z = fabsf(objOnePtr->pos[2] - objTwoPtr->pos[2]);
    
    // Compute radius sum
    sum_radii = fabsf(objOnePtr->radius) + fabsf(objTwoPtr->radius);
    
    // See if the distance offset for x and z are less than or equal to the
    // sum of the radii (for this initial dimension check).
    if(dis_x <= sum_radii && dis_z <= sum_radii)
    {
        // Combine X and Z into one metric
        distance = (dis_x * dis_x) + (dis_z * dis_z);
                        
        sum_radii *= sum_radii;     // Make radius sum in ^2 (Manhattan).
        
        // Check this dimension
        if(distance <= sum_radii)
        {
            // Add in final Y dimension
            distance += ((objOnePtr->pos[1] - objTwoPtr->pos[1])
                            * (objOnePtr->pos[1] - objTwoPtr->pos[1]));
            
            // Full sphere check
            if(distance <= sum_radii)
                return true;
        }
    }
    
    return false;
}

/*******************************************************************************
    function    :   bool cdr_checkIABB
    arguments   :   objOnePtr - Pointer to first object
                    objTwoPtr - Pointer to second object
    purpose     :   Collision inversely aligned bounding box inline function
                    check. Checks to see if  two objects collide using an
                    inverse from one's LCS to the other's LCS and checks the
                    end points against an axis alligned bounding box (AABB).
    notes       :   Inverse must be done to both objects since end points might
                    exist inside an AABB only in one inversed direction.
*******************************************************************************/
inline bool cdr_checkIABB(object* objOnePtr, object* objTwoPtr)
{
    GLfloat inversing_matrix[16];
    kVector front_left;
    kVector front_right;
    kVector rear_left;
    kVector rear_right;
    float* min_size;
    float* max_size;
    
    // Check object one and two are unique
    if(objOnePtr == objTwoPtr)
        return false;
        
    // Switch to modelview matrix and save current matrix onto stack
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    
    // Build LCSIM from object one to object two
    glLoadIdentity();
    glRotatef(-objTwoPtr->roll * radToDeg, 0.0, 0.0, 1.0);
    glRotatef(-((objTwoPtr->dir[1] * radToDeg) - 90.0), 1.0, 0.0, 0.0);
    glRotatef(-objTwoPtr->dir[2] * radToDeg, 0.0, 1.0, 0.0);
    glTranslatef(-objTwoPtr->pos[0], -objTwoPtr->pos[1], -objTwoPtr->pos[2]);
    glMultMatrixf(objOnePtr->hull_matrix);
    glGetFloatv(GL_MODELVIEW_MATRIX, inversing_matrix);
    
    // Get endpoints from model library
    min_size = models.getMinSize(objOnePtr->model_id);
    max_size = models.getMaxSize(objOnePtr->model_id);
    
    // Initialize endpoint position vectors
    front_left = kVector(max_size[0], objOnePtr->size[1]/2.0, max_size[2]);
    front_right = kVector(min_size[0], objOnePtr->size[1]/2.0, max_size[2]);
    rear_left = kVector(max_size[0], objOnePtr->size[1]/2.0, min_size[2]);
    rear_right = kVector(min_size[0], objOnePtr->size[1]/2.0, min_size[2]);
    
    // Transform endpoints based on LCSIM
    front_left.transform((float*)inversing_matrix);
    front_right.transform((float*)inversing_matrix);
    rear_left.transform((float*)inversing_matrix);
    rear_right.transform((float*)inversing_matrix);    
    
    // Get endpoints for AABB
    min_size = models.getMinSize(objTwoPtr->model_id);
    max_size = models.getMaxSize(objTwoPtr->model_id);
    
    // Check front left against the AABB
    if(front_left[0] >= min_size[0]  && front_left[0] <= max_size[0] &&
       front_left[1] >= min_size[1]  && front_left[1] <= max_size[1] &&
       front_left[2] >= min_size[2]  && front_left[2] <= max_size[2])
    {
        glPopMatrix();  // Restore matrix
        return true;
    }
        
    // Check front right against the AABB
    if(front_right[0] >= min_size[0]  && front_right[0] <= max_size[0] &&
       front_right[1] >= min_size[1]  && front_right[1] <= max_size[1] &&
       front_right[2] >= min_size[2]  && front_right[2] <= max_size[2])
    {
        glPopMatrix();  // Restore matrix
        return true;
    }
    
    // Check rear left against the AABB
    if(rear_left[0] >= min_size[0]  && rear_left[0] <= max_size[0] &&
       rear_left[1] >= min_size[1]  && rear_left[1] <= max_size[1] &&
       rear_left[2] >= min_size[2]  && rear_left[2] <= max_size[2])
    {
        glPopMatrix();  // Restore matrix
        return true;
    }
    
    // Check rear right against the AABB
    if(rear_right[0] >= min_size[0]  && rear_right[0] <= max_size[0] &&
       rear_right[1] >= min_size[1]  && rear_right[1] <= max_size[1] &&
       rear_right[2] >= min_size[2]  && rear_right[2] <= max_size[2])
    {
        glPopMatrix();  // Restore matrix
        return true;
    }
    
    // Build LCSIM from object two to object one
    glLoadIdentity();
    glRotatef(-objOnePtr->roll * radToDeg, 0.0, 0.0, 1.0);
    glRotatef(-((objOnePtr->dir[1] * radToDeg) - 90.0), 1.0, 0.0, 0.0);
    glRotatef(-objOnePtr->dir[2] * radToDeg, 0.0, 1.0, 0.0);
    glTranslatef(-objOnePtr->pos[0], -objOnePtr->pos[1], -objOnePtr->pos[2]);
    glMultMatrixf(objTwoPtr->hull_matrix);
    glGetFloatv(GL_MODELVIEW_MATRIX, inversing_matrix);
    
    // Get endpoints from model library
    min_size = models.getMinSize(objTwoPtr->model_id);
    max_size = models.getMaxSize(objTwoPtr->model_id);
    
    // Initialize endpoint position vectors
    front_left = kVector(max_size[0], objTwoPtr->size[1]/2.0, max_size[2]);
    front_right = kVector(min_size[0], objTwoPtr->size[1]/2.0, max_size[2]);
    rear_left = kVector(max_size[0], objTwoPtr->size[1]/2.0, min_size[2]);
    rear_right = kVector(min_size[0], objTwoPtr->size[1]/2.0, min_size[2]);
    
    // Transform endpoints based on LCSIM
    front_left.transform((float*)inversing_matrix);
    front_right.transform((float*)inversing_matrix);
    rear_left.transform((float*)inversing_matrix);
    rear_right.transform((float*)inversing_matrix);    
    
    // Get endpoints for AABB
    min_size = models.getMinSize(objOnePtr->model_id);
    max_size = models.getMaxSize(objOnePtr->model_id);
    
    // Check front left against the AABB
    if(front_left[0] >= min_size[0]  && front_left[0] <= max_size[0] &&
       front_left[1] >= min_size[1]  && front_left[1] <= max_size[1] &&
       front_left[2] >= min_size[2]  && front_left[2] <= max_size[2])
    {
        glPopMatrix();  // Restore matrix
        return true;
    }
        
    // Check front right against the AABB
    if(front_right[0] >= min_size[0]  && front_right[0] <= max_size[0] &&
       front_right[1] >= min_size[1]  && front_right[1] <= max_size[1] &&
       front_right[2] >= min_size[2]  && front_right[2] <= max_size[2])
    {
        glPopMatrix();  // Restore matrix
        return true;
    }
    
    // Check rear left against the AABB
    if(rear_left[0] >= min_size[0]  && rear_left[0] <= max_size[0] &&
       rear_left[1] >= min_size[1]  && rear_left[1] <= max_size[1] &&
       rear_left[2] >= min_size[2]  && rear_left[2] <= max_size[2])
    {
        glPopMatrix();  // Restore matrix
        return true;
    }
    
    // Check rear right against the AABB
    if(rear_right[0] >= min_size[0]  && rear_right[0] <= max_size[0] &&
       rear_right[1] >= min_size[1]  && rear_right[1] <= max_size[1] &&
       rear_right[2] >= min_size[2]  && rear_right[2] <= max_size[2])
    {
        glPopMatrix();  // Restore matrix
        return true;
    }
        
    glPopMatrix();  // Restore matrix
    
    // All possibility of collision has been exhausted, thus no collision.
    return false;
}

#endif
