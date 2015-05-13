/*******************************************************************************
                          Camera Module - Definition
*******************************************************************************/

#ifndef CAMERA_H
#define CAMERA_H

#include "metrics.h"

// Camera Orientations (for movement, rotation, etc.)
#define CAM_MOVE_FORWARD        0
#define CAM_MOVE_BACKWARD       1
#define CAM_MOVE_RIGHT          2
#define CAM_MOVE_LEFT           3
#define CAM_ROT_RIGHT           4
#define CAM_ROT_LEFT            5

#define CAM_MAX_DIR             6

#define CAM_MAX_SPOT_STACK      2

/*******************************************************************************
    class       :   camera_module
    purpose     :   The entire game as viewed is based entirely on the way the
                    camera is set up. This camera module is an extensive class
                    centered on providing a complex camera that is used to view
                    the scene. The main job of the camera module is to provide
                    a method of viewing the scene, in which the base routine is
                    the orient routine. Along with viewing the scene, the camera
                    is able to move around the map based on which orientation
                    is enabled (controlled obviously by event handlers). The
                    camera is meant to assist the commander in his duties. As
                    well, the camera also provides a method for clipping off-
                    screen objects out of the GPU pipeline via frustum culling.
    notes       :   1) The base method of movement is done via enabling and
                       disabling orientations, of which include moving forward,
                       backward, left, right, rotate left, and rotate right.
                       Upon enabling, the camera will "smoothly" transition its
                       speed up to the cam_speed values. This "smoothing" effect
                       is done on camera rotating, elevating, but not zooming.
                    2) The camera essentially "floats" above the ground. The
                       elevation up is depended on a step value, of which an
                       elevation value and camera pitch value are computed. As
                       the camera moves higher up, the camera pitch faces more
                       downward. This gives a nice top-down view as the step
                       value increases.
                    3) The camera is also equiped with a zooming function, of
                       which magnifies (based on normal 50mm focal length optic)
                       the scene. The zoom step is not the magnification level,
                       see the zoom() function for details.
                    4) The "smooth" transitioning effect can be overrided with
                       the forceValues() function, which effectively jumps the
                       camera to the new values.
                    5) Current camera position may be saved via a stack using
                       the pushCamSpot() and popCamSpot() functions. Note that
                       there is a stack limit, which is defined above.
                    5) Aside from player-only control, the camera is equipped
                       with the ability to disable player camera control temp-
                       orarily so that other *things* may occur - such as cpu
                       camera control scripts, etc.
                    6) It is incredibly handy to know the direction vector
                       formed at some screen coordinate on the viewing screen,
                       and as such the vectorAt() function will produce such
                       a vector. Note that this is the just the direction
                       vector. The position vector is obviously the camera
                       position (using getCamPosK() or getCamPos()).
                    7) The function getCamPos() returns a pointer to the EXACT
                       array being used for camera data - not a copy of the
                       data. You can use this with static var pointers and only
                       have to call getCamPos() once.
                    8) Mutator functions, such as position vector and direction
                       vector setting, is done without any "smooth" transition
                       effects. The end result is an immediate jump.
*******************************************************************************/
class camera_module
{
    private:
        kVector dir;            // Direction Vector
        kVector pos;            // Position Vector
        
        float aspect_ratio;     // Aspect Ratio of screen
        
        // Camera Movement/Rotation speed control functionality. Note: There
        // are two different arrays for cam_speed and enabled, which is due
        // to the nature of the player control function (array 0 -> player
        // control enabled, array 1 -> player control disabled).
        float cam_speed[CAM_MAX_DIR][2];    // Max speed for each direction
        float curr_speed[CAM_MAX_DIR];      // Current speed for each direction
        bool enabled[CAM_MAX_DIR][2];       // Direction/Rotation enable
        
        // Camera Height/Pitch control functionality
        int curr_elevate_step;  // Stepper value for camera elevate control
        float camera_elevate;   // Dest. height up from heightmap
        float camera_pitch;     // Dest. pitch value of direction vector
        
        // Camera Zoom control functionality
        int curr_zoom_step;     // Stepper value for camera zoom control
        float camera_fov;       // Field of View for camera
        
        bool plyr_ctrl;         // Player camera control enable (for scripts)
        
        float frustum[6][4];    // Frustrum storage for culling
        
        kVector pos_stack[CAM_MAX_SPOT_STACK];  // Position Stack
        kVector dir_stack[CAM_MAX_SPOT_STACK];  // Direction Stack
        int elevate_stack[CAM_MAX_SPOT_STACK];  // Elevation Step Stack
        int zoom_stack[CAM_MAX_SPOT_STACK];     // Zoom Step Stack
        int stack_num;          // Current stack index
        
    public:
        camera_module();        // Constructor
        ~camera_module();       // Deconstructor
        void reset() { camera_module(); }
        
        /* Camera Control Routines */
        
        // Base movement/rotation enable/disable
        void enable(int orientation)
            { enabled[orientation][(int)plyr_ctrl] = true; }
        void disable(int orientation)
            { enabled[orientation][(int)plyr_ctrl] = false; }
          
        // Camera elevating routines
        void elevate(int step);              // Relative elevate step
        inline void elevateTo(int step)      // Absolute elevate step
            { elevate(step - curr_elevate_step); }
        
        // Camera zooming routines
        void zoom(int step);                 // Relative zoom step
        inline void zoomTo(int step)         // Absolute zoom step
            { zoom(step - curr_zoom_step); }
            
        // "Smooth" transitioning override (useful for scripts)
        void forceValues();
        
        // Push & Pop camera pos/dir/elev/zoom (useful for scripts)
        void pushCamSpot();
        void popCamSpot();
        
        // Player control enable/disable (useful for scripts)
        void playerControl(bool enable);
        bool playerControlActive() { return plyr_ctrl; }
        
        /* Frustum Culling Routines */
        bool pointInView(float pos[3]);
        bool sphereInView(float pos[3], float radius);
        
        /* Camera Metrics */
        kVector vectorAt(int x, int y);
            
        /* Accessors */
        float* getCamPos()          // Camera position
            { return pos(); }
        kVector getCamPosV()        // Camera position vector (kVector)
            { return pos; }
        float* getCamDir()          // Camera direction
            { return dir(); }
        kVector getCamDirV()        // Camera direction vector (kVector)
            { return dir; }
        
        float getAspectRatio()
            { return aspect_ratio; }
        float getCamFOV()
            { return camera_fov; }
        
        /* Mutators */
        void setAspectRatio(float value)    // Aspect ratio
            { aspect_ratio = value; }
            
        void setCamSpeed(float speed, int orientation)  // Camera movement speed
            { cam_speed[orientation][(int)plyr_ctrl] = speed; }

        void setCamPosV(kVector position)   // Camera position vector (kVector)
            { pos = position; }
        void setCamPos(float position[3])   // Camera position
            { pos = kVector(position); }
        void setCamPos(float pos_x, float pos_y, float pos_z)
            { pos = kVector(pos_x, pos_y, pos_z); }

        void setCamDirV(kVector direction)  // Camera direction vector (kVector)
            { dir = direction; }
        void setCamDir(float direction[3])  // Camera direction
            { dir = kVector(direction); }
        void setCamDir(float dir_m, float dir_p, float dir_y)
            { dir = kVector(dir_m, dir_p, dir_y); }
        
        /* Base Orient & Update Routines */
        void orient(bool updateFrustum);    // Camera orientation
        inline void orient()
            { orient(true); }
        void update(float deltaT);          // Camera update
};

extern camera_module camera;

#endif
