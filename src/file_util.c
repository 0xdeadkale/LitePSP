/** @file file_util.c
 *
 * @brief File that holds the threading, dir walk, and 
 * printing functions used in the business logic of this
 * project.
 */

#include "header/file_util.h"

/**
 * @brief This function generates and joins threads for
 * the different aspects of the program (dumper, etc.)
 *
 * @param database Pointer to the hashtable.
 * 
 * @return The status of the any thread operations.
 */
int thread_dispatcher(database_i *database) 
{
    /* This could be a thread pool for less thread creation overhead/better efficiency, 
     * but it depends on the dataset. This is a simple implementation.
     */ 

    /* Set up variables */
    int status = 0;
    pthread_t thread_id_user, thread_id_dumper;
    pthread_t thread_id_workers[database->size];
    /********************/

    /* Create threads/detach for each substring to dir walk asynchronously */
    for (int i = 0; i < database->size; i++) {

        database->all_substrings[i]->assigned = i;
        status = pthread_create(&thread_id_workers[i], NULL, read_dir, (void *)database);
        if (status != 0)
        {
            perror("Could not create worker thread");
            exit_flag = true;
            goto END;
        }

        status = pthread_detach(thread_id_workers[i]);
        if (status != 0)
        {
            perror("Could not detach worker thread");
            exit_flag = true;
            status = pthread_join(thread_id_workers[i], NULL);
            if (status != 0)
            {
                perror("Could not join worker thread");
                goto END;
            }
        }

        pthread_mutex_lock(&shutdown_lock);
        jobs++;
        pthread_mutex_unlock(&shutdown_lock);
        
    }
    /****************************************************************/

    /* Create thread for the dumper and detach */
    status = pthread_create(&thread_id_dumper, NULL, dumper, (void *)database);
    if (status != 0)
    {
        perror("Could not create dumper thread");
        exit_flag = true;
        goto END;
    }

    status = pthread_detach(thread_id_dumper);
    if (status != 0)
    {
        perror("Could not detach dumper thread");
        exit_flag = true;
        status = pthread_join(thread_id_dumper, NULL);
        if (status != 0)
        {
            perror("Could not join dumper thread");
            goto END;
        }
    }
    pthread_mutex_lock(&shutdown_lock);
    jobs++;
    pthread_mutex_unlock(&shutdown_lock);
    /*******************************************/

    /* Create thread for user input */
    status = pthread_create(&thread_id_user, NULL, user_input, (void *)database);
    if (status != 0)
    {
        perror("Could not create user input thread");
        exit_flag = true;
        goto END;
    }
    pthread_mutex_lock(&shutdown_lock);
    jobs++;
    pthread_mutex_unlock(&shutdown_lock);

    status = pthread_join(thread_id_user, NULL);
    if (status != 0)
    {
        perror("Could not join user thread");
        exit_flag = true;
        goto END;
        
    }
    else {
        pthread_mutex_lock(&shutdown_lock);
        jobs--;
        pthread_mutex_unlock(&shutdown_lock);
    }  
    /********************************/

END:
    return status;
}

/**
 * @brief This function is the routine for detached worker threads. 
 * It directory walks down a root directory, and stores any
 * triggering file names into the hashtable.
 *
 * @param database Pointer to the hashtable.
 * 
 * @return A pointer to anything returned from the thread.
 */
void *read_dir(void *database)
{
    int index = -1;
    FTS *ftsp = NULL;

    char *paths[] = { ((database_i *)database)->root_dir, NULL };  // Used for fts_open

    /* Loop through substrings status's and assign each thread a job */
    while(true) {

        
        for (int i = 0; i < ((database_i *)database)->size; i++) {

            printf("Thread %ld status: %d\n", ((database_i *)database)->all_substrings[i]->assigned, ((database_i *)database)->all_substrings[i]->status);

            if (exit_flag == true)
                goto END;
           
            if(((database_i *)database)->all_substrings[i]->status == WORK_FREE){
    
                pthread_mutex_lock(&status_lock);
                ((database_i *)database)->all_substrings[i]->status = WORK_IN_PROGRESS;
                pthread_mutex_unlock(&status_lock);
                      
                index = ((database_i *)database)->all_substrings[i]->assigned;
                printf("Index %d assigned to thread: %ld\n", index, ((database_i *)database)->all_substrings[i]->assigned);
                printf("Thread %ld has substring: %s\n", ((database_i *)database)->all_substrings[i]->assigned, ((database_i *)database)->all_substrings[i]->substring);
                printf("Thread %ld status: %d\n", ((database_i *)database)->all_substrings[i]->assigned, ((database_i *)database)->all_substrings[i]->status);
                break;
            }
        }
        if (index != -1)
            break;
    }
    /*****************************************************************/

    /* Start of directory walk */
	ftsp = fts_open(paths, FTS_NOCHDIR, NULL);
	if(ftsp == NULL)
	{
		perror("fts_open");
		exit(EXIT_FAILURE);
	}

	while(true) { // call fts_read() enough times to parse through each directory/file

        if (exit_flag == true)
            break;

		FTSENT *ent = fts_read(ftsp); // get next entry (could be file or directory).
		if (ent == NULL)
		{
			if (errno == 0)
				break;
			else
			{
				perror("fts_read");
				goto END;
			}
		}
		
        /* A file is seen in the current directory */
		if(ent->fts_info & FTS_F) 
        {
            /* Main trigger for finding a file name that contains the subtring */
            char *trigger = strstr(ent->fts_name, ((database_i *)database)->all_substrings[index]->substring);

            /* If there is a file name hit */
            if(trigger) {
              
                file_i file_found = {};
                substring_i node = {};
            
                pthread_mutex_lock(&database_lock);  // Lock
                
                /* Prepare node to insert into hashtable, and chain to other nodes at hashtbale index */
                node.substring = strndup(((database_i *)database)->all_substrings[index]->substring, PATH_SIZE);

                file_found.file_dir = strndup(ent->fts_path, PATH_MAX);
                file_found.file_name = strndup(ent->fts_name, FILENAME_MAX);
                node.file_hits = &file_found;

                /* Speed of processing files is limited for stdout readability */
                if (((database_i *)database)->all_substrings[index]->file_count % 4 == 0) {
                    pthread_mutex_unlock(&database_lock);  // Unlock for dumper to process insertions
                    sleep(4);
                    pthread_mutex_lock(&database_lock);
                }

                /* Actual insertion of file into node's linked list */    
                insert_node(((database_i *)database), ((database_i *)database)->all_substrings[index]->key, &node);

                pthread_mutex_unlock(&database_lock);  // Unlock
                
                free(file_found.file_dir);
                free(file_found.file_name);
                free(node.substring);
            }             
        }
	}

    /* Flag as work completed */
    pthread_mutex_lock(&status_lock);
    ((database_i *)database)->all_substrings[index]->status = WORK_COMPLETE;
    pthread_mutex_unlock(&status_lock);

    if (exit_flag == true)
        goto END;
        
END:
    if (ftsp != NULL) {
        fts_close(ftsp);
    }

    /* When thread work is complete, decrement the sig_atomic for use in graceful shutdown */
    pthread_mutex_lock(&shutdown_lock);
    jobs--;
    pthread_mutex_unlock(&shutdown_lock);
	
	return 0;
}

/**
 * @brief This function poll's for user input and triggers
 * two actions: dump and exit.
 *
 * @param database Pointer to the hashtable.
 * 
 * @return A pointer to anything returned from the thread.
 */
void *user_input(void *database) 
{
    /* Set up variables */
    int status = 0;
    char input_buf[6] = {};  // Buffer for stdin input
    struct pollfd input[1] = {{fd: STDIN_FILENO, events: POLLIN}};  // Set up poll struct for stdin

    sigset_t thread_set;
    sigfillset(&thread_set);
    pthread_sigmask(SIG_BLOCK, &thread_set, NULL); // Block all signals.
    /********************/

    /* Set stdin to non-blocking */
    status = fcntl(STDIN_FILENO, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
    if (errno != 0)
    {
        perror("fcntl");
        status = -1;
        goto END;
    }
    /*****************************/

    /* Loop for input from user */
    while(true) {

        memset(input_buf, 0, 6);  // Clear input buffer every loop

        status = poll(input, 1, 3000);  // Poll stdin for immediate response (timeout is 3 sec)
        if (status == 0 || status == 1)
            ;
        else
            puts("Input error");
  
        /* If we see POLLIN for stdin, store input */
        if (input[1].revents & POLLIN)
            fgets(input_buf, 6, stdin);
            
        /* Two commands parsed, per the project's requirements */
        if (strncasecmp(input_buf, "Exit\n", 6) == 0) {  // Exit command

            puts("+++++++++++++++++++++++++++++++++++++");
            puts("[~] Getting ready to exit gracefully");
            puts("+++++++++++++++++++++++++++++++++++++\n");

            pthread_mutex_lock(&database_lock);
            exit_flag = true;
            pthread_mutex_unlock(&database_lock);
            
            break;

        }
        else if (strncasecmp(input_buf, "Dump\n", 6) == 0) {  // Dump command

            puts("+++++++++++++++++++++++++++++++++++++");
            puts("[+] Dumping container");
            puts("+++++++++++++++++++++++++++++++++++++\n");

            pthread_mutex_lock(&database_lock);
            dump_flag = true;
            pthread_cond_signal(&dumper_cond);
            pthread_mutex_unlock(&database_lock);
            
        }
        else if (strncmp(input_buf, "", 1) != 0)
            puts("Command invalid | Enter 'Exit' or 'Dump'");
        /***********************************************************/
        
        fflush(stdin);  // flush away stdin

        if (exit_flag == true)
            break;
    }

END:
    pthread_exit(NULL);
}

/**
 * @brief This function dumps the contents of the container
 * (hashtable), and clears the container for future files.
 *
 * @param database Pointer to the hashtable.
 * 
 * @return A pointer to anything returned from the thread.
 */
void *dumper(void *database) 
{
    int status = 0;
    size_t count = 0;

    while(exit_flag != true) {

        puts("Searching...");
        puts("-----------------------------------------------\n");

        /* Get system time for use in pthread_cond_timedwait */
        struct timespec wait_time = {0};

        status = clock_gettime(CLOCK_REALTIME, &wait_time);
        if (status == -1) {
            perror("clock_gettime");
            exit(EXIT_FAILURE);
        }
        
        wait_time.tv_sec += 3;  // Three second timeout
        /*****************************************************/

        /* Dump container in three seconds, or when cond_signal is sent ('Dump' cmd) */
        pthread_mutex_lock(&dump_lock);
        while(dump_flag == false) {
    
            status = pthread_cond_timedwait(&dumper_cond, &dump_lock, &wait_time);
            if (status == ETIMEDOUT)
                break;
    
        }
        pthread_mutex_unlock(&dump_lock);
        /*****************************************************************************/

        /* Actual dumping of the container */
        pthread_mutex_lock(&database_lock);     
        for (size_t i = 0; i < ((database_i *)database)->size; ++i) {

            count = 0;
            
            file_i *tmp = NULL;

            /* If there are file hits in the substring nodes, print them */
            if (((database_i *)database)->all_substrings[i]->file_hits) {

                printf("substring: %s\n", ((database_i *)database)->all_substrings[i]->substring);
                puts("********************");
                printf("File: %s\n", ((database_i *)database)->all_substrings[i]->file_hits->file_dir);

                /* If there are more than one files in chain, transverse and print */
                if (((database_i *)database)->all_substrings[i]->file_hits->next_hit) {
                    tmp = ((database_i *)database)->all_substrings[i]->file_hits->next_hit;
                }

                while(tmp && exit_flag != true) {

                    printf("File: %s\n", tmp->file_dir);

                    count++;
                    tmp = tmp->next_hit;

                }
                puts("********************\n");
                /*******************************************************************/
            }
            /* Else if there a no file hits, print empty substrings */
            else if (((database_i *)database)->all_substrings[i]->file_count == 0) 
            {
                printf("substring: %s\n", ((database_i *)database)->all_substrings[i]->substring);
                puts("********************");
                puts("********************\n");
            }
        }

        cleanup(database, false);  // Clear container (per project requirements)

        puts("\n-----------------------------------------------");
        puts("Enter Dump/Exit at any time");

        /* Reset dump flag */
        if (dump_flag == true) {
            pthread_mutex_lock(&dump_lock);
            dump_flag = false;
            pthread_mutex_unlock(&dump_lock);
        }
        pthread_mutex_unlock(&database_lock);

        if (exit_flag == true)  // Break out of main loop for clean exit
            break; 
    }

    /* When thread work is complete, decrement the sig_atomic for use in graceful shutdown */
    pthread_mutex_lock(&shutdown_lock);
    jobs--;
    pthread_mutex_unlock(&shutdown_lock);

    return 0;
}

/*** end of file ***/