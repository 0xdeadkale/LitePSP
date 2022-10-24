/** @file file_util.c
 *
 * @brief Provides the ability to interface with files.
 */

#include "header/file_util.h"

int thread_dispatcher(database_i *database) {

    for (int i = 0; i < database->size; i++) {

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, read_dir, (void *)database);
        pthread_join(thread_id, NULL);
    }

    return 0;
}

void *read_dir(void *varg_p)
{
    // database_i *database = (database_i *)varg_p;

    int index = 0;

    for (int i = 0; i < ((database_i *)varg_p)->size; i++) {

        
        
        if(((database_i *)varg_p)->all_substrings[i]->status == 0){

            pthread_mutex_lock(&database_lock);
            puts("Locked mutex");

            ((database_i *)varg_p)->all_substrings[i]->status = 1;
            printf("Job: %s\n", ((database_i *)varg_p)->all_substrings[i]->substring);

            pthread_mutex_unlock(&database_lock);
            puts("UnLocked mutex");

            index = i;
            
            break;
        }
    
        
    }

    char *paths[] = { ((database_i *)varg_p)->root_dir, NULL };
	
	/* 2nd parameter: An options parameter. Must include either
	   FTS_PHYSICAL or FTS_LOGICAL---they change how symbolic links
	   are handled.
	   The 2nd parameter can also include the FTS_NOCHDIR bit (with a
	   bitwise OR) which causes fts to skip changing into other
	   directories. I.e., fts will call chdir() to literally cause
	   your program to behave as if it is running into another
	   directory until it exits that directory. See "man fts" for more
	   information.
	   Last parameter is a comparator which you can optionally provide
	   to change the traversal of the filesystem hierarchy.
	*/
	FTS *ftsp = fts_open(paths, FTS_PHYSICAL, NULL);
	if(ftsp == NULL)
	{
		perror("fts_open");
		exit(EXIT_FAILURE);
	}

	while(true) // call fts_read() enough times to get each file
	{

		FTSENT *ent = fts_read(ftsp); // get next entry (could be file or directory).
		if(ent == NULL)
		{
			if(errno == 0)
				break; // No more items, bail out of while loop
			else
			{
				// fts_read() had an error.
				perror("fts_read");
				exit(EXIT_FAILURE);
			}
		}
			
		// Given a "entry", determine if it is a file or directory
		if(ent->fts_info & FTS_D)   // We are entering into a directory
			//printf("Enter dir: ");
            ;
		else if(ent->fts_info & FTS_DP) // We are exiting a directory
			//printf("Exit dir:  ");
            ;
		else if(ent->fts_info & FTS_F) // The entry is a file. 
        {
            char *trigger = strstr(ent->fts_name, ((database_i *)varg_p)->all_substrings[index]->substring);
            if(trigger) {
                puts("---------------");

                file_i file_found = {};
                substring_i node = {};
            
                pthread_mutex_lock(&database_lock);
                puts("Locked mutex for insert");
                
                node.substring = strndup(((database_i *)varg_p)->all_substrings[index]->substring, 255);

                file_found.file_dir = strndup(ent->fts_path, 255);
                file_found.file_name = strndup(ent->fts_name, 255);
                node.file_hits = &file_found;

                // puts("Here");
			    printf("File name found: %s\n", ent->fts_name);
                printf("File dir found: %s\n", ent->fts_path);
                insert_node(((database_i *)varg_p), ((database_i *)varg_p)->all_substrings[index]->key, &node);

                pthread_mutex_unlock(&database_lock);
                puts("UnLocked mutex for insert");
                puts("---------------");
                free(file_found.file_dir);
                free(file_found.file_name);
                free(node.substring);
            }
                
        }
		else // entry is something else
			//printf("Other:     ");
            ;

		// print path to file after the label we printed above.
		//printf("%s\n", ent->fts_path);

	}

    pthread_mutex_lock(&database_lock);
    ((database_i *)varg_p)->all_substrings[index]->status = 2;
    pthread_mutex_unlock(&database_lock);
        

	// close fts and check for error closing.
	if(fts_close(ftsp) == -1)
		perror("fts_close");
	return 0;
}

int print_node(database_i *database) {

    for (size_t i = 0; i < database->size; ++i) {

        file_i *tmp = NULL;

        if (database->all_substrings[i]->file_hits) {

            printf("substring: %s\n", database->all_substrings[i]->substring);
            puts("@@@@@@@@@@@@@@@@@@");
            printf("file name: %s\n", database->all_substrings[i]->file_hits->file_name);
            puts("");
            printf("file dir: %s\n", database->all_substrings[i]->file_hits->file_dir);
            puts("");
            puts("######");

            if (database->all_substrings[i]->file_hits->next_hit) {
                tmp = database->all_substrings[i]->file_hits->next_hit;
            }

            while(tmp) {
                
                printf("file name: %s\n", tmp->file_name);
                printf("file dir: %s\n", tmp->file_dir);
                puts("######");

                tmp = tmp->next_hit;
            }
        }
        puts("@@@@@@@@@@@@@@@@@@");
    }

    return 0;
}

/*** end of file ***/