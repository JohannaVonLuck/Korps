/*******************************************************************************
                      Camera Module - Implementation
*******************************************************************************/
#include "main.h"
#include "camera.h"
#include "console.h"
#include "misc.h"
#include "scenery.h"
#include "sounds.h"

/*******************************************************************************
    function    :   camera_module::camera_module
    arguments   :   <none>
    purpose     :   Constructor
    notes       :   <none>
*******************************************************************************/
camera_module::camera_module()
{
    int i;
    
    // Basic screen ratio (will always be 1.333333 upon startup)
    aspect_ratio = 1024.0 / 768.0;
    
    // Set standard speed settings
    cam_speed[CAM_MOVE_FORWARD][0] = cam_speed[CAM_MOVE_FORWARD][1] = 88.0;
    cam_speed[CAM_MOVE_BACKWARD][0] = cam_speed[CAM_MOVE_BACKWARD][1] = -88.0;
    cam_speed[CAM_MOVE_RIGHT][0] = cam_speed[CAM_MOVE_RIGHT][1] = 88.0;
    cam_speed[CAM_MOVE_LEFT][0] = cam_speed[CAM_MOVE_LEFT][1] = -88.0;
    cam_speed[CAM_ROT_RIGHT][0] = cam_speed[CAM_ROT_RIGHT][1] = PIHALF;
    cam_speed[CAM_ROT_LEFT][0] = cam_speed[CAM_ROT_LEFT][1] = -PIHALF;    
    
    // Clear curr_speed and enabled
    for(i = 0; i < CAM_MAX_DIR; i++)
    {
        curr_speed[i] = 0.0;
        enabled[i][0] = false;
        enabled[i][1] = false;
    }
    
    // Set standard pos and dir vectors
    pos = kVector(0.0, 100.0, 0.0);
    dir = kVector(1.0, PIHALF + (PIHALF / 4.0), PIHALF / 2.0);
    
    // Set standard elevation params
    curr_elevate_step = 4;
    camera_pitch = 2.094395;
    camera_elevate = 52.415077;
    
    // Set standard zoom params
    curr_zoom_step = 0;
    camera_fov = 46.79;
    
    // Set standard player ctrl params
    plyr_ctrl = true;
    
    // Set to first stack
    stack_num = 0;
}

/*******************************************************************************
    function    :   camera_module::~camera_module
    arguments   :   <none>
    purpose     :   Deconstructor
    notes       :   <none>
*******************************************************************************/
camera_module::~camera_module()
{
    // Do nothing for now...
    return;
}

/*******************************************************************************
    Camera Control Routines
*******************************************************************************/

/*******************************************************************************
    function    :   camera_module::elevate
    arguments   :   step - Relative value to add onto current elevate step
    purpose     :   The camera essentially "hovers" above the ground. The
                    elevation at which the camera will hover is based upon
                    an elevation step value. This function essentially will
                    calculate that elevation to hover at, and also at what
                    pitch angle to have the camera look at.
    notes       :   <none>
*******************************************************************************/
void camera_module::elevate(int step)
{
    // Effect current step with new step
    curr_elevate_step += step;
    
    // Check for bounds
    if(curr_elevate_step < 1)
        curr_elevate_step = 1;
    else if(curr_elevate_step > 10)
        curr_elevate_step = 10;
    
    // Recalculate the new elevation and pitch values
    camera_elevate = ((4.0 + (0.5 * (curr_elevate_step - 1))) * (curr_elevate_step - 1)) + 2.0;
    
    camera_pitch = PIHALF + ((float)(curr_elevate_step - 1) / 9.0) * 1.2;
}

/*******************************************************************************
    function    :   camera_module::zoom
    arguments   :   step - Relative value to add onto current zoom step
    purpose     :   The camera also will 
    notes       :   Zoom steps do not follow a step.0x pattern, instead it
                    follows the following pattern (step - mag):
                    0 - 0.7x, 1 - 1.0x, 2 - 2.0x, 3 - 4.0x, 4 - 8.0x, 5 - 12.0x
*******************************************************************************/
void camera_module::zoom(int step)
{
    float zoom_level[] = {1.0, 2.5, 5.0, 8.0, 10.0, 12.0};   // Zoom step mag
    char buffer[128];
    
    // Effect curret zoom step with new step
    curr_zoom_step += step;
    
    // Check for bounds
    if(curr_zoom_step < 0)
        curr_zoom_step = 0;
    else if(curr_zoom_step > 5)
        curr_zoom_step = 5;
    
    // Recalculate new field of view value for camera
    camera_fov =
        2.0 * atan(21.63175 / (45.0 * zoom_level[curr_zoom_step])) * radToDeg;
    
    if(plyr_ctrl)
    {
        // Add message to console screen saying that we have changed zoom
        sprintf(buffer, "Zoom Level: %.1fx", zoom_level[curr_zoom_step]);
        console.addMessage(buffer);
    }
}

/*******************************************************************************
    function    :   camera_module::forceValues
    arguments   :   <none>
    purpose     :   Forces all camera speed settings, elevate settings, and
                    pitch settings to their corresponding settings, effectively
                    overriding the "smooth" transitioning effect.
    notes       :   <none>
*******************************************************************************/
void camera_module::forceValues()
{
    int i;
    
    // Force curr_speed values to cam_speed values
    for(i = 0; i < CAM_MAX_DIR; i++)
    {
        if(enabled[i][(int)plyr_ctrl])
            curr_speed[i] = cam_speed[i][(int)plyr_ctrl];
    }
    
    // Force camera elevation
    pos[1] = map.getOverlayHeight(pos[0], pos[2]) + camera_elevate;
    
    // Force camera pitch
    dir[1] = camera_pitch;
}

/*******************************************************************************
    function    :   camera_module::pushCamSpot
    arguments   :   <none>
    purpose     :   Uses a stack to save the camera position, direction, ele-
                    vation, and zoom values.
    notes       :   <none>
*******************************************************************************/
void camera_module::pushCamSpot()
{
    // Check for bounds
    if(stack_num >= CAM_MAX_SPOT_STACK)
        return;
    
    // Save values
    pos_stack[stack_num] =
        kVector(pos[0],
                map.getOverlayHeight(pos[0], pos[2]) + camera_elevate,
                pos[2]);
    dir_stack[stack_num] = kVector(dir[0], camera_pitch, dir[2]);
    elevate_stack[stack_num] = curr_elevate_step;
    zoom_stack[stack_num] = curr_zoom_step;
    
    // Increment stack count
    stack_num++;
}

/*******************************************************************************
    function    :   camera_module::popCamSpot
    arguments   :   <none>
    purpose     :   Uses a stack to restore the camera position, direction,
                    elevation, and zoom values previously pushed.
    notes       :   Once a spot is popped off the stack, it will be
                    unrecoverable unless the stack_num (private) is changed.
*******************************************************************************/
void camera_module::popCamSpot()
{
    // Check for bounds
    if(stack_num <= 0)
        return;
    
    // Decrement stack count
    stack_num--;

    // Restore values
    pos = pos_stack[stack_num];
    dir = dir_stack[stack_num];
    elevateTo(elevate_stack[stack_num]);
    zoomTo(zoom_stack[stack_num]);
    
    // Reset current speed settings to 0.0 so that the camera doesn't violently
    // jump as the camera spot is restored.
    for(int i = 0; i < CAM_MAX_DIR; i++)
        curr_speed[i] = 0.0;
}

/*******************************************************************************
    function    :   camera_module::playerControl
    arguments   :   enable - setting for player control
    purpose     :   For purposes of scripting and otherwise, the game may want
                    to take away control of the camera from the player. This
                    function provides a bit of access helpful in allowing
                    the game to take control of the camera and then switch back
                    to the player's view.
    notes       :   1) Upon disabling the player control of the camera, the
                       current position and camera orientation is essentially
                       pushed onto a stack for safe keeping. Once player control
                       is returned, they are poped off the stack. Notice that
                       this does use the integrated pushCamSpot and popCamSpot
                       functions, of which are public functions. The pushing
                       and poping are done automatically. If any outside
                       code essentially pops the stack, there is no gaurantee
                       that the camera position in use by the player will be
                       restored to where it was before taking away control.
                    2) It is the responsiblity of the coder to make sure that
                       no calls from the user-input event handlers effect the
                       new camera's orientation. This is done by a call to
                       playerControlActive(), which returns true when the player
                       has control of the camera and false when not. Use this
                       to skip over calls to camera control funcs. (such as
                       enable, disable, elevate, zoom, etc.) in user-input
                       event handler functions.
*******************************************************************************/
void camera_module::playerControl(bool enable)
{
    // For sake of setting up the values to provide a fresh start for the
    // new control type, all curr_speed values and orientation enable values
    // are all reset to 0.0/false.
    for(int i = 0; i < CAM_MAX_DIR; i++)
    {
        curr_speed[i] = 0.0;
        
        enabled[i][(int)plyr_ctrl] = false;
    }
    
    // Save/Restore camera position when enabling/disabling player control
    if(plyr_ctrl != enable)
    {
        if(enable)
            popCamSpot();
        else
            pushCamSpot();
    }
        
    // Set player control value
    plyr_ctrl = enable;
}

/*******************************************************************************
    Frustum Culling Routines
*******************************************************************************/

/*******************************************************************************
    function    :   bool camera_module::pointInView
    arguments   :   float pos[3] - position vector
    purpose     :   Determines if the passed point is inside the view frustum.
    notes       :   <none>
*******************************************************************************/
bool camera_module::pointInView(float pos[3])
{
    static int i;
    
    // Determine if the point is outside of the viewing frustum by checking it
    // against all 6 sides.
    for(i = 0; i < 6; i++)
        if(frustum[i][0] * pos[0]
            + frustum[i][1] * pos[1]
            + frustum[i][2] * pos[2]
            + frustum[i][3] <= 0)
            return false;
    
    // Otherwise the test passes and it's inside of the frustum.
    return true;
}

/*******************************************************************************
    function    :   bool camera_module::sphereInView
    arguments   :   float pos[3] - position vector to center of sphere
                    radius - radius of sphere
    purpose     :   Determines if any portion of the passed sphere is inside
                    the view frustum.
    notes       :   <none>
*******************************************************************************/
bool camera_module::sphereInView(float pos[3], float radius)
{
    static int i;
    
    // Determine if the sphere (including it's radius) is outside of the
    // viewing frustum by checking it against all 6 sides.
    for(i = 0; i < 6; i++) 
        if(frustum[i][0] * pos[0]
            + frustum[i][1] * pos[1]
            + frustum[i][2] * pos[2]
            + frustum[i][3] <= -radius)
            return false;
    
    // Otherwise the test passes and it's inside of the frustum.
    return true;
}

/*******************************************************************************
    Camera Metrics Routines
*******************************************************************************/

/*******************************************************************************
    function    :   kVector camera_module::vectorAt
    arguments   :   x,y - Screen position coordinates.
    purpose     :   Returns the direction vector formed from clicking on the
                    screen at some position. Useful for many different apps.
    notes       :   Uses the linear algebra concept of linear transformation
                    to go from one coordinate system to another.
*******************************************************************************/
kVector camera_module::vectorAt(int x, int y)
{
    float x_off, y_off;         // Offset on screen in normalized coords.
    float clip_height;          // Clipping window height
    float clip_width;           // Clipping window width
    
    GLfloat modelview[16];      // Modelview matrix storage
    GLfloat clip_point[4];      // Clipping point
    GLfloat world_point[4];     // World  point
    
    // Determine screen offsets in normalized coords.
    y_off = (((float)y / (game_setup.screen_height - 1)) - 0.5) * 2.0;
    x_off = (((float)x / (game_setup.screen_width - 1)) - 0.5) * 2.0;
    
    // Determine clipping width and clipping height
    clip_height = tan((camera_fov * degToRad) / 2.0);
    clip_width = aspect_ratio * clip_height;
    
    // Define our clipping point (this will be passed through our matrix). This
    // point set is VERY important - it defines the point to have the linear
    // transformation done upon. Note that since a pitch and yaw values of 0.0
    // correspond to a vector pointing straight up, this clipping window is
    // oriented straight up.
    clip_point[0] = clip_width * -x_off;
    clip_point[1] = 1.0;
    clip_point[2] = clip_height * y_off;
    clip_point[3] = 1.0;
    
    // Build our matrix for the linear transformation
    glMatrixMode(GL_MODELVIEW);     // Do this in our modelview matrix
    
    glPushMatrix();     // Save current matrix
    
    glLoadIdentity();
    
    glTranslatef(pos[0], pos[1], pos[2]);           // Position translate
    glRotatef(dir[2] * radToDeg, 0.0, 1.0, 0.0);    // Yaw rotate
    glRotatef(dir[1] * radToDeg, 1.0, 0.0, 0.0);    // Pitch rotate
    
    // Grab modelview matrix
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    
    glPopMatrix();      // Restore matrix
    
    // Matrix Multiply - 4x4 matMult 1x4
    for (int i = 0; i < 4; i++)
        world_point[i] = modelview[0 + i] * clip_point[0] +
                         modelview[4 + i] * clip_point[1] +
                         modelview[8 + i] * clip_point[2] +
                         modelview[12 + i] * clip_point[3];
    
    // Return difference between the world coordinate point and the camera
    // point to get the direction vector.
    return kVector((world_point[0] - pos[0]),
                   (world_point[1] - pos[1]),
                   (world_point[2] - pos[2]));
}

/*******************************************************************************
    Base Camera Routines
*******************************************************************************/

/*******************************************************************************
    function    :   camera_module::orient
    arguments   :   updateFrustum - determines whenever the frustum culling
                        clipping planes should be updated or not.
    purpose     :   The base function which is called to orient a scene for
                    viewing by the camera.
    notes       :   1) Effects the projection matrix and modelview matrix,
                       therefore glLoadIdentity should not be called again as
                       it will un-orient the scene (so to speak).
                    2) For sake of not having any chance of corrupted matrix
                       data, the frustum culling planes are updated herein.
                       This can, at some times, cause redundant calculations,
                       but the likelihood of at least one module being updated
                       in some fashion after display is fairly high, so this
                       drawback is acceptable.
*******************************************************************************/
void camera_module::orient(bool updateFrustum)
{
    int i;
    float magnitude;        // Magnitude of plane normal (for normalizing)
    float projection[16];   // Project matrix storage
    float modelview[16];    // Modelview matrix storage
    float clip[16];         // Clipping matrix storage
    
    // Set perspective projection / viewing frustum
    glMatrixMode(GL_PROJECTION);
    gluPerspective(camera_fov, aspect_ratio, 2.0, 5000.0);
    
    // Set camera position and direction vectors
    glMatrixMode(GL_MODELVIEW);
    gluLookAt(
            // Eye coordinates (camera position)
            pos[0], pos[1], pos[2],
            // Look-at vector (speherical->cartesian transform)
            pos[0] + sin(dir[1])*sin(dir[2]),
            pos[1] + cos(dir[1]),
            pos[2] + sin(dir[1])*cos(dir[2]),
            // Up vector (0,1,0 - since we have no roll)
            0.0,
            1.0,
            0.0);
    
    if(updateFrustum)
    {
        // Grab corresponding project and modelview matrixes and store them.
        glGetFloatv(GL_PROJECTION_MATRIX, projection);
        glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
        
        // Determine our clipping matrix by taking the modelview matrix and
        // performing a matrix multiplication with the projection matrix, thus
        // forming the clipping matrix. Note that this is a fairly lenghty op.
        clip[0] = modelview[0] * projection[0] + modelview[1] * projection[4]
            + modelview[2] * projection[8] + modelview[3] * projection[12];
        clip[1] = modelview[0] * projection[1] + modelview[1] * projection[5]
            + modelview[2] * projection[9] + modelview[3] * projection[13];
        clip[2] = modelview[0] * projection[2] + modelview[1] * projection[6]
            + modelview[2] * projection[10] + modelview[3] * projection[14];
        clip[3] = modelview[0] * projection[3] + modelview[1] * projection[7]
            + modelview[2] * projection[11] + modelview[3] * projection[15];
        
        clip[4] = modelview[4] * projection[0] + modelview[5] * projection[4]
            + modelview[6] * projection[8] + modelview[7] * projection[12];
        clip[5] = modelview[4] * projection[1] + modelview[5] * projection[5]
            + modelview[6] * projection[9] + modelview[7] * projection[13];
        clip[6] = modelview[4] * projection[2] + modelview[5] * projection[6]
            + modelview[6] * projection[10] + modelview[7] * projection[14];
        clip[7] = modelview[4] * projection[3] + modelview[5] * projection[7]
            + modelview[6] * projection[11] + modelview[7] * projection[15];
        
        clip[8] = modelview[8] * projection[0] + modelview[9] * projection[4]
            + modelview[10] * projection[8] + modelview[11] * projection[12];
        clip[9] = modelview[8] * projection[1] + modelview[9] * projection[5]
            + modelview[10] * projection[9] + modelview[11] * projection[13];
        clip[10] = modelview[8] * projection[2] + modelview[9] * projection[6]
            + modelview[10] * projection[10] + modelview[11] * projection[14];
        clip[11] = modelview[8] * projection[3] + modelview[9] * projection[7]
            + modelview[10] * projection[11] + modelview[11] * projection[15];
        
        clip[12] = modelview[12] * projection[0] + modelview[13] * projection[4]
            + modelview[14] * projection[8] + modelview[15] * projection[12];
        clip[13] = modelview[12] * projection[1] + modelview[13] * projection[5]
            + modelview[14] * projection[9] + modelview[15] * projection[13];
        clip[14] = modelview[12] * projection[2] + modelview[13] * projection[6]
            + modelview[14] * projection[10] + modelview[15] * projection[14];
        clip[15] = modelview[12] * projection[3] + modelview[13] * projection[7]
            + modelview[14] * projection[11] + modelview[15] * projection[15];
        
        // Take our clipping matrix and now start to define the corresponding
        // frustum sides. This again is a fairly lenghty op. Each portion of the
        // frustum actually defines the plane equation, Ax+By+Cz=D, of which A,
        // B, C, and D are being stored in the array.
        frustum[0][0] = clip[3] - clip[0];
        frustum[0][1] = clip[7] - clip[4];
        frustum[0][2] = clip[11] - clip[8];
        frustum[0][3] = clip[15] - clip[12];
        
        frustum[1][0] = clip[3] + clip[0];
        frustum[1][1] = clip[7] + clip[4];
        frustum[1][2] = clip[11] + clip[8];
        frustum[1][3] = clip[15] + clip[12];
        
        frustum[2][0] = clip[3] + clip[1];
        frustum[2][1] = clip[7] + clip[5];
        frustum[2][2] = clip[11] + clip[9];
        frustum[2][3] = clip[15] + clip[13];
        
        frustum[3][0] = clip[3] - clip[1];
        frustum[3][1] = clip[7] - clip[5];
        frustum[3][2] = clip[11] - clip[9];
        frustum[3][3] = clip[15] - clip[13];
        
        frustum[4][0] = clip[3] - clip[2];
        frustum[4][1] = clip[7] - clip[6];
        frustum[4][2] = clip[11] - clip[10];
        frustum[4][3] = clip[15] - clip[14];
        
        frustum[5][0] = clip[3] + clip[2];
        frustum[5][1] = clip[7] + clip[6];
        frustum[5][2] = clip[11] + clip[10];
        frustum[5][3] = clip[15] + clip[14];
        
        // Finally, normalize the frustum sides.
        for(i = 0; i < 6; i++)
        {
            // D is not a part of the normal vector, thus it isn't included in
            // the magnitude computation.
            magnitude = sqrt(frustum[i][0] * frustum[i][0] + 
                             frustum[i][1] * frustum[i][1] + 
                             frustum[i][2] * frustum[i][2]);
            
            frustum[i][0] /= magnitude;
            frustum[i][1] /= magnitude;
            frustum[i][2] /= magnitude;
            frustum[i][3] /= magnitude;     // D still gets normalized.
        }
    }
}

/*******************************************************************************
    function    :   camera_module::update
    arguments   :   float deltaT - time passed since last update (in seconds)
    purpose     :   Base update fuction for camera module.
    notes       :   Camera movement is based on the "smooth" movement policy
                    in which speed values are approached rather than set.
*******************************************************************************/
void camera_module::update(float deltaT)
{
    int i;
    float interval;
    float map_height[2];    // Storage for two heightmap values
    kVector point;
    
    // Go through all possible camera movement directions and update as
    // necessary. Perform "smooth" movement and rotation computations.
    for(i = 0; i < CAM_MAX_DIR; i++)
    {
        // If enabled, approach the camera speed value, otherwise approach 0.
        if(enabled[i][(int)plyr_ctrl])
            curr_speed[i] += (cam_speed[i][(int)plyr_ctrl] - curr_speed[i]) * 2.5 * deltaT;
        else
            curr_speed[i] += -curr_speed[i] * 4.5 * deltaT;
        
        // Calculate the interval moved based on deltaT.
        interval = curr_speed[i] * deltaT;
        
        // Move camera, cylindrical->cartesian conversion
        switch(i)
        {
            case CAM_MOVE_FORWARD:
            case CAM_MOVE_BACKWARD:
                pos[0] += sin(dir[2]) * interval;
                pos[2] += cos(dir[2]) * interval;
                break;
            
            case CAM_MOVE_LEFT:
            case CAM_MOVE_RIGHT:
                pos[0] += sin(dir[2] - PIHALF) * interval;
                pos[2] += cos(dir[2] - PIHALF) * interval;
                break;
            
            case CAM_ROT_RIGHT:
            case CAM_ROT_LEFT:
                dir[2] -= interval;
                
                // Normalize yaw value to be between 0.0 and TWOPI.
                while(dir[2] > TWOPI)
                    dir[2] -= TWOPI;
                while(dir[2] < 0.0)
                    dir[2] += TWOPI;
                break;
        }
    }
    
    // Check bounds (so camera doesn't go off edge of map). This is in effect
    // a form of blockmapping.
    if(pos[0] < 0.0)
        pos[0] = 0.0;
    if(pos[0] > map.getMapWidth())
        pos[0] = map.getMapWidth();
    if(pos[2] < 0.0)
        pos[2] = 0.0;
    if(pos[2] > map.getMapHeight())
        pos[2] = map.getMapHeight();
    
    // Perform "smooth" camera elevation and "smooth" camera pitch computations.
    // Store height so redundant calculations aren't performed.
    map_height[0] = map.getOverlayHeight(pos[0], pos[2]);
    
    // Make sure camera doesn't "sink" into water.
    if(map_height[0] < map.getWaterHeight())
        map_height[0] = map.getWaterHeight();
    
    // Set a point out in front of the camera to smooth on as well    
    point = pos + vectorIn(kVector(50.0, PIHALF, dir[2]), CS_CARTESIAN);
    map_height[1] = map.getOverlayHeight(point[0], point[2]);

    // Make sure camera doesn't "sink" into water.
    if(map_height[1] < map.getWaterHeight())
        map_height[1] = map.getWaterHeight();
    
    // See if we need to raise our camera a bit...
    if(map_height[1] >= map_height[0])
        point[1] = (map_height[0] + map_height[1]) / 2.0;
    else
        point[1] = map_height[0];
    
    // Approach camera elevation value
    pos[1] += (point[1] + camera_elevate - pos[1]) * 2.0 * (deltaT > 0.5 ? 0.5 : deltaT);
    
    // Bound check (for under scenery)
    if(pos[1] < map_height[0] + 2.0)
        pos[1] = map_height[0] + 2.0;
    
    // Approach camera pitch value
    dir[1] += (camera_pitch - dir[1]) * 3.0 * (deltaT > 0.333333 ? 0.333333 : deltaT);
}
