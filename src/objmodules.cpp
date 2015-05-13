/*******************************************************************************
                        Object Modules - Implementation
*******************************************************************************/
#include "main.h"
#include "objmodules.h"
#include "atg.h"
#include "console.h"
#include "database.h"
#include "effects.h"
#include "metrics.h"
#include "misc.h"
#include "object.h"
#include "objunit.h"
#include "projectile.h"
#include "scenery.h"
#include "sounds.h"
#include "tank.h"

/*******************************************************************************
    Crew Module
*******************************************************************************/

/*******************************************************************************
    function    :   crew_module::crew_module
    arguments   :   <none>
    purpose     :   Constructor.
    notes       :   <none>
*******************************************************************************/
crew_module::crew_module()
{
    int i, j;
    
    // Initialize variables
    parent = NULL;
    
    crew_count = 0;
    for(i = 0; i < OBJ_CREW_MAX; i++)
    {
        crew_name[i] = NULL;
        crew_pos[i][0] = crew_pos[i][1] = crew_pos[i][2] = 0.0;
        crew_health[i] = 1.0f;
        crew_morale[i] = 1.0f;
        jl_head[i] = NULL;
        curr_job[i] = NULL;
    }
    jl_derelict = NULL;
    
    feedr_count = 0;
    for(i = 0; i < FEED_ROUTINE_MAX; i++)
        for(j = 0; j < OBJ_CREW_MAX; j++)
            feedr[i][0][j][0] = feedr[i][1][j][0] = feedr[i][2][j][0] =
            feedr[i][0][j][1] = feedr[i][1][j][1] = feedr[i][2][j][1] = 0x7F;
}

/*******************************************************************************
    function    :   crew_module::crew_module
    arguments   :   <none>
    purpose     :   Deconstructor.
    notes       :   <none>
*******************************************************************************/
crew_module::~crew_module()
{
    int i;
    job_node* jl_curr;
    
    // Deallocate all job nodes and jobs.
    for(i = 0; i < OBJ_CREW_MAX; i++)
        while((jl_curr = jl_head[i]) != NULL)
        {
            if(jl_curr->unique)
            {
                if(jl_curr->job_ptr->job_text)
                    delete jl_curr->job_ptr->job_text;
                delete jl_curr->job_ptr;
            }
            jl_head[i] = jl_head[i]->next;
            delete jl_curr;
            jl_curr = jl_head[i];
        }
    
    while((jl_curr = jl_derelict) != NULL)
    {
        if(jl_curr->unique)
        {
            if(jl_curr->job_ptr->job_text)
                delete jl_curr->job_ptr->job_text;
            delete jl_curr->job_ptr;
        }
        jl_derelict = jl_derelict->next;
        delete jl_curr;
        jl_curr = jl_derelict;
    }
}

/*******************************************************************************
    function    :   crew_module::job_feed
    arguments   :   job_ptr - Pointer to "filled-out" job being fed into system.
    purpose     :   Feeds jobs into the crew system using the task feeder system
                    as described. Returns true if successful addition, otherwise
                    returns false.
    notes       :   1) Job must have it's properties already assigned before
                       going through function.
                    2) Crew members who are WiA or KiA (e.g. health < 40%) are
                       not ever assigned jobs. This does not apply to morale.
                    3) If a crewmember is not busy at the moment that a job is
                       added into their job list and they have morale >= 40%,
                       then they are automatically assigned to that job.
*******************************************************************************/
bool crew_module::job_feed(job* job_ptr)
{
    int i, j;
    int routine, crewman;
    job_node* jl_curr = NULL;
    bool group_dead = true;
    bool unique_allocation = true;
    
    // Check for valid pointer
    if(job_ptr == NULL)
        return false;
    
    // Grab the feed routine #
    routine = job_ptr->feed_routine;
    
    // Check feed #
    if(routine < 0 || routine >= feedr_count)
        return false;
    
    // Loop through groups
    for(i = 0; i < 3 && group_dead; i++)
    {
        // Loop through crew directs
        for(j = 0; j < OBJ_CREW_MAX && feedr[routine][i][j][0] != 0x7F; j++)
        {
            crewman = feedr[routine][i][j][0];
            
            if(crew_health[crewman] >= 0.40)
            {
                // Allocate new job node
                jl_curr = new job_node;
                jl_curr->job_ptr = job_ptr;
                jl_curr->next = jl_head[crewman];   // Stack
                jl_head[crewman] = jl_curr;
                
                // Handle priority setting
                switch(feedr[routine][i][j][1])
                {
                    case FEED_PRIORITY_HIGH:
                        jl_curr->priority = 3.0;
                        break;
                    
                    case FEED_PRIORITY_MID:
                        jl_curr->priority = 2.0;
                        break;
                    
                    case FEED_PRIORITY_LOW:
                    default:
                        jl_curr->priority = 1.0;
                        break;
                }
                
                // Handle unique allocations
                if(unique_allocation)
                {
                    jl_curr->unique = true;
                    unique_allocation = false;      // Job is no longer unique
                }
                else
                    jl_curr->unique = false;
                
                // Handle automatic assignment of job to a crewmember
                if(!job_ptr->suspended && curr_job[crewman] == NULL &&
                   crew_morale[crewman] >= 0.40)
                {
                    curr_job[crewman] = job_ptr;
                    job_ptr->handled = true;
                    return true;
                }
                
                // Set our group dead flag to false since there were in fact
                // crew alive in this group that this job could be assigned to.
                // This causes it so that we don't try to feed into the next
                // group, as requirement described.
                group_dead = false;
            }
        }
    }
    
    // Job could not be assigned to a group if jl_curr is null
    if(jl_curr == NULL)
        return false;
    
    return true;
}

/*******************************************************************************
    function    :   crew_module::job_assign
    arguments   :   crewman - Which crew member
    purpose     :   Assignment function which is responsible for assigning the
                    highest priority job to active crew members.
    notes       :   1) Active crew members are defined as those of whom have
                       both morale and health >= 40%.
                    2) Crew members who are currently busy with a job, upon
                       calling this function, it is assumed we're looking for
                       a job with a higher priority to be doing. This applies
                       (in most instances) to when a job is set to unlimited
                       time expiration. If a job of higher priority is found,
                       then the job handlement switches over to the new job.
                    3) Although jobs are added in a stack fashion, the usage
                       of >= instead of > in priority checks acertains the
                       fact that we always will use the job that is the oldest,
                       e.g. first to be added, e.g. queue.
*******************************************************************************/
void crew_module::job_assign(int crewman)
{
    job_node* jl_curr = jl_head[crewman];
    job_node* jl_best = NULL;
    job_node* jl_curr_job = NULL;
    
    // Check to see if crewman is active
    if(crew_health[crewman] >= 0.40 && crew_morale[crewman] >= 0.40)
    {
        // Go through looking for a new job that isn't already being handled.
        while(jl_curr)
        {
            // Look for a better 'best' node
            if(!jl_curr->job_ptr->handled && !jl_curr->job_ptr->suspended &&
                (jl_best == NULL || jl_curr->priority >= jl_best->priority))
                jl_best = jl_curr;
            
            // Look for the 'current job' node
            if(curr_job[crewman] && jl_curr->job_ptr == curr_job[crewman])
                jl_curr_job = jl_curr;
            
            jl_curr = jl_curr->next;
        }
        
        // See if we found a 'better'/'new' job to assign to crewmember.
        if(jl_best != NULL)
        {
            if(curr_job[crewman] == NULL)
            {
                // Assign job (crewmember is currently not doing anything)
                curr_job[crewman] = jl_best->job_ptr;
                curr_job[crewman]->handled = true;
            }
            else if(jl_curr_job && jl_best != jl_curr_job &&
                jl_best->priority > jl_curr_job->priority)
            {
                // Assign job (a better job was found for this crewmember)
                curr_job[crewman]->handled = false;
                curr_job[crewman] = jl_best->job_ptr;
                curr_job[crewman]->handled = true;
            }
        }
    }
}

/*******************************************************************************
    function    :   crew_module::initCrew
    arguments   :   parentPtr - Pointer to parent object.
    purpose     :   Initializes crew of given object model as defined by the
                    parent object.
    notes       :   <none>
*******************************************************************************/
void crew_module::initCrew(object* parentPtr)
{
    int i, j, k;
    char* temp;
    char buffer[128];
    
    // Assign parent pointer
    parent = parentPtr;
    
    // Grab crew count
    temp = db.query(parent->obj_model, "CREW_COUNT");
    if(temp)
        crew_count = atoi(temp);
    else
    {
        sprintf(buffer, "Crew: Crew count not defined for \"%s\"",
            parent->obj_model);
        write_error(buffer);
        parent->obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // Grab crew specific
    for(i = 0; i < crew_count; i++)
    {
        // Grab crewmember name
        sprintf(buffer, "CREW%i_NAME", i+1);
        temp = db.query(parent->obj_model, buffer);
        if(temp)
        	crew_name[i] = temp;
        else
        {
            sprintf(buffer, "Crew: Name for crewman %i not defined for \"%s\".",
                i+1, parent->obj_model);
            write_error(buffer);
            crew_name[i] = "Crewman";
            parent->obj_status = OBJ_STATUS_REMOVE;
            return;
        }
        
        // Grab crewmember pos
        sprintf(buffer, "CREW%i_POS", i+1);
        temp = db.query(parent->obj_model, buffer);
        if(temp)
            sscanf(temp, "%f %f %f", &crew_pos[i][0], &crew_pos[i][1],
                &crew_pos[i][2]);
        else
        {
            sprintf(buffer, "Crew: Position for crewman %i not defined for \"%s\".",
                i+1, parent->obj_model);
            write_error(buffer);
            parent->obj_status = OBJ_STATUS_REMOVE;
            return;
        }
        
        // Grab crewmember attachment level
        sprintf(buffer, "CREW%i_ATTACH", i+1);
        temp = db.query(parent->obj_model, buffer);
        if(temp)
        {
                if(strstr(temp, "HULL"))
                    crew_attach[i] = OBJ_ATTACH_HULL;
                else if(strstr(temp, "TURRET"))
                {
                    sscanf(temp, "TURRET%i", &crew_attach[i]);
                    crew_attach[i] += OBJ_ATTACH_TURRET_OFF - 1;    // 1-1? ;)
                    
                    if(parent->obj_type == OBJ_TYPE_TANK ||
                       parent->obj_type == OBJ_TYPE_VEHICLE)
                    {
                        crew_pos[i][0] -= (dynamic_cast<turreted_object*>(parent))->turret_pivot[crew_attach[i] - OBJ_ATTACH_TURRET_OFF][0];
                        crew_pos[i][1] -= (dynamic_cast<turreted_object*>(parent))->turret_pivot[crew_attach[i] - OBJ_ATTACH_TURRET_OFF][1];
                        crew_pos[i][2] -= (dynamic_cast<turreted_object*>(parent))->turret_pivot[crew_attach[i] - OBJ_ATTACH_TURRET_OFF][2];
                    }
                }
                else
                {
                    sprintf(buffer, "Crew: Invalid attachment level \"%s\" for \"%s\".",
                        temp, parent->obj_model);
                    write_error(buffer);
                    parent->obj_status = OBJ_STATUS_REMOVE;
                    return;
                }
        }
        else
        {
            sprintf(buffer, "Crew: Attachment for crewman %i not defined for \"%s\".",
                i+1, parent->obj_model);
            write_error(buffer);
            parent->obj_status = OBJ_STATUS_REMOVE;
            return;
        }
    }
    
    // Grab feed count
    temp = db.query(parent->obj_model, "FEED_ROUTINE_COUNT");
    if(temp)
        feedr_count = atoi(temp);
    else
    {
        sprintf(buffer, "Crew: Feed routine count not defined for \"%s\".",
            parent->obj_model);
        write_error(buffer);
        parent->obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // Grab feed routines
    for(i = 0; i < feedr_count; i++)
    {
        char feed[3][32];
        int group_count;
        
        // Grab feed routine
        sprintf(buffer, "FEED_ROUTINE_%i", i+1);
        temp = db.query(parent->obj_model, buffer);
        if(temp)
        {
            group_count = sscanf(temp, "1:%s 2:%s 3:%s", feed[0], feed[1], feed[2]);
            
            if(group_count < 1 || group_count > 3)
            {
                sprintf(buffer, "Crew: Invalid parameters for feed routine %i for \"%s\".",
                    i+1, parent->obj_model);
                write_error(buffer);
                parent->obj_status = OBJ_STATUS_REMOVE;
                return;
            }
            
            // Assign feeds to the different group levels.
            for(j = 0; j < group_count; j++)
            {
                for(k = 0; k < (int)strlen(feed[j]); k += 2)
                {
                    if(feed[j][k] < '1' || feed[j][k] > '9')
                        feed[j][k] = '1';
                    
                    feedr[i][j][k][0] = feed[j][k] - '1';
                    
                    feedr[i][j][k][1] =
                        (feed[j][k+1] == 'H' ? FEED_PRIORITY_HIGH :
                        (feed[j][k+1] == 'M' ? FEED_PRIORITY_MID :
                                               FEED_PRIORITY_LOW));
                }
            }
        }
        else
        {
            sprintf(buffer, "Crew: Feed routine %i not defined for \"%s\".",
	           i+1, parent->obj_model);
    	    write_error(buffer);
    	    parent->obj_status = OBJ_STATUS_REMOVE;
    	    return;
        }
    }
}

/*******************************************************************************
    function    :   crew_module::enqueJob
    arguments   :   jobText - Text to assocaite job with (duplicated).
                    jobTime - Time spent doing job given a regular rated crew.
                    feedRoutine - Job feed routine to use.
    purpose     :   Enques a new job onto the crew given a job text, time to
                    complete, and a feed routine. Returns an ID # that can be
                    used to reference the module in the future with.
    notes       :   jobID returned is essentially a pointer.
*******************************************************************************/
unsigned int crew_module::enqueJob(char* jobText, float jobTime, int feedRoutine)
{
    job* job_ptr;
    
    // Check for valid parameters
    if(jobText == NULL || jobTime <= 0.0 || feedRoutine < 0 ||
       feedRoutine >= feedr_count)
        return JOB_NULL;
    
    // Allocate a new job
    job_ptr = new job;
    
    // Assign job criticals
    job_ptr->job_text = strdup(jobText);        // Note usage of allocation
    job_ptr->job_time = jobTime;
    job_ptr->feed_routine = feedRoutine;
    job_ptr->handled = false;
    job_ptr->suspended = false;
    
    // Send job through job feeder routine
    if(job_feed(job_ptr))
        // Job was added successfully, return an ID number (a pointer).
        return (unsigned int)job_ptr;
    
    // Job was not added succesfully, return JOB_NULL.
    if(job_ptr->job_text)
        delete job_ptr->job_text;
    delete job_ptr;
    return JOB_NULL;
}

/*******************************************************************************
    function    :   crew_module::isJobFinished
    arguments   :   jobID - Job ID number (job pointer)
    purpose     :   Determines if the job given is finished or not. If finished,
                    it is deallocated from the job list using a finishJob.
    notes       :   1) It is absolutely imperative to note that if this function
                       returns true that it will only do so once and only once,
                       as it sets the jobID to JOB_NULL after doing so.
                    2) Jobs do not finish by themselves - this function must be
                       called in order to free a crew member up from a job, even
                       if his job timer has finished.
                    3) Jobs defined as running an unlimited amount of time
                       always return false from this function, and can only
                       be removed using a direct call to finishJob outside
                       of this function.
*******************************************************************************/
bool crew_module::isJobFinished(unsigned int &jobID)
{
    // See if job is still being performed
    if(jobID == JOB_NULL || ((job*)jobID)->job_time > 0.0 ||
       ((job*)jobID)->job_time == JOB_TIME_UNLIMITED)
        return false;
    
    // We are assured from here that the job is finished, so we force a
    // finishment of the job.
    finishJob(jobID);
    
    return true;
}

/*******************************************************************************
    function    :   crew_module::finishJob
    arguments   :   jobID - Job ID number (job pointer)
    purpose     :   Forces a finishing of a job and removes it from the crew
                    module. Required to use if job is defined as an unlimtied
                    time based job. Called from isJobFinished.
    notes       :   1) Job given is deallocated in this function.
                    2) Job ID is automatically set to JOB_NULL after calling of
                       this function. Therefore, the job can never be referenced
                       again after the calling of this function.
*******************************************************************************/
void crew_module::finishJob(unsigned int &jobID)
{
    int i;
    job_node* jl_prev;
    job_node* jl_curr;
    
    // Check for valid job ID #
    if(jobID == JOB_NULL)
        return;
    
    // Check to see exactly where the job is located at. It may be assigned
    // to multiple crew members, so an entire transversal is required.
    for(i = 0; i < crew_count; i++)
    {
        jl_prev = NULL;
        jl_curr = jl_head[i];
        
        // See if the current job of current crewman is the job in question
        if(curr_job[i] == (job*)jobID)
        {
            curr_job[i] = NULL;
            job_assign(i);          // Assign a new job
        }
        
        // Go through job list of this crew member looking for any nodes which
        // contain the job.
        while(jl_curr)
        {
            if(jl_curr->job_ptr == (job*)jobID)     // Find
            {
                // Check for uniqueness of deallocation.
                if(jl_curr->unique)
                {
                    if(jl_curr->job_ptr->job_text)
                        delete jl_curr->job_ptr->job_text;
                    delete jl_curr->job_ptr;
                }
                
                // Delete node and reset pointers.
                if(jl_prev == NULL)
                {
                    jl_head[i] = jl_head[i]->next;
                    delete jl_curr;
                    jl_curr = jl_head[i];
                    continue;
                }
                else
                {
                    jl_prev->next = jl_curr->next;
                    delete jl_curr;
                    jl_curr = jl_prev->next;
                    continue;
                }
            }
            
            jl_prev = jl_curr;
            jl_curr = jl_curr->next;
        }
    }
    
    // Go through job list of the derelict list looking for any nodes which
    // contain the job.
    jl_prev = NULL;
    jl_curr = jl_derelict;
    while(jl_curr)
    {
        if(jl_curr->job_ptr == (job*)jobID)     // Find
        {
            // Check for uniqueness of deallocation.
            if(jl_curr->unique)
            {
                if(jl_curr->job_ptr->job_text)
                    delete jl_curr->job_ptr->job_text;
                delete jl_curr->job_ptr;
            }
            
            // Delete node and reset pointers.
            if(jl_prev == NULL)
            {
                jl_derelict = jl_derelict->next;
                delete jl_curr;
                jl_curr = jl_derelict;
                continue;
            }
            else
            {
                jl_prev->next = jl_curr->next;
                delete jl_curr;
                jl_curr = jl_prev->next;
                continue;
            }
        }
        
        jl_prev = jl_curr;
        jl_curr = jl_curr->next;
    }
    
    // Set the ID to JOB_NULL so that it may never can be referenced again.
    jobID = JOB_NULL;
}

/*******************************************************************************
    function    :   crew_module::suspendJob
    arguments   :   jobID - Job ID number (job pointer)
    purpose     :   Suspends a current job from being handled or priority value
                    updated, effectively "suspended" the job from handlement.
    notes       :   1) Once suspended, the job is never automatically resumed -
                       it must be explicitly re-enabled using resumeJob routine.
                    2) A crewmember who is currently performing the said job
                       will be re-taskfeed a new job.
*******************************************************************************/
void crew_module::suspendJob(unsigned int jobID)
{
    int i;
    
    // Check for valid job ptr
    if(jobID == JOB_NULL)
        return;
    
    // Set suspension flag to true
    ((job*)jobID)->suspended = true;
    
    // If job is currently being handled, then we need to see which crewmember
    // is handling this job and taskfeed that person a different job.
    if(((job*)jobID)->handled)
    {
        // Set handled flag to false
        ((job*)jobID)->handled = false;
        
        // See which crewmember is working on job
        for(i = 0; i < crew_count; i++)
            if(curr_job[i] == (job*)jobID)
            {
                // Crewmember is working on this job, reassign
                curr_job[i] = NULL;
                job_assign(i);
            }
    }
}

/*******************************************************************************
    function    :   crew_module::resumeJob
    arguments   :   jobID - Job ID number (job pointer)
    purpose     :   Resumes a suspended job, allowing it to be assigned to
                    handle once again.
    notes       :   An automatic re-taskfeed is performed on crewmembers who
                    are not performing any tasks.
*******************************************************************************/
void crew_module::resumeJob(unsigned int jobID)
{
    int i;
    
    // Check for valid job ptr
    if(jobID == JOB_NULL)
        return;
    
    // Set suspension flag to false so that it may be taskfeed once again.
    ((job*)jobID)->suspended = false;
    
    // Re-taskfeed crew members without jobs (don't wait on update to do this).
    for(i = 0; i < crew_count; i++)
        if(curr_job[i] == NULL)
            job_assign(i);
}

/*******************************************************************************
    function    :   crew_module::getCrewmanHealthStatus
    arguments   :   crewMember - Which crew member
    purpose     :   Returns a status string based on health of crewMember.
    notes       :   <none>
*******************************************************************************/
char* crew_module::getCrewmanHealthStatus(int crewMember)
{
    if(crew_health[crewMember] > 0.9)
        return (char*)"Uninjured";
    else if(crew_health[crewMember] > 0.65)
        return (char*)"Lgt Wounds";
    else if(crew_health[crewMember] > 0.2)
        return (char*)"Hvy Wounds";
    else if(crew_health[crewMember] > 0.0)
        return (char*)"Disabled";
    else
        return (char*)"Dead";
}

/*******************************************************************************
    function    :   crew_module::getCrewmanMoraleStatus
    arguments   :   crewMember - Which crew member
    purpose     :   Returns a status string based on morale of crewMember.
    notes       :   We lie a bit, if the health is not >= 40% then we return
                    WiA or KiA.
*******************************************************************************/
char* crew_module::getCrewmanMoraleStatus(int crewMember)
{
    if(crew_health[crewMember] > 0.4)
    {
        if(crew_morale[crewMember] >= 1.0)
            return (char*)"Good";
        else if(crew_morale[crewMember] > 0.80)
            return (char*)"Alarmed";
        else if(crew_morale[crewMember] > 0.60)
            return (char*)"Shaken";
        else if(crew_morale[crewMember] > 0.40)
            return (char*)"Shocked";
        else
            return (char*)"Dismayed";
    }
    else
    {
        if(crew_health[crewMember] > 0.0)
            return (char*)"WiA";
        else
            return (char*)"KiA";
    }
}

/*******************************************************************************
    function    :   crew_module::reduceHealth
    arguments   :   crewMember - Which crew member
                    amount - Amount of damage to inflict (0.0 to 1.0)
    purpose     :   Inflicts physical damage to the given crewmember.
    notes       :   If a crewmembers health falls below 40%, then all of his
                    jobs are re-taskfed through the system.
*******************************************************************************/
void crew_module::reduceHealth(int crewMember, float amount)
{
    // Check for valid parameter
    if(crewMember < 0 || crewMember >= crew_count)
        return;
    
    // Reduce health of crewMember
    if(crew_health[crewMember] > 0.0)
    {
        crew_health[crewMember] -= amount;
        
        if(crew_health[crewMember] < 0.0)
            crew_health[crewMember] = 0.0;
    }
    else
        return;
    
    // If crew health falls below 0.4 then we need to re-feed the job list
    // since the crewman is now technically WiA/KiA
    if(crew_health[crewMember] < 0.4)
    {
        // This becomes quite a more complex operation.
        int i;
        job_node* jl_crewMember_curr;
        job_node* jl_curr;
        job_node* jl_first_find;
        
        // Turn off handlement of any current job.
        if(curr_job[crewMember] != NULL)
        {
            curr_job[crewMember]->handled = false;
            curr_job[crewMember] = NULL;
        }
        
        jl_crewMember_curr = jl_head[crewMember];
        while(jl_crewMember_curr)
        {
            jl_first_find = NULL;
            
            // See if we can find the job elsewhere.
            for(i = 0; i < crew_count; i++)
            {
                jl_curr = jl_head[i];
                while(jl_curr)
                {
                    if(jl_curr->job_ptr == jl_crewMember_curr->job_ptr)
                    {
                        if(jl_first_find == NULL && i != crewMember)
                            jl_first_find = jl_curr;
                        jl_curr->unique = false;    // Falsify all unique flags
                    }
                    jl_curr = jl_curr->next;
                }
            }
            
            jl_head[crewMember] = jl_head[crewMember]->next;
            if(jl_first_find != NULL)
            {
                // Job is still defined elsewhere. (Set unique flag)
                jl_first_find->unique = true;
                
                // Deallocate job node (but not job!)
                delete jl_crewMember_curr;
            }
            else
            {
                // Job was NOT found elsewhere. We will need to re-task feed it.
                if(job_feed(jl_crewMember_curr->job_ptr))
                {
                    // Job fed correctly. Deallocate job node (but not job!)
                    delete jl_crewMember_curr;
                }
                else
                {
                    // If job was NOT fed correctly then we have a situation
                    // where this job is now known as a "derelict". We cannot
                    // delete job because of external communication can not
                    // be updated to be told a different job_id (pointer).
                    // So place this job into the derelict list.
                    jl_crewMember_curr->next = jl_derelict;
                    jl_derelict = jl_crewMember_curr;
                }
            }
            
            jl_crewMember_curr = jl_head[crewMember];
        }
    }
}

/*******************************************************************************
    function    :   crew_module::reduceMorale
    arguments   :   crewMember - Which crew member
                    amount - Amount of damage to morale to inflict (0.0 to 1.0)
    purpose     :   Inflicts morale damage to the given crewmember.
    notes       :   If a crewmembers morale falls below 40%, then he is no
                    longer able to perform his current job or any job assigned
                    to him until it improves.
*******************************************************************************/
void crew_module::reduceMorale(int crewMember, float amount)
{
    // Check for valid parameter
    if(crewMember < 0 || crewMember >= crew_count)
        return;
    
    // Reduce morale of crewMember
    if(crew_morale[crewMember] > 0.0)
    {
        crew_morale[crewMember] -= amount;
        
        if(crew_morale[crewMember] < 0.0)
            crew_morale[crewMember] = 0.0;
    }
    else
        return;
    
    // If crew morale falls below 0.4 then we need to unset any current job.
    if(crew_morale[crewMember] < 0.4 && curr_job[crewMember] != NULL)
    {
        curr_job[crewMember]->handled = false;
        curr_job[crewMember] = NULL;
    }
}

/*******************************************************************************
    function    :   crew_module::update
    arguments   :   deltaT - # of seconds elapsed since previous update
    purpose     :   
    notes       :   <none>
*******************************************************************************/
void crew_module::update(float deltaT)
{
    int i;
    job_node* jl_curr;
    
    // Update crewmembers
    for(i = 0; i < crew_count; i++)
    {
        // Update crew health
        if(crew_health[i] > 0.0 && crew_health[i] < 0.2)
        {
            crew_health[i] -= 0.000666667 * deltaT;
            
            if(crew_health[i] < 0.0)
                crew_health[i] = 0.0;
        }
        
        // Update crew morale
        if(crew_morale[i] < 1.0)
        {
            switch((parent->obj_modifiers & OBJ_MOD_RANK))
            {
                case OBJ_MOD_ELITE:
                    crew_morale[i] += 0.0333333 * deltaT;
                    break;
                
                case OBJ_MOD_VETERAN:
                    crew_morale[i] += 0.02222222 * deltaT;
                    break;
                
                case OBJ_MOD_EXPERIENCED:
                    crew_morale[i] += 0.01333333 * deltaT;
                    break;
                
                case OBJ_MOD_REGULAR:
                    crew_morale[i] += 0.00666667 * deltaT;
                    break;
                
                case OBJ_MOD_GREEN:
                    crew_morale[i] += 0.00333333 * deltaT;
                    break;
                
                case OBJ_MOD_MILITIA:
                default:
                    crew_morale[i] += 0.00166667 * deltaT;
                    break;
            }
            
            if(crew_morale[i] > 1.0)
                crew_morale[i] = 1.0;
        }
        
        // Update priorities of jobs on list
        jl_curr = jl_head[i];
        while(jl_curr)
        {
            if(!jl_curr->job_ptr->suspended && jl_curr->job_ptr->job_time != JOB_TIME_UNLIMITED)
                jl_curr->priority += (jl_curr->priority / 8.0) * deltaT;
            jl_curr = jl_curr->next;
        }
        
        // Update job time and assign jobs to those who have no jobs
        if(curr_job[i] != NULL)
        {
            if(curr_job[i]->job_time > 0.0 && curr_job[i]->job_time != JOB_TIME_UNLIMITED)
                curr_job[i]->job_time -= ((1.33333 * crew_health[i]) - 0.33333) * deltaT;
            
            if(curr_job[i]->job_time == JOB_TIME_UNLIMITED)
                job_assign(i);
        }
        else
            job_assign(i);
    }
}

/*******************************************************************************
    Motor Device
*******************************************************************************/

/*******************************************************************************
    function    :   motor_device::motor_device()
    arguments   :   <none>
    purpose     :   Constructor.
    notes       :   <none>
*******************************************************************************/
motor_device::motor_device()
{    
    // Initialize values
    parent = NULL;
    motor_sound_id = SOUND_NULL;
    
    throttle = 0.0;
    throttle_speed = 0.0;
    
    curr_blockmap_value = -1;
    blockmap_max_speed = 100.0;
    blockmap_multiplier = 1.0;
    suspension_multiplier = 1.0;
    
    curr_gear = 1;
    curr_time = 0.0;
    curr_speed = 0.0;
    curr_rpm = 600.0;
    
    gear_slope = NULL;
    gear_speed = NULL;
    gear_time = NULL;
    gear_count = 0;
    
    time_recompute = false;
    
    motor_life = 1.0;
    motor_pos[0] = motor_pos[1] = motor_pos[2] = 0.0;
    damage_effect_id[0] = damage_effect_id[1] = EFFECT_NULL;
    
    idle_rpm = min_rpm = max_rpm = 600.0;
}

/*******************************************************************************
    function    :   motor_device::~motor_device
    arguments   :   <none>
    purpose     :   Deconstructor
    notes       :   <none>
*******************************************************************************/
motor_device::~motor_device()
{
    // Delete allocated memory
    if(gear_slope)
        delete gear_slope;
    if(gear_speed)
        delete gear_speed;
    if(gear_time)
        delete gear_time;
    
    if(motor_sound_id != SOUND_NULL)
        sounds.killSound(motor_sound_id);
}

/*******************************************************************************
    function    :   motor_device::recompute_time
    arguments   :   <none>
    purpose     :   Resolves for gear and current time based on current speed.
                    Useful for when current speed varies offwards from the
                    functional speed.
    notes       :   <none>
*******************************************************************************/
void motor_device::recompute_time()
{
    int i;
    
    // Special case where speed is less than or equal to zero
    if(curr_speed <= 0.0)
    {
        curr_time = curr_speed = 0.0;
        curr_gear = 1;
        return;
    }
    
    // Determine which gear we are in - start at top and go down
    for(i = gear_count - 1; i >= 0; i--)
        if(curr_speed > gear_speed[i])
            break;
    curr_gear = i + 1;
    
    // Equation to be solved (I need to write it down or I'll forget)
    // y = m * x + b;
    // curr_speed = gslope * curr_time + gbasespeed
    // (y - b) / m = x;
    // (speed - gspeed) / gslope = gtime;
    
    // Recompute time by solving for y in y = m * x + b.
    curr_time =
        ((curr_speed - gear_speed[curr_gear - 1]) / gear_slope[curr_gear - 1])
        + gear_time[curr_gear - 1];     // Offset by init time for gear.
}

/*******************************************************************************
    function    :   motor_device::initMotor
    arguments   :   parentPtr - Pointer to parent object
    purpose     :   Initializes all values associated with the motor module.
    notes       :   <none>
*******************************************************************************/
void motor_device::initMotor(object* parentPtr)
{
    int i;
    char buffer[128];
    char* temp;
    char* mtr_model;
    
    // Set parent pointer
    parent = parentPtr;
    
    // Grab motor model
    mtr_model = db.query(parent->obj_model, "MOTOR");
    
    if(!mtr_model)
    {
        // Write out error & exit
        sprintf(buffer, "Motor: Motor model not defined for \"%s\".",
            parent->obj_model);
        write_error(buffer);
        return;
    }
    
    // Set gear count
    temp = db.query(parent->obj_model, "GEARS_FORWARD");
    if(temp)
        gear_count = atoi(temp);
    else
    {
        // Write out error & exit
        sprintf(buffer, "Motor: Gear count not defined for \"%s\".",
            parent->obj_model);
        write_error(buffer);
        return;
    }
    
    // Allocate memory for our arrays
    gear_speed = new float[gear_count + 1];
    gear_time = new float[gear_count + 1];
    gear_slope = new float[gear_count];
    
    // Init speed and time at 0.0
    gear_speed[0] = 0.0;
    gear_time[0] = 0.0;
    
    // Init our suspension
    temp = db.query(parent->obj_model, "SUSPENSION_EFF");
    if(temp)
        suspension_multiplier = atof(temp);
    else
    {
        // Write out error & exit
        sprintf(buffer, "Motor: Suspension not defined for \"%s\".",
            parent->obj_model);
        write_error(buffer);
        return;
    }
    
    // Init our array of speeds and times
    for(i = 1; i <= gear_count; i++)
    {
        // Grab max speed
        sprintf(buffer, "GEAR%i_MAX_SPEED", i);
        temp = db.query(parent->obj_model,buffer);
        if(temp)
            gear_speed[i] = atof(temp) * 0.277778;
        else
        {
            // Write out error & exit
            sprintf(buffer, "Motor: Gear speed not defined for \"%s\".",
                parent->obj_model);
            write_error(buffer);
            return;
        }
        
        // Grab attain time
        sprintf(buffer, "GEAR%i_ATTAIN_TIME", i);
        temp = db.query(parent->obj_model,buffer);
        if(temp)
            gear_time[i] = atof(temp);
        else
        {
            // Write out error & exit
            sprintf(buffer, "Motor: Gear attain time not defined for \"%s\".",
                parent->obj_model);
            write_error(buffer);
            return;
        }
    }
    
    // Compute slope for each gear
    for(i = 0; i < gear_count; i++)
        gear_slope[i] = (gear_speed[i+1] - gear_speed[i]) /
                        (gear_time[i+1] - gear_time[i]);
    
    // Get max engine RPM
    temp = db.query(mtr_model, "RPM");
    if(temp)
    {
        max_rpm = atof(temp);
        min_rpm = 0.42 * max_rpm;   // Compute min and idle from max
        idle_rpm = 0.45 * max_rpm;
    }
    else
    {
        // Write out error & exit
        sprintf(buffer, "Motor: RPM speed not defined for \"%s\".", mtr_model);
        write_error(buffer);
        return;
    }
    
    // Get engine fuel type and register sound with sound module
    temp = db.query(mtr_model, "FUEL");
    if(temp)
    {
        if(strcmp(temp, "Petrol") == 0)
            motor_sound_id = sounds.addSound(SOUND_TANKS_PETROL, SOUND_LOW_PRIORITY, parent->pos(), SOUND_PLAY_LOOP);
        else if(strcmp(temp, "Diesel") == 0)
            motor_sound_id = sounds.addSound(SOUND_TANKS_DIESEL, SOUND_LOW_PRIORITY, parent->pos(), SOUND_PLAY_LOOP);
        else
        {
            // Write out error & exit
            sprintf(buffer, "Motor: Unsupported fuel type \"%s\" for \"%s\".",
                temp, mtr_model);
            write_error(buffer);
            return;
        }
        
        sounds.setSoundRolloff(motor_sound_id, 0.25f);
        sounds.setSoundGain(motor_sound_id, 0.35f);
    }
    else
    {
        // Write out error & exit
        sprintf(buffer, "Motor: Fuel type not defined for \"%s\".", mtr_model);
        write_error(buffer);
        return;
    }
    
    // Grab motor position
    temp = db.query(parent->obj_model, "MOTOR_POS");
    if(temp)
        sscanf(temp, "%f %f %f", &motor_pos[0], &motor_pos[1], &motor_pos[2]);
    else
    {
        // Estimate
        motor_pos[0] = 0.0;
        motor_pos[1] = parent->size[1] * 0.4f;
        motor_pos[2] = parent->size[2] * -0.375f;
    }
}

/*******************************************************************************
    function    :   motor_device::setThrottle
    arguments   :   percent - Percentage of throttle (0.0 - 100.0)
    purpose     :   Sets throttle control variables
    notes       :   <none>
*******************************************************************************/
void motor_device::setThrottle(float percent)
{
    // Set throttle and the max speed relative to the new setting
    throttle = percent / 100.0;
    throttle_speed = throttle * gear_speed[gear_count];
}

/*******************************************************************************
    function    :   motor_device::reduceMotorLife
    arguments   :   amount - Amount of damage to apply to engine (0.0 to 1.0)
    purpose     :   Reduces motor health by applying an amount of damage.
    notes       :   <none>
*******************************************************************************/
void motor_device::reduceMotorLife(float amount)
{
    int snd_id;
    
    // Do nothing if motor already knocked out
    if(motor_life == 0.0)
        return;
    
    // Absolute value the amount modifier
    amount = fabsf(amount);
    
    if(motor_life >= 0.6 && amount >= motor_life)
    {
        // Take care of instant-death
        // Set health to zero
        motor_life = 0.0;
        
        // Set damage effect (flames and smoke)
        if(damage_effect_id[0] != EFFECT_NULL)
            effects.stopEffect(damage_effect_id[0]);
        if(damage_effect_id[1] != EFFECT_NULL)
            effects.stopEffect(damage_effect_id[1]);
        damage_effect_id[0] = effects.addEffect(SE_FIRE,
            parent->transform(motor_pos, OBJ_ATTACH_HULL), -1);
        damage_effect_id[1] = effects.addEffect(SE_BK_BILLOWING_SMOKE,
            parent->transform(motor_pos, OBJ_ATTACH_HULL), -1);
        
        // Kill engine sound, play a motor die sound, switch to flame sound.
        sounds.killSound(motor_sound_id);
        snd_id = sounds.addSound(SOUND_MOTOR_DIE, SOUND_HIGH_PRIORITY,
            parent->transform(motor_pos, OBJ_ATTACH_HULL), SOUND_PLAY_ONCE);
        sounds.setSoundRolloff(snd_id, 0.15f);
        sounds.setSoundGain(snd_id, 0.7f);
        motor_sound_id = sounds.addSound(SOUND_FLAMES, SOUND_LOW_PRIORITY,
            parent->transform(motor_pos, OBJ_ATTACH_HULL), SOUND_PLAY_LOOP);
        sounds.setSoundRolloff(motor_sound_id, 0.25f);
    }
    else if(motor_life > 0.9)
    {
        // Subtract motor health
        motor_life -= amount;
        
        // Handle correct addition of special effect
        if(motor_life <= 0.3)
        {
            if(roll(0.40))
            {
                if(damage_effect_id[0] != EFFECT_NULL)
                    effects.stopEffect(damage_effect_id[0]);
                damage_effect_id[0] = effects.addEffect(SE_BK_BILLOWING_SMOKE,
                    parent->transform(motor_pos, OBJ_ATTACH_HULL), -1);
            }
            // Kill engine sound & play a motor die sound.
            sounds.killSound(motor_sound_id);
            snd_id = sounds.addSound(SOUND_MOTOR_DIE, SOUND_HIGH_PRIORITY,
                parent->transform(motor_pos, OBJ_ATTACH_HULL), SOUND_PLAY_ONCE);
            sounds.setSoundRolloff(snd_id, 0.15f);
            sounds.setSoundGain(snd_id, 0.7f);
        }
        else if(motor_life <= 0.6)
        {
            if(roll(0.60))
            {
                if(damage_effect_id[0] != EFFECT_NULL)
                    effects.stopEffect(damage_effect_id[0]);
                damage_effect_id[0] = effects.addEffect(SE_WH_BILLOWING_SMOKE,
                    parent->transform(motor_pos, OBJ_ATTACH_HULL), -1);
            }
        }
        else if(motor_life <= 0.9)
        {
            if(roll(0.80))
            {
                if(damage_effect_id[0] != EFFECT_NULL)
                    effects.stopEffect(damage_effect_id[0]);
                damage_effect_id[0] = effects.addEffect(SE_QF_WH_SMOKE,
                    parent->transform(motor_pos, OBJ_ATTACH_HULL), -1);
            }
        }
    }
    else if(motor_life > 0.6)
    {
        // Subtract motor health
        motor_life -= amount;
        
        // Handle correct addition of special effect
        if(motor_life <= 0.3)
        {
            if(roll(0.40))
            {
                if(damage_effect_id[0] != EFFECT_NULL)
                    effects.stopEffect(damage_effect_id[0]);
                damage_effect_id[0] = effects.addEffect(SE_BK_BILLOWING_SMOKE,
                    parent->transform(motor_pos, OBJ_ATTACH_HULL), -1);
            }
            // Kill engine sound & play a motor die sound.
            sounds.killSound(motor_sound_id);
            snd_id = sounds.addSound(SOUND_MOTOR_DIE, SOUND_HIGH_PRIORITY,
                parent->transform(motor_pos, OBJ_ATTACH_HULL), SOUND_PLAY_ONCE);
            sounds.setSoundRolloff(snd_id, 0.15f);
            sounds.setSoundGain(snd_id, 0.7f);
        }
        else if(motor_life <= 0.6)
        {
            if(roll(0.60))
            {
                if(damage_effect_id[0] != EFFECT_NULL)
                    effects.stopEffect(damage_effect_id[0]);
                damage_effect_id[0] = effects.addEffect(SE_WH_BILLOWING_SMOKE,
                    parent->transform(motor_pos, OBJ_ATTACH_HULL), -1);
            }
        }
    }
    else if(motor_life > 0.3)
    {
        // Subtract motor health
        motor_life -= amount;
        
        // Handle correct addition of special effect
        if(motor_life <= 0.3)
        {
            if(roll(0.40))
            {
                if(damage_effect_id[0] != EFFECT_NULL)
                    effects.stopEffect(damage_effect_id[0]);
                damage_effect_id[0] = effects.addEffect(SE_BK_BILLOWING_SMOKE,
                    parent->transform(motor_pos, OBJ_ATTACH_HULL), -1);
            }
            // Kill engine sound & play a motor die sound.
            sounds.killSound(motor_sound_id);
            snd_id = sounds.addSound(SOUND_MOTOR_DIE, SOUND_HIGH_PRIORITY,
                parent->transform(motor_pos, OBJ_ATTACH_HULL), SOUND_PLAY_ONCE);
            sounds.setSoundRolloff(snd_id, 0.15f);
            sounds.setSoundGain(snd_id, 0.7f);
        }
    }
    else
        motor_life -= amount;           // Subtract motor health
    
    // Check for overdo
    if(motor_life <= FP_ERROR)
        motor_life = 0.0;
}

/*******************************************************************************
    function    :   motor_device::update
    arguments   :   deltaT - # of seconds elapsed since previous update
    purpose     :   Updates the motor speed output and time values.
    notes       :   <none>
*******************************************************************************/
void motor_device::update(float deltaT)
{
    float desired_speed;
    int new_blockmap_value;
    float new_rpm;
    
    // Grab new blockmap value
    new_blockmap_value = map.getBlockmap(parent->pos[0], parent->pos[2]);
    
    // Check for new blockmap value
    if(new_blockmap_value != curr_blockmap_value)
    {
        // If so, we need to recompute our max speed and multiplier.
        curr_blockmap_value = new_blockmap_value;
        blockmap_max_speed = (((float)(255 - new_blockmap_value) / 255.0)  *
            gear_speed[gear_count]) * suspension_multiplier;
        blockmap_multiplier = (0.2 +
            0.8 * ((float)(255 - curr_blockmap_value) / 255.0)) *
            suspension_multiplier;
    }
    
    // Compute desired speed
    if(motor_life >= 0.9)
    {
        // Desired speed simply the throttle speed
        desired_speed = throttle_speed;
        
        // Check for overdo against blockmap
        if(desired_speed > blockmap_max_speed)
            desired_speed = blockmap_max_speed;
    }
    else if(motor_life > 0.3)
    {
        // Set the desired speed to be a multiplier of the throttle speed
        desired_speed = throttle_speed *
            ((-1.66667f * (motor_life * motor_life)) +
             (3.16667f * motor_life) - 0.5);
        
        // Check for overdo against blockmap
        if(desired_speed > blockmap_max_speed)
            desired_speed = blockmap_max_speed;
    }
    else
        // Motor knocked out
        desired_speed = 0.0;
    
    // Check to see if we are to speed up or to slow down - or nothing.
    if(curr_speed < desired_speed)
    {
        // Speed up
        // See if we need to recompute our time value.
        if(time_recompute)
        {
            recompute_time();
            time_recompute = false;     // Turn boolean back off
        }
        
        // Incremement our current time
        curr_time += deltaT * blockmap_multiplier * motor_life;
        
        // Recompute new speed value
        curr_speed = gear_speed[curr_gear - 1] + (gear_slope[curr_gear - 1] *
            (curr_time - gear_time[curr_gear - 1]));
        
        // Check for overdo
        if(curr_speed > desired_speed)
            curr_speed = desired_speed;
    }
    else if(curr_speed > 0.0 && curr_speed > desired_speed)
    {
        // Slow down
        // Readjust speed to slow down
        curr_speed -= 2.0 * deltaT;
        
        if(curr_speed <= 0.0)
        {
            // Check for complete stop
            curr_speed = 0.0;
            curr_time = 0.0;
            
            // We already know the curr_time to be zero, we don't need to
            // recompute it.
            time_recompute = false;
        }
        else
        {
            // Check for overdo
            if(curr_speed < desired_speed)
                curr_speed = desired_speed;
            
            // Since we have externally changed speed without updating time,
            // upon next speed up, when time is cruticial, we must resolve for
            // the time value.
            time_recompute = true;
        }
    }
    
    // Automatic gear selection check - make sure we are in the correct gear.
    if(curr_gear < gear_count && curr_speed >= gear_speed[curr_gear])
        curr_gear++;        // Upshift if neccessary
    if(curr_gear > 1 && curr_speed < gear_speed[curr_gear - 1])
        curr_gear--;        // Downshift if neccessary
    
    // Calculate new RPM speed
    if(curr_speed <= FP_ERROR || throttle_speed <= FP_ERROR)
        new_rpm = idle_rpm;
    else if(curr_gear == 1)
        new_rpm = (sqrt(curr_speed / gear_speed[curr_gear]) * (max_rpm - min_rpm)) + min_rpm;
    else
        new_rpm = (sqrt((curr_speed - gear_speed[curr_gear - 1]) / (gear_speed[curr_gear] - gear_speed[curr_gear - 1])) * (max_rpm - min_rpm)) + min_rpm;
    curr_rpm += ((new_rpm - curr_rpm) / (new_rpm > curr_rpm ? 3.0 : 1.75)) * deltaT;
    
    // Update sounds with a new RPM value and the new position of the parent
    sounds.setSoundPosition(motor_sound_id, parent->pos());
    sounds.auxModOnRPM(motor_sound_id, curr_rpm);
    
    // Update engine damage
    if(motor_life <= 0.6)
        reduceMotorLife((.004 * deltaT) * (2.85714e-4 * curr_rpm));
    
    // Update damage effect
    if(damage_effect_id[0] != EFFECT_NULL)
        effects.setEffectPosition(damage_effect_id[0], parent->transform(motor_pos, OBJ_ATTACH_HULL));
    if(damage_effect_id[1] != EFFECT_NULL)
        effects.setEffectPosition(damage_effect_id[1], parent->transform(motor_pos, OBJ_ATTACH_HULL));
}

/*******************************************************************************
    Sight Device
*******************************************************************************/

/*******************************************************************************
    function    :   sight_device::sight_device
    arguments   :   <none>
    purpose     :   Constructor.
    notes       :   <none>
*******************************************************************************/
sight_device::sight_device()
{
    // Initialize values
    int i;
    
    parent = NULL;
    
    sight_num = -1;
    sight_type = NULL;
    sight_pivot[0] = sight_pivot[1] = sight_pivot[2] = 0.0;
    sight_attach = OBJ_ATTACH_HULL;
    
    field_of_view = 0.0;
    status = SIGHT_AWAITING;
    
    crew = NULL;
    job_id = JOB_NULL;
    aim_routine = 0;
    
    for(i = 0; i < OBJ_GUN_MAX; i++)
    {
        queue_id[i] = -1;
        queue_velocity[i] = 0.0;
    }
    previous_velocity = 0.0;
    head_pos = 0;
    curr_pos = 0;
    
    target_obj_ptr = NULL;
    target_position[0] = target_position[1] = target_position[2] = 0.0;
    target_spot = TARGET_BASE;
    target_assigned = false;
    target_isa_object = false;
    previous_theta = TWOPI;
    
    elevate_min = elevate_std = elevate_max = elevate_speed = elevate_error =
        desired_elevate = elevate = 0.0;
    
    trans_min = trans_std = trans_max = trans_speed = trans_error =
        desired_trans = transverse = 0.0;
}

/*******************************************************************************
    function    :   sight_device::initSight
    arguments   :   parentPtr - Pointer to parent object
                    sightNum - Sight ID number
    purpose     :   Initializes sighting mechanism based on passed ID number
                    and parent object pointer.
    notes       :   <none>
*******************************************************************************/
void sight_device::initSight(object* parentPtr, int sightNum)
{
    char* temp;
    char element[64];
    char buffer[128];
    
    // Assign base data
    parent = parentPtr;
    sight_num = sightNum;
    
    // Check for valid gun number
    if(sightNum < 0 || sightNum >= OBJ_SIGHT_MAX)
    {
        sprintf(buffer, "Sight: Invalid sight number '%i' for \"%s\".",
            sight_num + 1, parent->obj_model);
        write_error(buffer);
        parent->obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // Grab sight type
    sprintf(element, "SIGHT%i_TYPE", sight_num + 1);
    sight_type = db.query(parent->obj_model, element);
    // Check for valid sight type
    if(!sight_type)
    {
        sprintf(buffer, "Sight: Sight %i type not defined for \"%s\".",
            sight_num + 1, parent->obj_model);
        write_error(buffer);
        parent->obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // Grab pivot point
    sprintf(element, "SIGHT%i_PIVOT", sight_num + 1);
    temp = db.query(parent->obj_model, element);
    if(temp)
        sscanf(temp, "%f %f %f", &sight_pivot[0], &sight_pivot[1], &sight_pivot[2]);
    else
    {
        sprintf(buffer, "Sight: Sight %i pivot not defined for \"%s\".",
            sight_num + 1, parent->obj_model);
        write_error(buffer);
        parent->obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // Grab sight attachment
    sprintf(element, "SIGHT%i_ATTACH", sight_num + 1);
    temp = db.query(parent->obj_model, element);
    if(temp)
    {
        if(strstr(temp, "HULL"))
            sight_attach = OBJ_ATTACH_HULL;
        else if(strstr(temp, "TURRET"))
        {
            sscanf(temp, "TURRET%i", &sight_attach);
            sight_attach += OBJ_ATTACH_TURRET_OFF - 1;      // 1-1? ;)
            
            // Offset sight pivot based on attachment (only for tanks and vehicles)
            if(parent->obj_type == OBJ_TYPE_TANK)
                sight_pivot = sight_pivot - (dynamic_cast<tank_object*>(parent))->turret_pivot[sight_attach - OBJ_ATTACH_TURRET_OFF];
            /*else if(parent->obj_type == OBJ_TYPE_VEHICLE)
                sight_pivot = sight_pivot - (dynamic_cast<vehicle_object*>(parent))->turret_pivot[sight_attach - OBJ_ATTACH_TURRET_OFF];*/
        }
        else
        {
            sprintf(buffer, "Sight: Invalid sight %i attachment level \"%s\" for \"%s\".",
                sight_num + 1, temp, parent->obj_model);
            write_error(buffer);
            parent->obj_status = OBJ_STATUS_REMOVE;
            return;
        }
    }
    else
    {
        sprintf(buffer, "Sight: Sight %i attachment not defined for \"%s\".",
            sight_num + 1, parent->obj_model);
        write_error(buffer);
        parent->obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // Grab FOV for optic.
    temp = db.query(sight_type, "FIELD_OF_VIEW");
    if(temp)
        field_of_view = atof(temp) * degToRad;
    else
    {
        sprintf(buffer, "Sight: FOV for not defined for optic \"%s\".",
            sight_type);
        write_error(buffer);
        parent->obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // Assign crew pointer based on parent object.
    switch(parent->obj_type)
    {
        case OBJ_TYPE_TANK:
        case OBJ_TYPE_VEHICLE:
        case OBJ_TYPE_ATG:
        case OBJ_TYPE_ATR:
            crew = &((dynamic_cast<unit_object*>(parent))->crew);
            break;
        
        default:
            // No device present on object -> abort.
            sprintf(buffer, "Sight: Invalid sight device: \"%s\" is not a firing object.",
                parent->obj_model);
            parent->obj_status = OBJ_STATUS_REMOVE;
            return;
            break;
    }
    
    // Grab aiming routine
    sprintf(element, "SIGHT%i_AIMED_BY", sight_num + 1);
    temp = db.query(parent->obj_model, element);
    if(temp)
    {
        sscanf(temp, "FEED_ROUTINE_%hi", &aim_routine);
        aim_routine--;
    }
    else
    {
        sprintf(buffer, "Sight: No aiming routine defined for sight %i for \"%s\".",
            sight_num + 1, parent->obj_model);
        write_error(buffer);
        parent->obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // Flip min and max (since they mean different things inside/outside of
    // the mathematics involved). Grab sight elevate limits.
    sprintf(element, "SIGHT%i_ELEVATE_MIN", sight_num + 1);
    temp = db.query(parent->obj_model, element);
    if(temp)
        elevate_max = PIHALF - (atof(temp) * degToRad);
    else
    {
        sprintf(buffer, "Sight: Sight %i elevate minimum not defined for \"%s\".",
            sight_num + 1, parent->obj_model);
        write_error(buffer);
        parent->obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // Flip min and max (since they mean different things inside/outside of
    // the mathematics involved). Grab sight elevate limits.
    sprintf(element, "SIGHT%i_ELEVATE_MAX", sight_num + 1);
    temp = db.query(parent->obj_model, element);
    if(temp)
        elevate_min = PIHALF - (atof(temp) * degToRad);
    else
    {
        sprintf(buffer, "Sight: Sight %i elevate maximum not defined for \"%s\".",
            sight_num + 1, parent->obj_model);
        write_error(buffer);
        parent->obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // Grab sight transverse limits.
    sprintf(element, "SIGHT%i_TRANS_MIN", sight_num + 1);
    temp = db.query(parent->obj_model, element);
    if(temp)
        trans_min = -atof(temp) * degToRad;
    
    // Grab sight transverse limits.
    sprintf(element, "SIGHT%i_TRANS_MAX", sight_num + 1);
    temp = db.query(parent->obj_model, element);
    if(temp)
        trans_max = -atof(temp) * degToRad;
    
    // Grab sight normatic (no target) elevation.
    sprintf(element, "SIGHT%i_ELEVATE_STD", sight_num + 1);
    temp = db.query(parent->obj_model, element);
    if(temp)
        elevate_std = PIHALF - (atof(temp) * degToRad);
    
    // Grab sight normatic (no target) transverse.
    sprintf(element, "SIGHT%i_TRANS_STD", sight_num + 1);
    temp = db.query(parent->obj_model, element);
    if(temp)
        trans_std = -atof(temp) * degToRad;
    
    // Initialize some further values
    elevate = desired_elevate = elevate_std;
    transverse = desired_trans = trans_std;
    
    // Although this may not be correct for ALL vehicles, its roughly about
    // a good estimate for all vehicles.
    elevate_speed = TWOPI / 50.0;
    
    // Assign transverse speeds
    if(sight_attach == OBJ_ATTACH_HULL)
    {
        // For a hull attachment, transverse doesn't really matter for any
        // vehicles currently set to go in-game.
        trans_speed = TWOPI / 35.0;
    }
    else
    {
        // Grab turret transverse speed (since this is a very important value)
        sprintf(element, "TURRET%i_SPEED", sight_attach - OBJ_ATTACH_TURRET_OFF + 1);
        temp = db.query(parent->obj_model, element);
        if(temp)
            trans_speed = TWOPI / atof(temp);
        else
            trans_speed = TWOPI / 35.0;
    }
}

/*******************************************************************************
    function    :   sight_device::assignTarget
    arguments   :   objPtr - Pointer to targeted object
                    targetSpot - Position to target on target object
    purpose     :   Assigns a target to the sighting mechanism to start
                    tracking.
    notes       :   <none>
*******************************************************************************/
void sight_device::assignTarget(object* objPtr, short targetSpot)
{
    char* temp = NULL;
    
    // Set some booleans and assign target
    target_assigned = true;
    target_isa_object = true;
    target_obj_ptr = objPtr;
    target_spot = targetSpot;
    target_position[0] = target_position[1] = target_position[2] = 0.0;
    previous_theta = 7.0;
    
    // Assign aiming job
    if(job_id != JOB_NULL)
        crew->finishJob(job_id);
    job_id = crew->enqueJob("Aiming...", JOB_TIME_UNLIMITED, aim_routine);
    
    // Assign the spot of the target to target_position based on this huge
    // switch statement.
    switch(targetSpot)
    {
        case TARGET_HULL:
            target_position[1] = (target_obj_ptr->size[1] / 3.0) + 0.3;
            break;
        
        case TARGET_TURRET1:
            if(target_obj_ptr->obj_type == OBJ_TYPE_TANK)
            {
                target_position[0] =
                    (dynamic_cast<tank_object*>(target_obj_ptr))->turret_pivot[0][0];
                target_position[1] =
                    (dynamic_cast<tank_object*>(target_obj_ptr))->turret_pivot[0][1] + 0.35;
                target_position[2] =
                    (dynamic_cast<tank_object*>(target_obj_ptr))->turret_pivot[0][2];
            }
            break;
        
        case TARGET_TURRET2:
            if(target_obj_ptr->obj_type == OBJ_TYPE_TANK)
            {
                target_position[0] =
                    (dynamic_cast<tank_object*>(target_obj_ptr))->turret_pivot[1][0];
                target_position[1] =
                    (dynamic_cast<tank_object*>(target_obj_ptr))->turret_pivot[1][1] + 0.35;
                target_position[2] =
                    (dynamic_cast<tank_object*>(target_obj_ptr))->turret_pivot[1][2];
            }
            break;
        
        case TARGET_TURRET3:
            if(target_obj_ptr->obj_type == OBJ_TYPE_TANK)
            {
                target_position[0] =
                    (dynamic_cast<tank_object*>(target_obj_ptr))->turret_pivot[2][0];
                target_position[1] =
                    (dynamic_cast<tank_object*>(target_obj_ptr))->turret_pivot[2][1] + 0.35;
                target_position[2] =
                    (dynamic_cast<tank_object*>(target_obj_ptr))->turret_pivot[2][2];
            }
            break;
        
        case TARGET_CREW1:
            temp = db.query(target_obj_ptr->obj_model, "CREW1_POS");
            break;
        
        case TARGET_CREW2:
            temp = db.query(target_obj_ptr->obj_model, "CREW2_POS");
            break;
        
        case TARGET_CREW3:
            temp = db.query(target_obj_ptr->obj_model, "CREW3_POS");
            break;
        
        case TARGET_CREW4:
            temp = db.query(target_obj_ptr->obj_model, "CREW4_POS");
            break;
        
        case TARGET_CREW5:
            temp = db.query(target_obj_ptr->obj_model, "CREW5_POS");
            break;
        
        case TARGET_CREW6:
            temp = db.query(target_obj_ptr->obj_model, "CREW6_POS");
            break;
        
        case TARGET_LEFT_TRACK:
            temp = db.query(target_obj_ptr->obj_model, "TRACK_LEFT_POS");
            break;
        
        case TARGET_RIGHT_TRACK:
            temp = db.query(target_obj_ptr->obj_model, "TRACK_RIGHT_POS");
            break;
        
        case TARGET_LOWER_HULL:
            target_position[1] = target_obj_ptr->size[1] / 4.0;
            break;
        
        case TARGET_UPPER_HULL:
            target_position[1] = (target_obj_ptr->size[1] / 4.0) * 3.0;
            break;
        
        case TARGET_ENGINE:
            temp = db.query(target_obj_ptr->obj_model, "ENGINE_POS");
            break;
        
        case TARGET_SPECIAL1:
            temp = db.query(target_obj_ptr->obj_model, "SPECIAL1_POS");
            break;
        
        case TARGET_SPECIAL2:
            temp = db.query(target_obj_ptr->obj_model, "SPECIAL2_POS");
            break;
        
        case TARGET_SPECIAL3:
            temp = db.query(target_obj_ptr->obj_model, "SPECIAL3_POS");
            break;
        
        case TARGET_SPECIAL4:
            temp = db.query(target_obj_ptr->obj_model, "SPECIAL4_POS");
            break;
        
        case TARGET_SPECIAL5:
            temp = db.query(target_obj_ptr->obj_model, "SPECIAL5_POS");
            break;
    }
    
    // And if the temp value comes back, use it
    if(temp)
        sscanf(temp, "%f %f %f",
            &target_position[0], &target_position[1], &target_position[2]);
}

/*******************************************************************************
    function    :   assignTarget
    arguments   :   position - Some 3D coordinate in 3D space to track towards.
    purpose     :   Reduxed version of target tracking where instead of a
                    vehicle, we use just a static point. Good for when you
                    are just trying to track a point (such as a map coordinate
                    position).
    notes       :   <none>
*******************************************************************************/
void sight_device::assignTarget(float* position)
{
    // Set up the values to do such
    target_assigned = true;
    target_isa_object = false;
    target_obj_ptr = NULL;
    target_spot = TARGET_BASE;
    target_position[0] = position[0];   // Set target_position to the passed
    target_position[1] = position[1];   // position value.
    target_position[2] = position[2];
    previous_theta = 7.0;
    
    // Assign aiming job
    if(job_id != JOB_NULL)
        crew->finishJob(job_id);
    job_id = crew->enqueJob("Aiming...", JOB_TIME_UNLIMITED, aim_routine);
}

/*******************************************************************************
    function    :   sight_device::relieveTarget
    arguments   :   <none>
    purpose     :   Relieves sight of target.
    notes       :   <none>
*******************************************************************************/
void sight_device::relieveTarget()
{
    // Unset assignment values.
    target_assigned = false;
    target_isa_object = false;
    target_obj_ptr = NULL;
    target_spot = TARGET_BASE;
    target_position[0] = target_position[1] = target_position[2] = 0.0;
}

/*******************************************************************************
    function    :   sight_device::enqueueDevice
    arguments   :   deviceID - gun device ID
                    adjustmentVelocity - Velocity of proj. to adjust towards
    purpose     :   Enqueues a gunning device onto the waiting queue for the
                    sighting mechanism. Returns the assignment ID used to
                    access all further operations with the sighting device.
    notes       :   <none>
*******************************************************************************/
int sight_device::enqueueDevice(short deviceID, float adjustmentVelocity)
{
    int insert_id;
    
    // Insert into queue    
    insert_id = head_pos;
    
    queue_id[insert_id] = deviceID;
    queue_velocity[insert_id] = adjustmentVelocity;
    
    head_pos++;                         // Advance head pointer
    
    if(head_pos >= OBJ_GUN_MAX)         // Check for loop-around
        head_pos = 0;
    
    return insert_id;
}

/*******************************************************************************
    function    :   sight_device::dequeueDevice
    arguments   :   sightID - ID assigned to device when it was enqueued
    purpose     :   Takes a device off of the waiting queue.
    notes       :   Must be called by the gunning device.
*******************************************************************************/
void sight_device::dequeueDevice(short sightID)
{
    // Reset assigned ID to -1
    if(sightID >= 0)
        queue_id[sightID] = -1;
}

/*******************************************************************************
    function    :   bool sight_device::isDeviceOkayed
    arguments   :   sightID - ID assigned to device when it was enqueued
    purpose     :   Determines if the gun device is able to be using the sight
                    device.
    notes       :   Used since weapons with similiar projectile velocity rates
                    can both be firing at the same time (ex: Pz III turret MGs).
*******************************************************************************/
bool sight_device::isDeviceOkayed(short sightID)
{
    // If the assigned ID is not invalid (>= 0) and the sightID is either
    // currently lined up on the queue or has the same velocity, then allow.
    if(sightID >= 0 && (sightID == curr_pos ||
       queue_velocity[curr_pos] == queue_velocity[sightID]))
        return true;
    
    return false;
}

/*******************************************************************************
    function    :   sight_device::update
    arguments   :   deltaT - number of seconds elapsed since last update
    purpose     :   Updates the sighting device mechanism.
    notes       :   <none>
*******************************************************************************/
void sight_device::update(float deltaT)
{
    bool trans_on_target = false;
    bool elevate_on_target = false;
    float adjust_velocity;
    
    // Update curr position pointer to head pointer, basically cycling the
    // queue to the next device if the current device has been dequeued.
    while(curr_pos != head_pos && queue_id[curr_pos] == -1)
    {
        curr_pos++;
        
        if(curr_pos >= OBJ_GUN_MAX)
            curr_pos = 0;
    }
    
    // Only elevate/transverse when there is somebody commanding the sight.
    if(job_id != JOB_NULL && crew->getJobHandlement(job_id))
    {
        // Determine if we use a new adjusting velocity or just use the one
        // we have been already using. This is based on if we have a device
        // assigned or not. Not performing this check will make it such that
        // the gun elevates up and down between shells rather than just being
        // left alone.
        if(queue_id[curr_pos] == -1)
            adjust_velocity = previous_velocity;        // Use previous
        else
            adjust_velocity = previous_velocity = queue_velocity[curr_pos];
        
        // Normalize transverse to be between 0.0 and TWOPI
        if(transverse > TWOPI)
            transverse -= TWOPI;
        if(transverse < 0.0)
            transverse += TWOPI;
        
        // If a target is assigned, we must aim at the target.
        if(target_assigned)
        {
            kVector device_pos;
            kVector device_dir;
            kVector target_dir;
            kVector current_dir;
            kVector dir_offset;
            float time_to_target;
            float distance_to_target;
            
            // Determine the device current position and direction based on
            // the linear transformation of itself so that we can get an accurate
            // reading independent of having to do anything fancy for orientation.
            device_pos = parent->transform(sight_pivot, sight_attach);
            device_dir = parent->transform(sight_pivot + vectorIn(kVector(
                1000.0, elevate, (sight_attach == OBJ_ATTACH_HULL ? transverse : 0.0)),
                    CS_CARTESIAN), sight_attach);
            
            current_dir = device_dir - device_pos;      // Dir vector from device
            
            // Determine if we do a *smart* aim or just a standard point aim.
            if(target_isa_object)
            {
                // Perform smart aim based on object type
                if(target_obj_ptr->obj_type == OBJ_TYPE_TANK)
                {
                    if((dynamic_cast<tank_object*>(target_obj_ptr))->linear_vel == 0.0
                       || adjust_velocity == 0.0)
                    {
                        // Target isn't moving - easy aim
                        target_dir =
                            target_obj_ptr->transform(kVector(target_position), 0)
                            - device_pos;
                    }
                    else
                    {
                        // Target IS moving, estimate where we will need to fire
                        distance_to_target = distanceBetween(
                            target_obj_ptr->transform(kVector(target_position), 0),
                            device_pos);
                        time_to_target = distance_to_target / adjust_velocity;
                        
                        // Much more complicated aim    
                        target_dir =
                            (target_obj_ptr->transform(kVector(target_position), 0) 
                               + (vectorIn(target_obj_ptr->dir, CS_CARTESIAN)
                                    * (dynamic_cast<tank_object*>(target_obj_ptr))->linear_vel
                                    * time_to_target))
                            - device_pos;
                    }
                }
                /*else if(target_obj_ptr->obj_type == OBJ_TYPE_VEHICLE)
                {
                    distance_to_target = distanceBetween(
                        target_obj_ptr->transform(kVector(target_position), 0),
                        device_pos);
                    time_to_target = distance_to_target / adjust_velocity;
                    
                    target_dir =
                        (target_obj_ptr->transform(kVector(target_position), 0) + 
                            (target_obj_ptr->dir
                                * ((vehicle_firing_object*)target_obj_ptr)->linear_vel
                                * time_to_target))
                        - device_pos;
                }*/
                else
                {
                    // Standard type handler
                    target_dir =
                        target_obj_ptr->transform(kVector(target_position), 0)
                        - device_pos;
                }
            }
            else
            {
                // No object, just base aim on the 3D coordinate in target_pos
                target_dir = kVector(target_position) - device_pos;
            }
            
            // Run our inverse kinematic solver - only done if the velocity we're
            // adjusting towards exists as not zero.
            if(adjust_velocity != 0.0)
            {
                float theta;
                float iterator;
                float y_offset;
                float error;
                
                // Pick-up from where we last left off (or not).
                if(previous_theta == 7.0)
                {
                    // Start from the beginning :S
                    theta = 0.0;
                    iterator = PIHALF;
                }
                else
                {
                    // Readjust based on previous aiming
                    theta = previous_theta;
                    iterator = PI / 8192.0;
                }
                
                // X distance (disregarding Yaw, this is a 2D problem)
                distance_to_target = 
                    sqrt((target_dir[0] * target_dir[0]) +
                         (target_dir[2] * target_dir[2]));
                
                // Initialize values for first run through
                time_to_target = distance_to_target /
                    (cos(theta) * adjust_velocity);
                y_offset = sin(theta) * adjust_velocity * time_to_target -
                    4.905 * time_to_target * time_to_target;
                
                error = fabsf(target_dir[1] - y_offset);    // Compute error
                
                // While our error is high and our iterator is still large, hom-in
                // on the appropriate theta value which will give us a hit.
                while(error > 0.001 && iterator > 2.996056e-6)
                {
                    iterator /= 2.0;            // Cut iterator in half each time
                    
                    // Determine if we need to lower or raise theta
                    if(y_offset > target_dir[1])
                        theta -= iterator;
                    else
                        theta += iterator;
                    
                    // Recalculate values
                    time_to_target = distance_to_target /
                        (cos(theta) * adjust_velocity);
                    y_offset = sin(theta) * adjust_velocity * time_to_target -
                        4.9 * time_to_target * time_to_target;
                    
                    error = fabsf(target_dir[1] - y_offset);    // Compute error
                }
                
                // Determine Y offset to aim at
                y_offset = sin(theta) * adjust_velocity * time_to_target;
                
                // Assign to our target direction vector
                target_dir[1] = y_offset;
                
                // If we had computed a value with minimized error, save this
                // value for future run-through (efficiency). Otherwise, reset
                // theta to 7.0 which is seen above as meaning start all over.
                if(error <= 0.001)
                    previous_theta = theta;
                else
                    previous_theta = 7.0;
            }
            
            // Convert direction vectors to spherical (for pitch and yaw numbers)
            current_dir.convertTo(CS_SPHERICAL);
            target_dir.convertTo(CS_SPHERICAL);
            
            // And finally compare, producing an offset
            dir_offset = target_dir - current_dir;
            
            // Normalize offset between -PI (to the right) and PI (to the left)
            if(dir_offset[2] > PI)
                dir_offset[2] -= TWOPI;
            if(dir_offset[2] < -PI)
                dir_offset[2] += TWOPI;
            
            // Desired trans/elevate is the current + the offset
            desired_trans = transverse + dir_offset[2];
            desired_elevate = elevate + dir_offset[1];
        }
        else
        {
            // No target -> reset to the standard elevate and transverse setting
            kVector dir_offset = kVector(1.0,
                elevate_std - elevate,              // Relative offseting
                trans_std - transverse);
            
            // Normalize offset between -PI (to the right) and PI (to the left)
            if(dir_offset[2] > PI)
                dir_offset[2] -= TWOPI;
            if(dir_offset[2] < -PI)
                dir_offset[2] += TWOPI;
            
            desired_trans = transverse + dir_offset[2];
            desired_elevate = elevate + dir_offset[1];
        }
        // done setting aim to target
        
        // Update transverse settings based on which direction it needs to go.
        if(transverse > desired_trans + FP_ERROR)
        {
            transverse -= trans_speed * deltaT;
            
            // Check for overdue/on-target
            if(transverse <= desired_trans + FP_ERROR)
            {
                transverse = desired_trans;
                trans_on_target = true;
            }
        }
        else if(transverse < desired_trans - FP_ERROR)
        {
            transverse += trans_speed * deltaT;
            
            // Check for overdue/on-target
            if(transverse >= desired_trans - FP_ERROR)
            {
                transverse = desired_trans;
                trans_on_target = true;
            }
        }
        else
        {
            // Already on-target
            transverse = desired_trans;
            trans_on_target = true;
        }
        
        // Update elevate settings based on which direction it needs to go.
        if(elevate > desired_elevate + FP_ERROR)
        {
            elevate -= elevate_speed * deltaT;
            
            // Check for overdue/on-target
            if(elevate <= desired_elevate + FP_ERROR)
            {
                elevate = desired_elevate;
                elevate_on_target = true;
            }
        }
        else if(elevate < desired_elevate - FP_ERROR)
        {
            elevate += elevate_speed * deltaT;
            
            // Check for overdue/on-target
            if(elevate >= desired_elevate - FP_ERROR)
            {
                elevate = desired_elevate;
                elevate_on_target = true;
            }
        }
        else
        {
            // Already on-target
            elevate = desired_elevate;
            elevate_on_target = true;
        }
        
        // Bounds checking for elevate
        if(elevate > elevate_max)
        {
            /*if(target_assigned)
            {
                console.addComMessage("Weapon cannot aim at target.");
                relieveTarget();
            }*/
            elevate = elevate_max;
        }
        else if(elevate < elevate_min)
        {
            /*if(target_assigned)
            {
                console.addComMessage("Weapon cannot aim at target.");
                relieveTarget();
            }*/
            elevate = elevate_min;
        }
        
        // Bounds checking for transverse (only if device is hull mounted).
        if(sight_attach == OBJ_ATTACH_HULL)
        {
            //char buffer[128];
            if(transverse < PI && transverse > trans_min)
            {
                /*if(target_assigned)
                {
                    sprintf(buffer, "Cannot aim to %f, cut off at %f", transverse, trans_min);
                    console.addComMessage(buffer);
                    relieveTarget();
                }*/
                
                if(parent->obj_type == OBJ_TYPE_ATG)
                {
                    (dynamic_cast<atg_object*>(parent))->hull_yaw = (dynamic_cast<atg_object*>(parent))->dir[2] + desired_trans;
                    if((dynamic_cast<atg_object*>(parent))->hull_yaw < 0.0)
                        (dynamic_cast<atg_object*>(parent))->hull_yaw += TWOPI;
                    else if((dynamic_cast<atg_object*>(parent))->hull_yaw >= TWOPI)
                        (dynamic_cast<atg_object*>(parent))->hull_yaw -= TWOPI;
                    (dynamic_cast<atg_object*>(parent))->hull_yaw_override = true;
                }
                
                transverse = trans_min;
            }
            else if(transverse > PI && transverse < trans_max + TWOPI)
            {
                /*if(target_assigned)
                {
                    sprintf(buffer, "Cannot aim to %f, cut off at %f", transverse, trans_max);
                    console.addComMessage(buffer);
                    relieveTarget();
                }*/
                if(parent->obj_type == OBJ_TYPE_ATG)
                {
                    (dynamic_cast<atg_object*>(parent))->hull_yaw = (dynamic_cast<atg_object*>(parent))->dir[2] + desired_trans;
                    if((dynamic_cast<atg_object*>(parent))->hull_yaw < 0.0)
                        (dynamic_cast<atg_object*>(parent))->hull_yaw += TWOPI;
                    else if((dynamic_cast<atg_object*>(parent))->hull_yaw >= TWOPI)
                        (dynamic_cast<atg_object*>(parent))->hull_yaw -= TWOPI;
                    (dynamic_cast<atg_object*>(parent))->hull_yaw_override = true;
                }
                
                transverse = trans_max + TWOPI;
            }
            
            if(trans_on_target)
            {
                if(parent->obj_type == OBJ_TYPE_ATG &&
                   (dynamic_cast<atg_object*>(parent))->hull_yaw_override == true)
                {
                    (dynamic_cast<atg_object*>(parent))->hull_yaw = (dynamic_cast<atg_object*>(parent))->dir[2] + desired_trans;
                    if((dynamic_cast<atg_object*>(parent))->hull_yaw < 0.0)
                        (dynamic_cast<atg_object*>(parent))->hull_yaw += TWOPI;
                    else if((dynamic_cast<atg_object*>(parent))->hull_yaw >= TWOPI)
                        (dynamic_cast<atg_object*>(parent))->hull_yaw -= TWOPI;
                }
            }
        }
        // done adjusting aim towards target
        
        // Kill off current job if we're done reseting aiming device
        if(!target_assigned && trans_on_target && elevate_on_target)
            crew->finishJob(job_id);
    }
    
    // Update sight device status based on tracking properties
    if(target_assigned)
    {
        // See if we have the desired trans/elevate (if so, on-target)
        if(trans_on_target && elevate_on_target)
            status = SIGHT_TRACKING;    // On-target
        // See if it is inside/outside the FOV of the device
        else if(fabsf(elevate - desired_elevate) > field_of_view / 2.0 ||
                fabsf(transverse - desired_trans) > field_of_view / 2.0)
            status = SIGHT_AQUIRING;    // Target not in-view
        else
            status = SIGHT_AIMING;      // Still aiming at target
    }
    else
        status = SIGHT_AWAITING;        // Awaiting for target
}

/*******************************************************************************
    Gunning Device
*******************************************************************************/

/*******************************************************************************
    function    :   gun_device::gun_device
    arguments   :   <none>
    purpose     :   Constructor.
    notes       :   <none>
*******************************************************************************/
gun_device::gun_device()
{
    int i;
    
    // Initialize all values
    parent = NULL;
    
    gun_num = -1;
    gun_type = NULL;
    gun_pivot[0] = gun_pivot[1] = gun_pivot[2] = 0.0;
    gun_attach = OBJ_ATTACH_HULL;
    
    gun_recoil = 0.0;
    gun_max_recoil = 0.0;
    gun_length = 0.0;
    
    sight = NULL;
    sight_id = -1;
    
    status = BREECH_EMPTY;
    tracking_time = 0.0;
    breech_timer = 0.0;
    
    crew = NULL;
    job_id = JOB_NULL;
    load_routine = 0;
    
    clip_size = 0;
    clip_left = 0;
    clip_burst = 0;
    clip_fire_time = 0.0;
    
    ammo_in_breech = 0;
    ammo_in_usage = 0;
    ammo_load_time = 0.0;
    
    for(i = 0; i < OBJ_MAX_AMMOPOOL; i++)
        is_fireable[i] = false;
    
    enabled = true;
    automatic_eject = true;
    out_of_ammo = false;
    flash_supressor = false;
}

/*******************************************************************************
    function    :   gun_device::initGun
    arguments   :   parentPtr - Pointer to parent object
                    gunNum - Gun ID number
    purpose     :   Initializes gunning mechanism based on passed ID number
                    and parent object pointer.
    notes       :   <none>
*******************************************************************************/
void gun_device::initGun(object* parentPtr, int gunNum)
{
    int i;
    int sight_num;
    char* temp;
    char element[64];
    char buffer[128];
    
    // Assign base data
    parent = parentPtr;
    gun_num = gunNum;
    
    // Check for valid gun number
    if(gunNum < 0 || gunNum >= OBJ_GUN_MAX)
    {
        sprintf(buffer, "Gun: Invalid gun number '%i' for \"%s\".",
            gun_num + 1, parent->obj_model);
        write_error(buffer);
        parent->obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // Grab gun type
    sprintf(element, "GUN%i_TYPE", gun_num + 1);
    gun_type = db.query(parent->obj_model, element);
    // Check for valid gun type
    if(!gun_type)
    {
        sprintf(buffer, "Gun: Gun %i type not defined for \"%s\".",
            gun_num + 1, parent->obj_model);
        write_error(buffer);
        parent->obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // Grab gun pivot
    sprintf(buffer, "GUN%i_PIVOT", gun_num + 1);
    temp = db.query(parent->obj_model, buffer);
    if(temp)
        sscanf(temp, "%f %f %f", &gun_pivot[0], &gun_pivot[1], &gun_pivot[2]);
    else
    {
        sprintf(buffer, "Gun: Gun %i pivot not defined for \"%s\".",
            gun_num + 1, parent->obj_model);
        write_error(buffer);
        parent->obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // Grab gun attachment
    sprintf(buffer, "GUN%i_ATTACH", gun_num + 1);
    temp = db.query(parent->obj_model, buffer);
    if(temp)
    {
        if(strstr(temp, "HULL"))
            gun_attach = OBJ_ATTACH_HULL;
        else if(strstr(temp, "TURRET"))
        {
            sscanf(temp, "TURRET%i", &gun_attach);
            gun_attach += OBJ_ATTACH_TURRET_OFF - 1;    // 1-1? ;)
            
            // Offset gun pivot based on attachment (only for tanks and vehicles)
            if(parent->obj_type == OBJ_TYPE_TANK)
                gun_pivot = gun_pivot - (dynamic_cast<tank_object*>(parent))->turret_pivot[gun_attach - OBJ_ATTACH_TURRET_OFF];
            /*else if(parent->obj_type == OBJ_TYPE_VEHICLE)
                gun_pivot = gun_pivot - (dynamic_cast<vehicle_object*>(parent))->turret_pivot[gun_attach - OBJ_ATTACH_TURRET_OFF];*/
        }
        else
        {
            sprintf(buffer, "Gun: Invalid gun %i attachment level \"%s\" for \"%s\".",
                gun_num + 1, temp, parent->obj_model);
            write_error(buffer);
            parent->obj_status = OBJ_STATUS_REMOVE;
            return;
        }
    }
    else
    {
        sprintf(buffer, "Gun: Gun %i attachment not defined for \"%s\".",
            gun_num + 1, parent->obj_model);
        write_error(buffer);
        parent->obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // Grab sight ID device
    sprintf(element, "GUN%i_SIGHT", gun_num + 1);
    temp = db.query(parent->obj_model, element);
    if(temp)
    {
        sscanf(temp, "SIGHT%i", &sight_num);
        sight_num--;
    }
    else
    {
        sprintf(buffer, "Gun: No sight device defined for gun %i on \"%s\".",
            gun_num + 1, parent->obj_model);
        write_error(buffer);
        parent->obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // Assign sight pointer and crew pointer based on parent object.
    switch(parent->obj_type)
    {
        case OBJ_TYPE_TANK:
        case OBJ_TYPE_VEHICLE:
        case OBJ_TYPE_ATG:
        case OBJ_TYPE_ATR:
            sight = &((dynamic_cast<firing_object*>(parent))->sight[sight_num]);
            crew = &((dynamic_cast<unit_object*>(parent))->crew);
            break;
        
        default:
            // No device present on object -> abort.
            sprintf(buffer, "Gun: Invalid gun device: \"%s\" is not a firing object.",
                parent->obj_model);
            parent->obj_status = OBJ_STATUS_REMOVE;
            return;
            break;
    }
    
    // Grab loading routine
    sprintf(element, "GUN%i_LOADED_BY", gun_num + 1);
    temp = db.query(parent->obj_model, element);
    if(temp)
    {
        sscanf(temp, "FEED_ROUTINE_%hi", &load_routine);
        load_routine--;
    }
    else
    {
        sprintf(buffer, "Gun: No loading routine defined for gun %i for \"%s\".",
            gun_num + 1, parent->obj_model);
        write_error(buffer);
        parent->obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // Assign loading time
    sprintf(element, "GUN%i_LOAD_TIME", gun_num + 1);
    temp = db.query(parent->obj_model, element);
    if(temp)
        ammo_load_time = atof(temp);
    else
    {
        sprintf(buffer, "Gun: Load time not defined for gun %i for \"%s\".",
            gun_num + 1, parent->obj_model);
        write_error(buffer);
        parent->obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // Assign ammo pool properties    
    for(i = OBJ_MAX_AMMOPOOL - 1; i >= 0; i--)
    {
        // For each ammo poolm determine what round is fired
        sprintf(element, "A%i_FIRED", i+1);
        temp = db.query(gun_type, element);
        
        if(temp)
        {
            // Round is present
            is_fireable[i] = true;
            
            ammo_in_usage = i;      // Will wind up being the first position
            
            // Link ammo pool to round type. If it is already linked, and it is
            // linked to a different ammo pool, then we have a grecious error
            // in the defining of the gun.
            if((dynamic_cast<firing_object*>(parent))->ammo_pool_type[i] == NULL)
                (dynamic_cast<firing_object*>(parent))->ammo_pool_type[i] = temp;   // Linked
            else if(strcmp((dynamic_cast<firing_object*>(parent))->ammo_pool_type[i], temp) != 0)
            {
                // Multi-linked pool -> abort
                sprintf(buffer, 
                    "Gun: Ammo pool %i is linked to more than one shell type for \"%s\".",
                    i + 1, parent->obj_model);
                write_error(buffer);
                parent->obj_status = OBJ_STATUS_REMOVE;
                return;
            }
        }
    }
    
    // Grab clip size for gun (if applicable)
    temp = db.query(gun_type, "CLIP_SIZE");
    if(temp)
    {
        // Assign clip-size dependent values
        clip_size = atoi(temp);
        
        // Assign firing rate
        temp = db.query(gun_type, "CLIP_FIRING_RPM");
        if(temp)
            clip_fire_time = 1.0 / (atof(temp) / 60.0);
        else
        {
            sprintf(buffer, "Gun: Firing RPM not defined for gun %i for \"%s\".",
                gun_num + 1, parent->obj_model);
            write_error(buffer);
            parent->obj_status = OBJ_STATUS_REMOVE;
            return;
        }
        
        // Assign standard burst - If not defined by the model, then grab it
        // from the gun.
        sprintf(element, "GUN%i_STD_BURST", gun_num + 1);
        temp = db.query(parent->obj_model, element);
        if(temp)
            clip_burst = atoi(temp);
        else
        {
            // Grab from gun
            temp = db.query(gun_type, "CLIP_STD_BURST");
            if(temp)
                clip_burst = atoi(temp);
            else
            {
                sprintf(buffer, "Gun: Firing burst count not defined for gun %i for \"%s\".",
                    gun_num + 1, parent->obj_model);
                write_error(buffer);
                parent->obj_status = OBJ_STATUS_REMOVE;
                return;
            }
        }
    }
    else
    {
        // Default clip size (singular round)
        clip_size = 1;
        clip_burst = 1;
    }
    
    // Grab barrel recoil for gun
    temp = db.query(gun_type, "BARREL_RECOIL");
    if(temp)
        gun_max_recoil = atof(temp);
    else
    {
        // Check for valid entry in DB
        sprintf(buffer, "Gun: Barrel recoil not defined for \"%s\".", gun_type);
        write_error(buffer);
        parent->obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // Grab barrel length for gun
    sprintf(element, "GUN%i_LENGTH", gun_num + 1);
    temp = db.query(parent->obj_model, element);
    if(temp)
        gun_length = atof(temp);
    else
    {
        // Check for valid gun length definition
        sprintf(buffer, "Gun: Barrel length for gun %i not defined for \"%s\".",
            gun_num + 1, parent->obj_model);
        write_error(buffer);
        parent->obj_status = OBJ_STATUS_REMOVE;
        return;
    }
    
    // Grab auto-eject for gun
    temp = db.query(gun_type, "AUTOMATIC_EJECT");
    if(temp && (strstr(temp, "T") || strstr(temp, "Y") || strstr(temp, "1")))
        automatic_eject = true;
    else if(temp && (strstr(temp, "F") || strstr(temp, "N") || strstr(temp, "0")))
        automatic_eject = false;
    
    // Grab flash supressor
    temp = db.query(gun_type, "FLASH_SUPRESSOR");
    if(temp && (strstr(temp, "T") || strstr(temp, "Y") || strstr(temp, "1")))
        flash_supressor = true;
    else if(temp && (strstr(temp, "F") || strstr(temp, "N") || strstr(temp, "0")))
        flash_supressor = false;
}

/*******************************************************************************
    function    :   gun_device::setAmmoUsage
    arguments   :   poolNum - Which ammo pool to use for further breech
                              activities (e.g. mount next).
    purpose     :   Sets the ammo usage for the gun by defining which ammo
                    pool to start taking rounds out of.
    notes       :   <none>
*******************************************************************************/
void gun_device::setAmmoUsage(int poolNum)
{
    // Make sure the pool is fireable and that the pool has ammo in it.
    if(is_fireable[poolNum] && (dynamic_cast<firing_object*>(parent))->ammo_pool[poolNum] > 0)
        ammo_in_usage = poolNum;
}

/*******************************************************************************
    function    :   gun_device::loadBreech
    arguments   :   <none>
    purpose     :   Loads the breech if in the empty status mode with the
                    current ammo set to be in usage.
    notes       :   <none>
*******************************************************************************/
void gun_device::loadBreech()
{
    int i;
    char buffer[128];
    
    // Only do loading breech operations if the breech is empty.
    if(status == BREECH_EMPTY)
    {
        // Check to see if we are out of ammo
        if((dynamic_cast<firing_object*>(parent))->ammo_pool[ammo_in_usage] == 0)
        {
            int previous_ammo;
            
            // Keep track of current ammo
            previous_ammo = ammo_in_usage;
            
            // If out of ammo, switch to next pool (from A5-A0). setAmmoUsage
            // handles all boundary checks to make sure we can fire the pool.
            for(i = OBJ_MAX_AMMOPOOL - 1; i >= 0; i--)
                setAmmoUsage(i);
            
            // If we haven't swaped ammo usage to a new pool, that means that
            // we are completely out of ammo.
            if(previous_ammo == ammo_in_usage)
            {
                // Set flag and kill any current job
                out_of_ammo = true;
                if(job_id != JOB_NULL)
                    crew->finishJob(job_id);
                breech_timer = 0.0;
                return;             // Kick-out
            }
            
            // Display switch to another ammo message
            sprintf(buffer, "[%s] reports: %s out of %s, switching to %s.",
                (dynamic_cast<firing_object*>(parent))->obj_id, gun_type,
                db.query((dynamic_cast<firing_object*>(parent))->ammo_pool_type[previous_ammo], "TYPE"),
                db.query((dynamic_cast<firing_object*>(parent))->ammo_pool_type[ammo_in_usage], "TYPE"));
            console.addComMessage(buffer);
        }
        
        // See if we can only grab half a portion of a clip or a full clip
        if(clip_size <= (dynamic_cast<firing_object*>(parent))->ammo_pool[ammo_in_usage])
        {
            // Grab full clip, decrement from pool
            (dynamic_cast<firing_object*>(parent))->ammo_pool[ammo_in_usage] -= clip_size;
            clip_left = clip_size;
        }
        else
        {
            // Grab partial clip, set pool to zero
            clip_left = (dynamic_cast<firing_object*>(parent))->ammo_pool[ammo_in_usage];
            (dynamic_cast<firing_object*>(parent))->ammo_pool[ammo_in_usage] = 0;
        }
        
        // Set what ammo in breech there is
        ammo_in_breech = ammo_in_usage;
        
        // Suspend current sight job (to allow room for loading to take over
        // being handled in situations where it is a one-man turret).
        if(sight->getJobID() != JOB_NULL)
            crew->suspendJob(sight->getJobID());
        
        // Set the job and status to loading a shell
        if(clip_size == 1)
            job_id = crew->enqueJob("Loading Round", ammo_load_time, load_routine);
        else
            job_id = crew->enqueJob("Loading Magazine", ammo_load_time, load_routine);
        
        // Resume sight job that was just suspended
        if(sight->getJobID() != JOB_NULL)
            crew->resumeJob(sight->getJobID());
        
        // Set status to breech being loaded
        status = BREECH_LOADING;
    }
}

/*******************************************************************************
    function    :   gun_device::unloadBreech
    arguments   :   <none>
    purpose     :   Unloads the current shell/magazine in the breech.
    notes       :   <none>
*******************************************************************************/
void gun_device::unloadBreech()
{
    // Make sure we are in a state where this can be done.
    if(status != BREECH_EMPTY && status != BREECH_EJECTING &&
       status != BREECH_CYCLING && status != BREECH_UNLOADING)
    {
        float load_time;
        
        // If we are in the midst of loading a shell and we're told to unload,
        // penalize with a 0.5 second penality to whatever amount of time we
        // have already spent loading the shell (since if we haven't fully
        // loaded shell, we do not need to fully unload shell).
        if(status == BREECH_LOADING)
        {
            load_time = crew->getJobTime(job_id);
            load_time = (isMainGun() ? ammo_load_time : ammo_load_time * 0.5f)
                - load_time + 0.5;   // Add in penalty
        }
        else
            load_time = (isMainGun() ? ammo_load_time : ammo_load_time * 0.5f);
        
        // Finish off current job
        if(job_id != JOB_NULL)
            crew->finishJob(job_id);
        
        // Start new job
        if(clip_size == 1)
            job_id = crew->enqueJob("Unloading Round", load_time, load_routine);
        else
            job_id = crew->enqueJob("Unloading Magazine", load_time, load_routine);
        
        // Set status to breech being unloaded
        status = BREECH_UNLOADING;
    }
}

/*******************************************************************************
    function    :   gun_device::update
    arguments   :   deltaT - Number of seconds elapsed since last update
    purpose     :   Updates the gun device by controlling what is happening
                    with the breech.
    notes       :   Until a crew is programmed in, shells are auto-loaded.
*******************************************************************************/
void gun_device::update(float deltaT)
{
    bool job_finished = false;
    
    // Account for gun recoil
    if(gun_recoil > 0.0)
    {
        gun_recoil -= 0.35 * deltaT;
        
        if(gun_recoil < 0.0)
            gun_recoil = 0.0;
    }
    
    // See if we have a job done
    if(job_id != JOB_NULL)
        job_finished = crew->isJobFinished(job_id);
    
    // See if we have to decrement the breech timer
    if(breech_timer > 0.0)
    {
        breech_timer -= deltaT;    
        if(breech_timer <= 0.0)
        {
            breech_timer = 0.0;
            job_finished = true;
        }
    }
    
    // See if we are waiting in the sight queue and no target is assigned. If
    // this happens, we must immediately unqueue ourselves.
    if((sight_id != -1) && (!sight->isTargetAssigned() || !enabled))
    {
        sight->dequeueDevice(sight_id);
        sight_id = -1;
    }
    
    // Do status specific operations
    switch(status)
    {
        case BREECH_EMPTY:          // Breech is empty -> load a shell
            if(!out_of_ammo)
                loadBreech();
            break;
        
        case BREECH_LOADED:         // Breech is ready and loaded!
            // Determine what to do based on if we're on the sight queue or not.
            if(sight_id == -1)
            {
                // Not on sight queue, see if gun is enabled & target assigned.
                if(enabled && sight->isTargetAssigned())
                {
                    char* temp;
                    
                    // Sight is assigned to a target and we're not on the queue,
                    // so lets get on it.
                    
                    // Get the velocity of our round we're firing.
                    temp = db.query(
                        (dynamic_cast<firing_object*>(parent))->ammo_pool_type[ammo_in_breech],
                        "VELOCITY");
                    if(temp)
                    {
                        // Put us on the queue.
                        sight_id = sight->enqueueDevice(gun_num,
                            atof(temp) * PROJ_VEL_MULTIPLIER);
                        tracking_time = 0.0;
                    }
                    else
                    {
                        char buffer[128];
                        sprintf(buffer, "Gun: Cannot query velocity for %s.",
                            (dynamic_cast<firing_object*>(parent))->ammo_pool_type[ammo_in_breech]);
                        write_error(buffer);
                    }
                }
            }
            else
            {
                // We're on the queue. Determine if we are at a point so that
                // we may fire our weapon.
                if(sight->isDeviceOkayed(sight_id))
                {
                    // Update tracking time
                    if(sight->getSightStatus() == SIGHT_TRACKING)
                       tracking_time += deltaT;         // Incremement time
                    else if(tracking_time > 0.0)
                    {
                        // Decrememnt time at 1 1/2 times faster if we're not
                        // tracking the target.
                        tracking_time -= 1.5 * deltaT;
                        
                        // Bounds check
                        if(tracking_time < 0.0)
                            tracking_time = 0.0;
                    }
                    
                    // See if we're ready to fire the round, e.g. have been
                    // tracking for at least a second.
                    if(tracking_time >= 1.0)
                    {
                        object* shell = NULL;
                        
                        // Fire gun (returns pointer to new projectile)
                        shell = (dynamic_cast<firing_object*>(parent))->
                            fireGun(gun_num, ammo_in_breech);
                        
                        // Special case for German MG guns, which had a
                        // white tracer every 10 rounds.
                        if(shell != NULL &&
                           ammo_in_breech == OBJ_AMMOPOOL_MG &&
                           (dynamic_cast<firing_object*>(parent))->
                                ammo_pool_type[OBJ_AMMOPOOL_MG][0] == 'D' &&
                           (dynamic_cast<firing_object*>(parent))->
                                ammo_pool_type[OBJ_AMMOPOOL_MG][1] == 'E' &&
                           clip_left != 0 && clip_left % 10 == 0)
                        {
                            // Add a white tracer modifier to proj.
                            ((proj_object*)shell)->addModifier(AMMO_MOD_WHITE_TRACER);
                        }
                        
                        // Decrement clip left over
                        clip_left--;
                        
                        // If clip hits zero or the burst setting, then the
                        // gun is done with it's turn in the sighting mechanism
                        // queue and needs to take itself off of the queue.
                        if(clip_left == 0 ||
                           (clip_size - clip_left) % clip_burst == 0)
                        {
                            // Take gun off of queue.
                            sight->dequeueDevice(sight_id);
                            sight_id = -1;
                        }
                        
                        // Determine what to do next based on properties.
                        if(clip_left == 0 && automatic_eject)
                        {
                            // Autoeject mechanism present.
                            breech_timer = 0.5;
                            status = BREECH_EJECTING;
                        }
                        else if(clip_left > 0)
                        {
                            // Clip still left, cycle to next round, waiting
                            // the firing time which will effectively emulate
                            // firing rate (in rounds per minute).
                            breech_timer = clip_fire_time;
                            status = BREECH_CYCLING;
                        }
                        else
                        {
                            // Breech is spent, give it half a second to snap
                            // back into place.
                            breech_timer = 0.5;
                            status = BREECH_SPENT;
                        }
                    }
                }
            }            
            break;
        
        case BREECH_SPENT:          // Shell is spent -> assume no auto-eject
            if(job_finished)
                unloadBreech();     // Unload shell
            break;
        
        case BREECH_EJECTING:       // Shell is being auto ejected
            if(job_finished)
                status = BREECH_EMPTY;
            break;
        
        case BREECH_CYCLING:        // Shell is being changed in mid-clip
            if(job_finished)
                status = BREECH_LOADED;
            break;
        
        case BREECH_LOADING:        // Shell is being loaded
            if(job_finished)
                status = BREECH_LOADED;
            break;
        
        case BREECH_UNLOADING:      // Shell is being unloaded
            if(job_finished)
            {
                // Replace shell back onto the ammo pool from which it
                // came from and set our clip to 0.
                if(clip_left != 0)
                {
                    (dynamic_cast<firing_object*>(parent))->
                        ammo_pool[ammo_in_breech] += clip_left;
                    clip_left = 0;
                }
                
                status = BREECH_EMPTY;
            }
            break;
    }
}
