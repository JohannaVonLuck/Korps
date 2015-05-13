/*******************************************************************************
             Collision Detection & Response Module - Implementation
*******************************************************************************/
#include "main.h"
#include "collision.h"
#include "atg.h"
#include "console.h"
#include "database.h"
#include "effects.h"
#include "object.h"
#include "objhandler.h"
#include "metrics.h"
#include "misc.h"
#include "model.h"
#include "projectile.h"
#include "sounds.h"
#include "tank.h"

/*******************************************************************************
    function    :   collision_module::collision_module
    arguments   :   <none>
    purpose     :   Constructor.
    notes       :   <none>
*******************************************************************************/
collision_module::collision_module()
{
    ofstream fout;
    
    // Initialize some values
    acl_head = NULL;
    acl_pending = NULL;
    pen_log = true;
    
    // Create collision log header
    fout.open("penetration.log");
    fout << "                   -= Korps Penetration Log =-" << endl << endl;
    fout.close();
}

/*******************************************************************************
    function    :   collision_module::~collision_module
    arguments   :   <none>
    purpose     :   Deconstructor.
    notes       :   <none>
*******************************************************************************/
collision_module::~collision_module()
{
    // Kill all the ACNs
    ac_node* curr;
    
    curr = acl_head;
    while(curr)
    {
        acl_head = acl_head->next;
        delete curr;
        curr = acl_head;
    }
    
    curr = acl_pending;
    while(curr)
    {
        acl_pending = acl_pending->next;
        delete curr;
        curr = acl_pending;
    }
}

/*******************************************************************************
    function    :   collision_module::buildLCSIM
    arguments   :   obj_one_ptr - Object that is doing the colliding
                    obj_two_ptr - Object that is being collided with
                    attachment - Attachment type (for object two)
                    matrix - Pointer to matrix to store resultant matrix in
    purpose     :   Builds the local coordinate system inversing matrix between
                    object one's hull matrix to object two's attachment matrix.
    notes       :   Provides the matrix which converts from one's LCS to two's
                    LCS. Required for some of the ops we're doing here.
*******************************************************************************/
void collision_module::buildLCSIM(object* obj_one_ptr, object* obj_two_ptr,
    int attachment, GLfloat* matrix)
{
    tank_object* tank_ptr;
    atg_object* atg_ptr;
    
    // Save modelview matrix stack
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Switch to inversing build based on object type
    switch(obj_two_ptr->obj_type)
    {
        case OBJ_TYPE_TANK:
            // Assign a tank pointer object (makes code cleaner)
            tank_ptr = dynamic_cast<tank_object*>(obj_two_ptr);
            
            // Build for gun (recoil) attach
            if(attachment >= OBJ_ATTACH_GUN_OFF)
            {
                // Gun number (makes code cleaner)
                int gun = attachment - OBJ_ATTACH_GUN_OFF;
                
                // Build
                glTranslatef(0.0, 0.0, tank_ptr->gun[gun].getGunRecoil());
                
                // Update attachment for next level
                attachment = OBJ_ATTACH_GUNMNT_OFF + gun;
            }
            // Build for gun mount (elevate, transverse, pivot) attach
            if(attachment >= OBJ_ATTACH_GUNMNT_OFF)
            {
                // Gun number (makes code cleaner)
                int gun = attachment - OBJ_ATTACH_GUNMNT_OFF;
                
                // Build
                glRotatef(-((tank_ptr->gun[gun].getElevate() * radToDeg) - 90.0), 1.0, 0.0, 0.0);
                if(tank_ptr->gun[gun].getGunAttach() == OBJ_ATTACH_HULL)
                    glRotatef(-(tank_ptr->gun[gun].getTransverse() * radToDeg), 0.0, 1.0, 0.0);
                glTranslatef(-tank_ptr->gun[gun].getGunPivotV()[0], -tank_ptr->gun[gun].getGunPivotV()[1], -tank_ptr->gun[gun].getGunPivotV()[2]);
                
                // Update attachment for next level
                attachment = tank_ptr->gun[gun].getGunAttach();
            }
            // Build for turret (transverse, pivot) attach
            if(attachment >= OBJ_ATTACH_TURRET_OFF)
            {
                // Turret number (makes code cleaner)
                int turret = attachment - OBJ_ATTACH_TURRET_OFF;
                
                // Build
                glRotatef(-(tank_ptr->turret_rotation[turret] * radToDeg), 0.0, 1.0, 0.0);
                glTranslatef(-tank_ptr->turret_pivot[turret][0], -tank_ptr->turret_pivot[turret][1], -tank_ptr->turret_pivot[turret][2]);  
            }
            // Build for base hull (orientation) attach
            glRotatef(-(tank_ptr->roll * radToDeg), 0.0, 0.0, 1.0);
            glRotatef(-((tank_ptr->dir[1] * radToDeg) - 90.0), 1.0, 0.0, 0.0);
            glRotatef(-(tank_ptr->dir[2] * radToDeg), 0.0, 1.0, 0.0);
            glTranslatef(-tank_ptr->pos[0], -tank_ptr->pos[1], -tank_ptr->pos[2]);
            break;
        
        case OBJ_TYPE_ATG:
            // Assign an ATG pointer object (makes code cleaner)
            atg_ptr = dynamic_cast<atg_object*>(obj_two_ptr);
            
            // Build for gun (recoil) attach
            if(attachment >= OBJ_ATTACH_GUN_OFF)
            {
                // Build
                glTranslatef(0.0, 0.0, atg_ptr->gun[0].getGunRecoil());
                
                // Update attachment for next level
                attachment = OBJ_ATTACH_GUNMNT1;
            }
            // Build for gun mount (elevate, transverse, pivot) attach
            if(attachment >= OBJ_ATTACH_GUNMNT_OFF)
            {
                // Build
                glTranslatef(0.0, 0.0, 0.5 * atg_ptr->gun[0].getGunRecoil());
                glRotatef(-((atg_ptr->gun[0].getElevate() * radToDeg) - 90.0), 1.0, 0.0, 0.0);
                glRotatef(-(atg_ptr->gun[0].getTransverse() * radToDeg), 0.0, 1.0, 0.0);
                glTranslatef(-atg_ptr->gun[0].getGunPivotV()[0], -atg_ptr->gun[0].getGunPivotV()[1], -atg_ptr->gun[0].getGunPivotV()[2]);
            }
            // Build for base hull (orientation) attach
            glRotatef(-(atg_ptr->roll * radToDeg), 0.0, 0.0, 1.0);
            glRotatef(-((atg_ptr->dir[1] * radToDeg) - 90.0), 1.0, 0.0, 0.0);
            glRotatef(-(atg_ptr->dir[2] * radToDeg), 0.0, 1.0, 0.0);
            glTranslatef(-atg_ptr->pos[0], -atg_ptr->pos[1], -atg_ptr->pos[2]);
            break;
        
        case OBJ_TYPE_VEHICLE:
            // Do later
        case OBJ_TYPE_ATR:
            // Do later
        case OBJ_TYPE_STATIC:
        default:
            // Build for base hull (orientation) attach
            glRotatef(-obj_two_ptr->roll * radToDeg, 0.0, 0.0, 1.0);
            glRotatef(-((obj_two_ptr->dir[1] * radToDeg) - 90.0), 1.0, 0.0, 0.0);
            glRotatef(-obj_two_ptr->dir[2] * radToDeg, 0.0, 1.0, 0.0);
            glTranslatef(-obj_two_ptr->pos[0], -obj_two_ptr->pos[1], -obj_two_ptr->pos[2]);
            break;
    }
    
    // Multiply by object one's hull matrix, save into *matrix, pop matrix stack
    glMultMatrixf(obj_one_ptr->hull_matrix);
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
    glPopMatrix();
}

/*******************************************************************************
    function    :   collision_module::stop_object
    arguments   :   obj_ptr - object pointer to effecting object
    				obj_against_ptr - object pointer against another object
    purpose     :   Stops object from moving and backs it away from obj_against.
    notes       :   <none>
*******************************************************************************/
void collision_module::stop_object(object* obj_ptr, object* obj_against_ptr)
{
	// Move object back 0.005 units per each call
    obj_ptr->pos += normalized(obj_against_ptr->pos - obj_ptr->pos) * -0.005f;
    
    // Stop object movement based on type
    switch(obj_ptr->obj_type)
    {
        case OBJ_TYPE_TANK:
        case OBJ_TYPE_VEHICLE:
            // Set speed to zero (at all levels)
            (dynamic_cast<moving_object*>(obj_ptr))->motor.setThrottle(0.0);
            (dynamic_cast<moving_object*>(obj_ptr))->motor.setOutputSpeed(0.0);
            (dynamic_cast<moving_object*>(obj_ptr))->linear_vel = 0.0;
            break;
        
        default:
            break;
    }
}

/*******************************************************************************
    function    :   collision_module::go_around_object
    arguments   :   obj_ptr - object pointer to effecting object
    				obj_against_ptr - object pointer against another object
    purpose     :   Creates new path node head to make object "go around"
    				obj_against.
    notes       :   <none>
*******************************************************************************/
void collision_module::go_around_object(object* obj_ptr, object* obj_against_ptr)
{
    waypoint_node* new_head;
    kVector dir;
    
    switch(obj_ptr->obj_type)
    {
        case OBJ_TYPE_TANK:
        case OBJ_TYPE_VEHICLE:
        	// Only create new path head if there are no paths (which is unlikely),
        	// and if the current path head isn't a CR assigned node.
            if((dynamic_cast<moving_object*>(obj_ptr))->wl_head == NULL ||
                !((dynamic_cast<moving_object*>(obj_ptr))->wl_head->modifiers & WP_MOD_CR_ASSIGNED))
            {
	            // Get yaw offset based on obj_against ptr
                dir = vectorIn(obj_against_ptr->pos - obj_ptr->pos, CS_YAW_ONLY);
                dir[2] = dir[2] - obj_ptr->dir[2];
                
                // Normalize offset to -PI to PI (left/right)
                if(dir[2] > PI)
                    dir[2] -= TWOPI;
                if(dir[2] < -PI)
                    dir[2] += TWOPI;
                
                // Cancel any current job of the head waypoint
                if((dynamic_cast<moving_object*>(obj_ptr))->wl_head != NULL &&
                   (dynamic_cast<moving_object*>(obj_ptr))->wl_head->jobID != JOB_NULL)
                {
                    (dynamic_cast<unit_object*>(obj_ptr))->crew.finishJob(
                        (dynamic_cast<moving_object*>(obj_ptr))->wl_head->jobID);
                    (dynamic_cast<moving_object*>(obj_ptr))->wl_head->jobID = JOB_NULL;
                }
                
                // Create new path head node
                new_head = new waypoint_node;
                
                // Determine if obj should take a 90 left or right to go around
                if(dir[2] >= 0.0)
                    new_head->waypoint = obj_ptr->pos + vectorIn(obj_ptr->dir - kVector(0.0, 0.0, PIHALF), CS_CARTESIAN) * 6.0f;
                else
                    new_head->waypoint = obj_ptr->pos + vectorIn(obj_ptr->dir + kVector(0.0, 0.0, PIHALF), CS_CARTESIAN) * 6.0f;
                
                // Get height for y (to make sure everything works out)
                new_head->waypoint[1] = map.getOverlayHeight(
                    new_head->waypoint[0], new_head->waypoint[2]);
                
                // Set the modifiers - important to set CR_ASSIGNED
                new_head->modifiers = WP_MOD_NONE | WP_MOD_FORWARD | WP_MOD_TRANSMITTED | WP_MOD_CR_ASSIGNED;
                
                // Set to no current job (since already transmitted)
                new_head->jobID = JOB_NULL;
            	
                // Set new head for object
                new_head->next = (dynamic_cast<moving_object*>(obj_ptr))->wl_head;
                (dynamic_cast<moving_object*>(obj_ptr))->wl_head = new_head;
                
                // Create new path head node
                new_head = new waypoint_node;
                
                // Set reverse waypoint
                new_head->waypoint = obj_ptr->pos + (vectorIn(obj_ptr->dir, CS_CARTESIAN) * -10.0f);
                
                // Get height for y (to make sure everything works out)
                new_head->waypoint[1] = map.getOverlayHeight(
                    new_head->waypoint[0], new_head->waypoint[2]);
                
                // Set the modifiers - important to set CR_ASSIGNED
                new_head->modifiers = WP_MOD_NONE | WP_MOD_REVERSE | WP_MOD_TRANSMITTED | WP_MOD_CR_ASSIGNED;
                
                // Set to no current job (since already transmitted)
                new_head->jobID = JOB_NULL;
                
                // Set new head for object
                new_head->next = (dynamic_cast<moving_object*>(obj_ptr))->wl_head;
                (dynamic_cast<moving_object*>(obj_ptr))->wl_head = new_head;
            }
            break;
        
        default:
            break;
    }
}

/*******************************************************************************
    function    :   cr_data* collision_module::penetration_calculator
    arguments   :   proj_ptr - Pointer to projectile object
                    obj_ptr - Pointer to object being hit by projectile
                    slab - Armor slab struck by projectile on object
    				impact_angle - Impact angle that slab was struck (degrees)
    purpose     :   Determines what happens to the projectile object against
                    the said slab at the said angle against the said object.
    notes       :   1) The distance traveled is controled via the projectile
                       object, thus this value doesn't need to be passed.
                    2) There are 5 kinds of penetration: Full penetration,
                       partial penetration, major spalling, minor spalling, and
                       no penetration. These specific values are controlled via
                       the modifier value.
                    3) There are 5 kinds of no penetration effects: Ricochet,
                       explode, shatter, disapear/remove, and/or device arm (HE-
                       B/incd.). These are also part of the modifier value.
                    4) Uses a mix between formulas found in "WWII Ballistics"
                       and John Salt's documents (if diameter fix is enabled).
                    5) Spaced armor is always assumed to have RHA first plate.
                    6) Layered armor is always assumed to be same plate type.
                    7) Currently, HEAT rounds are rated either as full
                       penetration or no penetration until more data is avail.
                    8) HESH and API ammunition is not currently supported due
                       to lack of information regarding these types.
                    9) Returns a NEW allocation, which must be deleted later, if
                       not returning NULL.
*******************************************************************************/
cr_data* collision_module::penetration_calculator(proj_object* proj_ptr,
    object* obj_ptr, char* slab, float impact_angle)
{
    char* temp;
    char buffer[128];
    ofstream fout;
    
    cr_data* cr_dr;                     // CR data report
    
    int armor_type;                     // Armor data
    float armor_thickness = 0.0;
    float armor_resistance = 0.0;       
    float first_plate, second_plate;    // For layered/spaced armor calcs
    bool spaced_armor = false;
    bool layered_armor = false;
    
    int shell_type;                     // Shell data
    float shell_diameter;
    float shell_velocity;
    float shell_weight;
    float travel_distance;
    
    float pen_at_pb = 0.0;              // Curve #s
    float pen_curve = 0.0;
    float vel_at_mz = 0.0;
    
    float A, B;                         // Spare variables
    
    // Check for valid passing
    if(!proj_ptr || !obj_ptr || !slab)
    {
        write_error("CR: Penetration Calculator was passed invalid values.");
        return NULL;
    }
    
    // Allocate CR data report
    cr_dr = new cr_data;
    
    // Initialize values for new assignment
    cr_dr->modifiers = CR_MOD_NONE;
    cr_dr->impact_angle = impact_angle;
    cr_dr->impact_velocity = 0.0;
    cr_dr->impact_ke = 0.0;
    cr_dr->td_ratio = 0.0;
    cr_dr->slope_effect = 1.0;
    cr_dr->multipliers = 1.0;
    cr_dr->resistance = 0.0;
    cr_dr->penetration = 0.0;
    cr_dr->pr_ratio = 0.0;
    cr_dr->probability = 0.0;
    cr_dr->result_velocity = 0.0;
    cr_dr->result_ke = 0.0;
    cr_dr->distance_offset = 0.0;
    
    // Set shell travel distance
    travel_distance = proj_ptr->travel_distance + proj_ptr->distance_offset;
    
    /* BEGIN DATA GATHERING */
    
    // Grab armor thickness
    sprintf(buffer, "ARTHCK_%s", slab);
    temp = db.query(obj_ptr->obj_model, buffer);
    if(temp)
    {
        if(strstr(temp, "++") != NULL)      // Spaced armor ++ value
        {
            sscanf(temp, "%f++%f", &second_plate, &first_plate);
            armor_thickness = first_plate + second_plate;
            spaced_armor = true;
        }
        else if(strstr(temp, "+") != NULL)  // Layered armor + value
        {
            sscanf(temp, "%f+%f", &second_plate, &first_plate);
            armor_thickness = first_plate + second_plate;
            layered_armor = true;
        }
        else
            armor_thickness = atof(temp);   // Standard armor value
    }
    else
    {
        sprintf(buffer, "CR: Armor slab thickness for \"%s\" doesn't exist on model \"%s\".",
            slab, obj_ptr->obj_model);
        write_error(buffer);
        delete cr_dr;
        return NULL;
    }
    
    // Grab armor type
    sprintf(buffer, "ARTYPE_%s", slab);
    temp = db.query(obj_ptr->obj_model, buffer);
    if(temp)
    {
        if(strcmp(temp, "RHA") == 0 || strcmp(temp, "rha") == 0)
            armor_type = ARMOR_TYPE_RHA;
        else if(strcmp(temp, "FHS") == 0 || strcmp(temp, "fhs") == 0)
            armor_type = ARMOR_TYPE_FHS;
        else if(strcmp(temp, "CAST") == 0 || strcmp(temp, "cast") == 0)
            armor_type = ARMOR_TYPE_CAST;
        else
        {
            sprintf(buffer, "CR: Invalid armor type \"%s\" for slab \"%s\" on model \"%s\".",
                temp, slab, obj_ptr->obj_model);
            write_error(buffer);
            delete cr_dr;
            return NULL;
        }
    }
    else
    {
        sprintf(buffer, "CR: Armor slab type for \"%s\" doesn't exist on model \"%s\".",
            slab, obj_ptr->obj_model);
        write_error(buffer);
        delete cr_dr;
        return NULL;
    }
    
    // Grab shell type
    shell_type = proj_ptr->type;
    
    // Grab shell diameter (converting from cm to mm)
    shell_diameter = proj_ptr->diameter * 10.0;
    
    // Grab shell penetration curve data
    if(armor_type == ARMOR_TYPE_RHA || armor_type == ARMOR_TYPE_CAST)
    {
        temp = db.query(proj_ptr->obj_model, "PEN_RHA_AT_PB");
        if(temp)
            pen_at_pb = atof(temp);
        else
        {
            sprintf(buffer, "CR: Penetration curves not defined for projectile type \"%s\".",
                proj_ptr->obj_model);
            write_error(buffer);
            delete cr_dr;
            return NULL;
        }
        
        temp = db.query(proj_ptr->obj_model, "PEN_RHA_CURVE");
        if(temp)
            pen_curve = atof(temp);
        else
        {
            sprintf(buffer, "CR: Penetration curves not defined for projectile type \"%s\".",
                proj_ptr->obj_model);
            write_error(buffer);
            delete cr_dr;
            return NULL;
        }
    }
    else if(armor_type == ARMOR_TYPE_FHS)
    {
        temp = db.query(proj_ptr->obj_model, "PEN_FHS_AT_PB");
        if(temp)
            pen_at_pb = atof(temp);
        else
        {
            temp = db.query(proj_ptr->obj_model, "PEN_RHA_AT_PB");
            if(temp)
            {
                pen_at_pb = atof(temp);
                
                // If only the RHA is defined, yet FHS is being attacked, we
                // fill in the "blank" by using the Russian tests which show
                // about a 1.05 gain for capped ammo and 0.87 otherwise. We
                // only do this to the pen_at_pb since curve doesn't matter.
                if(shell_type == AMMO_TYPE_APC || shell_type == AMMO_TYPE_APCBC)
                    pen_at_pb *= 1.05;
                else if(shell_type == AMMO_TYPE_AP || shell_type == AMMO_TYPE_APCR || shell_type == AMMO_TYPE_API)
                    pen_at_pb *= 0.87;
            }
        }
        // If temp is still NULL, then neither FHS or RHA were defined
        if(temp == NULL)
        {
            sprintf(buffer, "CR: Penetration curves not defined for projectile type \"%s\".",
                proj_ptr->obj_model);
            write_error(buffer);
            delete cr_dr;
            return NULL;
        }
        
        temp = db.query(proj_ptr->obj_model, "PEN_FHS_CURVE");
        if(temp)
            pen_curve = atof(temp);
        else
        {
            temp = db.query(proj_ptr->obj_model, "PEN_RHA_CURVE");
            if(temp)
                pen_curve = atof(temp);
        }        
        // If temp is still NULL, then neither FHS or RHA were defined
        if(temp == NULL)
        {
            sprintf(buffer, "CR: Penetration curves not defined for projectile type \"%s\".",
                proj_ptr->obj_model);
            write_error(buffer);
            delete cr_dr;
            return NULL;
        }
    }
    
    // Grab muzzle velocity of shell (velocity rating of shell will not work
    // correctly with the formulas we have here-in if it isn't m.v.).
    temp = db.query(proj_ptr->obj_model, "VELOCITY");
    if(temp)
        vel_at_mz = atof(temp);
    else
    {
        // Check for valid entry in DB
        sprintf(buffer, "CR: Velocity for round \"%s\" not defined.",
            proj_ptr->obj_model);
        write_error(buffer);
        delete cr_dr;
        return NULL;
    }
    
    // Grab shell weight
    temp = db.query(proj_ptr->obj_model, "WEIGHT");
    if(temp)
        shell_weight = atof(temp);
    else
    {
        // Check for valid entry in DB
        sprintf(buffer, "CR: Weight for round \"%s\" not defined.",
            proj_ptr->obj_model);
        write_error(buffer);
        delete cr_dr;
        return NULL;
    }
    
    /* BEGIN DATA CALCULATIONS */
    
    // Note: Spaced armor essentially is, at its heart, two processing steps,
    // thus a portion of the code here splits into two pieces to appropriately
    // handle several situations.
    
    // Calculate thickness penetrated via penetration curve.
    cr_dr->penetration = pen_at_pb * powf(pen_curve, travel_distance);
    
    // Calculate shell striking velocity via penetration curve as well (since
    // penetration and shell velocity are hand-in-hand). Obviously this won't
    // tell us the correct number for HEAT shells (since pen_curve == 1.0).
    // This value is only used for a few minor calculations.
    shell_velocity = vel_at_mz * powf(pen_curve, travel_distance);
    
    // Calculate impacting kinetic energy.
    cr_dr->impact_ke = 0.5 * shell_weight * shell_velocity * shell_velocity;
    
    // Handle spaced armor -> Produce effective resistance as a singular plate.
    if(spaced_armor)
    {
        // Note: Spaced armor is always assumed to have an RHA first plate.
        
        // Apply current external multipliers (if any) for first plate
        first_plate *= cr_dr->multipliers;
        
        // Set deficiency multiplier for cast armor (for second plate only!)
        if(armor_type == ARMOR_TYPE_CAST)
            cr_dr->multipliers *= limit(0.8063 +
                ((second_plate * cr_dr->multipliers) * 0.001238) -
                (shell_diameter * 0.0002628) +
                (((second_plate * cr_dr->multipliers) / shell_diameter)
                * 0.02706), 1.0);
        
        // Apply external multipliers (if any) for second plate
        second_plate *= cr_dr->multipliers;
        
        // Compute T/D ratio (for log)
        cr_dr->td_ratio = (first_plate + second_plate) / shell_diameter;
        
        // Apply and compute slope effect multipliers for each plate (note:
        // save first plate slope effect for later use).
        first_plate *= slope_effect(shell_type, shell_diameter,
            first_plate / shell_diameter, impact_angle);
        second_plate *= slope_effect(shell_type, shell_diameter,
            second_plate / shell_diameter, impact_angle);
        
        // Determine constant value A for spaced equivalency equation
        if(armor_type == ARMOR_TYPE_FHS)
            A = 1.1;    // First plate blunts nose and/or removes cap
        else if((shell_type == AMMO_TYPE_APC || shell_type == AMMO_TYPE_APCBC) &&
            (armor_type == ARMOR_TYPE_RHA || armor_type == ARMOR_TYPE_CAST))
            A = 1.0;    // APC or APCBC against homogenous armor
        else
            A = 1.05;   // First plate blunts nose
        
        // Determine effective armor resistance at 0 degrees via Nathan Okun's
        // formula in "WWII Ballistics".
        cr_dr->resistance = powf(powf(1.15 * first_plate, 1.4) +
            (powf(A, 1.4) * powf(second_plate, 1.4)), 1.0/1.4);
        
        // Calculate slope effect (for log)
        cr_dr->slope_effect = cr_dr->resistance / (cr_dr->td_ratio * shell_diameter);
    }
    else
    {
        if(layered_armor)
        {
            // Handle layered armor -> Produce effective resistance as singular
            // plate by setting up the extra multiplier. Effective resistance is
            // combination of two plates (simple addition) times a multiplier.
            
            // Note: Layered armor is always assumed to have the same armor type
            // for both plates.
            
            // Compute layered armor multiplier
            if(armor_type == ARMOR_TYPE_RHA || armor_type == ARMOR_TYPE_CAST)
            {
                // For RHA (or Cast -> RHA), the equivalent thickness will be
                // slightly less than the added thicknesses. We use two seperate
                // formulas, take their average, and then add 50% from a third
                // formula (US Naval "Rule of Thumb" -> 70% first + second).
                // First Eq: Nathan Okun's equation
                A = (0.50 * (first_plate + second_plate +
                    powf(powf(first_plate, 1.4) + powf(second_plate, 1.4), 1.0/1.4)
                    )) / armor_thickness;
                // Second Eq: Lorrin Bird's equation
                B = 0.3129 * powf(first_plate / second_plate, 0.02527) * powf(
                    (first_plate > second_plate ? first_plate : second_plate),
                    0.2439);
                // Limit B value multiplier (as indicated)
                if(B > 0.96)
                    B = 0.96;
                else if(B < 0.3)
                    B = 0.3;
                // Average out multipliers and add in 50% of US Naval RoT.
                cr_dr->multipliers *=
                    (A + B + (0.5 * (((0.7 * first_plate) + second_plate) /
                    armor_thickness))) / 2.5;
            }
            else if(armor_type == ARMOR_TYPE_FHS)
            {
                // For FHS, the equivalent thickness is actually greater than
                // the combined values. Tests in North Afrika show than the IIIH
                // front hull combination of a 30 FHS plate and a 32 FHS plate
                // would resist like 69 FHS. Also, tests against a IVE show that
                // two 20 FHS plates would resist like 45 FHS. Although this
                // multiplier probably won't work with anything other than
                // these two incidents, we include it here regardless.
                // Note that truly, after the first plate, the cap would be
                // removed or the AP nose blunted, which would relate to a
                // an entire multiplier equation than just this single value...
                // Obviously, if the shell is damaged, without a nose, and hits
                // the second FHS plate, there is going to be a definitive
                // advantage in defeating the round, so at least we're on the
                // right track to some degree.
                cr_dr->multipliers *= 1.1189516129;
            }
        }
        
        // Calculate cast armor deficency multiplier
        if(armor_type == ARMOR_TYPE_CAST)
            cr_dr->multipliers *= limit(0.8063 +
                ((armor_thickness * cr_dr->multipliers) * 0.001238) -
                (shell_diameter * 0.0002628) +
                (((armor_thickness * cr_dr->multipliers) / shell_diameter)
                * 0.02706), 1.0);
        
        // Apply external multipliers (if any)
        armor_resistance = armor_thickness * cr_dr->multipliers;
        
        // Calculate T/D ratio
        cr_dr->td_ratio = armor_resistance / shell_diameter;
        
        // Calculate slope effect multipliers
        cr_dr->slope_effect = slope_effect(shell_type, shell_diameter,
            cr_dr->td_ratio, impact_angle);
        
        // Calculate effective armor resistance at 0 degrees
        cr_dr->resistance = armor_resistance *= cr_dr->slope_effect;
    }
    
    // Calculate P/R ratio
    cr_dr->pr_ratio = cr_dr->penetration / cr_dr->resistance;
    
    /* BEGIN PENETRATION DETERMINATIONS */
    
    // Special case some common instances
    if(spaced_armor && (shell_type == AMMO_TYPE_HEAT || shell_type == AMMO_TYPE_HESH))
    {
        // HEAT/HESH is completely defeated by spaced armor
        cr_dr->modifiers = cr_dr->modifiers | CR_MOD_NO_PENETRATION;
        cr_dr->modifiers = cr_dr->modifiers | CR_MOD_EXPLODE;
    }
    else if(shell_type == AMMO_TYPE_HEAT)
    {
        // Handle penetration of HEAT (for now) seperately
        if(cr_dr->penetration >= cr_dr->resistance - FP_ERROR)
            cr_dr->modifiers = cr_dr->modifiers | CR_MOD_FULL_PEN;
        else
            cr_dr->modifiers = cr_dr->modifiers | CR_MOD_NO_PENETRATION;
        cr_dr->modifiers = cr_dr->modifiers | CR_MOD_EXPLODE;
    }
    else if(shell_type == AMMO_TYPE_HESH || shell_type == AMMO_TYPE_API)
    {
        // Take care of later when more data is available
        cr_dr->modifiers = cr_dr->modifiers | CR_MOD_NO_PENETRATION;
        cr_dr->modifiers = cr_dr->modifiers | CR_MOD_EXPLODE;
    }
    else
    {
        // Calculate penetration probability based on P/R ratio
        cr_dr->probability = penetration_probability(cr_dr->pr_ratio);
        
        // Calculate shatter gap possibility for AP rounds: Only will effect
        // uncapped ammo, only will effect non-german rounds, an impact velocity
        // of at least 2800f/s (~850 m/s), t/d ratio of at least 0.8, a pr
        // ratio between 1.05 and 1.25 (inclusive), and 75% chance from there.
        // Note: Still some argument on the 75% chance - we know.
        if(shell_type == AMMO_TYPE_AP &&
            !(proj_ptr->obj_model[0] == 'D' && proj_ptr->obj_model[1] == 'E') &&
            shell_velocity >= 850.0 - FP_ERROR &&
            cr_dr->td_ratio >= 0.8 - FP_ERROR && cr_dr->pr_ratio >= 1.05 -
            FP_ERROR && cr_dr->pr_ratio <= 1.25 + FP_ERROR && roll(0.75))
        {
            cr_dr->modifiers = cr_dr->modifiers | CR_MOD_NO_PENETRATION;
            cr_dr->modifiers = cr_dr->modifiers | CR_MOD_SHATTER;
        }
        // Determine if it does or does not penetrate
        else if(roll(cr_dr->probability))
        {
            // Penetration
            // Determine full or partial penetration/spalling. The partial pen.
            // formula is not neccessarily correct - but we figure it is a nice
            // added touch. YES - we know.
            if(roll((-2.5 * cr_dr->pr_ratio) + 2.75))
            {
                if(shell_type == AMMO_TYPE_HE || shell_type == AMMO_TYPE_HESH)
                    cr_dr->modifiers = cr_dr->modifiers | CR_MOD_MINOR_SPALLING;
                else
                    cr_dr->modifiers = cr_dr->modifiers | CR_MOD_PARTIAL_PEN;
            }
            else
            {
                if(shell_type == AMMO_TYPE_HE || shell_type == AMMO_TYPE_HESH)
                    cr_dr->modifiers = cr_dr->modifiers | CR_MOD_MAJOR_SPALLING;
                else
                    cr_dr->modifiers = cr_dr->modifiers | CR_MOD_FULL_PEN;
            }
            
            // Set HE/HEAT/HESH shells to explode.
            if(shell_type == AMMO_TYPE_HE || shell_type == AMMO_TYPE_HEAT ||
               shell_type == AMMO_TYPE_HESH)
                cr_dr->modifiers = cr_dr->modifiers | CR_MOD_EXPLODE;
            
            // Check to see if we arm any HE burster device (7mm RHA for DE
            // shells) or ignite an API (incendinary) device (5mm RHA for now).
            if(((proj_ptr->obj_modifiers & AMMO_MOD_HE_BURSTER) &&
                    cr_dr->resistance >= 7.0 - FP_ERROR) ||
                (shell_type == AMMO_TYPE_API &&
                    cr_dr->resistance >= 5.0 - FP_ERROR))
                cr_dr->modifiers = cr_dr->modifiers | CR_MOD_ARM_DEVICE;
            
            // Calculate new resultant KE and velocity
            if(shell_type != AMMO_TYPE_HE && shell_type != AMMO_TYPE_HESH)
            {
                A = (1.0 - (cr_dr->resistance * 0.88) / cr_dr->penetration);
                if(cr_dr->modifiers & CR_MOD_PARTIAL_PEN) A *= 0.5;
                if(A < 0.000102) A = 0.000102;
                if(A > 0.999898) A = 0.999898; 
                cr_dr->result_ke = A * cr_dr->impact_ke;
                cr_dr->result_velocity = sqrt((2.0 * cr_dr->result_ke) / shell_weight);
            }
        }
        else
        {
            // No Penetration
            cr_dr->modifiers = cr_dr->modifiers | CR_MOD_NO_PENETRATION;
            
            // See how much "energy" lost we had, and see if it is equivalent
            // to arm any HE burster device (7mm RHA for DE shells) or ignite
            // an API (incendinary) device (5mm RHA for now).
            /*if(((proj_ptr->obj_modifiers & AMMO_MOD_HE_BURSTER) &&
                    (1.0 - (impact_angle / 90.0)) * cr_dr->resistance >=
                        7.0 - FP_ERROR) ||
                (shell_type == AMMO_TYPE_API &&
                    (1.0 - (impact_angle / 90.0)) * cr_dr->resistance >=
                        5.0 - FP_ERROR))
                cr_dr->modifiers = cr_dr->modifiers | CR_MOD_ARM_DEVICE;*/
            
            if(shell_type == AMMO_TYPE_HE || shell_type == AMMO_TYPE_HEAT ||
               shell_type == AMMO_TYPE_HESH || shell_type == AMMO_TYPE_API)
            {
                // Shell explodes on impact
                cr_dr->modifiers = cr_dr->modifiers | CR_MOD_EXPLODE;
            }
            else if(spaced_armor && roll(penetration_probability(cr_dr->penetration / first_plate)))
            {
                // Shell went through first plate but not second, trick
                // system by saying it shattered instead.
                cr_dr->modifiers = cr_dr->modifiers | CR_MOD_SHATTER;
            }
            else
            {
                // Shell richochets
                cr_dr->modifiers = cr_dr->modifiers | CR_MOD_RICOCHET;
                
                // Use the two special case values in the CR data report.
                // Calculate distance offseting value and resulting velocity
                // for richochets (be sure to check that no log() errors will
                // happen - otherwise something is wrong and we better not
                // risk setting any values for sake of system durability).
                if(!(pen_curve >= 1.0 - FP_ERROR || pen_curve <= FP_ERROR ||
                     pen_at_pb <= FP_ERROR))
                { 
                    if(impact_angle <= FP_ERROR)
                    {
                        // Special case where impact angle is "dead-on"
                        cr_dr->distance_offset = 123456789.0;   // hehe
                        cr_dr->result_velocity = FP_ERROR;      // why not
                    }
                    else
                    {
                        // Compute the dist. offset as a solver routine - solve
                        // for where on the curve distance = KE multiplier *
                        // overall penetration. This will allow us to cut down
                        // the energy of the projectile by a considerable amount
                        // and appropriately set the projectile energy to the
                        // correct amount (we do this in terms of dist. offsets
                        // that are plugged into PENPB*PENCURVE^DIST formulas).
                        cr_dr->distance_offset = (log((
                            (impact_angle / 90.0) * cr_dr->penetration) /
                                pen_at_pb) / log(pen_curve)) - travel_distance;
                        
                        // Compute the resultant velocity after richochet
                        // based on the offset we just computed.
                        cr_dr->result_velocity = vel_at_mz * powf(pen_curve,
                            travel_distance + cr_dr->distance_offset);
                        
                        // Compute resultant KE from above computed.
                        cr_dr->result_ke = 0.5 * shell_weight * cr_dr->result_velocity *
                            cr_dr->result_velocity;
                    }
                }
            }
        }
    }
    
    // Write to log file
    if(pen_log && (shell_diameter >= 15.0 || (shell_diameter < 15.0 && (cr_dr->modifiers & CR_MOD_PEN_OR_SPALL))))
    {
        fout.open("penetration.log", ios::app);
        fout << "Collision at " << (SDL_GetTicks() / 1000.0) << " seconds :" << endl
             << "  Object.................: " << obj_ptr->obj_model << endl
             << "  Projectile.............: " << proj_ptr->obj_model << endl
             << "  Impact Slab............: " << slab << endl
             << "  Impact Angle...........: " << cr_dr->impact_angle << endl
             << "  Armor Thickness (mm)...: " << armor_thickness << endl
             << "  Shell Diameter (mm)....: " << shell_diameter << endl
             << fixed << showpoint << setprecision(4)
             << "  T/D Ratio..............: " << cr_dr->td_ratio << endl
             << "  Slope Effect Multiplier: " << cr_dr->slope_effect << endl
             << "  Other Multipliers......: " << cr_dr->multipliers << endl
             << "  Armor Resistance (mm)..: " << cr_dr->resistance << endl
             << "  Shell Flight Dist. (m).: " << proj_ptr->travel_distance << endl;
        if(shell_type != AMMO_TYPE_HE && shell_type != AMMO_TYPE_HEAT &&
           shell_type != AMMO_TYPE_HESH)
            fout << "  Striking Velocity (m/s): " << shell_velocity << endl;
        fout << "  Shell Penetration (mm).: " << cr_dr->penetration << endl
             << "  P/R Ratio..............: " << cr_dr->pr_ratio << endl
             << "  Penetration Probability: " << cr_dr->probability << endl
             << resetiosflags(ios::fixed | ios::showpoint)
             << "  Penetration Resultant..: ";
        
        // Output penetration resultant
        if(cr_dr->modifiers & CR_MOD_FULL_PEN)
            fout << "Yes (Full Penetration)" << endl << endl;
        else if(cr_dr->modifiers & CR_MOD_PARTIAL_PEN)
            fout << "Yes (Partial Penetration)" << endl << endl;
        else if(cr_dr->modifiers & CR_MOD_MAJOR_SPALLING)
            fout << "Yes (Major Spalling)" << endl << endl;
        else if(cr_dr->modifiers & CR_MOD_MINOR_SPALLING)
            fout << "Yes (Minor Spalling)" << endl << endl;
        else
            fout << "None" << endl << endl;
    }
    
    // Yay! finished
    return cr_dr;
}

/*******************************************************************************
    function    :   float collision_module::slope_effect
    arguments   :   shell_type - Ammo type of projectile
                    shell_diameter - Diameter of projectile (mm)
                    td_ratio - Thickness divided by diameter (mm/mm)
                    impact_angle - Angle of impact (degrees)
    purpose     :   Determines the slope effect multiplier based on the passed
                    parameters.
    notes       :   1) Uses a mix between formulas found in "WWII Ballistics"
                       and John Salt's documents (only if diameter fix enabled).
                    2) Limits slope effect for T/D ratios >= 0.5 to 1.0 as the
                       minimum multiplier possible (keyword: minimum ;)).
*******************************************************************************/
float collision_module::slope_effect(int shell_type, float shell_diameter,
    float td_ratio, float impact_angle)
{
    float f, g;
    float multiplier;
    
    // Calculate slope effect multiplier
    switch(shell_type)
    {
        case AMMO_TYPE_AP:
        case AMMO_TYPE_APBC:    // Standard AP with BC, not Russian APBC.
        case AMMO_TYPE_API:
        case AMMO_TYPE_HE:
            if(impact_angle <= 65.0 + FP_ERROR)
            {
                if(impact_angle <= 40.0 + FP_ERROR)
                {
                    f = 0.95 * powf(2.71828, 0.0000539 * powf(impact_angle, 2.5));
                    g = 0.04433 * powf(2.71828, 0.04867 * impact_angle);
                }
                else if(impact_angle <= 55.0 + FP_ERROR)
                {
                    f = 0.04754 * powf(impact_angle, 0.953);
                    g = 0.02047164 * powf(impact_angle, 0.46471);
                }
                else
                {
                    f = 0.0001675 * powf(impact_angle, 2.3655);
                    g = 0.02047164 * powf(impact_angle, 0.46471);
                }
            }
            else
            {
                // Only angles up to 65.0 are presented with spiffy formulas.
                // These following equations are regressions from Appendix 16.
                f = (0.067087 * (impact_angle * impact_angle)) +
                    (-8.669801 * impact_angle) + 286.520251;
                g = (.0002225714 * (impact_angle * impact_angle)) +
                    (-0.015215 * impact_angle) + 0.970922;
            }
            
            multiplier = (f * powf(td_ratio, g)) + CR_SLOPE_ADDFIX;
            
            // Apply shell diameter fix to formula
            if(game_options.diameter_fix)
                multiplier += fabs((multiplier - 1.0) * CR_SLOPE_DIAFIX_MULTIPLIER *
                    ((-0.0000008948258 * (shell_diameter * shell_diameter * shell_diameter))
                     + (0.0002684203 * (shell_diameter * shell_diameter))
                     - (0.027234 * shell_diameter) + 0.933153));
            break;
        
        case AMMO_TYPE_APC:
        case AMMO_TYPE_APCBC:
        case AMMO_TYPE_HESH:
            if(impact_angle <= 85.0 + FP_ERROR)
            {
                if(impact_angle <= 55.0 + FP_ERROR)
                {
                    f = powf(2.71828, (0.0000408 * powf(impact_angle, 2.5)));
                    g = 0.0101 * powf(2.71828, (0.1313 * powf(impact_angle, 0.8)));
                }
                else if(impact_angle <= 60.0 + FP_ERROR)
                {
                    f = -3.434 + 0.10856 * impact_angle;
                    g = 0.2174 + 0.00046 * impact_angle;
                }
                else if(impact_angle <= 70.0 + FP_ERROR)
                {
                    f = 0.00000518 * powf(impact_angle, 3.25);
                    g = 0.00002123 * powf(impact_angle, 2.295);
                }
                else
                {
                    f = 0.0678 * powf(1.0634, impact_angle);
                    g = 0.1017 * powf(1.0178, impact_angle);
                }
            }
            else
            {
                // Only angles up to 85.0 are presented with spiffy formulas.
                // These following equations are regressions from Appendix 16.
                f = (0.017201 * (impact_angle * impact_angle)) +
                    (-2.14796 * impact_angle) + 71.020794;
                g = (.0001534286 * (impact_angle * impact_angle)) +
                    (-0.016924 * impact_angle) + 0.785494;
            }
            
            multiplier = (f * powf(td_ratio, g)) + CR_SLOPE_ADDFIX;
            
            // Apply shell diameter fix to formula
            if(game_options.diameter_fix)
                multiplier += fabs((multiplier - 1.0) * CR_SLOPE_DIAFIX_MULTIPLIER *
                    ((-0.0000008948258 * (shell_diameter * shell_diameter * shell_diameter))
                     + (0.0002684203 * (shell_diameter * shell_diameter))
                     - (0.027234 * shell_diameter) + 0.933153));
            break;
        
        case AMMO_TYPE_APCR:
            if(impact_angle <= 25.0 + FP_ERROR)
                multiplier =
                    1.0000 * powf(2.71828, 0.0001727 * powf(impact_angle, 2.2));
            else
                multiplier =
                    0.7277 * powf(2.71828, 0.003787 * powf(impact_angle, 1.5));
            break;
        
        case AMMO_TYPE_HEAT:
            multiplier = 1.0 / cos(impact_angle * degToRad);
            break;
        
        default:
            multiplier = 1.0;
            break;
    }
    
    // Limit slope effect for T/D ratios higher than 0.5
    if(multiplier < 1.0 && td_ratio > 0.5 + FP_ERROR)
        multiplier = 1.0;
    
    return multiplier;
}

/*******************************************************************************
    function    :   float collision_module::penetration_probability
    arguments   :   pr_ratio - Penetration to resistance ratio (mm/mm)
    purpose     :   Determines the percentage chance of a penetration given
                    the pr_ratio (chance between 0.0 to 1.0).
    notes       :   1) Follows US test data.
                    2) A pr_ratio of 1.0 follows to 50% success criteria.
*******************************************************************************/
float collision_module::penetration_probability(float pr_ratio)
{
    float probability = 0.0;
    
    if(game_options.pen_system == CR_PEN_SYSTEM_US)
    {
        if(pr_ratio <= 0.88 + FP_ERROR)                     // No possibility
            probability = 0.0;
        else if(pr_ratio < 0.92 - FP_ERROR)                 // Handle small line
            probability = ((40.0 * pr_ratio) - 35.0) / 100.0;
        else if(pr_ratio < 1.00 - FP_ERROR)                 // Cubic regression
            probability = ((6082.251082 * (pr_ratio * pr_ratio)) 
                           - (11077.922078 * pr_ratio)
                           + 5045.792208) / 100.0;
        else if(pr_ratio >= 1.00 - FP_ERROR && pr_ratio <= 1.00 + FP_ERROR)
            probability = 0.5;                              // 50% chance mark
        else if(pr_ratio < 1.09 - FP_ERROR)                 // Cubic regression
            probability = ((-5946.969697 * (pr_ratio * pr_ratio))
                           + (12971.590909 * pr_ratio)
                           - 6974.666667) / 100.0;
        else if(pr_ratio < 1.12 - FP_ERROR)                 // Handle small line
            probability = ((40.0 * pr_ratio) + 55.0) / 100.0;
        else                                                // Full possibility
            probability = 1.0;
    }
    else if(game_options.pen_system == CR_PEN_SYSTEM_RU)
    {
        // Fill in later
        probability = ((float)rand() / (float)RAND_MAX);
    }
    else
    {
        // Produce error
        write_error("CR: Penetration probability model not supported.");
        return 0.0;
    }
    
    return probability;    
}

/*******************************************************************************
    function    :   collision_module::checkMeshAABB
    arguments   :   rayPos - Ray start position
                    rayDir - Ray direction (cartesian)
                    id - Model library ID #
                    mesh - Model library mesh #
                    t_min - Cutoff value for T (no T less than are "passed")
                    t_max - Cutoff value for T (no T greater than are "passed")
    purpose     :   Performs the speed-up to test against ray striking the
                    AABB of a mesh before commiting to a PB test.
    notes       :   <none>
*******************************************************************************/
bool collision_module::checkMeshAABB(kVector rayPos, kVector rayDir,
    int id, int mesh, float t_min, float t_max)
{
    float t;
    kVector intercept;
    float* min;
    float* max;
    
    // Check for mesh ID
    if(mesh == -1)
        return false;
    
    // Grab min and max values for AABB for this mesh
    min = models.getMinMeshSize(id, mesh);
    max = models.getMaxMeshSize(id, mesh);
    
    // Test FORWARD Z
    if(fabsf(rayDir[2]) > FP_ERROR)
    {
        t = (max[2] - rayPos[2]) / rayDir[2];
        if(t > t_min && t < t_max)
        {
            intercept[0] = rayPos[0] + (rayDir[0] * t);
            intercept[1] = rayPos[1] + (rayDir[1] * t);
            if(intercept[0] >= min[0] - FP_ERROR &&
               intercept[0] <= max[0] + FP_ERROR &&
               intercept[1] >= min[1] - FP_ERROR &&
               intercept[1] <= max[1] + FP_ERROR)
                return true;
        }
    }
    
    // Test RIGHTWARD X
    if(fabsf(rayDir[0]) > FP_ERROR)
    {
        t = (max[0] - rayPos[0]) / rayDir[0];
        if(t > t_min && t < t_max)
        {
            intercept[1] = rayPos[1] + (rayDir[1] * t);
            intercept[2] = rayPos[2] + (rayDir[2] * t);
            if(intercept[1] >= min[1] - FP_ERROR &&
               intercept[1] <= max[1] + FP_ERROR &&
               intercept[2] >= min[2] - FP_ERROR &&
               intercept[2] <= max[2] + FP_ERROR)
                return true;
        }
    }
    
    // Test TOPMOST Y
    if(fabsf(rayDir[1]) > FP_ERROR)
    {
        t = (max[1] - rayPos[1]) / rayDir[1];
        if(t > t_min && t < t_max)
        {
            intercept[0] = rayPos[0] + (rayDir[0] * t);
            intercept[2] = rayPos[2] + (rayDir[2] * t);
            if(intercept[0] >= min[0] - FP_ERROR &&
               intercept[0] <= max[0] + FP_ERROR &&
               intercept[2] >= min[2] - FP_ERROR &&
               intercept[2] <= max[2] + FP_ERROR)
                return true;
        }
    }
    
    // Test REARWARD Z
    if(fabsf(rayDir[2]) > FP_ERROR)
    {
        t = (min[2] - rayPos[2]) / rayDir[2];
        if(t > t_min && t < t_max)
        {
            intercept[0] = rayPos[0] + (rayDir[0] * t);
            intercept[1] = rayPos[1] + (rayDir[1] * t);
            if(intercept[0] >= min[0] - FP_ERROR &&
               intercept[0] <= max[0] + FP_ERROR &&
               intercept[1] >= min[1] - FP_ERROR &&
               intercept[1] <= max[1] + FP_ERROR)
                return true;
        }
    }
    
    // Test LEFTWARD X
    if(fabsf(rayDir[0]) > FP_ERROR)
    {
        t = (min[0] - rayPos[0]) / rayDir[0];
        if(t > t_min && t < t_max)
        {
            intercept[1] = rayPos[1] + (rayDir[1] * t);
            intercept[2] = rayPos[2] + (rayDir[2] * t);
            if(intercept[1] >= min[1] - FP_ERROR &&
               intercept[1] <= max[1] + FP_ERROR &&
               intercept[2] >= min[2] - FP_ERROR &&
               intercept[2] <= max[2] + FP_ERROR)
                return true;
        }
    }
    
    // Test BOTTOMMOST Y
    if(fabsf(rayDir[1]) > FP_ERROR)
    {
        t = (min[1] - rayPos[1]) / rayDir[1];
        if(t > t_min && t < t_max)
        {
            intercept[0] = rayPos[0] + (rayDir[0] * t);
            intercept[2] = rayPos[2] + (rayDir[2] * t);
            if(intercept[0] >= min[0] - FP_ERROR &&
               intercept[0] <= max[0] + FP_ERROR &&
               intercept[2] >= min[2] - FP_ERROR &&
               intercept[2] <= max[2] + FP_ERROR)
                return true;
        }
    }
    
    // AABB was not hit - no polys could possibly be hit
    return false;
}

/*******************************************************************************
    function    :   collision_module::checkMeshPB
    arguments   :   rayPos - Ray start position
                    rayDir - Ray direction (cartesian)
                    id - Model library ID #
                    mesh - Model library mesh #
                    t_min - Cutoff value for T (no T less than are "passed")
                    t_max - Cutoff value for T (no T greater than are "passed")
    purpose     :   Performs the incredibly computational polygon based
                    collision check against the model library mesh.
    notes       :   <none>
*******************************************************************************/
cd_data* collision_module::checkMeshPB(kVector rayPos, kVector rayDir,
    int id, int mesh, float t_min, float t_max)
{
    int i;
    cd_data* cd_dr = NULL;
    int vertex_count = models.getVertexCount(id, mesh);
    GLfloat** vertex_data = models.getVertexData(id, mesh);
    GLfloat** normal_data = models.getNormalData(id, mesh);
    GLfloat* d_data = models.getDistanceData(id, mesh);
    float t = CD_T_START;
    float t_curr;
    float t_denom;
    kVector t_inter;
    kVector angle_one, angle_two, angle_three;
    int vertex_inter = 0;
    
    // Phase 1: Determine point of intersection on the infinite hyperplane
    //          formed by the plane equation fitted to the polygon.
    
    // Hi ho hi ho through each vertex we go
    for(i = 0; i < vertex_count; i += 3)
    {
        // Calculate t_denom, which will tell us if plane is orthogonal to ray
        t_denom = dotProduct(kVector(normal_data[i]), rayDir);
        
        // Check for orthogonal
        if(fabsf(t_denom) > FP_ERROR)
        {
            // Calculate t value
            t_curr = -(dotProduct(kVector(normal_data[i]), rayPos) + d_data[i])
                / t_denom;
            
            // Check for better t value then stored (and in area)
            if(t_curr < t && t_curr > t_min && t_curr < t_max)
            {
                // Phase 2: Check to see if angles formed between end points
                //          and intersection point add up to 360.0 degrees.
                
                // Calculate intercept point
                t_inter = rayPos + (rayDir * t_curr);
                
                // Slight speed up to determine angle between two vectors
                // includes pre-normalizing everybodys asses.
                angle_one = normalized(kVector(vertex_data[i]) - t_inter);
                angle_two = normalized(kVector(vertex_data[i+1]) - t_inter);
                angle_three = normalized(kVector(vertex_data[i+2]) - t_inter);
                
                // Angle measurement
                if(fabsf(TWOPI - (
                    angleBetweenNormals(angle_one, angle_two) +
                    angleBetweenNormals(angle_two, angle_three) +
                    angleBetweenNormals(angle_three, angle_one)
                    )) <= FP_ERROR)
                {
                    // T found
                    t = t_curr;
                    
                    // Create cdc data report
                    if(cd_dr == NULL)
                        cd_dr = new cd_data;
                    
                    // Record which vertex we were on
                    vertex_inter = i;
                }
            }
        }
    }
    
    // Create CD data report
    if(cd_dr != NULL)
    {
        // Copy over values to CD data report
        cd_dr->rayPos = rayPos;
        cd_dr->rayDir = rayDir;
        cd_dr->id = id;
        cd_dr->mesh = mesh;
        cd_dr->attachment = OBJ_ATTACH_HULL;     // Temporary value
        cd_dr->t = t;
        cd_dr->impactPoint = rayPos + (rayDir * t);
        // Determine impact angle (0 to PI/90)
        cd_dr->impactAngle = fabsf(PI - 
            fabsf(angleBetweenNormals(kVector(normal_data[vertex_inter]),
            rayDir)));
        // Determine reflection vector
        cd_dr->reflection = rayDir -
            (kVector(normal_data[vertex_inter]) * 2.0f * dotProduct(rayDir,
                kVector(normal_data[vertex_inter])));
        
        // Handle situations where the impact angle is greater than PI/2.
        if(cd_dr->impactAngle > PIHALF)
            cd_dr->impactAngle = fabsf(PI - cd_dr->impactAngle);
    }
    
    // Handle situations where the CDD report comes in as saying the impact
    // angle was greater than PI/2, in which case something REALLY crazy is
    // going on to cause such a thing.
    if(cd_dr && cd_dr->impactAngle > PIHALF)
    {
        delete cd_dr;
        cd_dr = NULL;
    }
    
    return cd_dr;
}

/*******************************************************************************
    function    :   int check_mesh
    arguments   :   A whole shit load of values that are better left untouched.
    purpose     :   Performs mesh checking on the passed data. Done incredibly
                    often so this function is here to make things look nicer.
                    Returns (int) 1 for collision found, otherwise 0.
    notes       :   <none>
*******************************************************************************/
inline int check_mesh(kVector ray_pos, kVector ray_dir, int id, int mesh,
    int attach, float &t_best, cd_data* &cd_dr,
    cd_data* &cd_dr_best, int mesh_exclude)
{
    cd_dr = NULL;
    
    if(mesh != -1 && mesh != mesh_exclude)  // Make sure mesh checks out
    {
        // Check against the AABB of the mesh
        if(cdr.checkMeshAABB(ray_pos, ray_dir, id, mesh, FP_ERROR, t_best))
        {
            // Do a polygon based check against the mesh
            cd_dr = cdr.checkMeshPB(ray_pos, ray_dir, id, mesh, FP_ERROR, t_best);
            if(cd_dr != NULL)
            {
                cd_dr->attachment = attach;    // Set the attachemnt value
                if(cd_dr->t < t_best)          // Check for best T value
                {
                    if(cd_dr_best != NULL)
                        delete cd_dr_best;
                    t_best = cd_dr->t;
                    cd_dr_best = cd_dr;
                }
                else
                    delete cd_dr;              // Won't change its ptr
            }
        }
    }
    
    if(cd_dr != NULL)
        return 1;
    return 0;
}

/*******************************************************************************
    function    :   check_mesh
    arguments   :   A whole shit load of values that are better left untouched.
    purpose     :   Performs mesh checking on the passed data. Done incredibly
                    often so this function is here to make things look nicer.
                    Returns (int) 1 for collision found, otherwise 0.
    notes       :   <none>
*******************************************************************************/
inline int check_mesh(kVector ray_pos, kVector ray_dir, int id, char* slab,
    int attach, float &t_best, cd_data* &cd_dr,
    cd_data* &cd_dr_best, int mesh_exclude)
{
    cd_dr = NULL;
    
    int mesh = models.getMeshID(id, slab);  // Get mesh ID number
    
    if(mesh != -1 && mesh != mesh_exclude)  // Make sure mesh checks out
    {
        // Check against the AABB of the mesh
        if(cdr.checkMeshAABB(ray_pos, ray_dir, id, mesh, FP_ERROR, t_best))
        {
            // Do a polygon based check against the mesh
            cd_dr = cdr.checkMeshPB(ray_pos, ray_dir, id, mesh, FP_ERROR, t_best);
            if(cd_dr != NULL)
            {
                cd_dr->attachment = attach;      // Set the attachemnt value
                if(cd_dr->t < t_best)            // Check for best T value
                {
                    if(cd_dr_best != NULL)
                        delete cd_dr_best;
                    t_best = cd_dr->t;
                    cd_dr_best = cd_dr;
                }
                else
                    delete cd_dr;
            }
        }
    }
    
    if(cd_dr != NULL)
        return 1;
    return 0;
}

/*******************************************************************************
    function    :   collision_module::checkMeshes
    arguments   :   objOnePtr - Object one which is colliding into object two
                    objTwoPtr - Object two which is being collided into by one
                    moveBack - How much the object should be "backed off"
                               slightly to correctly perform CDR onto (which
                               effectively sets the t minimum value).
                    meshExclude - Excludes mesh specified from testing. If all
                                  meshes wish to be tested against, pass the
                                  value of CD_EXCLUDE_NO_MESHES.
    purpose     :   Checks the object meshes between the two objects in an
                    efficient and heuristic manner to determine which mesh
                    is going to be collided with. Returns a newly created ACN
                    with collision data report held within if penetration does
                    occur, otherwise returns NULL. ACN is used in junction with
                    an addACN call afterwords (ACN is returned to determine
                    collision or not to the calling system).
    notes       :   NOTICE: Due to the absoluely insane nature of the way things
                            need to be checked, comments are left to a minimum
                            since even with them it would still be confusing.
                    1) Move back value is associated with the fact that when
                       we test against our meshes, we either test against
                       all meshes infront of the current position, or better
                       yet, back off the test object to account for system timer
                       resolution errors associated with object overshoot,
                       e.g. when the test object "goes into" the other object.
                       Default value should be set to CD_T_START, otherwise
                       any other value is acceptable. Synonomous with t_min.
                    2) An ACN is returned, that is an Anticipated Collision
                       Node, a node which the CDR system can use to predict
                       collisions and respond to them at the appropriate time
                       if added into the system after the function returns.
                       The ACN is returned so that the calling system can be
                       notified of collision (ACN != NULL) or not (ACN == NULL).
                    3) The returned value is a NEW allocation, unless of course
                       a collision does not happen, in which NULL is returned.
                       addACN will effectively handle the deallocation from
                       that point - otherwise its the calling system's problem.
*******************************************************************************/
ac_node* collision_module::checkMeshes(object* objOnePtr, object* objTwoPtr,
    float moveBack = CD_T_OFFSET, int meshExclude = CD_EXCLUDE_NO_MESHES)
{
    cd_data* cd_dr = NULL;
    cd_data* cd_dr_best = NULL;
    unsigned short cd_modifiers = CD_MOD_NONE;
    
    float t_best = CD_T_START;                      // Starts at t_max
    
    kVector ray_pos[OBJ_ATTACH_MAX], ray_dir[OBJ_ATTACH_MAX];
    GLfloat inversing_matrix[OBJ_ATTACH_MAX][16];
    
    char* temp;
    char buffer[128];
    
    int id = objTwoPtr->model_id;
    int mesh;
    
    int ccnt = 0;
    int i;
    
    /* Heuristic Variables */
    float t_reference;
    kVector p_reference;
    kVector p_ext_reference[2];
    float uphull_base = 0.0;
    float turret_base = 0.0;
    kVector incoming_dir[OBJ_TURRET_MAX + 1];
    
    bool turrets_checked = false;
    int attach;
    
    tank_object* tank_ptr;              // For ease
    atg_object* atg_ptr;                // "   "
    
    // Set up moveBack
    moveBack = fabsf(moveBack);
    
    // Run checkMeshes
    switch(objOnePtr->obj_type)
    {
        case OBJ_TYPE_PROJECTILE:
            switch(objTwoPtr->obj_type)
            {
                case OBJ_TYPE_TANK:
                    tank_ptr = dynamic_cast<tank_object*>(objTwoPtr);
                    attach = OBJ_ATTACH_HULL;
                    // Build inversing matrix for hull attachment
                    buildLCSIM(objOnePtr, objTwoPtr, attach, inversing_matrix[attach]);
                    
                    // Transform projectile from its LCS to tank's LCS.
                    ray_pos[attach] = kVector(0.0, -moveBack, 0.0);
                    ray_dir[attach] = kVector(0.0, CD_T_OFFSET, 0.0);
                    ray_pos[attach].transform((float*)inversing_matrix[attach]);
                    ray_dir[attach].transform((float*)inversing_matrix[attach]);
                    ray_dir[attach] = normalized(ray_dir[attach] - ray_pos[attach]);
                    
                    // Form heuristic values from upper hull base Y value
                    temp = db.query(objTwoPtr->obj_model, "CDH_UP_HULL");
                    if(temp)
                        uphull_base = atof(temp);
                    
                    // Form heuristic values from turret base Y value
                    temp = db.query(objTwoPtr->obj_model, "CDH_TURRET");
                    if(temp)
                        turret_base = atof(temp);
                    
                    // Form heuristic values from incoming direction
                    incoming_dir[attach] = vectorIn(ray_pos[attach], CS_SPHERICAL);
                    incoming_dir[attach][1] *= radToDeg;
                    incoming_dir[attach][2] *= radToDeg;
                    
                    // Form heuristic reference point from incoming direction
                    if(incoming_dir[attach][1] < 45.0)
                        t_reference = (uphull_base - ray_pos[attach][1]) / ray_dir[attach][1];
                    else if(incoming_dir[attach][2] < 45.0 || incoming_dir[attach][2] > 315.0 ||
                        (incoming_dir[attach][2] > 135.0 && incoming_dir[attach][2] < 225.0))
                        t_reference = -ray_pos[attach][2] / ray_dir[attach][2];
                    else
                        t_reference = -ray_pos[attach][0] / ray_dir[attach][0];
                    
                    p_reference = ray_pos[attach] + (ray_dir[attach] * t_reference);
                    p_ext_reference[0] = ray_pos[attach] + (ray_dir[attach] * (t_reference - objTwoPtr->radius));
                    p_ext_reference[1] = ray_pos[attach] + (ray_dir[attach] * (t_reference + objTwoPtr->radius));
                    
                    // Check gun mantlets and guns
                    for(i = 0; i < tank_ptr->gun_count; i++)
                    {
                        attach = OBJ_ATTACH_GUNMNT_OFF + i;
                        sprintf(buffer, "GUN_MANT%i", i+1);
                        mesh = models.getMeshID(id, buffer);
                        if(mesh != -1 && mesh != meshExclude)
                        {
                            buildLCSIM(objOnePtr, objTwoPtr, attach, inversing_matrix[attach]);
                            ray_pos[attach] = kVector(0.0, -moveBack, 0.0);
                            ray_dir[attach] = kVector(0.0, CD_T_OFFSET, 0.0);
                            ray_pos[attach].transform((float*)inversing_matrix[attach]);
                            ray_dir[attach].transform((float*)inversing_matrix[attach]);
                            ray_dir[attach] = normalized(ray_dir[attach] - ray_pos[attach]);
                            check_mesh(ray_pos[attach], ray_dir[attach], id, mesh, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                        }
                        attach = OBJ_ATTACH_GUN_OFF + i;
                        sprintf(buffer, "GUN%i", i+1);
                        mesh = models.getMeshID(id, buffer);
                        if(mesh != -1 && mesh != meshExclude)
                        {
                            buildLCSIM(objOnePtr, objTwoPtr, attach, inversing_matrix[attach]);
                            ray_pos[attach] = kVector(0.0, -moveBack, 0.0);
                            ray_dir[attach] = kVector(0.0, CD_T_OFFSET, 0.0);
                            ray_pos[attach].transform((float*)inversing_matrix[attach]);
                            ray_dir[attach].transform((float*)inversing_matrix[attach]);
                            ray_dir[attach] = normalized(ray_dir[attach] - ray_pos[attach]);
                            check_mesh(ray_pos[attach], ray_dir[attach], id, mesh, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                        }
                    }
                    
                    // Determination if we should at least try and check the turrets first thing
                    if(turret_base != 0.0 && tank_ptr->turret_count > 0 && p_reference[1] >= turret_base - FP_ERROR)
                    {
                        turrets_checked = true;
                        
                        // Check each turret
                        for(i = 0; i < tank_ptr->turret_count; i++)
                        {
                            attach = OBJ_ATTACH_TURRET_OFF + i;
                            buildLCSIM(objOnePtr, objTwoPtr, attach, inversing_matrix[attach]);
                            ray_pos[attach] = kVector(0.0, -moveBack, 0.0);
                            ray_dir[attach] = kVector(0.0, CD_T_OFFSET, 0.0);
                            ray_pos[attach].transform((float*)inversing_matrix[attach]);
                            ray_dir[attach].transform((float*)inversing_matrix[attach]);
                            ray_dir[attach] = normalized(ray_dir[attach] - ray_pos[attach]);
                            
                            incoming_dir[attach] = vectorIn(ray_pos[attach], CS_SPHERICAL);
                            incoming_dir[attach][1] *= radToDeg;
                            incoming_dir[attach][2] *= radToDeg;
                            
                            if(incoming_dir[attach][2] < 45.0 || incoming_dir[attach][2] > 315.0)
                            {
                                // Forward approach
                                cd_modifiers = cd_modifiers | CD_MOD_CK_TURRET_FRONT;
                                sprintf(buffer, "FR_TURRET%i", i+1);
                                ccnt = check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                if(ccnt == 0)
                                {
                                    sprintf(buffer, "LF_TURRET%i", i+1);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    sprintf(buffer, "RG_TURRET%i", i+1);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                }
                            }
                            else if(incoming_dir[attach][2] > 135.0 && incoming_dir[attach][2] < 225.0)
                            {
                                // Rear approach
                                cd_modifiers = cd_modifiers | CD_MOD_CK_TURRET_REAR;
                                sprintf(buffer, "RR_TURRET%i", i+1);
                                ccnt = check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                if(ccnt == 0)
                                {
                                    sprintf(buffer, "LF_TURRET%i", i+1);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    sprintf(buffer, "RG_TURRET%i", i+1);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                }
                            }
                            else if(incoming_dir[attach][2] >= 45.0 && incoming_dir[attach][2] <= 225.0)
                            {
                                // Left approach
                                cd_modifiers = cd_modifiers | CD_MOD_CK_TURRET_LEFT;
                                sprintf(buffer, "LF_TURRET%i", i+1);
                                ccnt = check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                if(ccnt == 0)
                                {
                                    sprintf(buffer, "FR_TURRET%i", i+1);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    sprintf(buffer, "RR_TURRET%i", i+1);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                }
                            }
                            else
                            {
                                // Right approach
                                cd_modifiers = cd_modifiers | CD_MOD_CK_TURRET_RIGHT;
                                sprintf(buffer, "RG_TURRET%i", i+1);
                                ccnt = check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                if(ccnt == 0)
                                {
                                    sprintf(buffer, "FR_TURRET%i", i+1);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    sprintf(buffer, "RR_TURRET%i", i+1);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                }
                            }
                            
                            if(ray_dir[attach][1] < 0.1)
                            {
                                sprintf(buffer, "TP_TURRET%i", i+1);
                                check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                            }
                            
                            if(tank_ptr->cupola_attach == attach)
                               check_mesh(ray_pos[attach], ray_dir[attach], id, "CUPOLA", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                            
                            // Check EXT_CREW special
                            if(tank_ptr->ext_crew_attach == attach)
                            {
                                mesh = models.getMeshID(id, "EXT_CREW");  // Get mesh ID number
                                if(mesh != -1 && mesh != meshExclude)  // Make sure mesh checks out
                                {
                                    // Check against the AABB of the mesh
                                    if(checkMeshAABB(ray_pos[attach], ray_dir[attach], id, mesh, FP_ERROR, t_best))
                                        cd_modifiers = cd_modifiers | CD_MOD_EXT_CREW_HIT;
                                }
                            }
                            
                            // Check EXT_AMMO special
                            if(tank_ptr->ext_ammo_attach == attach)
                            {
                                mesh = models.getMeshID(id, "EXT_AMMO");  // Get mesh ID number
                                if(mesh != -1 && mesh != meshExclude)  // Make sure mesh checks out
                                {
                                    // Check against the AABB of the mesh
                                    if(checkMeshAABB(ray_pos[attach], ray_dir[attach], id, mesh, FP_ERROR, t_best))
                                        cd_modifiers = cd_modifiers | CD_MOD_EXT_AMMO_HIT;
                                }
                            }
                        }
                    }
                    // End turret checking
                    
                    // Check hull
                    if(tank_ptr->turret_count == 0 || p_reference[1] < turret_base || p_ext_reference[0][1] < turret_base || p_ext_reference[1][1] < turret_base)
                    {
                        attach = OBJ_ATTACH_HULL;
                        
                        if(incoming_dir[attach][2] < 45.0 || incoming_dir[attach][2] > 315.0)
                        {
                            // Forward approach
                            cd_modifiers = cd_modifiers | CD_MOD_CK_HULL_FRONT;
                            if(p_reference[1] >= uphull_base - FP_ERROR)
                            {
                                ccnt = check_mesh(ray_pos[attach], ray_dir[attach], id, "FR_UP_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                if(ccnt == 0)
                                {
                                    ccnt += check_mesh(ray_pos[attach], ray_dir[attach], id, "GLACIS", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    ccnt += check_mesh(ray_pos[attach], ray_dir[attach], id, "FR_HL_NOSE", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    ccnt += check_mesh(ray_pos[attach], ray_dir[attach], id, "FR_LW_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                }
                                if(ccnt == 0)
                                {
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "TRACK_L", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "TRACK_R", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "LF_UP_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "RG_UP_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "LF_LW_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "RG_LW_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                }
                            }
                            else
                            {
                                ccnt = check_mesh(ray_pos[attach], ray_dir[attach], id, "FR_LW_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                if(ccnt == 0)
                                {
                                    ccnt += check_mesh(ray_pos[attach], ray_dir[attach], id, "FR_HL_NOSE", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    ccnt += check_mesh(ray_pos[attach], ray_dir[attach], id, "GLACIS", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    ccnt += check_mesh(ray_pos[attach], ray_dir[attach], id, "FR_UP_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                }
                                if(ccnt == 0)
                                {
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "TRACK_L", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "TRACK_R", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "LF_LW_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "RG_LW_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "LF_UP_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "RG_UP_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                }
                            }
                        }
                        else if(incoming_dir[attach][2] > 135.0 && incoming_dir[attach][2] < 225.0)
                        {
                            // Rear approach
                            cd_modifiers = cd_modifiers | CD_MOD_CK_HULL_REAR;
                            if(p_reference[1] >= uphull_base - FP_ERROR)
                            {
                                ccnt = check_mesh(ray_pos[attach], ray_dir[attach], id, "RR_UP_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                if(ccnt == 0)
                                    ccnt += check_mesh(ray_pos[attach], ray_dir[attach], id, "RR_LW_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                if(ccnt == 0)
                                {
                                    ccnt += check_mesh(ray_pos[attach], ray_dir[attach], id, "LF_UP_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    ccnt += check_mesh(ray_pos[attach], ray_dir[attach], id, "RG_UP_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    if(ccnt == 0)
                                    {
                                        check_mesh(ray_pos[attach], ray_dir[attach], id, "TRACK_L", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                        check_mesh(ray_pos[attach], ray_dir[attach], id, "TRACK_R", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                        check_mesh(ray_pos[attach], ray_dir[attach], id, "LF_LW_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                        check_mesh(ray_pos[attach], ray_dir[attach], id, "RG_LW_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    }
                                }
                            }
                            else
                            {
                                ccnt = check_mesh(ray_pos[attach], ray_dir[attach], id, "RR_LW_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                if(ccnt == 0)
                                    ccnt += check_mesh(ray_pos[attach], ray_dir[attach], id, "RR_UP_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                if(ccnt == 0)
                                {
                                    ccnt += check_mesh(ray_pos[attach], ray_dir[attach], id, "TRACK_L", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    ccnt += check_mesh(ray_pos[attach], ray_dir[attach], id, "TRACK_R", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    ccnt += check_mesh(ray_pos[attach], ray_dir[attach], id, "LF_LW_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    ccnt += check_mesh(ray_pos[attach], ray_dir[attach], id, "RG_LW_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    if(ccnt == 0)
                                    {
                                        check_mesh(ray_pos[attach], ray_dir[attach], id, "LF_UP_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                        check_mesh(ray_pos[attach], ray_dir[attach], id, "RG_UP_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    }
                                }
                            }
                        }
                        else if(incoming_dir[attach][2] >= 45.0 && incoming_dir[attach][2] <= 225.0)
                        {
                            // Left approach
                            cd_modifiers = cd_modifiers | CD_MOD_CK_HULL_LEFT;
                            if(p_reference[1] >= uphull_base - FP_ERROR)
                            {
                                ccnt = check_mesh(ray_pos[attach], ray_dir[attach], id, "LF_UP_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                if(ccnt == 0)
                                    ccnt += check_mesh(ray_pos[attach], ray_dir[attach], id, "LF_LW_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                            }
                            else
                            {
                                ccnt = check_mesh(ray_pos[attach], ray_dir[attach], id, "LF_LW_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                if(ccnt == 0)
                                    ccnt += check_mesh(ray_pos[attach], ray_dir[attach], id, "LF_UP_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                            }
                            if(ccnt == 0)
                            {
                                if(p_reference[2] >= -FP_ERROR)
                                {
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "FR_UP_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "GLACIS", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "FR_HL_NOSE", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "FR_LW_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                }
                                else
                                {
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "RR_UP_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "RR_LW_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                }
                            }
                        }
                        else
                        {
                            // Right approach
                            cd_modifiers = cd_modifiers | CD_MOD_CK_HULL_RIGHT;
                            if(p_reference[1] >= uphull_base - FP_ERROR)
                            {
                                ccnt = check_mesh(ray_pos[attach], ray_dir[attach], id, "RG_UP_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                if(ccnt == 0)
                                    ccnt += check_mesh(ray_pos[attach], ray_dir[attach], id, "RG_LW_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                            }
                            else
                            {
                                ccnt = check_mesh(ray_pos[attach], ray_dir[attach], id, "RG_LW_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                if(ccnt == 0)
                                    ccnt += check_mesh(ray_pos[attach], ray_dir[attach], id, "RG_UP_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                            }
                            if(ccnt == 0)
                            {
                                if(p_reference[2] >= -FP_ERROR)
                                {
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "FR_UP_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "GLACIS", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "FR_HL_NOSE", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "FR_LW_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                }
                                else
                                {
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "RR_UP_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, "RR_LW_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                }
                            }
                        }
                        
                        if(ray_dir[attach][1] < 0.1)
                            check_mesh(ray_pos[attach], ray_dir[attach], id, "TP_HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                        
                        if(tank_ptr->cupola_attach == attach)
                            check_mesh(ray_pos[attach], ray_dir[attach], id, "CUPOLA", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                    }
                    
                    // See if there is, even in the slightest, any chance of the turret being hit - even after checking the hull
                    if(turret_base != 0.0 && tank_ptr->turret_count > 0 && !turrets_checked && (p_ext_reference[0][1] >= turret_base - FP_ERROR || p_ext_reference[1][1] >= turret_base - FP_ERROR))
                    {
                        // Check each turret
                        for(i = 0; i < tank_ptr->turret_count; i++)
                        {
                            attach = OBJ_ATTACH_TURRET_OFF + i;
                            buildLCSIM(objOnePtr, objTwoPtr, attach, inversing_matrix[attach]);
                            ray_pos[attach] = kVector(0.0, -moveBack, 0.0);
                            ray_dir[attach] = kVector(0.0, CD_T_OFFSET, 0.0);
                            ray_pos[attach].transform((float*)inversing_matrix[attach]);
                            ray_dir[attach].transform((float*)inversing_matrix[attach]);
                            ray_dir[attach] = normalized(ray_dir[attach] - ray_pos[attach]);
                            
                            incoming_dir[attach] = vectorIn(ray_pos[attach], CS_SPHERICAL);
                            incoming_dir[attach][1] *= radToDeg;
                            incoming_dir[attach][2] *= radToDeg;
                            
                            if(incoming_dir[attach][2] < 45.0 || incoming_dir[attach][2] > 315.0)
                            {
                                // Forward approach
                                cd_modifiers = cd_modifiers | CD_MOD_CK_TURRET_FRONT;
                                sprintf(buffer, "FR_TURRET%i", i+1);
                                ccnt = check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                if(ccnt == 0)
                                {
                                    sprintf(buffer, "LF_TURRET%i", i+1);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    sprintf(buffer, "RG_TURRET%i", i+1);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                }
                            }
                            else if(incoming_dir[attach][2] > 135.0 && incoming_dir[attach][2] < 225.0)
                            {
                                // Rear approach
                                cd_modifiers = cd_modifiers | CD_MOD_CK_TURRET_REAR;
                                sprintf(buffer, "RR_TURRET%i", i+1);
                                ccnt = check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                if(ccnt == 0)
                                {
                                    sprintf(buffer, "LF_TURRET%i", i+1);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    sprintf(buffer, "RG_TURRET%i", i+1);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                }
                            }
                            else if(incoming_dir[attach][2] >= 45.0 && incoming_dir[attach][2] <= 225.0)
                            {
                                // Left approach
                                cd_modifiers = cd_modifiers | CD_MOD_CK_TURRET_LEFT;
                                sprintf(buffer, "LF_TURRET%i", i+1);
                                ccnt = check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                if(ccnt == 0)
                                {
                                    sprintf(buffer, "FR_TURRET%i", i+1);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    sprintf(buffer, "RR_TURRET%i", i+1);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                }
                            }
                            else
                            {
                                // Right approach
                                cd_modifiers = cd_modifiers | CD_MOD_CK_TURRET_RIGHT;
                                sprintf(buffer, "RG_TURRET%i", i+1);
                                ccnt = check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                if(ccnt == 0)
                                {
                                    sprintf(buffer, "FR_TURRET%i", i+1);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                    sprintf(buffer, "RR_TURRET%i", i+1);
                                    check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                                }
                            }
                            
                            if(ray_dir[attach][1] < 0.1)
                            {
                                sprintf(buffer, "TP_TURRET%i", i+1);
                                check_mesh(ray_pos[attach], ray_dir[attach], id, buffer, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                            }
                            
                            if(tank_ptr->cupola_attach == attach)
                                check_mesh(ray_pos[attach], ray_dir[attach], id, "CUPOLA", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                            
                            // Check EXT_CREW special
                            if(tank_ptr->ext_crew_attach == attach)
                            {
                                mesh = models.getMeshID(id, "EXT_CREW");  // Get mesh ID number
                                if(mesh != -1 && mesh != meshExclude)  // Make sure mesh checks out
                                {
                                    // Check against the AABB of the mesh
                                    if(checkMeshAABB(ray_pos[attach], ray_dir[attach], id, mesh, FP_ERROR, t_best))
                                        cd_modifiers = cd_modifiers | CD_MOD_EXT_CREW_HIT;
                                }
                            }
                            
                            // Check EXT_AMMO special
                            if(tank_ptr->ext_ammo_attach == attach)
                            {
                                mesh = models.getMeshID(id, "EXT_AMMO");  // Get mesh ID number
                                if(mesh != -1 && mesh != meshExclude)  // Make sure mesh checks out
                                {
                                    // Check against the AABB of the mesh
                                    if(checkMeshAABB(ray_pos[attach], ray_dir[attach], id, mesh, FP_ERROR, t_best))
                                        cd_modifiers = cd_modifiers | CD_MOD_EXT_AMMO_HIT;
                                }
                            }
                        }
                    }
                    
                    // Check EXT_CREW special
                    if(tank_ptr->ext_crew_attach == OBJ_ATTACH_HULL)
                    {
                        mesh = models.getMeshID(id, "EXT_CREW");  // Get mesh ID number
                        if(mesh != -1 && mesh != meshExclude)  // Make sure mesh checks out
                        {
                            attach = OBJ_ATTACH_HULL;
                            // Check against the AABB of the mesh
                            if(checkMeshAABB(ray_pos[attach], ray_dir[attach], id, mesh, FP_ERROR, t_best))
                                cd_modifiers = cd_modifiers | CD_MOD_EXT_CREW_HIT;
                        }
                    }
                    
                    // Check EXT_AMMO special
                    if(tank_ptr->ext_ammo_attach == OBJ_ATTACH_HULL)
                    {
                        mesh = models.getMeshID(id, "EXT_AMMO");  // Get mesh ID number
                        if(mesh != -1 && mesh != meshExclude)  // Make sure mesh checks out
                        {
                            attach = OBJ_ATTACH_HULL;
                            // Check against the AABB of the mesh
                            if(checkMeshAABB(ray_pos[attach], ray_dir[attach], id, mesh, FP_ERROR, t_best))
                                cd_modifiers = cd_modifiers | CD_MOD_EXT_AMMO_HIT;
                        }
                    }
                    
                    // Check EXT_ENGINE special
                    mesh = models.getMeshID(id, "EXT_ENGINE");  // Get mesh ID number
                    if(mesh != -1 && mesh != meshExclude)  // Make sure mesh checks out
                    {
                        attach = OBJ_ATTACH_HULL;
                        // Check against the AABB of the mesh
                        if(checkMeshAABB(ray_pos[attach], ray_dir[attach], id, mesh, FP_ERROR, CD_T_START))
                            cd_modifiers = cd_modifiers | CD_MOD_EXT_ENGINE_HIT;
                    }
                    
                    // Check EXT_FUEL special
                    mesh = models.getMeshID(id, "EXT_FUEL");  // Get mesh ID number
                    if(mesh != -1 && mesh != meshExclude)  // Make sure mesh checks out
                    {
                        attach = OBJ_ATTACH_HULL;
                        // Check against the AABB of the mesh
                        if(checkMeshAABB(ray_pos[attach], ray_dir[attach], id, mesh, FP_ERROR, CD_T_START))
                            cd_modifiers = cd_modifiers | CD_MOD_EXT_FUEL_HIT;
                    }
                    break;
                    
                case OBJ_TYPE_ATG:
                    atg_ptr = dynamic_cast<atg_object*>(objTwoPtr);
                    attach = OBJ_ATTACH_HULL;
                    // Build inversing matrix for hull attachment
                    buildLCSIM(objOnePtr, objTwoPtr, attach, inversing_matrix[attach]);
                    
                    // Transform projectile from its LCS to ATG's LCS.
                    ray_pos[attach] = kVector(0.0, -moveBack, 0.0);
                    ray_dir[attach] = kVector(0.0, CD_T_OFFSET, 0.0);
                    ray_pos[attach].transform((float*)inversing_matrix[attach]);
                    ray_dir[attach].transform((float*)inversing_matrix[attach]);
                    ray_dir[attach] = normalized(ray_dir[attach] - ray_pos[attach]);
                    
                    // Check wheels
                    check_mesh(ray_pos[attach], ray_dir[attach], id, "WHEEL_L1", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                    check_mesh(ray_pos[attach], ray_dir[attach], id, "WHEEL_R1", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                    
                    // Check gun mesh
                    attach = OBJ_ATTACH_GUN1;
                    // Build inversing matrix for attachment
                    buildLCSIM(objOnePtr, objTwoPtr, attach, inversing_matrix[attach]);
                    // Transform projectile from its LCS to ATG's gun's LCS.
                    ray_pos[attach] = kVector(0.0, -moveBack, 0.0);
                    ray_dir[attach] = kVector(0.0, CD_T_OFFSET, 0.0);
                    ray_pos[attach].transform((float*)inversing_matrix[attach]);
                    ray_dir[attach].transform((float*)inversing_matrix[attach]);
                    ray_dir[attach] = normalized(ray_dir[attach] - ray_pos[attach]);
                    check_mesh(ray_pos[attach], ray_dir[attach], id, "GUN1", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                    
                    // Check gun mantlet mesh
                    attach = OBJ_ATTACH_GUNMNT1;
                    // Build inversing matrix for attachment
                    buildLCSIM(objOnePtr, objTwoPtr, attach, inversing_matrix[attach]);
                    // Transform projectile from its LCS to ATG's gun mant.'s LCS.
                    ray_pos[attach] = kVector(0.0, -moveBack, 0.0);
                    ray_dir[attach] = kVector(0.0, CD_T_OFFSET, 0.0);
                    ray_pos[attach].transform((float*)inversing_matrix[attach]);
                    ray_dir[attach].transform((float*)inversing_matrix[attach]);
                    ray_dir[attach] = normalized(ray_dir[attach] - ray_pos[attach]);
                    check_mesh(ray_pos[attach], ray_dir[attach], id, "GUN_MANT1", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                    
                    // Check gun shield and hull
                    attach = OBJ_ATTACH_HULL;
                    check_mesh(ray_pos[attach], ray_dir[attach], id, "GUN_SHIELD", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                    check_mesh(ray_pos[attach], ray_dir[attach], id, "HULL", attach, t_best, cd_dr, cd_dr_best, meshExclude);
                    
                    // Check EXT_CREW special
                    if(atg_ptr->ext_crew_attach == OBJ_ATTACH_HULL)
                    {
                        mesh = models.getMeshID(id, "EXT_CREW");  // Get mesh ID number
                        if(mesh != -1 && mesh != meshExclude)  // Make sure mesh checks out
                        {
                            attach = OBJ_ATTACH_HULL;
                            // Check against the AABB of the mesh
                            if(checkMeshAABB(ray_pos[attach], ray_dir[attach], id, mesh, FP_ERROR, t_best))
                                cd_modifiers = cd_modifiers | CD_MOD_EXT_CREW_HIT;
                        }
                    }
                    
                    // Check EXT_AMMO special
                    if(atg_ptr->ext_ammo_attach == OBJ_ATTACH_HULL)
                    {
                        mesh = models.getMeshID(id, "EXT_AMMO");  // Get mesh ID number
                        if(mesh != -1 && mesh != meshExclude)  // Make sure mesh checks out
                        {
                            attach = OBJ_ATTACH_HULL;
                            // Check against the AABB of the mesh
                            if(checkMeshAABB(ray_pos[attach], ray_dir[attach], id, mesh, FP_ERROR, t_best))
                                cd_modifiers = cd_modifiers | CD_MOD_EXT_CREW_HIT;
                        }
                    }                    
                    break;
                
                case OBJ_TYPE_VEHICLE:
                    // Do later
                
                case OBJ_TYPE_ATR:
                    // Do later
                
                default:
                    attach = OBJ_ATTACH_HULL;
                    // Build inversing matrix for hull attachment
                    buildLCSIM(objOnePtr, objTwoPtr, attach, inversing_matrix[attach]);
                    
                    // Transform projectile from its LCS to object's LCS.
                    ray_pos[attach] = kVector(0.0, -moveBack, 0.0);
                    ray_dir[attach] = kVector(0.0, CD_T_OFFSET, 0.0);
                    ray_pos[attach].transform((float*)inversing_matrix[attach]);
                    ray_dir[attach].transform((float*)inversing_matrix[attach]);
                    ray_dir[attach] = normalized(ray_dir[attach] - ray_pos[attach]);
                    
                    // Check all meshes (brute force)
                    for(i = models.getMeshCount(id) - 1; i >= 0; i--)
                        ccnt += check_mesh(ray_pos[attach], ray_dir[attach], id, i, attach, t_best, cd_dr, cd_dr_best, meshExclude);
                    break;
            }
            break;
        
        // Does not currently check anything but projectiles
        default:
            return NULL;
            break;
    }
    
    // Sees if an ACN should be returned    
    if(cd_dr_best != NULL)
    {
        cd_dr_best->t -= moveBack;
        cd_dr_best->modifiers = cd_modifiers;
        return buildACN(objOnePtr, objTwoPtr, cd_dr_best);
    }
    else if(cd_modifiers & CD_MOD_EXT_CREW_HIT)
    {
        // do crew-hit CR
    }
    
    return NULL;
}

/*******************************************************************************
    function    :   collision_module::buildACN
    arguments   :   objOnePtr - Pointer to object doing the colliding
                    objTwoPtr - Pointer to object being collided with
                    cdDataReport - Pointer to the CD data report
    purpose     :   Creates an ACN based on the passed parameters.
    notes       :   Returns a NEW allocation.
*******************************************************************************/
ac_node* collision_module::buildACN(object* objOnePtr, object* objTwoPtr,
    cd_data* cdDataReport)
{
    ac_node* ACN = new ac_node;
    
    // Set pointers for ACN
    ACN->objOnePtr = objOnePtr;
    ACN->objTwoPtr = objTwoPtr;
    ACN->cdDataReport = cdDataReport;
    
    // Figure out the time to collision based on the type of object we're
    // dealing with here.
    switch(objOnePtr->obj_type)
    {
        case OBJ_TYPE_PROJECTILE:
            ACN->timeToCollision = cdDataReport->t /
                (((proj_object*)objOnePtr)->velocity * PROJ_VEL_MULTIPLIER);
            break;
        
        default:
            ACN->timeToCollision = 0.0;
            break;
    }
    
    // Initialize next pointer to null (probably not required, but just incase)
    ACN->next = NULL;
    
    return ACN;
}

/*******************************************************************************
    function    :   collision_module::addACN
    arguments   :   ACN - Anticipated collision node
    purpose     :   Adds an ACN to the currently handled list of such nodes so
                    that predictive collision detection works.
    notes       :   From this point forth, ACN deallocation will be handled
                    internally in the CDR module.
*******************************************************************************/
void collision_module::addACN(ac_node* ACN)
{
    // Check for NULL
    if(!ACN)
        return;
    
    // Determine if ACN should already be sent to handler
    if(ACN->timeToCollision <= 0.0)
    {
        handle(ACN);
        delete ACN->cdDataReport;
        delete ACN;
    }
    else
    {
        // Add ACN into linked list
        ACN->next = acl_head;
        acl_head = ACN;
        
        // Handle special cased cdr passes variable
        if(ACN->objOnePtr->obj_type == OBJ_TYPE_PROJECTILE)
            ((proj_object*)ACN->objOnePtr)->cdr_passes++;
        if(ACN->objTwoPtr->obj_type == OBJ_TYPE_PROJECTILE)
            ((proj_object*)ACN->objTwoPtr)->cdr_passes++;
    }
}

/*******************************************************************************
    function    :   collision_module::handle
    arguments   :   objOnePtr - Pointer to first object
                    objTwoPtr - Pointer to second object
    purpose     :   Handles collisions between basic objects.
    notes       :   Currently, only handling of moving objects is supported.
*******************************************************************************/
void collision_module::handle(object* objOnePtr, object* objTwoPtr)
{
    if((objOnePtr->obj_type == OBJ_TYPE_TANK ||
            objOnePtr->obj_type == OBJ_TYPE_VEHICLE) &&
        (objTwoPtr->obj_type == OBJ_TYPE_TANK ||
            objTwoPtr->obj_type == OBJ_TYPE_VEHICLE))
    {
        if((dynamic_cast<moving_object*>(objOnePtr))->wl_head != NULL)
        {
            if((dynamic_cast<moving_object*>(objTwoPtr))->wl_head != NULL)
            {
                if(fabsf(angleBetween(
                    (dynamic_cast<moving_object*>(objOnePtr))->wl_head->waypoint - 
                    (dynamic_cast<moving_object*>(objOnePtr))->pos,
                    (dynamic_cast<moving_object*>(objTwoPtr))->pos -
                    (dynamic_cast<moving_object*>(objOnePtr))->pos))
                    <= 60.0 * degToRad)
                {
                    if(fabsf(angleBetween(
                        (dynamic_cast<moving_object*>(objTwoPtr))->wl_head->waypoint - 
                        (dynamic_cast<moving_object*>(objTwoPtr))->pos,
                        (dynamic_cast<moving_object*>(objOnePtr))->pos -
                        (dynamic_cast<moving_object*>(objTwoPtr))->pos))
                        <= 60.0 * degToRad)
                    {
                        // Head-On Collision (1 vs. 2, 2 vs. 1)
                        stop_object(objOnePtr, objTwoPtr);
                        go_around_object(objOnePtr, objTwoPtr);
                        stop_object(objTwoPtr, objOnePtr);
                        go_around_object(objTwoPtr, objOnePtr);
                    }
                    else
                        // Side Collision (1 vs. 2)
                        stop_object(objOnePtr, objTwoPtr);
                }
                else if(fabsf(angleBetween(
                    (dynamic_cast<moving_object*>(objTwoPtr))->wl_head->waypoint - 
                    (dynamic_cast<moving_object*>(objTwoPtr))->pos,
                    (dynamic_cast<moving_object*>(objOnePtr))->pos -
                    (dynamic_cast<moving_object*>(objTwoPtr))->pos))
                    <= 60.0 * degToRad)
                {
                    // Side Collision (2 vs. 1)
                    stop_object(objTwoPtr, objOnePtr);
                }
                else
                {
                    // Akward collision (neither is facing each other but
                    // they still collide).
                    stop_object(objOnePtr, objTwoPtr);
                    go_around_object(objOnePtr, objTwoPtr);
                    stop_object(objTwoPtr, objOnePtr);
                    go_around_object(objTwoPtr, objOnePtr);
                }
            }
            else
            {
                // Collision into non-moving object
                stop_object(objOnePtr, objTwoPtr);
                go_around_object(objOnePtr, objTwoPtr);
            }
        }
        else if((dynamic_cast<moving_object*>(objTwoPtr))->wl_head != NULL)
        {
            // Collision into non-moving object
            stop_object(objTwoPtr, objOnePtr);
            go_around_object(objTwoPtr, objOnePtr);
        }
        else
        {
            // Neither object moving, but they bump into each other... :/
            stop_object(objOnePtr, objTwoPtr);
            stop_object(objTwoPtr, objOnePtr);
        }
    }
    else if((objOnePtr->obj_type == OBJ_TYPE_TANK ||
            objOnePtr->obj_type == OBJ_TYPE_VEHICLE) &&
            objTwoPtr->obj_type != OBJ_TYPE_TANK &&
            objTwoPtr->obj_type != OBJ_TYPE_VEHICLE)
    {
        // Collision into non-moving object
        stop_object(objOnePtr, objTwoPtr);
        go_around_object(objOnePtr, objTwoPtr);
    }
    else if(objOnePtr->obj_type != OBJ_TYPE_TANK &&
            objOnePtr->obj_type != OBJ_TYPE_VEHICLE &&
            (objTwoPtr->obj_type == OBJ_TYPE_TANK ||
            objTwoPtr->obj_type == OBJ_TYPE_VEHICLE))
    {
        // Collision into non-moving object
        stop_object(objTwoPtr, objOnePtr);
        go_around_object(objTwoPtr, objOnePtr);
    }
}

/*******************************************************************************
    function    :   collision_module::handle
    arguments   :   ACN - Anticipated collision node
    purpose     :   Handles collisions based on an ACN.
    notes       :   1) Currently only handles projectiles hitting tanks or ATGs.
                    2) Does not delete the ACN.
*******************************************************************************/
void collision_module::handle(ac_node* ACN)
{
    int i;
    char* temp;
    cd_data* cd_dr = NULL;
    cr_data* cr_dr = NULL;
    proj_object* proj_ptr;
    object* obj_ptr;
    kVector impact_pos;
    kVector impact_dir;
    kVector reflect_dir;
    int snd_id;
    
    // Various other pointers
    unit_object* uobj_ptr = NULL;
    moving_object* mobj_ptr = NULL;
    
    // Various vectors
    kVector device_dir;
    
    if(ACN == NULL)
    {
        write_error("CR: ACN handler was not given an ACN.");
        return;
    }
    
    // Set CD data report pointer
    cd_dr = ACN->cdDataReport;
    
    switch(((object*)(ACN->objOnePtr))->obj_type)
    {
        case OBJ_TYPE_PROJECTILE:
            // Set pointers for ease
            proj_ptr = ((proj_object*)(ACN->objOnePtr));
            obj_ptr = ((object*)(ACN->objTwoPtr));
            
            //cout << SDL_GetTicks() << ": [" << (unsigned int)proj_ptr << "]: Collision ACN handler performing CR." << endl;
            
            // We compute these values into variables for later retreival due
            // to their complex nature of computation, and so we can use them
            // in later functions without recomputing them.
            impact_pos = obj_ptr->transform(
                cd_dr->impactPoint, cd_dr->attachment);
            impact_dir = normalized(proj_ptr->dir);
            reflect_dir = normalized(obj_ptr->transform(
                cd_dr->impactPoint + cd_dr->reflection,
                cd_dr->attachment) - impact_pos);
            
            /* PHASE 1: Handle CR report generation */
            
            // Handle collision of static objects
            if(obj_ptr->obj_type == OBJ_TYPE_STATIC)
            {
                // We assume a static object to be a building or other scenery
                // element (if it has no reference tag saying otherwise).
                // Basically we fool the system into thinking we hit something
                // with a "phony" CR data report.
                cr_dr = new cr_data;
                cr_dr->modifiers = CR_MOD_NONE;
                
                // See if shell is explosive & set the explosion modifier. This
                // also insures addition of the appropriate sound later on.
                if(proj_ptr->explosive > 0.0)
                    cr_dr->modifiers = cr_dr->modifiers | CR_MOD_EXPLODE;
                
                // Grap this object's TAG property, which will help us identify
                // what we should do with this object.
                temp = db.query(obj_ptr->obj_model, "TAG");
                if(temp && strcmp(temp, "VEHC") == 0)
                {
                    // Vehicle
                    // Just set it up to say that we did "penetrate"
                    cr_dr->modifiers = cr_dr->modifiers | CR_MOD_FULL_PEN;
                }
                else if(temp && strcmp(temp, "TREE") == 0)
                {
                    // Tree
                    // Make projectile disappear out of thin air so we can add
                    // our own special effects.
                    cr_dr->modifiers = cr_dr->modifiers | CR_MOD_DISAPEAR;
                    
                    if(proj_ptr->diameter >= 1.5)
                    {
                        effects.addEffect(SE_DIRT, impact_pos, reflect_dir, 5.0f * proj_ptr->diameter);
                        effects.addEffect(SE_QF_BR_SMOKE, impact_pos, reflect_dir, 2.5f * proj_ptr->diameter);
                    }
                    else
                        effects.addEffect(SE_MG_GROUND_SMOKE, impact_pos);
                }
                else
                {
                    // Building (default)
                    // Make projectile disappear out of thin air so we can add
                    // our own special effects.
                    cr_dr->modifiers = cr_dr->modifiers | CR_MOD_DISAPEAR;
                    
                    if(proj_ptr->diameter >= 1.5)
                    {
                        effects.addEffect(SE_DEBRIS, impact_pos, reflect_dir, proj_ptr->diameter);
                        effects.addEffect(SE_QF_WH_SMOKE, impact_pos, 2.5f * proj_ptr->diameter);
                    }
                    else
                        effects.addEffect(SE_MG_GROUND_SMOKE, impact_pos);
                }
            }
            else
            {
                // Run over to the penetration calculator and gather a CR data
                // report based on the collision.
                cr_dr = penetration_calculator(proj_ptr, obj_ptr,
                    models.getMeshName(obj_ptr->model_id, cd_dr->mesh),
                    cd_dr->impactAngle * radToDeg);
            }
            
            if(cr_dr == NULL)    // Check for CR data report
            {
                write_error("CR: ACN handler could not generate a CR data report.");
                return;
            }
            
            /* PHASE 2: Handle Projectile (ACN->objOnePtr/proj_ptr) */
            
            // Handle interuption of projectile orientation through the
            // extension function provided. Richochets handle interuptions a
            // bit differently than non-richochets.
            if(cr_dr->modifiers & CR_MOD_RICOCHET)
            {
                // Add to the projectile the appropriate distance offseting
                // value for KE-loss mimicing.
                proj_ptr->distance_offset = fabsf(cr_dr->distance_offset);
                // Interupt projectile.
                proj_ptr->interupt(impact_pos, reflect_dir,
                    cr_dr->result_velocity, ACN->timeToCollision, true);
            }
            else
            {
                // If projectile is not richocheting, then it must be killed
                // off before we do an interuption.
                proj_ptr->killProj();
                // Interupt projectile.
                proj_ptr->interupt(impact_pos, reflect_dir, 0.0f,
                    ACN->timeToCollision, false);
            }
            
            // Handle projectile device arming
            if(cr_dr->modifiers & CR_MOD_ARM_DEVICE)
                proj_ptr->armDevice();
            
            /* PHASE 3: Handle Object (ACN->objTwoPtr/obj_ptr) */
            switch(obj_ptr->obj_type)
            {
                case OBJ_TYPE_TANK:
                    mobj_ptr = dynamic_cast<moving_object*>(obj_ptr);
                    
                    device_dir = vectorIn(obj_ptr->transform(mobj_ptr->motor.getMotorPosV(),
                            OBJ_ATTACH_HULL) - impact_pos, CS_SPHERICAL);
                    if((cr_dr->modifiers & CR_MOD_PEN_OR_SPALL) &&
                       ((cd_dr->modifiers & CD_MOD_EXT_ENGINE_HIT) ||
                       fabsf(angleBetween(vectorIn(device_dir, CS_CARTESIAN), impact_dir)) <= 10.0 * degToRad))
                    {
                        float amount =
                            2.0 * max_value(2.758621e-6 * cr_dr->result_ke + -0.02069, 0.0) *
                            max_value(min_value(device_dir[0] / (1.034483e-5 *
                                cr_dr->result_ke + 0.172414), 1.0), 0.0);
                        if(amount > 0.0)
                            mobj_ptr->motor.reduceMotorLife(amount);
                    }
                case OBJ_TYPE_ATG:
                    uobj_ptr = dynamic_cast<unit_object*>(obj_ptr);
                    
                    if(cr_dr->modifiers & CR_MOD_PEN_OR_SPALL)
                    {
                        // Apply damage to crew
                        for(i = 0; i < uobj_ptr->crew.getCrewmanCount(); i++)
                        {
                            device_dir = vectorIn(obj_ptr->transform(kVector(uobj_ptr->crew.getCrewmanPosition(i)),
                                uobj_ptr->crew.getCrewmanAttach(i)) - impact_pos, CS_SPHERICAL);
                            float amount =
                                2.0 * max_value(2.758621e-6 * cr_dr->result_ke + -0.02069, 0.0) *
                                    max_value(device_dir[0] * -0.514286 + 1.128571, 0.0) *
                                    max_value(fabsf(angleBetween(vectorIn(device_dir, CS_CARTESIAN), impact_dir)) *
                                        -0.0477465 + 1.0, 0.0);
                            if(amount > 0.0)
                                uobj_ptr->crew.reduceHealth(i, amount);
                        }
                    }
                    break;
                
                // Do ALL other objects at another time!!!
                default:
                    break;
            }
            
            /* PHASE 4: Handle Specular Effects */
            
            // Handle adding explosion effects
            if(cr_dr->modifiers & CR_MOD_EXPLODE)
            {
                // Add explosion effect
                effects.addEffect(SE_EXPLOSION, impact_pos, reflect_dir,
                    proj_ptr->explosive,
                    (obj_ptr->obj_type == OBJ_TYPE_TANK ||
                     obj_ptr->obj_type == OBJ_TYPE_VEHICLE ?
                    vectorIn(obj_ptr->dir, CS_CARTESIAN) *
                        (dynamic_cast<moving_object*>(obj_ptr))->linear_vel
                    : kVector(0.0, 0.0, 0.0)));
                
                // Add explosion sound
                snd_id = sounds.addSound(SOUND_EXPLOSION, SOUND_HIGH_PRIORITY,
                    impact_pos(), SOUND_PLAY_ONCE);
                sounds.setSoundRolloff(snd_id, 0.01);
                sounds.auxModOnExplosive(snd_id, proj_ptr->explosive);
            }
            
            // Handle adding shrapnel and penetration effects
            if(!(cr_dr->modifiers & CR_MOD_DISAPEAR))
            {
                // Have a fun spiffy little formula here to handle the addition
                // of shrapnel based on the caliber of the projectile.
                float amount = -10.0 + 10.0 * proj_ptr->diameter;
                
                if(amount >= 1.0)   // Check for at least 1 particle
                {
                    // Apply a deficiency for those which only partially
                    // penetrate or for those which richochet right off.
                    if(cr_dr->modifiers & CR_MOD_FULL_PEN)
                        amount *= 1.2;
                    else if(cr_dr->modifiers & CR_MOD_PARTIAL_PEN)
                        amount *= 0.9;
                    else if(cr_dr->modifiers & CR_MOD_MAJOR_SPALLING)
                        amount *= 2.0;
                    else if(cr_dr->modifiers & CR_MOD_MINOR_SPALLING)
                        amount *= 1.4;
                    else if(cr_dr->modifiers & CR_MOD_NO_PENETRATION)
                        amount *= 0.5 * (1.0 -
                            (cd_dr->impactAngle / PIHALF));
                    else
                        amount = 0;
                    
                    if(amount >= 1.0)   // Check for at least 1 particle
                        effects.addEffect(SE_SHRAPNEL,
                            impact_pos, reflect_dir, amount,
                            (obj_ptr->obj_type == OBJ_TYPE_TANK ||
                             obj_ptr->obj_type == OBJ_TYPE_VEHICLE ?
                               vectorIn(obj_ptr->dir, CS_CARTESIAN) *
                               (dynamic_cast<moving_object*>(obj_ptr))->linear_vel
                            : kVector(0.0, 0.0, 0.0)));
                }
                
                // Take care of adding sounds
                if(cr_dr->modifiers & CR_MOD_PENETRATION)
                {
                    // Add penetration sound
                    snd_id = sounds.addSound(SOUND_PENETRATE_1 + (rand() % 2),
                        SOUND_HIGH_PRIORITY, impact_pos(), SOUND_PLAY_ONCE);
                    sounds.setSoundRolloff(snd_id, 0.02);
                    sounds.auxModOnCaliber(snd_id, proj_ptr->diameter);
                }
                else if(cr_dr->modifiers & CR_MOD_SPALLING)
                {
                    // Add a "thud" sound
                    snd_id = sounds.addSound(SOUND_SHELL_THUD,
                        SOUND_HIGH_PRIORITY, impact_pos(), SOUND_PLAY_ONCE);
                    sounds.setSoundRolloff(snd_id, 0.02);
                    sounds.auxModOnCaliber(snd_id, proj_ptr->diameter);
                }
                else if(cr_dr->modifiers & CR_MOD_RICOCHET)
                {
                    if(proj_ptr->diameter >= 1.5)
                    {
                        // Add shell richochet
                        snd_id = sounds.addSound(SOUND_SHELL_RICOCHET_1 + (rand() % 3),
                            SOUND_HIGH_PRIORITY, impact_pos(), SOUND_PLAY_ONCE);
                        sounds.setSoundRolloff(snd_id, 0.02);
                        sounds.auxModOnCaliber(snd_id, proj_ptr->diameter);
                    }
                    else
                    {
                        // Add MG richochet
                        snd_id = sounds.addSound(SOUND_MG_RICHOCHET_1 + (rand() % 3),
                            SOUND_MID_PRIORITY, impact_pos(), SOUND_PLAY_ONCE);
                        sounds.setSoundRolloff(snd_id, 0.10);
                    }
                }
            }
            else
            {
                // Add a "thud" sound (this could happen if no appropriate
                // modifiers were set - something has to happen tho).
                snd_id = sounds.addSound(SOUND_SHELL_THUD, SOUND_MID_PRIORITY,
                    impact_pos(), SOUND_PLAY_ONCE);
                sounds.setSoundRolloff(snd_id, 0.05);
                sounds.auxModOnCaliber(snd_id, proj_ptr->diameter);
            }
            
            /* PHASE 5: Handle Multiple Collisions */
            
            // Handle richochet collisions that re-collide with tested object.
            if((cr_dr->modifiers & CR_MOD_RICOCHET) &&
               (proj_ptr->diameter >= 1.5 ||
                proj_ptr->obj_modifiers & AMMO_MOD_TRACER))
            {
                ac_node* rcACN = NULL;
                
                if(cd_dr->impactAngle >= 15.0 * degToRad)
                {
                    rcACN = checkMeshes(
                        (object*)ACN->objOnePtr, (object*)ACN->objTwoPtr,
                        ACN->timeToCollision * proj_ptr->velocity * PROJ_VEL_MULTIPLIER,
                        cd_dr->mesh);
                    
                    if(rcACN != NULL)
                    {
                        if(rcACN->timeToCollision <= 0.0)
                        {
                            // Note: also need some code here to increment
                            // distance traveled.
                            handle(rcACN);
                            delete rcACN->cdDataReport;
                            delete rcACN;
                        }
                        else
                        {
                            rcACN->next = acl_pending;
                            acl_pending = rcACN;
                        }
                    }
                }
                
                // Create a new CDTL otherwise since the object bounced off
                // there and is on a new collision course with other objects.
                if(rcACN == NULL && proj_ptr->cdtl_head == NULL)
                    proj_ptr->cdtl_head = objects.createCDTL(proj_ptr, 15.0, 20.0, obj_ptr);
            }
            
            // Clean up
            if(cr_dr) delete cr_dr;
            break;
        
        default:
            break;
    }
}

/*******************************************************************************
    function    :   collision_module::update
    arguments   :   deltaT - # of seconds elapsed since last update
    purpose     :   Updates the list of ACNs currently being held for later
                    handling.
    notes       :   <none>
*******************************************************************************/
void collision_module::update(float deltaT)
{
    ac_node* curr = acl_head;
    ac_node* prev = NULL;
    
    while(curr)
    {
        curr->timeToCollision -= deltaT;        // Decrement timer
        
        if(curr->timeToCollision <= 0.0)        // Check for timer timeout
        {
            handle(curr);                       // Pass ACN to handler function
            
            // Handle special cased cdr passes variable
            if(curr->objOnePtr->obj_type == OBJ_TYPE_PROJECTILE)
                ((proj_object*)curr->objOnePtr)->cdr_passes--;
            if(curr->objTwoPtr->obj_type == OBJ_TYPE_PROJECTILE)
                ((proj_object*)curr->objTwoPtr)->cdr_passes--;
            
            delete curr->cdDataReport;          // Delete the CD data report
            
            // Maintain linked list
            if(prev == NULL)
            {
                acl_head = acl_head->next;
                delete curr;
                curr = acl_head;
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
    
    // Move all nodes from the pending list to main list for next time.
    while(acl_pending)
    {
        curr = acl_pending->next;
        acl_pending->next = acl_head;
        acl_head = acl_pending;
        acl_pending = curr;
    }
}
