/*******************************************************************************
                         Model Library - Implementation
*******************************************************************************/
#include "main.h"
#include "model.h"
#include "misc.h"
#include "metrics.h"
#include "texture.h"

/*******************************************************************************
    function    :   model_library::model_library
    arguments   :   <none>
    purpose     :   Constructor.
    notes       :   <none>
*******************************************************************************/
model_library::model_library()
{
    int i;
    ofstream fout;
    
    // Initialize all our variables for our library
    model_count = 0;
    
    for(i = 0; i < MAX_MODELS; i++)
    {
        model[i].model_name = NULL;
        model[i].model_base = NULL;
        model[i].mesh = NULL;
        model[i].mesh_count = 0;
        
        model[i].radius = 0.0;
        model[i].min[0] = model[i].min[1] = model[i].min[2] = MDL_MESH_MINMAX_START;
        model[i].max[0] = model[i].max[1] = model[i].max[2] = -MDL_MESH_MINMAX_START;
    }
    
    // Initialize loading on the fly value
    setLoadOTF();
    
    // Initialize load log value
    setLoadLog();
    
    // Output load log header
    fout.open("Models/load.log", ios::out);
    fout << "           Korps Model Library Load Log" << endl << endl;
    fout.close();
}

/*******************************************************************************
    function    :   model_library::~model_library
    arguments   :   <none>
    purpose     :   Deconstructor.
    notes       :   <none>
*******************************************************************************/
model_library::~model_library()
{
    int i, j;
    
    // Go through our hash table looking for entries to delete
    for(i = 0; i < MAX_MODELS; i++)
    {
        // See if entry exists (name and base exist)
        if(model[i].model_name && model[i].model_base)
        {
            // If so, delete all data elements of entries
            for(j = 0; j < model[i].mesh_count; j++)
            {
                // Delete data arrays
                if(model[i].mesh[j].mesh_name)
                    delete model[i].mesh[j].mesh_name;            
                if(model[i].mesh[j].vertex_data)
                {
                    delete model[i].mesh[j].vertex_data[0]; // Data array
                    delete model[i].mesh[j].vertex_data;    // Pointer array
                }
                if(model[i].mesh[j].texel_data)
                {
                    delete model[i].mesh[j].texel_data[0];  // Data array
                    delete model[i].mesh[j].texel_data;     // Pointer array
                }
                if(model[i].mesh[j].normal_data)
                {
                    delete model[i].mesh[j].normal_data[0]; // Data array
                    delete model[i].mesh[j].normal_data;    // Pointer array
                }
                if(model[i].mesh[j].d_data)
                    delete model[i].mesh[j].d_data;         // Data array
                if(model[i].mesh[j].material_data)
                {
                    delete model[i].mesh[j].material_data[0];   // Data array
                    delete model[i].mesh[j].material_data;      // Pointer array
                }
            }
            
            delete model[i].model_name;
            delete model[i].model_base;
        }
    }
}

/*******************************************************************************
    function    :   unsigned int model_library::hash
    arguments   :   string - string to hash
    purpose     :   String hash function.
    notes       :   Uses string djb2 hash algorithm.
*******************************************************************************/
unsigned int model_library::hash(char* string)
{
    unsigned int hash = 5381;
    char* str = string;
    int c;
    
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    
    return hash % MAX_MODELS;
}

/*******************************************************************************
    function    :   int model_library::loadModel
    arguments   :   modelName - name of model to load into memory
    purpose     :   Loads a model into memory from disk using the LIB3DS
                    library. Stores model data into the model hash table.
    notes       :   1) Returns a Model Library reference ID number (same ID as
                       returned by getModel), otherwise -1 if in error.
                    2) Multiple loads of same file does not result in multiple
                       memory storage locations. The code will catch double
                       loads and still return as first note dictates.
                    3) Converts between .3ds axis notation and OpenGL axis
                       notation (e.g. Y values and Z values are swapped).
*******************************************************************************/
int model_library::loadModel(char* modelName)
{
    int i, j, k;
    char buffer[128];
    int insert_pos;
    char filename[128];
    char* low_detail;
    Lib3dsFile* model_file;
    Lib3dsMesh* curr_mesh;
    Lib3dsMaterial* material;
    GLfloat* temp_array;
    kVector normal;
    ofstream fout;              // For load logging
    
    // Check for max model load limit
    if(model_count >= MAX_MODELS)
    {
        write_error("ModLib: Model count limit reached.");
        return -1;
    }
    
    // Hash the model name to determine where to insert the model, or if it
    // is already loaded then return with that position
    for(insert_pos = hash(modelName); model[insert_pos].model_name != NULL;
        insert_pos++)
    {
        // Check to see if model is already loaded
        if(strcmp(model[insert_pos].model_name, modelName) == 0)
            return insert_pos;
        
        // Loop around if at end of circular hash table
        if(insert_pos + 1 >= MAX_MODELS)
            insert_pos = -1;
    }
    
    // Open load log file if enabled (open in append mode)
    if(load_log)
        fout.open("Models/load.log", ios::app);
    
    // Search for "low detail" specialty string ("ld" at end)
    low_detail = strstr(modelName, "_ld");
    
    // Formulate model base name based on model name (different for _ld cases)
    model[insert_pos].model_base = strdup(modelName);
    if(low_detail)
    {
        int offset = (unsigned int)low_detail - (unsigned int)modelName;
        model[insert_pos].model_base[offset] = '\0';
    }
    
    // Create filename based on model base and name
    sprintf(filename, "Models/%s/%s.3ds",
        model[insert_pos].model_base,
        modelName);
    
    // Display load log message
    if(load_log)
        fout << "Loading: \"" << filename << "\" - ";
    
    // Open the 3ds model file.
    model_file = lib3ds_file_load(filename);
    
    // See if it opened successfully
    if(!model_file)
    {
        fout << "[failure]" << endl;
        fout.close();
        // If not we consider this a FATAL error - print error message and exit
        sprintf(buffer, "ModLib: FATAL: Failure loading \"%s\" for read.",
            filename);
        write_error(buffer);
        exit(1);
    }
    
    // Display load log message
    if(load_log)
        fout << "[success]" << endl
             << "  Model Name   : \"" << modelName << "\"" << endl
             << "  Low Detail   : " << (low_detail ? "Yes" : "No") << endl;
    
    // Set TexLib to load textures on the fly
    textures.setLoadOTF(true);
    
    // Copy over model's name (officially inserting the model)
    model[insert_pos].model_name = strdup(modelName);
    
    // Start curr_mesh at head ptr
    curr_mesh = model_file->meshes;
    
    // Count the number of meshes (for mesh_count)
    model[insert_pos].mesh_count = 0;  // Initialize at zero
    while(curr_mesh != NULL)
    {
        // Go through each mesh and add to count
        model[insert_pos].mesh_count++;
        curr_mesh = curr_mesh->next;
    }
    
    // Display load log message
    if(load_log)
        fout << "  Object Meshes: " << model[insert_pos].mesh_count << endl;
    
    // Allocate memory for the meshes
    model[insert_pos].mesh = new object_mesh[model[insert_pos].mesh_count];
    
    // Start curr_mesh at head ptr
    curr_mesh = model_file->meshes;
    
    // Go through our object's meshes and start filling in data elements
    for(i = 0; i < model[insert_pos].mesh_count; i++)
    {
        // Copy over mesh name
        model[insert_pos].mesh[i].mesh_name = strdup(curr_mesh->name);
        
        // Set min/max finders
        model[insert_pos].mesh[i].min[0] = model[insert_pos].mesh[i].min[1] =
            model[insert_pos].mesh[i].min[2] = MDL_MESH_MINMAX_START;
        model[insert_pos].mesh[i].max[0] = model[insert_pos].mesh[i].max[1] =
            model[insert_pos].mesh[i].max[2] = -MDL_MESH_MINMAX_START;
        
        // Display load log message
        if(load_log)
            fout << "    Mesh " << (i + 1) << ":" << endl
                 << "      Name        : \"" << curr_mesh->name << "\"" << endl
                 << "      Poly Count  : " << curr_mesh->faces << endl
                 << "      Vertex Count: " << 3 * curr_mesh->faces << endl
                 << "        Memory Allocation   - ";
        
        // Copy over face count
        model[insert_pos].mesh[i].vertex_count = 3 * curr_mesh->faces;
        
        // Allocate memory for vertex data (3 floats per vertex, 3 vertexes per
        // face)
        model[insert_pos].mesh[i].vertex_data =
            new GLfloat*[3 * curr_mesh->faces];
        temp_array = new GLfloat[3 * 3 * curr_mesh->faces];
        for(j = 0; j < 3 * (int)curr_mesh->faces; j++)
            model[insert_pos].mesh[i].vertex_data[j] = temp_array + (3 * j);
        
        // Allocate memory for UV mapping data (2 texture coordinates per
        // vertex (indexed), 3 vertexes per face) if object mesh is indeed
        // texture mapped.
        if(curr_mesh->texels > 0)
        {
            model[insert_pos].mesh[i].texel_data =
                new GLfloat*[3 * curr_mesh->faces];
            temp_array = new GLfloat[2 * 3 * curr_mesh->faces];
            for(j = 0; j < 3 * (int)curr_mesh->faces; j++)
                model[insert_pos].mesh[i].texel_data[j] = temp_array + (2 * j);
        }
        
        // Allocate memory for normal vectors data (one vector (3 floats) per
        // vertex (indexed), 3 vertexes per face)
        model[insert_pos].mesh[i].normal_data =
            new GLfloat*[3 * curr_mesh->faces];
        temp_array = new GLfloat[3 * 3 * curr_mesh->faces];
        for(j = 0; j < 3 * (int)curr_mesh->faces; j++)
            model[insert_pos].mesh[i].normal_data[j] = temp_array + (3 * j);
        
        // Allocate memory for distance vectors data (one value per vertex, 3
        // vertexes per face)
        model[insert_pos].mesh[i].d_data = new GLfloat[3 * curr_mesh->faces];
        
        // Allocate memory for material's data (4 for Ambient, Diffuse, and
        // Specular, and 1 extra for Shininess (so (4*3)+1 = 13))
        model[insert_pos].mesh[i].material_data = new GLfloat*[4];
        temp_array = new GLfloat[13];
        for(j = 0; j < 4; j++)
            model[insert_pos].mesh[i].material_data[j] = temp_array + (4 * j);
            
        model[insert_pos].mesh[i].poly_offset[0] = 0.0;
        model[insert_pos].mesh[i].poly_offset[1] = 0.0;
        model[insert_pos].mesh[i].poly_offset[2] = 0.0;
        model[insert_pos].mesh[i].texel_offset[0] = 0.0;
        model[insert_pos].mesh[i].texel_offset[1] = 0.0;
            
        // Display load log message
        if(load_log)
            fout << "[success]" << endl
                 << "        Mesh Data Copy-Over - ";
        
        // Go through each face
        for(j = 0; j < (int)curr_mesh->faces; j++)
        {
            // Go through each vertex in each face
            for(k = 0; k < 3; k++)
            {
                float temp_radius;
                
                // Vertex Points (Y and Z axis are interchanged and X axis is
                // inverted (between 3DS and OpenGL).)
                model[insert_pos].mesh[i].vertex_data[(j * 3) + k][0] =
                    -curr_mesh->pointL[curr_mesh->faceL[j].points[k]].pos[0];
                model[insert_pos].mesh[i].vertex_data[(j * 3) + k][1] =
                    curr_mesh->pointL[curr_mesh->faceL[j].points[k]].pos[2];
                model[insert_pos].mesh[i].vertex_data[(j * 3) + k][2] =
                    curr_mesh->pointL[curr_mesh->faceL[j].points[k]].pos[1];
                    
                // Update global min and max for model
                if(model[insert_pos].mesh[i].vertex_data[(j * 3) + k][0] <
                    model[insert_pos].min[0])
                    model[insert_pos].min[0] =
                        model[insert_pos].mesh[i].vertex_data[(j * 3) + k][0];
                if(model[insert_pos].mesh[i].vertex_data[(j * 3) + k][0] >
                    model[insert_pos].max[0])
                    model[insert_pos].max[0] =
                        model[insert_pos].mesh[i].vertex_data[(j * 3) + k][0];
                if(model[insert_pos].mesh[i].vertex_data[(j * 3) + k][1] <
                    model[insert_pos].min[1])
                    model[insert_pos].min[1] =
                        model[insert_pos].mesh[i].vertex_data[(j * 3) + k][1];
                if(model[insert_pos].mesh[i].vertex_data[(j * 3) + k][1] >
                    model[insert_pos].max[1])
                    model[insert_pos].max[1] =
                        model[insert_pos].mesh[i].vertex_data[(j * 3) + k][1];
                if(model[insert_pos].mesh[i].vertex_data[(j * 3) + k][2] <
                    model[insert_pos].min[2])
                    model[insert_pos].min[2] =
                        model[insert_pos].mesh[i].vertex_data[(j * 3) + k][2];
                if(model[insert_pos].mesh[i].vertex_data[(j * 3) + k][2] >
                    model[insert_pos].max[2])
                    model[insert_pos].max[2] =
                        model[insert_pos].mesh[i].vertex_data[(j * 3) + k][2];
                
                // Update radius for model
                temp_radius = sqrt(
                    (model[insert_pos].mesh[i].vertex_data[(j * 3) + k][0] *
                        model[insert_pos].mesh[i].vertex_data[(j * 3) + k][0]) +
                    (model[insert_pos].mesh[i].vertex_data[(j * 3) + k][1] *
                        model[insert_pos].mesh[i].vertex_data[(j * 3) + k][1]) +
                    (model[insert_pos].mesh[i].vertex_data[(j * 3) + k][2] *
                        model[insert_pos].mesh[i].vertex_data[(j * 3) + k][2]));
                if(temp_radius > model[insert_pos].radius)
                    model[insert_pos].radius = temp_radius;
                
                if(curr_mesh->texels > 0)
                {
                    // ST Mapping Points
                    model[insert_pos].mesh[i].texel_data[(j * 3) + k][0] =
                        curr_mesh->texelL[curr_mesh->faceL[j].points[k]][0];
                    model[insert_pos].mesh[i].texel_data[(j * 3) + k][1] =
                        curr_mesh->texelL[curr_mesh->faceL[j].points[k]][1];
                }
                else
                    model[insert_pos].mesh[i].texel_data = NULL;
                
                // Normal Vector
                model[insert_pos].mesh[i].normal_data[(j * 3) + k][0] =
                    -curr_mesh->faceL[j].normal[0];
                model[insert_pos].mesh[i].normal_data[(j * 3) + k][1] =
                    curr_mesh->faceL[j].normal[2];
                model[insert_pos].mesh[i].normal_data[(j * 3) + k][2] =
                    curr_mesh->faceL[j].normal[1];
                
                // Distance Value (dot product normal vector with position
                // vector).
                model[insert_pos].mesh[i].d_data[(j * 3) + k] = 0.0;
            }
            normal = normalized(crossProduct(
                kVector(model[insert_pos].mesh[i].vertex_data[(j * 3) + 1])
                - kVector(model[insert_pos].mesh[i].vertex_data[(j * 3) + 0]),
                kVector(model[insert_pos].mesh[i].vertex_data[(j * 3) + 2])
                - kVector(model[insert_pos].mesh[i].vertex_data[(j * 3) + 1])));
            model[insert_pos].mesh[i].normal_data[(j * 3) + 0][0] =
                model[insert_pos].mesh[i].normal_data[(j * 3) + 1][0] =
                model[insert_pos].mesh[i].normal_data[(j * 3) + 2][0] =
                    normal[0];
            model[insert_pos].mesh[i].normal_data[(j * 3) + 0][1] =
                model[insert_pos].mesh[i].normal_data[(j * 3) + 1][1] =
                model[insert_pos].mesh[i].normal_data[(j * 3) + 2][1] =
                    normal[1];
            model[insert_pos].mesh[i].normal_data[(j * 3) + 0][2] =
                model[insert_pos].mesh[i].normal_data[(j * 3) + 1][2] =
                model[insert_pos].mesh[i].normal_data[(j * 3) + 2][2] =
                    normal[2];
        }
        
        // Grab material for mesh (entire object uses same material as it's
        // first face - as per specification)
        material = lib3ds_file_material_by_name(
                       model_file,
                       curr_mesh->faceL[0].material);
        
        if(material != NULL)
        {
            // Copy over Ambient color values
            model[insert_pos].mesh[i].material_data[0][0] =
                material->ambient[0];
            model[insert_pos].mesh[i].material_data[0][1] =
                material->ambient[1];
            model[insert_pos].mesh[i].material_data[0][2] =
                material->ambient[2];
            model[insert_pos].mesh[i].material_data[0][3] =
                material->ambient[3];
            // Copy over Diffuse color values
            model[insert_pos].mesh[i].material_data[1][0] =
                material->diffuse[0];
            model[insert_pos].mesh[i].material_data[1][1] =
                material->diffuse[1];
            model[insert_pos].mesh[i].material_data[1][2] =
                material->diffuse[2];
            model[insert_pos].mesh[i].material_data[1][3] =
                material->diffuse[3];
            // Copy over Specular color values
            model[insert_pos].mesh[i].material_data[2][0] =
                material->specular[0];
            model[insert_pos].mesh[i].material_data[2][1] =
                material->specular[1];
            model[insert_pos].mesh[i].material_data[2][2] =
                material->specular[2];
            model[insert_pos].mesh[i].material_data[2][3] =
                material->specular[3];
            // Copy over Shininess value
            model[insert_pos].mesh[i].material_data[3][0] =
                material->shininess;
        }
        else
        {
            // Give model some standard material properties
            model[insert_pos].mesh[i].material_data[0][0] = 0.50;
            model[insert_pos].mesh[i].material_data[0][1] = 0.50;
            model[insert_pos].mesh[i].material_data[0][2] = 0.50;
            model[insert_pos].mesh[i].material_data[0][3] = 0.5;
            model[insert_pos].mesh[i].material_data[1][0] = 0.75;
            model[insert_pos].mesh[i].material_data[1][1] = 0.75;
            model[insert_pos].mesh[i].material_data[1][2] = 0.75;
            model[insert_pos].mesh[i].material_data[1][3] = 0.5;
            model[insert_pos].mesh[i].material_data[2][0] = 0.25;
            model[insert_pos].mesh[i].material_data[2][1] = 0.25;
            model[insert_pos].mesh[i].material_data[2][2] = 0.25;
            model[insert_pos].mesh[i].material_data[2][3] = 0.5;
            model[insert_pos].mesh[i].material_data[3][0] = 0.5;
        }
        
        // Display load log message
        if(load_log)
            fout << "[success]" << endl;

        // Assign texture map data
        if(material != NULL && material->texture1_map.name &&
           material->texture1_map.name[0])
        {
            // Create filename for texture map
            sprintf(filename, "Models/%s/%s", model[insert_pos].model_base,
                material->texture1_map.name);
            
            // Grab texture ID for texture map
            model[insert_pos].mesh[i].texture_id =
                textures.getTextureID(filename);
            
            // Display load log message
            if(load_log)
                fout << "      Texture Map : \""
                     << material->texture1_map.name << "\"" << endl
                     << "        Texture Binding     - ["
                     << (model[insert_pos].mesh[i].texture_id != TEXTURE_NULL ?
                            "success]" : "failure]")
                     << endl;
        }
        else
        {
            // No texture - set texture map ID to TEXTURE_NULL
            model[insert_pos].mesh[i].texture_id = TEXTURE_NULL;
            
            // Display load log message
            if(load_log)
                fout << "      Texture Map : None" << endl;
        }
        
        // Proceed to next mesh (this will essentially follow along with i)
        curr_mesh = curr_mesh->next;
    }
    
    // Close the 3ds model file.
    lib3ds_file_free(model_file);
    
    // Display load completion message and close file
    if(load_log)
    {
        fout << "Load Completion." << endl << endl;
        fout.close();
    }
    
    // Finally return an ID tag number (which is just the insert position)
    return insert_pos;
}

/*******************************************************************************
    function    :   int model_library::getModelID
    arguments   :   modelName - name of model to reference
    purpose     :   Hashes into hash table and attempts to find the model
                    given (in model name format) and return it's Model Library
                    reference ID number, or -1 if error.
    notes       :   Will automatically load the model if it has not already
                    been pre-loaded with a call to loadModel.
*******************************************************************************/
int model_library::getModelID(char* modelName)
{
    int insert_pos;
    
    // Hash the model name to determine where the model was inserted, loop
    // through until the model is found or not.
    for(insert_pos = hash(modelName); model[insert_pos].model_name != NULL;
        insert_pos++)
    {
        // Check to see if model is loaded at position
        if(strcmp(model[insert_pos].model_name, modelName) == 0)
            return insert_pos;
        
        // Loop around if at end of circular hash table
        if(insert_pos + 1 >= MAX_MODELS)
            insert_pos -= MAX_MODELS;
    }
    
    // Otherwise load model from disk if loading on-the-fly is enabled
    if(load_otf)
        return loadModel(modelName);
        
    // Else produce error (return -1)
    return -1;
}

/*******************************************************************************
    function    :   int model_library::getMeshID
    arguments   :   id - Model Library reference ID tag number
                    meshName - mesh's name to find 
    purpose     :   Returns the mesh_id tag number for the given Mesh name and
                    model ID.
    notes       :   <none>
*******************************************************************************/
int model_library::getMeshID(int id, char* meshName)
{
    // Go through our mesh list for specified object and look for the mesh
    // named meshName. Wish there was a better way to do this but... oh well.
    for(int i = 0; i < model[id].mesh_count; i++)
        if(strcmp(model[id].mesh[i].mesh_name, meshName) == 0)
            return i;                       // Return index
            
    return -1;
}

/*******************************************************************************
    function    :   model_library::buildDistanceValues
    arguments   :   id - Model Library reference ID tag number
    purpose     :   Builds the D values associated with mesh data.
    notes       :   Seperate function from loading routine so that mesh offsets
                    can be applied and then D values computed afterwords.
*******************************************************************************/
void model_library::buildDistanceValues(int id)
{
    int i, j;
    
    for(i = 0; i < model[id].mesh_count; i++)
    {
        for(j = 0; j < model[id].mesh[i].vertex_count; j++)
        {
            // Distance Value (dot product normal vector with position
            // vector).
            model[id].mesh[i].d_data[j] = -dotProduct(
                kVector(model[id].mesh[i].normal_data[j]),
                kVector(model[id].mesh[i].vertex_data[j]));
        }
    }
}

/*******************************************************************************
    function    :   model_library::buildMeshMinMaxValues
    arguments   :   id - Model Library reference ID tag number
    purpose     :   Builds the min/max vertex values associated with mesh data.
    notes       :   Seperate function from loading routine so that mesh offsets
                    can be applied and then min/max values computed afterwords.
*******************************************************************************/
void model_library::buildMeshMinMaxValues(int id)
{
    int i, j;
    
    for(i = 0; i < model[id].mesh_count; i++)
    {
        for(j = 0; j < model[id].mesh[i].vertex_count; j++)
        {
            // Update local min and max for mesh
            if(model[id].mesh[i].vertex_data[j][0] <
                model[id].mesh[i].min[0])
                model[id].mesh[i].min[0] = 
                    model[id].mesh[i].vertex_data[j][0];
            if(model[id].mesh[i].vertex_data[j][0] >
                model[id].mesh[i].max[0])
                model[id].mesh[i].max[0] = 
                    model[id].mesh[i].vertex_data[j][0];
            if(model[id].mesh[i].vertex_data[j][1] <
                model[id].mesh[i].min[1])
                model[id].mesh[i].min[1] = 
                    model[id].mesh[i].vertex_data[j][1];
            if(model[id].mesh[i].vertex_data[j][1] >
                model[id].mesh[i].max[1])
                model[id].mesh[i].max[1] = 
                    model[id].mesh[i].vertex_data[j][1];
            if(model[id].mesh[i].vertex_data[j][2] <
                model[id].mesh[i].min[2])
                model[id].mesh[i].min[2] = 
                    model[id].mesh[i].vertex_data[j][2];
            if(model[id].mesh[i].vertex_data[j][2] >
                model[id].mesh[i].max[2])
                model[id].mesh[i].max[2] = 
                    model[id].mesh[i].vertex_data[j][2];
        }
    }
}

/*******************************************************************************
    function    :   model_library::setMeshOffset
    arguments   :   id - Model Library reference ID tag number
                    mesh - Object mesh to work off of
                    polyOffset - a 3 valued float offset to apply to the mesh
    purpose     :   Offsets the mesh by the passed float array.
    notes       :   
*******************************************************************************/
void model_library::setMeshPolyOffset(int id, int mesh, float* polyOffset)
{
    int i = 0;
    float offset[3];
    
    // Check for correct mesh (this is not an error in some instances, but
    // could be viewed as merely a check for existance of the mesh).
    if(mesh == -1)
        return;
    
    offset[0] = -polyOffset[0] - model[id].mesh[mesh].poly_offset[0];
    offset[1] = -polyOffset[1] - model[id].mesh[mesh].poly_offset[1];
    offset[2] = -polyOffset[2] - model[id].mesh[mesh].poly_offset[2];
    
    if(offset[0] == 0.0 && offset[1] == 0.0)
        return;
    
    // Offset each and every vertex for said mesh
    for(i = 0; i < model[id].mesh[mesh].vertex_count; i++)
    {
        model[id].mesh[mesh].vertex_data[i][0] += offset[0];
        model[id].mesh[mesh].vertex_data[i][1] += offset[1];
        model[id].mesh[mesh].vertex_data[i][2] += offset[2];
    }

    model[id].mesh[mesh].poly_offset[0] = polyOffset[0];
    model[id].mesh[mesh].poly_offset[1] = polyOffset[1];
    model[id].mesh[mesh].poly_offset[2] = polyOffset[2];
}

/*******************************************************************************
    function    :   model_library::setMeshSTOffset
    arguments   :   id - Model Library reference ID tag number
                    mesh - Object mesh to work off of
                    STOffset - a 2 valued float offset to apply to the ST of mesh
    purpose     :   Offsets the mesh ST by the passed float array.
    notes       :   
*******************************************************************************/
void model_library::setMeshTexelOffset(int id, int mesh, float* texelOffset)
{
    int i = 0;
    float offset[2];
    
    if(mesh == -1)
        return;
    
    offset[0] = texelOffset[0] - model[id].mesh[mesh].texel_offset[0];
    offset[1] = texelOffset[1] - model[id].mesh[mesh].texel_offset[1];
    
    if(offset[0] == 0.0 && offset[1] == 0.0)
        return;
    
    // Offset each and every texel for said mesh
    for(i = 0; i < model[id].mesh[mesh].vertex_count; i++)
    {
        model[id].mesh[mesh].texel_data[i][0] += offset[0];
        model[id].mesh[mesh].texel_data[i][1] += offset[1];
    }

    model[id].mesh[mesh].texel_offset[0] = texelOffset[0];
    model[id].mesh[mesh].texel_offset[1] = texelOffset[1];
}

/*******************************************************************************
    function    :   model_library::drawModel
    arguments   :   id - Model Library reference ID tag number
                    mode - drawing mode (either Immediate or Vertex array)
    purpose     :   Draws an entire object model node and all it's meshes using
                    the supplied mode using OpenGL commands.
    notes       :   1) Due to the usage of textures, calls are automatically
                       made to enable and disable texture mapping (using either
                       glEnable or glEnableClientState).
                    2) Each object mesh has it's own material values, and as
                       such, calls are made to glMaterial upon each mesh node.
                       This can hurt overall performance given the slow process
                       speed of changing material properties (as per GL specs).
*******************************************************************************/
void model_library::drawModel(int id, int mode)
{
    int i, j;
    object_mesh* curr_mesh;
    
    // Safety
    if(id == -1)
        return;
    
    // Switch based upon drawing mode
    switch(mode)
    {
        // Use immediate drawing mode
        case MDL_DRW_IMMEDIATE:
        case MDL_DRW_IMMEDIATE_NO_MATERIAL:
            // Go through all our object meshes and draw them
            for(i = 0; i < model[id].mesh_count; i++)
            {
                // Assign pointer to current object mesh to save on dereference
                curr_mesh = &(model[id].mesh[i]);
                
                if(mode != MDL_DRW_IMMEDIATE_NO_MATERIAL)
                {
                    // Change material properties for object mesh
                    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,
                        curr_mesh->material_data[0]);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,
                        curr_mesh->material_data[1]);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,
                        curr_mesh->material_data[2]);
                    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS,
                        curr_mesh->material_data[3][0]);
                }
                
                // Determine if object mesh has a texture map, of which we
                // execute two different types of code for.
                if(curr_mesh->texture_id == TEXTURE_NULL)
                {
                    // Disable texture mapping
                    glDisable(GL_TEXTURE_2D);
                    
                    // Draw using OpenGL immediate mode
                    glBegin(GL_TRIANGLES);
                        for(j = 0; j < curr_mesh->vertex_count; j++)
                        {
                            glNormal3fv(curr_mesh->normal_data[j]);
                            glVertex3fv(curr_mesh->vertex_data[j]);
                        }
                    glEnd();
                }
                else
                {
                    // Enable texture mapping
                    glEnable(GL_TEXTURE_2D);
                    
                    // Bind texture map
                    glBindTexture(GL_TEXTURE_2D, curr_mesh->texture_id);
                
                    // Draw using OpenGL immediate mode
                    glBegin(GL_TRIANGLES);
                        for(j = 0; j < curr_mesh->vertex_count; j++)
                        {
                            glNormal3fv(curr_mesh->normal_data[j]);
                            glTexCoord2fv(curr_mesh->texel_data[j]);
                            glVertex3fv(curr_mesh->vertex_data[j]);
                        }
                    glEnd();
                }
            }
            break;
        
        // Use vertex arrays for drawing mode
        case MDL_DRW_VERTEXARRAY:
        case MDL_DRW_VERTEXARRAY_NO_MATERIAL:
            // Go through all our object meshes and draw them
            for(i = 0; i < model[id].mesh_count; i++)
            {
                // Assign pointer to current object mesh to save on dereference
                curr_mesh = &(model[id].mesh[i]);
                
                if(mode != MDL_DRW_VERTEXARRAY_NO_MATERIAL)
                {
                    // Change material properties for object mesh
                    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,
                        curr_mesh->material_data[0]);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,
                        curr_mesh->material_data[1]);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,
                        curr_mesh->material_data[2]);
                    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS,
                        curr_mesh->material_data[3][0]);
                }
                
                // Determine if object mesh has a texture map, of which we
                // execute two different types of code for.
                if(curr_mesh->texture_id == TEXTURE_NULL)
                {
                    // Disable texture mapping
                    glDisable(GL_TEXTURE_2D);

                    // Draw using vertex arrays
                    glEnableClientState(GL_NORMAL_ARRAY);
                    glDisableClientState(GL_TEXTURE_COORD_ARRAY);   // No ST map
                    glEnableClientState(GL_VERTEX_ARRAY);
                    
                    glNormalPointer(GL_FLOAT, 0,
                        (void*)curr_mesh->normal_data[0]);
                    glVertexPointer(3, GL_FLOAT, 0,
                        (void*)curr_mesh->vertex_data[0]);
                    
                    glDrawArrays(GL_TRIANGLES, 0, curr_mesh->vertex_count);
                }
                else
                {
                    // Enable texture mapping
                    glEnable(GL_TEXTURE_2D);
                    
                    // Bind texture map
                    glBindTexture(GL_TEXTURE_2D, curr_mesh->texture_id);
                    
                    // Draw using vertex arrays
                    glEnableClientState(GL_NORMAL_ARRAY);
                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                    glEnableClientState(GL_VERTEX_ARRAY);
                    
                    glNormalPointer(GL_FLOAT, 0,
                        (void*)curr_mesh->normal_data[0]);
                    glTexCoordPointer(2, GL_FLOAT, 0,
                        (void*)curr_mesh->texel_data[0]);
                    glVertexPointer(3, GL_FLOAT, 0,
                        (void*)curr_mesh->vertex_data[0]);
                    
                    glDrawArrays(GL_TRIANGLES, 0, curr_mesh->vertex_count);
                }
            }
            break;
    }
}

/*******************************************************************************
    function    :   model_library::drawMesh
    arguments   :   id - Model Library reference ID tag number
                    mesh - Model Library object mesh number from model ID
                    mode - drawing mode (either Immediate or Vertex array)
    purpose     :   Draws an object mesh object using the supplied mode using
                    OpenGL commands.
    notes       :   1) Due to the usage of textures, calls are automatically
                       made to enable and disable texture mapping (using either
                       glEnable or glEnableClientState).
                    2) Each object mesh has it's own material values, and as
                       such, calls are made to glMaterial upon each mesh node.
                       This can hurt overall performance given the slow process
                       speed of changing material properties (as per GL specs).
*******************************************************************************/
void model_library::drawMesh(int id, int mesh, int mode)
{
    int i;
    object_mesh* curr_mesh = &(model[id].mesh[mesh]);
    
    // Safety
    if(id == -1 || mesh == -1)
        return;
    
    if(mode != MDL_DRW_IMMEDIATE_NO_MATERIAL &&
        mode != MDL_DRW_VERTEXARRAY_NO_MATERIAL)
    {
        // Change material properties for object mesh
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,
            curr_mesh->material_data[0]);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,
            curr_mesh->material_data[1]);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,
            curr_mesh->material_data[2]);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS,
            curr_mesh->material_data[3][0]);
    }
    
    // Switch based upon drawing mode
    switch(mode)
    {
        // Use immediate drawing mode
        case MDL_DRW_IMMEDIATE:
        case MDL_DRW_IMMEDIATE_NO_MATERIAL:
            // Determine if object mesh has a texture map, of which we execute
            // two different types of code for.
            if(curr_mesh->texture_id == TEXTURE_NULL)
            {
                // Disable texture mapping
                glDisable(GL_TEXTURE_2D);
                
                // Draw using OpenGL immediate mode                
                glBegin(GL_TRIANGLES);
                    for(i = 0; i < curr_mesh->vertex_count; i++)
                    {
                        glNormal3fv(curr_mesh->normal_data[i]);
                        glVertex3fv(curr_mesh->vertex_data[i]);
                    }
                glEnd();
            }
            else
            {
                // Enable texture mapping
                glEnable(GL_TEXTURE_2D);
                
                // Bind texture map
                glBindTexture(GL_TEXTURE_2D, curr_mesh->texture_id);
            
                // Draw using OpenGL immediate mode
                glBegin(GL_TRIANGLES);
                    for(i = 0; i < curr_mesh->vertex_count; i++)
                    {
                        glNormal3fv(curr_mesh->normal_data[i]);
                        glTexCoord2fv(curr_mesh->texel_data[i]);
                        glVertex3fv(curr_mesh->vertex_data[i]);
                    }
                glEnd();
            }
            break;
        
        // Use vertex arrays for drawing mode
        case MDL_DRW_VERTEXARRAY:
        case MDL_DRW_VERTEXARRAY_NO_MATERIAL:
            // Determine if object mesh has a texture map, of which we execute
            // two different types of code for.
            if(curr_mesh->texture_id == TEXTURE_NULL)
            {
                // Disable texture mapping
                glDisable(GL_TEXTURE_2D);
                
                // Draw using vertex arrays
                glEnableClientState(GL_NORMAL_ARRAY);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);   // No ST map
                glEnableClientState(GL_VERTEX_ARRAY);
                
                glNormalPointer(GL_FLOAT, 0,
                    (void*)curr_mesh->normal_data[0]);
                glVertexPointer(3, GL_FLOAT, 0,
                    (void*)curr_mesh->vertex_data[0]);
                
                glDrawArrays(GL_TRIANGLES, 0, curr_mesh->vertex_count);
            }
            else
            {
                // Enable texture mapping
                glEnable(GL_TEXTURE_2D);
                
                // Bind texture map
                glBindTexture(GL_TEXTURE_2D, curr_mesh->texture_id);
                
                // Draw using vertex arrays
                glEnableClientState(GL_NORMAL_ARRAY);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glEnableClientState(GL_VERTEX_ARRAY);
                
                glNormalPointer(GL_FLOAT, 0,
                    (void*)curr_mesh->normal_data[0]);
                glTexCoordPointer(2, GL_FLOAT, 0,
                    (void*)curr_mesh->texel_data[0]);
                glVertexPointer(3, GL_FLOAT, 0,
                    (void*)curr_mesh->vertex_data[0]);
                
                glDrawArrays(GL_TRIANGLES, 0, curr_mesh->vertex_count);
            }
            break;
    }
}
