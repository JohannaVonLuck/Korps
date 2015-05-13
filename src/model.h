/*******************************************************************************
                          Model Library - Definition
*******************************************************************************/
#ifndef MODEL_H
#define MODEL_H

#define MAX_MODELS              47

#define MDL_MESH_MAX            30
#define MDL_MESH_MINMAX_START   12345.0

#define MDL_DRW_IMMEDIATE               0
#define MDL_DRW_VERTEXARRAY             1
#define MDL_DRW_IMMEDIATE_NO_MATERIAL   2
#define MDL_DRW_VERTEXARRAY_NO_MATERIAL 3

#define DSPLIST_NULL            0xFFFFFFFF

/*******************************************************************************
    class       :   model_library
    purpose     :   Base model library which loads and stores all 3D models for
                    use in game. This relies on the usage of LIB3DS to load
                    models from disk in the .3ds format and then store the
                    data in a logical and understandable manner.
    notes       :   1) Uses an ID based system similiar to other OpenGL
                       constructs for ease of programming. This ID is valid
                       only for the Model Library and not other OpenGL ops.
                    2) For sake of checking to make sure objects load properly,
                       a loading log integration has been included as initially
                       enabled, which will output to "Models/load.log" data
                       outlining the loading process per each model file. This
                       functionality can be enabled/disabled via setLoadLog.
                    3) All model id grabbing is done using a hash table, of
                       which is very proficient at grabbing model ids while
                       the game is running (uses djb2 hash algorithm).
                    4) All models are referenced by their modelname/filename,
                       and are stored as "Models/model_base/model_name.3ds"
*******************************************************************************/
class model_library
{
    private:
        /***********************************************************************
            struct      :   object_mesh
            purpose     :   An object mesh storage structure for a model's
                            object mesh, capable of storing the main data of any
                            object mesh.
            notes       :   1) All data is stored in a raw/uncompressed format
                               in sequential memory, referenced by it's vertex
                               index.
                            2) texture_id is the OpenGL specific ID number
                               assigned and can be assigned via TexLib.
                            3) Public structure which is based on the idea of
                               ModLib public access.
        ***********************************************************************/
        struct object_mesh
        {
            char* mesh_name;
            
            int vertex_count;
            GLfloat** vertex_data;
            GLfloat** texel_data;
            GLfloat** normal_data;
            GLfloat* d_data;            // Distance value (normal . vector)
            
            GLfloat** material_data;
            GLuint texture_id;          // OpenGL specific ID
            
            float poly_offset[3];
            float texel_offset[2];
            
            float min[3];
            float max[3];
        };
        
        /***********************************************************************
            struct      :   model_node
            purpose     :   An entire model storage structure for a model and
                            its object meshes - Model Library specific.
            notes       :   1) model_name and model_base are basically the
                               filename and the file directory for the model in
                               the Model/ directory.
                            2) Used by ModLib as it's primary array data
                               structure.
                            3) Public structure which is based on the idea of
                               ModLib public access.
        ***********************************************************************/
        struct model_node
        {
            char* model_name;
            char* model_base;
            
            object_mesh* mesh;
            int mesh_count;
            
            float radius;
            float min[3];
            float max[3];
        };
        
        model_node model[MAX_MODELS];       // Base array of model_node objects
        int model_count;
        
        bool load_log;                      // Load logging
        bool load_otf;                      // On-The-Fly loading
        
        unsigned int hash(char* string);    // Hash function (djb2)
        
    public:
        model_library();                    // Constructor
        ~model_library();                   // Deconstructor
        
        int loadModel(char* modelName);         // Loads a model from disk
        
        /* Base ID Grab and Model Load Routines */
        int getModelID(char* modelName);        // Grabs a model ID
        int getMeshID(int id, char* meshName);  // Grabs an object mesh ID
        
        /* Misc. Routines */
        void buildDistanceValues(int id);
        void buildDistanceValues(char* modelName)
            { buildDistanceValues(getModelID(modelName)); }
        
        void buildMeshMinMaxValues(int id);
        void buildMeshMinMaxValues(char* modelName)
            { buildMeshMinMaxValues(getModelID(modelName)); }
        
        /* Accessors */
        bool doesModelExist(char* modelName)
            { load_otf = false; return getModelID(modelName) != -1; }
        bool doesMeshExist(int id, char* meshName)
            { load_otf = false; return getMeshID(id, meshName) != -1; }
        
        int getMeshCount(int id)
            { return model[id].mesh_count; }
        int getVertexCount(int id, int mesh)
            { return model[id].mesh[mesh].vertex_count; }
        int getVertexCount(int id, char* meshName)
            { return model[id].mesh[getMeshID(id, meshName)].vertex_count; }
        
        char* getModelName(int id)
            { return model[id].model_name; }
        char* getModelBase(int id)
            { return model[id].model_base; }
        char* getMeshName(int id, int mesh)
            { return model[id].mesh[mesh].mesh_name; }
        
        GLfloat** getVertexData(int id, int mesh)
            { return model[id].mesh[mesh].vertex_data; }
        GLfloat** getVertexData(int id, char* meshName)
            { return model[id].mesh[getMeshID(id, meshName)].vertex_data; }
        
        GLfloat** getTexelData(int id, int mesh)
            { return model[id].mesh[mesh].texel_data; }
        GLfloat** getTexelData(int id, char* meshName)
            { return model[id].mesh[getMeshID(id, meshName)].texel_data; }
        
        GLfloat** getNormalData(int id, int mesh)
            { return model[id].mesh[mesh].normal_data; }
        GLfloat** getNormalData(int id, char* meshName)
            { return model[id].mesh[getMeshID(id, meshName)].normal_data; }
        
        GLfloat* getDistanceData(int id, int mesh)
            { return model[id].mesh[mesh].d_data; }
        GLfloat* getDistanceData(int id, char* meshName)
            { return model[id].mesh[getMeshID(id, meshName)].d_data; }
        
        GLuint getTextureID(int id, int mesh)
            { return model[id].mesh[mesh].texture_id; }
        GLuint getTextureID(int id, char* meshName)
            { return model[id].mesh[getMeshID(id, meshName)].texture_id; }
        
        float* getMinMeshSize(int id, int mesh)
            { return model[id].mesh[mesh].min; }
        float* getMaxMeshSize(int id, int mesh)
            { return model[id].mesh[mesh].max; }
        float* getMinSize(int id)
            { return model[id].min; }
        float* getMaxSize(int id)
            { return model[id].max; }
        float getRadius(int id)
            { return model[id].radius; }
        
        /* Mutators */
        void setLoadOTF(bool value = true)      // OTF loading
            { load_otf = value; }
        void setLoadLog(bool enable = true)
            { load_log = enable; }
        
        /* Offset Mutators */
        void setMeshPolyOffset(int id, int mesh, float* polyOffset);
        void setMeshPolyOffset(int id, char* meshName, float* polyOffset)
            { setMeshPolyOffset(id, getMeshID(id, meshName), polyOffset); }
        void setMeshTexelOffset(int id, int mesh, float* texelOffset);
        void setMeshTexelOffset(int id, char* meshName, float* texelOffset)
            { setMeshTexelOffset(id, getMeshID(id, meshName), texelOffset); }
        
        /* Drawing Routines */
        void drawModel(int id, int mode);
        
        void drawMesh(int id, int mesh, int mode);
        void drawMesh(int id, char* meshName, int mode)
            { drawMesh(id, getMeshID(id, meshName), mode); }
};

extern model_library models;

#endif
