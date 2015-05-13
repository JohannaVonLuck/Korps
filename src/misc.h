/*******************************************************************************
                          Misc. Functions - Definition
*******************************************************************************/

#ifndef MISC_H
#define MISC_H

/*******************************************************************************
    function    :   float inv_sqrt
    arguments   :   x - value to perform function on
    purpose     :   Performs the efficiency inv_sqrt() approximation.
    notes       :   <none>
*******************************************************************************/
inline float inv_sqrt(float x)
{
    float xhalf = 0.5f*x;
    int i = *(int*)&x;
    i = 0x5f3759df - (i >> 1);
    x = *(float*)&i;
    x = x*(1.5f - xhalf*x*x);
    return x;
}

/*******************************************************************************
    function    :   bool roll
    arguments   :   chance - value between 0.0 and 1.0 (0 - 100% chance)
    purpose     :   Makes a random number and checks to see if it satisfies the
                    chance parameter - similiar to rolling a die based on a
                    chance % chance of "success". Used for randomization.
    notes       :   Yes we know, this is a very dumb function.
*******************************************************************************/
inline bool roll(float chance)
{    
    if(chance >= 1.0 || (chance > 0.0 && ((float)rand() / RAND_MAX) <= chance))
        return true;

    return false;
}

/*******************************************************************************
    function    :   int choose
    arguments   :   num
    purpose     :   Chooses a random number between one and num.
    notes       :   Yes... yet another dumb function.
*******************************************************************************/
inline int choose(int num)
{
    return (rand() % num) + 1;
}

/*******************************************************************************
    function    :   float limit
    arguments   :   value - what we got
                    max - max value possible
    purpose     :   Limits value to max - returns result.
    notes       :   Yes... yet another dumb function.
*******************************************************************************/
inline float limit(double value, double max)
{
    return (value > max) ? max : value;
}

float min_value(float value1, float value2);
float max_value(float value1, float value2);

bool fileExists(char* fileName);
void eatjunk(ifstream &fin);

// Writes error to error.log and standard out
void write_error(char* text);

// Determines lowest value between 4 numbers
float lowest(float n1, float n2, float n3, float n4);

// Misc. Integer based operations
unsigned short isqrt(unsigned long a);
unsigned short ihypot (unsigned long dx, unsigned long dy);
unsigned short iisqrt(unsigned long a);

#endif
