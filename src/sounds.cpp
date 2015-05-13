/*******************************************************************************
                           Sound Module - Implementation
*******************************************************************************/
#include "main.h"
#include "sounds.h"
#include "camera.h"
#include "console.h"
#include "misc.h"
#include <vector>

/*******************************************************************************
    function    :   sound_module::sound_module
    arguments   :   <none>
    purpose     :   Constructor.
    notes       :   <none>
*******************************************************************************/
sound_module::sound_module()
{
    sol_head = NULL;

    registered[0] = 0;
    registered[1] = 0;
    registered[2] = 0;
    registered[3] = 0;
    
    reserved[0] = SOUND_SLOTS_RESERVED_AMBIENT;
    reserved[1] = SOUND_SLOTS_RESERVED_HIGH;
    reserved[2] = SOUND_SLOTS_RESERVED_MID;
    reserved[3] = SOUND_SLOTS_RESERVED_LOW;
    
    max_sources = 16;
    sounds_playing = 0;
    sound_enabled = true;
}

/*******************************************************************************
    function    :   sound_module::~sound_module
    arguments   :   <none>
    purpose     :   Deconstructor.
    notes       :   <none>
*******************************************************************************/
sound_module::~sound_module()
{
    // Kill off all the sounds
    killAllSounds();
    
    // Free up the buffers
    alDeleteBuffers(SOUND_NUM_BUFFERS, &buffers[0]);
}

/*******************************************************************************
    function    :   sound_module::load_buffer
    arguments   :   buffer - address of buffer to file
                    filename - filename of the wav file to load
    purpose     :   Load a wav file into the buffer array
    notes       :   <none>
*******************************************************************************/
void sound_module::load_buffer(ALuint &buffer, char* filename)
{  
    char text[128];
    
    // Check to see if the file exsists    
    if(!fileExists(filename))
    {
        buffer = 0;
        sprintf(text, "Sound: Error file does not exist: \"%s\".", filename);
        write_error(text);
        return;
    }

    // Load the file based on its file extention
    if(strstr(filename, ".wav") != 0)
    {
        load_WAV(buffer, filename);
    }
    else if(strstr(filename, ".ogg") != 0)
    {    
        load_OGG(buffer, filename);
    }
    else
    {
        sprintf(text, "Sound: Incorrect file type for file \"%s\"", filename);
        write_error(text);
        return;
    }
 
    // Check for AL errors
    if(alGetError() != AL_NO_ERROR)
    {
        sprintf(text, "Sound: Error loading \"%s\" into buffer.", filename);
        write_error(text);
    }
}

/*******************************************************************************
    function    :   sound_module::load_WAV
    arguments   :   
    purpose     :   Load a wav file into the buffer array
    notes       :   <none>
*******************************************************************************/
void sound_module::load_WAV(ALuint &buffer, char* filename)
{
	return;
	/*
    ALenum format;
    ALvoid* data;
    ALsizei size;
    ALsizei freq;
    ALboolean loop;
    
    alutLoadWAVFile((ALbyte*)filename, &format, &data, &size, &freq, &loop);
    alBufferData(buffer, format, data, size, freq);
    alutUnloadWAV(format, data, size, freq);
	*/
}

/*******************************************************************************
    function    :   sound_module::load_OGG
    arguments   :   
    purpose     :   Load a ogg file into the buffer array
    notes       :   <none>
*******************************************************************************/
void sound_module::load_OGG(ALuint &buffer, char* filename)
{
    char array[65536];
    vector<char> stream;
    int bitstream;
    int bytes;
    vorbis_info *p_info;
    OggVorbis_File ogg_file;
    FILE *fin;
    ALenum format;
    ALsizei freq;

	cout << "Opening OGG file: " << filename << endl;
    
    // Open for binary reading
    fin = fopen(filename, "rb");

	cout << 0 << endl;
	
    ov_open(fin, &ogg_file, NULL, 0);
	
	cout << 1 << endl;
    
    // Get some information about the OGG file
    p_info = ov_info(&ogg_file, -1);
    
    // Check the number of channels... always use 16-bit samples
    if(p_info->channels == 1)
        format = AL_FORMAT_MONO16;
    else
        format = AL_FORMAT_STEREO16;

	cout << 2 << endl;
    
    // The frequency of the sampling rate
    freq = p_info->rate;
    
    do
    {
        bytes = ov_read(&ogg_file, array, 65536, 0, 2, 1, &bitstream);
        stream.insert(stream.end(), array, array+bytes);
    } while(bytes > 0);

	cout << 3 << endl;
    
    alBufferData(buffer, format, &stream[0], (ALsizei)stream.size(), freq);

	cout << 4 << endl;
    
    ov_clear(&ogg_file);

	cout << 5 << endl;
}

/*******************************************************************************
    function    :   sound_module::insert()
    arguments   :   head - Pointer to first element in list
                    item - Pointer to item to insert into list
    purpose     :   Inserts item into list based on its distance value.
    notes       :   <none>
*******************************************************************************/
void sound_module::insert(sound_object* &head, sound_object* &item)
{
    sound_object* prev = NULL;
    sound_object* curr = NULL;
    
    if(head == NULL)
    {
        head = item;
    }
    else if(item->dist <= head->dist + FP_ERROR)
    {
        item->next = head;
        head = item;
    }
    else
    {
        prev = head;
        curr = head->next;
        while(curr && item->dist > curr->dist + FP_ERROR)
        {
            prev = curr;
            curr = curr->next;
        }
        prev->next = item;
        item->next = curr;
    }
}

/*******************************************************************************
    function    :   sound_module::calc_dist
    arguments   :   object_pos - Position array
    purpose     :   Calculate the distance between the object and listener
    notes       :   <none>
*******************************************************************************/
float sound_module::calc_dist(float* object_pos)
{
    static float* listener_pos = camera.getCamPos();
    
    return
     fabsf((listener_pos[0]-object_pos[0]) * (listener_pos[0]-object_pos[0]) +
           (listener_pos[1]-object_pos[1]) * (listener_pos[1]-object_pos[1]) +
           (listener_pos[2]-object_pos[2]) * (listener_pos[2]-object_pos[2]));
}

/*******************************************************************************
    function    :   sound_module::create_source
    arguments   :   source - Container for integer source number
    purpose     :   Creates a source.
    notes       :   <none>
*******************************************************************************/
void sound_module::create_source(ALuint &source)
{
    alGenSources(1, &source);
}

/*******************************************************************************
    function    :   sound_module::create_buffer
    arguments   :   buffer - Container for integer buffer number
    purpose     :   Creates a buffer
    notes       :   <none>
*******************************************************************************/
void sound_module::create_buffer(ALuint &buffer)
{
    alGenBuffers(1, &buffer);
}

/*******************************************************************************
    function    :   sound_module::load_source
    arguments   :   sound_obj - Pointer to sound object
    purpose     :   Loads the source of a sound object with the sound object's
                    parameters.
    notes       :   <none>
*******************************************************************************/
void sound_module::load_source(sound_object* sound_obj)
{
    alSourcei  (sound_obj->source, AL_BUFFER,   sound_obj->buffer );
    alSourcefv (sound_obj->source, AL_POSITION, sound_obj->pos    );
    alSourcei  (sound_obj->source, AL_LOOPING,  sound_obj->looping);
    alSourcef  (sound_obj->source, AL_PITCH,    sound_obj->pitch  );
    alSourcef  (sound_obj->source, AL_GAIN,     sound_obj->gain   );
    alSourcef  (sound_obj->source, AL_ROLLOFF_FACTOR, sound_obj->rolloff);
    alSourcei  (sound_obj->source, AL_SOURCE_RELATIVE, sound_obj->relative);
}

/*******************************************************************************
    function    :   sound_module::play_source
    arguments   :   source - Container for integer source number
    purpose     :   Plays a sound via it's source integer tag.
    notes       :   <none>
*******************************************************************************/
void sound_module::play_source(ALuint &source)
{
    ALenum state;

    alGetSourcei(source, AL_SOURCE_STATE, &state);
   
    if(state != AL_PLAYING)
    {
        alSourcePlay(source);
    }
}

/*******************************************************************************
    function    :   sound_module::delete_source
    arguments   :   source - Container for integer source number
    purpose     :   Stops source from playing and then deletes the id tag, sets
                    it to 0.
    notes       :   <none>
*******************************************************************************/
void sound_module::delete_source(ALuint &source)
{
    ALenum state;
    
    if(source == 0)
        return;
 
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    
    if(state == AL_PLAYING)
        alSourceStop(source);
    
    alDeleteSources(1, &source);
    
    source = 0;
}

/*******************************************************************************
    function    :   sound_module::delete_buffer
    arguments   :   buffer - Container for integer buffer number
    purpose     :   Delete the buffer to clean up space.
    notes       :   <none>
*******************************************************************************/
void sound_module::delete_buffer(ALuint &buffer)
{
    alDeleteBuffers(1, &buffer);
}

/*******************************************************************************
    function    :   sound_module::sort
    arguments   :   head - Head pointer of (initially) blank list
                    item - Pointer to item to sort
    purpose     :   Sorts the list.
    notes       :   <none>
*******************************************************************************/
void sound_module::sort(sound_object* &head, sound_object* item)
{
    // Hit the end of list
   if(item == NULL)
        return;
    
    // Recursive call
    sort(head, item->next);
    
    // Recompute the distance
    item->dist = calc_dist(item->pos);
    
    // Place obj in the new list
    insert(head, item); 
}

/*******************************************************************************
    function    :   sound_module::sort_sounds()
    arguments   :   <none>
    purpose     :   Sorts the sounds registered within the module based on
                    the distance values.
    notes       :   <none>
*******************************************************************************/
void sound_module::sort_sounds()
{
    sound_object* new_head = NULL;
    
    // Place items into new list
    sort(new_head, sol_head);
    
    // Change the pointers so it changes the orginal list
    sol_head = new_head;
}

/*******************************************************************************
    function    :   sound_module::remove_finished
    arguments   :   <none>
    purpose     :   Removes sounds from the sol_head that have finished playing
    notes       :   <none>
*******************************************************************************/
void sound_module::remove_finished()
{
    ALenum state;
    sound_object* prev = NULL;
    sound_object* curr = sol_head;
    
    while(curr)
    {
        // Remove sound if it has stopped
        if(curr->source != 0)
        {
            alGetSourcei(curr->source, AL_SOURCE_STATE, &state);
            
            if(state != AL_PLAYING)
                curr->remove = true;
        }
        
        if(curr->remove)
        {
            if(curr->source != 0)
                delete_source(curr->source);
                
            if(curr->isTemp)
                delete_buffer(curr->buffer);
           
           registered[curr->priority]--;
                
            // Remove the sound item from the sol_head
            if(prev == NULL) // front of list
            {
                sol_head = sol_head->next;
                delete curr;
                curr = sol_head;
                continue;
            }
            else // middle and end of list
            {
                prev->next = curr->next;
                delete curr;
                curr = prev->next;
                continue;
            }
        }
        
        prev = curr;
        curr = curr->next;
    }
}

/*******************************************************************************
    function    :   sound_module::play_sounds()
    arguments   :   <none>
    purpose     :   Plays the sounds in the system which satisfy the priority
                    scheme of implemenation.
    notes       :   <none>
*******************************************************************************/
void sound_module::play_sounds()
{
    int count[4] = {0, 0, 0, 0};
    int leftovers =
        (reserved[0] - registered[0] <= 0 ? 0 : reserved[0] - registered[0]) +
        (reserved[1] - registered[1] <= 0 ? 0 : reserved[1] - registered[1]) +
        (reserved[2] - registered[2] <= 0 ? 0 : reserved[2] - registered[2]) +
        (reserved[3] - registered[3] <= 0 ? 0 : reserved[3] - registered[3]) +
        (max_sources - (reserved[0] + reserved[1] + reserved[2] + 
        reserved[3]));
        
    sound_object* curr = sol_head;
    
    sounds_playing = 0;
    
    while(curr)
    {
        if(sounds_playing < max_sources &&
           count[curr->priority] < reserved[curr->priority])
        {
            count[curr->priority]++;
            
            if(curr->source == 0)
            {
                create_source(curr->source);
                load_source(curr);
                play_source(curr->source);
            }
//            else
//                alSourcefv(curr->source, AL_POSITION, curr->pos);
            
            sounds_playing++;
        }
        else if(sounds_playing < max_sources && leftovers > 0)
        {
            leftovers--;
            
            if(curr->source == 0)
            {
                create_source(curr->source);
                load_source(curr);
                play_source(curr->source);
            }
//            else
//                alSourcefv(curr->source, AL_POSITION, curr->pos);
            
            sounds_playing++;
        }
        else
        {
            if(curr->source != 0)
                delete_source(curr->source);
            if(curr->looping == AL_FALSE)
                curr->remove = true;
        }
        
        curr = curr->next;
    }
}

/*******************************************************************************
    function    :   sound_module::init
    arguments   :   <none>
    purpose     :   Initializes the audio library.
    notes       :   <none>
*******************************************************************************/
void sound_module::init()
{
    float* listener_pos = camera.getCamPos();
    float listener_vel[3] = { 0.0, 0.0, 0.0 };
    float listener_ori[6];
    kVector listener_dir = camera.getCamDirV();
    listener_dir.convertTo(CS_CARTESIAN);

    listener_ori[0] = listener_dir[0];
    listener_ori[1] = listener_dir[1];
    listener_ori[2] = listener_dir[2];
    listener_ori[3] = 0;
    listener_ori[4] = 1;
    listener_ori[5] = 0;

    // Set the listeners POSITION
    alListenerfv(AL_POSITION, listener_pos);
    if(alGetError() != AL_NO_ERROR)
    {
        write_error("Sound: Could not set Audio: listener position.");
        return;
    }

    // Set the listeners ORIENTATION
    alListenerfv(AL_ORIENTATION, listener_ori);
    if(alGetError() != AL_NO_ERROR)
    {
        write_error("Sound: Could not set Audio: listener orientation.");
        return;
    }

    // Set the listeners VELOCITY
    alListenerfv(AL_VELOCITY, listener_vel);
    if(alGetError() != AL_NO_ERROR)
    {
        write_error("Sound: Could not set Audio: listener velocity.");
        return;
    }
    
    // Generate Buffers
    alGenBuffers(SOUND_NUM_BUFFERS, &buffers[0]);
    if(alGetError() != AL_NO_ERROR)
    {
        write_error("Sound: FATAL Could not generate sound buffers.");
        exit(1);
    }
    
    // Set misc sound varibles
    alDopplerFactor(1.0);
    alDopplerVelocity(340.0);
    if(alGetError() != AL_NO_ERROR)
    {
        write_error("Sound: Could not set Audio: doppler.");
    }
}

/*******************************************************************************
    function    :   sound_module::loadSounds()
    arguments   :   <none>
    purpose     :   Load all wav files into the buffer array
    notes       :   <none>
*******************************************************************************/
void sound_module::loadSounds()
{
    //load_buffer(buffers[SOUND_BATTLE],           "Sounds/battle.ogg");
    load_buffer(buffers[SOUND_AMBIENT],          "Sounds/ambient.ogg");
    load_buffer(buffers[SOUND_TANKS_PETROL],     "Sounds/tank_1.ogg");
    load_buffer(buffers[SOUND_TANKS_DIESEL],     "Sounds/tank_2.ogg");
    load_buffer(buffers[SOUND_MOTOR_DIE],        "Sounds/motor_die.ogg");
    load_buffer(buffers[SOUND_CANNON],           "Sounds/firing_cannon.ogg");
    load_buffer(buffers[SOUND_MG],               "Sounds/firing_mg.ogg");
    load_buffer(buffers[SOUND_SHELL_RICOCHET_1], "Sounds/shell_richochet_1.ogg");
    load_buffer(buffers[SOUND_SHELL_RICOCHET_2], "Sounds/shell_richochet_2.ogg");
    load_buffer(buffers[SOUND_SHELL_RICOCHET_3], "Sounds/shell_richochet_3.ogg");
    load_buffer(buffers[SOUND_MG_RICHOCHET_1],   "Sounds/richochet_mg_1.ogg");
    load_buffer(buffers[SOUND_MG_RICHOCHET_2],   "Sounds/richochet_mg_2.ogg");
    load_buffer(buffers[SOUND_MG_RICHOCHET_3],   "Sounds/richochet_mg_3.ogg");
    load_buffer(buffers[SOUND_PENETRATE_1],      "Sounds/penetrate_1.ogg");
    load_buffer(buffers[SOUND_PENETRATE_2],      "Sounds/penetrate_2.ogg");
    load_buffer(buffers[SOUND_SHELL_THUD],       "Sounds/thud.ogg");
    load_buffer(buffers[SOUND_EXPLOSION],        "Sounds/explosion.ogg");
    load_buffer(buffers[SOUND_GROUND_EXPLOSION], "Sounds/explosion_ground.ogg");
    load_buffer(buffers[SOUND_FLAMES],           "Sounds/flames.ogg");
}

/*******************************************************************************
    function    :   sound_module::AddSound
    arguments   :   buffer - Buffer index
                    priority - Priority basis
                    position - Position of sound
                    looping - Looping control
                    isTemp - true/false depending on if the buffer is temporary
                    relative - true/false if the sound is source relative
    purpose     :   Adds and registers a new sound into the sound module.
    notes       :   <none>
*******************************************************************************/
int sound_module::addSound(int buffer, int priority, float* position,
    int looping, bool isTemp, bool relative)
{
    sound_object* sound_obj;
    
    if(!sound_enabled)
        return SOUND_NULL;

    // Create a new sound object and initialize values
    sound_obj = new sound_object;
    
    if(sound_obj == NULL)
    {
        write_error("Sound: Could not create a new sound object.");
        return SOUND_NULL;
    }
    
    sound_obj->buffer = (buffer<SOUND_NUM_BUFFERS ? buffers[buffer]:buffer);
    sound_obj->source = 0;                      // No current source
    sound_obj->priority = priority;
    sound_obj->pos[0] = position[0];
    sound_obj->pos[1] = position[1];
    sound_obj->pos[2] = position[2];
    sound_obj->dist = calc_dist(position);
    sound_obj->isTemp = isTemp;
    sound_obj->remove = false;
    sound_obj->relative = (relative ? AL_TRUE : AL_FALSE);
    sound_obj->looping = (looping == SOUND_PLAY_LOOP ? AL_TRUE : AL_FALSE);
    sound_obj->pitch = 1.0f;
    sound_obj->gain = 1.0f;
    sound_obj->rolloff = 0.5f;
    sound_obj->next = NULL;
    
    insert(sol_head, sound_obj);
    
    registered[sound_obj->priority]++;
    
    return (int)sound_obj;
}

/*******************************************************************************
    function    :   sound_module::addSound
    arguments   :   filename - Name of file to play
                    priority - Priority basis
                    looping - Looping control
    purpose     :   Add an ambient sound during gameplay
    notes       :   Best used for background and voice over sounds
*******************************************************************************/
int sound_module::addSound(char* filename, int priority, float* position,
    int looping)
{
    ALuint buffer;
    create_buffer(buffer);    
    load_buffer(buffer, filename);    
    return addSound(buffer, priority, position, looping, true, false);
}

/*******************************************************************************
    function    :   sound_module::addAmbientSound
    arguments   :   buffer - Buffer index
                    priority - Priority basis
                    looping - Looping control
    purpose     :   Add an ambient sound during gameplay
    notes       :   Best used for background and voice over sounds
*******************************************************************************/
int sound_module::addAmbientSound(int buffer, int priority, int looping)
{
    float pos[3] = {0, 0, 0};    
    return addSound(buffer, priority, pos, looping, false, true);
}

/*******************************************************************************
    function    :   sound_module::addAmbientSound
    arguments   :   filename - Name of file to play
                    priority - Priority basis
                    looping - Looping control
    purpose     :   Add an ambient sound during gameplay
    notes       :   Best used for background and voice over sounds
*******************************************************************************/
int sound_module::addAmbientSound(char* filename, int priority, int looping)
{
    float pos[3] = {0, 0, 0};
    ALuint buffer;

    create_buffer(buffer);
    load_buffer(buffer, filename);  
    return addSound(buffer, priority, pos, looping, true, true);   
}

/*******************************************************************************
    function    :   sound_module::killSound
    arguments   :   id - the id of the sound to be killed off
    purpose     :   stops and removes a specific sound from the list
    notes       :   id is actually a pointer to a sound_object
*******************************************************************************/
void sound_module::killSound(int &id)
{
    sound_object* curr = sol_head;
    sound_object* prev = NULL;
    
    if(id == 0 || id == SOUND_NULL)
        return;
    
    while(curr)
    {
        if(curr == (sound_object*)id)
        {
            if(curr->source != 0)
                delete_source(curr->source);
            
            registered[curr->priority]--;
            
            if(prev == NULL)
                sol_head = sol_head->next;
            else
                prev->next = curr->next;
            delete curr;
            
            return;
        }
        
        prev = curr;
        curr = curr->next;
    }
    
    id = SOUND_NULL;
}

/*******************************************************************************
    function    :   sound_module::killAllSounds
    arguments   :   <none>
    purpose     :   Stop all sounds and delete them from the list
    notes       :   <none>
*******************************************************************************/
void sound_module::killAllSounds()
{
    sound_object* curr = sol_head;
    
    while(curr)
    {
        if(curr->source != 0)
            delete_source(curr->source);
        
        sol_head = sol_head->next;
        delete curr;
        curr = sol_head;
    }
    
    registered[0] = 0;
    registered[1] = 0;
    registered[2] = 0;
    registered[3] = 0;
    sounds_playing = 0;
}

/*******************************************************************************
    function    :   sound_module::setSystemVolume
    arguments   :   level - sound level (0 quietest - 10 loudest)
    purpose     :   Change the in-game overall system volume.
    notes       :   Listener gain should be between 0 and 1.
*******************************************************************************/
void sound_module::setSystemVolume(int level)
{
    if( level < 0 )
        level = 0;
    else if( level > 10 )
        level = 10;
    
    alListenerf(AL_GAIN, 0.1 * level);
}

/*******************************************************************************
    function    :   sound_module::setSoundPosition
    arguments   :   id - sound object identifier
                    pos - float array of the objects position
    purpose     :   Set the position of a sound_object
    notes       :   id is actually a pointer to a sound_object
*******************************************************************************/
void sound_module::setSoundPosition(int id, float* pos)
{
    sound_object* curr = (sound_object*)id;
   
    if(id != 0 && id != SOUND_NULL)
    {
        curr->pos[0] = pos[0];
        curr->pos[1] = pos[1];
        curr->pos[2] = pos[2];
        
        if(curr->source != 0 )
            alSourcefv(curr->source, AL_POSITION, curr->pos);
    }
}

/*******************************************************************************
    function    :   sound_module::setSoundPitch
    arguments   :   id - sound object identifier
                    amount - float value of which to set the value
    purpose     :   Set the pitch of a sound_object
    notes       :   id is actually a pointer to a sound_object
*******************************************************************************/
void sound_module::setSoundPitch(int id, float amount)
{
    sound_object* curr = (sound_object*)id;
    
    if(id != 0 && id != SOUND_NULL)
    {
        curr->pitch = amount;
                
        if(curr->source != 0)
            alSourcef(curr->source, AL_GAIN, curr->pitch);
    }
}

/*******************************************************************************
    function    :   sound_module::setSoundGain
    arguments   :   id - sound object identifier
                    amount - float value of which to set the value
    purpose     :   Set the gain of a sound_object
    notes       :   id is actually a pointer to a sound_object
*******************************************************************************/
void sound_module::setSoundGain(int id, float amount)
{
    sound_object* curr = (sound_object*)id;
    
    if(id != 0 && id != SOUND_NULL)
    {
        curr->gain = amount;
           
        if(curr->source != 0)
            alSourcef(curr->source, AL_GAIN, curr->gain);
    }
}

/*******************************************************************************
    function    :   sound_module::setSoundRolloff
    arguments   :   id - sound object identifier
                    amount - float value of which to set the value
    purpose     :   Set the rolloff factor of a sound_object
    notes       :   id is actually a pointer to a sound_object
*******************************************************************************/
void sound_module::setSoundRolloff(int id, float amount)
{
    sound_object* curr = (sound_object*)id;
    
    if(id != 0 && id != SOUND_NULL)
    {
        curr->rolloff = amount;
           
        if(curr->source != 0)
            alSourcef(curr->source, AL_ROLLOFF_FACTOR, curr->rolloff);
    }
}

/*******************************************************************************
    function    :   sound_module::update()
    arguments   :   <none>
    purpose     :   Resort the list and change witch sounds should be playing
    notes       :   <none>
*******************************************************************************/
void sound_module::update()
{
    static float* listener_pos = camera.getCamPos();
    float listener_ori[6];
    kVector listener_dir = camera.getCamDirV();
    listener_dir.convertTo(CS_CARTESIAN);
    
    listener_ori[0] = listener_dir[0];
    listener_ori[1] = listener_dir[1];
    listener_ori[2] = listener_dir[2];
    listener_ori[3] = 0;
    listener_ori[4] = 1;
    listener_ori[5] = 0;
    
    // Set the listeners position and orientation
    alListenerfv(AL_POSITION, listener_pos);
    alListenerfv(AL_ORIENTATION, listener_ori);
    
    // Sort the sounds list
    sort_sounds();
    
    // Remove finished sounds
    remove_finished();
    
    // Play sounds (with priority)
    play_sounds();
}

/*******************************************************************************
    function    :   sound_module::auxModOnRPM
    arguments   :   id - sound object identifier
                    rpm - the current rpm value for the object
    purpose     :   Change the pitch of the sound to emulate rpm changes
    notes       :   id is actually a pointer to a sound_object
*******************************************************************************/
void sound_module::auxModOnRPM(int id, float rpm)
{
    sound_object* curr = (sound_object*)id;
    
    if(id != 0 && id != SOUND_NULL)
    {
        curr->pitch = 0.0004166667 * rpm + 0.25;
            
        if(curr->source != 0)
            alSourcef(curr->source, AL_PITCH, curr->pitch);
    }
}

/*******************************************************************************
    function    :   sound_module::auxModOnCaliber
    arguments   :   id - sound object identifier
                    caliber - the current caliber value for the object
    purpose     :   Change the pitch/gain of the sound to emulate caliber of a
                    shell being fired.
    notes       :   id is actually a pointer to a sound_object
*******************************************************************************/
void sound_module::auxModOnCaliber(int id, float caliber)
{
    sound_object* curr = (sound_object*)id;
    
    if(id != 0 && id != SOUND_NULL)
    {
        curr->pitch = -0.065789 * caliber + 1.493421;
        curr->gain = -0.0153846 * caliber + 2.153846;
        
        if(curr->source != 0)
        {
            alSourcef(curr->source, AL_PITCH, curr->pitch);
            alSourcef(curr->source, AL_GAIN, curr->gain);
        }
    }
}

/*******************************************************************************
    function    :   sound_module::auxModOnCaliber
    arguments   :   id - sound object identifier
                    explosive - the amount of explosive (in kg) to mod on
    purpose     :   Change the pitch/gain of the sound to emulate the explosive
                    content being ignited.
    notes       :   id is actually a pointer to a sound_object
*******************************************************************************/
void sound_module::auxModOnExplosive(int id, float explosive)
{
    sound_object* curr = (sound_object*)id;
    
    if(id != 0 && id != SOUND_NULL)
    {
        explosive = sqrt(explosive);
        curr->pitch = -0.903875 * explosive + 1.750815;
        curr->gain = 1.265425 * explosive + 0.448859;
        
        if(curr->source != 0)
        {
            alSourcef(curr->source, AL_PITCH, curr->pitch);
            alSourcef(curr->source, AL_GAIN, curr->gain);
        }
    }
}

