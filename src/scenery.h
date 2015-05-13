/*******************************************************************************
                         Scenery Module - Definition
*******************************************************************************/

#ifndef SCENERY_H
#define SCENERY_H

#include "metrics.h"

// Season Types
#define SC_SEASON_SPRING            0
#define SC_SEASON_SUMMER            1
#define SC_SEASON_FALL              2
#define SC_SEASON_WINTER            3

// Weather Types
#define SC_WEATHER_CLEAR            0
#define SC_WEATHER_OVERCAST         1
#define SC_WEATHER_FOGGY            2
#define SC_WEATHER_RAINING          3
#define SC_WEATHER_DUSKDAWN         4
#define SC_WEATHER_NIGHT            5

// Tile Types
#define TT_OPEN                     0
#define TT_BRUSH                    1
#define TT_ROCKY                    2
#define TT_SPARSE_TREES             3
#define TT_DENSE_TREES              4
#define TT_PINE_TREES               5
#define TT_WHEAT_FIELD              6
#define TT_CORN_FIELD               7
#define TT_VINEYARD                 8
#define TT_DIRT_ROAD                10
#define TT_PAVED_ROAD               12
#define TT_DIRT_PAVED_EXT           13
#define TT_CITY_PAVEMENT            15
#define TT_CLIFFSIDE                20
#define TT_BACKWOOD_ROAD            23

// Tile Data
#define TD_FULL                     0
#define TD_N_CUT                    1
#define TD_E_CUT                    2
#define TD_S_CUT                    3
#define TD_W_CUT                    4
#define TD_NS_CUT                   5
#define TD_WE_CUT                   6
#define TD_NW_CUT                   7
#define TD_NE_CUT                   8
#define TD_SE_CUT                   9
#define TD_SW_CUT                   10
#define TD_WNE_CUT                  11
#define TD_NES_CUT                  12
#define TD_ESW_CUT                  13
#define TD_NSW_CUT                  14
#define TD_NESW_CUT                 15
#define TD_N_CUT_SE_SW_EDGE         16
#define TD_E_CUT_NW_SW_EDGE         17
#define TD_S_CUT_NW_NE_EDGE         18
#define TD_W_CUT_NE_SW_EDGE         19
#define TD_NW_CUT_SE_EDGE           20
#define TD_NE_CUT_SW_EDGE           21
#define TD_SE_CUT_NW_EDGE           22
#define TD_SW_CUT_NE_EDGE           23
#define TD_NW_NE_SE_SW_EDGE         24

#define TD_BRIDGE_1_X_START         100
#define TD_BRIDGE_1_X_STOP          101
#define TD_BRIDGE_1_Z_START         102
#define TD_BRIDGE_1_Z_STOP          103
#define TD_BRIDGE_2_X_START         104
#define TD_BRIDGE_2_X_STOP          105
#define TD_BRIDGE_2_Z_START         106
#define TD_BRIDGE_2_Z_STOP          107
#define TD_BRIDGE_3_X_START         108
#define TD_BRIDGE_3_X_STOP          109
#define TD_BRIDGE_3_Z_START         110
#define TD_BRIDGE_3_Z_STOP          111
#define TD_BRIDGE_4_X_START         112
#define TD_BRIDGE_4_X_STOP          113
#define TD_BRIDGE_4_Z_START         114
#define TD_BRIDGE_4_Z_STOP          115

#define SC_PARSEC_SIZE              4

#define SC_MAX_TREES_TILE           4
#define SC_MAX_TREES_GLOBAL         3000

#define SC_BRIDGE_BLOCKMAP          15

/*******************************************************************************
    class       :   scenery_module
    purpose     :   This is the main structure which controls everything Scenery
                    related. It is responsible for loading map data, building
                    the scenery, and displaying said scenery. It provides a
                    powerful getHeight function as well as a simple doesCollide
                    function, as well as other accessors.
    notes       :   1) display_firstpass will draw the base map object, which
                       consists of the advanced textured heightmap and skybox.
                    2) display_secondpass will draw remaining scenery objects,
                       which consists of billboard objects, buildings, bridges,
                       and the overlaying water graphic.
                    3) getHeight is an accurate form of height aquiring, while
                       getRelativeHeight is a faster less accurate form.
                    4) All scenery objects are culled using camera.sphereInView.
                       Culling is performed in the update() func, thus update()
                       should be called as much as possible throughout exec.
                    5) Base scenery objects are not culled (heightmap & skybox).
*******************************************************************************/
class scenery_module
{
    private:
        // Parsec culling
        struct parsec_data
        {
            GLuint dspList;
            float pos[3];
            float radius;
            bool draw;
        };
        
        // Tile mapper
        struct tilemap_data
        {
            short int png_num;          // Corresponding PNG # to use
            GLuint texture_id;              // Corresponding texture ID to use
            
            bool x_flip;                // Does TX get flipped left/right?
            bool y_flip;                // Does TX get flipped up/down?
            bool rotate_ccw;            // Does TX get rotated CCW after flips?
            
            short int tile_type;        // Uses TT_ definitions above
            short int tile_data;        // Stores extra data about tile
            short int tile_block;       // Blockmap data
        };
        
        // Dynamic scenery element object (LL)
        struct scenery_element
        {
            float pos[3];
            float dir[2];
            
            float width;
            float height;
            
            GLuint texture_id;
            float alpha;
            
            bool fade_in;
            bool fade_out;
            bool has_pitch;
            
            scenery_element* next;
            scenery_element* s_next;    // Reserved for display()
        };
        
        // Tree object (D-LL)
        struct tree_object
        {
            float pos[3];               // Position
            float dir[3];               // Rotation (curr yaw, actual yaw, pitch)
            parsec_data* parsec_ptr;    // Parsec link
            
            GLuint texture_id;          // Texture ID for tree (for bb)
            
            GLuint bb_dspList;          // Billboarded version
            float bb_alpha;             // Alpha value for billboarded
            
            GLuint f3d_dspList;         // Full 3D version
            float f3d_alpha;            // Alpha value for 3D version
            
            float scale[3];             // BB,F3D scaling factors
            
            float theta;                // Theta value for shear matrix
            
            int lpbb_count;             // # of LPBBs (0 -> LPBBs not in use)
            float** lpbb_pos;           // Position of LPBBs
            GLuint lpbb_texture;        // Texture ID of LPBBs
            
            tree_object* g_next;
            tree_object* t_next;
            
            tree_object* s_bb_next;     // Reserved for display()
            tree_object* s_f3d_next;    // Reserved for display()
        };
        
        // Bridge object (LL)
        struct bridge_object
        {
            GLuint dspList;     // Display list ID number
            
            float x_min;        // AABB definitions
            float x_max;
            float z_min;
            float z_max;
            float height;       // Bridge height (used for override)
            
            // Culling data
            bool draw;          // Draw control
            float pos[3];       // Center of object
            float radius;       // Defining Sphere
            
            bridge_object* next;
        };
        
        // Scenery tile data
        struct tile_data
        {
            short int tile_num;         // Tile number (not used/just incase)
            float plane[8];             // Plane eq. data (Ax,By,Cz,D x2)
            
            tilemap_data* tilemap_ptr;  // Tilemap link
            parsec_data* parsec_ptr;    // Parsec link
            
            float pos[3];               // North west position of tile
            float distance;             // Distance from camera
            
            GLuint overlay_dspList;     // Tile graphical overlay
            float overlay_alpha;        // Overlay alpha
            
            int sel_count;              // Scenery element count
            bool se_update;             // Controls list run-through updating
            scenery_element* sel_head;  // Scenery element list
            scenery_element* sel_tail;  // tail of se list
            
            tree_object* tol_head;      // Tree object (local) list head
            bridge_object* bridge_ptr;  // Bridge pointer (if present)
        };
        
        /* Season/Weather Control Variables */
        int season;                     // Season enum, uses SEASON type defs
        char season_name[8];            // Season name, e.g. "Spring", "Fall"
        int weather;                    // Weather enum, uses SEASON type defs
        char weather_name[16];          // Weather name, e.g. "Clear", etc.
        
        /* AHM Data */
        parsec_data* parsec;            // Parsec data (AHM culling data)
        int parsec_count;               // Parsec count
        
        float** heightmap;              // Heightmap elevation data
        tile_data** tile;               // Scenery tile data
        
        tilemap_data tilemap[256];      // Scenery tile mapper (tilemap file)
        
        /* Map Data */
        int ta_width;                   // Width and height of tile array
        int ta_height;
        
        float map_width;                // Width and height of actual map (m)
        float map_height;
        
        float tile_size;                // Map width/height multiplier
        
        float elevation_multiplier;     // Elevation multiplier (0-255 -> 0-mh)
        float max_height;               // Max height level (mh = em * 255)
        float water_height;             // Base water level
        
        /* Ext. Data */
        GLuint skybox_dspList;              // Display list for skybox
        GLuint ground_dspList;              // Display list for ground
        GLuint water_texture_id;            // Water texture ID
        float water_texture_offset;         // Water tx offset (s,t)
        float water_height_offset;          // Water height offset wave
        GLuint sc_textures[10][5];          // Scenery textures ID storage
        
        /* Object Data */
        tree_object* tol_head;              // Tree list (D-LL)
        int tree_count;
        bridge_object* bol_head;            // Bridge list (LL)
        
        /* Loading Routines */
        void load_tilemap();                // Loads scenery tilemap
        void load_mapdata(char* file);      // Loads base map data
        void load_heightmap(char* file);    // Loads heightmap data
        void load_scenery(char* file);      // Loads scenery tile data
        void load_textures();               // Loads base scenery textures
        
        /* Building Routines */
        void build_planes();                // Builds plane data for getHeights
        void build_heightmap();             // Builds the heightmap object
        void build_overlays();              // Builds tile overlays
        void build_skybox();                // Builds the skybox object
        void build_trees();                 // Builds tree objects
        void build_bridges();               // Builds bridge objects
        
        /* Misc. Routines */
        bool on_tile(int tile_type, int tile_data, float x_offset, float z_offset);
        
    public:
        scenery_module();                   // Constructor
        ~scenery_module();                  // Deconstructor
        
        /* Load Routine */
        void loadScenery(char* directory);  // Load map data from directory
        void buildScenery();
        
        /* General Routines */
        // Height routines
        float getHeight(float x_val, float z_val);
        float getOverlayHeight(float x_val, float z_val);
        float getRelativeHeight(float x_val, float z_val);
        
        // Map tile tyle determination
        int getTileType(float x_val, float z_val);
        int getBlockmap(float x_val, float z_val);
        
        /* Scenery Metric Extensions */
        // Heightmap/Ray Intersection
        kVector rayIntersect(kVector rayPos, kVector rayDir);
        // Basic Collision Detection
        bool groundCollision(kVector position);
        bool sceneryCollision(kVector position);
        
        /* Accessors */
        char* getSeasonName() { return season_name; }
        int getSeason() { return season; }
        char* getWeatherName() { return weather_name; }
        int getWeather() { return weather; }
        
        float getWaterHeight() { return water_height + water_height_offset; }
        
        float getMapWidth() { return map_width; }
        float getMapHeight() { return map_height; }
        
        float getTileSize() { return tile_size; }
        
        int getTileArrayWidth() { return ta_width; }
        int getTileArrayHeight() { return ta_height; }
        
        /* Base Display & Update Routines */
        void displayFirstPass();
        void displaySecondPass();
        void update(float deltaT);
};

extern scenery_module map;

#endif
