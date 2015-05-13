/*******************************************************************************
                     Reference Library - Implementation
*******************************************************************************/
#include "main.h"
#include "database.h"
#include "misc.h"

/*******************************************************************************
    function    :   database_module::database_module
    arguments   :   <none>
    purpose     :   Constructor.
    notes       :   <none>
*******************************************************************************/
database_module::database_module()
{
    int i;
    ofstream fout;
    
    // Initialize all our records to be NULL. This is one of the more time
    // consuming constructors, but nevertheless must be done.
    for(i = 0; i < MAX_RECORDS; i++)
    {
        record[i].table = NULL;
        record[i].table_allocation = false;
        record[i].element = NULL;
        record[i].element_allocation = false;
        record[i].value = NULL;
    }
    
    // Initialize our tracking variables to 0
    memory_allocated = 0;
    queries = 0;
    
    // Initially enable load log
    load_log = true;
    
    // Output load log header
    fout.open("Reference/load.log", ios::out);
    fout << "           Korps Reference Library Load Log" << endl << endl;
    fout.close();
}

/*******************************************************************************
    function    :   database_module::~database_module
    arguments   :   <none>
    purpose     :   Deconstructor.
    notes       :   <none>
*******************************************************************************/
database_module::~database_module()
{
    int i;
    ofstream fout;
    
    // Display final load log message - the statistics of the DB during
    // the runtime of the program.
    fout.open("Reference/load.log", ios::app);  // Open load log
    fout << "Exit: DB Statistics" << endl
         << "  Number of Queries  : " << queries << endl
         << "  Hash Table Size    : " << MAX_RECORDS << endl
         << "  Hash Table Usage   : " << record_count << endl
         << "  Data Memory Usage  : "
         << memory_allocated / 1024 << " KB" << endl
         << "  Table Memory Usage : "
         << (MAX_RECORDS * sizeof(record_node)) / 1024 << " KB" << endl
         << "  Total Memory Usage : "
         <<  (MAX_RECORDS * sizeof(record_node) + memory_allocated) / 1024
         << " KB" << endl;
    fout.close();   // Close load log
    
    // Deallocate allocations of data that was allocated using strdup
    for(i = 0; i < MAX_RECORDS; i++)
    {
        if(record[i].table && record[i].table_allocation)
            delete record[i].table;
        if(record[i].element && record[i].element_allocation)
            delete record[i].element;
        if(record[i].value)
            delete record[i].value;
    }
}

/*******************************************************************************
    function    :   unsigned int database_module::hash
    arguments   :   s1 - first portion of string to hash
                    s2 - second portion of string to hash
    purpose     :   String hash function.
    notes       :   Uses string djb2 hash algorithm.
*******************************************************************************/
unsigned int database_module::hash(char* s1, char* s2)
{
    unsigned int hash = 5381;
    char* str = s1;
    int c;

    // Hash first part
    while((c = *str++))
        hash = ((hash << 5) + hash) + c;
        
    // Hash second part
    str = s2;
    while((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash % MAX_RECORDS;
}

/*******************************************************************************
    function    :   char* database_module::query
    arguments   :   table - table to reference
                    element - element to reference
    purpose     :   References data from the hash table using the supplied
                    table name and element name, which resultantly hashes
                    into the hash table and finds the record and returns its
                    corresponding value.
    notes       :   <none>
*******************************************************************************/
char* database_module::query(char* table, char* element)
{
    int insert_pos;
    
    // Keep track of # of queries
    queries++;
    
    // Hash the query string to determine where the record was inserted, loop
    // through until the record is found (or not).
    for(insert_pos = hash(table, element); record[insert_pos].table != NULL;
        insert_pos += 7)
    {
        // Check to see if a table element is located at position
        if(strcmp(record[insert_pos].table, table) == 0 &&
           strcmp(record[insert_pos].element, element) == 0)
            return record[insert_pos].value;
        
        // Loop around if at end of circular hash table
        if(insert_pos + 7 >= MAX_RECORDS)
            insert_pos = insert_pos - MAX_RECORDS;
    }
    
    // Otherwise produce error (return NULL)
    return NULL;
}

/*******************************************************************************
    function    :   
    arguments   :   
    purpose     :   
    notes       :   
*******************************************************************************/
void database_module::insert(char* table, char* element, char* value)
{
    int insert_pos;
    
    // Hash the query string to determine where the record was inserted, loop
    // through until the record is found (or not).
    for(insert_pos = hash(table, element); record[insert_pos].table != NULL;
        insert_pos += 7)
    {
        // Check to see if a table element is located at position
        if(strcmp(record[insert_pos].table, table) == 0 &&
           strcmp(record[insert_pos].element, element) == 0)
        {
            if(record[insert_pos].value)
            {
                memory_allocated -= (strlen(record[insert_pos].value) + 1);
                delete record[insert_pos].value;
            }
            record[insert_pos].value = strdup(value);
            memory_allocated += strlen(value) + 1;
            return;
        }
        
        // Loop around if at end of circular hash table
        if(insert_pos + 7 >= MAX_RECORDS)
            insert_pos = insert_pos - MAX_RECORDS;
    }

    record[insert_pos].table = strdup(table);
    record[insert_pos].table_allocation = true;
    record[insert_pos].element = strdup(element);
    record[insert_pos].element_allocation = true;
    record[insert_pos].value = strdup(value);
    
    memory_allocated += strlen(table) + strlen(element) + strlen(value) + 3;
}

/*******************************************************************************
    function    :   
    arguments   :   
    purpose     :   
    notes       :   
*******************************************************************************/
void database_module::update(char* table, char* element, char* value)
{
    int insert_pos;
    
    // Hash the query string to determine where the record was inserted, loop
    // through until the record is found (or not).
    for(insert_pos = hash(table, element); record[insert_pos].table != NULL;
        insert_pos += 7)
    {
        // Check to see if a table element is located at position
        if(strcmp(record[insert_pos].table, table) == 0 &&
           strcmp(record[insert_pos].element, element) == 0)
        {
            if(record[insert_pos].value)
            {
                memory_allocated -= (strlen(record[insert_pos].value) + 1);
                delete record[insert_pos].value;
            }
            record[insert_pos].value = strdup(value);
            memory_allocated += strlen(value) + 1;
            return;
        }
        
        // Loop around if at end of circular hash table
        if(insert_pos + 7 >= MAX_RECORDS)
            insert_pos = insert_pos - MAX_RECORDS;
    }
}

/*******************************************************************************
    function    :   database_module::loadDirectory
    arguments   :   directory - directory to load all .dat files from
    purpose     :   Loads an entire directory filled with .dat files into
                    the database via calls to loadDatFile.
    notes       :   Platform specific code.
*******************************************************************************/
void database_module::loadDirectory(char* directory)
{
    char buffer[128];
    
    #if !defined(_WIN32)
    // Linux Version
    char file_name[128];
    DIR* dir_ptr;
    dirent64* file_ptr;
    
    // Check to see if the directory is root directory
    if(directory[0] != '/')
    {
        // If not, grab the current working directory for a base
        getcwd(buffer, 128);
        // Add onto the end the directory name
        strcat(buffer, "/");
        strcat(buffer, directory);
    }
    else
    {
        // If so, use the supplied directory name as a base
        strcpy(buffer, directory);
    }
    
    // Open directory
    dir_ptr = opendir(buffer);
    
    // Check for open
    if(!dir_ptr)
    {
        // Write error message if failed
        sprintf(buffer, "DB: Error opening directory \"%s\" for read.",
            directory);
        write_error(buffer);
        return;
    }
    
    // Read through file entries until at end (NULL)
    while((file_ptr = readdir64(dir_ptr)))
    {
        // Check to make sure file is a .dat file
        if(strstr(file_ptr->d_name, ".dat"))
        {
            // Build our filename from our base directory along with a /
            // and then the filename as the file_ptr->d_name tells us.
            strcpy(file_name, buffer);
            strcat(file_name, "/");
            strcat(file_name, file_ptr->d_name);
            loadDatFile(file_name);     // Call loadDatFile function
        }
    }
    
    #else
    // Windows Version - thanks for the code to do this one, Jerod ;)
    char original[128];
    _finddata_t c_file;
    long int hFile;
    
    // Save copy of current working directory
    _getcwd(original, 128);
    
    // Check to see if the directory is root directory
    if(!(directory[1] == ':' && directory[2] == '\\'))
    {
        // If not, grab the current working directory for a base
        _getcwd(buffer, 128);
        // Add onto the end the directory name
        strcat(buffer, "\\");
        strcat(buffer, directory);
    }
    else
    {
        // If so, use the supplied directory name as a base
        strcpy(buffer, directory);
    }
    
    // Change to supplied directory
    chdir(buffer);
    
    // Get the first dat file
    hFile = _findfirst("*.dat", &c_file);
    
    // hFile will be set to -1 by _findfirst if no dat files exist
    if(hFile != -1)
    {
        // Load first dat file
        loadDatFile(c_file.name);
        
        while(_findnext(hFile, &c_file) == 0)
        {
            // Load subsequent dat files
            loadDatFile(c_file.name);
        }
    }
    
    // Call _findclose to free up resources used.
    _findclose(hFile);
    
    // Change back to original working directory.
    chdir(original);
    
    #endif
}

/*******************************************************************************
    function    :   database_module::loadDatFile
    arguments   :   fileName - file to load data from
    purpose     :   Loads from a database file (.dat) and stores the collected
                    data into the hash table database.
    notes       :   1) A quick save on overall memory allocated is to go back
                       through our entire database and look for strings which
                       are the same and have already been allocated. We can
                       set the pointer to point to the same spot in memory thus
                       saving a good amount of memory that gets allocated using
                       strdup(). The bad side is that this operation is done
                       upon every record addition, a timely process if the hash
                       table was ever to start growing to sizes beyond the
                       25000 mark. For 5000, and the systems we're on, this is
                       not a big deal. However, this method does require that
                       each record have it's own boolean to know if it has been
                       allocated new memory, since trying to delete more than
                       one pointer which points to the same memory address will
                       do *bad* things. This increases our overal table index
                       memory usage size by 1 byte per record.
                    2) It is automatically assumed that the initial table name
                       in use will be the fileName's base name (last / to next
                       . in fileName). The code will automatically parse and
                       assign an initial table name (ex: If fileName was
                       "dir/hi.dat", "hi.dat", or "hi", they would all start
                       with an initial table name of "hi").
*******************************************************************************/
void database_module::loadDatFile(char* fileName)
{
    int i;
    char table[32];                     // Temp storage for table name
    char element[32];                   // Temp storage for element name
    char value[128];                    // Temp storage for elment value
    int start_pt;                       // Temp parsing int for table name
    int stop_pt;                        // Temp parsing int for table name
    char buffer[128];
    int insert_pos;                     // Position in hash table to insert
    ifstream fin;
    ofstream fout;                      // Load logging
    int table_count = 1;                // Count tracking for load log
    int attribute_count = 0;            // Count tracking for load log
    
    // Open load log and display load log message
    if(load_log)
    {
        fout.open("Reference/load.log", ios::app);
        fout << "Opening: \"" << fileName << "\"" << endl;
    }
    
    // Open fileName for parsing
    fin.open(fileName, ios::in);
    
    // Check for open
    if(!fin)
    {
        // Print error if opening failed and return
        sprintf(buffer, "DB: Failure loading \"%s\" for read.", fileName);
        write_error(buffer);
        return;
    }
    
    // Create the initial table name from the file name
    start_pt = 0;                   // Initially set to 0
    stop_pt = strlen(fileName);     // Initially set to the strlen (always a <)
    // Check to see where the last '/' appears
    for(i = stop_pt; i >= 0; i--)
        if(fileName[i] == '/' || fileName[i] == '\\')
        {
            // Found it
            start_pt = i + 1;   // Start to the right of the /
            break;              // We're done here
        }
    // Check to see where the first '.' appears
    for(i = start_pt; i < stop_pt; i++)
        if(fileName[i] == '.')
        {
            // Fount it
            stop_pt = i;        // Stop to the left of the . (handled by <)
            break;              // We're done here
        }
    // Copy over string from start_pt to stop_pt
    for(i = start_pt; i < stop_pt; i++)
        table[i - start_pt] = fileName[i];
    table[i - start_pt] = '\0';     // Add NULL terminator
    
    // Eat through junk in stream
    while(fin.peek() <= ' ' && !fin.eof())
        fin.ignore(1);
    
    // Loop through file until we hit the end
    while(!fin.eof())
    {
        // Grab the line
        fin.getline(buffer, 120);
        
        // Make sure that the first character on the line isn't a remark char
        // (e.g. # text). Otherwise we parse the line using the defined layout.
        if(buffer[0] != '#')
        {
            // Parse the line, which should be in the form of "element = value"
            // or just [tablename]. We check for [tablename] second since it is
            // not as likely as that of "element = value" is.
            if(sscanf(buffer, "%s = %s", element, value) == 2)
            {
                // Work around to make sure entire string is read in (not just
                // the first space seperated word, which %s dictates). Ex: If
                // "= Hi There" was the string, only "Hi" would be read due to
                // the usage of %s. We find "Hi" in "= Hi There" after the '=',
                // and then recopy the entire string from the buffer back into
                // value, which will copy everything up to the '\0' of buffer,
                // and effectively copy everything over... Neat, eh?
                strcpy(value, strstr(strstr(buffer, "="), value));
                
                // Check to see if there is a comment on this line & remove
                if(strstr(value, "#") != NULL)
                {
                    // Remove # from value
                    for(i = strlen(value) - 1; i >= 0 && value[i] != '#'; i--)
                        value[i] = '\0';
                    value[i] = '\0';    // Remove '#'
                    // Remove all trailing spaces
                    for(--i; i >= 0 && value[i] <= ' '; i--)
                        value[i] = '\0';
                }
                
                // See if we are setting the table name - if we are then we
                // don't go past setting the table string (thus not adding
                // anything to the database).
                if(strcmp(element, "TABLE") == 0)
                {
                    // Table specified, keep track - tables with empty record
                    // counts are tossed out (e.g. don't count towards vars).
                    if(attribute_count > 0)
                    {
                        // Display load log message
                        if(load_log)
                            fout << "  Table Assigned    : \"" << table << "\""
                                 << endl
                                 << "  Records Processed : " << attribute_count
                                 << endl;
                        table_count++;
                        attribute_count = 0;
                    }
                    
                    // Set the table name
                    strcpy(table, value);
                }
                else
                {
                    // Check for max record load limit
                    if(record_count >= MAX_RECORDS)
                    {
                        write_error("DB: Record count limit reached.");
                        return;
                    }
                    
                    // Hash the query string to determine where the record
                    // should be inserted into.
                    for(insert_pos = hash(table, element);
                        record[insert_pos].table != NULL;
                        insert_pos += 7)
                    {
                        // Loop around if at end of circular hash table
                        if(insert_pos + 7 >= MAX_RECORDS)
                            insert_pos = insert_pos - MAX_RECORDS;
                    }
                    
                    // Go through our entire database of records (which won't
                    // take too much time) and check to see if we can link our
                    // table ptr to a memory location which has been already
                    // allocated, saving us some memory.
                    for(i = 0; i < MAX_RECORDS; i++)    // For table entry
                    {
                        if(record[i].table != NULL &&
                            strcmp(record[i].table, table) == 0)
                        {
                            // If so, we just point the ptr to the same string
                            record[insert_pos].table = record[i].table;
                            break;      // We're done here - break out
                        }
                        else if(record[i].element != NULL &&
                            strcmp(record[i].element, table) == 0)
                        {
                            // If so, we just point the ptr to the same string
                            record[insert_pos].table = record[i].element;
                            break;      // We're done here - break out
                        }
                    }
                    
                    // Go through our entire database of records (which won't
                    // take too much time) and check to see if we can link our
                    // element ptr to a memory location which has been already
                    // allocated, saving us some memory.
                    for(i = 0; i < MAX_RECORDS; i++)    // For element entry
                    {
                        if(record[i].table != NULL &&
                            strcmp(record[i].table, element) == 0)
                        {
                            // If so, we just point the ptr to the same string
                            record[insert_pos].element = record[i].table;
                            break;      // We're done here - break out
                        }
                        else if(record[i].element != NULL &&
                            strcmp(record[i].element, element) == 0)
                        {
                            // If so, we just point the ptr to the same string
                            record[insert_pos].element = record[i].element;
                            break;      // We're done here - break out
                        }
                    }
                    
                    // If the table was not found, then the value we were going
                    // to try and set will currently be NULL
                    if(record[insert_pos].table == NULL)
                    {
                        // Allocate memory (using strdup) to duplicate the
                        // string and set the ptr for table
                        record[insert_pos].table = strdup(table);
                        // Make sure we notify the system that this is a unique
                        // allocation which will recieve a delete upon exit
                        record[insert_pos].table_allocation = true;

                        // Accumulate memory allocated to our tracking variable
                        memory_allocated += strlen(table) + 1;
                    }
                    
                    // If the element was not found, then the val we were going
                    // to try and set will currently be NULL
                    if(record[insert_pos].element == NULL)
                    {
                        // Allocate memory (using strdup) to duplicate the
                        // string and set the ptr for element
                        record[insert_pos].element = strdup(element);
                        // Make sure we notify the system that this is a unique
                        // allocation which will recieve a delete upon exit
                        record[insert_pos].element_allocation = true;
                        
                        // Accumulate memory allocated to our tracking variable
                        memory_allocated += strlen(element) + 1;
                    }
                    
                    // Allocate memory (using strdup) to duplicate the
                    // string and set the ptr for element
                    record[insert_pos].value = strdup(value);
                    
                    // Accumulate memory allocated to our tracking variable
                    memory_allocated += strlen(value) + 1;
                    
                    // Incremenet base record counting
                    record_count++;
                    
                    // Keep record count track for current table
                    attribute_count++;
                }
            }
            else if(sscanf(buffer, "[%s]", table) == 1)
            {
                // Take off that damn ']' at the end that sscanf insists on
                // reading in. Brute force fix.
                if(table[strlen(table) - 1] == ']')
                    table[strlen(table) - 1] = '\0';
                
                // Table specified, keep track - tables with empty record
                // counts are tossed out (e.g. don't count towards vars).
                if(attribute_count > 0)
                {
                    // Display load log message
                    if(load_log)
                        fout << "  Table Assigned    : \"" << table << "\""
                             << endl
                             << "  Records Processed : " << attribute_count
                             << endl;
                    table_count++;
                    attribute_count = 0;
                }          
            }
        }
        
        // Eat through junk on stream
        while(fin.peek() <= ' ' && !fin.eof())
            fin.ignore(1);
    }
    
    // Close dat file
    fin.close();
    
    // Display final load log message and close load log
    if(load_log)
    {
        // Display load log message
        fout << "  Table Assigned    : \"" << table << "\"" << endl
             << "  Unique Attributes : " << attribute_count << endl << endl
             << "  Total Tables Processed : " << table_count << endl << endl;
        fout.close();   // Close load log
    }
}
