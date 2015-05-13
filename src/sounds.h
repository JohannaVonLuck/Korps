/*******************************************************************************
                         Sound Module - Definition
*******************************************************************************/

#ifndef SOUND_H
#define SOUND_H

#include "metrics.h"

// Sound play types
#define SOUND_PLAY_ONCE                 0
#define SOUND_PLAY_LOOP                 1

// Sound priority system values
#define SOUND_AMBIENT_PRIORITY          0
#define SOUND_HIGH_PRIORITY             1
#define SOUND_MID_PRIORITY              2
#define SOUND_LOW_PRIORITY              3

// Number of slots reserved for the proity system
#define SOUND_SLOTS_RESERVED_AMBIENT    2
#define SOUND_SLOTS_RESERVED_HIGH       7
#define SOUND_SLOTS_RESERVED_MID        5
#define SOUND_SLOTS_RESERVED_LOW        2

// Number of files found below
#define SOUND_NUM_BUFFERS               20

// Slot number associated with each wav file
#define SOUND_BATTLE                    0
#define SOUND_AMBIENT                   1
#define SOUND_TANKS_PETROL              2
#define SOUND_TANKS_DIESEL              3
#define SOUND_MOTOR_DIE                 4
#define SOUND_CANNON                    5
#define SOUND_MG                        6
#define SOUND_SHELL_RICOCHET_1          7
#define SOUND_SHELL_RICOCHET_2          8
#define SOUND_SHELL_RICOCHET_3          9
#define SOUND_MG_RICHOCHET_1            10
#define SOUND_MG_RICHOCHET_2            11
#define SOUND_MG_RICHOCHET_3            12
#define SOUND_PENETRATE_1               13
#define SOUND_PENETRATE_2               14
#define SOUND_SHELL_THUD                15
#define SOUND_EXPLOSION                 16
#define SOUND_GROUND_EXPLOSION          17
#define SOUND_FLAMES                    18

/*******************************************************************************
   class       :   sound_module
   purpose     :
   notes       :   1)
*******************************************************************************/
class sound_module
{
	private:
		struct sound_object
		{
			ALuint buffer;
			ALuint source;
			int priority;
			float pos[3];
			float dist;
			float pitch;
			float gain;
			float rolloff;
            bool isTemp;
            bool remove;
            ALboolean relative;
			ALboolean looping;
			sound_object* next;
		};
        
		sound_object* sol_head;
		sound_object* ambient;
		ALuint buffers[SOUND_NUM_BUFFERS];
        
        bool sound_enabled;
        int max_sources;
        int sounds_playing;
		int registered[4];
		int reserved[4];
        
		void load_buffer(ALuint &buffer, char* filename);
        void load_WAV(ALuint &buffer, char* filename);
		void load_OGG(ALuint &buffer, char* filename);
        
		void insert(sound_object* &head, sound_object* &n);
		float calc_dist(float* object_pos);
        
        void create_source(ALuint &source);
        void create_buffer(ALuint &buffer);
		void load_source(sound_object* sound_obj);
		void play_source(ALuint &source);
		void delete_source(ALuint &source);
        void delete_buffer(ALuint &buffer);
        
		void sort(sound_object* &new_head, sound_object* item);
		void sort_sounds();
		void remove_finished();
		void play_sounds();

	public:
		sound_module();                 // Constructor
		~sound_module();                // Deconstructor
        
		/* Initialization Routines */
		void init();
		void loadSounds();
        
		/* Base Routines */
        int addSound(int buffer, int priority, float* position, int looping, bool isTemp, bool relative);
        int addSound(char* filename, int priority, float* position, int looping);
        
        inline int addSound(int buffer, int priority, float* position, int looping)
            { return addSound(buffer, priority, position, looping, false, false); }
        int addSound(int buffer, int priority, kVector position, int looping)
            { float pos[3] = {position[0], position[1], position[2]};
              return addSound(buffer, priority, pos, looping, false, false); }
        
        int addAmbientSound(int buffer, int priority, int looping);
        int addAmbientSound(char* filename, int priority, int looping);

		/* Mutators */
		void killSound(int &id);
		void killAllSounds();
       
        void setSystemEnabled(bool enabled)
            { sound_enabled = enabled; }
        void setSystemMaxSounds(int number)
            { max_sources = number; }
        void setSystemVolume(int level);
        
		void setSoundGain(int id, float amount);
		void setSoundPitch(int id, float amount);
		void setSoundPosition(int id, float* pos);
		void setSoundRolloff(int id, float amount);
        
		/* Base Update Routine */
		void update();
        
		/* Auxilary Routines */
		void auxModOnCaliber(int id, float caliber);
		void auxModOnExplosive(int id, float explosive);
		void auxModOnRPM(int id, float rpm);
};

extern sound_module sounds;

#endif

