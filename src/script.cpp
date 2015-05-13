/*******************************************************************************
                        Script Module - Implementation
*******************************************************************************/
#include "main.h"
#include "load.h"
#include "camera.h"
#include "scenery.h"
#include "console.h"
#include "database.h"
#include "sounds.h"
#include "effects.h"
#include "objhandler.h"
#include "objlist.h"
#include "object.h"
#include "tank.h"
#include "objunit.h"
//#include "astar.h"
#include "script.h"


/*******************************************************************************
    function    :   sc_command::sc_command
    arguments   :   command - command being added.
    purpose     :   Constructor of the sc_command struct.
    notes       :   <none>
*******************************************************************************/
sc_command::sc_command( char* command )
{
    char* word = NULL;
    
    // Setup variables incase read doesn't go correctly
    name[0] = '\0';
    section[0] = '\0';
    output[0] = '\0';

    // Setup Default info
    strcpy( output, command );
    strcpy( section, "general2" );
    strcpy( name, "default" );

    // Copy over name
    word = strtok( command, script.ignore_tokens );
    if( word == NULL )
    {
        return;
    }
    strcpy( name, word );

    // Copy over section
    word = strtok(NULL, script.ignore_tokens);
    if( word == NULL )
    {
        return;
    }
    strcpy( section, word );
}

/*******************************************************************************
    function    :   sc_sound_effect_handler::sc_sound_effect_handler
    arguments   :   <none>
    purpose     :   constructor
    notes       :   <none>
*******************************************************************************/
sc_sound_effect_handler::sc_sound_effect_handler()
{
    sound_head = NULL;
    sound_head = NULL;
}

/*******************************************************************************
    function    :   sc_sound_effect_handler::~sc_sound_effect_handler
    arguments   :   <none>
    purpose     :   deconstructor
    notes       :   <none>
*******************************************************************************/
sc_sound_effect_handler::~sc_sound_effect_handler()
{
    sound_effect_node* temp;

    while( sound_head != NULL )
    {
        temp = sound_head;
        sound_head = sound_head->next;
        delete temp;
    }

    while( effect_head != NULL )
    {
        temp = effect_head;
        sound_head = effect_head->next;
        delete temp;
    }
}
      
/*******************************************************************************
    function    :   sc_sound_effect_handler::displayEffects
    arguments   :   <none>
    purpose     :   Displays all effects started by scripts or command prompt.
    notes       :   <none>
*******************************************************************************/
void sc_sound_effect_handler::displayEffects()
{
    sound_effect_node* curr = effect_head;
    char buffer[128];

    console.addSysMessage(" ");
    console.addSysMessage("_EFFECT ID______________________________EFFECT NAME__________________________________________________________________________");
    
    while( curr != NULL )
    {
            sprintf( buffer, "%d                               %s", curr->id, curr->name );
            console.addSysMessage( buffer );
            curr = curr->next;
    }
    
    console.addSysMessage(" ");
}

/*******************************************************************************
    function    :   sc_sound_effect_handler::displaySounds
    arguments   :   <none>
    purpose     :   Displays all sounds started by scripts or command prompt.
    notes       :   <none>
*******************************************************************************/
void sc_sound_effect_handler::displaySounds()
{
    sound_effect_node* curr = sound_head;
    char buffer[128];

    console.addSysMessage(" ");
    console.addSysMessage("_SOUND ID_______________________________SOUND NAME___________________________________________________________________________");
    
    while( curr != NULL )
    {
            sprintf( buffer, "%d                               %s", curr->id, curr->name );
            console.addSysMessage( buffer );
            curr = curr->next;
    }
    
    console.addSysMessage(" ");
}

/*******************************************************************************
    function    :   sc_sound_effect_handler::effectDead
    arguments   :   <none>
    purpose     :   Removes node of effect that died.
    notes       :   <none>
*******************************************************************************/
void sc_sound_effect_handler::effectDead( int id )
{
    sound_effect_node* curr = effect_head;
    sound_effect_node* prev = effect_head;
    
    while( curr != NULL )
    {
        if( curr->id == id )
        {                
            // Remove effect node from system
            if( curr == effect_head )
            {
                curr = effect_head;
                effect_head = effect_head->next;
                delete curr;
            }
            else
            {
                prev->next = curr->next;
                delete curr;
                curr = prev;
            }
            return;
        }
        prev = curr;
        curr = curr->next;
    }
    
    return;
}

/*******************************************************************************
    function    :   sc_sound_effect_handler::getEffectType
    arguments   :   <none>
    purpose     :   Returns the integer value of an effect.
    notes       :   <none>
*******************************************************************************/
int sc_sound_effect_handler::getEffectType( char* type )
{

    // Explosion effects
    //#define SE_EXPLOSION            0       // Base HE explosion
    if( strcmp( "SE_EXPLOSION", type ) == 0 )
        return 0;       
    //#define SE_BASE_EXPLOSION       1       // Base flash for Big explosion
    if( strcmp( "SE_BASE_EXPLOSION", type ) == 0 )
        return 1;
    //#define SE_BIG_EXPLOSION        2       // HUGE explosion
    if( strcmp( "SE_BIG_EXPLOSION", type ) == 0 )
        return 2;
    //#define SE_SUB_EXPLOSION        3       // Sub explosion in HUGE explosion
    if( strcmp( "SE_SUB_EXPLOSION", type ) == 0 )
        return 3;
    //#define SE_MG_GROUND            4       // Effect for when bullet hits the ground
    if( strcmp( "SE_MG_GROUND", type ) == 0 )
        return 4;
    
    // Firing effects
    //#define SE_CANNON_FIRING        10      // Base cannon firing
    if( strcmp( "SE_CANNON_FIRING", type ) == 0 )
        return 10;
    //#define SE_MG_FIRING            11      // MG firing
    if( strcmp( "SE_MG_FIRING", type ) == 0 )
        return 11;
    //#define SE_FIRING_BLAST         12      // Cannon blast (ext)
    if( strcmp( "SE_FIRING_BLAST", type ) == 0 )
        return 12;
    //#define SE_FIRING_SMOKE         13      // Cannon smoke (ext)
    if( strcmp( "SE_FIRING_SMOKE", type ) == 0 )
        return 13;
    
    // Fountain-like effects
    //#define SE_DIRT                 20      // Dirt fragments fountain
    if( strcmp( "SE_DIRT", type ) == 0 )
        return 20;
    //#define SE_MG_DIRT              21      // Dirt used in MG_GROUND effect
    if( strcmp( "SE_MG_DIRT", type ) == 0 )
        return 21;
    //#define SE_LARGE_DIRT           22      // Large dirt fragments fountain
    if( strcmp( "SE_LARGE_DIRT", type ) == 0 )
        return 22;
    //#define SE_SHRAPNEL             23      // Shrapnel fragments fountain
    if( strcmp( "SE_SHRAPNEL", type ) == 0 )
        return 23;
    //#define SE_DEBRIS               24      // Debris fragments fountain
    if( strcmp( "SE_DEBRIS", type ) == 0 )
        return 24;
    //#define SE_FIRE_DEBRIS          25      // Debris fragments fountain that emit smoke
    if( strcmp( "SE_FIRE_DEBRIS", type ) == 0 )
        return 25;
    //#define SE_SMOKE_DEBRIS         26      // Debris fragments fountain that emit fire
    if( strcmp( "SE_SMOKE_DEBRIS", type ) == 0 )
        return 26;
    //#define SE_SPARKS               27      // Spark fountain
    if( strcmp( "SE_SPARKS", type ) == 0 )
        return 27;
    //#define SE_FIRE                 28      // Fire fountain
    if( strcmp( "SE_FIRE", type ) == 0 )
        return 28;
    
    // Smoke/Clouding effects
    //#define SE_WH_DISPENSER_SMOKE   30      // White smoke from dispenser
    if( strcmp( "SE_WH_DISPENSER_SMOKE", type ) == 0 )
        return 30;
    //#define SE_WH_BILLOWING_SMOKE   31      // White billowing smoke (engine damage)
    if( strcmp( "SE_WH_BILLOWING_SMOKE", type ) == 0 )
        return 31;
    //#define SE_BK_BILLOWING_SMOKE   32      // Black billowing smoke (on fire)
    if( strcmp( "SE_BK_BILLOWING_SMOKE", type ) == 0 )
        return 32;
    //#define SE_QF_WH_SMOKE          33      // Quick fading white smoke
    if( strcmp( "SE_QF_WH_SMOKE", type ) == 0 )
        return 33;
    //#define SE_QF_BR_SMOKE          34      // Quick fading brown smoke
    if( strcmp( "SE_QF_BR_SMOKE", type ) == 0 )
        return 34;
    //#define SE_DUST_CLOUD           35      // Dust cloud
    if( strcmp( "SE_DUST_CLOUD", type ) == 0 )
        return 35;
    //#define SE_DUST_TRAIL           36      // Dust emitted from tank tracks
    if( strcmp( "SE_DUST_TRAIL", type ) == 0 )
        return 36;
    //#define SE_BL_DEBRIS_SMOKE      37      // Black smoke left in wake of flying debris
    if( strcmp( "SE_BL_DEBRIS_SMOKE", type ) == 0 )
        return 37;
    //#define SE_FIRE_DEBRIS_SMOKE    38      // Black smoke left in wake of flying fire debris
    if( strcmp( "SE_FIRE_DEBRIS_SMOKE", type ) == 0 )
        return 38;
    //#define SE_MG_GROUND_SMOKE      39      // Emitted when mg fire hits the ground(ext)
    if( strcmp( "SE_MG_GROUND_SMOKE", type ) == 0 )
        return 39;
       
    // Weather effects
    //#define SE_RAIN                 50      // Rain weather effect
    if( strcmp( "SE_RAIN", type ) == 0 )
        return 50;
    //#define SE_SNOW                 51      // Snow weather effect
    if( strcmp( "SE_SNOW", type ) == 0 )
        return 51;
        
    return -1;
}

/*******************************************************************************
    function    :   sc_sound_effect_handler::killEffect
    arguments   :   <none>
    purpose     :   Kills an effect that was started from a script or command
                    prompt.
    notes       :   <none>
*******************************************************************************/
bool sc_sound_effect_handler::killEffect( int id )
{
    sound_effect_node* curr = effect_head;
    sound_effect_node* prev = effect_head;
    
    while( curr != NULL )
    {
        if( curr->id == id )
        {
            // Kill effect
            effects.killEffect( id );
             
            // Remove effect node from system
            if( curr == effect_head )
            {
                curr = effect_head;
                effect_head = effect_head->next;
                delete curr;
            }
            else
            {
                prev->next = curr->next;
                delete curr;
                curr = prev;
            }
            return 1;
        }
        prev = curr;
        curr = curr->next;
    }
    
    return 0;
}

/*******************************************************************************
    function    :   sc_sound_effect_handler::
    arguments   :   <none>
    purpose     :   Kills a sound that was started by a script or command 
                    prompt.
    notes       :   <none>
*******************************************************************************/
bool sc_sound_effect_handler::killSound( int id )
{
    sound_effect_node* curr = sound_head;
    sound_effect_node* prev = sound_head;
    
    while( curr != NULL )
    {
        if( curr->id == id )
        {
            // Kill sound
            sounds.killSound( id );
                        
            // Remove sound from system
            if( curr == sound_head )
            {
                curr = sound_head;
                effect_head = sound_head->next;
                delete curr;
            }
            else
            {
                prev->next = curr->next;
                delete curr;
                curr = prev;
            }
            
            return 1;
        }
        prev = curr;
        curr = curr->next;
    }
    
    return 0;
}

/*******************************************************************************
    function    :   sc_sound_effect_handler::startEffect
    arguments   :   <none>
    purpose     :   Adds an effect to the system.
    notes       :   <none>
*******************************************************************************/
void sc_sound_effect_handler::startEffect( char* command )
{
    int id;
    char type[32];
    float posx;
    float posy;
    float posz;
    float dirx;
    float diry;
    float dirz;
    float sysx;
    float sysy;
    float sysz;
    float modifier;
    
    kVector position;
    kVector direction;
    kVector systemDirection;
    
    char buffer[128];
    
    // Parse
    // This assumes that the name has no preceding spaces
    // remove startEffect
    command = &command[12];
    

     
    if( sscanf(command, "%s %f %f %f %f %f %f %f %f %f %f", type, &posx, &posy, &posz, 
                &dirx, &diry, &dirz, &modifier, &sysx, &sysy, &sysz) == 11 )
    {
        id = effects.addEffect( getEffectType(type), 
                                kVector(posx, posy, posz),
                                kVector(dirx, diry, dirz),
                                modifier,
                                kVector(sysx, sysy, sysz)
                                );
    }
    else if( sscanf(command, "%s %f %f %f %f %f %f %f", type, &posx, &posy, &posz, &dirx, &diry, &dirz, &modifier) == 8 )
    {
        id = effects.addEffect( getEffectType(type), 
                                kVector(posx, posy, posz),
                                kVector(dirx, diry, dirz),
                                modifier
                                );
    }
    else if( sscanf(command, "%s %f %f %f %f %f %f", type, &posx, &posy, &posz, &dirx, &diry, &dirz) == 7 )
    {
        id = effects.addEffect( getEffectType(type), 
                                kVector(posx, posy, posz),
                                kVector(dirx, diry, dirz)
                                );
    }
    else if( sscanf(command, "%s %f %f %f %f", type, &posx, &posy, &posz, &modifier) == 5 )
    {
        id = effects.addEffect( getEffectType(type), 
                                kVector(posx, map.getHeight( posx, posz), posz),
                                modifier
                                );
    }
    else if( sscanf(command, "%s %f %f %f", type, &posx, &posy, &posz) == 4 )
    {
        id = effects.addEffect( getEffectType(type),
                                kVector(posx, posy, posz)
                                );
    } 
    else if( sscanf(command, "%s", type) == 1 )
    {
        id = effects.addEffect( getEffectType(type) );
    }
    else
    {
        script.incorrectParams();
        return;
    }
    
    // Check to make sure id was returned
    if( id == 0 )
    {
        console.addMessage( "Effect could not be started. Type entered may not be valid." );
        return;
    }
    
    // An id of -1 is returned from effect like smoke and fire
    // that launch multiple effects in one call.
    if( id == -1 )
    {
        console.addMessage( "Effect id could not be returned because effect called has launched multiple effects." );
        return;
    }
    
    // Create sound node and add to list      
    sound_effect_node* new_node = new sound_effect_node( type, id );
    new_node->next = effect_head;
    effect_head = new_node;

    // Inform user of effect id
    sprintf( buffer, "Effect %s has started. Effect id: %d.", type, id );
    console.addSysMessage( buffer );
}

/*******************************************************************************
    function    :   sc_sound_effect_handler::startSound
    arguments   :   <none>
    purpose     :   Adds an ambient sound to the system.
    notes       :   <none>
*******************************************************************************/
void sc_sound_effect_handler::startSound( char* command )
{
    int id;
    char name[128];
    char buffer[128];
    
    // Check for correct parameters
    if( sscanf( command, "%*s %s", name ) != 1 )
        return; 
    
    id = sounds.addAmbientSound( name, SOUND_HIGH_PRIORITY, SOUND_PLAY_ONCE );
        
    // Check to make sure id was returned
    if( id == 0 )
    {
        console.addMessage( "Sound Could not be played." );
        return;
    }
        
    // Create sound node and add to list
    sound_effect_node* new_node = new sound_effect_node( name, id );
    new_node->next = sound_head;
    sound_head = new_node;
    
    // Inform user of sound id
    sprintf( buffer, "Sound %s is now playing. Sound id: %d.", name, id );
    console.addSysMessage( buffer );
}


/*******************************************************************************
    function    :   routine::routine
    arguments   :   name - Name of routine being defined
    purpose     :   Constructor
    notes       :   <none>
*******************************************************************************/
routine::routine( char *tName )
{
    // Make sure name is a null terminated string
    if( strlen(tName) >= SCR_NAME_LEN )
        name[SCR_NAME_LEN - 1] = '\0';

    strcpy(name, tName);
    num_commands = 0;
}

/*******************************************************************************
    function    :   routine::addCommand
    arguments   :   command - string that is added to the command list contained
                    in the routine.
    purpose     :   Adds command the routine at the head of the routine list.
    notes       :   <none>
*******************************************************************************/
bool routine::addCommand( char* command )
{
    // Make sure there is space in the list
    if( num_commands < SCR_MAX_COMMANDS_IN_ROUTINE )
    {
        // Copy pointer
        if( command != NULL )
        {
            strcpy(command_list[num_commands], command);
            num_commands++;
            return 1;
        }
    }

    return 0;
}

/*******************************************************************************
    function    :   routine::execRoutine
    arguments   :   <none>
    purpose     :   Runs all the commands through the handler function.
    notes       :   <none>
*******************************************************************************/
void routine::execRoutine()
{
    for(int i = 0; i < num_commands; i++)
    {
        script.handler(command_list[i]);
    }
}

/*******************************************************************************
    function    :   trigger::execRoutine
    arguments   :   <none>
    purpose     :   Base function.
    notes       :   <none>
*******************************************************************************/
void trigger::execRoutine()
{
    if( routine_ptr != NULL )
        routine_ptr->execRoutine();
}

/*******************************************************************************
    function    :   cellAreaTrigger::cellAreaTrigger
    arguments   :   tId             -   temp id
                    tSideEffected   -   temp side effected
                    tPermanent      -   temp permanent
                    tXMin           -   temp x min
                    tXMax           -   temp x max
                    tZMin           -   temp z min
                    tZMax           -   temp z max
                    tRoutine        -   temp routine name
    purpose     :   Constructor
    notes       :   <none>
*******************************************************************************/
cellAreaTrigger::cellAreaTrigger(char* tId, char* tSideEffected, bool tPermanent, 
            float tXMin, float tXMax, float tZMin, float tZMax, char* tRoutine)
{
    // Copy id
    strcpy(id, tId);
    strcpy(type, "cellAreaTrigger");

    // Copy side effected
    if( tSideEffected == NULL )
        side_effected[0] = '\0';
    else
        strcpy( side_effected, script.script_side );

    // Copy permanent
    permanent = tPermanent;
 
    x_min = tXMin;
    x_max = tXMax;
    z_min = tZMin;
    z_max = tZMax;

    // Get appropriate routine pointer
    routine_ptr = script.getRoutine( tRoutine );

    if(routine_ptr == NULL )
        dead = true;
}

/*******************************************************************************
    function    :   cellAreaTrigger::cellAreaTrigger
    arguments   :   <none>
    purpose     :   Tests if any unit of target side has entered area.
    notes       :   <none>
*******************************************************************************/
bool cellAreaTrigger::isActivated()
{

    //change pos to searching through object list and testing if any thing passes
    // Return true if point is inside box
    //if( (x_min <= pos[0]) && (x_max >= pos[0]) && (z_min <= pos[2]) && (z_min >= pos[2]) )
    //    return 1;

    return 0;
}

/*******************************************************************************
    function    :   cellAreaTrigger::displayTrigger
    arguments   :   none
    purpose     :   Displays important information about trigger to console 
                    window.
    notes       :   <none>
*******************************************************************************/
void cellAreaTrigger::displayTrigger()
{
    char buffer[256];

    sprintf(buffer, "Trigger id: %s", id);
    console.addSysMessage(buffer);
    sprintf(buffer, "Trigger type: %s", type);
    console.addSysMessage(buffer);
    sprintf(buffer, "Permanent: %d", permanent);
    console.addSysMessage(buffer);
    sprintf(buffer, "Side Effected: %s", side_effected);
    console.addSysMessage(buffer);
    sprintf(buffer, "xMin xMax yMin yMax: %f %f %f %f", x_min, x_max, z_min, z_max);
    console.addSysMessage(buffer);
    sprintf(buffer, "Associated routine: %s", routine_ptr->getRoutineName());
    console.addSysMessage(buffer);
}

/*******************************************************************************
    function    :   cellRadiusTrigger::cellRadiusTrigger
    arguments   :   tId             - temp id
                    tSide           - temp side effected
                    tPermanent      - temp permanent
                    tXCenter        - temp x coordinate
                    tZCenter        - temp z coordinate
                    tRadius         - temp radius
                    tRoutine        - temp routine
    purpose     :   Constructor
    notes       :   <none>
*******************************************************************************/
cellRadiusTrigger::cellRadiusTrigger( char* tId, char* tSideEffected, bool tPermanent, 
        float tXCenter, float tZCenter, float tRadius, char* tRoutine )
{
    // Copy id
    strcpy(id, tId);
    strcpy(type, "cellRadiusTrigger");

    // Copy side effected
    if( tSideEffected == NULL )
        side_effected[0] = '\0';
    else
        strcpy( side_effected, script.script_side );

    // Copy permanent
    permanent = tPermanent;
 
    x_center = tXCenter;
    z_center = tZCenter;
    radius = tRadius;

    // Get appropriate routine pointer
    routine_ptr = script.getRoutine( tRoutine );

    if(routine_ptr == NULL )
        dead = true;
}

/*******************************************************************************
    function    :   cellRadiusTrigger::isActivated
    arguments   :   <none>
    purpose     :   Tests if any unit of target side has entered radius.
    notes       :   <none>
*******************************************************************************/
bool cellRadiusTrigger::isActivated()
{/*
    //change pos to searching through object list and testing if any thing passes
    float distance;
    kVector temp_vector = kVector( x_center, 0, z_center );
    temp_vector -= pos;

    distance = sqrt(temp_vector[0] * temp_vector[0] + temp_vector[2] * temp_vector[2]);

    // Return true if point is inside box
    if( distance <= radius )
        return 1;*/

    return 0;
}

/*******************************************************************************
    function    :   cellRadiusTrigger::displayTrigger
    arguments   :   none
    purpose     :   Displays important information about trigger to console 
                    window.
    notes       :   <none>
*******************************************************************************/
void cellRadiusTrigger::displayTrigger()
{
    char buffer[256];

    sprintf(buffer, "Trigger id: %s", id);
    console.addSysMessage(buffer);
    sprintf(buffer, "Trigger type: %s", type);
    console.addSysMessage(buffer);
    sprintf(buffer, "Permanent: %d", permanent);
    console.addSysMessage(buffer);
    sprintf(buffer, "Side Effected: %s", side_effected);
    console.addSysMessage(buffer);
    sprintf(buffer, "x y radius: %f %f %f", x_center, z_center, radius);
    console.addSysMessage(buffer);
    sprintf(buffer, "Associated routine: %s", routine_ptr->getRoutineName());
    console.addSysMessage(buffer);
}

/*******************************************************************************
    function    :   conditionalTrigger::conditionalTrigger
    arguments   :   tId             - temp id
                    tSide           - temp side effected
                    tPermanent      - temp permanent
                    tFunction       - temp function name
                    tRoutine        - temp routine name
    purpose     :   Constructor
    notes       :   <none>
*******************************************************************************/
conditionalTrigger::conditionalTrigger( char* tId, char* tSideEffected, 
                        bool tPermanent, char* tFunction, char* tRoutine )
{
    // Copy id
    strcpy(id, tId);
    strcpy(type, "conditionalTrigger");

    // Copy side effected
    if( tSideEffected == NULL )
        side_effected[0] = '\0';
    else
        strcpy( side_effected, script.script_side );

    // Copy permanent
    permanent = tPermanent;
 
    // Get predicate function variable
    predicate_ptr = script.getPredicate(tFunction);
    if(predicate_ptr == NULL )
        dead = true;

    // Get appropriate routine pointer
    routine_ptr = script.getRoutine( tRoutine );
    if(routine_ptr == NULL )
        dead = true;
}

/*******************************************************************************
    function    :   conditionalTrigger::isActivated
    arguments   :   <none>
    purpose     :   Returns value of predicate function.
    notes       :   <none>
*******************************************************************************/
bool conditionalTrigger::isActivated()
{
        return (script.*(predicate_ptr->function_ptr))();
}

/*******************************************************************************
    function    :   conditionalTrigger::displayTrigger
    arguments   :   none
    purpose     :   Displays important information about goal to console window.
    notes       :   <none>
*******************************************************************************/
void conditionalTrigger::displayTrigger()
{
    char buffer[256];

    sprintf(buffer, "Trigger id: %s", id);
    console.addSysMessage(buffer);
    sprintf(buffer, "Trigger type: %s", type);
    console.addSysMessage(buffer);
    sprintf(buffer, "Permanent: %d", permanent);
    console.addSysMessage(buffer);
    sprintf(buffer, "Side Effected: %s", side_effected);
    console.addSysMessage(buffer);
    sprintf(buffer, "Predicate function: %s", predicate_ptr->name);
    console.addSysMessage(buffer);
    sprintf(buffer, "Associated routine: %s", routine_ptr->getRoutineName());
    console.addSysMessage(buffer);
}

/*******************************************************************************
    function    :   timeTrigger::timeTrigger
    arguments   :   tId             - temp id
                    tSide           - temp side effected
                    tPermanent      - temp permanent
                    tFunction       - temp function name
                    tRoutine        - temp routine name
    purpose     :   Constructor
    notes       :   <none>
*******************************************************************************/
timeTrigger::timeTrigger( char* tId, char* tSideEffected, bool tPermanent, 
                         char tSign, float tTime, char* tRoutine )
{
    // Copy id
    strcpy(id, tId);
    strcpy(type, "timeTrigger");

    // Copy side effected
    if( tSideEffected == NULL )
        side_effected[0] = '\0';
    else
        strcpy( side_effected, script.script_side );

    // Copy permanent
    permanent = tPermanent;
 
    // Used in comparing whether time > or < script.clock  
    sign = tSign;

    // Time to test clock against
    time = tTime;

    // Get appropriate routine pointer
    routine_ptr = script.getRoutine( tRoutine );

    if(routine_ptr == NULL )
        dead = true;
}

/*******************************************************************************
    function    :   timeTrigger::isActivated
    arguments   :   <none>
    purpose     :   Returns value of predicate function.
    notes       :   <none>
*******************************************************************************/
bool timeTrigger::isActivated()
{
    if( sign == '>' )
    {
        if( script.clock > time )
        { 
            char buffer[128];
            sprintf(buffer, "clock %f > time %f", script.clock, time);
            console.addSysMessage(buffer);
            return 1;
        }
    }
    else
    {
        if( script.clock < time )
        {
            char buffer[128];
            sprintf(buffer, "clock %f > time %f", script.clock, time);
            console.addSysMessage(buffer);
            return 1;
        }
    }
       
    return 0;
}

/*******************************************************************************
    function    :   timeTrigger::displayTrigger
    arguments   :   none
    purpose     :   Displays important information about goal to console window.
    notes       :   <none>
*******************************************************************************/
void timeTrigger::displayTrigger()
{
    char buffer[256];

    sprintf(buffer, "Trigger id: %s", id);
    console.addSysMessage(buffer);
    sprintf(buffer, "Trigger type: %s", type);
    console.addSysMessage(buffer);
    sprintf(buffer, "Permanent: %d", permanent);
    console.addSysMessage(buffer);
    sprintf(buffer, "Side Effected: %s", side_effected);
    console.addSysMessage(buffer);
    sprintf(buffer, "Greater than: %c", sign);
    console.addSysMessage(buffer);
    sprintf(buffer, "Time: %f", time);
    console.addSysMessage(buffer);
    sprintf(buffer, "Associated routine: %s", routine_ptr->getRoutineName());
    console.addSysMessage(buffer);
}

/*******************************************************************************
    function    :   goal::goal
    arguments   :   tId                 - temp id
                    tPriority           - temp priority
                    tPredicatePtr       - temp predicate ptr
                    tSuccessRoutine     - temp success routine ptr
                    tFailureRoutine     - temp failure routine ptr
                    tText               - temp text
    purpose     :   Constructor
    notes       :   <none>
*******************************************************************************/
goal::goal( char* tId, int tPriority, char* tPredicatePtr, char* tSuccessRoutine, 
           char* tFailureRoutine )
{
    strcpy(id, tId);
    priority = tPriority;
    predicate_ptr = script.getPredicate(tPredicatePtr);
    success_routine = script.getRoutine(tSuccessRoutine);
    failure_routine = script.getRoutine(tFailureRoutine);
    status = 0;
}

/*******************************************************************************
    function    :   goal::displayGoal
    arguments   :   none
    purpose     :   Displays important information about goal to console window.
    notes       :   <none>
*******************************************************************************/
void goal::displayGoal()
{
    char buffer[256];

    sprintf(buffer, "Goal id: %s", id);
    console.addSysMessage(buffer);

    if( priority == 1 )
        console.addSysMessage("Goal priority: High");
    else if( priority == 2 )
        console.addSysMessage("Goal priority: Low");
    else
        console.addSysMessage("Goal priority: Bonus");

    if( status == -1 )
        console.addSysMessage("Status of goal: FAILED");
    else if( status == 1 )
        console.addSysMessage("Status of goal: SUCCESSEDED");
    else
        console.addSysMessage("Status of goal: INCOMPLETE");

    sprintf(buffer, "Predicate function: %s", predicate_ptr->name);
    console.addSysMessage(buffer);
    sprintf(buffer, "Success function: %s", success_routine->getRoutineName());
    console.addSysMessage(buffer);
    sprintf(buffer, "Failure function: %s", failure_routine->getRoutineName());
    console.addSysMessage(buffer);
}

/*******************************************************************************
    function    :   goal::isComplete
    arguments   :   <none>
    purpose     :   Return -1: failed, 0: not completed, 1: completed
    notes       :   <none>
*******************************************************************************/
int goal::isComplete()
{
    return status;
}

/*******************************************************************************
    function    :   goal::execRoutine
    arguments   :   <none>
    purpose     :   Execute the proper routine based on whether the mission has 
                    successede or failed.
    notes       :   <none>
*******************************************************************************/
void goal::execRoutine()
{
    if( status == -1 )
        failure_routine->execRoutine();
    else if( status == 1 )
        success_routine->execRoutine();
}

/*******************************************************************************
    function    :   script_module::script_module
    arguments   :   <none>
    purpose     :   Constructor
    notes       :   <none>
*******************************************************************************/
script_module::script_module()
{
    // Set clocks
    clock = 0.0f;
    total_game_time = -1.0f;
    end_game_clock = -1.0f;

    // Create ignore string for strtok
    ignore_tokens = new char[3];
    ignore_tokens[0] = ' ';
    ignore_tokens[1] = 9;        // Ascii value for Tab, could also use '\t'
    ignore_tokens[2] = '\0';

    // Setup variables for reading from scripts
    defining_routine = false;
    cmd_from_script = false;
    trigger_head = NULL;
    routine_head = NULL;
    goal_head = NULL;
    predicate_head = NULL;
    command_head = NULL;
    general_load_success = true;
    mission_load_success = true;
    for( int i = 0; i < SCR_SECTION_MAX; i++ )
    {
        sections[i][0] ='\n';
    }
    fillSectionList();

    // Open file output stream
    fout.open("script.log", ios::out);
}

/*******************************************************************************
    function    :   script_module::~script_module
    arguments   :   <none>
    purpose     :   Deconstructor
    notes       :   <none>
*******************************************************************************/
script_module::~script_module()
{
    fout<<"Game exited at: "<<clock<<endl;

    // Cleanup ignore_tokens string
    delete ignore_tokens;

    // Run through trigger and routine lists and remove nodes
    routine* routine_curr = routine_head;
    trigger* trigger_curr = trigger_head;
    goal* goal_curr = goal_head;
    predicate* predicate_curr = predicate_head;
    sc_command* command_curr = command_head;

    while( routine_curr != NULL )
    {
        routine_head = routine_head->next;
        delete routine_curr;
        routine_curr = routine_head;
    }

    while( trigger_curr != NULL )
    {
        trigger_head = trigger_head->next;
        delete trigger_curr;
        trigger_curr = trigger_head;
    }

    while( goal_curr != NULL )
    {
        goal_head = goal_head->next;
        delete goal_curr;
        goal_curr = goal_head;
    }

    while( predicate_curr != NULL )
    {
        predicate_head = predicate_head->next;
        delete predicate_curr;
        predicate_curr = predicate_head;
    }

    while( command_curr != NULL )
    {
        command_head = command_head->next;
        delete command_curr;
        command_curr = command_head;
    }

    // Close file output stream
    fout.close();
}

/*******************************************************************************
    function    :   script_module::addCommand
    arguments   :   command - command to add
    purpose     :   Adds command to list.
    notes       :   <none>
*******************************************************************************/
void script_module::addCommand( char* command )
{
    // Create node 
    sc_command* cmd_ptr = new sc_command( command );

    // Add node to list
    cmd_ptr->next = command_head;
    command_head = cmd_ptr;
}

/*******************************************************************************
    function    :   script_module::addPredicate
    arguments   :   name - Name of function being added
                    pred_ptr_2_func - Pointer to function
    purpose     :   Adds predicate functions to the predicate list.
    notes       :   <none>
*******************************************************************************/
void script_module::addPredicate( char* tName, pred_ptr_2_func tFuncPointer )
{
    predicate* temp = new predicate;
    strcpy( temp->name, tName );
    temp->function_ptr = tFuncPointer;
    temp->next = predicate_head;
    predicate_head = temp;
}

/*******************************************************************************
    function    :   script_module::addTrigger
    arguments   :   trigger_ptr - Pointer to trigger.
    purpose     :   Adds trigger to trigger list.
    notes       :   <none>
*******************************************************************************/
void script_module::addTrigger( trigger* trigger_ptr )
{
    if( trigger_head == NULL )
    {
        trigger_head = trigger_ptr;
        trigger_ptr->next = NULL;
    }
    else
    {
        trigger_ptr->next = trigger_head;
        trigger_head = trigger_ptr;
    }  
}

/*******************************************************************************
    function    :   script_module::loadSuccess
    arguments   :   <none>
    purpose     :   Displays information about the script loading.
    notes       :   <none>
*******************************************************************************/
void script_module::loadSuccess()
{
    console.addSysMessage(" ");
    console.addSysMessage("_SCRIPT LOADING SUCCESS_________________________________________________________________________________________________________");
    if( general_load_success )
        console.addSysMessage("The General script loaded successfully.");
    else
        console.addSysMessage("The General script did not load successfully.");

    if( mission_load_success )
        console.addSysMessage("The Mission script loaded successfully.");
    else
        console.addSysMessage("The Mission script did not load successfully.");
}

/*******************************************************************************
    function    :   script_module::clockGreaterThanGameTime
    arguments   :   <none>
    purpose     :   Function that tests wether clock is greater than game time
                    or not.
    notes       :   <none>
*******************************************************************************/
bool script_module::clockGreaterThanGameTime()
{
    if( (total_game_time != -1) && (clock > total_game_time) )
        return true;
    return false;
}

/*******************************************************************************
    function    :   script_module::createPredicateList
    arguments   :   <none>
    purpose     :   Calls the addPredicate function to add different predicate
                    functions.
    notes       :   <none>
*******************************************************************************/
void script_module::createPredicateList()
{
    // Adding predicate functions to list
    script.addPredicate( "clockGreaterThanGameTime", 
        &script_module::clockGreaterThanGameTime );
}

/*******************************************************************************
    function    :   script_module::displayGoals
    arguments   :   <none>
    purpose     :   Displays the goals in the system.
    notes       :   <none>
*******************************************************************************/
void script_module::displayGoals()
{
    goal* curr = goal_head;

    console.addSysMessage(" ");
    console.addSysMessage("_GOALS__________________________________________________________________________________________________________________________");
    while( curr != NULL )
    {
        curr->displayGoal();
        curr = curr->next;
        console.addSysMessage(" ");
    }
}

/*******************************************************************************
    function    :   script_module::
    arguments   :   name - Name
    purpose     :   
    notes       :   <none>
*******************************************************************************/
void script_module::displayRoutines()
{
    routine* curr = routine_head;
    char buffer[128];

    console.addSysMessage(" ");
    console.addSysMessage("_ROUTINES_______________________________________________________________________________________________________________________");
    while( curr != NULL )
    {
        console.addSysMessage( curr->name );
        for( int i = 0; i < curr->num_commands; i++ )
        {
            buffer[0] = '-';
            buffer[1] = '\0';
            strcat( buffer, curr->command_list[i] );
            console.addSysMessage( buffer );
        }

        console.addSysMessage(" ");
        curr = curr->next;
    }
}

/*******************************************************************************
    function    :   script_module::displaySection
    arguments   :   section - Section name
    purpose     :   Finds all commands with matching sectino and displays them.
    notes       :   <none>
*******************************************************************************/
bool script_module::displaySection( char* section )
{
    sc_command* curr = command_head;
    char buffer[128];

    for( int i = 0; i < SCR_SECTION_MAX; i++ )
    {
        // Section name match
        if( strcmp( sections[i], section ) == 0 )
        {
            if( strcmp( sections[i], "effect" ) == 0 )
            {
                sprintf( buffer, "Because there was not enough space in the parameters section some of the parameters are abbreviated." );
                console.addSysMessage( buffer );
                sprintf( buffer, "Here is a list of the appreviated parameters:" );
                console.addSysMessage( buffer );
                sprintf( buffer, "f_pos = f_xPosition f_yPosition f_zPosition" );
                console.addSysMessage( buffer );
                sprintf( buffer, "f_dir = f_xDirection f_yDirection f_zDirection" );
                console.addSysMessage( buffer );
                sprintf( buffer, "f_sys = f_xSysDirection f_ySysDirection f_zSysDirection" );
                console.addSysMessage( buffer );
                sprintf( buffer, " " );
                console.addSysMessage( buffer );      
            }
            console.addSysMessage(" ");
            sprintf( buffer, "_COMMANDS_______SECTION_________PARAMETERS______________________DEFINITION_____________________________________________________" );
            console.addSysMessage( buffer );

            while( curr )
            {
                if( strcmp( curr->section, section ) == 0 )
                {
                        console.addSysMessage( curr->output );
                }
                curr = curr->next;
            }
            console.addSysMessage(" ");
            return 1;
        }   
    }
    return 0;
}

/*******************************************************************************
    function    :   script_module::displayTriggers
    arguments   :   <none>
    purpose     :   Displays the triggers in the system.
    notes       :   <none>
*******************************************************************************/
void script_module::displayTriggers()
{
    trigger* curr = trigger_head;

    console.addSysMessage(" ");
    console.addSysMessage("_TRIGGERS_______________________________________________________________________________________________________________________");
    while( curr != NULL )
    {
        curr->displayTrigger();
        curr = curr->next;
        console.addSysMessage(" ");
    }
}

/*******************************************************************************
    function    :   script_module::findAndDisplayCommand
    arguments   :   name - Name of command
    purpose     :   Looks through list of commands until match found or end of 
                    file reached. Displays command information if it is found. 
    notes       :   <none>
*******************************************************************************/
bool script_module::findAndDisplayCommand( char* name )
{
    sc_command* curr = command_head;
    char buffer[128];

    while( curr )
    {
        if( strcmp( curr->name, name ) == 0 )
        {
            console.addSysMessage(" ");
            sprintf( buffer, "_COMMAND________SECTION_________PARAMETERS______________________DEFINITION_____________________________________________________" );
            console.addSysMessage( buffer );
            console.addSysMessage( curr->output );
            console.addSysMessage(" ");
            return 1;
        }

        curr = curr->next;
    }

    return 0;
}

/*******************************************************************************
    function    :   script_module::fillSectionList
    arguments   :   <none>
    purpose     :   Sets up section list for help menu navigation.
    notes       :   <none>
*******************************************************************************/
void script_module::fillSectionList()
{
    strcpy(sections[0], "general1");
    strcpy(sections[1], "general2");
    strcpy(sections[2], "effect");
    strcpy(sections[3], "unit");
    strcpy(sections[4], "scenery");
    strcpy(sections[5], "ai");
    strcpy(sections[6], "defines");
}

/*******************************************************************************
    function    :   script_module::getPredicate
    arguments   :   name - Name of the function it's looking for
    purpose     :   Finds a predicate function that has the matching name or if
                    function is not found, returns NULL;
    notes       :   <none>
*******************************************************************************/
predicate* script_module::getPredicate( char* name )
{
    return predicate_head;
    predicate* curr = predicate_head;

    while( curr != NULL )
    {
        if( curr->name != NULL )
        {
            if( strcmp( name, curr->name ) == 0 )
                return curr;
        }
    }

    return NULL;
}

/*******************************************************************************
    function    :   script_module::
    arguments   :   name - Name
    purpose     :   
    notes       :   <none>
*******************************************************************************/
char* script_module::getPredicateName()
{
    return "AlwaysTrue";
}

/*******************************************************************************
    function    :   script_module::
    arguments   :   name - Name
    purpose     :   
    notes       :   <none>
*******************************************************************************/
routine* script_module::getRoutine( char* name )
{
    routine* curr = routine_head;

    while( curr != NULL )
    {
        if( strcmp(curr->name, name) == 0 )
            return curr;

        curr = curr->next;
    }
    
    return NULL;
}

/*******************************************************************************
    function    :   script_module::handler
    arguments   :   command - A string passed in from a script or the command
                    prompt.
    purpose     :   Back bone of the scripting module.  Parses strings passed
                    in from the command prompt or a script, Calls correct 
                    function or stores the command in a routine depending on
                    string origin, and executes commands from routines in the
                    same manner as if from the command prompt.
    notes       :   <none>
*******************************************************************************/
bool script_module::handler( char* command )
{
    char cmd_backup[SCR_CMD_MAX];
    char buffer[SCR_CMD_MAX];
    char* word = NULL;
    char* str_ptr;
    char str_array[SCR_CMD_MAX];
    char str_array2[SCR_CMD_MAX];

    float float_array[4];
    float* float_ptr;
    int int_temp;


    // Remove Beginning spaces and tabs
    int_temp = 0;
    while( (command[int_temp] == ' ') || (command[int_temp] == 9) )
        int_temp++;
    command = &command[int_temp];


    // Remove comments
    str_ptr = strstr( command, "#");
    if(str_ptr != NULL)
        str_ptr[0] = '\0';

    str_ptr = strstr( command, "//");
    if( str_ptr != NULL )
        str_ptr[0] = '\0';


    // Backup command - because strtok is destructive
    strcpy( cmd_backup, command );

    // Tokenize string
    word = strtok( cmd_backup, ignore_tokens );
    
    // If line is blank or the line contained just a comment
    if( word == NULL )
    {
        fout<<"#\n";
        return 1;
    }
  
    /*
        General Game commands
        
        -Name----------------------Arguments----------------command Promt-----script--
        addMessage                  text                            y           y
        addComMessage               text                            y           y
        addSysMessage               text                            y           y
        displayClock                                                y           n
        displayEffects                                              y           n
        displayGoals                                                y           y
        displayRoutines                                             y           y
        displaySounds                                               y           n
        displayTriggers                                             y           n
        exit                                                        y           y
        gainControl                 none                            y           y
        getCamDir                                                   y           n
        getCamPos                                                   y           n
        getheight                   x, y coordinates                y           n
        getControl                                                  y           n
        help                                                        y           n
        jumpClock                   secs                            y           y
        jumpClock                   secs                            y           y
        killEffect                  EffectID                        y           n
        killSound                   SoundId                         y           n
        loadSuccess                                                 y           n
        loseMission                                                 y           y
        loseControl                                                 y           y
        loadScript                  filename                        y           n
        playSound                   filename                        y           y
        query                       buffer                          y           n
        runRoutine                  routine name                    y           y
        setCamDir                   direction                       y           y
        setCamPos                   position                        y           y
        setSideEffected             side                            y           y
        startEffect                 type                            y           y
        winMission                                                  y           y
        winResize                   width, height                   y           n
    */
    /*if( (strcmp(word, "testPath")) == 0 )
    {
        if( cmd_from_script == false )
        {
            if( sscanf( command, "testPath %f %f %f %f", &float_array[0], &float_array[1], &float_array[2], &float_array[3] ) == 4 )
            {
                kVector temp;
                temp = astar.generatePath( kVector(float_array[0], 0.0f, float_array[1]), kVector(float_array[2], 0.0f, float_array[3]) );
                
                sprintf( buffer,"End point: <%f, %f, %f>", temp[0], temp[1], temp[2] );
                console.addSysMessage( buffer );
            }
            else
            {
            incorrectParams();
            return 1;
            }
        }
        
    }*/
    if( (strcmp(word, "addMessage")) == 0 )
    {
        // We must make sure there's another word after addMessage
        // or else the game will crash when we add command 
        word = strtok(NULL, ignore_tokens);
        if( word == NULL )
        {
            incorrectParams();
            return 0;
        }

        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Text will be displayed"<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            console.addMessage( &command[11] );
            return 1;
        }
        else
        {
            fout<<command<<endl;
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word, "addComMessage")) == 0 )
    {
        // We must make sure there's another word after addMessage
        // or else the game will crash when we add command 
        word = strtok(NULL, ignore_tokens);
        if( word == NULL )
        {
            incorrectParams();
            return 0;
        }

        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Text will be displayed"<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            console.addComMessage(&command[14]);
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word, "addSysMessage")) == 0 )
    {
        // We must make sure there's another word after addMessage
        // or else the game will crash when we add command 
        word = strtok(NULL, ignore_tokens);
        if( word == NULL )
        {
            incorrectParams();
            return 0;
        }

        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Text will be displayed"<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            console.addSysMessage(&command[14]);
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }    
    if( (strcmp(word, "displayGoals")) == 0 )
    {
        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  displayGoals"<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            displayGoals();
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word, "displayClock")) == 0 )
    {
        // Handle command based on source
        if( cmd_from_script == false )
        {
            char buffer[128];
            sprintf( buffer, "The current time is: %f", clock );
            console.addSysMessage(buffer);
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word, "displayEffects")) == 0 )
    {
        // Handle command based on source
        if( cmd_from_script == false )
        {
            sound_effect_handler.displayEffects();
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word, "displayGoals")) == 0 )
    {
        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  displayGoals"<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            displayGoals();
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"displayRoutines") == 0) )
    {
        if( cmd_from_script == false )
        {
            displayRoutines();
            return 1;
        }
        else
        {
            fout<<"Command can not be Called from a script"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"displaySounds") == 0) )
    {
        if( cmd_from_script == false )
        {
            sound_effect_handler.displaySounds();
            return 1;
        }
        else
        {
            fout<<"Command can not be Called from a script"<<endl;
            return 0;
        }
    }
    if( (strcmp(word, "displayTriggers")) == 0 )
    {
        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  displayGoals"<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            displayTriggers();
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word, "exit")) == 0 )
    {
        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Game will exit."<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            exit(0);
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"gainControl") == 0) )
    {
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Player gains control"<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            camera.playerControl( true );
            console.addSysMessage("Player has regained control.");
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"getCamDir") == 0) )
    {
        if( cmd_from_script == false )
        {
            float_ptr = camera.getCamDir();
        
            sprintf( buffer, "camera direction: <%f, %f, %f>", float_ptr[0],
                float_ptr[1], float_ptr[2] );
            console.addSysMessage( buffer );
            return 1;
        }
        else
        {
            fout<<"Command can not be Called from a script"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"getCamPos") == 0) )
    {
        if( cmd_from_script == false )
        {
            float_ptr = camera.getCamPos();
        
            sprintf( buffer, "camera position: <%f, %f, %f>.", float_ptr[0],
                float_ptr[1], float_ptr[2] );
            console.addSysMessage( buffer );
            return 1;
        }
        else
        {
            fout<<"Command can not be Called from a script"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"getHeight") == 0) )
    {
        // Make sure getHeight has the correct parameter
        if( sscanf(command, "getHeight %f %f", &float_array[0], &float_array[1]) != 2 )
        {
            incorrectParams();
            return 0;
        }

        if( cmd_from_script == false )
        {
            sprintf( buffer, "The height at x = %f and y = %f is %f",
                float_array[0], float_array[1], map.getHeight(float_array[0], float_array[1]) );
            console.addSysMessage( buffer );
            return 1;
        }
        else
        {
            fout<<"Command can not be Called from a script"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"getControl") == 0) )
    {
        if( cmd_from_script == false )
        {
            if( camera.playerControlActive() == false )
                console.addSysMessage("Player doesn't have control.");
            else
                console.addSysMessage("Player has control.");
            return 1;
        }
        else
        {
            fout<<"Command can not be Called from a script"<<endl;
            return 0;
      
        }
    }
    if( (strcmp(word,"help") == 0) )
    {   
        if( cmd_from_script == false )
        {
            word = strtok(NULL, ignore_tokens);
            
            if( word == NULL )
            {
                console.addSysMessage(" ");
                console.addSysMessage("_HOW TO USE HELP________________________________________________________________________________________________________________");
                console.addSysMessage("The help command is here to make it easier for you to call commands from the console.");
                console.addSysMessage("The commands are diveded into sections so they can be read and displayed to the screen more easily.");
                console.addSysMessage(" ");
                console.addSysMessage("To look up the section names, type: help sections");
                console.addSysMessage("To look up the commands in a section, type: help section_name");
                console.addSysMessage("To look up a command, type: help command_name");
                console.addSysMessage("If you do look up a command, you may see something like this:");
                console.addSysMessage("help            general1        s_cmd OR s_section OR <none>    Displays info about commands.");
                console.addSysMessage(" ");
                console.addSysMessage("All parameters of commands have a preceding letter, this letter indicates the parameter type.");
                console.addSysMessage("Here is a listing of these types and their definitions:");
                console.addSysMessage("i - integer (whole number)");
                console.addSysMessage("f - float (a number that may have a decimal point)");
                console.addSysMessage("b - boolean (0 or 1)");
                console.addSysMessage("c - single character");
                console.addSysMessage("s - string/word ");
                console.addSysMessage("t - list of words (maybe a sentence)");
                console.addSysMessage("The OR is a special word and means one or the other but not both (also know as the XOR).");
                console.addSysMessage(" ");
                console.addSysMessage("If you would like a list of ways to call help, type: help help");
                console.addSysMessage(" ");

                return 1;
            }

            // If parameter = "sections"
            if( strcmp(word, "sections") == 0 )
            {
                console.addSysMessage(" ");
                console.addSysMessage("_SECTIONS_______________________________________________________________________________________________________________________");

                // Display Sections
                for( int i = 0; i < SCR_SECTION_MAX; i++ )
                {
                    console.addSysMessage( sections[i] );
                }
                console.addSysMessage(" ");
                return 1;
            }

            // Look to see if parameter is a section name
            if( displaySection( word ) )
                return 1;
               
            // Check to see if paramter is a command
            if( findAndDisplayCommand( word ) == true )
                return 1;

            console.addSysMessage("Parameter was not a command or section name.");
            console.addSysMessage(" ");
            return 0;
        }
        else
        {
            fout<<"Command can not be Called from a script"<<endl;
            return 0;
        }
    }
    if( (strcmp(word, "jumpClock")) == 0 )
    {
        // Make sure jumpClock has the correct parameter
        if( sscanf( command, "%*s %f", &float_array[0]) != 1 )
        {
            incorrectParams();
            return 0;
        }

        // Handle command based on source
        if( defining_routine == true )
        {
            if( float_array[0] < 0 )
            {
                fout<<"Clock can not be jump backwards"<<endl;
                return 0;
            }

            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Jump clock by "<<float_array[0]<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            if( float_array[0] < 0 )
            {
                console.addSysMessage("Clock can not be jump backwards.");
                return 0;
            }

            // Jump clock
            clock += float_array[0];
            
            sprintf( buffer, "Game clock now at %f.", float_array[0] );
            console.addSysMessage( buffer );
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word, "jumpClockTo")) == 0 )
    {
        // Make sure jumpClock has the correct parameter
        if( sscanf(command, "%*s %f", &float_array[0]) != 1 )
        {
            incorrectParams();
            return 0;
        }

        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Jump clock by "<<float_array[0]<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            if( float_array[0] < clock )
            {
                sprintf( buffer, "Time can not be set back. Current time is %f.", clock );
                console.addSysMessage( buffer );
                return 0;
            }
            else
            {
                clock = float_array[0];
            
                sprintf( buffer, "Game clock now at %f", float_array[0] );
                console.addSysMessage( buffer );
                return 1;
            }
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word, "killEffect")) == 0 )
    {
        // Make sure killEffect has the correct parameter
        if( sscanf(command, "%*s %d ", &int_temp) != 1 )
        {
            incorrectParams();
            return 0;
        }

        if( cmd_from_script == false )
        {
            if( sound_effect_handler.killEffect( int_temp ) )
            {
            
                sprintf( buffer, "Effect %d has been killed.", int_temp );
                console.addSysMessage( buffer );
            }
            else
            {
                sprintf( buffer, "Effect with id %d was not found in system.", int_temp );
                console.addSysMessage( buffer );
            }
            return 1;
        }
        else
        {
            fout<<"Command can not be Called in a script."<<endl;
            return 0;
        }
    }
    if( (strcmp(word, "killSound")) == 0 )
    {
        // Make sure killSound has the correct parameter
        if( sscanf(command, "%*s %d", &int_temp) != 1 )
        {
            incorrectParams();
            return 0;
        }

        if( cmd_from_script == false )
        {
            if( sound_effect_handler.killSound(int_temp) )
            {
                sprintf( buffer, "Sound %d has been stopped.", int_temp );
                console.addSysMessage( buffer );
            }
            else
            {
                sprintf( buffer, "Sound with id %d was not found in system.", int_temp );
                console.addSysMessage( buffer );
            }
            return 1;
        }
        else
        {
            fout<<"Command can not be Called in a script"<<endl;
            return 0;
        }
    }
    if( (strcmp(word, "loadSuccess")) == 0 )
    {
        // Handle command based on source
        if( cmd_from_script == false )
        {
            loadSuccess();
            return 1;
        }
        else
        {
            fout<<"Command can not be Called from script"<<endl;
            return 0;
        }
    }
    if( (strcmp( word,"loseMission") == 0) )
    {
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Player loses mission"<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            // Write lose mission func here
            console.addSysMessage("Player has lost Mission!");
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"loseControl") == 0) )
    {
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Player losses control"<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            camera.playerControl( false );
            console.addSysMessage("Player has lost control.");
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word, "loadScript")) == 0 )
    {
        char file_name[SCR_CMD_MAX];
        bool loading_success;
        // Make sure loadScript has the correct parameter
        if( sscanf(command, "%*s %s ", file_name) != 1 )
        {
            incorrectParams();
            return 0;
        }

        // Handle command based on source
        if( cmd_from_script == false )
        {
            // If we don't set cmd_from_script to true
            // the handler will execute all of the commands
            // defined inside the routines and won't let
            // use declare defines, triggers, and so on.
            cmd_from_script = true;
            loading_success = loadScript( file_name );
            cmd_from_script = false;
            
            if( loading_success == 1 )
                console.addSysMessage("Script loaded successfully.");
            else
                console.addSysMessage("The General script did not load successfully.");
           
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word, "playSound")) == 0 )
    {
        // Make sure playSound has the correct parameter
        if( sscanf(command, "%*s %s", str_array) != 1 )
        {
            incorrectParams();
            return 0;
        }

        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Sound "<<str_array<<" added."<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            sound_effect_handler.startSound( command );
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"query") == 0) )
    {
        // Make sure query has the correct parameter
        if( sscanf(command, "%*s %s", &buffer[0]) != 1 )
        {
            incorrectParams();
            return 0;
        }

        if( cmd_from_script == false )
        {
            // Reference Library query string
            str_ptr = db.query( &buffer[0], &buffer[64] );
            
            if( str_ptr != NULL )
            {
                strcpy( buffer, "DB returns: ." );
                strcat( buffer, str_ptr );
                console.addSysMessage( buffer );
            }
            else
            {
                console.addSysMessage("DB returns: <NULL>.");
            }
            return 1;
        }
        else
        {
            fout<<"Command can not be Called from a script"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"runRoutine") == 0) )
    {
        char routineName[128];

        // Make sure runRoutine has the correct parameter
        if( sscanf(command, "%*s %s", routineName) != 1 )
        {
            incorrectParams();
            return 0;
        }


        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"Run routine"<<endl;
            }

            return 1;
        }
        else if( cmd_from_script == false )
        {
            routine* curr = routine_head;

            // Find routine that matches name
            while( curr != NULL )
            {
                if( strcmp( curr->name, routineName ) == 0 )
                {
                    // Execute routine
                    curr->execRoutine();
                    return 1;
                }

                curr = curr->next;
                
            }

            // Routine not found
            console.addSysMessage("Routine not found.");
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"setCamDir") == 0) )
    {
        // Make sure setCamDir has the correct parameter
        if( sscanf(command, "%*s %f %f %f", &float_array[0], &float_array[1], &float_array[2]) != 3 )
        {
            incorrectParams();
            return 0;
        }


        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                sprintf( buffer, "  Camera direction set to <%f, %f, %f>.", 
                    float_array[0], float_array[1], float_array[2] );

                fout<<buffer<<endl;
            }

            return 1;
        }
        else if( cmd_from_script == false )
        {
            camera.setCamDir( float_array );
            camera.forceValues();
            
            sprintf(buffer, "Camera direction set to <%f, %f, %f>.", 
                float_array[0], float_array[1], float_array[2]);
            console.addSysMessage(buffer);
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"setCamPos") == 0) )
    {
        // Make sure setCamPos has the correct parameter
        if( sscanf(command, "%*s %f %f %f", &float_array[0], &float_array[1], &float_array[2]) != 3 )
        {
            incorrectParams();
            return 0;
        }


        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                sprintf( buffer, "  Camera position set to <%f, %f, %f>", 
                    float_array[0], float_array[1], float_array[2] );

                fout<<buffer<<endl;
            }

            return 1;
        }
        else if( cmd_from_script == false )
        {
            camera.setCamPos( float_array );
            camera.forceValues();
            
            sprintf( buffer, "Camera position set to <%f, %f, %f>.", 
                float_array[0], float_array[1], float_array[2] );
            console.addSysMessage( buffer );
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }    
    if( (strcmp(word, "setSideEffected")) == 0 )
    {
        char side_effected[16];
        // Make sure setSideEffected has the correct parameter
        if( sscanf(command, "%*s %s ", side_effected) != 1 )
        {
            incorrectParams();
            return 0;
        }

        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                sprintf( buffer, "Side effected will be set to: %s", side_effected );
                fout<<buffer<<endl;
            }
            return 1;
        }
        else
        {
            strcpy( script_side, side_effected );
            
            if( cmd_from_script == false )
            {
                sprintf( buffer, "Side effected has now been set to: %s.", side_effected );
                console.addSysMessage( buffer );
            }
            else
            {
                sprintf( buffer, "Side effected will be set to: %s.", side_effected );
                fout<<buffer<<endl;

            }
            
            return 1;
        }
    }
    if( (strcmp(word,"winMission") == 0) )
    {
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Player Wins mission"<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            // Write win mission func here
            console.addSysMessage("Player has won Mission!");
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }   
    if( (strcmp(word,"winResize") == 0) )
    {

        // Make sure winResize has the correct parameter
        if( sscanf(command, "%*s %i %i", &game_setup.screen_width, &game_setup.screen_height) != 2 )
        {
            incorrectParams();
            return 0;
        }

        if( cmd_from_script == false )
        {
            // I wonder if this causes a memory leak??? Depends if SDL handles the
            // deletion of old events - something to look up one day.
            SDL_Event* event = new SDL_Event;
            
            event->type = SDL_VIDEORESIZE;
            event->resize.type = SDL_VIDEORESIZE;
            event->resize.w = game_setup.screen_width;
            event->resize.h = game_setup.screen_height;
            
            SDL_PushEvent(event);
            sprintf(buffer, "screen resized to: %i x %i.",game_setup.screen_width, game_setup.screen_height);
            console.addSysMessage(buffer);
            return 1;
        }
        else
        {
            fout<<"Command can not be Called from a script"<<endl;
            return 0;
        }
    }

    /*
    Effect Game commands
    
    -Name----------------------Arguments----------------command Promt-----script--
    startEffect     effect     s_name                          y               y                     
    startEffect     effect     s_name f_px f_py f_pz           y               y
    startEffect     effect     s_name f_px f_py f_pz f_mod     y               y
    startEffect     effect     s_name f_pos f_dir              y               y
    startEffect     effect     s_name f_pos f_dir f_mod        y               y
    startEffect     effect     s_name f_pos f_dir f_mod f_sys  y               y
    */
    if( (strcmp(word, "startEffect")) == 0 )
    {
        // Make sure startEffect has the correct parameter
        if( sscanf(command, "%*s %s ", str_array) != 1 )
        {
            incorrectParams();
            return 0;
        }

        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  start"<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            sound_effect_handler.startEffect( command );
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    
    /*
    Unit modifier commands

    -Name----------------------Arguments----------------command Promt-----script--
    addWaypoint                ID, position                    y           y        
    destroyUnit                ID                              y           y
    killWaypoints              ID                              y           y
    killCrew                   ID                              y           y
    moveTo                     ID, x, z                        y           y
    neutUnitFiring             ID                              y           y
    neutUnitMove               ID                              y           y
    removeUnit                 ID                              y           y
    repairUnit                 ID                              y           y
    restoreAmmo                ID                              y           y
    restoreCrew                ID                              y           y
    restUnitFiring             ID                              y           y
    restUnitMove               ID                              y           y
    */
    if( (strcmp(word,"addWaypoint") == 0) )
    {
        // Make sure addWaypoint has the correct parameter
        if( sscanf(command, "%*s %s %f %f", str_array2, &float_array[0], &float_array[1]) != 3 )
        {
            incorrectParams();
            return 0;
        }

        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Waypoint will be added to tank."<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            // Get pointer to object
            object* objectToMove = objects.getUnitWithID(str_array2);
            
            // Checks to make sure object pointer points to something
            if( objectToMove == NULL )
            {
                sprintf( buffer, "Object with id %s does not exist.", str_array2 );
                console.addSysMessage( buffer );
                return 1;
            }
            
            // Add waypoint to object
            if(objectToMove->obj_type == OBJ_TYPE_TANK ||
               objectToMove->obj_type == OBJ_TYPE_VEHICLE ||
               objectToMove->obj_type == OBJ_TYPE_ATG ||
               objectToMove->obj_type == OBJ_TYPE_ATR
              )
            {          
                (dynamic_cast<moving_object*>(objectToMove))->addWaypoint(kVector( float_array[0],  map.getHeight(float_array[0], float_array[1]), float_array[1]));
                sprintf(buffer, "Point <%f,%f,%f> has been added to tanks path.", float_array[0], map.getHeight(float_array[0], float_array[1]), float_array[1]);
                console.addSysMessage( buffer );
                return 1;
            }
            
            sprintf( buffer, "Id %s does not belong to a tank/vehicle/atg/atr.", str_array2 );
            console.addSysMessage( buffer );
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"destroyUnit") == 0) )
    {
        // Make sure destroyUnit has the correct parameter
        if( sscanf(command, "%*s %s", str_array2) != 1 )
        {
            incorrectParams();
            return 0;
        }

        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Unit will be destroyed"<<endl;
            }
            
            return 1;
        }
        else if( cmd_from_script == false )
        {
            // Get pointer to object
            object* objectToDestroy = objects.getUnitWithID(str_array2);
            
            // Checks to make sure object pointer points to something
            if( objectToDestroy == NULL )
            {
                sprintf( buffer, "Unit with id %s does not exist.", str_array2 );
                console.addSysMessage( buffer );
                return 1;
            }
            
            // Destroys tank/vehicle
            if(objectToDestroy->obj_type == OBJ_TYPE_TANK ||
               objectToDestroy->obj_type == OBJ_TYPE_VEHICLE ||
               objectToDestroy->obj_type == OBJ_TYPE_ATG ||
               objectToDestroy->obj_type == OBJ_TYPE_ATR
              )
            {
                // Adding
                int snd_id;
                effects.addEffect(SE_BIG_EXPLOSION, objectToDestroy->pos, 8.0f);
                snd_id = sounds.addSound(SOUND_GROUND_EXPLOSION,
                    SOUND_HIGH_PRIORITY, objectToDestroy->pos(), SOUND_PLAY_ONCE);
                sounds.setSoundRolloff(snd_id, 0.01);
                sounds.auxModOnExplosive(snd_id, 2.25f);
                
                // Kill crew
                int crew_count;
                crew_count = (dynamic_cast<unit_object*>(objectToDestroy))->crew.getCrewmanCount();
                for( int i = 0; i < crew_count; i++ )
                    (dynamic_cast<unit_object*>(objectToDestroy))->crew.reduceHealth( i, 1.0f );
                
                // Damage tank
                
                
                // Destroy motor
                (dynamic_cast<moving_object*>(objectToDestroy))->motor.reduceMotorLife( 1.0f );
                
                
                console.addSysMessage("Unit has been destroyed."); 
                return 1;
            }
                
            sprintf( buffer, "Id %s does not belong to a tank/vehicle/atg/atr.", str_array2 );
            console.addSysMessage( buffer );
            return 1;
            
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"killCrew") == 0) )
    {
        // Make sure killCrew has the correct parameter
        if( sscanf(command, "%*s %s", str_array2) != 1 )
        {
            incorrectParams();
            return 0;
        }

        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Crew will be killed"<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            // Get pointer to object
            object* objectToDestroy = objects.getUnitWithID(str_array2);
            
            // Checks to make sure object pointer points to something
            if( objectToDestroy == NULL )
            {
                sprintf( buffer, "Unit with id %s does not exist.", str_array2 );
                console.addSysMessage( buffer );
                return 1;
            }
            
            // Destroys tank/vehicle
            if(objectToDestroy->obj_type == OBJ_TYPE_TANK ||
               objectToDestroy->obj_type == OBJ_TYPE_VEHICLE ||
               objectToDestroy->obj_type == OBJ_TYPE_ATG ||
               objectToDestroy->obj_type == OBJ_TYPE_ATR
              )
            {
                // Kill crew
                int crew_count;
                crew_count = (dynamic_cast<unit_object*>(objectToDestroy))->crew.getCrewmanCount();
                for( int i = 0; i < crew_count; i++ )
                    (dynamic_cast<unit_object*>(objectToDestroy))->crew.reduceHealth( i, 1.0f );
                
                console.addSysMessage("Unit's crew has been killed"); 
                return 1;
            }
                
            sprintf( buffer, "Id %s does not belong to a tank/vehicle/atg/atr.", str_array2 );
            console.addSysMessage( buffer );
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"killWaypoints") == 0) )
    {
        // Make sure killWaypoints has the correct parameter
        if( sscanf(command, "%*s %s", str_array2) != 1 )
        {
            incorrectParams();
            return 0;
        }

        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Unit's waypoints will be removed."<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            // Get pointer to object
            object* objectToMove = objects.getUnitWithID(str_array2);
            
            // Checks to make sure object pointer points to something
            if( objectToMove == NULL )
            {
                sprintf( buffer, "Unit with id %s does not exist.", str_array2 );
                console.addSysMessage( buffer );
                return 1;
            }
            
            // Kill waypoints of object
            if(objectToMove->obj_type == OBJ_TYPE_TANK ||
               objectToMove->obj_type == OBJ_TYPE_VEHICLE ||
               objectToMove->obj_type == OBJ_TYPE_ATG ||
               objectToMove->obj_type == OBJ_TYPE_ATR
              )
            {
                (dynamic_cast<moving_object*>(objectToMove))->killWaypoints();           
                sprintf( buffer, "Unit %s's waypoints have removed.", str_array2 );
                console.addSysMessage( buffer );
                return 1;
            }
            
            sprintf( buffer, "Id %s does not belong to a tank/vehicle/atg/atr.", str_array2 );
            console.addSysMessage( buffer );
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"moveTo") == 0) )
    {
        // Make sure moveTo has the correct parameter
        if( sscanf(command, "%*s %s %f %f", str_array2, &float_array[0], &float_array[1]) != 3 )
        {
            incorrectParams();
            return 0;
        }

        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                sprintf( buffer, "  Unit will move to point <%f, %f, %f>", 
                    float_array[0], map.getHeight( float_array[0], float_array[1] ), float_array[1] );
                fout<<buffer<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            // Get pointer to object
            object* objectToMove = objects.getUnitWithID(str_array2);
            
            // Checks to make sure object pointer points to something
            if( objectToMove == NULL )
            {
                sprintf( buffer, "Unit with id %s does not exist.", str_array2 );
                console.addSysMessage( buffer );
                return 1;
            }
            
            // Move object to point specified
            if(objectToMove->obj_type == OBJ_TYPE_TANK ||
               objectToMove->obj_type == OBJ_TYPE_VEHICLE ||
               objectToMove->obj_type == OBJ_TYPE_ATG ||
               objectToMove->obj_type == OBJ_TYPE_ATR
              )
            {
                (dynamic_cast<moving_object*>(objectToMove))->killWaypoints();           
                (dynamic_cast<moving_object*>(objectToMove))->addWaypoint(kVector( float_array[0], map.getHeight(float_array[0], float_array[1]), float_array[1]));
                sprintf(buffer, "Unit is moving to: <%f,%f,%f>", float_array[0], map.getHeight(float_array[0], float_array[1]), float_array[1]);
                console.addSysMessage( buffer );
                return 1;
            }
            
            sprintf( buffer, "Id %s does not belong to a tank/vehicle/atg/atr.", str_array2 );
            console.addSysMessage( buffer );
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"neutUnitFiring") == 0) )
    {
        // Make sure neutUnitFiring has the correct parameter
        if( sscanf(command, "%*s %s", str_array2) != 1 )
        {
            incorrectParams();
            return 0;
        }

        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Unit firing will be neutralized"<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            console.addSysMessage("Unit firing has been neutralized"); 
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"neutUnitMove") == 0) )
    {
        // Make sure neutUnitMove has the correct parameter
        if( sscanf(command, "%*s %s", str_array2) != 1 )
        {
            incorrectParams();
            return 0;
        }

        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Unit movement will be neutralized"<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            // Get pointer to object
            object* unit = objects.getUnitWithID(str_array2);
            
            // Checks to make sure object pointer points to something
            if( unit == NULL )
            {
                sprintf( buffer, "Unit with id %s does not exist.", str_array2 );
                console.addSysMessage( buffer );
                return 1;
            }
            
            if(unit->obj_type == OBJ_TYPE_TANK ||
               unit->obj_type == OBJ_TYPE_VEHICLE ||
               unit->obj_type == OBJ_TYPE_ATG ||
               unit->obj_type == OBJ_TYPE_ATR
              )
            {
                // Destroy motor
                (dynamic_cast<moving_object*>(unit))->motor.reduceMotorLife( 1.0f );
                                
                console.addSysMessage("Unit movement has been neutralized"); 
                return 1;
            }
                
            sprintf( buffer, "Id %s does not belong to a tank/vehicle/atg/atr.", str_array2 );
            console.addSysMessage( buffer );
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"removeUnit") == 0) )
    {
        // Make sure removeUnit has the correct parameter
        if( sscanf(command, "%*s %s", str_array2) != 1 )
        {
            incorrectParams();
            return 0;
        }

        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Unit will be removed"<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            // Get pointer to object
            object* objectToRemove = objects.getUnitWithID(str_array2);
            
            // Checks to make sure object pointer points to something
            if( objectToRemove == NULL )
            {
                sprintf( buffer, "Unit with id %s does not exist.", str_array2 );
                console.addSysMessage( buffer );
                return 1;
            }
            
            // Remove object from system
            if(objectToRemove->obj_type == OBJ_TYPE_TANK ||
               objectToRemove->obj_type == OBJ_TYPE_VEHICLE ||
               objectToRemove->obj_type == OBJ_TYPE_ATG ||
               objectToRemove->obj_type == OBJ_TYPE_ATR
              )
            {
                objects.removeObject( objectToRemove );        
                console.addSysMessage("Unit has been removed"); 
                return 1;
            }
            
                        
            sprintf( buffer, "Id %s does not belong to a tank/vehicle/atg/atr.", str_array2 );
            console.addSysMessage( buffer );
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"repairUnit") == 0) )
    {
        // Make sure repairUnit has the correct parameter
        if( sscanf(command, "%*s %s", str_array) != 1 )
        {
            incorrectParams();
            return 0;
        }

        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Unit will be repaired"<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            // Get pointer to object
            object* unit = objects.getUnitWithID(str_array2);
            
            // Checks to make sure object pointer points to something
            if( unit == NULL )
            {
                sprintf( buffer, "Unit with id %s does not exist.", str_array2 );
                console.addSysMessage( buffer );
                return 1;
            }
            
            // Remove object from system
            if(unit->obj_type == OBJ_TYPE_TANK ||
               unit->obj_type == OBJ_TYPE_VEHICLE ||
               unit->obj_type == OBJ_TYPE_ATG ||
               unit->obj_type == OBJ_TYPE_ATR
              )
            {
                // Restore motor
                (dynamic_cast<moving_object*>(unit))->motor.reduceMotorLife( -1.0f );
                
                // Restore tank
                
                      
                console.addSysMessage("Unit has been repaired"); 
                return 1;
            }
            
                     
                // Call repairUnit function
                console.addSysMessage("Unit has been repaired"); 
                return 1;
               
            sprintf( buffer, "Id %s does not belong to a tank/vehicle/atg/atr.", str_array2 );
            console.addSysMessage( buffer );
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"restoreAmmo") == 0) )
    {
        // Make sure restoreAmmo has the correct parameter
        if( sscanf(command, "%*s %s", str_array2) != 2 )
        {
            incorrectParams();
            return 0;
        }

        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Unit ammo will be restored"<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            // do stuff here
            console.addSysMessage("Unit ammo has been restored"); 
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"restoreCrew") == 0) )
    {
        // Make sure restoreCrew has the correct parameter
        if( sscanf(command, "%*s %s", str_array2) != 1 )
        {
            incorrectParams();
            return 0;
        }

        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Unit's crew will be restored"<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            // Get pointer to object
            object* unit = objects.getUnitWithID(str_array2);
            
            // Checks to make sure object pointer points to something
            if( unit == NULL )
            {
                sprintf( buffer, "Unit with id %s does not exist.", str_array2 );
                console.addSysMessage( buffer );
                return 1;
            }
            
            // Destroys tank/vehicle
            if(unit->obj_type == OBJ_TYPE_TANK ||
               unit->obj_type == OBJ_TYPE_VEHICLE ||
               unit->obj_type == OBJ_TYPE_ATG ||
               unit->obj_type == OBJ_TYPE_ATR
              )
            {
                // Kill crew
                int crew_count;
                crew_count = (dynamic_cast<unit_object*>(unit))->crew.getCrewmanCount();
                for( int i = 0; i < crew_count; i++ )
                    (dynamic_cast<unit_object*>(unit))->crew.reduceHealth( i, -1.0f );
                
                console.addSysMessage("Unit's crew has been restored."); 
                return 1;
            }
                
            sprintf( buffer, "Id %s does not belong to a tank/vehicle/atg/atr.", str_array2 );
            console.addSysMessage( buffer );
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"restUnitFiring") == 0) )
    {
        // Make sure restUnitFiring has the correct parameter
        if( sscanf(command, "%*s %s", str_array2) != 1 )
        {
            incorrectParams();
            return 0;
        }

        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Unit's firing will be restored"<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            console.addSysMessage("Unit's firing has been restored"); 
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"restUnitMove") == 0) )
    {
        // Make sure restUnitMove has the correct parameter
        if( sscanf(command, "%*s %s", str_array2) != 1 )
        {
            incorrectParams();
            return 0;
        }

        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Unit's movement will be restored"<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            // Get pointer to object
            object* unit = objects.getUnitWithID(str_array2);
            
            // Checks to make sure object pointer points to something
            if( unit == NULL )
            {
                sprintf( buffer, "Unit with id %s does not exist.", str_array2 );
                console.addSysMessage( buffer );
                return 1;
            }
            
            // Destroys tank/vehicle
            if(unit->obj_type == OBJ_TYPE_TANK ||
               unit->obj_type == OBJ_TYPE_VEHICLE ||
               unit->obj_type == OBJ_TYPE_ATG ||
               unit->obj_type == OBJ_TYPE_ATR
              )
            {
                // Restore motor
                (dynamic_cast<moving_object*>(unit))->motor.reduceMotorLife( -1.0f );
                
                console.addSysMessage("Unit's movement has been restored"); 
                return 1;
            }
                
            sprintf( buffer, "Id %s does not belong to a tank/vehicle/atg/atr.", str_array2 );
            console.addSysMessage( buffer );
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }
    
    /*
    Scenery commands

    -Name----------------------Arguments----------------command Promt-----script--
    destroyBuilding           (object ID)                      y           y
    */
    if( (strcmp(word,"destroyBuilding") == 0) )
    {
        // Make sure destroyBuilding has the correct parameter
        if( sscanf(command, "%*s %s", str_array2) != 1 )
        {
            incorrectParams();
            return 0;
        }

        // Handle command based on source
        if( defining_routine == true )
        {
            if( routine_head != NULL )
            {
                routine_head->addCommand( command );
                fout<<"  Building will be destroyed"<<endl;
            }
            return 1;
        }
        else if( cmd_from_script == false )
        {
            console.addSysMessage("Building is destroyed");
            return 1;
        }
        else
        {
            fout<<"Command can not be Called outside of define"<<endl;
            return 0;
        }
    }

    /*
    AI commands

    -Name----------------------Arguments----------------command Promt-----script--
    airAttack                  position                       y           n
    attackEnemy                tank ID, enemy ID, type        y           n
    halt                       tank ID                        y           n
    modeToDefensive            tank ID                        y           n
    modeToOffensive            tank ID                        y           n
    moveIntoForm               tank ID, formation, position   y           n
    recon                      tank ID                        y           n
    retreat                    tank ID, position, type        y           n
    routeEnemy                 tank ID                        y           n
    stopAttacking              tank ID                        y           n
    */
    if( (strcmp(word,"airAttack") == 0) )
    {
        // Make sure airAttack has the correct parameter
        if( sscanf(command, "%*s %f %f", &float_array[0], &float_array[1]) != 2 )
        {
            incorrectParams();
            return 0;
        }

        if( cmd_from_script == false )
        {
                console.addSysMessage("Air attack command"); 
                return 1;
        }
        else
        {
            fout<<"Command can not be Called from a script."<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"attackEnemy") == 0) )
    {
        // Make sure attackEnemy has the correct parameter
        if( sscanf(command, "%*s %s %s %d", str_array2, str_array2, &int_temp) != 3 )
        {
            incorrectParams();
            return 0;
        }

        if( cmd_from_script == false )
        {
                console.addSysMessage("Attack enemy command"); 
                return 1;
        }
        else
        {
            fout<<"Command can not be Called from a script."<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"halt") == 0) )
    {
        // Make sure halt has the correct parameter
        if( sscanf(command, "%*s %s", str_array2) != 1 )
        {
            incorrectParams();
            return 0;
        }

        if( cmd_from_script == false )
        {
                console.addSysMessage("Halt command"); 
                return 1;
        }
        else
        {
            fout<<"Command can not be Called from a script."<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"modeToDefensive") == 0) )
    {
        // Make sure setModeToDefensive has the correct parameter
        if( sscanf(command, "%*s %s", str_array2) != 1 )
        {
            incorrectParams();
            return 0;
        }

        if( cmd_from_script == false )
        {
                console.addSysMessage("Set mode to defensive command"); 
                return 1;
        }
        else
        {
            fout<<"Command can not be Called from a script."<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"modeToOffensive") == 0) )
    {
        // Make sure setModeToOffensive has the correct parameter
        if( sscanf(command, "%*s %s", str_array2) != 1 )
        {
            incorrectParams();
            return 0;
        }

        if( cmd_from_script == false )
        {
                console.addSysMessage("Set mode to offensive command"); 
                return 1;
        }
        else
        {
            fout<<"Command can not be Called from a script."<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"moveIntoForm") == 0) )
    {
        // Make sure moveIntoFormation has the correct parameter
        if( sscanf(command, "%*s %s %s %f %f", 
            str_array2, str_array2, &float_array[0], &float_array[1]) != 4 )
        {
            incorrectParams();
            return 0;
        }

        if( cmd_from_script == false )
        {
                console.addSysMessage("Move into formation command"); 
                return 1;
        }
        else
        {
            fout<<"Command can not be Called from a script."<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"recon") == 0) )
    {
        // Make sure recon has the correct parameter
        if( sscanf(command, "%*s %s", str_array2) != 1 )
        {
            incorrectParams();
            return 0;
        }

        if( cmd_from_script == false )
        {
                console.addSysMessage("Recon command"); 
                return 1;
        }
        else
        {
            fout<<"Command can not be Called from a script."<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"retreat") == 0) )
    {
        // Make sure retreat has the correct parameter
        if( sscanf(command, "%*s %s %f %f %s", 
            str_array2, &float_array[0], &float_array[1], str_array2) != 4 )
        {
            incorrectParams();
            return 0;
        }

        if( cmd_from_script == false )
        {
                console.addSysMessage("Retreat from enemy command"); 
                return 1;
        }
        else
        {
            fout<<"Command can not be Called from a script."<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"routeEnemy") == 0) )
    {
        // Make sure routeEnemy has the correct parameter
        if( sscanf(command, "%*s %s", str_array2) != 1 )
        {
            incorrectParams();
            return 0;
        }

        if( cmd_from_script == false )
        {
                console.addSysMessage("Route enemy command"); 
                return 1;
        }
        else
        {
            fout<<"Command can not be Called from a script."<<endl;
            return 0;
        }
    }
    if( (strcmp(word,"stopAttacking") == 0) )
    {
        // Make sure stopAttacking has the correct parameter
        if( sscanf(command, "%*s %s", str_array2) != 1 )
        {
            incorrectParams();
            return 0;
        }

        if( cmd_from_script == false )
        {
                console.addSysMessage("Stop attacking command"); 
                return 1;
        }
        else
        {
            fout<<"Command can not be Called from a script."<<endl;
            return 0;
        }
    }
    
    /*
    Define commands

    -Name----------------------Arguments----------------command Promt-----script--
    define group            GroupID, {UnitID}                   n           y
    define goal             GoalID, Priority, Text              n           y
    define trigger      TriggerID, Type, Params, Routine        n           y
    define routine          Name, {Commands}                    n           y

    // Groups
    Define Group GroupID UnitID UnitID .... UnitID
    GroupID - Group ID tag string
    UnitID - Unit ID tag string
    
    // Mission goals
    Define Goal GoalID GoalPriority "GoalText"
    GoalID - Integer # tag
    GoalPriority - [1-3] - 1: Primary, 2: Seconday, 3: Bonus
    GoalText - String to display

    // Triggers
    Define Trigger TriggerID TriggerType [TriggerParameters] Routine

    TriggerID - Trigger ID tag string
    TriggerType:
    CellArea - Rectangular area to test against
        Parameters: XMin YMin XMax YMax GroupID Routine
        GroupID - What units to test against (Group ID tag string)
        Routine  - Routine to run once trigger is satisfied
    CellRadius - Circular area to test against
        Parameters: X Y Radius GroupID Routine
        GroupID - What units to test against (Group ID tag string)
        Routine  - Routine to run once trigger is satisfied
    Conditional - Conditional operator to test against
        Paramters: Statement Routine
        Statement - A conditional statement to test against:
            GroupIsDead GroupID
            UnitIsDead UnitID
            ... others???

    // Routine
    Routine - Routine to run once trigger is satisfied
    */
    if( (strcmp(word,"define") == 0) )
    {
        word = strtok(NULL, ignore_tokens);
        if( word == NULL )
        {
            incorrectParams();
            return 0;
        }

        if( strcmp(word, "goal") == 0 )
        {
            int priority;
            char predicate_func[SCR_CMD_MAX];
            char success[SCR_CMD_MAX];
            char failure[SCR_CMD_MAX];
            char text[SCR_CMD_MAX];

            // Id priority predicate success failure -tempText-
            if( sscanf(command, "%*s %*s %s %d %s %s %s %s", 
                str_array, &priority, predicate_func, success, failure, text) != 6)
            {
                incorrectParams();
                return 0;
            }

            if( (defining_routine == false) && (cmd_from_script == true) )
            {
                // Create goal and add to goal list
                goal* curr = new goal( str_array, priority, predicate_func, success, failure );
                curr->next = goal_head;
                goal_head = curr;

                fout<<"Goal "<<str_array<<" defined"<<endl;
                return 1;
            }
            else if( defining_routine == true )
            {
                fout<<"Goals can not be defined inside of a routine."<<endl;
                return 1;
            }
            else
            {
                console.addSysMessage("You can not define goals from the command prompt.");
                return 0;
            }
        }
        if( strcmp(word, "group") == 0 )
        {
            if( sscanf(command, "%*s %*s %s", str_array) != 1 )
            {
                incorrectParams();
                return 0;
            }

            if( (defining_routine == false) && (cmd_from_script == true) )
            {
                // Do work here
                fout<<"defining goal"<<endl;
                return 1;
            }
            else if( defining_routine == true )
            {
                fout<<"Goals can not be defined inside of a routine."<<endl;
                return 1;
            }
            else
            {
                console.addSysMessage("You can not define goals from the command prompt.");
                return 0;
            }
        }
        
        
        if( strcmp(word, "routine") == 0 )
        {
            if( cmd_from_script == true )
            {
                char name[SCR_CMD_MAX];

                // Define routine name
                if( sscanf(command, "%*s %*s %s", name) != 1)
                {
                    fout<<"Incorrect parameters"<<endl;
                    return 0;
                }

                if( defining_routine == true )
                {
                    fout<<"Can not define a routine inside a routine"<<endl;
                    return 0;
                }
                    

                defining_routine = true;

                fout<<"Defining routine"<<endl;
                routine* routine_ptr = new routine(name);
                routine_ptr->next = routine_head;
                routine_head = routine_ptr;
                return 1;
            }
            else
            {
                console.addSysMessage("Can not define routine from the command promt");
                return 0;
            }
        }
    
            
        if( strcmp(word, "trigger") == 0 )
        {
            // Grab next word
            word = strtok(NULL, ignore_tokens);
            
            // Make sure it isn't NULL
            if( word == NULL )
            {
                incorrectParams();
                return 0;
            }

            // Create trigger pointer
            trigger* trigger_ptr;

            if( strcmp(word, "cellAreaTrigger") == 0 )
            {
                char id[SCR_CMD_MAX];
                int permanent;
                float xmin, xmax, zmin, zmax;
                char routine[SCR_CMD_MAX];

                // Define trigger cellAreaTrigger id permanent xmin xmax zmin zmax routine
                if( sscanf(command, "%*s %*s %*s %s %d %f %f %f %f %s", 
                    id, &permanent, &xmin, &xmax, &zmin, &zmax, routine) == 7 )
                {
                    // Do some checking
                    if( permanent != 0 && permanent != 1 )
                    {
                        fout<<"Permanent value was not a bool (0 or 1)"<<endl;
                        return 0;
                    }

                    // Create trigger
                    trigger_ptr = new cellAreaTrigger(id, script_side, permanent, xmin, xmax, zmin, zmax, routine);
                    
                    // Place trigger in list
                    addTrigger( trigger_ptr );


                    fout<<"defining trigger cell area trigger"<<endl;
                    return 1;
                }

                fout<<"cellAreaTrigger was not in the correct format: define trigger cellAreaTrigger s_id b_perm f_xmin f_xmax f_zmin f_zmax s_routine"<<endl;
                return 0;
            }
            if( strcmp(word, "cellRadiusTrigger") == 0 )
            {
                char id[SCR_CMD_MAX];
                int permanent;
                float x, y, radius;
                char routine[SCR_CMD_MAX];

                // Define trigger cellRadiusTrigger id permanent x y radius routine
                if( sscanf(command, "%*s %*s %*s %s %d %f %f %f %s", id, &permanent, &x, &y, &radius, routine ) == 6 )
                {
                    // Do some checking
                    if( permanent != 0 && permanent != 1 )
                    {
                        fout<<"Permanent value was not a bool (0 or 1)"<<endl;
                        return 0;
                    }

                    // Create trigger
                    trigger_ptr = new cellRadiusTrigger( id, script_side, permanent, x, y, radius, routine );

                    // Place trigger in list
                    addTrigger( trigger_ptr );
                    
                    fout<<"defining trigger cell radius trigger"<<endl;
                    return 1;
                }

                fout<<"cellRadiusTrigger was not in the correct format: define trigger cellRadiusTrigger str_id bool_perm f_x, f_y, f_radius routine"<<endl;
                return 0;
            }
            if( strcmp(word, "conditionalTrigger") == 0 )
            {
                char id[SCR_CMD_MAX];
                int permanent;
                char function[SCR_CMD_MAX];
                char routine[SCR_CMD_MAX];

                // Define trigger conditionalTrigger id permanent function routine
                if( sscanf(command, "%*s %*s %*s %s %d %s %s", 
                    id, &permanent, function, routine) == 4 )
                {
                    if( permanent != 0 && permanent != 1 )
                    {
                        fout<<"Permanent value was not a bool (0 or 1)"<<endl;
                        return 0;
                    }

                    // Create trigger
                    trigger_ptr = new conditionalTrigger( id, script_side, permanent, function, routine );

                    // Place trigger in list
                    addTrigger( trigger_ptr );
                    
                    fout<<"defining trigger conditional trigger"<<endl;
                    return 1;
                }

                fout<<"conditionalTrigger was not in the correct format: define trigger conditionalTrigger str_id bool_perm boolfunction routine"<<endl;
                return 0;
            }
            if( strcmp(word, "timeTrigger") == 0 )
            {
                char id[SCR_CMD_MAX];
                int permanent;
                char sign;
                float time;
                char routine[SCR_CMD_MAX];

                // Define trigger timeTrigger sign time id permanent routine
                if( sscanf(command, "%*s %*s %*s %c %f %s %d %s", &sign, &time, 
                    id, &permanent, routine) == 5 )
                {
                    // Check to permanent
                    if( permanent != 0 && permanent != 1 )
                    {
                        fout<<"Permanent value was not a bool (0 or 1)"<<endl;
                        return 0;
                    }

                    // Check sign
                    if( (sign != '<') && (sign != '>') )
                    {
                        fout<<"The sign must be either < or >."<<endl;
                        return 0;
                    }

                    // Create trigger
                    trigger_ptr = new timeTrigger( id, script_side, permanent, sign, time, routine );

                    // Place trigger in list
                    addTrigger( trigger_ptr );
                    
                    fout<<"defining trigger time trigger"<<endl;
                    return 1;
                }

                fout<<"timeTrigger was not in the correct format: define trigger timeTrigger s_sign f_time s_id b_perm s_routine"<<endl;
                return 0;
            }


            fout<<"Type of trigger not found"<<endl;
            return 0;
        }
        
        

        fout<<"Type of define not found"<<endl;
        return 0;
    }

    
    if( (strcmp(word,"endRoutine") == 0) )
    {
        if( (defining_routine == true) && (cmd_from_script == true) )
        {
            fout<<"end define"<<endl;
            if( defining_routine == false )
            {
                fout<<"No match for endDefine tag found"<<endl;
                return 0;
            }

            defining_routine = false;

            return 1;
        }
        else if( defining_routine == false )
        {
            fout<<"Can not end a routine if one hasn't been started"<<endl;
            return 0;
        }
        else
        {
            console.addSysMessage("Cann't endRoutine or even define a routine from the command prompt");
            return 1;
        }
    }


    // If we get here, no matching command was found
    if( cmd_from_script == true )
        fout<<"Unrecognized command:"<<command<<endl;
    else
    {
        console.addSysMessage("Unrecognized command. Type 'help' for command listing.");
        console.addSysMessage(" ");
    }
    return 0;
}

/*******************************************************************************
    function    :   script_module::incorrectParams
    arguments   :   <none>
    purpose     :   Outputs message tell user that a command was used with the
                    incorrect parameters.
    notes       :   <none>
*******************************************************************************/
void script_module::incorrectParams()
{
    if( defining_routine == true )
        fout<<"Incorrect parameter"<<endl;
    if( cmd_from_script == false )
        console.addSysMessage("Incorrect parameter");
}

/*******************************************************************************
    function    :   script_module::killTopRoutine
    arguments   :   <void>
    purpose     :   Kills the top routine. Usually used when routine is not
                    written correctly in a script file.
    notes       :   <none>
*******************************************************************************/
void script_module::killTopRoutine()
{
    if( routine_head != NULL )
    {
        routine* curr = routine_head;

        routine_head = routine_head->next;

        delete curr;
    }
}

/*******************************************************************************
    function    :   script_module::load
    arguments   :   <none>
    purpose     :   Calls load_script for each file being loaded and also does
                    some error checking on file opening.
    notes       :   <none>
*******************************************************************************/
void script_module::load()
{
    char buffer[128];

    // Setup predicate list
    script.createPredicateList();

    // Starting to read in commands from scripts
    cmd_from_script = true;

    // Load general Script
    strcpy(script_side, "ALL");
    general_load_success = true;//loadScript("Reference/general.script");

    // Load mission script
    strcpy(script_side, "ALL");
    strcpy(buffer, game_setup.mission_folder);
    strcat(buffer, "/mission.script");
    mission_load_success = loadScript(buffer);

    // Load help
    loadHelp();

    // Done reading from scripts
    // Set variables to ensure correct command promt processing
    strcpy(script_side, "ALL");
    cmd_from_script = false;
    defining_routine = false;

    fout<<"\nGame began at: "<<clock<<endl;
}

/*******************************************************************************
    function    :   script_module::loadHelp
    arguments   :   <none>
    purpose     :   Loads commands from a file called help.ref.
    notes       :   <none>
*******************************************************************************/
void script_module::loadHelp()
{
    char command[128];
    char* str_ptr;

    // Open file
    // Creates and open file input stream
    ifstream fin;
    fin.open( "Reference/help.ref", ios::in );

    // Makes sure file is opened properly
    if( fin.bad() )
    {
        fout<<"Input file name: "<<"Reference/help.ref"<<" not valid"<<endl;
        return;
    }

    // Reads each line of the file and sends it to the handler function for processing
    while( !fin.eof() )
    {
        // Make sure string contains a null character
        fin.getline( command, SCR_CMD_MAX );

        // Remove comments
        str_ptr = strstr( command, "#");
        if(str_ptr != NULL)
            str_ptr[0] = '\0';

        str_ptr = strstr( command, "//");
        if( str_ptr != NULL )
            str_ptr[0] = '\0';

        if( strlen(command) != 0 )
            addCommand( command );
    }

    // Close input stream
    fin.close();
    
    // Reverse list so display is in alphabetical order
    sc_command* curr = command_head;
    sc_command* newHead = NULL;
    
    while( curr != NULL )
    {
        command_head = command_head->next;
        curr->next = newHead;
        newHead = curr;
        curr = command_head;
    }
    
    command_head = newHead;
}

/*******************************************************************************
    function    :   script_module::loadScript
    arguments   :   fileName - Name of file being loaded
    purpose     :   Opens file and passes each line to the handler function.
    notes       :   <none>
*******************************************************************************/
bool script_module::loadScript( char* fileName)
{
    int error_check = 0;

    // Check to see if file name is NULL
    if( !fileName )
    {
        fout<<"File name equals null"<<endl;
        return 0;
    }

    fout<<"Attempting to load: "<<fileName<<endl;

    // Tracks what line we are reading from for informative error output
    int line = 0;      

    // Stores each line from script
    char cmd_str[SCR_CMD_MAX];

    // Creates and open file input stream
    ifstream fin;
    fin.open( fileName, ios::in );

    // Makes sure file is opened properly
    if( fin.fail() )
    {
        fout<<"Input file name: "<<fileName<<" not valid"<<endl;
        return 0;
    }

    // Reads each line of the file and sends it to the handler function for processing
    while( !fin.eof() )
    {
        // Make sure string contains a null character
        fin.getline( cmd_str, SCR_CMD_MAX - 1 );
        if( handler(cmd_str) == 0 )
            error_check--;
        
        line++;
    }

    // Close input stream
    fin.close();

    fout<<"Done reading: "<<fileName<<" script"<<endl<<endl;
    if( error_check < 0 )
        return 0;
    else
        return 1;
}

/*******************************************************************************
    function    :   script_module::update
    arguments   :   deltaT - Float value that represents the time between the 
                    last Call to this function and this Call.
    purpose     :   Runs through trigger and goal lists and checks to see if 
                    any of the triggers or goals have been activated.
    notes       :   <none>
*******************************************************************************/
void script_module::update( float deltaT )
{
    // Increment clock
    clock += deltaT;

    // Trigger pointers
    trigger* curr = trigger_head;
    trigger* prev = NULL;

    // Goal pointers
    goal* gCurr = goal_head;

    // Check each goal to see if it has successeded or failed
    while( gCurr != NULL )
    {
        if( gCurr->isComplete() != 0)
            gCurr->execRoutine();

        gCurr = gCurr->next;
    }

    // Check each trigger to see if it should be activated
    while( curr != NULL )
    {
        // If isActivated returns true then execute routine
        if( (curr->isActivated()) || (curr->dead == true) )
        {
            if( curr->dead == false )
                (curr->routine_ptr)->execRoutine();

            // If trigger not permanent then remove trigger from list
            if( curr->permanent == false )
            {
                // Node at head of list
                if( curr == trigger_head )
                {
                     curr = curr->next;
                     delete trigger_head;
                     trigger_head = curr;
                }

                // Node at end of list
                else if( curr->next == NULL )
                {
                    delete curr;
                    prev->next = NULL;
                    curr = prev;
                }

                // Node is in the middle of the list
                else
                {
                    prev->next = curr->next;
                    delete curr;
                    curr = prev->next;
                }
            }
        }
        else
        {
            prev = curr;
            curr = curr->next;
        }
    }
}
