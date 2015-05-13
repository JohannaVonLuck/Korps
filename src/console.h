/*******************************************************************************
                          Command Console Definition
*******************************************************************************/

#ifndef CONSOLE_H
#define CONSOLE_H

// Max array lengths
#define CON_MAX_LINES           64
#define CON_MAX_COM_LINES       8

// Spots on the screen (y val %) for console pulldown edge and com lines start
#define CON_PULLDOWN_SPOT       0.5
#define CON_COM_START_SPOT      0.55

// String length for command[]
#define CON_CMD_MAX             80

// Special console commands
#define CON_CMD_PREVIOUS        0
#define CON_CMD_NEXT            1
#define CON_CMD_ABORT           2

/*******************************************************************************
    class       :   console_module
    purpose     :   The console is an integral piece of code which allows the
                    player the ability to enter in commands to the game while
                    playing, as well as offering easy text display to the screen
                    for any purpose. The console can be accessed by pressing the
                    tilde key (~) usually. The console commmand entering capab-
                    ilities allows on-the-fly script code to be ran, which also
                    allows modders/designers/etc. some very important debugging
                    tools since the normal stdout can't be seen/accessed easily.
                    The console also is said to control any and all text which
                    is to be displayed to the player during game execution, and
                    as such as a "sub-console" communication line system based
                    on the exact same principles as the rest.
    notes       :   1) The console is said to be in the "pulled down" state
                       when the console is in view. This boolean can be accessed
                       via the isPulledDown() function,
                    2) Console command entering mode is enabled via a call to
                       cmdEnter() with a char_code of either 0x0a or 0x0d. This
                       is said to "open" and "close" a command line. Upon the
                       "closing" of a command line, the command is sent to the
                       script handlers for execution. This boolean can be
                       accessed via teh isEnteringCmd() function.
                    3) The three main ways of adding text to the console are
                       as follows: 1) addMessage - adds message to standard
                       console display at top of screen and on console pulldown,
                       2) addComMessage - adds message to a sub-console for
                       display at a larger font and 2x time interval, usually
                       for player-specific messages (e.g. communications), and
                       3) addSysMessage - adds message to console display only
                       when the console is pulled down (e.g. system related
                       messaging not important to current state of player).
                       Note that addComMessage makes a corresponding call to
                       addSysMessage so that all communication lines are stored
                       on the console as well for easy retrieval.
                    4) Special commands are built-in to mimic the Linux command
                       line, specifically with features like the nifty command
                       traceback and command abort. An auto-complete was con-
                       sidered, but is a bit heafty for our time constraints.
                    5) The console has a smooth pulldown and pullup animation
                       of which isPulledDown() does not really tell the truth
                       about. If the console is anywhere on the screen, then
                       curr_down will not be 0.0. When the console is told to
                       go back up, isPulledDown() will return false even though
                       the console is in the process of being animated upwards.
                       As such, the accessor function isDisplayed() will return
                       the boolean of the comparison with curr_down and 0.0 
                       which, as such, tells if the console is not fully pulled
                       back up and not being displayed onto the screen.
*******************************************************************************/
class console_module
{
    private:
        /***********************************************************************
            struct  :   node
            purpose :   Storage for each entry for each console line.
            notes   :   <none>
        ***********************************************************************/
        struct node
        {
            char* text;             // Text pointer (uses strdup())
            GLubyte* image;         // Pointer to image of generated text
            int width;              // Width of generated image text
            int height;             // Height of generated image text
            float expire_time;      // Expire time timer for lines
            bool pulldown_only;     // System message (not used in com)
        };
        
        node lines[CON_MAX_LINES];          // Main console lines
        
        node com_lines[CON_MAX_COM_LINES];  // Sub-console communication lines
        
        float line_interval;        // Interval for lines to display (in sec.)
        int line_count;             // # of lines with non-expired timers
        int com_line_count;         // same as line_count but for com_lines[]
        int head_ptr;               // position in lines[] for next message
        int com_head_ptr;           // same as head_ptr but for com_lines[]
        
        bool pulldown;              // Console pulldown state
        float curr_down;            // Current bottom position of console win.
        float dest_down;            // Dest. bottom position of console win.
        
        bool cmd_entering;          // Console command entering mode
        char command[CON_CMD_MAX];  // Current command line (filled /w cmdEnter)
        int cmd_strlen;             // Current string length of command[]
        int cmd_backpos;            // Previous command look-up (backtrace) var.
        
        void cmd_handler();         // Command handler (calls script handler)
        
    public:
        console_module();                   // Constructor
        ~console_module();                  // Deconstructor
        
        /* Message Adding Routines */
        void addMessage(char* text);        // Basic add
        void addComMessage(char* text);     // Com. system add
        void addSysMessage(char* text);     // Console pulldown only add
        
        /* Console Pulldown/Pullup Routine */
        void togglePullDown();              // Toggles console pulldown
        
        /* Console Command Handler Routines */
        void cmdEnter(char char_code);      // Add character to command[]
        void cmdSpecial(int operation);     // Perform special operation
        
        /* Accessors */
        bool isPulledDown()                 // Pulldown state
            { return pulldown; }
        bool isDisplayed()                  // Console display state
            { return (curr_down != 0.0); }
        bool isEnteringCmd()                // Command entering state
            { return cmd_entering; }
        
        /* Mutators */
        void setInterval(float value)       // Line expire time interval
            { line_interval = value; }
        
        /* Base Display & Update Routines */
        void display();
        void update(float deltaT);
};

extern console_module console;

#endif
