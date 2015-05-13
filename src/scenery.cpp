/*******************************************************************************
                        Scenery Module - Implementation
*******************************************************************************/
/* Dependencies */
#include "main.h"
#include "scenery.h"
#include "camera.h"
#include "database.h"
#include "load.h"
#include "metrics.h"
#include "misc.h"
#include "model.h"
#include "texture.h"

/*******************************************************************************
    function    :   scenery_module::scenery_module
    arguments   :   <none>
    purpose     :   Constructor.
    notes       :   <none>
*******************************************************************************/
scenery_module::scenery_module()
{
    int i, j;
    
    // Initialize all variables
    season = 0;
    season_name[0] = '\0';
    parsec = NULL;
    parsec_count = 0;
    heightmap = NULL;
    tile = NULL;
    ta_width = ta_height = 0;
    map_width = map_height = 0.0;
    tile_size = 15.0;
    elevation_multiplier = 1.0;
    max_height = 0.0;
    water_height = -1.0;
    water_texture_offset = 0.0;
    water_height_offset = 0.0;
    tol_head = NULL;
    tree_count = 0;
    bol_head = NULL;
    
    for(i = 0; i < 256; i++)
    {
        tilemap[i].png_num = 0;
        tilemap[i].texture_id = TEXTURE_NULL;
        tilemap[i].tile_type = 0;
        tilemap[i].tile_data = 0;
        tilemap[i].tile_block = 0;
        tilemap[i].x_flip = false;
        tilemap[i].y_flip = false;
        tilemap[i].rotate_ccw = false;
    }
    
    for(i = 0; i < 10; i++)
        for(j = 0; j < 5; j++)
            sc_textures[i][j] = TEXTURE_NULL;
}

/*******************************************************************************
    function    :   scenery_module::~scenery_module
    arguments   :   <none>
    purpose     :   Deconstructor.
    notes       :   <none>
*******************************************************************************/
scenery_module::~scenery_module()
{
    int i, x, z;
    scenery_element* sel_curr;
    tree_object* tol_curr;
    bridge_object* bol_curr;
    
    if(parsec)
        delete parsec;
    
    if(heightmap)
    {
        delete *heightmap;
        delete heightmap;
    }
    
    for(z = 0; z < ta_height; z++)
        for(x = 0; x < ta_width; x++)
            if((sel_curr = tile[x][z].sel_head) != NULL)
                while(sel_curr)
                {
                    tile[x][z].sel_head = tile[x][z].sel_head->next;
                    delete sel_curr;
                    sel_curr = tile[x][z].sel_head;
                }
    
    if(tile)
    {
        delete *tile;
        delete tile;
    }
    
    if((tol_curr = tol_head) != NULL)
        while(tol_curr)
        {
            if(tol_curr->lpbb_count > 0)
            {
                if(tol_curr->lpbb_pos)
                {
                    for(i = 0; i < tol_curr->lpbb_count; i++)
                        if(tol_curr->lpbb_pos[i])
                            delete tol_curr->lpbb_pos[i];
                    delete tol_curr->lpbb_pos;
                }
            }
            tol_head = tol_head->g_next;
            delete tol_curr;
            tol_curr = tol_head;
        }
    
    if((bol_curr = bol_head) != NULL)
        while(bol_curr)
        {
            bol_head = bol_head->next;
            delete bol_curr;
            bol_curr = bol_head;
        }
}

/*******************************************************************************
    Map Loading Routines
*******************************************************************************/

/*******************************************************************************
    function    :   scenery_module::loadScenery
    arguments   :   directory - base directory of mission files
    purpose     :   Base function that invokes all other loading routines
                    which build the general scenery of the game.
    notes       :   Load failures are handled in each individual function.
*******************************************************************************/
void scenery_module::loadScenery(char* directory)
{
    char buffer[128];
    
    // Load tilemap
    load_tilemap();
    loader.advanceLoadBar(5);
    loader.display();
    
    // Load base map data
    sprintf(buffer, "%s/mission.mapdata", directory);
    load_mapdata(buffer);
    loader.advanceLoadBar(5);
    loader.display();
    
    // Load heightmap data.
    sprintf(buffer, "%s/mission.heightmap.bmp", directory);
    load_heightmap(buffer);
    loader.advanceLoadBar(5);
    loader.display();
    
    // Load scenery data.
    sprintf(buffer, "%s/mission.scenery.bmp", directory);
    load_scenery(buffer);
    loader.advanceLoadBar(5);
    loader.display();
    
    // Load base scenery textures
    load_textures();
    loader.advanceLoadBar(15);
    loader.display();
}
    
/*******************************************************************************
    function    :   scenery_module::buildScenery
    arguments   :   <none>
    purpose     :   Base function that invokes all other building routines
                    which build the general scenery of the game.
    notes       :   Load failures are handled in each individual function.
*******************************************************************************/
void scenery_module::buildScenery()
{
    // Build plane data for getHeights
    build_planes();
    loader.advanceLoadBar(5);
    loader.display();
    
    // Build heightmap
    build_heightmap();
    loader.advanceLoadBar(10);
    loader.display();
    
    // Build graphical overlays
    build_overlays();
    loader.advanceLoadBar(5);
    loader.display();
    
    // Build skybox
    build_skybox();
    loader.advanceLoadBar(5);
    loader.display();
    
    // Build tree objects
    build_trees();
    loader.advanceLoadBar(10);
    loader.display();
    
    // Build bridge objects
    build_bridges();
    loader.advanceLoadBar(5);
    loader.display();
}

/*******************************************************************************
    function    :   scenery_module::load_tilemap
    arguments   :   <none>
    purpose     :   Loads the scenery.tilemap file.
    notes       :   <none>
*******************************************************************************/
void scenery_module::load_tilemap()
{
    ifstream fin;
    int tile_num;
    int temp;           // For >> boolean Win32 compatibility
    
    // Load scenery tile mapping file
    fin.open("Scenery/scenery.tilemap");
    if(!fin)
    {
        // If error, write error message and exit
        write_error("Scenery: FATAL: Scenery tilemap not found.");
        exit(1);
    }
    
    // Clean through whitespace
    eatjunk(fin);
    
    // Read in tile map data (slow process, oh well)
    while(!fin.eof())
    {   
        // Check for comment line
        if(fin.peek() == '#')
        {
            // Comment line - skip
            while(fin.peek() != 0x0A && fin.peek() != 0x0D && !fin.eof())
                fin.ignore(1);
        }
        else
        {
            // The use of fin this way is kinda slow, but oh well.
            fin >> tile_num;
            fin >> tilemap[tile_num].png_num;
            fin >> temp; tilemap[tile_num].x_flip = (bool)temp;
            fin >> temp; tilemap[tile_num].y_flip = (bool)temp;
            fin >> temp; tilemap[tile_num].rotate_ccw = (bool)temp;
            fin >> tilemap[tile_num].tile_type;
            fin >> tilemap[tile_num].tile_data;
            fin >> tilemap[tile_num].tile_block;
        }
        
        // Clean through whitespace
        eatjunk(fin);
    }
    fin.close();
}

/*******************************************************************************
    function    :   scenery_module::load_mapdata
    arguments   :   file - base mapdata file to be loaded from mission directory
    purpose     :   Loads the base map data into memory from mission file.
    notes       :   <none>
*******************************************************************************/
void scenery_module::load_mapdata(char* file)
{
    ifstream fin;
    char buffer[128];
    float cam_start[3];
    
    // Open file
    fin.open(file);
    
    // Check for open
    if(!fin)
    {
        // If error, write error message and exit
        sprintf(buffer, "Scenery: FATAL: Failure loading \"%s\" for read.",
            file);
        write_error(buffer);
        exit(1);
    }
    
    // Clean through whitespace
    eatjunk(fin);
    
    // Start reading in map data
    while(!fin.eof())
    {
        fin.getline(buffer, 120, '\n');  // Grab line from file
        
        // Parse through params and assign values (uses sscanf for parsing)
        if(buffer[0] == '#') ;
        else if(sscanf(buffer, "MAP_WIDTH = %f", &map_width)) ;
        else if(sscanf(buffer, "MAP_HEIGHT = %f", &map_height)) ;
        else if(sscanf(buffer, "TILE_SIZE = %f", &tile_size)) ;
        else if(sscanf(buffer, "MAX_ELEVATION = %f", &max_height)) ;
        else if(sscanf(buffer, "WATER_ELEVATION = %f", &water_height)) ;
        else if(sscanf(buffer, "SEASON = %s", season_name))
        {
            // Assign season variable
            if(strcmp(season_name, "Spring") == 0)
                season = SC_SEASON_SPRING;
            else if(strcmp(season_name, "Summer") == 0)
                season = SC_SEASON_SUMMER;
            else if(strcmp(season_name, "Fall") == 0)
                season = SC_SEASON_FALL;
            else if(strcmp(season_name, "Winter") == 0)
                season = SC_SEASON_WINTER;
            else
            {
                // Unsupported season type - print error
                sprintf(buffer,
                    "Scenery: Season type \"%s\" not supported.", season_name);
                write_error(buffer);
                // Set default to spring
                strcpy(season_name, "Spring");
                season = SC_SEASON_SPRING;
            }
        }
        else if(sscanf(buffer, "WEATHER = %s", weather_name))
        {
            // Assign weather variable
            if(strcmp(weather_name, "Clear") == 0)
                weather = SC_WEATHER_CLEAR;
            else if(strcmp(weather_name, "Overcast") == 0)
                weather = SC_WEATHER_OVERCAST;
            else if(strcmp(weather_name, "Foggy") == 0)
                weather = SC_WEATHER_FOGGY;
            else if(strcmp(weather_name, "Raining") == 0)
                weather = SC_WEATHER_RAINING;
            else if(strcmp(weather_name, "Dusk") == 0 || strcmp(weather_name, "Dawn") == 0)
                weather = SC_WEATHER_DUSKDAWN;
            else if(strcmp(weather_name, "Night") == 0)
                weather = SC_WEATHER_NIGHT;
            else
            {
                // Unsupported weather type - print error
                sprintf(buffer,
                    "Scenery: Weather type \"%s\" not supported.", weather_name);
                write_error(buffer);
                // Set default to clear
                strcpy(weather_name, "Clear");
                weather = SC_WEATHER_CLEAR;
            }
        }
        else if((game_setup.player_side == PS_AXIS &&
                sscanf(buffer, "AXIS_CAMSTART = %f %f %f",
                    &cam_start[0], &cam_start[1], &cam_start[2]) == 3) ||
            (game_setup.player_side == PS_ALLIED &&
                sscanf(buffer, "ALLIED_CAMSTART = %f %f %f",
                    &cam_start[0], &cam_start[1], &cam_start[2]) == 3))
        {
            camera.setCamPos(cam_start[0], 0.0, cam_start[1]);
            camera.setCamDir(1.0, PIHALF + (PIHALF / 4.0),
                             (180.0 - cam_start[2]) * degToRad);
        }
        
        // Clean through whitespace
        eatjunk(fin);
    }
    fin.close();    // Close file
    
    // Calculate tile array width and height from read-in data
    ta_width = (int)((float)map_width / tile_size);
    ta_height = (int)((float)map_height / tile_size);
    
    // Calculate elevation multiplier from read-in data
    elevation_multiplier =  max_height / 255.0;
}

/*******************************************************************************
    function    :   scenery_module::load_heightmap
    arguments   :   file - base heightmap data file to be loaded for mission
    purpose     :   Loads the heightmap data into memory from mission file.
    notes       :   The heightmap data will always consist of 1 extra element
                    per width and height since tiles require a point on both
                    sides and this produces the "off by 1" effect.
*******************************************************************************/
void scenery_module::load_heightmap(char* file)
{
    int x, z;
    int i;
    void* temp_ptr;
    char buffer[128];
    GLubyte* data_array;
    
    // Open heightmap data file
    data_array = loadImage(file, 8, x, z);
    
    // Check for open
    if(!data_array)
    {
        // If error, write error message and exit
        sprintf(buffer, "Scenery: FATAL: Failure loading \"%s\" for read.",
            file);
        write_error(buffer);
        exit(1);
    }
    
    // Check for correct size
    if(x != ta_width + 1 || z != ta_height + 1)
    {
        // Write out error message and exit
        sprintf(buffer,
            "Scenery: FATAL: Map data width/height does not match \"%s\".",
            file);
        write_error(buffer);
        exit(1);
    }
    
    // Allocate memory for heightmap data
    heightmap = new float* [ta_width + 1];
    temp_ptr = (void*)(new float [(ta_width + 1) * (ta_height + 1)]);
    for(i = 0; i < ta_width + 1; i++)
        heightmap[i] = (float*)temp_ptr + (i * (ta_height + 1));
    
    // Allocate memory for parsec culling data
    parsec_count = (int)(ceil((float)ta_width / (float)SC_PARSEC_SIZE) *
                         ceil((float)ta_height / (float)SC_PARSEC_SIZE));
    parsec = new parsec_data[parsec_count];
    
    // Read in values through the elevation_multiplier to the heightmap
    for(z = 0; z < ta_height + 1; z++)
        for(x = 0; x < ta_width + 1; x++)
            heightmap[x][z] = ((float)data_array[(z * (ta_width + 1)) + x])
                * elevation_multiplier;
    
    delete data_array;
}

/*******************************************************************************
    function    :   scenery_module::load_scenery
    arguments   :   file - base scenery data file to be loaded for mission
    purpose     :   Loads scenery data into memory from mission file.
    notes       :   1) This function is also responsible for loading the base
                       scenery file scenery.tilemap, which is used to map all
                       tiles to specific .png files as well as store some vital
                       information the program will require about said tile.
                    2) scenery_object is also allocated herein, and must be
                       initialized to -1 for all positions - this way getHeight
                       knows that no object index is specified for that tile.
*******************************************************************************/
void scenery_module::load_scenery(char* file)
{
    int i, x, z;
    void* temp_ptr;
    char buffer[128];
    ifstream fin;
    GLubyte* data_array = NULL;
    int parsec_pitch;
    
    // Set pitch size of parsec layout (width of parsecs across board)
    parsec_pitch = (int)ceil((float)ta_width / (float)SC_PARSEC_SIZE);
    
    // Open scenery data file
    data_array = loadImage(file, 8, x, z);
    
    // Check for open
    if(!data_array)
    {
        // If error, write error message and exit
        sprintf(buffer, "Scenery: FATAL: Failure loading \"%s\" for read.",
            file);
        write_error(buffer);
        exit(1);
    }
    
    // Check for correct size
    if(x != ta_width || z != ta_height)
    {
        // Write out error message and exit
        sprintf(buffer,
            "Scenery: FATAL: Map data width/height does not match \"%s\".",
            file);
        write_error(buffer);
        exit(1);
    }
    
    // Allocate memory for scenery data
    tile = new tile_data* [ta_width];
    temp_ptr = (void*)(new tile_data [ta_width * ta_height]);
    for(i = 0; i < ta_width; i++)
        tile[i] = (tile_data*)temp_ptr + (i * ta_height);
    
    // Store read-in values to the scenery data
    for(z = 0; z < ta_height; z++)
        for(x = 0; x < ta_width; x++)
        {
            tile[x][z].tile_num = data_array[(z * ta_width) + x];
            tile[x][z].tilemap_ptr = &tilemap[tile[x][z].tile_num];
            tile[x][z].parsec_ptr = &parsec[
                ((z / SC_PARSEC_SIZE) * parsec_pitch) + (x / SC_PARSEC_SIZE)];
            tile[x][z].pos[0] = x * tile_size;
            tile[x][z].pos[1] = 0.0;
            tile[x][z].pos[2] = z * tile_size;
            tile[x][z].distance = 5000.0;           // Temp init
            tile[x][z].overlay_dspList = DISPLAY_NULL;
            tile[x][z].overlay_alpha = 0.8;
            tile[x][z].sel_count = 0;
            tile[x][z].se_update = false;
            tile[x][z].sel_head = NULL;
            tile[x][z].sel_tail = NULL;
            tile[x][z].tol_head = NULL;
            tile[x][z].bridge_ptr = NULL;
        }
    
    delete data_array;
}

/*******************************************************************************
    function    :   scenery_module::load_textures()
    arguments   :   <none>
    purpose     :   Pre-loads all textures associated with the scenery.
    notes       :   <none>
*******************************************************************************/
void scenery_module::load_textures()
{
    int i, x, z;
    GLubyte* base_image;
    GLubyte* curr_image;
    int width, height;
    char buffer[128];
    char prefix[10][16] = {{"grass"}, {"brush"}, {"rocky"}, {"leaves"}, {'\0'},
                           {'\0'}, {"wheat"}, {"corn"}, {"vine"}, {'\0'}};
    int tile_type;
    int png_num;
    
    // Load water texture
    water_texture_id = textures.loadTexture("Scenery/Misc/water.png", "Scenery/water.png");
    
    // Set texture wrapping to clamped for tiles and SE textures
    textures.setWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    
    // Load skybox textures
    textures.loadTexture("Scenery/Misc/ground.jpg", "Scenery/ground.jpg");
    
    switch(weather)
    {
        case SC_WEATHER_CLEAR:
            textures.loadTexture("Scenery/Misc/sky_clear.png", "Scenery/skybox.png");
            break;
        
        case SC_WEATHER_OVERCAST:
        case SC_WEATHER_FOGGY:
        case SC_WEATHER_RAINING:
            textures.loadTexture("Scenery/Misc/sky_overcast.png", "Scenery/skybox.png");
            break;
        
        case SC_WEATHER_DUSKDAWN:
            textures.loadTexture("Scenery/Misc/sky_dawndusk.png", "Scenery/skybox.png");
            break;
        
        case SC_WEATHER_NIGHT:
            textures.loadTexture("Scenery/Misc/sky_night.png", "Scenery/skybox.png");
            break;
    }
    
    // Load base texture
    sprintf(buffer, "Scenery/%s/0.png", season_name);
    base_image = loadImage(buffer, 32, width, height);
    if(base_image)
        tilemap[0].texture_id =
            textures.addTexture(buffer, base_image, 32, width, height);
    else
    {
        sprintf(buffer, "Scenery: FATAL: Tile base Scenery/%s/0.png not found!",
            season_name);
        write_error(buffer);
        exit(1);
    }
    
    // Load all tile and SE textures
    for(z = 0; z < ta_height; z++)
        for(x = 0; x < ta_width; x++)
            {
                // For ease
                tile_type = tile[x][z].tilemap_ptr->tile_type;
                png_num = tile[x][z].tilemap_ptr->png_num;
                
                // Load base tile graphic
                if(tile[x][z].tilemap_ptr->texture_id == TEXTURE_NULL)
                {
                    if(tilemap[png_num].texture_id != TEXTURE_NULL)
                    {
                        // Copy over texture if already loaded
                        tile[x][z].tilemap_ptr->texture_id =
                            tilemap[png_num].texture_id;
                    }
                    else
                    {
                        // Try loading tile texture from specific season
                        sprintf(buffer, "Scenery/%s/%i.png", season_name, png_num);
                        if(!fileExists(buffer))
                        {
                            // If that doesn't work, load texture from common
                            sprintf(buffer, "Scenery/Common/%i.png", png_num);
                            if(!fileExists(buffer))
                            {
                                // Error - tile file not found
                                sprintf(buffer,
                                    "Scenery: FATAL: Scenery/%s/%i.png not found!",
                                    season_name, png_num);
                                write_error(buffer);
                                exit(1);
                            }
                        }
                        
                        // Load image, blend into base, and register with texlib
                        curr_image = loadImage(buffer, 32, width, height);
                        blendImage(base_image, curr_image, 32, width, height);
                        sprintf(buffer, "Scenery/%i.png", png_num);
                        tile[x][z].tilemap_ptr->texture_id =
                            textures.addTexture(buffer, curr_image, 32, width, height);
                        
                        // Free current texture
                        delete curr_image;
                    }
                }
                
                // Load base SE textures
                switch(tile_type)
                {
                    case TT_OPEN:
                    case TT_ROCKY:
                        if(sc_textures[tile_type][0] == TEXTURE_NULL)
                        {
                            // Load SE texture for this tile - 4 chances at
                            // texture #s, in each of the three sub dirs.
                            sc_textures[tile_type][0] = 0;
                            for(i = 1; i <= 4; i++)
                            {
                                // Try in season, common, and finally misc
                                sprintf(buffer, "Scenery/%s/%s_%i.png", season_name, prefix[tile_type], i);
                                if(!fileExists(buffer))
                                {
                                    sprintf(buffer, "Scenery/Common/%s_%i.png", prefix[tile_type], i);
                                    if(!fileExists(buffer))
                                    {
                                        sprintf(buffer, "Scenery/Misc/%s_%i.png", prefix[tile_type], i);
                                        if(!fileExists(buffer))
                                            continue;   // F-it, Next!
                                    }
                                }
                                
                                // Load file (finally) into texture library and
                                // into sc_textures listing.
                                sprintf(&buffer[64], "Scenery/%s_%i.png", prefix[tile_type], i);
                                sc_textures[tile_type][sc_textures[tile_type][0]+1] =
                                    textures.loadTexture(buffer, &buffer[64]);
                                sc_textures[tile_type][0]++;
                            }
                        }
                        break;
                        
                    case TT_BRUSH:
                    case TT_SPARSE_TREES:
                    case TT_DENSE_TREES:
                    case TT_PINE_TREES:
                        // Brush & Trees use TT_OPEN and TT_BRUSH textures
                        if(sc_textures[TT_OPEN][0] == TEXTURE_NULL)
                        {
                            // Load SE texture for this tile - 4 chances at
                            // texture #s, in each of the three sub dirs.
                            sc_textures[TT_OPEN][0] = 0;
                            for(i = 1; i <= 4; i++)
                            {
                                // Try in season, common, and finally misc
                                sprintf(buffer, "Scenery/%s/%s_%i.png", season_name, prefix[TT_OPEN], i);
                                if(!fileExists(buffer))
                                {
                                    sprintf(buffer, "Scenery/Common/%s_%i.png", prefix[TT_OPEN], i);
                                    if(!fileExists(buffer))
                                    {
                                        sprintf(buffer, "Scenery/Misc/%s_%i.png", prefix[TT_OPEN], i);
                                        if(!fileExists(buffer))
                                            continue;   // F-it, Next!
                                    }
                                }
                                
                                // Load file (finally) into texture library and
                                // into sc_textures listing.
                                sprintf(&buffer[64], "Scenery/%s_%i.png", prefix[TT_OPEN], i);
                                sc_textures[TT_OPEN][sc_textures[TT_OPEN][0]+1] =
                                    textures.loadTexture(buffer, &buffer[64]);
                                sc_textures[TT_OPEN][0]++;
                            }
                        }
                        if(sc_textures[TT_BRUSH][0] == TEXTURE_NULL)
                        {
                            // Load SE texture for this tile - 4 chances at
                            // texture #s, in each of the three sub dirs.
                            sc_textures[TT_BRUSH][0] = 0;
                            for(i = 1; i <= 4; i++)
                            {
                                // Try in season, common, and finally misc
                                sprintf(buffer, "Scenery/%s/%s_%i.png", season_name, prefix[TT_BRUSH], i);
                                if(!fileExists(buffer))
                                {
                                    sprintf(buffer, "Scenery/Common/%s_%i.png", prefix[TT_BRUSH], i);
                                    if(!fileExists(buffer))
                                    {
                                        sprintf(buffer, "Scenery/Misc/%s_%i.png", prefix[TT_BRUSH], i);
                                        if(!fileExists(buffer))
                                            continue;   // F-it, Next!
                                    }
                                }
                                
                                // Load file (finally) into texture library and
                                // into sc_textures listing.
                                sprintf(&buffer[64], "Scenery/%s_%i.png", prefix[TT_BRUSH], i);
                                sc_textures[TT_BRUSH][sc_textures[TT_BRUSH][0]+1] =
                                    textures.loadTexture(buffer, &buffer[64]);
                                sc_textures[TT_BRUSH][0]++;
                            }
                        }
                        // Load leaves for LPBBs
                        if(tile_type != TT_BRUSH && sc_textures[TT_SPARSE_TREES][0] == TEXTURE_NULL)
                        {
                            // Load leave textures for this tile - 4 chances at
                            // texture #s, in each of the three sub dirs.
                            sc_textures[TT_SPARSE_TREES][0] = 0;
                            for(i = 1; i <= 4; i++)
                            {
                                // Try in season, common, and finally misc
                                sprintf(buffer, "Scenery/%s/%s_%i.png", season_name, prefix[TT_SPARSE_TREES], i);
                                if(!fileExists(buffer))
                                {
                                    sprintf(buffer, "Scenery/Common/%s_%i.png", prefix[TT_SPARSE_TREES], i);
                                    if(!fileExists(buffer))
                                    {
                                        sprintf(buffer, "Scenery/Misc/%s_%i.png", prefix[TT_SPARSE_TREES], i);
                                        if(!fileExists(buffer))
                                            continue;   // F-it, Next!
                                    }
                                }
                                
                                // Load file (finally) into texture library and
                                // into sc_textures listing.
                                sprintf(&buffer[64], "Scenery/%s_%i.png", prefix[TT_SPARSE_TREES], i);
                                sc_textures[TT_SPARSE_TREES][sc_textures[TT_SPARSE_TREES][0]+1] =
                                    textures.loadTexture(buffer, &buffer[64]);
                                sc_textures[TT_SPARSE_TREES][0]++;
                            }
                        }
                        break;
                        
                    case TT_WHEAT_FIELD:
                    case TT_CORN_FIELD:
                    case TT_VINEYARD:
                        // Crop fields have an additive overlay
                        if(sc_textures[tile_type][0] == TEXTURE_NULL)
                        {
                            // Load SE texture for this tile - 4 chances at
                            // texture #s, in each of the three sub dirs.
                            sc_textures[tile_type][0] = 0;
                            for(i = 1; i <= 4; i++)
                            {
                                // Try in season, common, and finally misc
                                sprintf(buffer, "Scenery/%s/%s_%i.png", season_name, prefix[tile_type], i);
                                if(!fileExists(buffer))
                                {
                                    sprintf(buffer, "Scenery/Common/%s_%i.png", prefix[tile_type], i);
                                    if(!fileExists(buffer))
                                    {
                                        sprintf(buffer, "Scenery/Misc/%s_%i.png", prefix[tile_type], i);
                                        if(!fileExists(buffer))
                                            continue;   // F-it, Next!
                                    }
                                }
                                
                                // Load file (finally) into texture library and
                                // into sc_textures listing.
                                sprintf(&buffer[64], "Scenery/%s_%i.png", prefix[tile_type], i);
                                sc_textures[tile_type][sc_textures[tile_type][0]+1] =
                                    textures.loadTexture(buffer, &buffer[64]);
                                sc_textures[tile_type][0]++;
                            }
                            
                            // Load-in additive overlay
                            // Try in season, common, and finally misc
                            sprintf(buffer, "Scenery/%s/%s_overlay.png", season_name, prefix[tile_type]);
                            if(!fileExists(buffer))
                            {
                                sprintf(buffer, "Scenery/Common/%s_overlay.png",  prefix[tile_type]);
                                if(!fileExists(buffer))
                                {
                                    sprintf(buffer, "Scenery/Misc/%s_overlay.png",  prefix[tile_type]);
                                    if(!fileExists(buffer))
                                        break;  // F-it, Next!
                                }
                            }
                            
                            // Load overlay file into texlib
                            sprintf(&buffer[64], "Scenery/%s_overlay.png", prefix[tile_type]);
                            textures.loadTexture(buffer, &buffer[64]);
                        }
                        break;
                    
                    default:
                        break;
                }
                
                // Load bridge textures up if a starting tile is found (TD)
                switch(tile[x][z].tilemap_ptr->tile_data)
                {
                    case TD_BRIDGE_1_X_START:
                    case TD_BRIDGE_1_Z_START:
                    case TD_BRIDGE_2_X_START:
                    case TD_BRIDGE_2_Z_START:
                    case TD_BRIDGE_3_X_START:
                    case TD_BRIDGE_3_Z_START:
                    case TD_BRIDGE_4_X_START:
                    case TD_BRIDGE_4_Z_START:
                        i = ((tile[x][z].tilemap_ptr->tile_data - TD_BRIDGE_1_X_START) / 4) + 1;
                        
                        sprintf(buffer, "Scenery/%s/bridge_%i_sides.png", season_name, i);
                        if(!fileExists(buffer))
                        {
                            sprintf(buffer, "Scenery/Common/bridge_%i_sides.png", i);
                            if(!fileExists(buffer))
                            {
                                sprintf(buffer, "Scenery/Misc/bridge_%i_sides.png",  i);
                                if(!fileExists(buffer))
                                    break;  // F-it, Next!
                            }
                        }
                        
                        // Set texture wrapping for sides
                        textures.setWrapping(GL_REPEAT, GL_CLAMP_TO_EDGE);
                        sprintf(&buffer[64], "Scenery/bridge_%i_sides.png", i);
                        textures.loadTexture(buffer, &buffer[64]);
                        
                        sprintf(buffer, "Scenery/%s/bridge_%i_road.png", season_name, i);
                        if(!fileExists(buffer))
                        {
                            sprintf(buffer, "Scenery/Common/bridge_%i_road.png", i);
                            if(!fileExists(buffer))
                            {
                                sprintf(buffer, "Scenery/Misc/bridge_%i_road.png",  i);
                                if(!fileExists(buffer))
                                {
                                    // Set texture wrapping back to clamped
                                    textures.setWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
                                    break;  // F-it, Next!
                                }
                            }
                        }
                        
                        // Set texture wrapping for road
                        textures.setWrapping(GL_CLAMP_TO_EDGE, GL_REPEAT);
                        sprintf(&buffer[64], "Scenery/bridge_%i_road.png", i);
                        textures.loadTexture(buffer, &buffer[64]);
                        
                        // Set texture wrapping back to clamped
                        textures.setWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
                        break;
                        
                    default:
                        break;
                }
            }
    
    // Free base texture
    delete base_image;
    
    // Reset texture wrapping to system default.
    textures.setWrapping();
}

/*******************************************************************************
    Map Building Routines
*******************************************************************************/

/*******************************************************************************
    function    :   scenery_module::build_planes()
    arguments   :   <none>
    purpose     :   Builds the heightmap planes which are used to calculate
                    the approximate height of the map at any position.
    notes       :   Is based off of extensive use of the plane eq. Ax+By+Cz=D
                    as well as the use of a reduced version of the "planer best
                    fit" algorithm which manufacturers the best fit plane to
                    four points in 3D space.
*******************************************************************************/
void scenery_module::build_planes()
{
    int x, z;
    kVector u;
    kVector v;
    kVector n;
    
    for(z = 0; z < ta_height; z++)
        for(x = 0; x < ta_width; x++)
        {
            // 0 - North West Trisect
            // U = west edge, V = north edge
            u = kVector(0.0, heightmap[x][z+1] - heightmap[x][z], tile_size);
            v = kVector(tile_size, heightmap[x+1][z] - heightmap[x][z], 0.0);
            // N = cross u,v, D = dot N point
            n = normalized(crossProduct(u, v));
            tile[x][z].plane[0] = n[0];
            tile[x][z].plane[1] = n[1];
            tile[x][z].plane[2] = n[2];
            tile[x][z].plane[3] = dotProduct(n, kVector(
                x * tile_size, heightmap[x][z], z * tile_size));
            
            // 1 - South East Trisect
            // U = east edge, V = south edge
            u = kVector(0.0, heightmap[x+1][z] - heightmap[x+1][z+1], -tile_size);
            v = kVector(-tile_size, heightmap[x][z+1] - heightmap[x+1][z+1], 0.0);
            // N = cross u,v, D = dot N point
            n = normalized(crossProduct(u, v));
            tile[x][z].plane[4] = n[0];
            tile[x][z].plane[5] = n[1];
            tile[x][z].plane[6] = n[2];
            tile[x][z].plane[7] = dotProduct(n, kVector(
                (x+1) * tile_size, heightmap[x+1][z+1], (z+1) * tile_size));
        }
}

/*******************************************************************************
    function    :   scenery_module::build_heightmap
    arguments   :   <none>
    purpose     :   Builds the AHM and compiles it into a series of display
                    lists per each parsec cut.
    notes       :   1) To save from having to rotate the Texture matrix we have
                       specialized functions which control rotation and flipping
                       of tiles in terms of the UV mapping coordinates used.
                    2) glColor is called to set the heightmap to have a unique
                       brightness based on the height of the vertex - this makes
                       a very interesting effect in which high portions of the
                       map are lighter in color than lower portions, giving a
                       nice touch of depth.
*******************************************************************************/
void scenery_module::build_heightmap()
{
    int i, x, z;
    bool x_flip, y_flip, rotate;    // For ease of coding
    int parsec_pitch;
    float y_min;                    // For parsec's culling radius detect
    float y_max;
    
    // Set pitch size of parsec layout (width of parsecs across board)
    parsec_pitch = (int)ceil((float)ta_width / (float)SC_PARSEC_SIZE);
    
    // Go through and build each parsec
    for(i = 0; i < parsec_count; i++)
    {   
        // Set up min/max y finders for this parsec (used for radius detect)
        y_min = max_height;
        y_max = 0.0;
        
        // Create a new display list for this parsec
        parsec[i].dspList = glGenLists(1);
        glNewList(parsec[i].dspList, GL_COMPILE);
        
        for(x = (i % parsec_pitch) * SC_PARSEC_SIZE;
            x < ((i % parsec_pitch) + 1) * SC_PARSEC_SIZE &&
            x < ta_width; x++)
            for(z = (i / parsec_pitch) * SC_PARSEC_SIZE;
                z < ((i / parsec_pitch) + 1) * SC_PARSEC_SIZE &&
                z < ta_height; z++)
            {
                // Bind an initial texture for the initial row
                glBindTexture(GL_TEXTURE_2D, tile[x][z].tilemap_ptr->texture_id);
                
                // Start heightmap for this tile
                glBegin(GL_TRIANGLE_STRIP);
                
                // Save us the headache of having to type out the big longwinded
                // line of code.
                rotate = tile[x][z].tilemap_ptr->rotate_ccw;
                
                // In actuality, the x and y flip are flipped if rotate is true.
                // This is done in relation to the flips being done mentally
                // first, and then the texture is rotated - even though
                // technically it works backwards from that.
                if(!rotate)
                {
                    x_flip = tile[x][z].tilemap_ptr->x_flip;
                    y_flip = tile[x][z].tilemap_ptr->y_flip;
                }
                else
                {
                    x_flip = tile[x][z].tilemap_ptr->y_flip;
                    y_flip = tile[x][z].tilemap_ptr->x_flip;
                }
                
                // Set the normal vector for this upper edge to 0th trisect.
                glNormal3fv(&tile[x][z].plane[0]);
                
                // Vertex 0 (North West)
                //glColor3f(0.3 * (heightmap[x][z] / max_height) + 0.7,
                //          0.3 * (heightmap[x][z] / max_height) + 0.7,
                //          0.3 * (heightmap[x][z] / max_height) + 0.7);
                if(!rotate)
                    glTexCoord2f(               // orig: 0.0, 1.0
                        x_flip ? 1.0 : 0.0,
                        y_flip ? 0.0 : 1.0);
                else
                    glTexCoord2f(               // orig: 1.0, 1.0
                        x_flip ? 0.0 : 1.0,
                        y_flip ? 0.0 : 1.0);
                glVertex3f(x * tile_size, heightmap[x][z], z * tile_size);
                
                // Check for new min/max y value for current vertex
                if(heightmap[x][z] < y_min)
                    y_min = heightmap[x][z];
                if(heightmap[x][z] > y_max)
                    y_max = heightmap[x][z];
                
                // Vertex 1 (North East)
                //glColor3f(0.3 * (heightmap[x+1][z] / max_height) + 0.7,
                //          0.3 * (heightmap[x+1][z] / max_height) + 0.7,
                //          0.3 * (heightmap[x+1][z] / max_height) + 0.7);
                if(!rotate)
                    glTexCoord2f(               // orig: 1.0, 1.0
                        x_flip ? 0.0 : 1.0,
                        y_flip ? 0.0 : 1.0);
                else
                    glTexCoord2f(               // orig: 1.0, 0.0
                        x_flip ? 0.0 : 1.0,
                        y_flip ? 1.0 : 0.0);
                glVertex3f((x+1) * tile_size, heightmap[x+1][z], z * tile_size);
                
                // Check for new min/max y value for current vertex
                if(heightmap[x+1][z] < y_min)
                    y_min = heightmap[x+1][z];
                if(heightmap[x+1][z] > y_max)
                    y_max = heightmap[x+1][z];
                
                // Set the normal vector for this lower edge to 1st trisect.
                glNormal3fv(&tile[x][z].plane[4]);
                
                // Vertex 2 (South West)
                //glColor3f(0.3 * (heightmap[x][z+1] / max_height) + 0.7,
                //          0.3 * (heightmap[x][z+1] / max_height) + 0.7,
                //          0.3 * (heightmap[x][z+1] / max_height) + 0.7);
                if(!rotate)
                    glTexCoord2f(               // orig: 0.0, 0.0 (tex)
                        x_flip ? 1.0 : 0.0,
                        y_flip ? 1.0 : 0.0);
                else
                    glTexCoord2f(               // orig: 0.0 (tex), 1.0
                        x_flip ? 1.0 : 0.0,
                        y_flip ? 0.0 : 1.0);
                glVertex3f(x * tile_size, heightmap[x][z+1], (z+1) * tile_size);
                
                // Check for new min/max y value for current vertex
                if(heightmap[x][z+1] < y_min)
                    y_min = heightmap[x][z+1];
                if(heightmap[x][z+1] > y_max)
                    y_max = heightmap[x][z+1];
                
                // Vertex 3 (South East)
                //glColor3f(0.3 * (heightmap[x+1][z+1] / max_height) + 0.7,
                //          0.3 * (heightmap[x+1][z+1] / max_height) + 0.7,
                //          0.3 * (heightmap[x+1][z+1] / max_height) + 0.7);
                if(!rotate)
                    glTexCoord2f(               // orig: 1.0, 0.0 (tex)
                        x_flip ? 0.0 : 1.0,
                        y_flip ? 1.0 : 0.0);
                else
                    glTexCoord2f(               // orig: 0.0 (tex), 0.0
                        x_flip ? 1.0 : 0.0,
                        y_flip ? 1.0 : 0.0);
                glVertex3f((x+1) * tile_size, heightmap[x+1][z+1], (z+1) * tile_size);
                
                // Check for new min/max y value for current vertex
                if(heightmap[x+1][z+1] < y_min)
                    y_min = heightmap[x+1][z+1];
                if(heightmap[x+1][z+1] > y_max)
                    y_max = heightmap[x+1][z+1];
                
                glEnd();
            }
        
        // End display list for this parsec
        glEndList();
        
        // Set remaining parsec data
        parsec[i].pos[0] = ((float)((i % parsec_pitch) * SC_PARSEC_SIZE) +
            ((float)SC_PARSEC_SIZE / 2.0)) * tile_size;
        parsec[i].pos[1] = (y_min + y_max) / 2.0;
        parsec[i].pos[2] = ((float)((i / parsec_pitch) * SC_PARSEC_SIZE) +
            ((float)SC_PARSEC_SIZE / 2.0)) * tile_size;
        parsec[i].radius = sqrt(
            ((((float)SC_PARSEC_SIZE * (float)SC_PARSEC_SIZE) *
                tile_size * tile_size) / 2.0) + 
            ((y_max - parsec[i].pos[1])*(y_max - parsec[i].pos[1])));
        parsec[i].draw = true;
    }
}

/*******************************************************************************
    function    :   scenery_module::build_overlays
    arguments   :   <none>
    purpose     :   Builds any overlaying graphics that appear overtop of the
                    AHM and compiles it into a display list.
    notes       :   1) To save from having to rotate the Texture matrix we have
                       specialized functions which control rotation and flipping
                       of tiles in terms of the UV mapping coordinates used.
                    2) glColor is called to set the overlay to have a unique
                       brightness based on the height of the vertex - this makes
                       a very interesting effect in which high portions of the
                       map are lighter in color than lower portions, giving a
                       nice touch of depth.
*******************************************************************************/
void scenery_module::build_overlays()
{
    int x, z;
    float x_val, z_val;
    GLuint overlay_id;
    GLuint border_id;
    bool tiled_west;
    bool tiled_east;
    bool tiled_north;
    bool tiled_south;
    
    for(z = 0; z < ta_height; z++)
        for(x = 0; x < ta_width; x++)
        {
            switch(tile[x][z].tilemap_ptr->tile_type)
            {
                case TT_WHEAT_FIELD:
                    overlay_id = textures.getTextureID("Scenery/wheat_overlay.png");
                    border_id = sc_textures[TT_WHEAT_FIELD][1];
                    break;
                
                case TT_CORN_FIELD:
                    overlay_id = textures.getTextureID("Scenery/corn_overlay.png");
                    border_id = sc_textures[TT_CORN_FIELD][1];
                    break;
                
                case TT_VINEYARD:
                    overlay_id = textures.getTextureID("Scenery/vine_overlay.png");
                    border_id = sc_textures[TT_VINEYARD][1];
                    break;
                
                default:
                    overlay_id = TEXTURE_NULL;
                    border_id = TEXTURE_NULL;
                    break;
            }
            
            // If an overlay/border pair were assigned, draw such.
            if(overlay_id != TEXTURE_NULL && border_id != TEXTURE_NULL)
            {
                // Determine bordering tiles
                tiled_west = (x <= 0 || tile[x-1][z].tilemap_ptr->tile_type ==
                                        tile[x][z].tilemap_ptr->tile_type);
                tiled_east = (x+1 >= ta_width || tile[x+1][z].tilemap_ptr->tile_type ==
                                                 tile[x][z].tilemap_ptr->tile_type);
                tiled_north = (z <= 0 || tile[x][z-1].tilemap_ptr->tile_type ==
                                         tile[x][z].tilemap_ptr->tile_type);
                tiled_south = (z+1 >= ta_height || tile[x][z+1].tilemap_ptr->tile_type ==
                                                   tile[x][z].tilemap_ptr->tile_type);
                
                // Create a display list for this overlay
                tile[x][z].overlay_dspList = glGenLists(1);
                glNewList(tile[x][z].overlay_dspList, GL_COMPILE);
                
                // Bind an initial texture for the initial row
                glBindTexture(GL_TEXTURE_2D, overlay_id);
                
                // Start overlay
                glBegin(GL_TRIANGLE_STRIP);
                
                    // Set the normal vector for this upper edge to 0th trisect.
                    glNormal3fv(&tile[x][z].plane[0]);
                    
                    // Vertex 0 (North West)
                    glTexCoord2f(0.0, 1.0);
                    x_val = (x * tile_size) + (!tiled_west ? 0.85 : 0.0);
                    z_val = (z * tile_size) + (!tiled_north ? 0.85 : 0.0);
                    glVertex3f(x_val, getHeight(x_val, z_val) + 0.6, z_val);
                    
                    // Vertex 1 (North East)
                    glTexCoord2f(1.0, 1.0);
                    x_val = ((x+1) * tile_size) - (!tiled_east ? 0.85 : 0.0);
                    z_val = (z * tile_size) + (!tiled_north ? 0.85 : 0.0);
                    glVertex3f(x_val, getHeight(x_val, z_val) + 0.6, z_val);
                    
                    // Set the normal vector for this lower edge to 1st trisect.
                    glNormal3fv(&tile[x][z].plane[4]);
                    
                    // Vertex 2 (South West)
                    glTexCoord2f(0.0, 0.0);
                    x_val = (x * tile_size) + (!tiled_west ? 0.85 : 0.0);
                    z_val = ((z+1) * tile_size) - (!tiled_south ? 0.85 : 0.0);
                    glVertex3f(x_val, getHeight(x_val, z_val) + 0.6, z_val);
                    
                    // Vertex 3 (South East)
                    glTexCoord2f(1.0, 0.0);
                    x_val = ((x+1) * tile_size) - (!tiled_east ? 0.85 : 0.0);
                    z_val = ((z+1) * tile_size) - (!tiled_south ? 0.85 : 0.0);
                    glVertex3f(x_val, getHeight(x_val, z_val) + 0.6, z_val);
                
                glEnd();
                
                glBindTexture(GL_TEXTURE_2D, border_id);
                
                // Build west overlay bordering
                if(!tiled_west)
                {
                    glBegin(GL_QUADS);
                        glNormal3f(-1.0, 0.0, 0.0);
                        
                        glTexCoord2f(0.1, 0.0);
                        x_val = (x * tile_size) + 1.0;
                        z_val = (z * tile_size) + (!tiled_north ? 1.0 : 0.0);
                        glVertex3f(x_val, getHeight(x_val, z_val), z_val);
                        
                        glTexCoord2f(0.9, 0.0);
                        //x_val = (x * tile_size) + 1.0;
                        z_val = ((z+1) * tile_size) - (!tiled_south ? 1.0 : 0.0);
                        glVertex3f(x_val, getHeight(x_val, z_val), z_val);
                        
                        glTexCoord2f(0.9, 1.0);
                        x_val = (x * tile_size) + 0.75;
                        z_val = ((z+1) * tile_size) - (!tiled_south ? 0.75 : 0.0);
                        glVertex3f(x_val, getHeight(x_val, z_val) + 1.0, z_val);
                        
                        glTexCoord2f(0.1, 1.0);
                        //x_val = (x * tile_size) + 0.75;
                        z_val = (z * tile_size) + (!tiled_north ? 0.75 : 0.0);
                        glVertex3f(x_val, getHeight(x_val, z_val) + 1.0, z_val);
                    glEnd();
                }
                // Build east overlay bordering
                if(!tiled_east)
                {
                    glBegin(GL_QUADS);
                        glNormal3f(1.0, 0.0, 0.0);
                        
                        glTexCoord2f(0.1, 0.0);
                        x_val = ((x+1) * tile_size) - 1.0;
                        z_val = (z * tile_size) + (!tiled_north ? 1.0 : 0.0);
                        glVertex3f(x_val, getHeight(x_val, z_val), z_val);
                        
                        glTexCoord2f(0.9, 0.0);
                        //x_val = ((x+1) * tile_size) - 1.0;
                        z_val = ((z+1) * tile_size) - (!tiled_south ? 1.0 : 0.0);
                        glVertex3f(x_val, getHeight(x_val, z_val), z_val);
                        
                        glTexCoord2f(0.9, 1.0);
                        x_val = ((x+1) * tile_size) - 0.75;
                        z_val = ((z+1) * tile_size) - (!tiled_south ? 0.75 : 0.0);
                        glVertex3f(x_val, getHeight(x_val, z_val) + 1.0, z_val);
                        
                        glTexCoord2f(0.1, 1.0);
                        //x_val = ((x+1) * tile_size) - 0.75;
                        z_val = (z * tile_size) + (!tiled_north ? 0.75 : 0.0);
                        glVertex3f(x_val, getHeight(x_val, z_val) + 1.0, z_val);
                    glEnd();
                }
                // Build north overlay bordering
                if(!tiled_north)
                {    
                    glBegin(GL_QUADS);
                        glNormal3f(0.0, 0.0, -1.0);
                        
                        glTexCoord2f(0.1, 0.0);
                        x_val = (x * tile_size) + (!tiled_west ? 1.0 : 0.0);
                        z_val = (z * tile_size) + 1.0;
                        glVertex3f(x_val, getHeight(x_val, z_val), z_val);
                        
                        glTexCoord2f(0.9, 0.0);
                        x_val = ((x+1) * tile_size) - (!tiled_east ? 1.0 : 0.0);
                        //z_val = (z * tile_size) + 1.0;
                        glVertex3f(x_val, getHeight(x_val, z_val), z_val);
                        
                        glTexCoord2f(0.9, 1.0);
                        x_val = ((x+1) * tile_size) - (!tiled_east ? 0.75 : 0.0);
                        z_val = (z * tile_size) + 0.75;
                        glVertex3f(x_val, getHeight(x_val, z_val) + 1.0, z_val);
                        
                        glTexCoord2f(0.1, 1.0);
                        x_val = (x * tile_size) + (!tiled_west ? 0.75 : 0.0);
                        //z_val = (z * tile_size) + 0.75;
                        glVertex3f(x_val, getHeight(x_val, z_val) + 1.0, z_val);
                    glEnd();
                }
                // Build south overlay bordering
                if(!tiled_south)
                {
                    glBegin(GL_QUADS);
                        glNormal3f(0.0, 0.0, 1.0);
                        
                        glTexCoord2f(0.1, 0.0);
                        x_val = (x * tile_size) + (!tiled_west ? 1.0 : 0.0);
                        z_val = ((z+1) * tile_size) - 1.0;
                        glVertex3f(x_val, getHeight(x_val, z_val), z_val);
                        
                        glTexCoord2f(0.9, 0.0);
                        x_val = ((x+1) * tile_size) - (!tiled_east ? 1.0 : 0.0);
                        //z_val = ((z+1) * tile_size) - 1.0;
                        glVertex3f(x_val, getHeight(x_val, z_val), z_val);
                        
                        glTexCoord2f(0.9, 1.0);
                        x_val = ((x+1) * tile_size) - (!tiled_east ? 0.75 : 0.0);
                        z_val = ((z+1) * tile_size) - 0.75;
                        glVertex3f(x_val, getHeight(x_val, z_val) + 1.0, z_val);
                        
                        glTexCoord2f(0.1, 1.0);
                        x_val = (x * tile_size) + (!tiled_west ? 0.75 : 0.0);
                        //z_val = ((z+1) * tile_size) - 0.75;
                        glVertex3f(x_val, getHeight(x_val, z_val) + 1.0, z_val);
                    glEnd();
                }
                
                glEndList();
            }
        }
}

/*******************************************************************************
    function    :   scenery_module::build_skybox
    arguments   :   <none>
    purpose     :   Builds the skybox and ground texture that fits around the
                    scenery heightmap.
    notes       :   The skybox will be an ellipsoid cylinder object based on the
                    size of the map width and height.
*******************************************************************************/
void scenery_module::build_skybox()
{
    float increment = 360.0 / 15.0;     // Number of cuts (360 / cut#)
    float degree = 0.0;
    float radius = 2.47 * (map_width > map_height ? map_width : map_height);
    
    // Set texlib to fail if texture is not already pre-loaded. This is very
    // important since we're inside of a display list and there are GL commands
    // called to load textures - those commands will be repeated and will mess
    // everything up if called inside the display list.
    textures.setLoadOTF(false);
    
    /* Base Ground Build */
    
    // Create a display list
    ground_dspList = glGenLists(1);
    glNewList(ground_dspList, GL_COMPILE);
    
    // All operations here-in use a basic normal vector pointing up to LIGHT0.
    glNormal3f(0.0, 1.0, 0.0);
    
    // Build Ground - uses pre-determined multipliers
    glBindTexture(GL_TEXTURE_2D, textures.getTextureID("Scenery/ground.jpg"));
    glBegin(GL_QUADS);
        // Bottom North West
        glTexCoord2f(0.0, 1.0);
        glVertex3f((map_width / 2.0) - (map_width * 3.5), 0.0,
                   (map_height / 2.0) - (map_height * 3.5));
        // Bottom North East
        glTexCoord2f(1.0, 1.0);
        glVertex3f((map_width / 2.0) + (map_width * 3.5), 0.0,
                   (map_height / 2.0) - (map_height * 3.5));
        
        // Bottom South East
        glTexCoord2f(1.0, 0.0);
        glVertex3f((map_width / 2.0) + (map_width * 3.5), 0.0,
                   (map_height / 2.0) + (map_height * 3.5));
        
        // Bottom South West
        glTexCoord2f(0.0, 0.0);
        glVertex3f((map_width / 2.0) - (map_width * 3.5), 0.0,
                   (map_height / 2.0) + (map_height * 3.5));
    glEnd();
    glEndList();
    
    /* Sky Build */
    
    // Create a display list
    skybox_dspList = glGenLists(1);
    glNewList(skybox_dspList, GL_COMPILE);
    
    // Note: Skybox uses no materials (is decaled)
    
    // Build Sky Box
    glBindTexture(GL_TEXTURE_2D, textures.getTextureID("Scenery/skybox.png"));
    glBegin(GL_TRIANGLE_STRIP);
        // Start at 0 degrees, go in increments until 360 degrees is hit. Note
        // that we will be creating an ellipsoid to fit around the map. The
        // sin and cos functions are basic mathematics and will create our
        // points for the edges of the skybox.
        for(degree = 0.0; degree <= 360.0; degree += increment)
        {
            glTexCoord2f(degree / 360.0, 1.0);
            glVertex3f(cos(degree * degToRad) * radius
                            * ((2.47 * map_width) / radius),
                       1500.0,
                       (sin(degree * degToRad) * radius
                            * ((2.47 * map_height) / radius)) +
                            (degree >= 360.0 ? 10.0 : 0.0));
            glTexCoord2f(degree / 360.0, 0.0);
            glVertex3f(cos(degree * degToRad) * radius
                            * ((2.47 * map_width) / radius),
                       0.0,
                       (sin(degree * degToRad) * radius
                            * ((2.47 * map_height) / radius)) +
                            (degree >= 360.0 ? 10.0 : 0.0));
        }
    glEnd();
    glEndList();
    
    // Reset texlib loading methods to default
    textures.setLoadOTF();
}

/*******************************************************************************
    function    :   scenery_module::build_trees
    arguments   :   <none>
    purpose     :   Builds the advanced tree objects that appear on the map.
    notes       :   1) Uses a multipass lottery system to determine which tiles
                       get trees assigned to them, up to a max number on both
                       a per tile and per global basis.
                    2) Regardless of the position of tree tiles in the tile
                       map, spare trees map to position 0, dense to 1, and
                       pine to 2 (refering to array referencing).
*******************************************************************************/
void scenery_module::build_trees()
{
    int i, j, x, z;
    int tree_tiles[3] = {0, 0, 0};
    int tree_tiles_left[3];
    int** tile_placement[3];
    int chosen[2];
    tree_object* tree;
    tree_object* curr;
    tree_object* g_tail = NULL;
    float proximity, curr_proximity;
    char buffer[128];
    char model[16];
    int modlib_id;
    char* temp;
    
    // Count the # of tree tiles which exist on the map
    for(z = 0; z < ta_height; z++)
        for(x = 0; x < ta_width; x++)
        {
            if(tile[x][z].tilemap_ptr->tile_type == TT_SPARSE_TREES)
                tree_tiles[0]++;
            else if(tile[x][z].tilemap_ptr->tile_type == TT_DENSE_TREES)
                tree_tiles[1]++;
            else if(tile[x][z].tilemap_ptr->tile_type == TT_PINE_TREES)
                tree_tiles[2]++;
        }
    
    // No trees on map? Possible, but unlikely.
    if(tree_tiles[0] + tree_tiles[1] + tree_tiles[2] == 0)
        return;
    
    // Create the list of tree tile placements
    for(i = 0; i < 3; i++)
    {
        tile_placement[i] = new int*[tree_tiles[i]];
        for(j = 0; j < tree_tiles[i]; j++)
            tile_placement[i][j] = new int[3];
    }
    
    // Assign tree tiles
    tree_tiles[0] = tree_tiles[1] = tree_tiles[2] = 0;
    for(z = 0; z < ta_height; z++)
        for(x = 0; x < ta_width; x++)
        {
            if(tile[x][z].tilemap_ptr->tile_type == TT_SPARSE_TREES)
            {
                tile_placement[0][tree_tiles[0]][0] = x;       // -> x pos
                tile_placement[0][tree_tiles[0]][1] = z;       // -> z pos
                tile_placement[0][tree_tiles[0]][2] = 0;       // -> # trees
                tree_tiles[0]++;
            }
            else if(tile[x][z].tilemap_ptr->tile_type == TT_DENSE_TREES)
            {
                tile_placement[1][tree_tiles[1]][0] = x;       // -> x pos
                tile_placement[1][tree_tiles[1]][1] = z;       // -> z pos
                tile_placement[1][tree_tiles[1]][2] = 0;       // -> # trees
                tree_tiles[1]++;
            }
            else if(tile[x][z].tilemap_ptr->tile_type == TT_PINE_TREES)
            {
                tile_placement[2][tree_tiles[2]][0] = x;       // -> x pos
                tile_placement[2][tree_tiles[2]][1] = z;       // -> z pos
                tile_placement[2][tree_tiles[2]][2] = 0;       // -> # trees
                tree_tiles[2]++;
            }
        }
    
    // Start lottery system
    tree_tiles_left[0] = tree_tiles[0];
    tree_tiles_left[1] = tree_tiles[1];
    tree_tiles_left[2] = tree_tiles[2];
    for(i = 0; i < SC_MAX_TREES_GLOBAL &&
        (tree_tiles_left[0] + tree_tiles_left[1] + tree_tiles_left[2]) > 0;
        i++)
    {
        // Choose victim list
        if(tree_tiles_left[0] > 0 && tree_tiles_left[1] > 0 && tree_tiles_left[2] > 0)
        {
            if(roll(0.20))
                chosen[0] = 0;
            else if(roll(0.45))
                chosen[0] = 1;
            else
                chosen[0] = 2;
        }
        else if(tree_tiles_left[0] > 0 && tree_tiles_left[1] > 0 && tree_tiles_left[2] == 0)
        {
            if(roll(0.20 / 0.65))
                chosen[0] = 0;
            else
                chosen[0] = 1;
        }
        else if(tree_tiles_left[0] > 0 && tree_tiles_left[1] == 0 && tree_tiles_left[2] > 0)
        {
            if(roll(0.20 / 0.55))
                chosen[0] = 0;
            else
                chosen[0] = 2;
        }
        else if(tree_tiles_left[0] == 0 && tree_tiles_left[1] > 0 && tree_tiles_left[2] > 0)
        {
            if(roll(0.40 / 0.75))
                chosen[0] = 1;
            else
                chosen[0] = 2;
        }
        else if(tree_tiles_left[0] > 0 && tree_tiles_left[1] == 0 && tree_tiles_left[2] == 0)
        {
            chosen[0] = 0;
        }
        else if(tree_tiles_left[0] == 0 && tree_tiles_left[1] > 0 && tree_tiles_left[2] == 0)
        {
            chosen[0] = 1;
        }
        else if(tree_tiles_left[0] == 0 && tree_tiles_left[1] == 0 && tree_tiles_left[2] > 0)
        {
            chosen[0] = 2;
        }
        
        // Choose victim tile
        chosen[1] = rand() % tree_tiles_left[chosen[0]];
        
        // For ease
        x = tile_placement[chosen[0]][chosen[1]][0];
        z = tile_placement[chosen[0]][chosen[1]][1];
        
        // Increment tree count
        tile_placement[chosen[0]][chosen[1]][2]++;
        
        // Create new tree object
        tree = new tree_object;
        
        // Set up global values
        
        // Assign position - making sure we don't get too close to other trees.
        do
        {
            // Randomize a position, assuring that it is on the tile data
            do
            {
                tree->pos[0] = ((float)rand() / (float)RAND_MAX);
                tree->pos[2] = ((float)rand() / (float)RAND_MAX);
            } while(!on_tile(tile[x][z].tilemap_ptr->tile_type,
                             tile[x][z].tilemap_ptr->tile_data,
                             tree->pos[0], tree->pos[2]));
            
            // Finalize tree position
            tree->pos[0] = tile[x][z].pos[0] + (tile_size * tree->pos[0]);
            tree->pos[2] = tile[x][z].pos[2] + (tile_size * tree->pos[2]);
            
            // Check for proximity
            proximity = sqrt((tile_size + tile_size) * (tile_size + tile_size));
            curr = tile[x][z].tol_head;
            while(curr)
            {
                curr_proximity = sqrt(((curr->pos[0] - tree->pos[0]) *
                                        (curr->pos[0] - tree->pos[0])) +
                                      ((curr->pos[2] - tree->pos[2]) *
                                        (curr->pos[2] - tree->pos[2])));
                if(curr_proximity < proximity)
                    proximity = curr_proximity;
                
                curr = curr->t_next;
            }
        } while(proximity <= (tile_size / 3.0f));
        
        tree->pos[1] = getHeight(tree->pos[0], tree->pos[2]);
        tree->dir[0] = tree->dir[1] = 0.0;
        
        // Set up parsec pointer for tree object
        tree->parsec_ptr = tile[x][z].parsec_ptr;
        
        // Randomly pick some theta value to start off for the shear
        tree->theta = ((float)rand() / (float)RAND_MAX) * TWOPI;
        
        // Set alpha to full bb none f3d (initially)
        tree->bb_alpha = 1.0;
        tree->f3d_alpha = 0.0;
        
        // Build model name
        if(chosen[0] == 0 || chosen[0] == 1)
        {
            if(season == SC_SEASON_FALL)
                sprintf(model, "SC_A_Tree_%i", choose(4));  // Autumn tree
            else
                sprintf(model, "SC_S_Tree_%i", choose(4));  // Std. tree
        }
        else
            sprintf(model, "SC_P_Tree_%i", choose(4));      // Pine tree
        
        // Determine if we need to built model or not
        if(db.query(model, "BUILT") == NULL)
        {
            // Load model
            modlib_id = models.loadModel(model);
            
            // Set texture ID to first mesh
            tree->texture_id = models.getTextureID(modlib_id, 0);
            
            // Build 2D (bb) version of this tree
            tree->bb_dspList = glGenLists(1);
            glNewList(tree->bb_dspList, GL_COMPILE);
            glBindTexture(GL_TEXTURE_2D, tree->texture_id);
            glBegin(GL_QUADS);
                glNormal3f(0.0, 0.0, 1.0);
                glTexCoord2f(0.0, 0.0);
                glVertex3f(-0.5, 0.0, 0.0);
                glTexCoord2f(1.0, 0.0);
                glVertex3f(0.5, 0.0, 0.0);
                glTexCoord2f(1.0, 1.0);
                glVertex3f(0.5, 1.0, 0.0);
                glTexCoord2f(0.0, 1.0);
                glVertex3f(-0.5, 1.0, 0.0);
            glEnd();
            glEndList();
            sprintf(buffer, "%i", (int)tree->bb_dspList);       // Store in DB
            db.insert(model, "BB_DSPLIST", buffer);
            
            // Build 3D (f3d) version of this tree
            tree->f3d_dspList = glGenLists(1);
            glNewList(tree->f3d_dspList, GL_COMPILE);
            models.drawModel(modlib_id, MDL_DRW_IMMEDIATE_NO_MATERIAL);
            glEndList();
            sprintf(buffer, "%i", (int)tree->f3d_dspList);      // Store in DB
            db.insert(model, "F3D_DSPLIST", buffer);
            
            // Add identifier to DB that object is built
            db.insert(model, "BUILT", "T");
        }
        else
        {
            // Grab model library ID
            modlib_id = models.getModelID(model);
            
            // Grab texture ID for tree graphic (from first mesh)
            tree->texture_id = models.getTextureID(modlib_id, 0);
            
            // Grab 2D version of tree
            temp = db.query(model, "BB_DSPLIST");
            if(temp) tree->bb_dspList = atoi(temp);
            
            // Grab 3D version of tree
            temp = db.query(model, "F3D_DSPLIST");
            if(temp) tree->f3d_dspList = atoi(temp);
        }
        
        // Grab scaling factor
        temp = db.query(model, "SCALING");
        if(temp)
        {
            if(sscanf(temp, "%f %f %f", &(tree->scale[0]), &(tree->scale[1]), &(tree->scale[2])) == 2)
                tree->scale[2] = tree->scale[0] * 0.55f;
        }
        else
        {
            // Set up scaling factor from F3D version
            tree->scale[0] = (models.getMaxSize(modlib_id))[0] * 2.5f;
            tree->scale[1] = (models.getMaxSize(modlib_id))[1];
        }
        
        // Grab LPBB
        temp = db.query(model, "LPBB_COUNT");
        if(temp)
            tree->lpbb_count = atoi(temp);
        else
            tree->lpbb_count = 0;
        
        // Set up LPBB's
        if(tree->lpbb_count > 0)
        {
            // Allocate memory for the LPBBs and set texture ID
            tree->lpbb_pos = new float*[tree->lpbb_count];
            tree->lpbb_texture = sc_textures[TT_SPARSE_TREES][
                        choose(sc_textures[TT_SPARSE_TREES][0])];
            
            // Set up each LPBB
            for(j = 0; j < tree->lpbb_count; j++)
            {
                tree->lpbb_pos[j] = new float[3];   // Allocate for position
                
                // Grab position of LPBB
                sprintf(buffer, "LPBB_%i_POS", j+1);
                temp = db.query(model, buffer);
                if(temp)
                {
                    // Set up position
                    sscanf(temp, "%f %f %f", &(tree->lpbb_pos[j][0]),
                        &(tree->lpbb_pos[j][1]), &(tree->lpbb_pos[j][2]));
                }
                else
                {
                    // Write out error
                    sprintf(buffer, "Scenery: LPBB #%i not defined for \"%s\".",
                        j+1, model);
                    write_error(buffer);
                    
                    // Set default parameters
                    tree->lpbb_pos[j][0] = tree->lpbb_pos[j][1] = 
                        tree->lpbb_pos[j][2] = 0.0;
                    tree->lpbb_texture = TEXTURE_NULL;
                }
            }
        }
        else
        {
            tree->lpbb_pos = NULL;
            tree->lpbb_texture = TEXTURE_NULL;
        }
        
        // Add tree to linked lists
        if(g_tail != NULL)
            g_tail->g_next = tree;
        else
            tol_head = tree;
        tree->g_next = NULL;
        g_tail = tree;
        tree->t_next = tile[x][z].tol_head;     // Tile/Locally
        tile[x][z].tol_head = tree;
        
        // Remove from list if satisfied tree tile max
        if(tile_placement[chosen[0]][chosen[1]][2] >= SC_MAX_TREES_TILE ||
           (chosen[0] == 0 && tile_placement[chosen[0]][chosen[1]][2] >= 2))
        {
            tree_tiles_left[chosen[0]]--;
            tile_placement[chosen[0]][chosen[1]][0] =
                tile_placement[chosen[0]][tree_tiles_left[chosen[0]]][0];
            tile_placement[chosen[0]][chosen[1]][1] =
                tile_placement[chosen[0]][tree_tiles_left[chosen[0]]][1];
            tile_placement[chosen[0]][chosen[1]][2] =
                tile_placement[chosen[0]][tree_tiles_left[chosen[0]]][2];
        }
        
        // & finally increment our counter
        tree_count++;
    }
    
    // Clean up used memory
    for(i = 0; i < 3; i++)
    {
        for(j = 0; j < tree_tiles[i]; j++)
            delete tile_placement[i][j];
        delete tile_placement[i];
    }
}

/*******************************************************************************
    function    :   scenery_module::build_bridges
    arguments   :   <none>
    purpose     :   Builds bridge objects.
    notes       :   1) Bridge objects are based on tile connectors. One tile
                       initially says "start spanning bridge" until another
                       special tile is hit which will stop the span and the
                       bridge will correspondingly be built.
                    2) The connectors are in the form of a paved or dirt road.
                       Dirt roads will create a wooden bridge where as paved
                       roads will create a stone bridge.
                    3) Bridge data will essentially override heightmap data at
                       certain positions, defined by it's AABB, in the height
                       routine getOverlayHeigth.
*******************************************************************************/
void scenery_module::build_bridges()
{
    int x, z;
    int i = 0, j;
    int bridge_num = 0;
    float y_size = 0.0;                 // y_size of bridge sides
    GLuint side_id;                     // TxID for bridge sides
    GLuint road_id;                     // TxID for bridge road way
    float x_min = 0.0, x_max = 0.0;     // For ease of coding
    float z_min = 0.0, z_max = 0.0;     // "
    float height = 0.0;                 // "
    int tile_data;                      // "
    int length = 0;                     // "
    bool rotate = false;                // "
    char buffer[128];
    bridge_object* curr;
    float lip = 0.65;                   // Lip of sides on bridge
    float bridge_offset[4] = {0.24, 0.2, 0.22, 0.22};   // XZmin/max TS offset
    
    // Go through our map and assign bridge data to any tile which is defined
    // as a bridge connector.
    for(z = 0; z < ta_height; z++)
        for(x = 0; x < ta_width; x++)
        {
            // Bridge connectors will have a TT of either a dirt road or
            // paved road, however, they will have something other than 0
            // for the TD value.
            if(tile[x][z].tilemap_ptr->tile_type == TT_DIRT_ROAD ||
                tile[x][z].tilemap_ptr->tile_type == TT_PAVED_ROAD)
            {
                switch((tile_data = tile[x][z].tilemap_ptr->tile_data))
                {
                    case TD_BRIDGE_1_X_START:
                    case TD_BRIDGE_2_X_START:
                    case TD_BRIDGE_3_X_START:
                    case TD_BRIDGE_4_X_START:
                        // Bridge Connector from West to East
                        
                        // See how long the bridge is - go until we hit a
                        // tile_data of +1 or edge of map.
                        for(i = x; i < ta_width; i++)
                            if(tile[i][z].tilemap_ptr->tile_data == tile_data + 1)
                                break;
                        
                        // Bounds check (bridge must not land on a ta_width)
                        if(i >= ta_width)
                            continue;   // Skip to next scenery element
                        
                        // Allocate new bridge object and add into bridge list
                        curr = new bridge_object;
                        curr->next = bol_head;
                        bol_head = curr;
                        
                        // Add bridge into tile list (span it across the scenery).
                        for(j = x; j <= i; j++)
                            tile[j][z].bridge_ptr = curr;
                        
                        // Define length of bridge
                        length = (i - x) + 1;       // Off by 1
                        
                        // Set bridge type style based on tile_data
                        bridge_num = ((tile_data - TD_BRIDGE_1_X_START) / 4) + 1;
                        
                        // Set texture rotation (for bridges which go W/E instead N/S)
                        rotate = true;
                        
                        // Set up data for bridge - used for overriding the
                        // heightmap data in getHeight (AABB).
                        x_min = curr->x_min = x * tile_size;
                        x_max = curr->x_max = (i + 1) * tile_size;
                        z_min = curr->z_min = (z * tile_size)
                            + (bridge_offset[bridge_num-1] * tile_size);
                        z_max = curr->z_max = ((z+1) * tile_size)
                            - (bridge_offset[bridge_num-1] * tile_size);
                        
                        // Define height of bridge, which is basically the avg.
                        // of the height in the center of the two connecting
                        // tiles. This could be off though if the tiles are
                        // too off centered, but that is more left up to the
                        // map designer to make sure of being accurate or not.
                        height = curr->height =
                            (((heightmap[x][z] + heightmap[x+1][z]
                            + heightmap[x][z+1] + heightmap[x+1][z+1])
                            / 4.0) +
                            ((heightmap[i][z] + heightmap[i+1][z]
                            + heightmap[i][z+1] + heightmap[i+1][z+1])
                            / 4.0)) / 2.0;
                        break;
                        
                    case TD_BRIDGE_1_Z_START:
                    case TD_BRIDGE_2_Z_START:
                    case TD_BRIDGE_3_Z_START:
                    case TD_BRIDGE_4_Z_START:
                        // Bridge Connector from North to South
                        
                        // See how long the bridge is - go until we hit a
                        // tile_data of +1 or edge of map.
                        for(i = z; i < ta_height; i++)
                            if(tile[x][i].tilemap_ptr->tile_data == tile_data + 1)
                                break;
                        
                        // Bounds check (bridge must not land on a ta_height)
                        if(i >= ta_height)
                            continue;   // Skip to next scenery element
                        
                        // Allocate new bridge object and add into bridge list
                        curr = new bridge_object;
                        curr->next = bol_head;
                        bol_head = curr;
                        
                        // Add bridge into tile list (span it across the scenery).
                        for(j = z; j <= i; j++)
                            tile[x][j].bridge_ptr = curr;
                        
                        // Define length of bridge
                        length = (i - z) + 1;       // Off by 1 otherwise
                        
                        // Set bridge type style based on tile_data
                        bridge_num = ((tile_data - TD_BRIDGE_1_X_START) / 4) + 1;
                        
                        // Set texture rotation (for bridges which go W/E instead N/S)
                        rotate = false;
                        
                        // Set up data for bridge - used for overriding the
                        // heightmap data in getHeight (AABB definition).
                        x_min = curr->x_min = (x * tile_size)
                            + (bridge_offset[bridge_num-1] * tile_size);
                        x_max = curr->x_max = ((x+1) * tile_size)
                            - (bridge_offset[bridge_num-1] * tile_size);
                        z_min = curr->z_min = z * tile_size;
                        z_max = curr->z_max = (i + 1) * tile_size;
                        
                        // Define height of bridge, which is basically the avg.
                        // of the height in the center of the two connecting
                        // tiles. This could be off though if the tiles are
                        // too off centered, but that is more left up to the
                        // map designer to make sure of being accurate or not.
                        height = curr->height =
                            (((heightmap[x][z] + heightmap[x+1][z]
                            + heightmap[x][z+1] + heightmap[x+1][z+1])
                            / 4.0) +
                            ((heightmap[x][i] + heightmap[x+1][i]
                            + heightmap[x][i+1] + heightmap[x+1][i+1])
                            / 4.0)) / 2.0;
                        break;
                    
                    default:
                        continue;   // Skip to next scenery element
                        break;
                }
                
                // The continue clause of default should skip past this part
                // which only applies to tiles which are bridge connectors.
                
                // Set up data for culling (center position of object)
                curr->pos[0] = (x_min + x_max) / 2.0;
                curr->pos[2] = (z_min + z_max) / 2.0;
                
                // Define the y size of the bridge - this is basically the
                // height of the bridge minus the height of the heightmap
                // at the center position, of which was just calculated.
                y_size = height - getHeight(curr->pos[0], curr->pos[2]);
                
                // Define y position of center (easier done with y_size defined)
                curr->pos[1] = height - (y_size / 2.0);
                
                // Define radius of bridge (for frustum culling)
                curr->radius = sqrt(
                    powf(curr->pos[0] - x_min, 2.0) +
                    powf(curr->pos[1] - height, 2.0) +
                    powf(curr->pos[2] - z_min, 2.0));
                
                // Initially set to draw
                curr->draw = true;
                
                /*  Bridge Building Routine  */
                
                // Assign texture IDs
                sprintf(buffer, "Scenery/bridge_%i_sides.png", bridge_num);
                side_id = textures.getTextureID(buffer);
                
                // Load the road texture using a nice linear filter.
                sprintf(buffer, "Scenery/bridge_%i_road.png", bridge_num);
                road_id = textures.getTextureID(buffer);
                
                // Create display lists for bridge
                curr->dspList = glGenLists(1);
                glNewList(curr->dspList, GL_COMPILE);
                
                // Build sides of bridge.
                glBindTexture(GL_TEXTURE_2D, side_id);
                
                if(rotate)
                {
                    // North Side
                    glBegin(GL_TRIANGLE_STRIP);
                        glNormal3f(0.0, 0.0, -1.0);
                        // Bottom North West
                        glTexCoord2f(0.0, 0.0);
                        glVertex3f(x_min, height - y_size, z_min);
                        // Bottom North East
                        glTexCoord2f(ceilf((float)length / 4.0), 0.0);
                        glVertex3f(x_max, height - y_size, z_min);
                        // Top North West
                        glTexCoord2f(0.0, 1.0);
                        glVertex3f(x_min, height + lip, z_min);
                        // Top North East
                        glTexCoord2f(ceilf((float)length / 4.0), 1.0);
                        glVertex3f(x_max, height + lip, z_min);
                        // Inner Top North West
                        glTexCoord2f(0.0, 0.922);
                        glVertex3f(x_min, height + lip, z_min + (lip / 2.5));
                        // Inner Top North East
                        glTexCoord2f(ceilf((float)length / 4.0), 0.922);
                        glVertex3f(x_max, height + lip, z_min + (lip / 2.5));
                        // Inner Road North West
                        glTexCoord2f(0.0, 1.0);
                        glVertex3f(x_min, height, z_min + (lip / 2.5));
                        // Inner Road North East
                        glTexCoord2f(ceilf((float)length / 4.0), 1.0);
                        glVertex3f(x_max, height, z_min + (lip / 2.5));
                    glEnd();
                    // South Side
                    glBegin(GL_TRIANGLE_STRIP);
                        glNormal3f(0.0, 0.0, 1.0);
                        // Bottom South West
                        glTexCoord2f(0.0, 0.0);
                        glVertex3f(x_min, height - y_size, z_max);
                        // Bottom South East
                        glTexCoord2f(ceilf((float)length / 4.0), 0.0);
                        glVertex3f(x_max, height - y_size, z_max);
                        // Top South West
                        glTexCoord2f(0.0, 1.0);
                        glVertex3f(x_min, height + lip, z_max);
                        // Top South East
                        glTexCoord2f(ceilf((float)length / 4.0), 1.0);
                        glVertex3f(x_max, height + lip, z_max);
                        // Inner Top South West
                        glTexCoord2f(0.0, 0.922);
                        glVertex3f(x_min, height + lip, z_max - (lip / 2.5));
                        // Inner Top South East
                        glTexCoord2f(ceilf((float)length / 4.0), 0.922);
                        glVertex3f(x_max, height + lip, z_max - (lip / 2.5));
                        // Inner Road South West
                        glTexCoord2f(0.0, 1.0);
                        glVertex3f(x_min, height, z_max - (lip / 2.5));
                        // Inner Road South East
                        glTexCoord2f(ceilf((float)length / 4.0), 1.0);
                        glVertex3f(x_max, height, z_max - (lip / 2.5));
                    glEnd();
                }
                else
                {
                    // West Side
                    glBegin(GL_TRIANGLE_STRIP);
                        glNormal3f(-1.0, 0.0, 0.0);
                        // Bottom North West
                        glTexCoord2f(0.0, 0.0);
                        glVertex3f(x_min, height - y_size, z_min);
                        // Bottom South West
                        glTexCoord2f(ceilf((float)length / 4.0), 0.0);
                        glVertex3f(x_min, height - y_size, z_max);
                        // Top North West
                        glTexCoord2f(0.0, 1.0);
                        glVertex3f(x_min, height + lip, z_min);
                        // Top South West
                        glTexCoord2f(ceilf((float)length / 4.0), 1.0);
                        glVertex3f(x_min, height + lip, z_max);
                        // Inner Top North West
                        glTexCoord2f(0.0, 0.922);
                        glVertex3f(x_min + (lip / 2.5), height + lip, z_min);
                        // Inner Top South West
                        glTexCoord2f(ceilf((float)length / 4.0), 0.922);
                        glVertex3f(x_min + (lip / 2.5), height + lip, z_max);
                        // Inner Road North West
                        glTexCoord2f(0.0, 1.0);
                        glVertex3f(x_min + (lip / 2.5), height, z_min);
                        // Inner Road South West
                        glTexCoord2f(ceilf((float)length / 4.0), 1.0);
                        glVertex3f(x_min + (lip / 2.5), height, z_max);
                    glEnd();
                    // East Side
                    glBegin(GL_TRIANGLE_STRIP);
                        glNormal3f(1.0, 0.0, 0.0);
                        // Bottom North East
                        glTexCoord2f(0.0, 0.0);
                        glVertex3f(x_max, height - y_size, z_min);
                        // Bottom South East
                        glTexCoord2f(ceilf((float)length / 4.0), 0.0);
                        glVertex3f(x_max, height - y_size, z_max);
                        // Top North East
                        glTexCoord2f(0.0, 1.0);
                        glVertex3f(x_max, height + lip, z_min);
                        // Top South East
                        glTexCoord2f(ceilf((float)length / 4.0), 1.0);
                        glVertex3f(x_max, height + lip, z_max);
                        // Inner Top North East
                        glTexCoord2f(0.0, 0.922);
                        glVertex3f(x_max - (lip / 2.5), height + lip, z_min);
                        // Inner Top South East
                        glTexCoord2f(ceilf((float)length / 4.0), 0.922);
                        glVertex3f(x_max - (lip / 2.5), height + lip, z_max);
                        // Inner Road North East
                        glTexCoord2f(0.0, 1.0);
                        glVertex3f(x_max - (lip / 2.5), height, z_min);
                        // Inner Road South East
                        glTexCoord2f(ceilf((float)length / 4.0), 1.0);
                        glVertex3f(x_max - (lip / 2.5), height, z_max);
                    glEnd();
                }
                
                // Build road way on top of bridge.
                glBindTexture(GL_TEXTURE_2D, road_id);
                
                glBegin(GL_QUADS);
                    glNormal3f(0.0, 1.0, 0.0);
                    // North West
                    glTexCoord2f(
                        rotate ? 1.0 : 0.0,
                        ceilf((float)length / 4.0));
                    glVertex3f(x_min, height, z_min);
                    // South West
                    glTexCoord2f(
                        0.0,
                        rotate ? ceilf((float)length / 4.0) : 0.0);
                    glVertex3f(x_min, height, z_max);
                    // South East
                    glTexCoord2f(
                        rotate ? 0.0 : 1.0,
                        0.0);
                    glVertex3f(x_max, height, z_max);
                    // North East
                    glTexCoord2f(
                        1.0,
                        rotate ? 0.0 : ceilf((float)length / 4.0));
                    glVertex3f(x_max, height, z_min);
                glEnd();
                // note: replace / 1.0 with how many tiles to go before restart-
                // ing the texture over again.
                
                glEndList();
            }
        }
}

/*******************************************************************************
    Misc. Private Routines
*******************************************************************************/

/*******************************************************************************
    function    :   bool scenery_module::on_tile
    arguments   :   x_offset, z_offset - normalized position on tile (0.0-1.0)
                    tile_data - tile data for this tile
    purpose     :   Determines whenever or not the passed normalized position
                    exists on the tile's data portion (that is - not on the
                    edges which are a part of the base texture).
    notes       :   <none>
*******************************************************************************/
bool scenery_module::on_tile(int tile_type, int tile_data,
    float x_offset, float z_offset)
{
    float offset = 0.0;
    
    switch(tile_type)
    {
        case TT_SPARSE_TREES:
        case TT_DENSE_TREES:
        case TT_PINE_TREES:
            offset = 0.039;
            break;
        
        case TT_WHEAT_FIELD:
        case TT_CORN_FIELD:
        case TT_VINEYARD:
            offset = 0.019;
            break;
        
        case TT_DIRT_ROAD:
            offset = 0.274;
            break;
        
        case TT_PAVED_ROAD:
            offset = 0.195;
            break;
        
        case TT_DIRT_PAVED_EXT:
            offset = 0.234;
            break;
        
        default:
            return true;
    }
    
    switch(tile_data)
    {
        case TD_FULL:
            return true;
        
        case TD_N_CUT:
            if(z_offset <= offset)
                return false;
            return true;
        
        case TD_E_CUT:
            if(x_offset >= 1.0 - offset)
                return false;
            return true;
            
        case TD_S_CUT:
            if(z_offset >= 1.0 - offset)
                return false;
            return true;
            
        case TD_W_CUT:
            if(x_offset <= offset)
                return false;
            return true;
            
        case TD_NS_CUT:
        case TD_BRIDGE_1_X_START:
        case TD_BRIDGE_1_X_STOP:
        case TD_BRIDGE_2_X_START:
        case TD_BRIDGE_2_X_STOP:
        case TD_BRIDGE_3_X_START:
        case TD_BRIDGE_3_X_STOP:
        case TD_BRIDGE_4_X_START:
        case TD_BRIDGE_4_X_STOP:
            if(z_offset <= offset || z_offset >= 1.0 - offset)
                return false;
            return true;
            
        case TD_WE_CUT:
        case TD_BRIDGE_1_Z_START:
        case TD_BRIDGE_1_Z_STOP:
        case TD_BRIDGE_2_Z_START:
        case TD_BRIDGE_2_Z_STOP:
        case TD_BRIDGE_3_Z_START:
        case TD_BRIDGE_3_Z_STOP:
        case TD_BRIDGE_4_Z_START:
        case TD_BRIDGE_4_Z_STOP:
            if(x_offset <= offset || x_offset >= 1.0 - offset)
                return false;
            return true;
        
        case TD_NW_CUT:
            if(x_offset <= offset || z_offset <= offset)
                return false;
            return true;
            
        case TD_NE_CUT:
            if(x_offset >= 1.0 - offset || z_offset <= offset)
                return false;
            return true;
            
        case TD_SE_CUT:
            if(x_offset >= 1.0 - offset || z_offset >= 1.0 - offset)
                return false;
            return true;
            
        case TD_SW_CUT:
            if(x_offset <= offset || z_offset >= 1.0 - offset)
                return false;
            return true;
        
        case TD_WNE_CUT:
            if(x_offset <= offset || z_offset <= offset || x_offset >= 1.0 - offset)
                return false;
            return true;
            
        case TD_NES_CUT:
            if(z_offset <= offset || x_offset >= 1.0 - offset || z_offset >= 1.0 - offset)
                return false;
            return true;
            
        case TD_ESW_CUT:
            if(x_offset >= 1.0 - offset || z_offset >= 1.0 - offset || x_offset <= offset)
                return false;
             return true;
        
        case TD_NSW_CUT:
            if(z_offset <= offset || z_offset >= 1.0 - offset || x_offset <= offset)
                return false;
            return true;
            
        case TD_NESW_CUT:
            if(z_offset <= offset || x_offset >= 1.0 - offset || z_offset >= 1.0 - offset || x_offset <= offset)
                return false;
            return true;
            
        case TD_N_CUT_SE_SW_EDGE:
            if(z_offset <= offset || (x_offset <= offset && z_offset >= 1.0 - offset) || (x_offset >= 1.0 - offset && z_offset >= 1.0 - offset))
                return false;
            return true;
        
        case TD_E_CUT_NW_SW_EDGE:
            if(x_offset >= 1.0 - offset || (x_offset <= offset && z_offset <= offset) || (x_offset <= offset && z_offset >= 1.0 - offset))
                return false;
            return true;
        
        case TD_S_CUT_NW_NE_EDGE:
            if(z_offset >= 1.0 - offset || (x_offset <= offset && z_offset <= offset) || (x_offset >= 1.0 - offset && z_offset <= offset))
                return false;
            return true;
        
        case TD_W_CUT_NE_SW_EDGE:
            if(x_offset <= offset || (x_offset >= 1.0 - offset && z_offset <= offset) || (x_offset <= offset && z_offset >= 1.0 - offset))
                return false;
            return true;
        
        case TD_NW_CUT_SE_EDGE:
            if(z_offset <= offset || x_offset <= offset || (x_offset >= 1.0 - offset && z_offset >= 1.0 - offset))
                return false;
            return true;
        
        case TD_NE_CUT_SW_EDGE:
            if(z_offset <= offset || x_offset >= 1.0 - offset || (x_offset <= offset && z_offset >= 1.0 - offset))
                return false;
            return true;
        
        case TD_SE_CUT_NW_EDGE:
            if(z_offset >= 1.0 - offset || x_offset <= offset || (x_offset <= offset && z_offset <= offset))
                return false;
            return true;
        
        case TD_SW_CUT_NE_EDGE:
            if(x_offset <= offset || z_offset >= 1.0 - offset || (x_offset >= 1.0 - offset && z_offset <= offset))
                return false;
            return true;
        
        case TD_NW_NE_SE_SW_EDGE:
            if((x_offset <= offset && z_offset <= offset) || (x_offset >= 1.0 - offset && z_offset <= offset) ||
               (x_offset <= offset && z_offset >= 1.0 - offset) || (x_offset >= 1.0 - offset && z_offset >= 1.0 - offset))
                return false;
            return true;
        
        default:
            return true;
    }
}

/*******************************************************************************
    General Public Routines
*******************************************************************************/

/*******************************************************************************
    function    :   float scenery_module::getHeight
    arguments   :   x_val, z_val - position on map
    purpose     :   Calculates the precise height of the heightmap at x,z using
                    plane equation data.
    notes       :   Does NOT provide heightmap overlays, such as bridge overlay.
*******************************************************************************/
float scenery_module::getHeight(float x_val, float z_val)
{
    int x, z;
    float x_off, z_off, height;
    
    // Bounds checking
    if(x_val < 0.0)
        x_val = 0.0;
    else if(x_val > map_width - 0.001)
        x_val = map_width - 0.001;
    
    if(z_val < 0.0)
        z_val = 0.0;
    else if(z_val > map_height - 0.001)
        z_val = map_height - 0.001;
    
    // Determine corresponding x,z tile array offset
    x_off = x_val / tile_size;
    x = (int)x_off;
    x_off = x_off - floor(x_off);
    z_off = z_val / tile_size;
    z = (int)z_off;
    z_off = z_off - floor(z_off);
    
    if(x_off <= (1.0 - z_off))
        // Use 0th Trisect
        height = (tile[x][z].plane[3] - (tile[x][z].plane[0] * x_val) -
            (tile[x][z].plane[2] * z_val)) / tile[x][z].plane[1];
    else
        // Use 1st Trisect
        height = (tile[x][z].plane[7] - (tile[x][z].plane[4] * x_val) -
            (tile[x][z].plane[6] * z_val)) / tile[x][z].plane[5];
    
    return height;
}

/*******************************************************************************
    function    :   float scenery_module::getOverlayHeight
    arguments   :   x_val, z_val - position on map
    purpose     :   Calculates the precise height of the heightmap at x,z using
                    plane equation data, providing and accounting for scenery
                    elements which overlay the heightmap (such as bridges).
    notes       :   <none>
*******************************************************************************/
float scenery_module::getOverlayHeight(float x_val, float z_val)
{
    int x, z;
    float x_off, z_off, height;
    
    // Bounds checking
    if(x_val < 0.0)
        x_val = 0.0;
    else if(x_val > map_width - 0.001)
        x_val = map_width - 0.001;
    
    if(z_val < 0.0)
        z_val = 0.0;
    else if(z_val > map_height - 0.001)
        z_val = map_height - 0.001;
    
    // Determine corresponding x,z tile array offset
    x_off = x_val / tile_size;
    x = (int)x_off;
    x_off = x_off - floor(x_off);
    z_off = z_val / tile_size;
    z = (int)z_off;
    z_off = z_off - floor(z_off);
    
    if(x_off <= (1.0 - z_off))
        // Use 0th Trisect
        height = (tile[x][z].plane[3] - (tile[x][z].plane[0] * x_val) -
            (tile[x][z].plane[2] * z_val)) / tile[x][z].plane[1];
    else
        // Use 1st Trisect
        height = (tile[x][z].plane[7] - (tile[x][z].plane[4] * x_val) -
            (tile[x][z].plane[6] * z_val)) / tile[x][z].plane[5];
    
    // Account for bridge overlay
    if(tile[x][z].bridge_ptr != NULL)
    {
        // Check against bridge's AABB
        if(x_val >= tile[x][z].bridge_ptr->x_min &&
           x_val <= tile[x][z].bridge_ptr->x_max &&
           z_val >= tile[x][z].bridge_ptr->z_min &&
           z_val <= tile[x][z].bridge_ptr->z_max &&
           height <= tile[x][z].bridge_ptr->height)
            height = tile[x][z].bridge_ptr->height;
    }
    
    return height;
}

/*******************************************************************************
    function    :   float scenery_module::getRelativeHeight
    arguments   :   x_val, z_val - position on map
    purpose     :   Rounds x and z to nearest defined height and returns that
                    value from an array look-up.
    notes       :   Does NOT provide heightmap overlays, such as bridge overlay.
*******************************************************************************/
float scenery_module::getRelativeHeight(float x_val, float z_val)
{
    int x, z;
    
    // Bounds checking
    if(x_val < 0.0)
        x_val = 0.0;
    else if(x_val > map_width - 0.001)
        x_val = map_width - 0.001;
    
    if(z_val < 0.0)
        z_val = 0.0;
    else if(z_val > map_height - 0.001)
        z_val = map_height - 0.001;
    
    // Determine corresponding x,z tile array offset (with round-off).
    x = (int)((x_val / tile_size) + 0.5);
    z = (int)((z_val / tile_size) + 0.5);
    
    // Approximation
    return heightmap[x][z];
}

/*******************************************************************************
    function    :   int scenery_module::getTileType
    arguments   :   x_val, z_val - position on map
    purpose     :   Returns the tile type of the tile that x,z corresponds to.
    notes       :   <none>
*******************************************************************************/
int scenery_module::getTileType(float x_val, float z_val)
{
    int x, z;
    float x_off, z_off;
    
    // Bounds checking
    if(x_val < 0.0)
        x_val = 0.0;
    else if(x_val > map_width - 0.001)
        x_val = map_width - 0.001;
    
    if(z_val < 0.0)
        z_val = 0.0;
    else if(z_val > map_height - 0.001)
        z_val = map_height - 0.001;
    
    // Determine corresponding x,z tile array offset
    x_off = x_val / tile_size;
    x = (int)x_off;
    x_off = x_off - floor(x_off);
    z_off = z_val / tile_size;
    z = (int)z_off;
    z_off = z_off - floor(z_off);
    
    // Determine if x_val, z_val is on tile data section or not
    if(on_tile(tile[x][z].tilemap_ptr->tile_type,
               tile[x][z].tilemap_ptr->tile_data,
               x_off, z_off))
        return tile[x][z].tilemap_ptr->tile_type;
    else
        return tilemap[0].tile_type;
}

/*******************************************************************************
    function    :   int scenery_module::getBlockmap
    arguments   :   x_val, z_val - position on map
    purpose     :   Rounds x and z to nearest tile definition and returns the
                    blockmap data at that position.
    notes       :   <none>
*******************************************************************************/
int scenery_module::getBlockmap(float x_val, float z_val)
{
    int x, z;
    float x_off, z_off;
    
    // Bounds checking
    if(x_val < 0.0)
        x_val = 0.0;
    else if(x_val > map_width - 0.001)
        x_val = map_width - 0.001;
    
    if(z_val < 0.0)
        z_val = 0.0;
    else if(z_val > map_height - 0.001)
        z_val = map_height - 0.001;
    
    // Determine corresponding x,z tile array offset
    x_off = x_val / tile_size;
    x = (int)x_off;
    x_off = x_off - floor(x_off);
    z_off = z_val / tile_size;
    z = (int)z_off;
    z_off = z_off - floor(z_off);
    
    // Account for bridge overlay
    if(tile[x][z].bridge_ptr != NULL)
    {
        // Check against bridge's AABB
        if(x_val >= tile[x][z].bridge_ptr->x_min &&
           x_val <= tile[x][z].bridge_ptr->x_max &&
           z_val >= tile[x][z].bridge_ptr->z_min &&
           z_val <= tile[x][z].bridge_ptr->z_max)
            return SC_BRIDGE_BLOCKMAP;
    }
    
    // Determine if x_val, z_val is on tile data section or not
    if(on_tile(tile[x][z].tilemap_ptr->tile_type,
               tile[x][z].tilemap_ptr->tile_data,
               x_off, z_off))
        return tile[x][z].tilemap_ptr->tile_block;
    else
        return tilemap[0].tile_block;
}

/*******************************************************************************
    Scenery Metrics Routines
*******************************************************************************/

/*******************************************************************************
    function    :   kVector scenery_module::rayIntersect
    arguments   :   rayPos - Initial position of ray
                    rayDir - Direction vector of ray
    purpose     :   Determines where a ray will intersect the heightmap.
    notes       :   1) May not be too accurate on maps which have rapidly
                       changing terrain. However, this risk with REAL terrain
                       is very minimal.
                    2) Returned value is a position vector, of which the y
                       value is the value on the ray - not of the scenery.
                    3) The position vector must have a y value which is above
                       the heightmap at the x,z point.
*******************************************************************************/
kVector scenery_module::rayIntersect(kVector rayPos, kVector rayDir)
{
    kVector curr_spot;                  // Curr spot on ray
    float iteration = tile_size;        // Iteration value
    float multiplier = 0.0;             // Current multiplier
    float max_map_size =                // Maximum map size
        sqrt((map_width * map_width) + (map_height * map_height));
    
    // First, go along our ray until we go under the scenery or go beyond
    // the map size. If we do not define a max size, rays going above the
    // heightmap will result in an infinite loop (since the ray never will
    // pass underneath the heightmap).
    do
    {
        // Add iteration value to multiplier
        multiplier += iteration;
        
        // Calculate new current spot
        curr_spot = rayPos + (rayDir * multiplier);
        
        // Check bounds
        if(curr_spot[0] < 0.0)
            curr_spot[0] = 0.0;
        else if(curr_spot[0] > map_width)
            curr_spot[0] = map_width;
        
        if(curr_spot[2] < 0.0)
            curr_spot[2] = 0.0;
        else if(curr_spot[2] > map_height)
            curr_spot[2] = map_height;
        
    } while(curr_spot[1] > getOverlayHeight(curr_spot[0], curr_spot[2]) &&
            multiplier <= max_map_size);
    
    // Second, use the binary search method to "hom"-in on the correct value
    // along the ray that passes at the heightmap. If the rayPos along the
    // ray at the given multiplier is above the heightmap, we add onto the
    // multiplier, but if it is underneath the heightmap, we subtract from
    // the multiplier. The value added/subtracted is the iteration value, which
    // is cut by half at each iteration. The end result is a multiplier along
    // the ray which is quite close to the heightmap - e.g. the intersect.
    // It is assumed that the ray starts above the heightmap.
    do
    {
        // Cut iteration value in half
        iteration /= 2.0;
        
        // Calculate new current spot
        curr_spot = rayPos + (rayDir * multiplier);
        
        // Check bounds
        if(curr_spot[0] < 0.0)
            curr_spot[0] = 0.0;
        else if(curr_spot[0] > map_width)
            curr_spot[0] = map_width;
        if(curr_spot[2] < 0.0)
            curr_spot[2] = 0.0;
        else if(curr_spot[2] > map_height)
            curr_spot[2] = map_height;
        
        // Add/subtract multiplier based on if it is below/above the heightmap.
        if(curr_spot[1] < getOverlayHeight(curr_spot[0], curr_spot[2]))
            multiplier -= iteration;
        else
            multiplier += iteration;
    }
    while(iteration >= 0.05);
    
    // Return the rayPos vector at the current rayPos, which is curr_spot
    return kVector(curr_spot());
}

/*******************************************************************************
    function    :   scenery_module::groundCollision
    arguments   :   position - position vector
    purpose     :   Determines if the position vector passed is in "collision"
                    with the ground in an efficient mannerism.
    notes       :   1) Does not test against scenery objects.
                    2) More efficient use of getRelativeHeight and getHeight.
*******************************************************************************/
bool scenery_module::groundCollision(kVector position)
{
    int x, z;
    float x_off, z_off, height;
    
    // Bounds check
    if(position[0] < 0.0)
        position[0] = 0.0;
    else if(position[0] > map_width - 0.001)
        position[0] = map_width - 0.001;
        
    if(position[2] < 0.0)
        position[2] = 0.0;
    else if(position[2] > map_height - 0.001)
        position[2] = map_height - 0.001;
    
    // Determine corresponding x,z tile array offset
    x_off = position[0] / tile_size;
    x = (int)x_off;
    z_off = position[2] / tile_size;
    z = (int)z_off;
    
    // See if it passes the first check against all edge relative heights
    if(position[1] <= heightmap[x][z] ||
       position[1] <= heightmap[x+1][z] ||
       position[1] <= heightmap[x][z+1] ||
       position[1] <= heightmap[x+1][z+1])
    {
        x_off = x_off - floor(x_off);
        z_off = z_off - floor(z_off);
        
        if(x_off <= (1.0 - z_off))
            // Use 0th Trisect
            height = (tile[x][z].plane[3] - (tile[x][z].plane[0] * position[0]) -
                (tile[x][z].plane[2] * position[2])) / tile[x][z].plane[1];
        else
            // Use 1st Trisect
            height = (tile[x][z].plane[7] - (tile[x][z].plane[4] * position[0]) -
                (tile[x][z].plane[6] * position[2])) / tile[x][z].plane[5];
        
        if(position[1] <= height + FP_ERROR)
            return true;
    }
    
    return false;
}

/*******************************************************************************
    function    :   scenery_module::sceneryCollision
    arguments   :   position - position vector
    purpose     :   Determines if the position vector passed is in "collision"
                    with any scenery overlay objects (minus billboard objects).
    notes       :   <none>
*******************************************************************************/
bool scenery_module::sceneryCollision(kVector position)
{
    int x, z;
    
    // Bounds check
    if(position[0] < 0.0)
        position[0] = 0.0;
    else if(position[0] > map_width - 0.001)
        position[0] = map_width - 0.001;
    
    if(position[2] < 0.0)
        position[2] = 0.0;
    else if(position[2] > map_height - 0.001)
        position[2] = map_height - 0.001;
    
    // Determine corresponding x,z tile array offset
    x = (int)(position[0] / tile_size);
    z = (int)(position[2] / tile_size);
    
    // Check against bridge overlay
    if(tile[x][z].bridge_ptr != NULL)
    {
        // Check against bridge's AABB
        if(position[0] >= tile[x][z].bridge_ptr->x_min &&
           position[0] <= tile[x][z].bridge_ptr->x_max &&
           position[2] >= tile[x][z].bridge_ptr->z_min &&
           position[2] <= tile[x][z].bridge_ptr->z_max &&
           position[1] <= tile[x][z].bridge_ptr->height)
            return true;
    }
    
    return false;
}

/*******************************************************************************
    Base Display and Update Routines
*******************************************************************************/

/*******************************************************************************
    function    :   scenery_module::billboard_object::display
    arguments   :   <none>
    purpose     :   Display function to draw billboard.
    notes       :   Uses width_half pre-calculation to save us a few cycles, of
                    which over a couple thousand objects is a decent save.
*******************************************************************************/
/*void scenery_module::billboard_object::display()
{
    glColor4f(1.0, 1.0, 1.0, alpha);    // Set alpha value
    
    glPushMatrix();
    
    // Translate to position and rotate object
    glTranslatef(pos[0], pos[1], pos[2]);
    
    glRotatef(rotate, 0.0, 1.0, 0.0);
    
    // Draw the square which defines the scenery object
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex3f(-width_half, 0.0, 0.0);
        
        glTexCoord2f(1.0, 0.0);
        glVertex3f(width_half, 0.0, 0.0);
        
        glTexCoord2f(1.0, 1.0);
        glVertex3f(width_half, height, 0.0);
        
        glTexCoord2f(0.0, 1.0);
        glVertex3f(-width_half, height, 0.0);
    glEnd();
    
    glPopMatrix();
}*/

/*******************************************************************************
    function    :   scenery_module::update
    arguments   :   deltaT - time difference since last update
    purpose     :   Updates scenery objects, including 
    notes       :   Called once every so many milliseconds to update scenery
                    objects, cull objects, etc.
*******************************************************************************/
void scenery_module::update(float deltaT)
{
    static float* cam_pos = camera.getCamPos();
    int i, x, z;
    scenery_element* sel_curr;
    scenery_element* sel_prev;
    bridge_object* bol_curr;
    tree_object* tol_curr;
    int se_want;
    float se_ratio;
    bool se_updated;
    
    // Update parsec culling
    for(i = 0; i < parsec_count; i++)
        parsec[i].draw =
            camera.sphereInView(parsec[i].pos, parsec[i].radius);

    // Update bridges culling
    bol_curr = bol_head;
    while(bol_curr)
    {
        bol_curr->draw = camera.sphereInView(bol_curr->pos, bol_curr->radius);
        bol_curr = bol_curr->next;
    }
    
    // Update water texture movement (gives a smooth flowing effect)
    water_texture_offset += (0.0025) * deltaT;
    while(water_texture_offset >= 1.0)    // Normalize between 0.0 and 1.0
        water_texture_offset -= 1.0;
    
    // Update water height offset
    water_height_offset = 0.15 * cos(water_texture_offset * 20.0 * TWOPI);
    
    // Update tiles, SE's, & trees
    for(z = 0; z < ta_height; z++)
        for(x = 0; x < ta_width; x++)
            if(tile[x][z].parsec_ptr->draw)
            {
                if(tile[x][z].tilemap_ptr->tile_type < TT_DIRT_ROAD)
                {
                    // Determine tile distance from camera - only tiles that
                    // are populated with SE's or trees need distance comp.
                    tile[x][z].distance = sqrt(
                        ((tile[x][z].pos[0] - cam_pos[0]) *
                            (tile[x][z].pos[0] - cam_pos[0])) + 
                        ((tile[x][z].pos[2] - cam_pos[2]) *
                            (tile[x][z].pos[2] - cam_pos[2])));
                    
                    // Determine if SE list needs updating or not (values still
                    // running up to or down to other values).
                    if(tile[x][z].se_update)
                    {
                        se_updated = false;
                        sel_prev = NULL;
                        sel_curr = tile[x][z].sel_head;
                        while(sel_curr)
                        {
                            if(sel_curr->fade_in)
                            {
                                sel_curr->alpha += (0.75 * deltaT);
                                
                                if(sel_curr->alpha >= 1.0)
                                {
                                    sel_curr->alpha = 1.0;
                                    sel_curr->fade_in = false;
                                }
                                else
                                    se_updated = true;
                            }
                            else if(sel_curr->fade_out)
                            {
                                sel_curr->alpha -= (0.75 * deltaT);
                                
                                if(sel_curr->alpha <= 0.0)
                                {
                                    // Remove element
                                    if(sel_prev == NULL)
                                    {
                                        tile[x][z].sel_head = tile[x][z].sel_head->next;
                                        delete sel_curr;
                                        if(tile[x][z].sel_head == NULL)
                                            tile[x][z].sel_tail = NULL;
                                        sel_curr = tile[x][z].sel_head;
                                        continue;
                                    }
                                    else
                                    {
                                        sel_prev->next = sel_curr->next;
                                        if(tile[x][z].sel_tail == sel_curr)
                                            tile[x][z].sel_tail = sel_prev;
                                        delete sel_curr;
                                        sel_curr = sel_prev->next;
                                        continue;
                                    }
                                }
                                else
                                    se_updated = true;
                            }
                            
                            sel_prev = sel_curr;
                            sel_curr = sel_curr->next;
                        }
                        
                        tile[x][z].se_update = se_updated;
                    }
                    
                    // Determine SE ratio and SE want values
                    if(game_options.scenery_elements == SC_COVERAGE_NONE ||
                       tile[x][z].distance >= 350.0)
                    {
                        se_ratio = 0.0;
                        se_want = 0;
                    }
                    else
                    {
                        // Anything under 100.0 is auto 1.0
                        if(tile[x][z].distance < 100.0)
                            se_ratio = 1.0;
                        else
                            se_ratio = 1.0 - ((tile[x][z].distance - 100.0) / 250.0);
                        
                        switch(tile[x][z].tilemap_ptr->tile_type)
                        {
                            case TT_OPEN:
                            case TT_BRUSH:
                                se_want = (int)((25.0 * se_ratio) + 0.5);
                                break;
                            
                            case TT_ROCKY:
                                se_want = (int)((15.0 * se_ratio) + 0.5);
                                break;
                            
                            case TT_SPARSE_TREES:
                            case TT_DENSE_TREES:
                            case TT_PINE_TREES:
                                se_want = (int)((10.0 * se_ratio) + 0.5);
                                break;
                            
                            case TT_WHEAT_FIELD:
                            case TT_CORN_FIELD:
                            case TT_VINEYARD:
                                se_want = (int)((10.0 * se_ratio) + 0.5);
                                break;
                            
                            default:
                                se_want = 0;
                                break;
                        }
                        
                        // Account for option settings
                        if(game_options.scenery_elements == SC_COVERAGE_MODERATE)
                            se_want = (int)((float)se_want * 0.666667f);
                        else if(game_options.scenery_elements == SC_COVERAGE_SPARSE)
                            se_want = (int)((float)se_want * 0.333333f);
                        
                        // Double check for correct dimensions
                        if(se_want < 0)
                            se_want = 0;
                        else if(se_want > 25)
                            se_want = 25;
                    }
                    
                    // Set the overlay alpha for this tile based upon the SE
                    // ratio. Kind of an out-of-the-way setting, but best done
                    // here rather than elsewhere.
                    if(tile[x][z].overlay_dspList != DISPLAY_NULL)
                        tile[x][z].overlay_alpha = 0.9 * se_ratio;
                    
                    // Add SE's for tiles where the want is greater than count.
                    while(tile[x][z].sel_count < se_want)
                    {
                        sel_curr = new scenery_element;
                        
                        if(tile[x][z].distance <= 100.0)
                        {
                            // Full alpha for those add-ins under 100m (very
                            // possible if camera is rotated fast and the parsec
                            // culling kicks in).
                            sel_curr->alpha = 1.0;
                            sel_curr->fade_in = false;
                        }
                        else
                        {
                            // Start alpha at 0.0, and set to run up to value.
                            sel_curr->alpha = 0.0;
                            sel_curr->fade_in = true;
                            tile[x][z].se_update = true;    // Will need update
                        }
                        sel_curr->fade_out = false;
                        sel_curr->has_pitch = false;
                        
                        sel_curr->pos[0] = tile[x][z].pos[0] +
                            (((float)rand() / (float)RAND_MAX) * tile_size);
                        sel_curr->pos[2] = tile[x][z].pos[2] +
                            (((float)rand() / (float)RAND_MAX) * tile_size);
                        sel_curr->dir[0] =
                            ((float)rand() / (float)RAND_MAX) * 360.0;
                        
                        switch(tile[x][z].tilemap_ptr->tile_type)
                        {
                            case TT_OPEN:
                                sel_curr->width = 1.75;
                                sel_curr->height = 1.25;
                                sel_curr->texture_id = sc_textures[TT_OPEN][
                                    choose(sc_textures[TT_OPEN][0])];
                                break;
                            
                            case TT_BRUSH:
                            case TT_SPARSE_TREES:
                            case TT_DENSE_TREES:
                            case TT_PINE_TREES:
                                if(roll(0.65))
                                {
                                    sel_curr->width = 1.25;
                                    sel_curr->height = 1.25;
                                    sel_curr->texture_id = sc_textures[TT_BRUSH][
                                        choose(sc_textures[TT_BRUSH][0])];
                                }
                                else
                                {
                                    sel_curr->width = 1.75;
                                    sel_curr->height = 1.0;
                                    sel_curr->texture_id = sc_textures[TT_OPEN][
                                        choose(sc_textures[TT_OPEN][0])];
                                }
                                break;
                            
                            case TT_ROCKY:
                                sel_curr->width = 1.0;
                                sel_curr->height = 1.0;
                                sel_curr->texture_id = sc_textures[TT_ROCKY][
                                    choose(sc_textures[TT_ROCKY][0])];
                                break;
                            
                            case TT_WHEAT_FIELD:
                            case TT_CORN_FIELD:
                            case TT_VINEYARD:
                                sel_curr->pos[0] = tile[x][z].pos[0] +
                                    ((rand() % 7) * (tile_size / 7.5)) + (tile_size * 0.1) +
                                    ((((float)rand() / (float)RAND_MAX) * 0.5) - 0.25);
                                sel_curr->pos[2] = tile[x][z].pos[2] +
                                    (((float)rand() / (float)RAND_MAX) * (0.7 * tile_size)) +
                                    (0.15 * tile_size);
                                
                                sel_curr->dir[0] = ((((float)rand() / (float)RAND_MAX) * 20.0) - 10.0) + 90.0;
                                sel_curr->has_pitch = true;
                                sel_curr->dir[1] = (((float)rand() / (float)RAND_MAX) * 45.0) - 22.5;
                                
                                sel_curr->width = 2.25;
                                sel_curr->height = 1.0;
                                
                                sel_curr->texture_id = sc_textures[tile[x][z].tilemap_ptr->tile_type][
                                    choose(sc_textures[tile[x][z].tilemap_ptr->tile_type][0])];
                                break;
                            
                            default:
                                break;
                        }
                        
                        // Set height position (w/o overlays)
                        sel_curr->pos[1] = getHeight(sel_curr->pos[0], sel_curr->pos[2]);
                        
                        // Add at tail of SE list
                        if(tile[x][z].sel_head)
                        {
                            tile[x][z].sel_tail->next = sel_curr;
                            tile[x][z].sel_tail = sel_curr;
                        }
                        else
                        {
                            tile[x][z].sel_head = sel_curr;
                            tile[x][z].sel_tail = sel_curr;
                        }
                        tile[x][z].sel_tail->next = NULL;
                        
                        tile[x][z].sel_count++;
                    }
                    
                    // Fade out SE's for tiles that count is greater than want.
                    if(tile[x][z].sel_count > se_want)
                    {
                        sel_curr = tile[x][z].sel_head;
                        while(sel_curr && tile[x][z].sel_count > se_want)
                        {
                            if(sel_curr->fade_out == false)
                            {
                                sel_curr->fade_out = true;
                                tile[x][z].sel_count--;
                                tile[x][z].se_update = true;
                            }
                            sel_curr = sel_curr->next;
                        }
                    }
                }
                
                // Update trees
                if((tol_curr = tile[x][z].tol_head) != NULL)
                {
                    kVector dir;
                    float theta_addon = ((((float)rand() / (float)RAND_MAX) * 0.20) + 0.90) * deltaT;
                    
                    while(tol_curr)
                    {
                        if(tile[x][z].distance >= 125.0)
                        {
                            // For farther away trees, do a minimal amount of
                            // work neccessary (since LPBBs can't possibly
                            // happen anyways - not to mention these are just
                            // billboarded versions).
                            dir = camera.getCamPosV() - kVector(tol_curr->pos);
                            dir.convertTo(CS_YAW_ONLY);
                            tol_curr->dir[0] = 12345.0f;    // Will be random
                            tol_curr->dir[1] = dir[2];
                            
                            // Set up to be a fully opaque BB and not 3D.
                            tol_curr->bb_alpha = 1.0f;
                            tol_curr->f3d_alpha = 0.0f;
                        }
                        else
                        {
                            // For close-up trees, do a full amount of work to
                            // handle LPBBs (if defined), otherwise still only
                            // do the minimal required.
                            if(tol_curr->lpbb_count > 0)
                            {
                                // Grab pitch as well as yaw
                                dir = camera.getCamPosV() - (kVector(tol_curr->pos) +
                                    kVector(0.0, 0.75f * tol_curr->scale[1], 0.0));
                                dir.convertTo(CS_SPHERICAL);
                                tol_curr->dir[2] = dir[1] - PIHALF; // Pitch
                                tol_curr->dir[1] = dir[2];      // Actual Yaw
                            }
                            else
                            {
                                // Still only grab yaw
                                dir = camera.getCamPosV() - kVector(tol_curr->pos);
                                dir.convertTo(CS_YAW_ONLY);
                                tol_curr->dir[1] = dir[2];      // Actual Yaw
                            }
                            
                            // Set a random twist on the 3D version if and only
                            // if we are initially coming in from a BB version,
                            // in which dir[0] will be .. 12345.0f, amazing, its
                            // the combination on my luggage!
                            if(tol_curr->dir[0] == 12345.0f)    // Current Yaw
                                tol_curr->dir[0] = ((float)rand() / (float)RAND_MAX) * TWOPI;
                            
                            if(tile[x][z].distance <= 75.0)
                            {
                                // Fully opaque 3D version, no BB
                                tol_curr->bb_alpha = 0.0f;
                                tol_curr->f3d_alpha = 1.0f;
                            }
                            else
                            {
                                // Alpha fade between 3D<->BB
                                tol_curr->bb_alpha = (tile[x][z].distance >= 100.0 ?
                                    1.0 : (tile[x][z].distance - 75.0) / 25.0);
                                tol_curr->f3d_alpha = (tile[x][z].distance <= 100.0 ?
                                    1.0 : 1.0 - ((tile[x][z].distance - 100.0) / 25.0));
                            }
                        }
                        
                        // Add theta onto shearing comp. value
                        tol_curr->theta += theta_addon;
                        
                        tol_curr = tol_curr->t_next;
                    }
                }
            }
            else
            {
                // Kill off any SE's in non-displaying tiles (if present)
                if(tile[x][z].sel_head != NULL)
                {
                    sel_curr = tile[x][z].sel_head;
                    while(sel_curr)
                    {
                        tile[x][z].sel_head = tile[x][z].sel_head->next;
                        delete sel_curr;
                        sel_curr = tile[x][z].sel_head;
                    }
                    tile[x][z].sel_tail = NULL;
                    tile[x][z].sel_count = 0;
                    tile[x][z].se_update = false;
                }
            }
}

/*******************************************************************************
    function    :   scenery_module::display_firstpass
    arguments   :   <none>
    purpose     :   First pass display: Orients & renders skybox and base map
                    terrain (parsecs).
    notes       :   <none>
*******************************************************************************/
void scenery_module::displayFirstPass()
{
    int i;
    static float map_center[3] = {map_width / 2.0, 0.0, map_height / 2.0};
    static float* cam_pos = camera.getCamPos();
    
    // Setup OpenGL for rendering
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_TEXTURE_2D);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    
    // Step 1) Draw skybox
    glPushMatrix();
    
    // Translate our sky box along with the camera - The skybox is so far away
    // that we set it to only move a 1/2 the speed of the camera thus giving
    // it a nice depth effect. Note that we could make this a part of the
    // updatefunc() if wanted to.
    glTranslatef(
        map_center[0] + ((cam_pos[0] - map_center[0]) * 0.5), 0.0,
        map_center[2] + ((cam_pos[2] - map_center[2]) * 0.5));
    
    // Set texture mapping mode to decal so that the skybox doesn't have to
    // worry about materials.
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    
    // Render skybox
    glCallList(skybox_dspList);
    
    // We must have our texture mapping mode set to modulate so the unique
    // brightness values show through.
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    // Render ground
    glCallList(ground_dspList);
    
    glPopMatrix();
    
    // Step 2) Draw parsecs
    
    // Render (visible) parsecs
    for(i = 0; i < parsec_count; i++)
        if(parsec[i].draw)
            glCallList(parsec[i].dspList);
}

/*******************************************************************************
    function    :   scenery_module::displaySecondPass
    arguments   :   <none>
    purpose     :   Second pass display: Orients & renders bridges, SE's, trees,
                    water overlay, and tile overlays.
    notes       :   Uses an on-the-fly bucket sort (only accurate for first
                    decimal digit) to handle alpha/depth buffer related issues.
*******************************************************************************/
void scenery_module::displaySecondPass()
{
    int i, j, x, z;
    int bucket;
    scenery_element* sel_bucket[10] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
    tree_object* tol_f3d_bucket[10] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
    tree_object* tol_bb_bucket[10] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
    bridge_object* bol_curr;
    scenery_element* sel_curr;
    tree_object* tol_curr;
    int tree_count_left;
    GLfloat shear[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};   // Shearing Matrix
    
    // Setup tree count left
    if(game_options.tree_coverage == SC_COVERAGE_DENSE)
        tree_count_left = tree_count;
    else if(game_options.tree_coverage == SC_COVERAGE_MODERATE)
        tree_count_left = (tree_count  * 2) / 3;
    else if(game_options.tree_coverage == SC_COVERAGE_SPARSE)
        tree_count_left = tree_count / 3;
    else
        tree_count_left = 0;
    
    // Setup OpenGL for rendering
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_COLOR_MATERIAL);
    
    // We must have our texture mapping mode set to modulate so that glColor
    // can control the alpha values of renderings.
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    // No transparency for the first couple of steps here (get all the opaque
    // stuff out of the way before rendering the alphatized ones). This also
    // reduces the number of material changes -> greater display efficiency.
    glColor4f(1.0, 1.0, 1.0, 1.0);
    
    // Step 1) Draw bridges (trivial)
    bol_curr = bol_head;
    while(bol_curr)
    {
        if(bol_curr->draw)
            glCallList(bol_curr->dspList);
        bol_curr = bol_curr->next;
    }
    
    // Step 2) Draw fully opaque SE's
    for(z = 0; z < ta_height; z++)
        for(x = 0; x < ta_width; x++)
        {
            // Check for tile draw
            if(!tile[x][z].parsec_ptr->draw)
                continue;
            
            // Draw any SE's first
            if(tile[x][z].sel_count > 0)
            {
                sel_curr = tile[x][z].sel_head;
                while(sel_curr)
                {
                    if(!sel_curr->fade_out && !sel_curr->fade_in)
                    {
                        glPushMatrix();
                        
                        // Orient SE
                        glTranslatef(sel_curr->pos[0], sel_curr->pos[1], sel_curr->pos[2]);
                        glRotatef(sel_curr->dir[0], 0.0, 1.0, 0.0);
                        if(sel_curr->has_pitch)
                            glRotatef(sel_curr->dir[1], 1.0, 0.0, 0.0);
                        
                        // Render SE
                        glBindTexture(GL_TEXTURE_2D, sel_curr->texture_id);
                        glBegin(GL_QUADS);
                            glNormal3f(0.0, 1.0, 0.0);
                            glTexCoord2f(0.0, 0.0);
                            glVertex3f(-sel_curr->width, 0.0, 0.0);
                            glTexCoord2f(1.0, 0.0);
                            glVertex3f(sel_curr->width, 0.0, 0.0);
                            glTexCoord2f(1.0, 1.0);
                            glVertex3f(sel_curr->width, sel_curr->height, 0.0);
                            glTexCoord2f(0.0, 1.0);
                            glVertex3f(-sel_curr->width, sel_curr->height, 0.0);
                        glEnd();
                        
                        glPopMatrix();
                    }
                    else
                    {
                        // Add into corresponding bucket for later drawing
                        bucket = (int)(sel_curr->alpha * 10.0f);
                        if(bucket < 0) bucket = 0;          // Bounds check
                        else if (bucket > 9) bucket = 9;
                        sel_curr->s_next = sel_bucket[bucket];
                        sel_bucket[bucket] = sel_curr;
                    }
                    
                    sel_curr = sel_curr->next;
                }
            }
        }
    
    // Step 3) Draw any opaque trees
    tol_curr = tol_head;
    while(tree_count_left > 0 && tol_curr)
    {
        // Check for tile draw
        if(!tol_curr->parsec_ptr->draw)
        {
            tree_count_left--;
            tol_curr = tol_curr->g_next;
            continue;
        }
        
        if(tol_curr->f3d_alpha == 1.0f)
        {
            glPushMatrix();
            
            // Orient tree
            glTranslatef(tol_curr->pos[0], tol_curr->pos[1], tol_curr->pos[2]);
            glRotatef(tol_curr->dir[0] * radToDeg, 0.0, 1.0, 0.0);
            shear[4] = shear[6] = 0.024f * cos(tol_curr->theta);
            glMultMatrixf(shear);
            
            // Render tree
            glCallList(tol_curr->f3d_dspList);
            
            // Render LPBBs
            for(i = 0; i < tol_curr->lpbb_count; i++)
            {
                glPushMatrix();
                
                // Orient LPBB
                glTranslatef(tol_curr->lpbb_pos[i][0], tol_curr->lpbb_pos[i][1], tol_curr->lpbb_pos[i][2]);
                glRotatef((tol_curr->dir[1] - tol_curr->dir[0]) * radToDeg, 0.0, 1.0, 0.0);
                glRotatef(tol_curr->dir[2] * radToDeg, 1.0, 0.0, 0.0);
                
                // Render LPBB
                glBindTexture(GL_TEXTURE_2D, tol_curr->lpbb_texture);
                glScalef(tol_curr->scale[2], tol_curr->scale[2], 0.0);
                glBegin(GL_QUADS);
                    glNormal3f(0.0, 0.0, 1.0);
                    glTexCoord2f(0.0, 0.0);
                    glVertex3f(-0.5, -0.5, 0.0);
                    glTexCoord2f(1.0, 0.0);
                    glVertex3f(0.5, -0.5, 0.0);
                    glTexCoord2f(1.0, 1.0);
                    glVertex3f(0.5, 0.5, 0.0);
                    glTexCoord2f(0.0, 1.0);
                    glVertex3f(-0.5, 0.5, 0.0);
                glEnd();
                
                glPopMatrix();
            }
            
            glPopMatrix();
        }
        else if(tol_curr->f3d_alpha > 0.0f)
        {
            // Add into corresponding bucket for later drawing
            bucket = (int)(tol_curr->f3d_alpha * 10.0f);
            if(bucket < 0) bucket = 0;          // Bounds check
            else if (bucket > 9) bucket = 9;
            tol_curr->s_f3d_next = tol_f3d_bucket[bucket];
            tol_f3d_bucket[bucket] = tol_curr;
        }
        
        if(tol_curr->bb_alpha == 1.0f)
        {
            glPushMatrix();
            
            // Orient tree
            glTranslatef(tol_curr->pos[0], tol_curr->pos[1], tol_curr->pos[2]);
            glRotatef(tol_curr->dir[1] * radToDeg, 0.0, 1.0, 0.0);
            glScalef(tol_curr->scale[0], tol_curr->scale[1], 1.0);
            shear[4] = shear[6] = 0.024f * cos(tol_curr->theta);
            glMultMatrixf(shear);
            
            // Render tree
            glCallList(tol_curr->bb_dspList);
            
            glPopMatrix();
        }
        else if(tol_curr->bb_alpha > 0.0f)
        {
            // Add into corresponding bucket for later drawing
            bucket = (int)(tol_curr->bb_alpha * 10.0f);
            if(bucket < 0) bucket = 0;          // Bounds check
            else if (bucket > 9) bucket = 9;
            tol_curr->s_bb_next = tol_bb_bucket[bucket];
            tol_bb_bucket[bucket] = tol_curr;
        }
        
        tree_count_left--;
        tol_curr = tol_curr->g_next;
    }
    
    // Step 3) Draw water overlay
    // Set alpha values
    glColor4f(0.85, 0.85, 0.85, 0.92);      // Water is semi-transparent
    
    // Render water graphic from each of the four corners of the earth
    glBindTexture(GL_TEXTURE_2D, water_texture_id);
    glBegin(GL_QUADS);
        glNormal3f(0.0, 1.0, 0.0);
        glTexCoord2f(water_texture_offset, 1.0 + water_texture_offset);
        glVertex3f(0.0, water_height + water_height_offset, 0.0);
        glTexCoord2f(1.0 + water_texture_offset, 1.0 + water_texture_offset);
        glVertex3f(map_width, water_height + water_height_offset, 0.0);
        glTexCoord2f(1.0 + water_texture_offset, water_texture_offset);
        glVertex3f(map_width, water_height + water_height_offset, map_height);
        glTexCoord2f(0.0 + water_texture_offset, water_texture_offset);
        glVertex3f(0.0, water_height + water_height_offset, map_height);
    glEnd();
    
    // Step 4) Draw any alphatized SE's based on the bucket sort
    for(i = 9; i >= 0; i--)
    {
        sel_curr = sel_bucket[i];
        while(sel_curr)
        {
            glPushMatrix();
            
            // Orient SE
            glTranslatef(sel_curr->pos[0], sel_curr->pos[1], sel_curr->pos[2]);
            glRotatef(sel_curr->dir[0], 0.0, 1.0, 0.0);
            if(sel_curr->has_pitch)
                glRotatef(sel_curr->dir[1], 1.0, 0.0, 0.0);
            
            // Set alpha values
            glAlphaFunc(GL_GEQUAL, ALPHA_PASS * sel_curr->alpha);
            glColor4f(1.0, 1.0, 1.0, sel_curr->alpha);
            
            // Render SE
            glBindTexture(GL_TEXTURE_2D, sel_curr->texture_id);
            glBegin(GL_QUADS);
                glNormal3f(0.0, 1.0, 0.0);
                glTexCoord2f(0.0, 0.0);
                glVertex3f(-sel_curr->width, 0.0, 0.0);
                glTexCoord2f(1.0, 0.0);
                glVertex3f(sel_curr->width, 0.0, 0.0);
                glTexCoord2f(1.0, 1.0);
                glVertex3f(sel_curr->width, sel_curr->height, 0.0);
                glTexCoord2f(0.0, 1.0);
                glVertex3f(-sel_curr->width, sel_curr->height, 0.0);
            glEnd();
            
            glPopMatrix();
            
            sel_curr = sel_curr->s_next;
        }
    }
    
    // Step 5) Draw tile overlays
    for(z = 0; z < ta_height; z++)
        for(x = 0; x < ta_width; x++)
            if(tile[x][z].overlay_dspList != DISPLAY_NULL &&
               tile[x][z].parsec_ptr->draw &&
               tile[x][z].overlay_alpha > FP_ERROR)
            {
                // Set alpha values
                glAlphaFunc(GL_GEQUAL, ALPHA_PASS * tile[x][z].overlay_alpha);
                glColor4f(1.0, 1.0, 1.0, tile[x][z].overlay_alpha);
                
                // Render tile overlay (already oriented)
                glCallList(tile[x][z].overlay_dspList);
            }
    
    // Step 6) Draw any alphatized trees based on the bucket sort
    for(i = 9; i >= 0; i--)
    {
        // 3D versions have priority over BB versions
        tol_curr = tol_f3d_bucket[i];
        while(tol_curr)
        {
            glPushMatrix();
            
            // Orient tree
            glTranslatef(tol_curr->pos[0], tol_curr->pos[1], tol_curr->pos[2]);
            glRotatef(tol_curr->dir[0] * radToDeg, 0.0, 1.0, 0.0);
            shear[4] = shear[6] = 0.024f * cos(tol_curr->theta);
            glMultMatrixf(shear);
            
            // Set alpha values
            glAlphaFunc(GL_GEQUAL, ALPHA_PASS * tol_curr->f3d_alpha);
            glColor4f(1.0, 1.0, 1.0, tol_curr->f3d_alpha);
            
            // Render tree
            glCallList(tol_curr->f3d_dspList);
            
            // Render LPBBs
            for(j = 0; j < tol_curr->lpbb_count; j++)
            {
                glPushMatrix();
                
                // Orient LPBB
                glTranslatef(tol_curr->lpbb_pos[j][0], tol_curr->lpbb_pos[j][1], tol_curr->lpbb_pos[j][2]);
                glRotatef((tol_curr->dir[1] - tol_curr->dir[0]) * radToDeg, 0.0, 1.0, 0.0);
                glRotatef(tol_curr->dir[2] * radToDeg, 1.0, 0.0, 0.0);
                
                // Render LPBB
                glBindTexture(GL_TEXTURE_2D, tol_curr->lpbb_texture);
                glScalef(tol_curr->scale[2], tol_curr->scale[2], 0.0);
                glBegin(GL_QUADS);
                    glNormal3f(0.0, 0.0, 1.0);
                    glTexCoord2f(0.0, 0.0);
                    glVertex3f(-0.5, -0.5, 0.0);
                    glTexCoord2f(1.0, 0.0);
                    glVertex3f(0.5, -0.5, 0.0);
                    glTexCoord2f(1.0, 1.0);
                    glVertex3f(0.5, 0.5, 0.0);
                    glTexCoord2f(0.0, 1.0);
                    glVertex3f(-0.5, 0.5, 0.0);
                glEnd();
                
                glPopMatrix();
            }
            
            glPopMatrix();
            
            tol_curr = tol_curr->s_f3d_next;
        }
        
        tol_curr = tol_bb_bucket[i];
        while(tol_curr)
        {
            glPushMatrix();
            
            // Orient tree
            glTranslatef(tol_curr->pos[0], tol_curr->pos[1], tol_curr->pos[2]);
            glRotatef(tol_curr->dir[1] * radToDeg, 0.0, 1.0, 0.0);
            glScalef(tol_curr->scale[0], tol_curr->scale[1], 1.0);
            shear[4] = shear[6] = 0.024f * cos(tol_curr->theta);
            glMultMatrixf(shear);
            
            // Set alpha values
            glAlphaFunc(GL_GEQUAL, ALPHA_PASS * tol_curr->bb_alpha);
            glColor4f(1.0, 1.0, 1.0, tol_curr->bb_alpha);
            
            // Render tree
            glCallList(tol_curr->bb_dspList);
            
            glPopMatrix();
            
            tol_curr = tol_curr->s_bb_next;
        }
    }
}
