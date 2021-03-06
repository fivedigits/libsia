#include <stdlib.h>
#include <assert.h>
#include <sox.h>
#include <complex.h>
#include <math.h>
#include <fftw3.h>

#define SAMPLE_BUFFER_SIZE 1024

#define FRAME_LENGTH 43

#define SUBBANDS 32

#define SENSITIVITY 10 

typedef struct {
	int band;
	double time;
} beat;

// first build a linked list of beats
typedef struct beat_node{
	beat * data;
	struct beat_node * next;
} beat_node_t;

// return value of compute_beat_vector
typedef struct {
	double rate;
	beat_node_t * beats;
} beat_vector;

int * compute_subbands() {

	int i;
	int * result;
	result = malloc((SUBBANDS+1)* sizeof(int));

	for (i=0; i <= SUBBANDS; i++) {

		result[i] = i * SAMPLE_BUFFER_SIZE / SUBBANDS;
	}

	return result;
}

void copy_ar(double ar1[SUBBANDS], double ar2[SUBBANDS]) {

	int i;

	for (i=0; i < SUBBANDS; i++) {

		ar2[i] = ar1[i];
	}
}

void push_ar_end(double ar1[SUBBANDS], double ar2[FRAME_LENGTH][SUBBANDS]) {

	int i;

	for (i=0; i < FRAME_LENGTH -1 ; i++) {

		copy_ar(ar2[i+1], ar2[i]);
	}

	copy_ar(ar1,ar2[FRAME_LENGTH-1]);
}

int get_length_node(beat_node_t * head) {

	int result = 0;

	beat_node_t * current;

	while (current != NULL) {

		result++;
		current = current->next;
	}

	return result;
}

beat_vector* compute_beat_vector(const char * filename) {

	static sox_format_t * file;
	int chans;
	int frames_read;
	double rate;

	fftw_complex mono_samples[SAMPLE_BUFFER_SIZE];
	fftw_complex freq_samples[SAMPLE_BUFFER_SIZE];
	fftw_plan fftplan = fftw_plan_dft_1d(SAMPLE_BUFFER_SIZE, mono_samples, freq_samples, FFTW_FORWARD, FFTW_ESTIMATE);

	double energy[SUBBANDS];
	double avg_energy[SUBBANDS];
	//This will hold the energy history
	double energy_hist[FRAME_LENGTH][SUBBANDS];

	int * subband_indices = compute_subbands();

	beat_node_t * head = NULL;
	beat_node_t * current = NULL;

	beat_vector* bv = malloc(sizeof(beat_vector));

	assert(sox_init() == SOX_SUCCESS);

	assert(file = sox_open_read(filename, NULL, NULL, NULL));

	chans = file->signal.channels;
	rate = file->signal.rate;

	int chunk_size = SAMPLE_BUFFER_SIZE * file->signal.channels;

	int insamples[chunk_size];

	frames_read = 0;

	while (sox_read(file, insamples, chunk_size) == chunk_size) {
		int i;

		// convert the samples to mono samples
		for (i = 0; i < SAMPLE_BUFFER_SIZE; i++) {

			int average = 0;	
			int j;

			for (j=0; j < chans; j++) {

				average += insamples[chans*i + j]/chans;
			}

			mono_samples[i] = (double) average * 1./(SOX_SAMPLE_MAX+1.);
		}


		fftw_execute(fftplan);

		// Compute energy of current sample, divided into subbands
		for (i=0; i < SUBBANDS; i++) {

			energy[i] = 0;

			int j;

			for (j= subband_indices[i]; j < subband_indices[i+1]; j++) {

				energy[i] += pow(cabs(freq_samples[j]),2)/(subband_indices[i+1] - subband_indices[i] + 1);
			}
		}

		// At the beginning, just fill in the energy hist
		if (frames_read < FRAME_LENGTH) {

			copy_ar(energy, energy_hist[frames_read]);

		} else { 
			// if the energy_hist is full, push current 
			// energy to the end and detect beats

			// first compute average energy on every subband
			for (i=0; i < SUBBANDS; i++) {

				avg_energy[i] = 0;

				int j;

				for (j=0;j < FRAME_LENGTH; j++) {

					avg_energy[i] += energy_hist[j][i] / FRAME_LENGTH;
				}

				if (energy[i] > SENSITIVITY * avg_energy[i]) {

					beat * b = malloc(sizeof(beat));
					
					b->band = i;
					b->time = frames_read * SAMPLE_BUFFER_SIZE / rate;

					if (current != NULL) {

						current->next = malloc(sizeof(beat_node_t));
						current->next->data = b;
						current->next->next = NULL;
						current = current->next;

					} else {

						head = malloc(sizeof(beat_node_t));
						head->data = b;
						head->next = NULL;

						current = head;
					}

				}
			}

			push_ar_end(energy,energy_hist);
		}

		frames_read++;
	}

	bv->beats = head;
	bv->rate = rate;

	fftw_destroy_plan(fftplan);

	sox_close(file);
	sox_quit();

	current = NULL;
	free(current);
	free(subband_indices);

	return bv;
}

int main () {

	beat_vector * bv = compute_beat_vector("test.ogg");

	printf("%f\n", bv->beats->data->time);

	free(bv);

	return 0;
}
