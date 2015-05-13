/*******************************************************************************
                          Loading Module - Definition
*******************************************************************************/

#ifndef LOAD_H
#define LOAD_H

/*******************************************************************************
    struct      :   loader_module
    purpose     :   The loader module is a module that contains some useful
                    functions that are used upon game load. This includes the
                    game loading screen, as well as the calling of different
                    module load routines upon program initialization.
    notes       :   OpenGL parameters for game run are also set up herein.
*******************************************************************************/
class loader_module
{
    private:
        bool loading;               // Boolean to determine if game is in load
        int load_value;             // Loading screen bar value
        int max_load_value;         // Max loading bar value
        
        char display_text[32];      // Text to display while loading
        
        GLubyte* load_screen;       // Load screen (base)
        int load_screen_width;
        int load_screen_height;
        
        GLubyte* back_drop;         // Back drop picture (drop)
        int back_drop_width;
        int back_drop_height;
        
        GLubyte* load_bar;          // Loading bar (bar)
        int load_bar_width;
        int load_bar_height;
        
    public:    
        loader_module();            // Constructor
        
        /* Load Routines */
        void loadGameSettings(int argc, char *argv[]);  // Load settings.ini
        void loadGame();                                // Load game (init)
        
        /* Accessors */
        void advanceLoadBar(int value)
            { load_value += value; }
        void setDisplayText(char* text)
            { strncpy(display_text, text, 31); }
        
        /* Base Display Routine */
        void display();
};

extern loader_module loader;

#endif
