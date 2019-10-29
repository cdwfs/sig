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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

size_t OUTPUT_BUFFER_SIZE = 8000 * 60 * sizeof(uint8_t);
uint8_t* outputBuffer = NULL;
uint64_t sampleCount = 0;
uint64_t playedSampleCount = 0;

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
  (void)pDevice;
  (void)pInput;

  memset(pOutput, 0, frameCount * sizeof(uint8_t));
  uint64_t numToCopy = sampleCount - playedSampleCount;
  if (numToCopy > frameCount)
    numToCopy = frameCount;
  memcpy(pOutput, outputBuffer + playedSampleCount, numToCopy);
  playedSampleCount += numToCopy;
  if (playedSampleCount >= OUTPUT_BUFFER_SIZE)
    exit(0);
}

int main()
{
    outputBuffer = (uint8_t*)malloc(OUTPUT_BUFFER_SIZE);

    int z=0,u=0,t=0;

    ma_device audioDevice;
    ma_device_config audioDeviceConfig = ma_device_config_init(ma_device_type_playback);

    audioDeviceConfig.playback.format = ma_format_u8;
    audioDeviceConfig.playback.channels = 1;
    audioDeviceConfig.sampleRate = 8000;
    audioDeviceConfig.dataCallback = data_callback;
    audioDeviceConfig.pUserData = NULL;

    if (ma_device_init(NULL, &audioDeviceConfig, &audioDevice) != MA_SUCCESS) {
      printf("Failed to open playback device.\n");
      return -3;
    }

    if (ma_device_start(&audioDevice) != MA_SUCCESS) {
      printf("Failed to start playback device.\n");
      ma_device_uninit(&audioDevice);
      return -4;
    }

    unsigned char *refSamples = NULL;
    size_t refFileNbytes = 0;
    FILE *refFile = fopen("reference.bin", "rb");
    if (refFile != NULL)
    {
        fseek(refFile, 0, SEEK_END);
        refFileNbytes = ftell(refFile);;
        fseek(refFile, 0, SEEK_SET);
        refSamples = malloc(refFileNbytes);
        fread(refSamples, refFileNbytes, 1, refFile);
        fclose(refFile);
    }

#if 0
    // Original string as it appeared in viznut's code
    const char *str = "`cW`g[`cgcg[eYcb^bV^eW^be^bVecb^";
    // ...and the loop to generate the array below
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

    int noteIndex = -1;
    for(;;)
    {
        // This constant is the number of samples to generate for each
        // note in the sequence. Higher values -> slower tempo
        int samplesPerStep = 999;
        // Advance to the next note in the sequence.
        noteIndex += 1;
        int sequenceLength = sizeof(notes) / sizeof(notes[0]);
        char note = notes[noteIndex % sequenceLength];
        // Every 3rd & 4th iteration through the sequence should be transposed
        // up three steps.
        if (noteIndex & 64)
            note += 3;
        // 1.06 is (approximately) the ratio of the frequencies of
        // successive notes in a 12-tone Western scale. We know that
        // each octave doubles the frequency. The increase between
        // each pair of notes is thus the 12th root of 2.0, or
        // 1.059463039(...).  This factor is VERY finicky; a
        // relatively small change will significantly affect the
        // tonality of the result. Try changing to 1.059, 1.058, etc.
        double stepFreqRatio = 1.06; // viznut's approximation, rounded to two places for brevity.
        // double stepFreqRatio = pow(2.0, 1.0/12.0); // more precise value

        // This gives a
        int freqRatio = (int)pow(stepFreqRatio, note);

        while(samplesPerStep != 0)
        {
            if (noteIndex & 1)
            {
                // Odd notes take this path
                u += t/2;
            }
            else
            {
                // Even notes take this path.
                t = (noteIndex & 6) ? t : freqRatio/4;
                u += t;
            }
            z += freqRatio;
            unsigned char byte = (unsigned char)(128 +
                ((8191&u)>samplesPerStep ? 0 : samplesPerStep/8) -
                ((8191&z)*samplesPerStep >> 16));
            // Check against reference samples, if available
            static int sampleIndex = 0;
            if (refSamples &&
                sampleIndex < refFileNbytes &&
                byte != refSamples[sampleIndex])
            {
                fprintf(stderr, "ERROR: sample mismatch at index %d (generated 0x%02X, expected 0x%02X)\n",
                    sampleIndex, byte, refSamples[sampleIndex]);
            }
            sampleIndex += 1;
            if (sampleIndex == refFileNbytes)
                fprintf(stderr, "All samples matched reference values until now\n");
            // Output
            //fputc(byte, dsp);
            if (sampleCount < OUTPUT_BUFFER_SIZE) {
              outputBuffer[sampleCount++] = byte;
            }
            samplesPerStep -= 1;
        }
    }

    ma_device_uninit(&audioDevice);

    return 0;
}
