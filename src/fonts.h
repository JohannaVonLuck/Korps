/*******************************************************************************
                          Font Module - Definition
*******************************************************************************/

#ifndef FONT_H
#define FONT_H

#define FONT_ARIAL_8                0
#define FONT_ARIAL_10               1
#define FONT_ARIAL_12               2
#define FONT_ARIAL_14               3
#define FONT_ARIAL_16               4
#define FONT_ARIAL_18               5
#define FONT_ARIAL_20               6
#define FONT_ARIAL_22               7
#define FONT_ARIAL_24               8
#define FONT_ARIAL_26               9
#define FONT_ARIAL_28               10
#define FONT_ARIAL_30               11
#define FONT_ARIAL_32               12

#define FONT_COURIER_8              20
#define FONT_COURIER_10             21
#define FONT_COURIER_12             22
#define FONT_COURIER_14             23
#define FONT_COURIER_16             24
#define FONT_COURIER_18             25
#define FONT_COURIER_20             26
#define FONT_COURIER_22             27
#define FONT_COURIER_24             28
#define FONT_COURIER_26             29
#define FONT_COURIER_28             30
#define FONT_COURIER_30             31
#define FONT_COURIER_32             32

#define FONT_GOTHIC_8               40
#define FONT_GOTHIC_10              41
#define FONT_GOTHIC_12              42
#define FONT_GOTHIC_14              43
#define FONT_GOTHIC_16              44
#define FONT_GOTHIC_18              45
#define FONT_GOTHIC_20              46
#define FONT_GOTHIC_22              47
#define FONT_GOTHIC_24              48
#define FONT_GOTHIC_26              49
#define FONT_GOTHIC_28              50
#define FONT_GOTHIC_30              51
#define FONT_GOTHIC_32              52

#define FONT_LUCON_8                60
#define FONT_LUCON_10               61
#define FONT_LUCON_12               62
#define FONT_LUCON_14               63
#define FONT_LUCON_16               64
#define FONT_LUCON_18               65
#define FONT_LUCON_20               66
#define FONT_LUCON_22               67
#define FONT_LUCON_24               68
#define FONT_LUCON_26               69
#define FONT_LUCON_28               70
#define FONT_LUCON_30               71
#define FONT_LUCON_32               72

/*******************************************************************************
    struct      :   font_module
    purpose     :   Font wrapper to SDL's TTF engine.
    notes       :   Must recieve a call to quit() beforing exiting.
*******************************************************************************/
class font_module
{
    private:
        TTF_Font* font_list[3][13];     // TTF containers
        
        TTF_Font* font_grab(int font_type);
        
    public:
        font_module();                  // Constructor
        ~font_module();                 // Deconstructor
        
        /* Generate Text Routines */
        GLubyte* generateText(char* string, int fontType, GLubyte r, GLubyte g, GLubyte b, int &width, int &height);
        inline GLubyte* generateText(char* string, int &width, int &height)
            { return generateText(string, FONT_ARIAL_14, 0xFF, 0xFF, 0xFF, width, height); }
        inline GLubyte* generateText(char* string, int fontType, int &width, int &height)
            { return generateText(string, fontType, 0xFF, 0xFF, 0xFF, width, height); }
        inline GLubyte* generateText(char* string, GLubyte r, GLubyte g, GLubyte b, int &width, int &height)
            { return generateText(string, FONT_ARIAL_14, r, g, b, width, height); }
        
        /* Render Text Routines */
        void renderText(char* string, int fontType, GLubyte r, GLubyte g, GLubyte b, int x, int y);
        inline void renderText(char* string, int x, int y)
            { renderText(string, FONT_ARIAL_14, 0xFF, 0xFF, 0xFF, x, y); }
        inline void renderText(char* string, int fontType, int x, int y)
            { renderText(string, fontType, 0xFF, 0xFF, 0xFF, x, y); }
        inline void renderText(char* string, GLubyte r, GLubyte g, GLubyte b, int x, int y)
            { renderText(string, FONT_ARIAL_14, r, g, b, x, y); }
};

extern font_module fonts;

#endif
