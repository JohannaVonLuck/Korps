/*******************************************************************************
                        Command Console Implementation
*******************************************************************************/
#include "main.h"
#include "console.h"
#include "camera.h"
#include "database.h"
#include "fonts.h"
#include "script.h"
#include "metrics.h"
#include "scenery.h"
#include "texture.h"

/*******************************************************************************
    function    :   console_module::console_module
    arguments   :   <none>
    purpose     :   Constructor.
    notes       :   <none>
*******************************************************************************/
console_module::console_module()
{
    int i;
    
    // Initialize line interval to 5 seconds per line (2x for com lines)
    line_interval = 5.0;
    
    // Initialize counters to 0
    line_count = 0;
    com_line_count = 0;
    head_ptr = 0;
    com_head_ptr = 0;
    
    // Initialize console pulldown
    pulldown = false;
    curr_down = 0.0;
    dest_down = 0.0;
    
    // Initialize command line    
    cmd_entering = false;
    command[0] = '\0';
    cmd_strlen = 0;
    cmd_backpos = 0;
    
    // Initialize console lines
    for(i = 0; i < CON_MAX_LINES; i++)
    {
        lines[i].text = NULL;
        lines[i].image = NULL;
        lines[i].expire_time = 0.0;
        lines[i].pulldown_only = false;
    }
    
    // Initialize com lines
    for(i = 0; i < CON_MAX_COM_LINES; i++)
    {
        com_lines[i].text = NULL;
        com_lines[i].image = NULL;
        com_lines[i].expire_time = 0.0;
    }
}

/*******************************************************************************
    function    :   console_module::~console_module
    arguments   :   <none>
    purpose     :   Deconstructor.
    notes       :   <none>
*******************************************************************************/
console_module::~console_module()
{
    int i;
    
    // Go through our lists and deallocate all alloted memory, all of which was
    // allocated through strdup().
    for(i = 0; i < CON_MAX_LINES; i++)
    {
        if(lines[i].text)
            delete lines[i].text;
        if(lines[i].image)
            delete lines[i].image;
    }
    
    for(i = 0; i < CON_MAX_COM_LINES; i++)
    {
        if(com_lines[i].text)
            delete com_lines[i].text;
        if(com_lines[i].image)
            delete com_lines[i].image;
    }
}

/*******************************************************************************
    Message Adding Routines
*******************************************************************************/

/*******************************************************************************
    function    :   console_module::addMessage
    arguments   :   text - ptr to string to be entered into console lines
    purpose     :   Adds the given message to the console lines for display of
                    text when console is pulled up or pulled down.
    notes       :   1) Text will be copied using strdup() - possible memory
                       leak if not taken care of properly.
                    2) Text will display despite whenever the console is pulled
                       down or pulled up. If pulled up, it will exist on-screen
                       for the given line_interval value (initially 5 seconds).
*******************************************************************************/
void console_module::addMessage(char* text)
{
    // Special check to make sure current line we are about to overwrite has
    // yet to be cleaned (and removed from the list via a line_count-- and text
    // pointer delete, which is VERY important or memory leaks will occur since
    // the text is allocated via strdup()).
    if(lines[head_ptr].text)
    {
        // If there is text at the current position and the time is yet to be
        // expired then make sure we keep the correct line_count value by
        // "removing" the line from display via line_count--.
        if(lines[head_ptr].expire_time > 0.0)
            line_count--;
        
        // Delete allocated text (important! possible memory leaks otherwise!)
        delete lines[head_ptr].text;
        delete lines[head_ptr].image;
        lines[head_ptr].text = NULL;
        lines[head_ptr].image = NULL;
    }
    
    // Generate text as a 32bpp image through the font module (and flip it so
    // OpenGL may render it right-side up).
    lines[head_ptr].image = fonts.generateText(text, FONT_COURIER_10,
        lines[head_ptr].width, lines[head_ptr].height);
    if(lines[head_ptr].image == NULL)
        return;
    flipImage(lines[head_ptr].image, 32,
        lines[head_ptr].width, lines[head_ptr].height);
        
    // Duplicate string. Note that strdup() allocates new data.
    lines[head_ptr].text = strdup(text);
    
    // Set time interval to be displayed when console is pulled up. 
    lines[head_ptr].expire_time = line_interval;
    
    // Line will exist even when console is pulled up
    lines[head_ptr].pulldown_only = false;
    
    // Increment our line count and set head_ptr to next open position
    line_count++;
    head_ptr++;
    
    // Bounds check for head_ptr
    if(head_ptr >= CON_MAX_LINES)
        head_ptr = 0;
}

/*******************************************************************************
    function    :   console_module::addComMessage
    arguments   :   text - ptr to string to be entered into communication lines
    purpose     :   Adds the given message to the communication lines for
                    display of text.
    notes       :   1) This function is different from addMessage. The console
                       also controls a sub-console system known as the commun-
                       ication lines. These lines use a much larger font and
                       are displayed on-screen for 2x as long as normal lines.
                       These lines are to be used for in-game messages to the
                       player, with possiblity of a corresponding sound to play.
                    2) Text will still get entered into the console lines, but
                       will not be displayed when the console is pulled up.
                    3) Text will be copied using strdup() - possible memory
                       leak if not taken care of properly.
                    4) Text will display despite whenever the console is pulled
                       down or pulled up. It will display on the lower portion
                       of the screen entirely seperate of that of regular
                       console text.
*******************************************************************************/
void console_module::addComMessage(char* text)
{
    // Special check to make sure current line we are about to overwrite has
    // yet to be cleaned (and removed from the list via a line_count-- and text
    // pointer delete, which is VERY important or memory leaks will occur since
    // the text is allocated via strdup()).
    if(com_lines[com_head_ptr].text)
    {
        // If there is text at the current position and the time is yet to be
        // expired then make sure we keep the correct line_count value by
        // "removing" the line from display via line_count--.
        if(com_lines[com_head_ptr].expire_time > 0.0)
            com_line_count--;
        
        // Delete allocated text (important! possible memory leaks otherwise!)
        delete com_lines[com_head_ptr].text;
        delete com_lines[com_head_ptr].image;
        com_lines[com_head_ptr].text = NULL;
        com_lines[com_head_ptr].image = NULL;
    }
    
    // Generate text as a 32bpp image through the font module (and flip it so
    // OpenGL may render it right-side up).
    com_lines[com_head_ptr].image = fonts.generateText(text, FONT_ARIAL_18,
        com_lines[com_head_ptr].width, com_lines[com_head_ptr].height);
    if(com_lines[com_head_ptr].image == NULL)
        return;
    flipImage(com_lines[com_head_ptr].image, 32,
        com_lines[com_head_ptr].width, com_lines[com_head_ptr].height);
    
    // Duplicate string. Note that strdup() allocates new data.
    com_lines[com_head_ptr].text = strdup(text);
    
    // Set time interval to be displayed (2x as that of normal console text). 
    com_lines[com_head_ptr].expire_time = line_interval + line_interval;
    
    // Note: pulldown_only value of com_lines node are not used for com lines.
    
    // Increment our line count and set head_ptr to next open position
    com_line_count++;
    com_head_ptr++;
    
    // Bounds check for head_ptr
    if(com_head_ptr >= CON_MAX_COM_LINES)
        com_head_ptr = 0;
    
    // Make sure text is also entered into the console as well so players can
    // look-up past com messages via the console. Note that these system
    // messages do not display on screen when console is pulled up since they
    // set pulldown_only to true.
    addSysMessage(text);
}

/*******************************************************************************
    function    :   console_module::addSysMessage
    arguments   :   text - ptr to string to be entered into console lines
    purpose     :   Adds the given message to the console lines for display of
                    text only when console is pulled down.
    notes       :   1) This function is relatively the same thing as the regular
                       addMessage, however, when using this function the text
                       will only be displayed when the console is pulled down.
                       This simulates a sorta "system message" concept.
                    2) Text will be copied using strdup() - possible memory
                       leak if not taken care of properly.
*******************************************************************************/
void console_module::addSysMessage(char* text)
{
    // Special check to make sure current line we are about to overwrite has
    // yet to be cleaned (and removed from the list via a line_count-- and text
    // pointer delete, which is VERY important or memory leaks will occur since
    // the text is allocated via strdup()).
    if(lines[head_ptr].text)
    {
        // If there is text at the current position and the time is yet to be
        // expired then make sure we keep the correct line_count value by
        // "removing" the line from display via line_count--.
        if(lines[head_ptr].expire_time > 0.0)
            line_count--;
        
        // Delete allocated text (important! possible memory leaks otherwise!)
        delete lines[head_ptr].text;
        delete lines[head_ptr].image;
        lines[head_ptr].text = NULL;
        lines[head_ptr].image = NULL;
    }
    
    // Generate text as a 32bpp image through the font module (and flip it so
    // OpenGL may render it right-side up).
    lines[head_ptr].image = fonts.generateText(text, FONT_COURIER_10,
        lines[head_ptr].width, lines[head_ptr].height);
    if(lines[head_ptr].image == NULL)
        return;
    flipImage(lines[head_ptr].image, 32,
        lines[head_ptr].width, lines[head_ptr].height);
    
    // Duplicate string. Note that strdup() allocates new data.
    lines[head_ptr].text = strdup(text);
    
    // Set time interval to be 0.0 since messages won't be displayed when
    // console is pulled down.
    lines[head_ptr].expire_time = 0.0;
    
    // Line will exist only when console is pulled down
    lines[head_ptr].pulldown_only = true;
    
    // Increment only head_ptr to next open position - line_count specifically
    // controls only how many lines of text to be displayed when the console
    // is pulled up, not down. As such, system messages will only appear
    // when the console is down (e.g. messages immediately expire).
    head_ptr++;
    
    // Bounds check for head_ptr
    if(head_ptr >= CON_MAX_LINES)
        head_ptr = 0;
}

/*******************************************************************************
    Console Pulldown/Pullup Routine
*******************************************************************************/

/*******************************************************************************
    function    :   console_module::togglePullDown
    arguments   :   <none>
    purpose     :   Turns console to be pulled down (on) or pulled up (off).
    notes       :   Simply swaps the boolean value to control whenever the
                    console is pulled down or pulled up as well as setting
                    the destination y val of the console pulldown animation.
*******************************************************************************/
void console_module::togglePullDown()
{
    // Swap pulldown boolean
    pulldown = !pulldown;

    // Determine where the bottom of the console will go
    if(pulldown)
        dest_down = (float)game_setup.screen_height * CON_PULLDOWN_SPOT;    // Pulldown
    else
        dest_down = 0.0;                                        // Pullup
    
    // Set command line entering mode to false
    cmd_entering = false;
}

/*******************************************************************************
    Console Command Handler Routines
*******************************************************************************/

/*******************************************************************************
    function    :   console_module::cmdEnter
    arguments   :   char_code - ascii character code (from 0x00 to 0x7F)
    purpose     :   Basically handles keyboard input from the keyboard event
                    handler and places the corresponding ascii characters into
                    the command string. Also handles "opening" and "closing" of
                    the command line (done via Enter).
    notes       :   1) It is assumed that this command will ONLY be called after
                       a check to make sure the console is in the pulled-down
                       state via the isPulledDown() accessor function. This
                       function does not check to see if the console is pulled
                       down or not before preceeding.
                    2) Enter (ascii 0x0a or 0x0d) essentially "opens" & "closes"
                       command line entering mode. After a command line is open,
                       only backspace and valid ascii characters are accepted.
                    3) The command line is stored in command[] until the command
                       line is closed, where it is then sent to the cmd handler.
*******************************************************************************/
void console_module::cmdEnter(char char_code)
{
    // Do not proceed if we are sent a control character. Only proceed if we
    // are sent a command line opening/closing (enter key) or (if we are in
    // command line entering mode) a backspace (8) or ascii character (>= ' ').
    if(!(char_code == 10 || char_code == 13 ||
       (cmd_entering && (char_code == 8 || char_code >= ' '))))
        return;
    
    // See if we are opening the command line
    if(!cmd_entering && char_code == 13 || char_code == 10)
    {
        // If so, set command enter mode to true and set up vars
        cmd_entering = true;
        command[0] = '>';       // All commands begin with '>'.
        command[1] = '\0';      // Not to mention that these are not cleared
        cmd_strlen = 1;         // anywhere else. Very important that they
        cmd_backpos = 0;        // are cleared here before continuing.
        return;
    }
    
    // See if we are closing the command line
    if(cmd_entering && char_code == 13 || char_code == 10)
    {
        // If so, pass to command parsing handler and set entering mode to false
        cmd_handler();
        cmd_entering = false;
        return;
    }
    
    // The following code will add the character to the command string line.
    // Execution will not be here if we are not in command line entering mode.
    
    // Special case the backspace character, 0x08
    if(char_code == 8)
    {
        cmd_strlen--;                   // Back-up
        
        if(cmd_strlen < 1)              // Bounds check (do not overwrite the
            cmd_strlen = 1;             // initial '>').
            
        command[cmd_strlen] = '\0';     // Overwrite with null terminator
    }
    // Make sure we can add the character to our array
    else if(cmd_strlen < (CON_CMD_MAX - 2))
    {
        // Add character, follow-up with new null terminator
        command[cmd_strlen++] = char_code;
        command[cmd_strlen] = '\0';
    }
}

/*******************************************************************************
    function    :   console_module::cmdSpecial
    arguments   :   operation - operation to perform (see CON_CMD_* in .h)
    purpose     :   Performs special command line operations such as previous
                    command retrieval, abort, etc.
    notes       :   1) Due to the nature of circular arrays in general, the code
                       presented here is not very pretty at all.
                    2) All commands are said to start with the '>' character. If
                       any other message was added with a '>' first character it
                       will be mistaken as a command.
*******************************************************************************/
void console_module::cmdSpecial(int operation)
{
    int i, j;
    int line_pos;
    int last_pos;       // Used for commands repeat detection
    
    // Operation Handler
    switch(operation)
    {
        // Previous command look-up handler
        case CON_CMD_PREVIOUS:
            cmd_backpos += 2;   // Will get decremented to 1 here in a sec...
            
        case CON_CMD_NEXT:
            cmd_backpos--;      // ;)
            
            // Bounds check for cmd_packpos. If it tries to go beyond the
            // current command line, set command line to blank and return.
            if(cmd_backpos < 1)
            {
                cmd_backpos = 0;
                strcpy(command, ">");
                cmd_strlen = 1;
                return;
            }
            
            // Start at current position in our console lines, and go backwards
            // searching for previous commands (of which start with ">".) Yes,
            // not very pretty code.
            i = j = 0;      // Initialize our counters
            last_pos = -1;  // Used for commands repeat detection
            for(line_pos = (head_ptr == 0) ? CON_MAX_LINES - 1 : head_ptr - 1;
                i < CON_MAX_LINES && lines[i].text;
                line_pos = (line_pos == 0) ? CON_MAX_LINES - 1 : line_pos - 1)
            {
                // Check to see if current line is a command and is not the
                // same command as previously checked (aka command repeat).
                if(lines[line_pos].text[0] == '>' &&
                   (last_pos == -1 || (last_pos != -1 &&
                   strcmp(lines[line_pos].text, lines[last_pos].text) != 0)))
                {
                    // And if it is a command and not a repeat command, then
                    // increment our counter and update last_pos.
                    last_pos = line_pos;
                    j++;                    // back-trace counter
                }
                
                // If we are as far back as we care to go, use this command
                // as our new command. Copy it over into command[], set the
                // new string length, and return.
                if(j == cmd_backpos)
                {
                    strcpy(command, lines[line_pos].text);
                    cmd_strlen = strlen(command);
                    return;
                }
                
                i++;    // lines[] bounds check variable
            }
            
            // If we are at this point, then we have gone all the way through
            // our list and have essentially overstepped the bounds with the
            // current value of cmd_backpos. So decrement as a form of bounds
            // checking.
            cmd_backpos--;
            break;
            
        // Command entry abort handler
        case CON_CMD_ABORT:
            // Simple enough - just set the cmd_entering to false.
            cmd_entering = false;
            break;
    }
}

/*******************************************************************************
    function    :   console_module::cmd_handler
    arguments   :   <none>
    purpose     :   Handles commands entered into the console. Somewhat messy
                    due to the diversity of commands able to be ran.
    notes       :   1) Static handler code will be replaced with seperate
                       script handler functions calls as such script handlers
                       are implemented.
                    2) Makes a copy of command[] using addSysMessage so that
                       commands typed in can be seen.
                    3) command[] is edited by setting all chars to lowercase.
*******************************************************************************/
void console_module::cmd_handler()
{   
    // Check to see if we're sent a blank command (which essentially will skip
    // the handler and close the command line without launching a command).
    // This basically makes it so if you open then immediately close a command
    // line nothing will really happen.
    if(cmd_strlen <= 1)
        return;
    
    // Up until this point, the command line has been displaying by means of
    // just checking to see if a command line entering is enabled. It never
    // existed in our lines, therefore, it would be kinda nice to make sure
    // that you can see your command line you have typed in. This ensures
    // that the command line is entered into our lines.
    addSysMessage(command);
    
    // Removes the '>' in spot 0
    script.handler(&command[1]);
}

/*******************************************************************************
    Base Display & Update Routines
*******************************************************************************/

/*******************************************************************************
    function    :   console_module::display
    arguments   :   <none>
    purpose     :   Handles the displaying of the console to the screen through
                    the use of OpenGL commands.
    notes       :   1) Relies on the usage of the fonts.renderText() function
                       for command line display - which might be a tad slow.
                    2) All text can be drawn either from bottom of the display
                       screen to top of the screen or top of the screen to
                       the bottom of the screen. The simple changing of the
                       plus or minus value associated with the increment of
                       the y variable changes the entire view.
                       Thus, drawing from bottom to top results in the newest
                       messages being at the bottom where as drawing from top
                       to the bottom is the exact opposite.
                    3) For sake of argument, it is well to be known that we
                       draw our on-screen messages (console pulled up) as newest
                       message on top. This is actually known as the descending
                       ordering, of which can always be tweaked around.
*******************************************************************************/
void console_module::display()
{
    int i;
    int x, y;
    int line_pos;
    
    // First we draw all the communication lines to the screen. Note that if
    // there are no current com lines up, nothing is drawn.
    if(com_line_count > 0)
    {
        // Start drawing at the defined point based on game_setup.screen_height
        y = (int)((float)game_setup.screen_height * CON_COM_START_SPOT);
        // The "goofy" lines[] loop. We start at the last filled position,
        // which is head_ptr - 1, and go backwards (looping around if need be)
        // until we go through all our lines (counted by i for sake of ease).
        // Communication lines are written in a larger font.
        i = 0;
        for(
            line_pos =
            (com_head_ptr == 0) ? CON_MAX_COM_LINES - 1 : com_head_ptr - 1;
            i < com_line_count;
            line_pos = (line_pos == 0) ? CON_MAX_COM_LINES - 1 : line_pos - 1)
        {   
            // Write text
            glRasterPos2i(10, y);
            glDrawPixels(
                com_lines[line_pos].width,
                com_lines[line_pos].height,
                GL_RGBA, GL_UNSIGNED_BYTE,
                (void*)com_lines[line_pos].image);
            
            // Advance to next line
            y += 25;
            
            // Count how many lines we have done
            i++;
        }
    }
    
    // Second, we draw the console lines. However, we only draw it one way
    // or another: If the console is pulled down then we draw all lines. If
    // the console is pulled up then we only draw the lines which line_count
    // controls (which again is based on the expire_time value).
    // Check to see if we draw the console in the pulled down state, of which
    // the curr_down value will not be 0.0.
    if(curr_down != 0.0)
    {
        x = 1000;                               // Width of console (tweaked)
        y = (int)curr_down + 18;                // Bottom y val of console
        
        // We produce the nice console window currently by a simple set of
        // box forming commands. In the future, this might be replaced by a
        // nice-looking image BLIT.
        
        // Draw the nice dark-grey semi-transparent background
        glColor4f(0.2, 0.2, 0.2, 0.85);
        glBegin(GL_QUADS);
            glVertex2i(1, 1);
            glVertex2i(x, 1);
            glVertex2i(x, y);
            glVertex2i(1, y);
        glEnd();
        
        // Enable smooth shading for line drawing and set linewidth to 3.0
        glEnable(GL_LINE_SMOOTH);
        glLineWidth(3.0);
        
        // Draw the nice silver border around the transparent background
        glColor4f(0.9, 0.9, 0.9, 1.0);
        glBegin(GL_LINE_LOOP);
            glVertex2i(1, 1);
            glVertex2i(x, 1);
            glVertex2i(x, y);
            glVertex2i(1, y);
        glEnd();
        
        // Reset the values we just set
        glLineWidth(1.0);
        glDisable(GL_LINE_SMOOTH);
        
        // Advance to where lines will start being drawn (5 is tweaked)
        y = (int)curr_down + 15;

        // Draw all messages from bottom to top which will show our messages
        // as oldest message at top e.g. normal ascending ordering.
        
        // If we are in command line entering mode, the very bottom line will
        // always be the current command line. We don't want it jiggling around
        // so this is always done first before any lines[] are displayed.
        if(cmd_entering)
        {
            // Write current command text (slow method - but it works)
            fonts.renderText(command, 10, y - 18);
            
            // Advance to next line
            y -= 16;
        }
        
        // The "goofy" lines[] loop. We start at the last filled position,
        // which is head_ptr - 1, and go backwards (looping around if need be)
        // until we go through all our lines (counted by i for sake of ease).
        i = 0;
        for(line_pos = (head_ptr == 0) ? CON_MAX_LINES - 1 : head_ptr - 1;
            i < CON_MAX_LINES && lines[i].text && y > 0;
            line_pos = (line_pos == 0) ? CON_MAX_LINES - 1 : line_pos - 1)
        {
            // Write text
            glRasterPos2i(10, y);
            glDrawPixels(
                lines[line_pos].width,
                lines[line_pos].height,
                GL_RGBA, GL_UNSIGNED_BYTE,
                (void*)lines[line_pos].image);
            
            // Advance to next line
            y -= 16;
            
            // Count how many lines we have done
            i++;
        }
    }
    else
    {
        // Check to make sure we have on-screen lines to draw when the console
        // is in the pulled-up position. This is controled via line_count which
        // represents how many messages have expire_time > 0.0.
        if(line_count == 0)
            return;
            
        // The "goofy" lines[] loop. We start at the last filled position,
        // which is head_ptr - 1, and go backwards (looping around if need be)
        // until we go through all our lines (counted by i for sake of ease).
        y = 23;     // Start at this tweaked value at the top of the screen.
        i = 0;
        for(line_pos = (head_ptr == 0) ? CON_MAX_LINES - 1 : head_ptr - 1;
            i < line_count && i < 8;
            line_pos = (line_pos == 0) ? CON_MAX_LINES - 1 : line_pos - 1)
        {
            if(!lines[line_pos].pulldown_only)
            {
                // Count how many lines we have done
                // Write text
                glRasterPos2i(5, y);
                glDrawPixels(
                    lines[line_pos].width,
                    lines[line_pos].height,
                    GL_RGBA, GL_UNSIGNED_BYTE,
                    (void*)lines[line_pos].image);
                
                // Count how many lines we have done
                y += 16;
            
                // Count how many lines we have done
                i++;
            }
        }
    }
}

/*******************************************************************************
    function    :   console_module::update
    arguments   :   deltaT - time elapsed since last call (in seconds)
    purpose     :   Handles the updation of the console. Messages will appear
                    for only so long, the console has the drop-down animation,
                    etc., all of which is updated here-in.
    notes       :   1) It is very important to realize that line_count ONLY
                       counts the number of lines (from the head_ptr backwards)
                       to be displayed when the console is in the pulled up
                       state. Decrementing this value will cause the most latter
                       message to not display anymore (effectively "removing" it
                       from the on-screen display). It is assured that the dec-
                       rementing of this value will only effect the most latter
                       messages due to the nature of the circular array.
                    2) Console pulldown and pullup animations are updated
                       here-in.
*******************************************************************************/
void console_module::update(float deltaT)
{
    int i;
    
    // We simply just go through our entire array and update all the timers
    // if text at that spot exists. If the expire_time is expired, the message
    // is no longer displayed by a decrement of line_count. Note that lines
    // that only appear when the console is pulled down have an expire_time
    // of 0.0, and thus no updation is ever done upon them.
    for(i = 0; i < CON_MAX_LINES; i++)
        if(lines[i].text && lines[i].expire_time > 0.0)
        {
            lines[i].expire_time -= deltaT;
            
            if(lines[i].expire_time <= 0.0)
                line_count--;
        }
    
    // We simply just go through our entire array and update all the timers
    // if text at that spot exists. If the expire_time is expired, the message
    // is no longer displayed by a decrement of line_count.
    for(i = 0; i < CON_MAX_COM_LINES; i++)
        if(com_lines[i].text && com_lines[i].expire_time > 0.0)
        {
            com_lines[i].expire_time -= deltaT;
            
            if(com_lines[i].expire_time <= 0.0)
                com_line_count--;
        }
    
    // Smooth console pull-down animation. If the curr_down is not at the
    // dest_down then the console needs animation work performed (sounds like
    // we're going into surgery when we say it that way :P).
    if(curr_down != dest_down)
    {
        // If we are pulling down the console, it will need to go in the
        // positive direction, where as if we are pulling up the console it
        // wil need to go in the negative direction (the bounds checks are
        // different as well). The speed value for this animation is keyed
        // into 1400.0 pixels per second - tweaked value.
        if(pulldown)
        {
            // Update position based on speed * delta time
            curr_down += 1400.0 * deltaT;
            
            // Bounds check
            if(curr_down > dest_down)
                curr_down = dest_down;
        }
        else
        {
            // Update position based on speed * delta time
            curr_down -= 1400.0 * deltaT;
            
            // Bounds check
            if(curr_down < dest_down)
                curr_down = dest_down;
        } 
    }
}
