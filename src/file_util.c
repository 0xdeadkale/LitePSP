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

    /* Create threads for the dumper and user input*/
    status = pthread_create(&thread_id_dumper, NULL, print_all, (void *)database);
    status = pthread_create(&thread_id_user, NULL, user_input, (void *)database);
    if (status != 0)
    {
        perror("Could not create thread");
        goto END;
    }
    
    /* Create threads for each substring to dir walk asynchronously */
    for (int i = 0; i < database->size; i++) {
        pthread_t thread_id_workers;

        status = pthread_create(&thread_id_workers, NULL, read_dir, (void *)database);
        if (status != 0)
        {
            perror("Could not create thread");
            goto END;
        }

        status = pthread_join(thread_id_workers, NULL);
        if (status != 0)
        {
            perror("Could not join thread");
            goto END;
        }
    }

    // exit_flag = true;

    status = pthread_join(thread_id_dumper, NULL);
    status = pthread_join(thread_id_user, NULL);
    if (status != 0)
        {
            perror("Could not join thread");
            goto END;
        }

END:
    return status;
}


void *user_input(void *varg_p) 
{

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

    while(true) {

        memset(input_buf, 0, 6);

        status = poll(input, 1, 5000);  // Poll for stdin for immediate response.
        
        if (input[1].revents & POLLIN)
            fgets(input_buf, 6, stdin);

        /* Commands parsed per the project's requirements */
        if (strncmp(input_buf, "Exit\n", 6) == 0) {
            pthread_mutex_lock(&database_lock);
            exit_flag = true;
            pthread_mutex_unlock(&database_lock);
            printf("Command entered: %s\n", input_buf);
            break;
        }
        else if (strncmp(input_buf, "Dump\n", 6) == 0) {
            pthread_mutex_lock(&database_lock);
            dump_flag = true;
            pthread_mutex_unlock(&database_lock);
            printf("Command entered: %s\n", input_buf);
        }
        else if (strncmp(input_buf, "", 1) != 0)
            puts("Command invalid | Enter 'Exit' or 'Dump'");
        
        fflush(NULL);

        if (exit_flag == true)
            break;
    }

END:
    printf("Status: %d\n", status);

    return 0;

}

void *read_dir(void *varg_p)
{
    int index = 0;
    FTS *ftsp = NULL;

    for (int i = 0; i < ((database_i *)varg_p)->size; i++) {

        if (exit_flag == true)
            goto END;
        if (dump_flag == true) {
            pthread_mutex_lock(&database_lock);
            dump_flag = false;
            pthread_mutex_unlock(&database_lock);

            print_all(((database_i *)varg_p));
        }

        if(((database_i *)varg_p)->all_substrings[i]->status == WORK_FREE){

            pthread_mutex_lock(&database_lock);
            ((database_i *)varg_p)->all_substrings[i]->status = WORK_IN_PROGRESS;
            pthread_mutex_unlock(&database_lock);
            
            index = i;
            break;
        } 
    }

    char *paths[] = { ((database_i *)varg_p)->root_dir, NULL };
	
	ftsp = fts_open(paths, FTS_PHYSICAL, NULL);
	if(ftsp == NULL)
	{
		perror("fts_open");
		exit(EXIT_FAILURE);
	}

	while(true) // call fts_read() enough times to get each file
	{
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
            char *trigger = strstr(ent->fts_name, ((database_i *)varg_p)->all_substrings[index]->substring);

            /* If there is a file name hit */
            if(trigger) {
              
                file_i file_found = {};
                substring_i node = {};
            
                pthread_mutex_lock(&database_lock);  // Lock
                
                /* Prepare node to insert into hashtable, and chain to other nodes at hashtbale index */
                node.substring = strndup(((database_i *)varg_p)->all_substrings[index]->substring, PATH_SIZE);

                file_found.file_dir = strndup(ent->fts_path, PATH_MAX);
                file_found.file_name = strndup(ent->fts_name, FILENAME_MAX);
                node.file_hits = &file_found;

                /* Speed of processing files is limited for stdout readability */
                if (((database_i *)varg_p)->all_substrings[index]->count == 10) {
                    sleep(7);
                }
                    
                insert_node(((database_i *)varg_p), ((database_i *)varg_p)->all_substrings[index]->key, &node);

                pthread_mutex_unlock(&database_lock);  // Unlock
                
                free(file_found.file_dir);
                free(file_found.file_name);
                free(node.substring);
            }             
        }
	}

    // TODO ENUM status

    pthread_mutex_lock(&database_lock);
    ((database_i *)varg_p)->all_substrings[index]->status = WORK_COMPLETE;
    pthread_mutex_unlock(&database_lock);

    if (exit_flag == true)
        goto END;
        
END:

    if (ftsp != NULL) {
        fts_close(ftsp);
    }
	
	return 0;
}

void *print_all(void *database) 
{
    bool dump_contents = false;
    size_t count = 0;

    while(exit_flag != true) {

        puts("Searching...\n");

        pthread_mutex_lock(&database_lock);

        for (size_t i = 0; i < ((database_i *)database)->size; ++i) {

            count = 0;

            /* Logic that decides when to print to stdout */
            if (dump_flag == true) {
                dump_contents = true;
                
                dump_flag = false;
                
                break;  // Reset loop
            }            
            else if (dump_contents == false && ((database_i *)database)->all_substrings[i]->count % 2 != 0)
                continue;
            

            file_i *tmp = NULL;

            if (((database_i *)database)->all_substrings[i]->file_hits) {

                printf("substring: %s\n", ((database_i *)database)->all_substrings[i]->substring);
                puts("********************");
                printf("File: %s\n", ((database_i *)database)->all_substrings[i]->file_hits->file_dir);

                if (((database_i *)database)->all_substrings[i]->file_hits->next_hit) {
                    tmp = ((database_i *)database)->all_substrings[i]->file_hits->next_hit;
                }

                while(tmp && exit_flag != true) {
                    printf("File: %s\n", tmp->file_dir);
                    count++;

                    if (dump_flag != true && count % 10 == 0) {
                        sleep(1);
                        break;
                    }
                        
                    else if(count >= 100) {
                        puts("Too many file hits to print. Skipping");
                        break;
                    }
                    tmp = tmp->next_hit;
                }
                puts("********************\n");
            }
            else if (dump_contents == true && !((database_i *)database)->all_substrings[i]->file_hits) {
                printf("substring: %s\n", ((database_i *)database)->all_substrings[i]->substring);
                puts("********************");
                puts("********************\n");
                if (i == ((database_i *)database)->size - 1)
                    dump_contents = false;  // Reset flag
            }

            if (dump_contents == true && i == ((database_i *)database)->size - 1) 
                dump_contents = false;  // Reset flag
        }

        pthread_mutex_unlock(&database_lock);

        if (dump_contents != true) {
            pthread_mutex_lock(&database_lock);
            cleanup(database, false);
            pthread_mutex_unlock(&database_lock);
        }

        if (exit_flag == true)
            break;
        puts("Enter Command: ");
        if (dump_contents != true)
            sleep(3);
    }

    return 0;
}

/*** end of file ***/