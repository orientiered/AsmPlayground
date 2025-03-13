#include <SFML/Audio/SoundStream.hpp>
#include <SFML/System/InputStream.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <curl/curl.h>
#include <SFML/Audio.hpp>

#include <NetworkAudio.h>

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"


void waitForSync();
void waitForLoadStop();

void swapBuffers();

size_t writeCallback(char *data, size_t size, size_t nmemb, void *clientp);

void fetchAudio();



/*Yes, these are global variables*/
/*They were initially hidden in class fields, but I got errors trying to access them within a separate thread */

std::thread loadThread;
bool stopLoading   = false;
bool isFetching    = false;
CURL *curl         = NULL;
stb_vorbis *vorbis = NULL;

int channels = 0;
size_t unusedSamples = 0;
float **floatSamples = NULL;
size_t  floatSamplesPos = 0;
loadBuffer_t loadBuffer = {0};
readBuffer_t readBuffer = {0};
// buffer to construct blocks of data for vorbis decoder
decodeBuffer_t decodeBuffer = {0};
typedef struct chunkBuffer_t {
    short data[MAX_SAMPLES];
    size_t ptr;
} chunkBuffer_t;
chunkBuffer_t chunkBuffer = {0};

bool NetworkOggAudio::open(const char *url_link) {
    // Setting up curl
    curl = curl_easy_init();
    if (!curl)  {
        fprintf(stderr, "Failed to initialize curl\n");
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url_link);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &loadBuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // starting data fetching
    loadThread = std::thread(fetchAudio);
    loadThread.detach();

    // getting first pacakge of data
    swapBuffers();
    // initializing vorbis
    int bytesConsumed = 0;
    int vorbis_error = 0;
    vorbis = stb_vorbis_open_pushdata(readBuffer.data, readBuffer.size, &bytesConsumed, &vorbis_error, NULL);
    readBuffer.readPtr += bytesConsumed;
    if (vorbis_error)
        printf("Error on vorbis init\n");

    printf("Vorbis used %d bytes on init\n", bytesConsumed);
    if (!vorbis) {
        printf("Vorbis failed to initiliaze\n");
        return false;
    }

    stb_vorbis_info audio_info = stb_vorbis_get_info(vorbis);
    printf("Channels = %d, sample_rate = %u\n", audio_info.channels, audio_info.sample_rate);
    channels = audio_info.channels;
    // forcing to use 1 channels
    initialize(1, audio_info.sample_rate);
    // initialize(audio_info.channels, audio_info.sample_rate);

    decodeBuffer.ptr = TEMP_BUFFER_SIZE; // ptr points to first byte that is not used

    printf("Radio started\n");
    return true;
}

NetworkOggAudio::~NetworkOggAudio() {
    NetworkOggAudio::stop();
    stopLoading = true;
    if (loadThread.joinable())
        loadThread.join();

    curl_easy_cleanup(curl);
    free(loadBuffer.data);
    free(readBuffer.data);
    stb_vorbis_close(vorbis);
    printf("Radio deinitialized\n");
}

//moves data in decodeBuffer to the start of buffer
int moveDecodeBuffer() {
    int move = decodeBuffer.ptr;
    decodeBuffer.ptr = 0;

    if (move == 0 || move == TEMP_BUFFER_SIZE) return move;

    for (size_t idx = 0; idx < TEMP_BUFFER_SIZE - move; idx++)
        decodeBuffer.data[idx] = decodeBuffer.data[idx + move];

    return move;
}

void refillDecodeBuffer() {
    size_t readAvailableBytes = readBuffer.size - (size_t) (readBuffer.readPtr - readBuffer.data);
    // data is given to vorbis in packages of TEMP_BUFFER_SIZE  size

    size_t bytesToLoad = decodeBuffer.ptr;
    moveDecodeBuffer();

    size_t writeOffset = TEMP_BUFFER_SIZE - bytesToLoad;
    if (readAvailableBytes < bytesToLoad) {
        memcpy(decodeBuffer.data + writeOffset, readBuffer.readPtr, readAvailableBytes);
        writeOffset += readAvailableBytes;
        swapBuffers();
        bytesToLoad -= readAvailableBytes;
        memcpy(decodeBuffer.data + writeOffset, readBuffer.readPtr, bytesToLoad);
        readBuffer.readPtr += bytesToLoad;
    } else {
        memcpy(decodeBuffer.data + writeOffset, readBuffer.readPtr, bytesToLoad);
        readBuffer.readPtr += bytesToLoad;
    }


}

float **getSamples(size_t *sampleCount) {
    int     samplesCount = 0;
    float **output = NULL;

    while (samplesCount == 0) {
        refillDecodeBuffer();

        int bytes_read = stb_vorbis_decode_frame_pushdata(vorbis,
                                                        decodeBuffer.data,
                                                        TEMP_BUFFER_SIZE,
                                                        NULL,
                                                        &output,
                                                        &samplesCount);

        int verror = stb_vorbis_get_error(vorbis);
        if (verror) {
            printf("Vorbis error %d\n", verror);
            return 0;
        }
        decodeBuffer.ptr += bytes_read;

        printf("Vorbis read %d bytes and created %d samples\n", bytes_read, samplesCount);

        if (bytes_read == 0)
            decodeBuffer.ptr = TEMP_BUFFER_SIZE; // forcing to fill buffer with new data
    }

    *sampleCount = samplesCount;
    return output;
}

void convertFloatAudioToSamples(short *dest, float *src, size_t size) {
    for (size_t idx = 0; idx < size; idx++) {
        dest[idx] = (signed short) ( src[size] * (32768) );
    }
}


bool NetworkOggAudio::onGetData(Chunk& data) {
    printf("Data request:\n\n");

    data.sampleCount = MAX_SAMPLES;
    chunkBuffer.ptr = 0;
    while (chunkBuffer.ptr < MAX_SAMPLES) {
        if (unusedSamples == 0) {
            floatSamples = getSamples(&unusedSamples);
            floatSamplesPos = 0;
            if (unusedSamples == 0) {
                printf("Data request failed\n");
                return false;

            }
        }

        size_t samplesToGet = std::min(unusedSamples, MAX_SAMPLES-chunkBuffer.ptr);
        // convertFloatAudioToSamples(chunkBuffer.data + chunkBuffer.ptr, floatSamples[0], samplesToGet);
        convert_channels_short_interleaved(1, chunkBuffer.data + chunkBuffer.ptr, 1, floatSamples, floatSamplesPos, samplesToGet);
        // int samplesGet = stb_vorbis_get_samples_short_interleaved(vorbis, channels,
                                // chunkBuffer.data + chunkBuffer.ptr,
                                // samplesToGet);

        unusedSamples -= samplesToGet;
        chunkBuffer.ptr += samplesToGet;
        floatSamplesPos += samplesToGet;

    }
    data.samples = chunkBuffer.data;
    printf("Returned %d samples\n", data.sampleCount);

    return true;

}

void NetworkOggAudio::onSeek(sf::Time timeOffset) {
    printf("Seek is not supported for network audio\n");
    return;
}

void waitForSync() {
    if (stopLoading)
        isFetching = false;

    while (stopLoading) ;

    isFetching = true;
}

void waitForLoadStop() {

    while (isFetching) ;
}

void swapBuffers() {
    // waiting for enough data to load
    while (loadBuffer.size < MINIMUM_BUFFER_SIZE) ;

    stopLoading = true;
    waitForLoadStop();

    std::swap(loadBuffer.data, readBuffer.data);
    std::swap(loadBuffer.size, readBuffer.size);
    std::swap(loadBuffer.reserved, readBuffer.reserved);
    readBuffer.readPtr = readBuffer.data;
    loadBuffer.size    = 0;

    printf("Read buffer updated, %lu bytes available\n", readBuffer.size);
    stopLoading = false;
}

// curl write callback function
size_t writeCallback(char *data, size_t size, size_t nmemb, void *clientp) {
    waitForSync();

    size_t realsize = size * nmemb;
    loadBuffer_t *mem = (loadBuffer_t *)clientp;

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

    return realsize;
}

//this function must be performed async
void fetchAudio() {
    curl_easy_perform(curl);
}
