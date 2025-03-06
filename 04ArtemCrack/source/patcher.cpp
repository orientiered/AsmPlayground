#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <math.h>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <globals.h>
#include <sf-textform.h>
#include <triangle-bkg.h>
#include <patcher.h>


typedef struct hashConfig {
    uint64_t hash_original;
    uint64_t hash_patched;
    uint64_t hash_before; // before patching
    uint64_t hash_after;  // after  patching
    FILE *file;
    bool hasConfig;
} hashConfig_t;

typedef struct fileData {
    uint8_t *data;
    int64_t length;
} fileData_t;

static hashConfig_t config;
static fileData_t   fileData;
enum EXIT_CODES loadConfig(const char *fileName) {
    config.hash_original= 0u;
    config.hash_patched = 0u;
    config.file = fopen(CONFIG_NAME, "r+");
    config.hasConfig = true;

    if (!config.file) {
        printf("! No config found, assuming all file choices are correct\n");
        config.file = fopen(CONFIG_NAME, "w");
        config.hasConfig = false;
    } else {
        fscanf(config.file, "%ju %ju", &config.hash_original, &config.hash_patched);
    }


    return SUCCESS_EXIT;
}

static int64_t getFileSize(const int fd) {
    struct stat st;
    int errCode = fstat(fd, &st);
    return st.st_size;
}

enum EXIT_CODES loadFile(const wchar_t *fileName) {

    // string in textforms are stored in wchar_t, so conversion is needed
    const size_t MBS_FILENAME_SIZE = 256;
    char mbsFileName[MBS_FILENAME_SIZE] = "";
    wcstombs(mbsFileName, fileName, MBS_FILENAME_SIZE);

    // opeping flie
    int fileDesc = open(mbsFileName, O_RDWR);

    if (fileDesc == -1)
        return OPEN_ERROR_EXIT;

    // getting length
    int64_t fileLength = getFileSize(fileDesc);

    // mapping file to virtual memory
    fileData.data = (uint8_t *) mmap(NULL, fileLength, PROT_WRITE | PROT_READ, MAP_SHARED,
                                      fileDesc, 0);
    fileData.length = fileLength;

    // we don't file anymore
    close(fileDesc);
    return SUCCESS_EXIT;
}

enum EXIT_CODES checkHashBeforePatch() {
    config.hash_before = memHash(fileData.data, fileData.length);
    printf("! Hash before patching: %#jX\n", config.hash_before);

    if (!config.hasConfig) {
        fprintf(config.file, "%ju\n", config.hash_before);
        return SUCCESS_EXIT;
    }

    if (config.hash_before == config.hash_original) {
        printf("! Originality of the file confirmed!\n");
        return SUCCESS_EXIT;
    }
    else if (config.hash_before == config.hash_patched) {
        printf("! Warning: this program has already been patched\n");
        return ALREADY_PATCHED_EXIT;
    } else {
        fprintf(stderr, "This file is not original. If you want to patch this file, delete %s\n", CONFIG_NAME);
        return BAD_HASH_EXIT;
    }

    return SUCCESS_EXIT;

}

enum EXIT_CODES checkHashAfterPatch() {
    config.hash_after = memHash(fileData.data, fileData.length);
    printf("! Hash after patching: %#jX\n", config.hash_after);

    if (!config.hasConfig) {
        fprintf(config.file, "%ju\n", config.hash_after);
        return SUCCESS_EXIT;
    }

    if (config.hash_after == config.hash_patched) {
        return SUCCESS_EXIT;
    }
    else {
        fprintf(stderr, "Something went wrong: hash of patched program doesn't match with reference hash\n");
        return BAD_HASH_EXIT;
    }

    return SUCCESS_EXIT;
}

enum EXIT_CODES patchCode(const BytePatch_t *patchTable, size_t patchSize) {
    for (size_t byte_idx = 0; byte_idx < patchSize; byte_idx++) {
        size_t addr = patchTable[byte_idx].addr;
        if (addr >= fileData.length) {
            fprintf(stderr, "! Error: Invalid address %zu\n", addr);
            return ADDR_ERROR_EXIT;
        }

        fileData.data[addr] = patchTable[byte_idx].replacement;
    }

    return SUCCESS_EXIT;
}

enum EXIT_CODES closeFile() {
    if (munmap(fileData.data, fileData.length) == -1)
        return CLOSE_ERROR_EXIT;

    return SUCCESS_EXIT;
}

enum EXIT_CODES closeConfig() {
    if (fclose(config.file) == EOF) {
        fprintf(stderr, "Failed to close file %s\n", CONFIG_NAME);
        return CLOSE_ERROR_EXIT;
    }

    return SUCCESS_EXIT;

}
