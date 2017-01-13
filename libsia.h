#ifndef LIBSIA_H
#define LIBSIA_H

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

extern beat_vector* compute_beat_vector(const char * filename);

#endif
