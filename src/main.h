/*******************************************************************************
                            Common Header Includes
*******************************************************************************/
/*  Standard Includes  */
#include<iostream>
#include<iomanip>
#include<fstream>
#include<cstdlib>
#include<cstring>
#include<cmath>
#include<cstdio>
#include<ctime>
using namespace std;

#if !defined(_WIN32)
/*  Linux Specific  */
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#else
/*  Win32 Specific  */
#include<windows.h>
#include<direct.h>
#include<io.h>
#endif

/*  OpenGL Library  */
#include<GL/gl.h>
#include<GL/glu.h>

/*  SDL Library  */
#include<SDL/SDL.h>
#include<SDL/SDL_image.h>
#include<SDL/SDL_ttf.h>

/*  OpenAL Library  */
#include<AL/al.h>
#include<AL/alc.h>
//#include<AL/alut.h>
#include<vorbis/vorbisfile.h>

/*  LIB3DS Library  */
#include<lib3ds/file.h>
#include<lib3ds/camera.h>
#include<lib3ds/mesh.h>
#include<lib3ds/node.h>
#include<lib3ds/material.h>
#include<lib3ds/matrix.h>
#include<lib3ds/vector.h>
#include<lib3ds/light.h>

/*  Win32/Linux Compatibility  */
#if !defined(GL_BGR)
#define GL_BGR      GL_BGR_EXT
#endif
#if !defined(GL_BGRA)
#define GL_BGRA     GL_BGRA_EXT
#endif
#if !defined(GL_TEXTURE_LOD_BIAS)
#define GL_TEXTURE_LOD_BIAS               0x8501
#endif
#if !defined(GL_TEXTURE_FILTER_CONTROL)
#define GL_TEXTURE_FILTER_CONTROL         0x8500
#endif
#if !defined(GL_TEXTURE_FILTER_CONTROL_EXT)
#define GL_TEXTURE_FILTER_CONTROL_EXT     0x8500
#endif
#if !defined(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT)
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif
#if !defined(GL_TEXTURE_MAX_ANISOTROPY_EXT)
#define GL_TEXTURE_MAX_ANISOTROPY_EXT     0x84FE
#endif
#if !defined(GL_CLAMP_TO_EDGE)
#define GL_CLAMP_TO_EDGE                  0x812F
#endif

#ifndef MAIN_H
#define MAIN_H

#define DISPLAY_NULL                0xFFFFFFFF
#define TEXTURE_NULL                0xFFFFFFFF
#define SOUND_NULL                  0x7FFFFFFF
#define EFFECT_NULL                 0x7FFFFFFF
#define JOB_NULL                    0x00000000

#define ALPHA_PASS                  0.80

#define PS_AXIS                     0
#define PS_ALLIED                   1
#define PN_GERMANY                  0
#define PN_POLAND                   1
#define PN_FRANCE                   2
#define PN_BRITIAN                  3
#define PN_BELGIUM                  4
#define SC_COVERAGE_NONE            0
#define SC_COVERAGE_SPARSE          1
#define SC_COVERAGE_MODERATE        2
#define SC_COVERAGE_DENSE           3
#define CR_PEN_SYSTEM_US            0
#define CR_PEN_SYSTEM_RU            1

/*******************************************************************************
                             Game Options Control
*******************************************************************************/
struct korps_options
{
    // Game Options
    int tree_coverage;
    int scenery_elements;
    
    // Controls
    int scroll_speed;
    int rotate_speed;
    
    // Video
    bool weather;
    
    // Sound
    bool sound;
    int sound_volume;
    int max_sounds;
    
    // Realism
    int pen_system;
    bool diameter_fix;
};

extern korps_options game_options;

/*******************************************************************************
                             Game Setup Control
*******************************************************************************/
struct korps_setup
{
    // Mission Setup
    char mission_folder[128];
    int player_side;
    int player_nation;
    
    // Video Setup
    int screen_width;
    int screen_height;
    bool full_screen;
    GLuint filtering;
    bool anisotropic;
    GLfloat lod_bias;
};

extern korps_setup game_setup;

/*******************************************************************************
                              Global Variables
*******************************************************************************/
/*  Globals  */
extern int frameCount;              // Frame counter


#endif
