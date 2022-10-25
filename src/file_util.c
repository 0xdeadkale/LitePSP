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

    pthread_mutex_lock(&database_lock);
    puts("Exit lock");
    exit_flag = true;
    pthread_mutex_unlock(&database_lock);

    pthread_join(thread_id_dumper, NULL);
    pthread_join(thread_id_user, NULL);

    return 0;
}

void *user_input(void *varg_p) {

    int status = 0;
    char input_buf[6] = {};
    struct pollfd input[1] = {{fd: STDIN_FILENO, events: POLLIN}};


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

        status = poll(input, 1, -1);  // Poll for stdin for immediate response.
        
        fgets(input_buf, 6, stdin);

        printf("Command entered: %s\n", input_buf);

        /* Commands parsed per the project's requirements */
        if (strncmp(input_buf, "Exit\n", 6) == 0) {
            pthread_mutex_lock(&database_lock);
            puts("Exit lock");
            exit_flag = true;
            pthread_mutex_unlock(&database_lock);
            break;
        }
        else if (strncmp(input_buf, "Dump\n", 6) == 0) {
            pthread_mutex_lock(&database_lock);
            puts("Dump lock");
            dump_flag = true;
            pthread_mutex_unlock(&database_lock);
        }
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
            puts("Lock1");
            dump_flag = false;
            pthread_mutex_unlock(&database_lock);

            print_all(((database_i *)varg_p));
        }

        if(((database_i *)varg_p)->all_substrings[i]->status == 0){

            pthread_mutex_lock(&database_lock);
            puts("Status lock");
            ((database_i *)varg_p)->all_substrings[i]->status = 1;
            pthread_mutex_unlock(&database_lock);
             puts("Status unlock");
            
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
                puts("Insert lock");
                
                /* Prepare node to insert into hashtable, and chain to other nodes at hashtbale index */
                node.substring = strndup(((database_i *)varg_p)->all_substrings[index]->substring, 255);

                file_found.file_dir = strndup(ent->fts_path, 255);
                file_found.file_name = strndup(ent->fts_name, 255);
                node.file_hits = &file_found;

                insert_node(((database_i *)varg_p), ((database_i *)varg_p)->all_substrings[index]->key, &node);

                pthread_mutex_unlock(&database_lock);
                puts("Insert unlock");
                
                free(file_found.file_dir);
                free(file_found.file_name);
                free(node.substring);
            }             
        }
	}

    // TODO ENUM status

    pthread_mutex_lock(&database_lock);
    puts("status done lock");
    ((database_i *)varg_p)->all_substrings[index]->status = 2;
    pthread_mutex_unlock(&database_lock);
    puts("status done unlock");

    if (exit_flag == true)
        goto END;
        
END:

    if (ftsp != NULL) {
        fts_close(ftsp);
    }
	
	return 0;
}

int print_substring(substring_i *substring) 
{
    file_i *tmp = NULL;

    if (substring) {

        printf("substring: %s\n", substring->substring);
        puts("Start********************");
        printf("file name: %s\n", substring->file_hits->file_name);
        printf("file dir: %s\n", substring->file_hits->file_dir);
        puts("--------------------");

        if (substring->file_hits->next_hit) {
            tmp = substring->file_hits->next_hit;
        }

        while(tmp) {
            
            printf("file name: %s\n", tmp->file_name);
            printf("file dir: %s\n", tmp->file_dir);
            puts("--------------------");

            tmp = tmp->next_hit;
        }
    }
    puts("End********************");
    puts("\n");
    
    return 0;
}

void *print_all(void *database) 
{
    sleep(3);
    puts("Made it here");
    while(exit_flag != true) {
        printf("Database size:%ld\n", ((database_i *)database)->size);

        for (size_t i = 0; i < ((database_i *)database)->size; ++i) {

            file_i *tmp = NULL;

            if (((database_i *)database)->all_substrings[i]->file_hits) {

                printf("substring: %s\n", ((database_i *)database)->all_substrings[i]->substring);
                puts("********************");
                printf("file: %s\n", ((database_i *)database)->all_substrings[i]->file_hits->file_dir);
                puts("--------------------");

                if (((database_i *)database)->all_substrings[i]->file_hits->next_hit) {
                    tmp = ((database_i *)database)->all_substrings[i]->file_hits->next_hit;
                }

                while(tmp) {
                    printf("file: %s\n", tmp->file_dir);
                    puts("--------------------");

                    tmp = tmp->next_hit;
                }
            }
            puts("********************\n");
        }

        pthread_mutex_lock(&database_lock);
        puts("cleanup lock");
        cleanup(database, false);
        pthread_mutex_unlock(&database_lock);
        puts("cleanup unlock");

        sleep(5);

    }

    return 0;
}

/*** end of file ***/