#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


enum EXIT_CODES {
    NO_FILE_EXIT = 2,
    OPEN_ERROR_EXIT = 3,
    FSEEK_ERROR_EXIT = 4,
    WRITE_ERROR_EXIT = 5,
    CLOSE_ERROR_EXIT = 6
};

typedef struct BytePatch {
    size_t addr;
    uint8_t replacement;
} BytePatch_t;


BytePatch_t PatchTable[] = {
    {0x1, 0x29}
};

const size_t PatchSize = sizeof(PatchTable) / sizeof(PatchTable[0]);

int main(int argc, char *argv[]) {

    if (argc == 1) {
        fprintf(stderr, "No file selected\n");
        return NO_FILE_EXIT;
    }

    const char *fileName = argv[1];
    FILE *file = fopen(fileName, "rb+");
    if (!file) {
        fprintf(stderr, "Failed to open file %s\n", fileName);
        return OPEN_ERROR_EXIT;
    }

    printf("! Opened %s\n! Starting patching...\n", fileName);

    for (size_t byte_idx = 0; byte_idx < PatchSize; byte_idx++) {
        if (fseek(file, PatchTable[byte_idx].addr, SEEK_SET)) {
            fprintf(stderr, "Failed to set position 0x%X in file %s\n", PatchTable[byte_idx].addr, fileName);
            return FSEEK_ERROR_EXIT;
        }

        if (putc(PatchTable[byte_idx].replacement, file) == EOF) {
            fprintf(stderr, "Failed to write byte at position 0x%X\n", PatchTable[byte_idx].addr);
            return WRITE_ERROR_EXIT;
        }

    }

    printf("! Patching complete. Saving changes...\n");
    if (fclose(file) == EOF) {
        fprintf(stderr, "Failed to close file %s\n", fileName);
        return CLOSE_ERROR_EXIT;
    }

    printf("! Done \n$ Your executable is patched! $\n");

    return 0;
}
