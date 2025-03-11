#include <SFML/System/InputStream.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <mutex>
#include <curl/curl.h>
#include <SFML/Audio.hpp>

#include <NetworkAudio.h>

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

typedef struct memory {
    uint8_t *data;
    size_t   size;
    size_t   reserved;
    uint8_t *playBuffer;
    size_t   playBufferSize;
    int64_t   playBufferPtr;
    int64_t  realPosition;
    size_t   playBufferReserved;
} memory_t;

void swapBuffers(memory_t *mem);
void clearDataBuffer(memory_t *mem);
void fetchAudio();


/*move it to structure or class*/
CURL *curl = NULL;
memory_t data = {0};
// sf::SoundBuffer audioChunk;
// sf::Sound       player;
sf::Music       player;

std::thread     loadThread;
std::mutex      mtx;
bool stopLoading = false;
bool fetching    = false;
stb_vorbis *vorbis = NULL;
/*------------------------------*/

class NetworkStream : public sf::InputStream {
public:
    sf::Int64 read(void *dest, sf::Int64 size) override {
        printf("Read call to %X of %jd bytes\n", dest, size);

        if (data.playBufferPtr + size < data.playBufferSize ) {
            memcpy(dest, data.playBuffer + data.playBufferPtr, size);
            data.playBufferPtr += size;
            data.realPosition  += size;
            return size;
        }

        int64_t availableBytes = data.playBufferSize - data.playBufferPtr;

        if (availableBytes > 0) {
            memcpy(dest, data.playBuffer + data.playBufferPtr, availableBytes);
            size -= availableBytes;
            data.realPosition += availableBytes;
            dest = (uint8_t*) dest + availableBytes;
        }

        while (data.size < size || data.size < MINIMUM_BUFFER_SIZE) {
            //waiting while data is fetching
        }
        stopLoading = true;
        while (fetching) {
            // waiting to stop loadThread
        }
        swapBuffers(&data);
        clearDataBuffer(&data);
        // unlocking thread
        stopLoading = false;

        //copying left bytes
        memcpy(dest, data.playBuffer, size);
        data.playBufferPtr = size;
        data.realPosition += size;
        return size;

    }

    sf::Int64 seek(sf::Int64 position) override {
        printf("seek request to %jd\n", position);

        if (position < 0)
            return data.realPosition;
        // it is possible to seek only in playBuffer
        if (data.realPosition - position > data.playBufferPtr)
            return data.realPosition;
        if (position - data.realPosition > (int64_t) (data.playBufferSize - data.playBufferPtr) )
            return data.realPosition;

        data.playBufferPtr += position - data.realPosition;
        data.realPosition = position;
        return position;

        // Нет поддержки перемотки для сетевого потока
        return -1;
    }

    sf::Int64 tell() override {
        printf("Tell request: %jd\n", data.realPosition);
        return data.realPosition; // Не поддерживается
    }

    sf::Int64 getSize() override {
        printf("getSize request\n");
        return -1; // Неизвестен заранее
    }
private:

};

NetworkStream netStream = {};

size_t writeCallback(char *data, size_t size, size_t nmemb, void *clientp) {
    size_t realsize = size * nmemb;
    memory_t *mem = (memory_t *)clientp;

    uint8_t *ptr = mem->data;
    if (mem->size + realsize + 1 > mem->reserved) {
        ptr = (uint8_t*)realloc(mem->data, mem->reserved + realsize + 1);
        mem->reserved += realsize + 1;
    }

    if(!ptr)
        return 0;  /* out of memory */

    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), data, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;

    printf("Fetched %ju bytes\n", realsize);
    if (mem->size < MINIMUM_BUFFER_SIZE)
        return realsize;

    if (stopLoading)
        fetching = false;
    while (stopLoading) {
        // waiting
    }

    fetching = true;

    return realsize;
}

int startRadio(const char *link) {
    // if (!link)
        // link = keygen_FM;
    curl = curl_easy_init();
    if (!curl) return -1;

    curl

    curl_easy_setopt(curl, CURLOPT_URL, link);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    loadThread = std::thread(fetchAudio);
    loadThread.detach();
    // audioChunk.loadFromStream(netStream);
    // player.setBuffer(audioChunk);
    player.openFromStream(netStream);
    player.play();

    printf("Started radio\n");
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

    printf("Swapped buffers\n");
    printf("Available: %jd bytes\n", mem->playBufferSize);
}

void clearDataBuffer(memory_t *mem) {
    mem->size = 0;
}

void fetchAudio() {
    curl_easy_perform(curl);
}

int closeRadio() {
    player.stop();
    stopLoading = true;
    curl_easy_cleanup(curl);
    free(data.data);
    free(data.playBuffer);
    return 0;
}
