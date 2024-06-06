/*************************************Headers***************************************/
// File I/O
#include <stdio.h>
// Directory Traversal
#include <dirent.h>
// Threading
#include <pthread.h>
// String Manipulation
#include <string.h>
// Memory Allocation
#include <stdlib.h>
// Error Handling
#include <unistd.h>

/************************************Macros****************************************/
#define MAX_DIR_PATH 4096

/************************************Globals***************************************/
// FragmentData struct definition
typedef struct {
    int fragmentIndex;
    char *fragmentData;
    size_t fragmentSize;
} FragmentData;

// Array of pointers to FragmentData structs
FragmentData **fragmentArray = NULL;
// Total number of fragments
int totalFragments = 0;
// Mutex for synchronization
pthread_mutex_t fragmentMutex = PTHREAD_MUTEX_INITIALIZER;

/************************************Functions*************************************/
// Thread function to traverse directories recursively
void *traverse_directory(void *path);
// Function to read a file fragment and store it in the shared array
void read_file_fragment(char *filePath, int fragmentIndex);

/************************************Main******************************************/
int main(int argc, char **argv)
{
    // Check command-line arguments
    if (argc != 3)
    {
        printf("Usage: %s <directory> <output_file>\n", argv[0]);
        return 1;
    }

    // Check if the output file has a .mp3 extension
    char *outputExtension = strrchr(argv[2], '.');
    if (outputExtension == NULL || strcmp(outputExtension, ".mp3") != 0)
    {
        printf("Error: Output file must have a .mp3 extension.\n");
        return 2;
    }

    // Change current directory to the specified directory
    if (chdir(argv[1]) != 0)
    {
        fprintf(stderr, "Error: Could not change directory to %s.\n", argv[1]);
        return 3;
    }

    // Open the current directory
    DIR *currentDir = opendir(".");
    if (currentDir == NULL)
    {
        fprintf(stderr, "Error: Could not open current directory.\n");
        return 4;
    }

    // Array to store subdirectory paths
    char **subdirPaths = malloc(sizeof(char *));
    int numSubdirs = 0;

    // Read directory entries
    struct dirent *dirEntry;
    while ((dirEntry = readdir(currentDir)) != NULL)
    {
        // Skip "." and ".." entries
        if (strcmp(dirEntry->d_name, ".") == 0 || strcmp(dirEntry->d_name, "..") == 0)
        {
            continue;
        }

        // Allocate memory for the subdirectory path
        char *subdirPath = malloc(MAX_DIR_PATH);
        snprintf(subdirPath, MAX_DIR_PATH, "./%s", dirEntry->d_name);

        // Reallocate memory for the subdirectory paths array
        subdirPaths = realloc(subdirPaths, (numSubdirs + 1) * sizeof(char *));
        subdirPaths[numSubdirs] = subdirPath;
        numSubdirs++;
    }

    // Close the current directory
    closedir(currentDir);

    // Create threads for each subdirectory
    pthread_t *threads = malloc(numSubdirs * sizeof(pthread_t));
    for (int i = 0; i < numSubdirs; i++)
    {
        pthread_create(&threads[i], NULL, traverse_directory, subdirPaths[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < numSubdirs; i++)
    {
        pthread_join(threads[i], NULL);
        free(subdirPaths[i]);
    }

    // Free memory allocated for subdirectory paths array
    free(subdirPaths);
    free(threads);

    // Change back to the original directory
    chdir("..");

    // Open the output file for writing
    FILE *outputFile = fopen(argv[2], "wb");
    if (outputFile == NULL)
    {
        fprintf(stderr, "Error: Could not open output file %s.\n", argv[2]);
        return 5;
    }

    // Write the file fragments to the output file
    for (int i = 0; i < totalFragments; i++)
    {
        fwrite(fragmentArray[i]->fragmentData, 1, fragmentArray[i]->fragmentSize, outputFile);
        free(fragmentArray[i]->fragmentData);
        free(fragmentArray[i]);
    }

    // Free memory allocated for the fragment array
    free(fragmentArray);

    // Close the output file
    fclose(outputFile);

    return 0;
}

// Thread function to traverse directories recursively
void *traverse_directory(void *path)
{
    char *dirPath = (char *)path;

    // Open the directory
    DIR *dir = opendir(dirPath);
    if (dir == NULL)
    {
        fprintf(stderr, "Error: Could not open directory %s.\n", dirPath);
        return NULL;
    }

    // Read directory entries
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        // Skip "." and ".." entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        // Construct the full path of the entry
        char entryPath[MAX_DIR_PATH];
        snprintf(entryPath, MAX_DIR_PATH, "%s/%s", dirPath, entry->d_name);

        // Check if the entry is a directory
        if (entry->d_type == DT_DIR)
        {
            // Recursively traverse the subdirectory
            traverse_directory(entryPath);
        }
        // Check if the entry is a regular file with a ".bin" extension
        else if (entry->d_type == DT_REG && strstr(entry->d_name, ".bin") != NULL)
        {
            // Extract the fragment index from the file name
            int fragmentIndex = atoi(entry->d_name);
            // Process the file fragment
            read_file_fragment(entryPath, fragmentIndex);
        }
    }

    // Close the directory
    closedir(dir);

    return NULL;
}

// Function to read a file fragment and store it in the shared array
void read_file_fragment(char *filePath, int fragmentIndex)
{
    // Open the file in binary mode
    FILE *file = fopen(filePath, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "Error: Could not open file %s.\n", filePath);
        return;
    }

    // Getting the file size
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the file fragment data
    char *fragmentData = malloc(fileSize);
    if (fragmentData == NULL)
    {
        fprintf(stderr, "Error: Could not allocate memory for file fragment.\n");
        fclose(file);
        return;
    }

    // Read the file fragment into memory
    fread(fragmentData, 1, fileSize, file);

    // Close the file
    fclose(file);

    // Create a new FragmentData struct
    FragmentData *fragment = malloc(sizeof(FragmentData));
    fragment->fragmentIndex = fragmentIndex;
    fragment->fragmentData = fragmentData;
    fragment->fragmentSize = fileSize;

    // Lock the mutex
    pthread_mutex_lock(&fragmentMutex);

    // Resize the fragment array
    if (fragmentIndex >= totalFragments)
    {
        fragmentArray = realloc(fragmentArray, (fragmentIndex + 1) * sizeof(FragmentData *));
        for (int i = totalFragments; i < fragmentIndex + 1; i++)
        {
            fragmentArray[i] = NULL;
        }
        totalFragments = fragmentIndex + 1;
    }

    // Store the fragment in the array
    fragmentArray[fragmentIndex] = fragment;

    // Unlock the mutex
    pthread_mutex_unlock(&fragmentMutex);
}