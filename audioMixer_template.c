// Incomplete implementation of an audio mixer. Search for "REVISIT" to find things
// which are left as incomplete.
// Note: Generates low latency audio on BeagleBone Black; higher latency found on host.
#include "audioMixer_template.h"
#include "sleep.h"
#include "joystick.h"
#include "accelerometer.h"


#define DEFAULT_VOLUME 80
#define SEC_PER_MINUTE 60
#define MIN_BPM 40
#define MAX_BPM 300

// Sample size note: This works for mono files because each sample ("frame') is 1 value.
// If using stereo files then a frame would be two samples.
#define SAMPLE_RATE 44100
#define NUM_CHANNELS 1
#define SAMPLE_SIZE (sizeof(short)) 			// bytes per sample
#define MAX_SOUND_BITES 30


static unsigned long playbackBufferSize = 0;
static short *playbackBuffer = NULL;
static int debounce = 100000000;
long int sec2nsec = 1000000000; //seconds to nanoseconds

// Currently active (waiting to be played) sound bites
typedef struct {
	// A pointer to a previously allocated sound bite (wavedata_t struct).
	// Note that many different sound-bite slots could share the same pointer
	// (overlapping cymbal crashes, for example)
	wavedata_t *pSound;

	// The offset into the pData of pSound. Indicates how much of the
	// sound has already been played (and hence where to start playing next).
	int location;
} playbackSound_t;


static playbackSound_t soundBites[MAX_SOUND_BITES];
static pthread_mutex_t audioMutex = PTHREAD_MUTEX_INITIALIZER;

int threshold = 500;
int gravity = 1000;

// Playback threading
void* playbackThread(void* arg);
static pthread_t playbackThreadId;

// Joystick control threading
void* joystickThread(void* arg);
static pthread_t joystickThreadId;

// Accelerometer control threading
void* accelerometerThread(void* arg);
static pthread_t accelerometerThreadId;


void Audio_playFile(snd_pcm_t *handle, wavedata_t *pWaveData)
{
	// If anything is waiting to be written to screen, can be delayed unless flushed.
	fflush(stdout);

	// Write data and play sound (blocking)
	snd_pcm_sframes_t frames = snd_pcm_writei(handle, pWaveData->pData, pWaveData->numSamples);

	// Check for errors
	if (frames < 0)
		frames = snd_pcm_recover(handle, frames, 0);
	if (frames < 0) {
		fprintf(stderr, "ERROR: Failed writing audio with snd_pcm_writei(): %li\n", frames);
		exit(EXIT_FAILURE);
	}
	if (frames > 0 && frames < pWaveData->numSamples)
		printf("Short write (expected %d, wrote %li)\n", pWaveData->numSamples, frames);
}


void AudioMixer_init(void)
{
	AudioMixer_setVolume(DEFAULT_VOLUME);

	BPM = 120; //default beats per minute
	// Initialize the currently active sound-bites being played
	// REVISIT:- Implement this. Hint: set the pSound pointer to NULL for each
	//     sound bite.
	for (int i = 0; i < MAX_SOUND_BITES; i ++){
		soundBites[i].pSound = NULL;
		soundBites[i].location = 0;
	}

	// Open the PCM output
	int err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	// Configure parameters of PCM output
	err = snd_pcm_set_params(handle,
			SND_PCM_FORMAT_S16_LE,
			SND_PCM_ACCESS_RW_INTERLEAVED,
			NUM_CHANNELS,
			SAMPLE_RATE,
			1,			// Allow software resampling
			50000);		// 0.05 seconds per buffer
	if (err < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	// Allocate this software's playback buffer to be the same size as the
	// the hardware's playback buffers for efficient data transfers.
	// ..get info on the hardware buffers:
 	unsigned long unusedBufferSize = 0;
	snd_pcm_get_params(handle, &unusedBufferSize, &playbackBufferSize);
	// ..allocate playback buffer:
	playbackBuffer = malloc(playbackBufferSize * sizeof(*playbackBuffer));

	// Launch playback thread:
	pthread_create(&playbackThreadId, NULL, playbackThread, NULL);

	// Launch joystick control thread:
	pthread_create(&joystickThreadId, NULL, joystickThread, NULL);

	// Launch accelerometer control thread:
	pthread_create(&accelerometerThreadId, NULL, accelerometerThread, NULL);
}


// Client code must call AudioMixer_freeWaveFileData to free dynamically allocated data.
void AudioMixer_readWaveFileIntoMemory(char *fileName, wavedata_t *pSound)
{
	assert(pSound);

	// The PCM data in a wave file starts after the header:
	const int PCM_DATA_OFFSET = 44;

	// Open the wave file
	FILE *file = fopen(fileName, "r");
	if (file == NULL) {
		fprintf(stderr, "ERROR: Unable to open file %s.\n", fileName);
		exit(EXIT_FAILURE);
	}

	// Get file size
	fseek(file, 0, SEEK_END);
	int sizeInBytes = ftell(file) - PCM_DATA_OFFSET;
	pSound->numSamples = sizeInBytes / SAMPLE_SIZE;

	// Search to the start of the data in the file
	fseek(file, PCM_DATA_OFFSET, SEEK_SET);

	// Allocate space to hold all PCM data
	pSound->pData = malloc(sizeInBytes);
	if (pSound->pData == 0) {
		fprintf(stderr, "ERROR: Unable to allocate %d bytes for file %s.\n",
				sizeInBytes, fileName);
		exit(EXIT_FAILURE);
	}

	// Read PCM data from wave file into memory
	int samplesRead = fread(pSound->pData, SAMPLE_SIZE, pSound->numSamples, file);
	if (samplesRead != pSound->numSamples) {
		fprintf(stderr, "ERROR: Unable to read %d samples from file %s (read %d).\n",
				pSound->numSamples, fileName, samplesRead);
		exit(EXIT_FAILURE);
	}
	fclose(file);
}

void AudioMixer_readFiles(){
	AudioMixer_readWaveFileIntoMemory(SOURCE_FILE53, &Hihat);
	AudioMixer_readWaveFileIntoMemory(SOURCE_FILE51, &Base);
	AudioMixer_readWaveFileIntoMemory(SOURCE_FILE59, &Snare);
	AudioMixer_readWaveFileIntoMemory(SOURCE_FILE52, &Kick);

	AudioMixer_readWaveFileIntoMemory(SOURCE_FILE56, &airDrumX);
	AudioMixer_readWaveFileIntoMemory(SOURCE_FILE54, &airDrumY);
	AudioMixer_readWaveFileIntoMemory(SOURCE_FILE60, &airDrumZ);
}

void AudioMixer_freeWaveFileData(wavedata_t *pSound)
{
	pSound->numSamples = 0;
	free(pSound->pData);
	pSound->pData = NULL;
}

void AudioMixer_queueSound(wavedata_t *pSound)
{
	// Ensure we are only being asked to play "good" sounds:
	assert(pSound->numSamples > 0);
	assert(pSound->pData);

	// Insert the sound by searching for an empty sound bite spot
	/*
	 * REVISIT: Implement this:
	 * 1. Since this may be called by other threads, and there is a thread
	 *    processing the soundBites[] array, we must ensure access is threadsafe.
	 * 2. Search through the soundBites[] array looking for a free slot.
	 * 3. If a free slot is found, place the new sound file into that slot.
	 *    Note: You are only copying a pointer, not the entire data of the wave file!
	 * 4. After searching through all slots, if no free slot is found then print
	 *    an error message to the console (and likely just return vs asserting/exiting
	 *    because the application most likely doesn't want to crash just for
	 *    not being able to play another wave file.
	 */

	int i;
	pthread_mutex_lock(&audioMutex);
	for (i = 0; i < MAX_SOUND_BITES; i++){
		if ( !soundBites[i].pSound ){
			soundBites[i].pSound = pSound;
			soundBites[i].location = 0;
			// printf("wave queued soundBites[%i]\n", i);
			pthread_mutex_unlock(&audioMutex);
			break; //break this loop
		}
	}
	if(i == MAX_SOUND_BITES) printf("No free slots found in soundBites");
}

void AudioMixer_cleanup(void)
{
	printf("Stopping audio...\n");

	// Stop the PCM generation thread
	stopping = true;
	pthread_join(playbackThreadId, NULL);
	pthread_join(joystickThreadId, NULL);
	pthread_join(accelerometerThreadId, NULL);

	
	// Shutdown the PCM output, allowing any pending sound to play out (drain)
	snd_pcm_drain(handle);
	snd_pcm_close(handle);

	// Free playback buffer
	// (note that any wave files read into wavedata_t records must be freed
	//  in addition to this by calling AudioMixer_freeWaveFileData() on that struct.)
	free(playbackBuffer);
	playbackBuffer = NULL;

	printf("Done stopping audio...\n");
	fflush(stdout);
}


int AudioMixer_getVolume()
{
	// Return the cached volume; good enough unless someone is changing
	// the volume through other means and the cached value is out of date.
	return volume;
}

// Function copied from:
// http://stackoverflow.com/questions/6787318/set-alsa-master-volume-from-c-code
// Written by user "trenki".
void AudioMixer_setVolume(int newVolume)
{
	// Ensure volume is reasonable; If so, cache it for later getVolume() calls.
	if (newVolume < 0 || newVolume > AUDIOMIXER_MAX_VOLUME) {
		printf("ERROR: Volume must be between 0 and 100.\n");
		return;
	}
	else volume = newVolume;
	
    long min, max;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "PCM";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);

    snd_mixer_close(handle);
}


// Fill the playbackBuffer array with new PCM values to output.
//    playbackBuffer: buffer to fill with new PCM data from sound bites.
//    size: the number of values to store into playbackBuffer
static void fillPlaybackBuffer(short *playbackBuffer, int size)
{
	/*
	 * REVISIT: Implement this
	 * 1. Wipe the playbackBuffer to all 0's to clear any previous PCM data.
	 *    Hint: use memset()
	 * 2. Since this is called from a background thread, and soundBites[] array
	 *    may be used by any other thread, must synchronize this.
	 * 3. Loop through each slot in soundBites[], which are sounds that are either
	 *    waiting to be played, or partially already played:
	 *    - If the sound bite slot is unused, do nothing for this slot.
	 *    - Otherwise "add" this sound bite's data to the play-back buffer
	 *      (other sound bites needing to be played back will also add to the same data).
	 *      * Record that this portion of the sound bite has been played back by incrementing
	 *        the location inside the data where play-back currently is.
	 *      * If you have now played back the entire sample, free the slot in the
	 *        soundBites[] array.
	 *
	 * Notes on "adding" PCM samples:
	 * - PCM is stored as signed shorts (between SHRT_MIN and SHRT_MAX).
	 * - When adding values, ensure there is not an overflow. Any values which would
	 *   greater than SHRT_MAX should be clipped to SHRT_MAX; likewise for underflow.
	 * - Don't overflow any arrays!
	 * - Efficiency matters here! The compiler may do quite a bit for you, but it doesn't
	 *   hurt to keep it in mind. Here are some tips for efficiency and readability:
	 *   * If, for each pass of the loop which "adds" you need to change a value inside
	 *     a struct inside an array, it may be faster to first load the value into a local
	 *      variable, increment this variable as needed throughout the loop, and then write it
	 *     back into the struct inside the array after. For example:
	 *           int offset = myArray[someIdx].value;
	 *           for (int i =...; i < ...; i++) {
	 *               offset ++;
	 *           }
	 *           myArray[someIdx].value = offset;
	 *   * If you need a value in a number of places, try loading it into a local variable
	 *          int someNum = myArray[someIdx].value;
	 *          if (someNum < X || someNum > Y || someNum != Z) {
	 *              someNum = 42;
	 *          }
	 *          ... use someNum vs myArray[someIdx].value;
	 *
	 */
	// printf("call fill buffer\n");
	memset(playbackBuffer, 0, size*SAMPLE_SIZE);
	pthread_mutex_lock(&audioMutex);
	for(int i=0; i<MAX_SOUND_BITES; i++){
		if(soundBites[i].pSound){
			int j = 0;
			while(j < playbackBufferSize && soundBites[i].location < soundBites[i].pSound->numSamples){
				int data = (int) playbackBuffer[j] + soundBites[i].pSound->pData[soundBites[i].location];
				if(data < SHRT_MIN) data = SHRT_MIN;
				if(data > SHRT_MAX) data = SHRT_MAX;
				playbackBuffer[j] = (short)data;
				soundBites[i].location++;
				j++;
			}
			if(soundBites[i].location >= soundBites[i].pSound->numSamples){
				// printf("finished readig 1 beat, set to null\n");
				soundBites[i].pSound = NULL;
			}
		}
	}
	pthread_mutex_unlock(&audioMutex);
}


void AudioMixer_setmode(int i){
	mode = i;
}

void AudioMixer_nextMode() {
	if (mode < 3) mode ++;
	else mode = 1;
};

void AudioMixer_rock(){
	//beat 1
	AudioMixer_queueSound(&Hihat);
	AudioMixer_queueSound(&Base);
	Sleeeep(0, ( (double) SEC_PER_MINUTE/BPM/2 )*sec2nsec);

	//beat 1.5
	AudioMixer_queueSound(&Hihat);
	Sleeeep(0, ( (double) SEC_PER_MINUTE/BPM/2 )*sec2nsec);

	//beat 2
	AudioMixer_queueSound(&Hihat);
	AudioMixer_queueSound(&Snare);	
	Sleeeep(0, ( (double) SEC_PER_MINUTE/BPM/2 )*sec2nsec);

	//beat 2.5
	AudioMixer_queueSound(&Hihat);
	Sleeeep(0, ( (double) SEC_PER_MINUTE/BPM/2 )*sec2nsec);
}

void AudioMixer_customrock(){	
	//beat 1
	AudioMixer_queueSound(&Kick);
	Sleeeep(0, ( (double) SEC_PER_MINUTE/BPM/2 )*sec2nsec);

	//beat 1.5
	AudioMixer_queueSound(&Hihat);
	Sleeeep(0, ( (double) SEC_PER_MINUTE/BPM/2 )*sec2nsec);

	//beat 2
	AudioMixer_queueSound(&Snare);
	Sleeeep(0, ( (double) SEC_PER_MINUTE/BPM/2 )*sec2nsec); //sleep for half a beat

	//beat 2.5
	AudioMixer_queueSound(&Hihat);
	Sleeeep(0, ( (double) SEC_PER_MINUTE/BPM/2 )*sec2nsec);

	//beat 3
	AudioMixer_queueSound(&Kick);
	Sleeeep(0, ( (double) SEC_PER_MINUTE/BPM/2 )*sec2nsec);

	//beat 3.5
	AudioMixer_queueSound(&Hihat);
	Sleeeep(0, ( (double) SEC_PER_MINUTE/BPM/4 )*sec2nsec);
	AudioMixer_queueSound(&Hihat);
	Sleeeep(0, ( (double) SEC_PER_MINUTE/BPM/4 )*sec2nsec);

	//beat 4
	AudioMixer_queueSound(&Snare);
	Sleeeep(0, ( (double) SEC_PER_MINUTE/BPM/2 )*sec2nsec);

	//beat 4.5
	AudioMixer_queueSound(&Hihat);
	Sleeeep(0, ( (double) SEC_PER_MINUTE/BPM/2 )*sec2nsec);

}

void AudioMixer_setBPM(int newbpm){
	if (newbpm < MIN_BPM || newbpm > MAX_BPM) {
		printf("ERROR: BPM must be between 40 and 300.\n");
		return;
	}
	else BPM = newbpm;
}

void AudioMixer_off(){

}

void* AudioMixer_Thread(void* arg)
{
	AudioMixer_init();
	AudioMixer_readFiles();
	AudioMixer_setmode(2);

	while(!stopping){
		// printf("playing volume %i, temp %i\n", volume, BPM);
		if(mode == 1) AudioMixer_off();
		if(mode == 2) AudioMixer_rock();
		if(mode == 3) AudioMixer_customrock();
	}

	//for tesing
	// Audio_playFile(handle,&Hihat);
	// Audio_playFile(handle,&Snare);
	// Audio_playFile(handle,&Base);

	return NULL;
}

void* playbackThread(void* arg)
{
	while (!stopping) {
		// Generate next block of audio
		fillPlaybackBuffer(playbackBuffer, playbackBufferSize);


		// Output the audio
		snd_pcm_sframes_t frames = snd_pcm_writei(handle,
				playbackBuffer, playbackBufferSize);

		// Check for (and handle) possible error conditions on output
		if (frames < 0) {
			fprintf(stderr, "AudioMixer: writei() returned %li\n", frames);
			frames = snd_pcm_recover(handle, frames, 1);
		}
		if (frames < 0) {
			fprintf(stderr, "ERROR: Failed writing audio with snd_pcm_writei(): %li\n",
					frames);
			exit(EXIT_FAILURE);
		}
		if (frames > 0 && frames < playbackBufferSize) {
			printf("Short write (expected %li, wrote %li)\n",
					playbackBufferSize, frames);
		}
	}

	return NULL;
}

void* joystickThread(void* arg)
{
	joystickInit();
	while( !stopping )
	{
		if (joystick_pressedIn())
		{
			AudioMixer_nextMode();
			Sleeeep(0,debounce);
		}
		else if (joystick_pressedUp())
		{
			volume += 5;
			AudioMixer_setVolume(volume);
			Sleeeep(0,debounce);
		}
		else if (joystick_pressedDw())
		{
			volume -= 5;
			AudioMixer_setVolume(volume);
			Sleeeep(0,debounce);
		}
		else if (joystick_pressedRt())
		{
			BPM += 5;
			if (BPM > MAX_BPM) BPM = MAX_BPM;
			Sleeeep(0,debounce);
		}
		else if (joystick_pressedLt())
		{
			BPM -= 5;
			if (BPM < MIN_BPM) BPM = MIN_BPM;
			Sleeeep(0,debounce);
		}
		// joystick_waitfor_release();
	}

	return NULL;
}


void* accelerometerThread(void* arg){
	printf("start accelerometer thread\n");
	Sleeeep(0, debounce);
	int file = accelerometerInit();
	while(!stopping){

		readAccelerometer(file);
		
		if (abs(x) > threshold) {
			printf("x detected\n");
			AudioMixer_queueSound(&airDrumX);
		}
		else if(abs(y) > threshold){
			printf("y detected\n");
			AudioMixer_queueSound(&airDrumY);
		}

		else if(abs(z-gravity) > threshold) {
			printf("z detected\n");
			AudioMixer_queueSound(&airDrumZ);
		}

		Sleeeep(0, debounce);
		// printf("Acceleration in X-Axis : %d \n", x);
		// printf("Acceleration in Y-Axis : %d \n", y);
		// printf("Acceleration in Z-Axis : %d \n\n", abs(z-1000));
		
	}
	return NULL;
}








