#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>
#include <sndfile.h>
#include <stdatomic.h>
#include "libsia.h"

typedef struct sf_data {
	int s_position;
	int f_position;
	SNDFILE * file;
	SF_INFO sfInfo;
} sf_data_t;

int callback(const void *input,
		void *output,
		unsigned long frameCount,
		const PaStreamCallbackTimeInfo* paTimeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData)
{
	sf_data_t *data = (sf_data_t *) userData;
	int *out = (int* ) output;
	int *cursor;
	int thisSize = frameCount;
	int thisRead;
	int i;

	cursor = out; 

	
	while (thisSize > 0) {
		// if we would read past end of file, don't read as much
		
		sf_seek(data->file,data->s_position,SEEK_SET);

		if (thisSize > (data->sfInfo.frames - data->s_position)) {

			thisRead = data->sfInfo.frames - data->s_position;

			data->s_position = 0;
		} else {

			thisRead = thisSize;

			data->s_position=data->s_position + thisRead;
		}

		data->f_position=data->s_position /data->sfInfo.channels;

		// copy thisRead frames from input to output

		sf_readf_int(data->file, cursor,thisRead);

		cursor += thisRead;

		thisSize -= thisRead;

	}

	return paContinue;	
}




int main() {

	PaStream *stream;
	PaError error;
	PaStreamParameters outputParameters;

	beat_vector * beatV = compute_beat_vector("test.ogg");
	
	double time;

	/*printf("%f\n",beatV->beats->data->time);*/

	sf_data_t * soundData;

	soundData = malloc(sizeof(sf_data_t));
       
	soundData->sfInfo.format = 0;

	soundData->file = sf_open("test.ogg", SFM_READ, &soundData->sfInfo);

	if (soundData->file == NULL) {

		printf("Error opening audio file!\n");
		return -1;
	}

	// Filling Data for PortAudio Callback
	soundData->s_position = 0;
	soundData->f_position = 0;

	// Start PortAudio
	
	Pa_Initialize();

	outputParameters.device = Pa_GetDefaultOutputDevice();

	if(outputParameters.device == paNoDevice) {

		printf("Couldn't initialize default output device\n");
		return -1;
	}

	outputParameters.channelCount = soundData->sfInfo.channels;
	
	outputParameters.sampleFormat = paInt32;

	outputParameters.hostApiSpecificStreamInfo = 0;

	outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultHighOutputLatency;

	error = Pa_OpenStream(&stream,
				0,
				&outputParameters,
				soundData->sfInfo.samplerate,
				paFramesPerBufferUnspecified,
				paNoFlag,
				callback,
				soundData);

	if (error) {
		printf("error opening output, error code = %i\n", error);
		Pa_Terminate();
		return 1;
	}

	Pa_StartStream(stream);

	time = 0;
	/*while (beatV->beats != NULL) {*/

		/*printf("Beat in band %d\n", beatV->beats->data->band);*/
		/*beatV->beats = beatV->beats->next;*/
	/*}*/
	Pa_Sleep(20000);

	Pa_StopStream(stream);

	
	Pa_Terminate();

	sf_close(soundData->file);
	free(soundData);

	return 0;

}
