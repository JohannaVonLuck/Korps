/*******************************************************************************
                            UI Module - Definition
*******************************************************************************/
#ifndef UI_H
#define UI_H
#include "main.h"
#include "object.h"
#include "objlist.h"

/*******************************************************************************
   class       :   ui_module
   purpose     :
   notes       :   1)
*******************************************************************************/
class ui_module
{
	private:
        object_list selection;
        
        // Variables to handle drawing of screen
        GLubyte* usr_interface_base;
        GLubyte* usr_interface;
        int usr_interface_width;
        int usr_interface_height;
        int usr_interface_size;
        float usr_interface_trans;
        bool usr_interface_updated;
        
        // Variables to handle mouse dragging
        bool mouse_down;
        bool mouse_dragging;
        int mouse_coord_start_x;
        int mouse_coord_start_y;
        int mouse_coord_end_x;
        int mouse_coord_end_y;
        
        // Other things for the usr_interface
        GLubyte* picture_bg;
        int picture_bg_width;
        int picture_bg_height;
        
	public:
		ui_module();                  // Constructor
		~ui_module();                 // Deconstructor
        
		/* Initialization Routines */
		void loadUI();
        
		/* Base Routines */
        // Event Handlers
        void keystrokeDown(SDL_KeyboardEvent &key);
        void keystrokeUp(SDL_KeyboardEvent &key);
        void mouseMove(SDL_MouseMotionEvent &motion);       
        void mouseButtonDown(SDL_MouseButtonEvent &button);
        void mouseButtonUp(SDL_MouseButtonEvent &button);
        
        // Mutators
        void updateInterfacingObject() { usr_interface_updated = false; }
        void emptySelection() { selection.setSelected(false); selection.empty(); }
        
        // Accessors
        bool isSelectionEmpty() { return selection.isEmpty(); }
        object_list* getSelection() { return &selection; }
        
        object* getInterfacingObject() { return (selection.isEmpty() ? NULL : selection.getHeadPtr()->obj_ptr); }
        
        /* Base Update & Display Routines */
        void display();
        void update(float deltaT);
};

extern ui_module ui;

#endif
