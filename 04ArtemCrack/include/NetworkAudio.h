#ifndef NETWORK_AUDIO_H
#define NETWORK_AUDIO_H

#include <stdint.h>
#include <SFML/Audio.hpp>

const int64_t MINIMUM_BUFFER_SIZE = 64*1024; //64 kb
// const size_t  FREE_START_SPACE    =
const size_t TEMP_BUFFER_SIZE   = 16 * 1024; // 2 kb
const size_t MAX_SAMPLES  = 32000;

const char * const keygen_FM =     "http://stream.keygen-fm.ru:8042/live.ogg";
const char * const keygen_FM_MP3 = "http://stream.keygen-fm.ru:8082/listen.mp3";


//included from libs
// typedef struct stb_vorbis stb_vorbis;

typedef struct loadBuffer_t {
    uint8_t *data;
    size_t   size;
    size_t   reserved;
} loadBuffer_t;

typedef struct readBuffer_t {
    uint8_t *data;
    size_t   size;
    uint8_t *readPtr;
    size_t   reserved;
} readBuffer_t;

typedef struct decodeBuffer_t {
    uint8_t data[TEMP_BUFFER_SIZE];
    size_t  ptr;
} decodeBuffer_t;

class NetworkOggAudio : public sf::SoundStream {
public:
    bool open(const char *url_link);
    ~NetworkOggAudio();
protected:

    virtual bool onGetData(Chunk &data) override;
    virtual void onSeek(sf::Time timeOffset) override;

};

#endif
