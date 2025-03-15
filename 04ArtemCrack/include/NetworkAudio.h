#ifndef NETWORK_AUDIO_H
#define NETWORK_AUDIO_H

/* Simple way to play .ogg radio in your SFML project  */
/*  Dependencies:   libcurl      -- for stream data fetching
                    stb_vorbis.c -- for audio decoding
                    NOTE: NetworkOggAudio relies on convert_channels_short_interleaved
                        which is defined static in stb_vorbis.c, so this function was made public
    Issues: author doesn't know C++ well and cannot fix issues with running class methods in separate thread
        That's why all variables and functions that should be private class fields are global in NeworkAudio.cpp
        It means that only one instance of NetworkOggAudio may play simultaneously

    Usage:
        #include <NetworkOggAudio.h>
        ...
        NetworkOggAudio player;
        player.open("http://.../audio.ogg"); // example audio stream is keygen_FM
        player.play();
*/

#include <stdint.h>
#include <SFML/Audio.hpp>

// define NET_AUDIO_DBG to see every fetch and decode
#ifdef NET_AUDIO_DBG
    #define ON_NET_AUDIO_DBG(...) __VA_ARGS__
#else
    #define ON_NET_AUDIO_DBG(...) ;
#endif

/********************* CONSTANTS ************************************/

// Curl won't give data until it fetches MINIMUM_LOAD_BUFFER_FILL
const int64_t MINIMUM_LOAD_BUFFER_FILL  = 64*1024; //64 kb
// Array size for stb_vorbis decoding
const size_t  DECODE_BUFFER_SIZE        = 16*1024; //16 kb
// How many samples are computed on onGetData requst
const size_t  SAMPLES_PER_DATA_REQUEST  = 32000;

// Example radio link
const char * const keygen_FM =     "http://stream.keygen-fm.ru:8042/live.ogg";

/******************************** STRUCTS ******************************/

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
    uint8_t data[DECODE_BUFFER_SIZE];
    size_t  ptr;
} decodeBuffer_t;

class NetworkOggAudio : public sf::SoundStream {
public:
    /// @brief Initializes radio with given link
    /// @brief Use .play() method to start playing
    /// @return True on success, false otherwise
    bool open(const char *url_link);
    ~NetworkOggAudio();
protected:

    virtual bool onGetData(Chunk &data) override;
    virtual void onSeek(sf::Time timeOffset) override;

};

#endif
