#ifndef NETWORK_AUDIO_H
#define NETWORK_AUDIO_H

#include <stdint.h>

const int64_t MINIMUM_BUFFER_SIZE = 128*1024; //8 kb

const char * const keygen_FM =     "http://stream.keygen-fm.ru:8042/live.ogg";
const char * const keygen_FM_MP3 = "http://stream.keygen-fm.ru:8082/listen.mp3";
int startRadio(const char *link);

int closeRadio();

#endif
