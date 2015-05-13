/*******************************************************************************
                            Object List - Definition
*******************************************************************************/
#ifndef OBJLIST_H
#define OBJLIST_H

#include "object.h"

// Object List Node
struct ol_node
{
    object* obj_ptr;
    ol_node* next;
};

/*******************************************************************************
    class       :   object_list
    purpose     :   Manages a list of object pointers to encapsulate the idea
                    of lists of objects.
    notes       :   1) Add routines all asure that object repeats do not occur.
                    2) To clear the list, the empty routine is used. To see if
                       a list is empty, the isEmpty routine is used.
                    3) filterUncontrollable does filtering so that the list
                       only contains units the player can "control".
                    4) Sorting is based on rank level of the object.
*******************************************************************************/
class object_list
{
    private:
        ol_node* ol_head;
        
    public:
        object_list();                  // Constructor
        ~object_list();                 // Deconstructor
        
        /* Mutators */
        void add(object* objPtr);
        void add(object_list* objList);
        
        void fastAdd(object* objPtr);
        void fastAdd(object_list* objList);
        
        void remove(object* objPtr);
        void remove(object_list* objList);
        
        void empty();
        
        void keepNation(int nation);
        void keepSide(int side);
        void keepModel(char* modelName);
        void keepPlatoon(int platoon);

        void filterNation(int nation);
        void filterSide(int side);
        void filterModel(char* modelName);
        void filterPlatoon(int platoon);

        void filterAttached();
        void filterUnattached();
        void keepAttached(){ filterUnattached(); }
        void keepUnattached(){ filterAttached(); }
        
        void sort();
        int size();
        
        /* Accessors */
        bool containsNation(int nation);
        bool containsSide(int side);
        bool containsUnit(object* objPtr);
        bool containsAttached();
        bool containsUnattached();
        
        void setSelected(bool cond);
        
        bool isEmpty()
            { return (ol_head == NULL); }
        
        ol_node* getHeadPtr()
            { return ol_head; }
};

#endif

