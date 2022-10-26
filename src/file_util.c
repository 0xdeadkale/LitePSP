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
    /* This could be a thread pool for less thread creation overhead/better efficiency */ 

    int status = 0;
    pthread_t thread_id_user, thread_id_dumper;
    pthread_t thread_id_workers[database->size];

    /********************************/

    /* Create threads for each substring to dir walk asynchronously */
    for (int i = 0; i < database->size; i++) {
        
        database->all_substrings[i]->assigned = i;
        status = pthread_create(&thread_id_workers[i], NULL, read_dir, (void *)database);
        if (status != 0)
        {
            // database->all_substrings[i]->assigned = i;
            perror("Could not create worker thread");
            exit_flag = true;
            goto END;
        }
        printf("thread created: %d\n", i);

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
        jobs++;
        printf("thread detached: %d\n", i);
    }
    /****************************************************************/

    /* Create thread for the dumper */
    status = pthread_create(&thread_id_dumper, NULL, dumper, (void *)database);
    if (status != 0)
    {
        perror("Could not create dumper thread");
        exit_flag = true;
        goto END;
    }

    /* status = pthread_join(thread_id_dumper, NULL);
    if (status != 0)
    { 
        perror("Could not join dumper thread");
        goto END;
    } */

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
    jobs++;

    /* Create thread for user input */
    status = pthread_create(&thread_id_user, NULL, user_input, (void *)database);
    if (status != 0)
    {
        perror("Could not create user input thread");
        exit_flag = true;
        goto END;
    }
    jobs++;

    status = pthread_join(thread_id_user, NULL);
    if (status != 0)
    {
        perror("Could not join user thread");
        exit_flag = true;
        goto END;
        
    }
    else
        jobs--;

   /*  status = pthread_detach(thread_id_user);
    if (status != 0)
    {
        perror("Could not detach worker thread");
        status = pthread_join(thread_id_user, NULL);
        if (status != 0)
        {
            perror("Could not join worker thread");
            goto END;
        }
    } */
    /********************************/

    
    

    printf("database size: %ld\n", database->size);
    
   

    /* for (int i = 0; i < database->size; i++) {
        status = pthread_join(thread_id_workers[i], NULL);
        if (status != 0)
        {
            perror("Could not join thread");
            goto END;
        }
        printf("thread joined: %d\n", i);
    } */
    // exit_flag = true;

    /* status = pthread_join(thread_id_dumper, NULL);
    status = pthread_join(thread_id_user, NULL);
    if (status != 0)
        {
            perror("Could not join dumper and user thread");
            goto END;
        } */

END:
    return status;
}


void *user_input(void *database) 
{
    puts("*****\nIn input");
    /* Set up variables */
    int status = 0;
    char input_buf[6] = {};
    struct pollfd input[1] = {{fd: STDIN_FILENO, events: POLLIN}};  // Set up poll for stdin

    sigset_t thread_set;
    sigfillset(&thread_set);
    pthread_sigmask(SIG_BLOCK, &thread_set, NULL); // Block all signals.
    /********************/

    status = fcntl(STDIN_FILENO, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);  // Set stdin to non-blocking
    {
        if (errno != 0)
        {
            perror("fcntl");
            status = -1;
            goto END;
        }
    }

    /* Loop for input from user */
    while(true) {

        

        memset(input_buf, 0, 6);  // Clear input buffer

        puts("polling for input");
        status = poll(input, 1, 5000);  // Poll for stdin for immediate response
        
        
        /* If we see POLLIN for stdin, store input */
        if (input[1].revents & POLLIN) {
            puts("Poll triggered");
            fgets(input_buf, 6, stdin);
        }
            

        /* Two commands parsed for per the project's requirements */
        if (strncmp(input_buf, "Exit\n", 6) == 0) {  // Exit command

            pthread_mutex_lock(&database_lock);
            exit_flag = true;
            pthread_mutex_unlock(&database_lock);
            printf("Command entered: %s\n", input_buf);
            break;

        }
        else if (strncmp(input_buf, "Dump\n", 6) == 0) {  // Dump command

            printf("Command entered: %s\n", input_buf);
            
           
            pthread_mutex_lock(&database_lock);
            dump_flag = true;
            pthread_cond_signal(&dumper_cond);
            pthread_mutex_unlock(&database_lock);
            
        }
        else if (strncmp(input_buf, "", 1) != 0)
            puts("Command invalid | Enter 'Exit' or 'Dump'");
        
        fflush(stdin);  // flush away stdin

        if (exit_flag == true)
            break;
    }

END:
    if (status == 0 || status == 1)
        ;
    else
        puts("Input error");

    puts("*****\nExiting input");
    return 0;

}

void *read_dir(void *database)
{

    puts("*****\nMade it to read dir");

    int index = -1;
    FTS *ftsp = NULL;

    
    while(true) {

        for (int i = 0; i < ((database_i *)database)->size; i++) {

            if (exit_flag == true)
                goto END;
            /* if (dump_flag == true) {
                pthread_mutex_lock(&dump_lock);
                printf("Thread %ld dump lock\n",  ((database_i *)database)->all_substrings[index]->assigned);
                dump_flag = false;
                pthread_mutex_unlock(&dump_lock);
                printf("Thread %ld dump unlock\n",  ((database_i *)database)->all_substrings[index]->assigned);

                dumper(((database_i *)database));
            } */

           
            puts("Thread status lock begin\n");
            printf("index: %d\n", i);
            if(((database_i *)database)->all_substrings[i]->status == WORK_FREE){

                
                pthread_mutex_lock(&status_lock);
                ((database_i *)database)->all_substrings[i]->status = WORK_IN_PROGRESS;
                pthread_mutex_unlock(&status_lock);
                puts("Thread status unlock begin\n");
                
                
                index = ((database_i *)database)->all_substrings[i]->assigned;
                puts("break");
                
                
                break;
            
            }

            
        
        }

        if (index != -1)
            break;
    }
    

    printf("Thread assigned: %ld\n",  ((database_i *)database)->all_substrings[index]->assigned);
    printf("substring assigned: %s\n",  ((database_i *)database)->all_substrings[index]->substring);

    char *paths[] = { ((database_i *)database)->root_dir, NULL };
	
	ftsp = fts_open(paths, FTS_NOCHDIR, NULL);
	if(ftsp == NULL)
	{
		perror("fts_open");
		exit(EXIT_FAILURE);
	}

   printf("Thread %ld before while\n",  ((database_i *)database)->all_substrings[index]->assigned);
	while(true) // call fts_read() enough times to get each file
	{
        if (exit_flag == true)
            break;

        //printf("Thread %ld before fts_read\n",  ((database_i *)database)->all_substrings[index]->assigned);
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
            printf("Thread %ld see's file\n",  ((database_i *)database)->all_substrings[index]->assigned);
            char *trigger = strstr(ent->fts_name, ((database_i *)database)->all_substrings[index]->substring);

            /* If there is a file name hit */
            if(trigger) {
              
                file_i file_found = {};
                substring_i node = {};
            
                pthread_mutex_lock(&database_lock);  // Lock
                printf("Thread %ld insert lock\n",  ((database_i *)database)->all_substrings[index]->assigned);
                
                /* Prepare node to insert into hashtable, and chain to other nodes at hashtbale index */
                node.substring = strndup(((database_i *)database)->all_substrings[index]->substring, PATH_SIZE);

                file_found.file_dir = strndup(ent->fts_path, PATH_MAX);
                file_found.file_name = strndup(ent->fts_name, FILENAME_MAX);
                node.file_hits = &file_found;

                /* Speed of processing files is limited for stdout readability */
                if (((database_i *)database)->all_substrings[index]->file_count % 4 == 0) {
                    puts("Thread sleeping");
                    pthread_mutex_unlock(&database_lock);  // Unlock for dumper to process insertions
                    sleep(4);
                    pthread_mutex_lock(&database_lock);
                }
                    
                insert_node(((database_i *)database), ((database_i *)database)->all_substrings[index]->key, &node);
                puts("Node inserted");

                pthread_mutex_unlock(&database_lock);  // Unlock
                printf("Thread %ld insert unlock\n",  ((database_i *)database)->all_substrings[index]->assigned);
                
                free(file_found.file_dir);
                free(file_found.file_name);
                free(node.substring);
            }             
        }
	}

    pthread_mutex_lock(&status_lock);
    printf("Thread %ld status work lock end\n",  ((database_i *)database)->all_substrings[index]->assigned);
    ((database_i *)database)->all_substrings[index]->status = WORK_COMPLETE;
    pthread_mutex_unlock(&status_lock);
    printf("Thread %ld status work unlock end\n",  ((database_i *)database)->all_substrings[index]->assigned);

    if (exit_flag == true)
        goto END;
        
END:

    if (ftsp != NULL) {
        fts_close(ftsp);
    }

    pthread_mutex_lock(&shutdown_lock);
    jobs--;
    pthread_mutex_unlock(&shutdown_lock);
	
    puts("*****\nExiting dir walk");
	return 0;
}

void *dumper(void *database) 
{

    puts("*****\nMade it in dumper");

    int status = 0;

    // bool dump_contents = false;
    size_t count = 0;

    while(exit_flag != true) {

        // puts("Searching...\n");

        struct timespec wait_time = {0};

        status = clock_gettime(CLOCK_REALTIME, &wait_time);
        if (status == -1) {
            perror("clock_gettime");
            exit(EXIT_FAILURE);
        }
        
        wait_time.tv_sec += 3;

        
        

        puts("++++++++Dumper 3 sec or signal++++++++++");
        pthread_mutex_lock(&dump_lock);

        

        while(dump_flag == false) {
            puts("In while loop cond");
            status = pthread_cond_timedwait(&dumper_cond, &dump_lock, &wait_time);
            if (status == ETIMEDOUT) {
                puts("cond timeout");
                break;
            }
            else
                puts("Signal");
        }
        pthread_mutex_unlock(&dump_lock);
        puts("Unlocking dump lock");


        pthread_mutex_lock(&database_lock);
        puts("++++++++Locking for dumper++++++++++");

        /* Either dumps to stdout every 3 seconds, or immediately on user cmd*/

       

        

        /* pthread_cond_wait(&dumper_cond, &dump_lock);
        if (dump_flag != true)
            sleep(3);
        else if (dump_flag == true) {
            puts("Dump Triggered");
            dump_flag = false;
        } */
            

        for (size_t i = 0; i < ((database_i *)database)->size; ++i) {

            count = 0;

            /* Logic that decides when to print to stdout */
            /* if (dump_flag == true) {
                dump_contents = true;
                
                dump_flag = false;
                
                break;  // Reset loop
            } */            
            // else if (dump_contents == false && ((database_i *)database)->all_substrings[i]->count % 2 != 0)
                // continue;
            

            file_i *tmp = NULL;

            if (((database_i *)database)->all_substrings[i]->file_hits) {

                printf("substring: %s\n", ((database_i *)database)->all_substrings[i]->substring);
                puts("********************");
                printf("File: %s\n", ((database_i *)database)->all_substrings[i]->file_hits->file_dir);

                if (((database_i *)database)->all_substrings[i]->file_hits->next_hit) {
                    tmp = ((database_i *)database)->all_substrings[i]->file_hits->next_hit;
                }

                while(tmp && exit_flag != true) {

                    count++;
                    printf("File: %s\n", tmp->file_dir);

                    /* if (dump_flag != true && count % 10 == 0) {
                        // sleep(1);
                        continue;
                    } */
                        
                    /* if(count >= 100) {
                        puts("Too many file hits to print. Skipping");
                        break;
                    } */
                    tmp = tmp->next_hit;
                }
                puts("********************\n");
            }
            /* else if (dump_contents == true && ((database_i *)database)->all_substrings[i]->file_count != 0) {
                puts("+++++++DUMPER TRIGGERED+++++++");
                printf("substring: %s\n", ((database_i *)database)->all_substrings[i]->substring);
                puts("********************");
                printf("File: %s\n", ((database_i *)database)->all_substrings[i]->file_hits->file_dir);

                if (((database_i *)database)->all_substrings[i]->file_hits->next_hit) {
                    tmp = ((database_i *)database)->all_substrings[i]->file_hits->next_hit;
                }

                while(tmp && exit_flag != true) {

                    count++;
                    printf("File: %s\n", tmp->file_dir); */

                    /* if (dump_flag != true && count % 10 == 0) {
                        // sleep(1);
                        continue;
                    } */
                        
                    /* if(count >= 100) {
                        puts("Too many file hits to print. Skipping");
                        break;
                    } */
                    // tmp = tmp->next_hit;
                // }
                // puts("********************\n");
                // puts("++++++++++++++++++++\n");

                // if (i == ((database_i *)database)->size - 1)
                  //   dump_contents = false;  // Reset flag

            // } 
            else if (((database_i *)database)->all_substrings[i]->file_count == 0) 
            {
                // puts("+++++++Dumper NOT triggered+++++++");
                printf("substring: %s\n", ((database_i *)database)->all_substrings[i]->substring);
                puts("********************");
                puts("********************\n");

                /* if (i == ((database_i *)database)->size - 1)
                    dump_contents = false;  // Reset flag */

                
                
                // puts("++++++++++++++++++++\n");
            }
            

            /* if (dump_contents == true && i == ((database_i *)database)->size - 1) 
                dump_contents = false;  // Reset flag */
        }

        // puts("--------------------");

        
        

        // if (dump_contents != true) {
            // pthread_mutex_lock(&database_lock);
        cleanup(database, false);
            // pthread_mutex_unlock(&database_lock);
        // }

        if (dump_flag == true) {
            pthread_mutex_lock(&dump_lock);
            dump_flag = false;
            pthread_mutex_unlock(&dump_lock);
        }
        
            
        pthread_mutex_unlock(&database_lock);
        puts("--------UnLocking for dumper--------");

        if (exit_flag == true)
            break;
        // puts("Enter Command: ");
        /* if (dump_contents != true) {
            puts("Sleeping dumper");
            sleep(3);
        } */
            
    }
    pthread_mutex_lock(&shutdown_lock);
    jobs--;
    pthread_mutex_unlock(&shutdown_lock);

    puts("*****\nExiting dumper");
    return 0;
}

/*** end of file ***/