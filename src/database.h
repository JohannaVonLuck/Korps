/*******************************************************************************
                        Reference Library - Definition
*******************************************************************************/

#ifndef DATABASE_H
#define DATABASE_H

#define MAX_RECORDS             8087

/*******************************************************************************
    class       :   database_module
    purpose     :   The one the only, the d-a-t-a-b-a-s-e. Stores all knowledge
                    associated with the program. Basically just an overly
                    ellaborate hash table with query and load functions.
    notes       :   1) Loads all base DB data from .dat files. The files take
                       on the form of entry data "%s = %s". These files are
                       made as such to allow users of the program an easy way
                       to mod the game and change things that they feel are
                       incorrect, or anything else that is concieveable.
                    2) Upon loading data, the DB will not allocate memory
                       for new strings if the string already exists in memory
                       (goes back through the database and checks for already
                       existant strings to point to instead of allocating over
                       and over again using strdup()).
*******************************************************************************/
class database_module
{
    private:
        /***********************************************************************
            struct  :   record_node
            purpose :   The base node which powers the database is the simple
                        record_node, which only has one type of datatype (the
                        string).
            notes   :   The three boolean values are used to know if the memory
                        which is pointed at has been allocated by this specific
                        record or not. All three values exist in one byte.
        ***********************************************************************/
        struct record_node
        {
            char* table;                // Table name entry
            char* element;              // Element (attribute) name entry
            char* value;                // Value entry
            bool table_allocation;      // Unique allocation identifiers
            bool element_allocation;
        };
        
        record_node record[MAX_RECORDS];    // Base array of record_node objects
        int record_count;
        
        bool load_log;                      // Load logging
        int memory_allocated;               // Bytes of memory allocated so far
        int queries;                        // # of queries performed
        
        unsigned int hash(char* s1, char* s2);      // Hash function (djb2)
        
    public:
        database_module();                  // Constructor
        ~database_module();                 // Deconstructor
        
        /* Base DB Routines - Query, Insert, Update */
        char* query(char* table, char* element);
        void insert(char* table, char* element, char* value);
        void update(char* table, char* element, char* value);
        
        /* .dat Loading Routines */
        void loadDirectory(char* directory);
        void loadDatFile(char* fileName);
        
        /* Accessors */
        int getQueryCount()
            { return queries; }
        int getMemoryUsage()
            { return memory_allocated + (MAX_RECORDS * sizeof(record_node)); }
        
        /* Mutators */
        void setLoadLog(bool enable = true)
            { load_log = enable; }
};

extern database_module db;

#endif
