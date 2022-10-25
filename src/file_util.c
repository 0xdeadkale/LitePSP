/** @file file_util.c
 *
 * @brief Provides the ability to interface with files.
 */

#include "header/file_util.h"

int thread_dispatcher(database_i *database) {

    /* This could be a thread pool for less thread creation overhead/better efficiency */ 

    pthread_t thread_id_user, thread_id_dumper;

    pthread_create(&thread_id_dumper, NULL, print_all, (void *)database);
    pthread_create(&thread_id_user, NULL, user_input, (void *)database);
    
    for (int i = 0; i < database->size; i++) {

        pthread_t thread_id_workers;
        pthread_create(&thread_id_workers, NULL, read_dir, (void *)database);
        pthread_join(thread_id_workers, NULL);
    }

    pthread_join(thread_id_dumper, NULL);
    pthread_join(thread_id_user, NULL);

    return 0;
}

void *user_input(void *varg_p) {

    int status = 0;
    char input_buf[6] = {};
    struct pollfd input[1] = {{fd: STDIN_FILENO, events: POLLIN}};

    sigset_t thread_set;
    sigfillset(&thread_set);
    pthread_sigmask(SIG_BLOCK, &thread_set, NULL); // Block all signals.



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

        if(((database_i *)varg_p)->all_substrings[i]->status == 0){

            pthread_mutex_lock(&database_lock);
            ((database_i *)varg_p)->all_substrings[i]->status = 1;
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
            
                pthread_mutex_lock(&database_lock);
                
                /* Prepare node to insert into hashtable, and chain to other nodes at hashtbale index */
                node.substring = strndup(((database_i *)varg_p)->all_substrings[index]->substring, 255);

                file_found.file_dir = strndup(ent->fts_path, 255);
                file_found.file_name = strndup(ent->fts_name, 255);
                node.file_hits = &file_found;

                insert_node(((database_i *)varg_p), ((database_i *)varg_p)->all_substrings[index]->key, &node);

                pthread_mutex_unlock(&database_lock);
                
                free(file_found.file_dir);
                free(file_found.file_name);
                free(node.substring);
            }             
        }
	}

    // TODO ENUM status

    pthread_mutex_lock(&database_lock);
    ((database_i *)varg_p)->all_substrings[index]->status = 2;
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

    while(exit_flag != true) {

        puts("Searching...\n");

        for (size_t i = 0; i < ((database_i *)database)->size; ++i) {

            /* Logic that decides when to print to stdout*/
            if (dump_flag == true) {
                puts("Dump triggered");
                dump_contents = true;
                pthread_mutex_lock(&database_lock);
                dump_flag = false;
                pthread_mutex_unlock(&database_lock);
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

                while(tmp) {
                    printf("File: %s\n", tmp->file_dir);
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