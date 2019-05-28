// Playback sounds in real time, allowing multiple simultaneous wave files
// to be mixed together and played without jitter.
#ifndef AUDIO_MIXER_H
#define AUDIO_MIXER_H

#include <stdbool.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <limits.h>
#include <alloca.h> // needed for mixer


#define SOURCE_FILE51 "beatbox-wav-files/100051__menegass__gui-drum-bd-hard.wav"
#define SOURCE_FILE52 "beatbox-wav-files/100052__menegass__gui-drum-bd-soft.wav"
#define SOURCE_FILE53 "beatbox-wav-files/100053__menegass__gui-drum-cc.wav"
#define SOURCE_FILE54 "beatbox-wav-files/100054__menegass__gui-drum-ch.wav"
#define SOURCE_FILE55 "beatbox-wav-files/100055__menegass__gui-drum-co.wav"
#define SOURCE_FILE56 "beatbox-wav-files/100056__menegass__gui-drum-cyn-hard.wav"
#define SOURCE_FILE57 "beatbox-wav-files/100057__menegass__gui-drum-cyn-soft.wav"
#define SOURCE_FILE58 "beatbox-wav-files/100058__menegass__gui-drum-snare-hard.wav"
#define SOURCE_FILE59 "beatbox-wav-files/100059__menegass__gui-drum-snare-soft.wav"
#define SOURCE_FILE60 "beatbox-wav-files/100060__menegass__gui-drum-splash-hard.wav"
#define AUDIOMIXER_MAX_VOLUME 100


typedef struct {
	int numSamples;
	short *pData;
} wavedata_t;

snd_pcm_t *handle;
wavedata_t Hihat, Base, Snare, Kick, airDrumX, airDrumY, airDrumZ;

int mode; //default at standard rock
_Bool stopping;
//8th notes- 8 notes in 1 measure (play 2 beats every second, so BPM = 2 * 60 = 120 beats per minute)
//16th notes- 16 notes in 1 measure (play 4 beats every second, so 4 * 60 = 240 beats per minute)
int BPM;  
int volume;

// init() must be called before any other functions,
// cleanup() must be called last to stop playback threads and free memory.
void AudioMixer_init(void);
void AudioMixer_cleanup(void);

// Read the contents of a wave file into the pSound structure. Note that
// the pData pointer in this structure will be dynamically allocated in
// readWaveFileIntoMemory(), and is freed by calling freeWaveFileData().
void AudioMixer_readWaveFileIntoMemory(char *fileName, wavedata_t *pSound);
void AudioMixer_freeWaveFileData(wavedata_t *pSound);

// Queue up another sound bite to play as soon as possible.
void AudioMixer_queueSound(wavedata_t *pSound);

// Get/set the volume.
// setVolume() function posted by StackOverflow user "trenki" at:
// http://stackoverflow.com/questions/6787318/set-alsa-master-volume-from-c-code
int  AudioMixer_getVolume();
void AudioMixer_setVolume(int newVolume);
void AudioMixer_setBPM(int newbpm);

// beatbox modes
void AudioMixer_setmode(int i); //1 is no drum beat, 2 is rock, 3 is rock
void AudioMixer_off();
void AudioMixer_rock();
void AudioMixer_customrock();
void AudioMixer_nextMode();

//threads
void* AudioMixer_Thread(void* arg);

//play a sound
void Audio_playFile(snd_pcm_t *handle, wavedata_t *pWaveData);


#endif
