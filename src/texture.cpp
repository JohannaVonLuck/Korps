/*******************************************************************************
                        Texture Library - Implementation
*******************************************************************************/
#include "main.h"
#include "texture.h"
#include "misc.h"

/*******************************************************************************
    function    :   GLubyte* loadImage
    arguments   :   fileName - Image to load
                    bpp - Bits per pixel requested (8 for GS, 24/32 for RGB/A)
                    width - Width variable to be changed (passed by ref)
                    height - Height variable to be changed (passed by ref)
    purpose     :   Wrapper function which makes use of SDL to load the passed
                    image file and return an array of data of which is in
                    a RAW format - no headers or fancy junk.
    notes       :   1) Only 8, 24, or 32 bpp is currently supported.
                    2) 24/32bpp images are automatically assumed to be read in
                       the BGR format, and are reveresed into RGB.
                    3) 32bpp images are automatically scanned for purple color
                       and such pixels are replaced with full transparency. This
                       is only done to .jpg, .bmp, and .gif image types. The
                       color purple is defined where R >= 0xF0, G <= 0x0F, and
                       B >= 0xF0.
*******************************************************************************/
GLubyte* loadImage(char* fileName, int bpp, int &width, int &height)
{
    SDL_Surface* image = NULL;
    GLubyte* new_image;
    int x, y;
    int Bpp = bpp / 8;
    bool flip_rgb = true;
    bool set_alpha = false;
    char buffer[128];
    int offset_1, offset_2;
    
    // Pixel format descriptors for 8, 24, and 32 bpp
    SDL_PixelFormat pfRGBA =
        {NULL, 32, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    SDL_PixelFormat pfRGB =
        {NULL, 24, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    SDL_PixelFormat pfGS =
        {NULL, 8, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    
    // Check to make sure a valid bpp was passed
    if(bpp != 8 && bpp != 24 && bpp != 32)
    {
        // Write error message and return NULL
        sprintf(buffer, "Image: Unhandled bpp specified for \"%s\".",
            fileName);
        write_error(buffer);
        return NULL;
    }
    
    // Load image
    image = IMG_Load(fileName);
    
    // Check for open (don't write error message)
    if(!image)
        return NULL;
    
    // Do not flip R<->B
    if(bpp == image->format->BitsPerPixel)
        flip_rgb = false;
    
    // See if we will need to set alpha values manually
    if(bpp == 32 && image->format->BitsPerPixel < 32)
        set_alpha = true;
    
    // Check for correct format & convert if neccessary
    if(image->format->BitsPerPixel != bpp ||
       image->format->BytesPerPixel != Bpp)
    {
        SDL_Surface* temp = NULL;
        
        if(bpp == 8)
            temp = SDL_ConvertSurface(image, &pfGS, SDL_SWSURFACE);
        else if(bpp == 24)
            temp = SDL_ConvertSurface(image, &pfRGB, SDL_SWSURFACE);
        else if(bpp == 32)
            temp = SDL_ConvertSurface(image, &pfRGBA, SDL_SWSURFACE);
        
        if(temp != NULL)
        {
            SDL_FreeSurface(image);
            image = temp;
        }
    }
    
    // Mutex lock
    SDL_LockSurface(image);
    
    // Set width and height of image
    width = image->w;
    height = image->h;
    
    // Create new array for our new raw image
    new_image = new GLubyte[width * height * Bpp];
    
    // Copy over data from current array into new array and perform transparency
    // checks of any nature.
    for(y = 0; y < height; y++)
        for(x = 0; x < width; x++)
        {
            offset_1 = ((y * width) + x) * Bpp;
            offset_2 = (image->pitch * y) + (x * Bpp);
            
            new_image[offset_1] = ((GLubyte*)image->pixels)[offset_2];
            
            if(bpp >= 24)
            {
                new_image[offset_1 + 1] = ((GLubyte*)image->pixels)[offset_2 + 1];
                new_image[offset_1 + 2] = ((GLubyte*)image->pixels)[offset_2 + 2];
                
                // Flip BGR to RGB
                if(flip_rgb)
                {
                    GLubyte temp = new_image[offset_1];
                    new_image[offset_1] = new_image[offset_1 + 2];
                    new_image[offset_1 + 2] = temp;
                }
                
                if(bpp == 32)
                {
                    new_image[offset_1 + 3] =
                        ((GLubyte*)image->pixels)[offset_2 + 3];
                    
                    if(new_image[offset_1] >= 0xF0 &&
                       new_image[offset_1 + 1] <= 0x0F &&
                       new_image[offset_1 + 2] >= 0xF0)
                    {
                        new_image[offset_1] = 0x00;
                        new_image[offset_1 + 1] = 0x00;
                        new_image[offset_1 + 2] = 0x00; 
                        new_image[offset_1 + 3] = 0x00;
                    }
                    else if(set_alpha)
                        new_image[offset_1 + 3] = 0xFF;
                }
            }
        }
    
    // Mutex unlock
    SDL_UnlockSurface(image);
    
    // Free surface
    SDL_FreeSurface(image);
    
    // Return newly allocated array
    return new_image;
}

/*******************************************************************************
    function    :   flipImage
    arguments   :   image - Pointer to raw image data
                    bpp - Bits per pixel that image exists in
                    width - Width of image
                    height - Height of image
    purpose     :   Flips the image in memory to be upside-down. This is a
                    neccessity given that OpenGL uses a 0,0 that is in the
                    bottom left hand corner instead of the top left hand
                    corner. This is provided as a seperate function since not
                    all images will need to be flipped.
    notes       :   <none>
*******************************************************************************/
void flipImage(GLubyte* image, int bpp, int width, int height)
{
    int x, y;
    int offset_1;
    int offset_2;
    int Bpp = bpp / 8;
    GLubyte temp;
    
    // Check to make sure a valid bpp and image data pointer was passed
    if((bpp != 8 && bpp != 24 && bpp != 32) || image == NULL)
        return;         // Else return
    
    for(y = 0; y < height / 2; y++)
        for(x = 0; x < width; x++)
        {
            // Calculate offsets - we meet half-way in the middle.
            offset_1 = (y * width + x) * Bpp;
            offset_2 = ((height - y - 1) * width + x) * Bpp;

            // Flip entirety of image based on bpp
            if(bpp >= 8)
            {
                temp = image[offset_1];
                image[offset_1] = image[offset_2];
                image[offset_2] = temp;
            
                if(bpp >= 24)
                {
                    temp = image[offset_1 + 1];
                    image[offset_1 + 1] = image[offset_2 + 1];
                    image[offset_2 + 1] = temp;
    
                    temp = image[offset_1 + 2];
                    image[offset_1 + 2] = image[offset_2 + 2];
                    image[offset_2 + 2] = temp;
                
                    if(bpp >= 32)
                    {
                        temp = image[offset_1 + 3];
                        image[offset_1 + 3] = image[offset_2 + 3];
                        image[offset_2 + 3] = temp;
                    }
                }
            }
        }
}

/*******************************************************************************
    function    :   copyImage
    arguments   :   image - Pointer to raw image data to copy
                    bpp - Bits per pixel that image exists in
                    width - Width of image
                    height - Height of image
    purpose     :   Copies passed image verbatim as a newly allocated image.
    notes       :   <none>
*******************************************************************************/
GLubyte* copyImage(GLubyte* image, int bpp, int width, int height)
{
    GLubyte* new_image;
    int Bpp = bpp / 8;
    int i;
    
    // Check to make sure a valid bpp and image data pointer was passed
    if((bpp != 8 && bpp != 24 && bpp != 32) || image == NULL)
        return NULL;    // Else return
    
    // Allocate memory for new image
    new_image = new GLubyte[width * height * Bpp];
    
    // Copy over data verbatim
    for(i = (width * height * Bpp) - 1; i >= 0; i--)
        new_image[i] = image[i];
    
    // Return new image
    return new_image;
}

/*******************************************************************************
    function    :   blendImage
    arguments   :   src_image - Pointer to raw image data for source
                    dest_image - Pointer to raw image data for destination
                    bpp - Bits per pixel that both images exist in (24 or 32)
                    width - Width of both images
                    height - Height of both images
    purpose     :   Blends togther two images of the same type and dimensions
                    using either blending or purple replacement.
    notes       :   1) src_image is always being applied overtop of dest_image.
                    2) Result is always stored in dest_image.
                    3) 32bpp images will combine using smooth alpha blending.
                    4) 24bpp images will combine using purple replacement.
                    5) 8bpp images are not allowed.
                    6) Currently, both images MUST be same dimensions and bpp.
*******************************************************************************/
void blendImage(GLubyte* src_image, GLubyte* dest_image, int bpp, int width,
    int height)
{
    int x, y;
    int offset;
    double dfactor;     // Destination scaling factor
    double sfactor;     // Source scaling factor
    
    // Check to make sure a valid bpp and image data pointer was passed
    if((bpp != 24 && bpp != 32) || src_image == NULL || dest_image == NULL)
        return;         // Else return
    
    if(bpp == 32)
    {
        for(y = 0; y < height; y++)
            for(x = 0; x < width; x++)
            {
                offset = ((y * width) + x) * 4;
                
                dfactor = (double)(int)dest_image[offset + 3] / 255.0;
                sfactor = 1.0 - dfactor;
                
                dest_image[offset] =
                    (GLubyte)(int)min_value(255.0,
                    ((double)dest_image[offset] * dfactor) +
                    ((double)src_image[offset] * sfactor));
                dest_image[offset + 1] =
                    (GLubyte)(int)min_value(255.0,
                    ((double)dest_image[offset + 1] * dfactor) +
                    ((double)src_image[offset + 1] * sfactor));
                dest_image[offset + 2] =
                    (GLubyte)(int)min_value(255.0,
                    ((double)dest_image[offset + 2] * dfactor) +
                    ((double)src_image[offset + 2] * sfactor));
                dest_image[offset + 3] = 0xFF;
            }
    }
    else if(bpp == 24)
    {
        for(y = 0; y < height; y++)
            for(x = 0; x < width; x++)
            {
                offset = ((y * width) + x) * 3;
                
                // Purple replacement method
                if(dest_image[offset] >= 0xF0 &&
                   dest_image[offset + 1] <= 0x0F &&
                   dest_image[offset + 2] >= 0xF0)
                {
                    dest_image[offset] = src_image[offset];
                    dest_image[offset + 1] = src_image[offset + 1];
                    dest_image[offset + 2] = src_image[offset + 2];    
                }
            }
    }
}

/*******************************************************************************
    function    :   shrinkImage
    arguments   :   image - Pointer to raw image data (gets modified)
                    bpp - Bits per pixel that both images exist in (24 or 32)
                    width - Width of both images
                    height - Height of both images
    purpose     :   Shrinks an image by exactly 1/2 it's size.
    notes       :   Used in mip making. Props out to Dr. Weiss.
*******************************************************************************/
void shrinkImage(GLubyte* &image, int bpp, int &width, int &height)
{
    int x, y;
    int new_width;
    int new_height;
    int Bpp = bpp / 8;
    GLubyte* new_image;
    
    // Check to make sure a valid bpp and image data pointer was passed
    if((bpp != 8 && bpp != 24 && bpp != 32) || image == NULL)
        return;         // Else return
    
    // Set up new image
    new_width = width / 2;
    if(new_width < 1) new_width = 1;
    new_height = height / 2;
    if(new_height < 1) new_height = 1;
    new_image = new GLubyte[new_width * new_height * Bpp];
    
    for(y = 0; y < new_height; y++)
    {
        for(x = 0; x < new_width; x++)
        {
            new_image[(((y * new_width) + x) * Bpp) + 0] =
                (image[((((y * 2) * width) + (x * 2)) * Bpp) + 0] +
                 image[((((y * 2) * width) + ((x * 2) + 1)) * Bpp) + 0] +
                 image[(((((y * 2) + 1) * width) + (x * 2)) * Bpp) + 0] +
                 image[(((((y * 2) + 1) * width) + ((x * 2) + 1)) * Bpp) + 0]) / 4;
            
            if(bpp >= 24)
            {
                new_image[((((y * new_width) + x)) * Bpp) + 1] =
                    (image[((((y * 2) * width) + (x * 2)) * Bpp) + 1] +
                     image[((((y * 2) * width) + ((x * 2) + 1)) * Bpp) + 1] +
                     image[(((((y * 2) + 1) * width) + (x * 2)) * Bpp) + 1] +
                     image[(((((y * 2) + 1) * width) + ((x * 2) + 1)) * Bpp) + 1]) / 4;
                
                new_image[((((y * new_width) + x)) * Bpp) + 2] =
                    (image[((((y * 2) * width) + (x * 2)) * Bpp) + 2] +
                     image[((((y * 2) * width) + ((x * 2) + 1)) * Bpp) + 2] +
                     image[(((((y * 2) + 1) * width) + (x * 2)) * Bpp) + 2] +
                     image[(((((y * 2) + 1) * width) + ((x * 2) + 1)) * Bpp) + 2]) / 4;
                
                if(bpp >= 32)
                    new_image[((((y * new_width) + x)) * Bpp) + 3] =
                        (image[((((y * 2) * width) + (x * 2)) * Bpp) + 3] +
                         image[((((y * 2) * width) + ((x * 2) + 1)) * Bpp) + 3] +
                         image[(((((y * 2) + 1) * width) + (x * 2)) * Bpp) + 3] +
                         image[(((((y * 2) + 1) * width) + ((x * 2) + 1)) * Bpp) + 3]) / 4;
            }
        }
    }
    
    delete image;
    image = new_image;
    height = new_height;
    width = new_width;
}

/*******************************************************************************
    function    :   blitImage
    arguments   :   src_image - Pointer to raw image data for source
                    dest_image - Pointer to raw image data for destination
                    bpp - Bits per pixel that both images exist in (8, 24, or 32)
                    src_width - Width of source image
                    src_height - Height of source image
                    dest_width - Width of destination image
                    dest_height - Height of destination image
    purpose     :   Performs a raw software BLIT of an entire source image onto
                    a destination image at the given top left hand coordinates.
    notes       :   1) src_image is always being applied overtop of dest_image.
                    2) Result is always stored in dest_image.
                    3) 8bpp, 24bpp, and 32bpp images are allowed only.
                    6) Currently, both images MUST be same bpp.
*******************************************************************************/
void blitImage(GLubyte* src_image, GLubyte* dest_image, int bpp, int src_width,
    int src_height, int dest_width, int dest_height, int x_pos, int y_pos)
{
    int x, y;
    int offset_1;
    int offset_2;
    int offset_3;
    int offset_4;
    
    if(!src_image || !dest_image)
        return;
    
    if(bpp == 24)
    {
        for(y = 0; y < src_height; y++)
        {
            offset_3 = ((y + y_pos) * dest_width) + x_pos;
            offset_4 = y * src_width;
            for(x = 0; x < src_width; x++)
            {
                offset_1 = (offset_3 + x) * 3;
                offset_2 = (offset_4 + x) * 3;
                
                dest_image[offset_1] = src_image[offset_2];
                dest_image[offset_1 + 1] = src_image[offset_2 + 1];
                dest_image[offset_1 + 2] = src_image[offset_2 + 2];
            }
        }
    }
    else if(bpp == 32)
    {
        double dfactor;
        double sfactor;
        
        for(y = 0; y < src_height; y++)
        {
            offset_3 = ((y + y_pos) * dest_width) + x_pos;
            offset_4 = y * src_width;
            
            for(x = 0; x < src_width; x++)
            {
                offset_1 = (offset_3 + x) * 4;
                offset_2 = (offset_4 + x) * 4;
                
                sfactor = (double)src_image[offset_2 + 3] / 255.0;
                dfactor = 1.0 - sfactor;
                
                dest_image[offset_1] =
                    (GLubyte)(int)min_value(255.0,
                    ((double)dest_image[offset_1] * dfactor) +
                    ((double)src_image[offset_2] * sfactor));
                dest_image[offset_1 + 1] =
                    (GLubyte)(int)min_value(255.0,
                    ((double)dest_image[offset_1 + 1] * dfactor) +
                    ((double)src_image[offset_2 + 1] * sfactor));
                dest_image[offset_1 + 2] =
                    (GLubyte)(int)min_value(255.0,
                    ((double)dest_image[offset_1 + 2] * dfactor) +
                    ((double)src_image[offset_2 + 2] * sfactor));
                dest_image[offset_1 + 3] = 0xFF;
            }
        }
    }
    else if(bpp == 8)
    {
        for(y = 0; y < src_height; y++)
        {
            offset_1 = ((y + y_pos) * dest_width) + x_pos;
            offset_2 = y * src_width;
            for(x = 0; x < src_width; x++)
                dest_image[offset_1 + x] = src_image[offset_2 + x];
        }
    }
}

/*******************************************************************************
    function    :   texture_library::texture_library
    arguments   :   <none>
    purpose     :   Constructor.
    notes       :   <none>
*******************************************************************************/
texture_library::texture_library()
{
    // Initialize texture library.
	texture_count = 0;
    
    // Initialize all file_name's to NULL
    for(int i = 0; i < MAX_TEXTURES; i++)
    {
        texture[i].texture_name = NULL;
        texture[i].texture_id = TEXTURE_NULL;
    }
        
    // Reset to default options.
    load_otf = true;
    filtering = GL_LINEAR;
    wrap_s = GL_REPEAT;
    wrap_t = GL_REPEAT;
    anisotropic = false;
}

/*******************************************************************************
    function    :   texture_library::~texture_library
    arguments   :   <none>
    purpose     :   Deconstructor.
    notes       :   <none>
*******************************************************************************/
texture_library::~texture_library()
{
    int i;
    
    // Loop through list deleting memory allocated with strdup for file_name
    for(i = 0; i < MAX_TEXTURES; i++)
    {
        if(texture[i].texture_name)
            delete texture[i].texture_name;
        if(texture[i].texture_id != TEXTURE_NULL)
            glDeleteTextures(1, &texture[i].texture_id);
    }
}

/*******************************************************************************
    function    :   unsigned int texture_library::hash
    arguments   :   string - string to hash
    purpose     :   String hash function.
    notes       :   Uses string djb2 hash algorithm.
*******************************************************************************/
unsigned int texture_library::hash(char* string)
{
    unsigned int hash = 5381;
    char* str = string;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash % MAX_TEXTURES;
}

/*******************************************************************************
    function    :   register_texture
    arguments   :   texture_name - Associative name of texture
                    insert_pos - Hash table insert position (must be defined)
                    image - Pointer to raw image data
                    bpp - Bits per pixel (24 or 32 supported only)
                    width - Width of image
                    height - Height of image
    purpose     :   Registers texture with the texture library and loads it
                    into video texture memory.
    notes       :   All textures are assumed to exist either as RGB or RGBA.
*******************************************************************************/
void texture_library::register_texture(char* texture_name, int insert_pos,
    GLubyte* image, int bpp, int width, int height)
{
    int i;
    GLubyte* mip;
    int mip_width;
    int mip_height;
    int Bpp = bpp / 8;
    
    // Check for valid bpp and image data
    if((bpp != 24 && bpp != 32) || image == NULL)
        return;
    
    // Copy texture name over
    texture[insert_pos].texture_name = strdup(texture_name);
    
    // Initialize the texture & set up our texture mapping options.
    glGenTextures(1, &texture[insert_pos].texture_id);
    glBindTexture(GL_TEXTURE_2D, texture[insert_pos].texture_id);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);
    
    // Define wrapping for s,t coordinates
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
    
    // Define anisotropic filtering options (if enabled)
    if(anisotropic)
    {
        GLfloat value;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &value);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, value);
    }
    else
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0.0f);
    
    if(filtering == GL_NEAREST || filtering == GL_LINEAR)
    {
        // Generate base mip for this texture
        glTexImage2D(GL_TEXTURE_2D, 0, Bpp, width, height, 0,
            (bpp == 32 ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, image);
    }
    else if(filtering == GL_NEAREST_MIPMAP_NEAREST ||
            filtering == GL_LINEAR_MIPMAP_NEAREST ||
            filtering == GL_NEAREST_MIPMAP_LINEAR ||
            filtering == GL_LINEAR_MIPMAP_LINEAR)
    {
        // Generate 2D mipmaps for this texture
        //gluBuild2DMipmaps(GL_TEXTURE_2D, Bpp, width, height,
        //    (bpp == 32 ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, image);
        
        glTexImage2D(GL_TEXTURE_2D, 0, Bpp, width, height, 0,
            (bpp == 32 ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, image);
        
        mip = copyImage(image, bpp, width, height);
        mip_width = width;
        mip_height = height;
        
        for(i = 1; mip_width >= 2 || mip_height >= 2; i++)
        {
            shrinkImage(mip, bpp, mip_width, mip_height);
            
            glTexImage2D(GL_TEXTURE_2D, i, Bpp, mip_width, mip_height, 0,
                (bpp == 32 ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, mip);
        }
        
        delete mip;
    }
    
    // Increment count
    texture_count++;
}

/*******************************************************************************
    function    :   int texture_library::loadTexture
    arguments   :   fileName - given texture file to work off of
                :   textureName - name of texture to store and access as
    purpose     :   Loads an image into memory from disk. Stores image texture
                    id tag into texture hash table.
    notes       :   1) Returns the OpenGL texture ID tag, otherwise returns
                       TEXTURE_NULL if in error.
                    2) Multiple loads of same file may result in multiple
                       storage if the textureName is different. Otherwise,
                       similiar textureNames are caught and appropriate ID
                       tag is referenced and returned (without any loading).
*******************************************************************************/
GLuint texture_library::loadTexture(char* fileName, char* textureName)
{
    int insert_pos;
    GLubyte* image;
    int width, height;
    char buffer[128];
    
    // Hash the fileName to determine where to insert the texture, or if it
    // is already loaded then return that position
    for(insert_pos = hash(textureName); texture[insert_pos].texture_name != NULL;
        insert_pos++)
    {
        if(strcmp(texture[insert_pos].texture_name, textureName) == 0)
            return texture[insert_pos].texture_id;
        
        if(insert_pos + 1 >= MAX_TEXTURES)
            insert_pos -= MAX_TEXTURES;
    }
    
    // Check for available space
    if(texture_count >= MAX_TEXTURES)
    {
        write_error("TexLib: Texture count limit reached.");
        return TEXTURE_NULL;
    }
    
    // Load image from disk
    image = loadImage(fileName, 32, width, height);
    
    // Check for sucess
    if(!image)
    {
        sprintf(buffer, "TexLib: Failure loading \"%s\" for read.", fileName);
        write_error(buffer);
        return TEXTURE_NULL;
    }
    
    // Flip image for OpenGL use
    flipImage(image, 32, width, height);
    
    // Register texture with texture library
    register_texture(textureName, insert_pos, image, 32, width, height);
    
    // Free surface
    delete image;
    
    // Return ID
    return texture[insert_pos].texture_id;
}

/*******************************************************************************
    function    :   GLuint texture_library::addTexture
    arguments   :   textureName - Name of texture to reference texture by
                    image - Pointer to raw image data
                    bpp - Bits per pixel that image exists in
                    width - Width of image
                    height - Height of image
    purpose     :   Adds the based raw pixel data to the texture library. Stores
                    image texture id tag into texture hash table.
    notes       :   1) Returns the OpenGL texture ID tag, otherwise returns
                       TEXTURE_NULL if in error.
                    2) Multiple loads of same file does not result in multiple
                       memory storage locations. The code will catch double
                       loads and return the already loaded texture's ID tag.
*******************************************************************************/
GLuint texture_library::addTexture(char* textureName, GLubyte* image, int bpp,
    int width, int height)
{
    int insert_pos;
    char buffer[128];
    
    // Hash the fileName to determine where to insert the texture, or if it
    // is already loaded then return that position
    for(insert_pos = hash(textureName); texture[insert_pos].texture_name != NULL;
        insert_pos++)
    {
        if(strcmp(texture[insert_pos].texture_name, textureName) == 0)
            return texture[insert_pos].texture_id;
        
        if(insert_pos + 1 >= MAX_TEXTURES)
            insert_pos -= MAX_TEXTURES;
    }
    
    // Check for available space
    if(texture_count >= MAX_TEXTURES)
    {
        write_error("TexLib: Texture count limit reached.");
        return TEXTURE_NULL;
    }
    
    // Check image data
    if(!image)
    {
        sprintf(buffer, "TexLib: Cannot add NULL texture: \"%s\".", textureName);
        write_error(buffer);
        return TEXTURE_NULL;
    }
    
    // Flip image for OpenGL use
    flipImage(image, bpp, width, height);
    
    // Register texture with texture library
    register_texture(textureName, insert_pos, image, bpp, width, height);
    
    // Return ID
    return texture[insert_pos].texture_id;
}

/*******************************************************************************
    function    :   GLuint texture_library::getTextureID
    arguments   :   textureName - Name of texture to reference texture by
    purpose     :   Hashes into hash table and attempts to find the texture
                    given (in file name format) and return it's texture ID.
    notes       :   If the texture is not found, and OTF loading is disabled,
                    TEXTURE_NULL is returned. Otherwise, the textureName is
                    treated as a path to a file and that file is attempted
                    to be loaded (hence the name On-The-Fly (OTF) loading).
*******************************************************************************/
GLuint texture_library::getTextureID(char* textureName)
{
    int insert_pos;

    // Start at hash point, continue checking until a blank file is hit.
    for(insert_pos = hash(textureName); texture[insert_pos].texture_name != NULL;
        insert_pos++)
    {    
        // Check for correct file
        if(strcmp(texture[insert_pos].texture_name, textureName) == 0)
            return texture[insert_pos].texture_id;
           
        // Loop-around for hash table
        if(insert_pos + 1 >= MAX_TEXTURES)
            insert_pos -= MAX_TEXTURES;
    }
    
    // Otherwise load texture from disk if loading on-the-fly is enabled
    if(load_otf)
        return loadTexture(textureName);
    
    // Else produce error (return TEXTURE_NULL)
    return TEXTURE_NULL;
}

/*******************************************************************************
    function    :   texture_library::setAnisotropicy
    arguments   :   enabled - Flag to enable or disable anisotropic filtering
    purpose     :   Sets the anisotropic mode. If enabling, it must be
                    determined that the hardware supports anisotropic filtering.
    notes       :   <none>
*******************************************************************************/
void texture_library::setAnisotropicy(bool enabled = false)
{
    const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
    
    if(enabled)
    {
        if(extensions != NULL &&
           strstr(extensions, "GL_EXT_texture_filter_anisotropic") != NULL)
            anisotropic = true;
        else
        {
            write_error(
                "TexLib: Hardware does not support anisotropic filtering.");
            anisotropic = false;
        }
    }
    else
        anisotropic = false;
}
