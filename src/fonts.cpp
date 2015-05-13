/*******************************************************************************
                        Font Module - Implementation
*******************************************************************************/
#include "main.h"
#include "fonts.h"
#include "misc.h"
#include "texture.h"

/*******************************************************************************
    function    :   font_module::font_module
    arguments   :   <none>
    purpose     :   Constructor.
    notes       :   <none>
*******************************************************************************/
font_module::font_module()
{
    int i, j;
    
    // Initialize all fonts to NULL (e.g. not loaded)
    for(i = 0; i < 3; i++)
        for(j = 0; j < 13; j++)
            font_list[i][j] = NULL;
}

/*******************************************************************************
    function    :   font_module::font_module
    arguments   :   <none>
    purpose     :   Deconstructor.
    notes       :   <none>
*******************************************************************************/
font_module::~font_module()
{
    int i, j;
    
    // Free all loaded TTF containers
    for(i = 0; i < 3; i++)
        for(j = 0; j < 13; j++)
            if(font_list[i][j] != NULL)
                TTF_CloseFont(font_list[i][j]);
}

/*******************************************************************************
    function    :   TTF_Font* font_module::font_grab
    arguments   :   font_type - Font type to grab
    purpose     :   Returns a pointer to the TTF container of the given font
                    type relative to this module.
    notes       :   <none>
*******************************************************************************/
TTF_Font* font_module::font_grab(int font_type)
{
    if(font_type >= FONT_ARIAL_8 && font_type <= FONT_ARIAL_32)
    {
        // See if font is loaded (if not then load it)
        if(font_list[0][font_type - FONT_ARIAL_8] == NULL)
            font_list[0][font_type - FONT_ARIAL_8] =
                TTF_OpenFont("Misc/arial.ttf",
                    8 + ((font_type - FONT_ARIAL_8) * 2));
        return font_list[0][font_type - FONT_ARIAL_8];
    }
    else if(font_type >= FONT_COURIER_8 && font_type <= FONT_COURIER_32)
    {
        // See if font is loaded (if not then load it)
        if(font_list[1][font_type - FONT_COURIER_8] == NULL)
            font_list[1][font_type - FONT_COURIER_8] =
                TTF_OpenFont("Misc/courier.ttf",
                    8 + ((font_type - FONT_COURIER_8) * 2));
        return font_list[1][font_type - FONT_COURIER_8];
    }
    else if(font_type >= FONT_GOTHIC_8 && font_type <= FONT_GOTHIC_32)
    {
        // See if font is loaded (if not then load it)
        if(font_list[2][font_type - FONT_GOTHIC_8] == NULL)
            font_list[2][font_type - FONT_GOTHIC_8] =
                TTF_OpenFont("Misc/gothic.ttf",
                    8 + ((font_type - FONT_GOTHIC_8) * 2));
        return font_list[2][font_type - FONT_GOTHIC_8];
    }
    else if(font_type >= FONT_LUCON_8 && font_type <= FONT_LUCON_32)
    {
        // See if font is loaded (if not then load it)
        if(font_list[2][font_type - FONT_LUCON_8] == NULL)
            font_list[2][font_type - FONT_LUCON_8] =
                TTF_OpenFont("Misc/lucon.ttf",
                    8 + ((font_type - FONT_LUCON_8) * 2));
        return font_list[2][font_type - FONT_LUCON_8];
    }
    
    return NULL;
}

/*******************************************************************************
    function    :   GLubyte* font_module::generateText
    arguments   :   string - Text string (ASCII) to render
                    fontType - Font type selection
                    r,g,b - Color of generated text
                    width, height - Width and height of generated text (passed
                                    by reference, changed in function)
    purpose     :   Generates an RGBA (32bpp) raw pixel surface of the text
                    string passed with the given parameters.
    notes       :   Efficiency demands that the text be generated once and then
                    stored for all future usage - remember to delete memory when
                    finished (memory is dynamically allocated).
*******************************************************************************/
GLubyte* font_module::generateText(char* string, int fontType, GLubyte r,
    GLubyte g, GLubyte b, int &width, int &height)
{
    TTF_Font* font_container;
    SDL_Color text_color = {r, g, b, 0xFF};
    SDL_Surface* text;
    GLubyte* image;
    int i, j;
    
    // Grab the font requested
    font_container = font_grab(fontType);
    
    // Check for successful grab
    if(font_container == NULL)
        return NULL;
    
    // Render string into a temporary portion of memory
    text = TTF_RenderText_Blended(font_container, string, text_color);
    
    // Lock surface for direct pixel access
    SDL_LockSurface(text);
    
    // Set width and height variables (passed by reference)
    width = text->w;
    height = text->h;
    
    // Create storage array for text image
    image = new GLubyte[width * height * 4];
    
    for(i = 0; i < height; i++)
    {
        for(j = 0; j < width; j++)
        {
            // Copy over RGBA
            image[((i * width) + j) * 4]     =
                ((GLubyte*)text->pixels)[(i * text->pitch) + (j * 4)];
            image[((i * width) + j) * 4 + 1] =
                ((GLubyte*)text->pixels)[(i * text->pitch) + (j * 4) + 1];
            image[((i * width) + j) * 4 + 2] =
                ((GLubyte*)text->pixels)[(i * text->pitch) + (j * 4) + 2];
            image[((i * width) + j) * 4 + 3] =
                ((GLubyte*)text->pixels)[(i * text->pitch) + (j * 4) + 3];
        }
    }
    
    // Unlock surface
    SDL_UnlockSurface(text);
    
    // Free surface
    SDL_FreeSurface(text);
    
    // Return text image data pointer
    return image;
}

/*******************************************************************************
    function    :   font_module::renderText
    arguments   :   string - Text string (ASCII) to render
                    fontType - Font type selection
                    r,g,b - Color of generated text
                    x,y - Position on screen to write text to
    purpose     :   Renders a self-generated RGBA (32bpp) raw pixel surface of
                    the text string passed with the given parameters at the
                    x, y offset given (in OpenGL coordinates).
    notes       :   Unefficient method used purely for immediate writing to
                    the screen buffer for those "quick and dirty" renders.
*******************************************************************************/
void font_module::renderText(char* string, int fontType, GLubyte r,
    GLubyte g, GLubyte b, int x, int y)
{
    TTF_Font* font_container;
    SDL_Color text_color = {r, g, b, 0xFF};
    SDL_Surface* text;
    int i;
    
    // Grab the font requested
    font_container = font_grab(fontType);
    
    // Check for successful grab
    if(font_container == NULL)
        return;
    
    // Render string into a temporary portion of memory
    text = TTF_RenderText_Blended(font_container, string, text_color);
    
    // Lock surface for direct pixel access
    SDL_LockSurface(text);
    
    // Write out each row (since OpenGL does it upside down)
    for(i = 0; i < text->h; i++)
    {
        glRasterPos2i(x, y++);
        glDrawPixels(text->w, 1, GL_RGBA, GL_UNSIGNED_BYTE,
            (void*)&((GLubyte*)text->pixels)[i * text->pitch]);
    }
    
    // Unlock surface
    SDL_UnlockSurface(text);
    
    // Free surface
    SDL_FreeSurface(text);
}
