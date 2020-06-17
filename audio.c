#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

static unsigned counter = 0;
static int mode;

static float randf(void) {
	return (float)rand() / (float)RAND_MAX;
}
// 0.001sin(0.001x) + 0.25sin(0.25x) + 0.5sin(0.5x) + sin(x) + 2sin(2x) + 3sin(3x) + 4sin(4x) + 5sin(5x) + 10sin(10x) 100sin(100x) + 1000sin(1000x) + -x + random^-1.5
static float f(float x, int len) {
	++counter;
	switch (mode) {
	case 0: return sin(x);
	case 1: return sin(x) + 2*sin(2*x) + 3*sin(3*x) + 4*sin(4*x) + 5*sin(5*x) + 6*sin(6*x) + 7*sin(7*x) + 8*sin(8*x) + 9*sin(9*x) + 10*sin(10*x);
	case 2: return x / len;
	case 3: return randf() + 10*sin(x/10);
	case 4: return x * sin(x);
	case 5: return randf() + sin(x) + 2*sin(x/2) + 3*sin(x/3) + 4*sin(x/4) + 5*sin(x/5) + 6*sin(x/6);
	case 6: return randf() + sin(x) + 2*sin(x/2) + 4*sin(x/4) + 8*sin(x/8) + 16*sin(x/16) + 32*sin(x/32);
	case 7: return sin(x) + 4*sin(x/4) + 16*sin(x/16) + 64*sin(x/64) + 256*sin(x/256) + 1024*sin(x/1024);
	case 8: return sin(x) + 0.5*sin(x/0.5) + 0.25*sin(x/0.25) + 0.125*sin(x/0.125);
	case 9: return randf() / 5 + sin(x/4) + sin(x/3) + sin(x/2) + sin(x) + sin(2*x) + sin(3*x) + sin(4*x);
	case 10:return randf() * 1000 + x;
	case 11: return 0.001*sin(0.001*x) + 0.25*sin(0.25*x) + 0.5*sin(0.5*x) + sin(x) + 2*sin(2*x) + 3*sin(3*x) + 4*sin(4*x) + 5*sin(5*x) + 10*sin(10*x) + 100*sin(100*x) + 1000*sin(1000*x) - x + pow(randf(), -1.5);
	default: return (float)rand() / RAND_MAX;
	}
}


static float* normalize(float* a, int len) {
	float m = a[0];
	for (int i = 1; i < len; ++i) {
		if (a[i] > m) m = a[i];
	}
	for (int i = 0; i < len; ++i) a[i] /= m;
	return a;
}
static void my_audio_callback(void* userdata, uint8_t* stream, int len) {
	(void)userdata;
	float* buf = (float*)malloc(sizeof(float)*len);
	for (int i = 0; i < len; ++i) buf[i] = f(i, len);
	normalize(buf, len);
	for (int i = 0; i < len; ++i) stream[i] = (uint8_t)(buf[i] * 255.f);
	free(buf);
}

static void quit(void) {
	SDL_CloseAudio();
	SDL_Quit();
}
int main(void) {
	if (SDL_Init(SDL_INIT_AUDIO) > 0)
		return 1;
	SDL_AudioSpec wav_spec;
	wav_spec.callback = my_audio_callback;
	wav_spec.userdata = NULL;
	atexit(quit);
	srand(time(NULL));

	printf("Enter mode: ");
	scanf("%d", &mode);

	if (SDL_OpenAudio(&wav_spec, NULL) < 0) {
		printf("Failed to open audio: %s\n", SDL_GetError());
		return 2;
	}

	SDL_PauseAudio(0);
	puts("Press CTRL + C to exit");
	while (1);
}
