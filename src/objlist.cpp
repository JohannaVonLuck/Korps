/*******************************************************************************
                          Object List - Implementation
*******************************************************************************/
#include "main.h"
#include "objlist.h"
#include "object.h"
#include "objunit.h"

/*******************************************************************************
    function    :   object_list::object_list
    arguments   :   <none>
    purpose     :   Constructor.
    notes       :   <none>
*******************************************************************************/
object_list::object_list()
{
    ol_head = NULL;
}

/*******************************************************************************
    function    :   object_list::~object_list
    arguments   :   <none>
    purpose     :   Deconstructor.
    notes       :   <none>
*******************************************************************************/
object_list::~object_list()
{
    empty();
}

/*******************************************************************************
    function    :   object_list::add
    arguments   :   cond - true/false coorisponding to hide or show
    purpose     :   changes the obj_ptr->selected to the given condintion
    notes       :   <none>
*******************************************************************************/
void object_list::setSelected(bool cond)
{
    ol_node* curr = ol_head;
    while(curr)
    {
        (dynamic_cast<unit_object*>(curr->obj_ptr))->selected = cond;
        curr = curr->next;
    }
}

/*******************************************************************************
    function    :   object_list::add
    arguments   :   objPtr - pointer to an object
    purpose     :   Adds an object pointer onto the end of the current stored
                    object list. Automatically makes sure object isn't already
                    defined in list (so no repeats occur).
    notes       :   <none>
*******************************************************************************/
void object_list::add(object* objPtr)
{
    ol_node* prev = NULL;
    ol_node* curr = ol_head;
    
    // Get to end of list - checking for repeats as we go
    while(curr)
    {
        if(curr->obj_ptr == objPtr)
            return;

        prev = curr;
        curr = curr->next;
    }
    
    // Create new object
    curr = new ol_node;
    curr->obj_ptr = objPtr;
    curr->next = NULL;
    
    // Insert into list
    if(prev == NULL)
        ol_head = curr;
    else
        prev->next = curr;
}

/*******************************************************************************
    function    :   object_list::add
    arguments   :   objList - pointer to an object list
    purpose     :   Appends an object list onto the end of the currently stored
                    object list. Automatically makes sure object isn't already
                    defined in list (so no repeats occur).
    notes       :   Object list passed in is NOT effected in any way.
*******************************************************************************/
void object_list::add(object_list* objList)
{
    ol_node* curr = objList->ol_head;
    
    // Go through passed list adding objects as required.
    while(curr)
    {
        add(curr->obj_ptr);
        curr = curr->next;
    }
}

/*******************************************************************************
    function    :   object_list::fastAdd
    arguments   :   objPtr - pointer to an object
    purpose     :   Quickly inserts an object as the head of the list. This
                    method provides no checking capabilities to make sure
                    objects do not repeat themselves.
    notes       :   Should only be used if it is assured that object repeats
                    will not occur (otherwise add() should be used).
*******************************************************************************/
void object_list::fastAdd(object* objPtr)
{
    ol_node* curr;
    
    // Create new object & insert as head object
    curr = new ol_node;
    curr->obj_ptr = objPtr;
    curr->next = ol_head;
    ol_head = curr;
}

/*******************************************************************************
    function    :   object_list::fastAdd
    arguments   :   objList - pointer to an object list
    purpose     :   Appends an object list as the head of the currently stored
                    object list. This method provides no checking capabilities
                    to make sure objects do not repeat themselves.
    notes       :   1) Object list passed in is NOT effected in any way.
                    2) Should only be used if it is assured that object repeats
                       will not occur (otherwise add() should be used).
*******************************************************************************/
void object_list::fastAdd(object_list* objList)
{
    ol_node* curr = objList->ol_head;
    
    // Go through passed list adding objects as required.
    while(curr)
    {
        fastAdd(curr->obj_ptr);
        curr = curr->next;
    }
}

/*******************************************************************************
    function    :   object_list::remove
    arguments   :   objPtr - pointer to an object
    purpose     :   Removes objects from the list (if present).
    notes       :   <none>
*******************************************************************************/
void object_list::remove(object* objPtr)
{
    ol_node* prev = NULL;
    ol_node* curr = ol_head;
    
    // Go through list looking for the said object
    while(curr)
    {
        if(curr->obj_ptr == objPtr)
        {
            // Found it
            if(prev == NULL)
                ol_head = ol_head->next;
            else
                prev->next = curr->next;
            delete curr;
            return;
        }
        
        prev = curr;
        curr = curr->next;
    }
}

/*******************************************************************************
    function    :   object_list::remove
    arguments   :   objList - pointer to an object list
    purpose     :   Removes a series of objects from the list (if any of them
                    are present).
    notes       :   Object list passed in is NOT effected in any way.
*******************************************************************************/
void object_list::remove(object_list* objList)
{
    ol_node* curr = objList->ol_head;
    
    // Go through passed list removing objects as required.
    while(curr)
    {
        remove(curr->obj_ptr);
        curr = curr->next;
    }
}

/*******************************************************************************
    function    :   object_list::empty
    arguments   :   <none>
    purpose     :   Completely deallocates the entire list, effectively emptying
                    all stored contents.
    notes       :   <none>
*******************************************************************************/
void object_list::empty()
{
    ol_node* curr = ol_head;
    
    // Go through and delete all nodes
    while(curr)
    {
        ol_head = ol_head->next;
        delete curr;
        curr = ol_head;
    }
    
    ol_head = NULL;
}

/*******************************************************************************
    function    :   object_list::keepNation
    arguments   :   nation - nation identifier
    purpose     :   Filters (saves) any objects in the current list which are 
                    a part of the given nation.
    notes       :   <none>
*******************************************************************************/
void object_list::keepNation(int nation)
{
    ol_node* prev = NULL;
    ol_node* curr = ol_head;
    
    // Go through our list looking for any nodes which are to be filtered
    while(curr)
    {
        if(!isUnitNation(curr->obj_ptr, nation))
        {
            // Remove this node
            if(prev == NULL)
            {
                ol_head = ol_head->next;
                delete curr;
                curr = ol_head;
                continue;
            }
            else
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
    function    :   object_list::keepSide
    arguments   :   side - side identifier
    purpose     :   Filters (save) any objects in the current list which are 
                    a part of the given side.
    notes       :   <none>
*******************************************************************************/
void object_list::keepSide(int side)
{
    ol_node* prev = NULL;
    ol_node* curr = ol_head;
    
    // Go through our list looking for any nodes which are to be filtered
    while(curr)
    {
        if(!isUnitSide(curr->obj_ptr, side))
        {
            // Remove this node
            if(prev == NULL)
            {
                ol_head = ol_head->next;
                delete curr;
                curr = ol_head;
                continue;
            }
            else
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
    function    :   object_list::keepModel
    arguments   :   modelName - character string of the model name
    purpose     :   Filters (save) any objects in the current list which are 
                    have the same model name.
    notes       :   <none>
*******************************************************************************/
void object_list::keepModel(char* modelName)
{
    ol_node* prev = NULL;
    ol_node* curr = ol_head;
    
    // Go through our list looking for any nodes which are to be filtered
    while(curr)
    {
        if(strcmp(curr->obj_ptr->obj_model, modelName) != 0)
        {
            // Remove this node
            if(prev == NULL)
            {
                ol_head = ol_head->next;
                delete curr;
                curr = ol_head;
                continue;
            }
            else
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
    function    :   object_list::keepPlatoon
    arguments   :   platoon - platoon number to keep
    purpose     :   Filters (save) any objects in the current list which are 
                    have the same platoon number.
    notes       :   <none>
*******************************************************************************/
void object_list::keepPlatoon(int platoon)
{
    ol_node* prev = NULL;
    ol_node* curr = ol_head;
    
    // Go through our list looking for any nodes which are to be filtered
    while(curr)
    {
        if((((dynamic_cast<unit_object*>(curr->obj_ptr))->obj_organization & 0x00F0) >> 4) != platoon)
        {
            // Remove this node
            if(prev == NULL)
            {
                ol_head = ol_head->next;
                delete curr;
                curr = ol_head;
                continue;
            }
            else
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
    function    :   object_list::filterNation
    arguments   :   nation - nation identifier
    purpose     :   Filters (removes) any objects in the current list which are 
                    a part of the given nation.
    notes       :   <none>
*******************************************************************************/
void object_list::filterNation(int nation)
{
    ol_node* prev = NULL;
    ol_node* curr = ol_head;
    
    // Go through our list looking for any nodes which are to be filtered
    while(curr)
    {
        if(isUnitNation(curr->obj_ptr, nation))
        {
            // Remove this node
            if(prev == NULL)
            {
                ol_head = ol_head->next;
                delete curr;
                curr = ol_head;
                continue;
            }
            else
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
    function    :   object_list::filterSide
    arguments   :   side - side identifier
    purpose     :   Filters (removes) any objects in the current list which are 
                    a part of the given side.
    notes       :   <none>
*******************************************************************************/
void object_list::filterSide(int side)
{
    ol_node* prev = NULL;
    ol_node* curr = ol_head;
    
    // Go through our list looking for any nodes which are to be filtered
    while(curr)
    {
        if(isUnitSide(curr->obj_ptr, side))
        {
            // Remove this node
            if(prev == NULL)
            {
                ol_head = ol_head->next;
                delete curr;
                curr = ol_head;
                continue;
            }
            else
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
    function    :   object_list::filterModel
    arguments   :   modelName - character string of the model name
    purpose     :   Filters (removes) any objects in the current list which are 
                    have the same model name.
    notes       :   <none>
*******************************************************************************/
void object_list::filterModel(char* modelName)
{
    ol_node* prev = NULL;
    ol_node* curr = ol_head;
    
    // Go through our list looking for any nodes which are to be filtered
    while(curr)
    {
        if(strcmp(curr->obj_ptr->obj_model, modelName) == 0)
        {
            // Remove this node
            if(prev == NULL)
            {
                ol_head = ol_head->next;
                delete curr;
                curr = ol_head;
                continue;
            }
            else
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
    function    :   object_list::filterPlatoon
    arguments   :   platoon - platoon number to keep
    purpose     :   Filters (removes) any objects in the current list which are 
                    have the same platoon number.
    notes       :   <none>
*******************************************************************************/
void object_list::filterPlatoon(int platoon)
{
    ol_node* prev = NULL;
    ol_node* curr = ol_head;
    
    // Go through our list looking for any nodes which are to be filtered
    while(curr)
    {
        if((dynamic_cast<unit_object*>(curr->obj_ptr))->obj_organization & 0x00F0 == platoon)
        {
            // Remove this node
            if(prev == NULL)
            {
                ol_head = ol_head->next;
                delete curr;
                curr = ol_head;
                continue;
            }
            else
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
    function    :   object_list::filterAttached
    arguments   :   <none>
    purpose     :   Filters (removes) any objects in the current list which are 
                    attached to player command and control.
    notes       :   Takes into account which side the player is on.
*******************************************************************************/
void object_list::filterAttached()
{
    ol_node* prev = NULL;
    ol_node* curr = ol_head;
    
    // Go through our list looking for any nodes which are to be filtered
    while(curr)
    {
        if(isUnitAttached(curr->obj_ptr))
        {
            // Remove this node
            if(prev == NULL)
            {
                ol_head = ol_head->next;
                delete curr;
                curr = ol_head;
                continue;
            }
            else
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
    function    :   object_list::filterUnattached
    arguments   :   <none>
    purpose     :   Filters (removes) any objects in the current list which are 
                    not attached to player command and control.
    notes       :   Takes into account which side the player is on.
*******************************************************************************/
void object_list::filterUnattached()
{
    ol_node* prev = NULL;
    ol_node* curr = ol_head;
    
    // Go through our list looking for any nodes which are to be filtered
    while(curr)
    {
        if(!isUnitAttached(curr->obj_ptr))
        {
            // Remove this node
            if(prev == NULL)
            {
                ol_head = ol_head->next;
                delete curr;
                curr = ol_head;
                continue;
            }
            else
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
    function    :   object_list::sort
    arguments   :   <none>
    purpose     :   Sorts the list based on object rank.
    notes       :   <none>
*******************************************************************************/
void object_list::sort()
{
    // Do later
    return;
}

/*******************************************************************************
    function    :   object_list::size
    arguments   :   <none>
    purpose     :   Returns the number of elements that the list contains
    notes       :   <none>
*******************************************************************************/
int object_list::size()
{
    ol_node* curr = ol_head;
    int i = 0;

    while(curr != NULL)
    {
        i++;
        curr = curr->next;
    }
    
    return i;
}


/*******************************************************************************
    function    :   object_list::containsNation
    arguments   :   nation - nation identifier
    purpose     :   Determines if any objects in the current list are a part of
                    the given nation.
    notes       :   <none>
*******************************************************************************/
bool object_list::containsNation(int nation)
{
    ol_node* curr = ol_head;
    
    // Go through our list looking for any nodes which satisfy
    while(curr)
    {
        if(isUnitNation(curr->obj_ptr, nation))
            return true;
        
        curr = curr->next; 
    }
    
    return false;
}

/*******************************************************************************
    function    :   object_list::containsSide
    arguments   :   side - side identifier
    purpose     :   Determines if any objects in the current list are a part of
                    the given side.
    notes       :   <none>
*******************************************************************************/
bool object_list::containsSide(int side)
{
    ol_node* curr = ol_head;
    
    // Go through our list looking for any nodes which satisfy
    while(curr)
    {
        if(isUnitSide(curr->obj_ptr, side))
            return true;
        
        curr = curr->next; 
    }
    
    return false;
}

/*******************************************************************************
    function    :   object_list::containsUnit
    arguments   :   objPtr - pointer to an object
    purpose     :   Determines if the given object is in the current list.
    notes       :   <none>
*******************************************************************************/
bool object_list::containsUnit(object* objPtr)
{
    ol_node* curr = ol_head;
    
    // Go through our list looking for any nodes which satisfy
    while(curr)
    {
        if(curr->obj_ptr == objPtr)
            return true;
        
        curr = curr->next; 
    }
    
    return false;
}

/*******************************************************************************
    function    :   object_list::containsAttached
    arguments   :   <none>
    purpose     :   Determines if any objects in the current list are attached
                    to player command and control.
    notes       :   Takes into account which side the player is on.
*******************************************************************************/
bool object_list::containsAttached()
{
    ol_node* curr = ol_head;
    
    // Go through our list looking for any nodes which satisfy
    while(curr)
    {
        if(isUnitAttached(curr->obj_ptr))
            return true;
        
        curr = curr->next; 
    }
    
    return false;
}

/*******************************************************************************
    function    :   object_list::containsUnattached
    arguments   :   <none>
    purpose     :   Determines if any objects in the current list are not
                    attached to player command and control.
    notes       :   Takes into account which side the player is on.
*******************************************************************************/
bool object_list::containsUnattached()
{
    ol_node* curr = ol_head;
    
    // Go through our list looking for any nodes which satisfy
    while(curr)
    {
        if(!isUnitAttached(curr->obj_ptr))
            return true;
        
        curr = curr->next; 
    }
    
    return false;
}

