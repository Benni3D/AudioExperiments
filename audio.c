#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static unsigned counter = 0;
static float f(float x) {
	++counter;
	return (float)rand() / RAND_MAX;
}


static float* normalize(float* a, int len) {
	float m = a[0];
	for (int i = 1; i < len; ++i) {
		if (a[i] > m) m = a[i];
	}
	printf("maximum: %f\n", m);
	for (int i = 0; i < len; ++i) a[i] /= m;
	return a;
}
static void my_audio_callback(void* userdata, uint8_t* stream, int len) {
	(void)userdata;
	float* buf = (float*)malloc(sizeof(float)*len);
	for (int i = 0; i < len; ++i) buf[i] = f(i);
	normalize(buf, len);
	for (int i = 0; i < len; ++i) stream[i] = (uint8_t)(buf[i] * 255.f);
	free(buf);
}

int main(void) {
	if (SDL_Init(SDL_INIT_AUDIO) > 0)
		return 1;
	SDL_AudioSpec wav_spec;
	wav_spec.callback = my_audio_callback;
	wav_spec.userdata = NULL;

	if (SDL_OpenAudio(&wav_spec, NULL) < 0) {
		printf("Failed to open audio: %s\n", SDL_GetError());
		return 2;
	}

	SDL_PauseAudio(0);
	printf("Press a key to finish");
	getchar();
	SDL_CloseAudio();
	SDL_Quit();
	return 0;
}
