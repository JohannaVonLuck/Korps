/*******************************************************************************
                        Misc. Functions - Implementation
*******************************************************************************/
#include "main.h"
#include "misc.h"

/*******************************************************************************
    function    :   write_error
    arguments   :   text - error message
    purpose     :   Outputs the error message to both the error.log file as well
                    as standard out. Base handler function.
    notes       :   Appends an "Error: " string infront of error message.
*******************************************************************************/
void write_error(char* text)
{
    static bool open_new_file = true;
    ofstream fout;

    // Determine if we need to open our file, or just append
    if(open_new_file)
    {
        fout.open("error.log", ios::out);
        open_new_file = false;
    }
    else
        fout.open("error.log", ios::app);

    // Check for success
    if(!fout)
        return;

    // Output error to error log
    fout << "Error: " << text << endl;

    fout.close();
    
    // Output error to standard out
    cout << "Error: " << text << endl;
}

/*******************************************************************************
    function    :   float lowest
    arguments   :   n1,n2,n3,n4 - list of numbers to compare
    purpose     :   Returns lowest value between 4 numbers.
    notes       :   Yet another dumb function - actually turns out to save us
                    some code space in certain situations, believe it or not.
*******************************************************************************/
float lowest(float n1, float n2, float n3, float n4)
{
    float temp;
    
    temp = n1;
    
    // Compare between numbers for lowest value
    if(n2 < temp)
        temp = n2;
    if(n3 < temp)
        temp = n3;
    if(n4 < temp)
        temp = n4;
        
    return temp;
}

/*******************************************************************************
    function    :   float min_value
    arguments   :   value1 - First value
                    value2 - Second value
    purpose     :   Returns the minimum of two floats.
    notes       :   Yes... yet another dumb function.
*******************************************************************************/
float min_value(float value1, float value2)
{
    if(value1 <= value2)
        return value1;
    else
        return value2;
}

/*******************************************************************************
    function    :   float max_value
    arguments   :   value1 - First value
                    value2 - Second value
    purpose     :   Returns the maximum of two floats.
    notes       :   Yes... yet another dumb function.
*******************************************************************************/
float max_value(float value1, float value2)
{
    if(value1 >= value2)
        return value1;
    else
        return value2;
}

bool fileExists(char* fileName)
{
    ifstream fin;
    
    fin.open(fileName);
    
    if(!fin)
        return false;
        
    fin.close();
    return true;
}

void eatjunk(ifstream &fin)
{
    while(fin.peek() <= ' ' && !fin.eof())
        fin.ignore(1);
}


/* Remaining functions here are integer based math ops (fast ops), of which
   might be taken out before release since their not really being used.       */

unsigned short isqrt(unsigned long a)
{
    unsigned long rem = 0;
    unsigned long root = 0;
    int i;

    for (i = 0; i < 16; i++) {
        root <<= 1;
        rem = ((rem << 2) + (a >> 30));
        a <<= 2;
        root++;
        if (root <= rem) {
            rem -= root;
            root++;
        }
        else
            root--;
    }
    return (unsigned short) (root >> 1);
}

unsigned short ihypot (unsigned long dx, unsigned long dy)
{
    return isqrt (dx * dx + dy * dy);
}

unsigned short iisqrt(unsigned long a)
{
    unsigned long rem = 0;
    unsigned long root = 0;
    unsigned long divisor = 0;
    int i;

    for (i = 0; i < 16; i++) {
        root <<= 1;
        rem = ((rem << 2) + (a >> 30));
        a <<= 2;
        divisor = (root << 1) + 1;
        
        if (divisor <= rem) {
            rem -= divisor;
            root++;
        }
    }
    
    return (unsigned short)(root);
}
