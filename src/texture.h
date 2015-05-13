/*******************************************************************************
                         Texture Library - Definition
*******************************************************************************/

#ifndef TEXTURE_H
#define TEXTURE_H

#define MAX_TEXTURES                647

// SDL_image wrapper functions for RAW image support
GLubyte* loadImage(char* fileName, int bpp, int &width, int &height);
void flipImage(GLubyte* image, int bpp, int width, int height);
GLubyte* copyImage(GLubyte* image, int bpp, int width, int height);
void blendImage(GLubyte* src_image, GLubyte* dest_image, int bpp, int width, int height);
void shrinkImage(GLubyte* &image, int bpp, int &width, int &height);
void blitImage(GLubyte* src_image, GLubyte* dest_image, int bpp, int src_width, int src_height,
    int dest_width, int dest_height, int x_pos, int y_pos);

/*******************************************************************************
    class       :   texture_library
    purpose     :   Base texture library which loads and stores all textures for
                    use in game. This is basically an extension tool which is
                    responsible for loading the textures in a profecient and
                    understandable manner for use in texture mapping.
    notes       :   1) All textures are loaded using the SDL_Image library, of
                       which can load a variety of image formats.
                    2) setLoadOTF controls what is known as On The Fly loading.
                       When enabled, when the program attempts to get a texture
                       id using getTexture, and the said texture is not loaded,
                       getTexture will make a call to load said texture into
                       memory. The reason for the ability to change this func.
                       is due to reasons with the creation of display lists,
                       where each and every call to an OpenGL function is
                       compilied into a display list. To load a texture from
                       disk, the usage of certain OpenGL commands is required,
                       and thus if trying to load an image inside of a display
                       list, BAD THINGS will happen.
                    3) All texture id grabbing is done using a hash table, of
                       which is very proficient at grabbing texture ids while
                       the game is running (uses djb2 hash algorithm).
                    4) All textures are referenced by either their filename
                       or by a name of choosing if the raw image data is only
                       available.
*******************************************************************************/
class texture_library
{
	private:
        /***********************************************************************
            struct      :   texture_node
            purpose     :   Texture storage node for a loaded texture image.
            notes       :   1) The primary key of this structure is the
                               file_name string.
                            2) texture_id is the OpenGL specific ID number
                               assigned to the texture map as loaded into
                               video memory.
                            3) Primary array data structure for TexLib.
        ***********************************************************************/
		struct texture_node
		{
			char* texture_name;
			GLuint texture_id;       // OpenGL specific ID
		};
        
		texture_node texture[MAX_TEXTURES];     // Base array of texture_node
		int texture_count;                      // Load count
        
        bool load_otf;                          // On-The-Fly loading
        GLuint filtering;                       // Texture filtering option
        GLuint wrap_s;                          // Texture S wrapping option
        GLuint wrap_t;                          // Texture T wrapping option
        bool anisotropic;                       // Anisotropic filtering
        
        unsigned int hash(char* string);        // Hash function (djb2)
        
        // Video memory load *Base Routine*
        void register_texture(char* texture_name, int insert_pos,
            GLubyte* image, int bpp, int width, int height);
		
	public:
		texture_library();                      // Constructor
		~texture_library();                     // Deconstructor
     
        /* Base Texture Loading Routines */
        GLuint loadTexture(char* fileName, char* textureName);
        inline GLuint loadTexture(char* fileName)
            { return loadTexture(fileName, fileName); }
        GLuint addTexture(char* textureName, GLubyte* image, int bpp, int width,
            int height);
        
        /* Base ID Grab */
        GLuint getTextureID(char* textureName); // Grabs a texture ID
        
        /* Accessors */
        bool doesTextureExist(char* textureName)
            { load_otf = false; return getTextureID(textureName) != TEXTURE_NULL; }
        
        /* Mutators */
        void setLoadOTF(bool value = true)              // On-The-Fly loading
            { load_otf = value; }
        void setFiltering(GLuint method = GL_LINEAR)    // MIN/MAG Filters
            { filtering = method; }
        void setWrapping(GLuint s_method = GL_REPEAT, GLuint t_method = GL_REPEAT)
            { wrap_s = s_method; wrap_t = t_method; }
        void setLODBias(float value = 0.0f)
            { glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, value); }
        void setAnisotropicy(bool enabled);
        
};

extern texture_library textures;

#endif
