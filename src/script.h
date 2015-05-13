/*******************************************************************************
                        Script Module - Definition
*******************************************************************************/
#ifndef SCRIPT_H
#define SCRIPT_H

#define SCR_CMD_MAX                     128     // Maximmum length of a command
#define SCR_ID_LEN                      32      // Maximmum length of ID
#define SCR_TYPE_LEN                    32
#define SCR_NAME_LEN                    32
#define SCR_MAX_COMMANDS_IN_ROUTINE     32
#define SCR_SECTION_MAX                 7

// Class prototype needed so we can declare a typedef for pointers to
// scripting module member functions
class script_module;
typedef bool(script_module::*pred_ptr_2_func)();

/*******************************************************************************
   struct      :   sc_command
   purpose     :   Stores a command for the help menu.
   notes       :   1) Then name and section of the command are stored to make
                      searchs easier.      
*******************************************************************************/
struct sc_command
{
    char name[16];                  // The name of the command
    char section[16];               // The section the command belongs to
    char output[128];               // The actual line that is displayed
    sc_command* next;               // Pointer to next command
    sc_command( char* command );    // Constructor
};

/*******************************************************************************
   class       :   sc_sound_effect_handler
   purpose     :   
   notes       :   1) 
*******************************************************************************/
class sc_sound_effect_handler
{
private:
    struct sound_effect_node
    {
        sound_effect_node( char* tName, int tId )
        {
            strcpy( name, tName );
            id = tId;
        }
        char name[128];
        int id;
        sound_effect_node* next;
    };

    sound_effect_node* sound_head;
    sound_effect_node* effect_head;
public:
    sc_sound_effect_handler();
    ~sc_sound_effect_handler();
    
    void displayEffects();
    void displaySounds();
    void effectDead( int id );
    int getEffectType( char* type );
    bool killEffect( int id );
    bool killSound( int id );
    void startEffect( char* command );
    void startSound( char* command );
};

/*******************************************************************************
   struct      :   predicate
   purpose     :   Stores a predicate function for use in conditional triggers.
   notes       :   <none>
*******************************************************************************/
struct predicate
{
    pred_ptr_2_func function_ptr;   // Function pointer
    char name[SCR_CMD_MAX];         // Name of predicate function
    predicate* next;                // Pointer to next predicate function
};

/*******************************************************************************
   class       :   routine
   purpose     :   The routine class is used as storage for function pointers.  
                   When a trigger isActivated it calls its routine class's
                   execRoutine function.  The execRoutine function will then
                   go through and call all the functions.
   notes       :   1) Routines are basically a list of commands.
                   2) Every goal and trigger has an associated but not 
                     necessarily unique routine.
                   3) Routines can ONLY be defined in a script and are NOT
                     removed once executed.
*******************************************************************************/
class routine
{
private:
    // Declared friend so we don't have to make tons of accessor functions in
    // the trigger class. This also saves in function overhead calling.
    friend class script_module;

protected:
    // Name of routine
    char name[SCR_NAME_LEN];

    // Number of function pointers in array
    int num_commands;

    // Function pointer list
    //void (*func_ptr[SCR_MAX_COMMANDS_IN_ROUTINE])(void);

    char command_list[SCR_MAX_COMMANDS_IN_ROUTINE][SCR_CMD_MAX];
    // Next routine in list
    routine* next;

public:
    routine( char *tName );
    ~routine(){};

    // Member functions
    bool addCommand( char* command );
    void execRoutine();
    char* getRoutineName(){return name;}
};

/*******************************************************************************
   class       :   trigger
   purpose     :   The trigger is the base class object that contains all the 
                   data and functions that are come to all derived trigger 
                   classes.
   notes       :   1) Triggers are what make the game interactive. They allow
                     the system to react to unit actions by calling a routine.
                   2) Triggers can ONLY be defined in a script.
                   3) Triggers can be used once than removed or can be 
                     permanent.
*******************************************************************************/
class trigger
{
private:
    // Declared friend so we don't have to make tons of accessor functions in
    // the trigger class. This also saves in function overhead calling.
    friend class script_module;

protected:
    bool dead;                          // Used when we want to remove trigger
    bool permanent;                     // If false, remove trigger once satisfied
    char id[SCR_ID_LEN];                // Unique trigger id
    char type[SCR_TYPE_LEN];            // Type of trigger=
    char side_effected[SCR_CMD_MAX];    // Side effected by trigger
    routine* routine_ptr;               // Pointer to associated routine
    trigger* next;                      // Pointer to next trigger
    // Other conditional statements

public:
    // Constructors
    trigger(){dead = false;}
    trigger(char* tId, char* tSideEffected, bool tPermanent, char* tRoutine){}

    // Deconstructor
    virtual ~trigger(){}

    // Virtual functions
    virtual bool isActivated(){return 0;}
    virtual void displayTrigger(){};

    // Member function
    void execRoutine();
};

/*******************************************************************************
   class       :   cellAreaTrigger
   purpose     :   This class inherits from the trigger class and is used for 
                   trigger that can be defined by a rectangular region.
   notes       :   1) A specific type of trigger used to test if a tank has
                     entered into a define x z based rectangle.
*******************************************************************************/
class cellAreaTrigger: public trigger
{
private:
    // Area of trigger
    float x_min;
    float x_max;
    float z_min;
    float z_max;

public:
    // Constructor
    cellAreaTrigger(char* tempid, char* sideeffected, bool perm, 
        float xmin, float xmax, float zmin, float zmax, char* temproutine);

    // Deconstructor
    ~cellAreaTrigger(){}

    // Member functions
    bool isActivated();
    void displayTrigger();
};

/*******************************************************************************
   class       :   cellRadiusTrigger
   purpose     :   This class inherits from the trigger class and is used for 
                   triggers that can be defined by a spherical region.
   notes       :   1) A specific type of trigger used to test if a tank has
                     entered into a define x z based circle.
*******************************************************************************/
class cellRadiusTrigger: public trigger
{
private:
    // Area of trigger
    float x_center;
    float z_center;
    float radius;

public:
    // Constructor
    cellRadiusTrigger( char* tId, char* tSide, bool tPermanent, float tXCenter, 
        float tZCenter, float tRadius, char* tRoutine );

    // Deconstructor
    ~cellRadiusTrigger(){}

    // Member fuctions
    bool isActivated();
    void displayTrigger();
};

/*******************************************************************************
   class       :   conditionalTrigger
   purpose     :   This class inherits from the trigger class and is used for 
                   things that can be defined such as: if(tankDestroyed tankID)
   notes       :   1) Calls a predicate function that determines if trigger is
                     Acticated or not.
*******************************************************************************/
class conditionalTrigger: public trigger
{
private:
    // Function pointer
    predicate* predicate_ptr;

public:
    // Constructor
    conditionalTrigger( char* tId, char* tSide, bool tPermanent, 
        char* tFunction, char* tRoutine );

    // Deconstructor
    ~conditionalTrigger(){}

    // Member functions
    bool isActivated();
    void displayTrigger();
};

/*******************************************************************************
   class       :   conditionalTrigger
   purpose     :   This class inherits from the trigger class and is used for 
                   things that can be defined such as: if(tankDestroyed tankID)
   notes       :   1) Calls a predicate function that determines if trigger is
                     Acticated or not.
*******************************************************************************/
class timeTrigger: public trigger
{
private:
    // Time of trigger
    float time;   

    // Determines if trigger should activate when the clock is > or < time
    char sign;      

public:
    // Constructor
    timeTrigger( char* tId, char* tSideEffected, bool tPermanent, 
                 char tSign, float tTime, char* tRoutine );

    // Deconstructor
    ~timeTrigger(){}

    // Member Functions
    bool isActivated();
    void displayTrigger();
};

/*******************************************************************************
   class       :   goal
   purpose     :   The goal class is basically here because we can not fully
                   repressent what we want to a goal to do with routines and
                   triggers.  The goal class is basically a node in a list of
                   goals and has attributes that represent what the goal is,
                   how we test if it's complete, and other varialbes and 
                   funtions.
   notes       :   1) A goal is different from a trigger and routine 
                     combination because the structure of the goal class
                     allows use to display goals on request, store goal
                     text, and keep information around once it has 
                     successeded or failed.
                   2) Allows for routines to be run upon success or failure of
                     objective.
*******************************************************************************/
class goal
{
private:
    // Declared friend so we don't have to make tons of accessor functions in
    // the trigger class. This also saves in function overhead calling.
    friend class script_module;

    char id[128];                   // Identifier
    char text[128];                 // Text to define how to complete goal
    int priority;                   // 1: Primary, 2: Seconday, 3: Bonus
    int status;                     // -1: failed, 0: not completed, 1: completed
    predicate* predicate_ptr;          // Pointer to a predicate function
    routine* success_routine;       // Pointer to routine to run if successeded
    routine* failure_routine;       // Pointer to routine to run if failed
    goal* next;                     // Pointer to next goal
    

public:
    // Constructor
    goal( char* tId, int tPriority, char* tPredicatePtr, char* tSuccessRoutine, 
           char* tFailureRoutine );

    // Deconstructor
    ~goal(){}

    // Member Functions
    void displayTrigger();
    void displayGoal();
    void execRoutine();
    int isComplete();
};

/*******************************************************************************
   class       :   script_module
   purpose     :   The Script module is responsible for reading in scripts, 
                   storing the commands, and parsing through them at run time.
                   This will represent the game AI and gameplay. The Script 
                   module also can take a command from the command prompt and
                   handler it in the same manner as if the command was read in
                   from a script.
   notes       :   <none>
*******************************************************************************/
class script_module
{
private:
    // Declared friend so we don't have to make tons of accessor functions in
    // the trigger class. This also saves in function overhead calling.
    friend class conditionalTrigger;
    friend class cellRadiusTrigger;
    friend class cellAreaTrigger;
    friend class timeTrigger;

    // Script loading success variables
    bool general_load_success;
    bool mission_load_success;

    // Used to indicate if between define routine and endRoutine tags
    // This is because defining routines takes several lines and processing
    // is different when between these tags
    bool defining_routine;

    // Indicates whether the command comes from a script or the command prompt
    bool cmd_from_script;

    // Indicates which side the trigger effects
    // ALL, AXIS, or ALLIES
    char script_side[16];

    // Game clock
    float clock;

    // Total game time, note: it is up to the script maker to 
    // define what happens when the clock passes this point
    // Not currently using
    float total_game_time;

    // Should be used when game is over and gives user say 
    // 10 seconds tell end of game
    // Not currently using
    float end_game_clock;

    // Command list and parameters listed for help system
    sc_command* command_head;
    char sections[SCR_SECTION_MAX][16];
    
    // Head pointer to trigger list
    trigger* trigger_head;

    // Head pointer to routine list
    routine* routine_head;

    // Head pointer to goal list
    goal* goal_head;

    // Head pointer to predicate struct list
    predicate* predicate_head; 
public:      
    script_module();                // Class constructor
    ~script_module();               // Class deconstructor
    
    // Used in string tokenizing. List of characters to skip
    char* ignore_tokens;

    // Sound and effects handler for scripts
    sc_sound_effect_handler sound_effect_handler;
    
    // Output file stream 
    ofstream fout;                    
    
    // Member functions
    void addCommand( char* command );
    void addPredicate( char* tName, pred_ptr_2_func tFuncPointer );
    void addTrigger( trigger* trigger_ptr );
    void loadSuccess();
    bool clockGreaterThanGameTime();
    void createPredicateList();
    void displayGoals();
    void displayRoutines();
    bool displaySection( char* section );
    void displayTriggers();
    bool findAndDisplayCommand( char* name );
    void fillSectionList();
    predicate* getPredicate( char* name );
    char* getPredicateName();
    routine* getRoutine( char* name );
    bool handler( char *str );
    inline void incorrectParams();
    void killTopRoutine();
    void load();
    void loadHelp();
    bool loadScript( char *str );
    void update( float deltaT );
};

extern script_module script;

#endif
