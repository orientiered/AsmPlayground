#include <stdlib.h>
#include <string.h>
#include <thread>
#include <curl/curl.h>
#include <SFML/Audio.hpp>

typedef struct memory {
    uint8_t *data;
    size_t   size;
    size_t   reserved;
    uint8_t *playBuffer;
    size_t   playBufferSize;
    size_t   playBufferReserved;
} memory_t;

size_t writeCallback(char *data, size_t size, size_t nmemb, void *clientp) {
    size_t realsize = size * nmemb;
    memory_t *mem = (memory_t *)clientp;

    uint8_t *ptr = mem->data;
    if (mem->size + realsize + 1 > mem->reserved)
        ptr = (uint8_t*)realloc(mem->data, mem->reserved + realsize + 1);

    if(!ptr)
        return 0;  /* out of memory */

    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), data, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;

    return realsize;
}

const char * const keygen_FM = "http://stream.keygen-fm.ru:8042/live.ogg";

/*move it to structure or class*/
CURL *curl = NULL;
memory_t data = {0};
sf::SoundBuffer audioChunk;
sf::Sound       player;
/*------------------------------*/

int startRadio(const char *link) {
    curl = curl_easy_init();
    if (!curl) return -1;

    curl_easy_setopt(curl, CURLOPT_URL, link);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    return 0;
}

void swapBuffers(memory_t *mem) {
    uint8_t *tempPtr = mem->data;
    mem->data = mem->playBuffer;
    mem->playBuffer = tempPtr;

    size_t tempSize = mem->size;
    mem->size = mem->playBufferSize;
    mem->playBufferSize = tempSize;

    tempSize = mem->reserved;
    mem->reserved = mem->playBufferReserved;
    mem->playBufferReserved = tempSize;

}

void clearDataBuffer(memory_t *mem) {
    mem->size = 0;
}

void radioPlay() {
    curl_easy_perform(curl);

    if (player.getStatus() != sf::Sound::Stopped)
        return;

    swapBuffers(&data);
    clearDataBuffer(&data);

    audioChunk.loadFromMemory(data.playBuffer, data.playBufferSize);
    player.setBuffer(audioChunk);
    player.play();
}

int closeRadio() {
    curl_easy_cleanup(curl);
    free(data.data);

    return 0;
}
