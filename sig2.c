/**
 * /dev/dsp doesn't exist any more! New plan:
 * 1) Output data directly to /dev/dsp instead of stdout
 * 2) Compile: clang -lm sig.c
 * 3) Use PulseAudio to redirect /dev/dsp traffic to an actual audio driver:
 *    padsp ./a.out
 */

/*** ---- viznut --- http://www.hytti.uku.fi/~vheikkil/ */
/* gcc -lm sig.c; a.out > /dev/dsp */
#include <math.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
	int v,i,z,n,u,t;
	FILE *dsp = fopen("/dev/dsp", "wb");
#if 0
	const char *str = "`cW`g[`cgcg[eYcb^bV^eW^be^bVecb^";
	for(i=0; i<strlen(str); ++i)
	{
		printf("%d,", str[i]);
	}
	printf("\n");
	return 0;
#else
	char notes[] = {
		 96, 99, 87, 96,103, 91, 96, 99,
		103, 99,103, 91,101, 89, 99, 98,
		 94, 98, 86, 94,101, 87, 94, 98,
	    101, 94, 98, 86,101, 99, 98, 94,
	};
#endif

	for(v=-1;;)
	{
		/* This constant is the number of samples to generate for each
		 * note in the sequence. Higher values -> slower tempo
		 */
		int samplesPerStep = 999;
		/* Advance to the next note in the sequence. */
		v += 1; 
		char note = notes[v&31]; 
		/* Every 3rd & 4th iteration through the sequence should be transposed
		 * up three steps.
		 */
		if (v&64) 
			note += 3;
		/* 1.06 is (approximately) the ratio of the frequencies of
		 * successive notes in a 12-tone Western scale. We know that
		 * each octave doubles the frequency. The increase between
		 * each pair of notes is thus the 12th root of 2.0, or
		 * 1.059463039(...).  This factor is VERY finicky; a
		 * relatively small change will significantly affect the
		 * tonality of the result. Try changing to 1.059, 1.058, etc.
		*/
		double freqRatio = 1.06;
		int freq = pow(freqRatio, note);
		for(;samplesPerStep;
			fputc(128+((8191&u)>samplesPerStep?0:samplesPerStep/8)-((8191&(z+=freq))*samplesPerStep >> 16), dsp), samplesPerStep-=1
			)
		{
			u += v&1?t/2:(t=v&6?t:freq/4);
		}
	}
	fclose(dsp);
	return 0;
}
