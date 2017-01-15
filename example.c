#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>
#include <sndfile.h>

typedef struct sf_data {
	int s_position;
	int f_position;
	int length;
	int channels;
	int rate;
	int * samples;
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
		if ( data->length -1 < data->s_position + thisSize) {

			thisRead = data->length - data->s_position;

			data->s_position = 0;
		} else {

			thisRead = thisSize;

			data->s_position += thisRead;
		}

		data->f_position = data->s_position /data->channels;

		// copy thisRead frames from input to output

		for(i=0; i < thisRead; i++) {

			cursor[i] = data->samples[i + data->s_position];
		}

		cursor += thisRead;

		thisSize -= thisRead;

	}

	return paContinue;	
}




int main() {

	SNDFILE * file;
	SF_INFO * sfinfo;

	int chans;
	int rate;
	int frames;

	PaStream *stream;
	PaError error;
	PaStreamParameters outputParameters;

	sf_data_t * soundData;

	soundData = malloc(sizeof(sf_data_t));
       
	sfinfo = malloc(sizeof(SF_INFO));

	sfinfo->format = 0;

	file = sf_open("test.ogg", SFM_READ, sfinfo);

	if (file == NULL) {

		printf("Error opening audio file!\n");
		return -1;
	}

	chans = sfinfo->channels;
	rate = sfinfo-> samplerate;	
	frames = sfinfo->frames;

	soundData->samples = malloc(sizeof(int) * frames * chans);

	// somehow can't read all frames at once
	frames = sf_readf_int(file, soundData->samples, frames);

	int j;

	for (j= 0; j < frames * chans; j++) {
		printf("%d\n",soundData->samples[j]);
	}
	// Done with file I/O
	sf_close(file);
	
	// Filling Data for PortAudio Callback
	soundData->length = frames * chans;
	soundData->channels = chans;
	soundData->rate = sfinfo->samplerate;
	soundData->s_position = 0;
	soundData->f_position = 0;

	free(sfinfo);
	// Start PortAudio
	
	Pa_Initialize();

	outputParameters.device = Pa_GetDefaultOutputDevice();

	outputParameters.channelCount = soundData->channels;
	
	outputParameters.sampleFormat = paInt32;

	outputParameters.suggestedLatency = 0.3;

	outputParameters.hostApiSpecificStreamInfo = 0;

	error = Pa_OpenStream(&stream,
				0,
				&outputParameters,
				soundData->rate,
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
	Pa_Sleep(2000);
	Pa_StopStream(stream);

	
	Pa_Terminate();

	free(soundData);

	return 0;

}
