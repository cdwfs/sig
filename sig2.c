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
void main(void)
{
	int v,i,z,n,u,t;
	FILE *dsp = fopen("/dev/dsp", "wb");
	for(v=-1;;)
	{
		for(n=pow(
1.06,"`cW`g[`cgcg[eYcb^bV^eW^be^bVecb^"[++v&31]+(v&64)/21),i=999;i;fputc(
	128+((8191&u)>i?0:i/8)-((8191&(z+=n))*i-->>16),dsp))u+=v&1?t/2:(t=v&6?t:n/4);
	}
	fclose(dsp);
}
