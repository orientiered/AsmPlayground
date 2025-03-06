#ifndef PATCHER_H
#define PATCHER_H

#include <stdlib.h>
#include <stdint.h>

enum EXIT_CODES {
    SUCCESS_EXIT = 0,
    NO_FILE_EXIT = 2,
    OPEN_ERROR_EXIT = 3,
    ADDR_ERROR_EXIT = 4,
    WRITE_ERROR_EXIT = 5,
    CLOSE_ERROR_EXIT = 6,
    ALREADY_PATCHED_EXIT = 7,
    BAD_HASH_EXIT = 8
};

typedef struct PatchStatus {
    const char * msg;
    enum EXIT_CODES code;
} PatchStatus_t;

typedef struct BytePatch {
    size_t addr;
    uint8_t replacement;
} BytePatch_t;


const BytePatch_t PatchTable[] = {
    {0x1, 0x29}
};

const char * const CONFIG_NAME = "patch.cfg";

const size_t PatchSize = sizeof(PatchTable) / sizeof(PatchTable[0]);

enum EXIT_CODES loadConfig(const char *fileName);

enum EXIT_CODES loadFile(const wchar_t *fileName);

enum EXIT_CODES checkHashBeforePatch();
enum EXIT_CODES checkHashAfterPatch();

enum EXIT_CODES patchCode(const BytePatch_t *patchTable, size_t patchSize);

enum EXIT_CODES closeFile();

enum EXIT_CODES closeConfig();




#endif
