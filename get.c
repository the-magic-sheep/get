#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

int file_copy(char *source, char *target) {
    FILE *source_fd = fopen(source, "r");
    if (source_fd == NULL) {
        return 1;
    }

    FILE *target_fd = fopen(target, "w");
    if (target_fd == NULL) {
        fclose(source_fd);
        return 2;
    }

    char c;

    while ((c = fgetc(source_fd)) != EOF) {
        fputc(c, target_fd);
    }

    fclose(source_fd);
    fclose(target_fd);

    return 0;
}

char *get_dir_name(char *source_file, char *base_dir) {
    char *file_path = realpath(source_file, NULL);
    char *base_path = realpath(base_dir, NULL);

    if (file_path == NULL) {
        printf("Cannot find file with name '%s'\n", source_file);
        return NULL;
    }

    if (base_path == NULL) {
        printf("Cannot find base directory: \n%s\n", base_dir);
        free(file_path);
        return NULL;
    }

    // Verified that base_path exists. Now we can free this pointer.
    free(base_path);

    int dash_count = 0;
    for (int ii = 0; ii < strlen(file_path); ii++) {
        if (file_path[ii] == '/') {
            dash_count += 1;
        }
    }

    int dirname_size = strlen(file_path) + dash_count + strlen(base_dir) + 2;

    char *dir_name = malloc(dirname_size);

    int idx = strlen(base_dir);

    memcpy(dir_name, base_dir, idx);

    for (int ii = 0; ii < strlen(file_path); ii++) {
        if (file_path[ii] != '/') {
            dir_name[idx] = file_path[ii];
            idx += 1;
        } else {
            dir_name[idx] = '.';
            dir_name[idx + 1] = '.';
            idx += 2;
        }
    }
    dir_name[idx] = '\0';

    free(file_path);

    return dir_name;
}

char *get_checkpoint_path(char *source_file, char *checkpoint_name) {

    // Compute the directory name associated with the target file
    char *dir_name = get_dir_name(source_file, "./");

    if (dir_name == NULL) {
        return NULL;
    }

    // Create the cache directory for the target file
    // (If it doesn't exist)
    int dirmade = mkdir(dir_name, 0777);

    // Save the lengths of the dirname and the checpoint name
    int dir_name_len = strlen(dir_name);
    int cp_name_len = strlen(checkpoint_name);

    // Allocate a new string for the full file path
    // Includes space for the dirname, the checkpoint name,
    // and the forward slash separator.
    char *file_name = malloc(dir_name_len + cp_name_len + 2);

    // Copy the dirname into the file path
    memcpy(file_name, dir_name, dir_name_len);

    // We no longer need dir_name
    free(dir_name);

    // Add a forward slash separator
    file_name[dir_name_len] = '/';

    // Sopy the checkpoint name into the file path
    // This is the name of the file under the target file's cache dir.
    memcpy(&file_name[dir_name_len + 1], checkpoint_name, cp_name_len + 1);

    // Return the path to the checkpoint file.
    return file_name;
}

const char *STRINGS[] = {"cp", "checkpoint", "r", "revert"};

int print_help() {
    printf("Welcome to the get help menu!\n");
    return 0;
}

/**
 * Execute the checkpoint command of the get tool.
 *
 */
int checkpoint(int argc, char **argv) {

    // Verify that the correct number of arguments was provided.
    if (argc != 4) {
        printf("checkpoint requires 2 arguments:\n<target-file> "
               "<checkpoint-name>\n");
        return 3;
    }

    char *file_name = get_checkpoint_path(argv[2], argv[3]);

    // Only proceed if the dirname was successfully computed.
    if (file_name != NULL) {

        // copy the target file into the checkpoint cache file
        int did_copy = file_copy(argv[2], file_name);

        // Free the memory we used.
        free(file_name);

        // Return the file_copy return code.
        return did_copy;
    }

    // If the dirname computation failed, return 1.
    return 1;
}

int drop(int argc, char **argv) {
    /**
     * Execute the drop command of the get tool.
     *
     */

    // Verify that the correct number of arguments was provided.
    if (argc != 4) {
        printf("'drop' requires 2 arguments:\n<target-file> "
               "<checkpoint-name>\n");
        return 3;
    }

    // Double check that the user isn't trying to overwrite the
    // reserved __prev checkpoint.
    if (strcmp(argv[3], "__prev") == 0) {
        printf("'__prev' checkpoint cannot be dropped.\n");
        return 5;
    }

    char *file_name = get_checkpoint_path(argv[2], argv[3]);

    // Only proceed if the dirname was successfully computed.
    if (file_name != NULL) {

        // copy the target file into the checkpoint cache file
        int did_remove = remove(file_name);

        // Free the memory we used.
        free(file_name);

        // Return the did_remove return code.
        if (did_remove != 0) {
            printf(
                "Failed to remove checkpoint '%s'. It likely doesn't exist.\n",
                argv[3]);
        }
        return did_remove;
    }

    // If the dirname computation failed, return 1.
    return 1;
}

int load(int argc, char **argv) {
    /**
     * Execute the load command of the get tool.
     *
     */

    // TODO: save the current file in the file cache, just in case.

    // Verify that the correct number of arguments was provided.
    if (argc != 4) {
        printf("'load' requires 2 arguments:\n<target-file> "
               "<checkpoint-name>\n");
        return 3;
    }

    // Double check that the user isn't trying to overwrite the
    // reserved __prev checkpoint.
    if (strcmp(argv[3], "__prev") == 0) {
        printf("'__prev' cannot be manually loaded (use 'undo' instead).\n");
        return 5;
    }

    // Attempt to save the current state as a __prev checkpoint.
    char *save_prev_args[] = {"", "", argv[2], "__prev"};
    int did_save_prev = checkpoint(4, save_prev_args);

    if (did_save_prev != 0) {
        printf("Failed to cache current file state. "
               "Aborting.\n");
        return did_save_prev;
    }

    char *file_name = get_checkpoint_path(argv[2], argv[3]);

    // Only proceed if the dirname was successfully computed.
    if (file_name != NULL) {

        // copy the checkpoint cacke file into the target file.
        int did_copy = file_copy(file_name, argv[2]);

        if (did_copy == 1) {
            printf("File checkpoint '%s' does not exist.\n", argv[3]);
        }

        // Free the memory we used.
        free(file_name);

        // Return the did_copy return code.
        return did_copy;
    }

    // If the dirname computation failed, return 1.
    return 1;
}

int undo(int argc, char **argv) {
    /**
     * Execute the undo command of the get tool.
     *
     */

    // Verify that the correct number of arguments was provided.
    if (argc != 3) {
        printf("'undo' requires 1 arguments:\n<target-file>\n");
        return 3;
    }

    char *file_name = get_checkpoint_path(argv[2], "__prev");

    // If the file name was not computed successfully, exit with an error.
    if (file_name == NULL) {
        printf("Failed to fetch previous state.\n");
        return 1;
    }

    // copy the checkpoint cacke file into the target file.
    int did_copy = file_copy(file_name, argv[2]);

    if (did_copy == 1) {
        printf("Previous version is not saved. Perhaps no checkpoints have "
               "been loaded?\n");
    }

    // Free the memory we used.
    free(file_name);

    // Return the did_copy return code.
    return did_copy;

    return 0;
}

int list(int argc, char **argv) {
    /**
     * Execute the list command of the get tool.
     *
     */

    // Verify that the correct number of arguments was provided.
    if (argc != 3) {
        printf("'list' requires 1 arguments:\n<target-file>\n");
        return 3;
    }

    // Compute the directory name associated with the target file
    char *dir_name = get_dir_name(argv[2], "./");

    if (dir_name == NULL) {
        printf("Failed to compute cache directory name for '%s'\n", argv[2]);
        return 0;
    }

    DIR *d;

    struct dirent *dir;

    d = opendir(dir_name);

    free(dir_name);

    if (d) {
        while ((dir = readdir(d)) != NULL) {
            char *d_name = dir->d_name;
            if (strcmp(d_name, ".") != 0 && strcmp(d_name, "..") != 0 &&
                strcmp(d_name, "__prev") != 0) {
                printf("%s\n", d_name);
            }
        }
        closedir(d);
    }

    return 0;
}

int purge(int argc, char **argv) {
    /**
     * Execute the purge command of the get tool.
     *
     */

    // Verify that the correct number of arguments was provided.
    if (argc != 3) {
        printf("'purge' requires 1 arguments:\n<target-file>\n");
        return 3;
    }

    // Compute the directory name associated with the target file
    char *dir_name = get_dir_name(argv[2], "./");

    if (dir_name == NULL) {
        printf("Failed to compute cache directory name for '%s'\n", argv[2]);
        return 0;
    }

    DIR *d;

    struct dirent *dir;

    d = opendir(dir_name);

    int dir_name_len = strlen(dir_name);

    if (d) {
        while ((dir = readdir(d)) != NULL) {
            char *d_name = dir->d_name;
            if (strcmp(d_name, ".") != 0 && strcmp(d_name, "..") != 0) {
                int d_name_len = strlen(d_name);
                char *file_path = malloc(dir_name_len + d_name_len + 1);
                memcpy(file_path, dir_name, dir_name_len);
                file_path[dir_name_len] = '/';
                memcpy(&file_path[dir_name_len + 1], d_name, d_name_len + 1);

                remove(file_path);

                free(file_path);
            }
        }
        closedir(d);
    }

    int did_remove_dir = remove(dir_name);

    free(dir_name);

    return did_remove_dir;
}

int main(int argc, char **argv) {

    if (argc == 1 || (argc == 2 && strcmp(argv[1], "-h") == 0)) {
        return print_help();
    }

    if (strcmp(argv[1], "cp") == 0 || strcmp(argv[1], "checkpoint") == 0) {
        return checkpoint(argc, argv);
    }

    if (strcmp(argv[1], "d") == 0 || strcmp(argv[1], "drop") == 0) {
        return drop(argc, argv);
    }

    if (strcmp(argv[1], "l") == 0 || strcmp(argv[1], "load") == 0) {
        return load(argc, argv);
    }

    if (strcmp(argv[1], "u") == 0 || strcmp(argv[1], "undo") == 0) {
        return undo(argc, argv);
    }

    if (strcmp(argv[1], "list") == 0) {
        return list(argc, argv);
    }

    if (strcmp(argv[1], "purge") == 0) {
        return purge(argc, argv);
    }

    printf("'%s' is not a valid command.\nUse 'get' or 'get -h' to see valid "
           "commands.\n",
           argv[1]);

    return 1;
}
