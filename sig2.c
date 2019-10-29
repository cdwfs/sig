#if 0 // Original sig.c for reference
#include <math.h> /*** ---- viznut --- http://www.hytti.uku.fi/~vheikkil/ */
main(v,i,z,n,u,t){for(v=-1;;)for(n=pow(/* gcc -lm sig.c; a.out > /dev/dsp */
1.06,"`cW`g[`cgcg[eYcb^bV^eW^be^bVecb^"[++v&31]+(v&64)/21),i=999;i;putchar(
128+((8191&u)>i?0:i/8)-((8191&(z+=n))*i-->>16)))u+=v&1?t/2:(t=v&6?t:n/4);}
#endif

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
    // Initialize output buffer
    outputBuffer = (uint8_t*)malloc(OUTPUT_BUFFER_SIZE);

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

    // set to zero while playing with the code to avoid a spew of errors.
    // leave as one normally to validate that deobfuscation doesn't affect the output.
    const int validateOutput = 1;
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

    // The sequence of notes to play.
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

    int z = 0, u = 0, t = 0;
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
        // successive semitones in a 12-tone Western scale. We know that
        // each octave doubles the frequency. The increase between
        // each pair of notes is thus the 12th root of 2.0, or
        // 1.059463039(...).  This factor is VERY finicky; a
        // relatively small change will significantly affect the
        // tonality of the result. Try changing to 1.059, 1.058, etc.
        double semitoneFreqRatio = 1.06; // viznut's approximation, rounded to two places for brevity.
        // double stepFreqRatio = pow(2.0, 1.0/12.0); // more precise value

        // Raising each note of the sequence by the semitoneFreqRatio gives the following sequence of values,
        // which are the frequencies of each note to be played. Rounding the frequencies to integers does
        // cause some quantization error, but it's close enough and every character counts.
#if 0
        int noteFreqs[] = {
            268, 320, 159, 268, 404, 200, 268, 320,
            404, 320, 404, 200, 359, 178, 320, 301,
            239, 301, 150, 239, 359, 159, 239, 301,
            359, 239, 301, 150, 359, 320, 301, 239,
        };
#endif
        int freq = (int)pow(semitoneFreqRatio, note);

        // Generate a value for each sample in this step of the sequence
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
                t = (noteIndex & 6)
                    ? t
                    : freq / 4;
                u += t;
            }
            z += freq;

            // The original output code:
            //   putchar(128 + ((8191 & u) > i ? 0 : i / 8) - ((8191 & (z += n)) * i-- >> 16))
            // Regroup as:
            //   putchar(128 + (              a           ) - (                b             )
            int a = (8191 & u) > samplesPerStep
                ? 0
                : samplesPerStep / 8;
            // Let's say you had a table of 8192 samples of a waveform. If you write 8192 samples/second
            // from this table in a straight loop (for i=0; i<8192; i+=1), you would generate that waveform
            // exactly as represented in the input. If you increment i by 2 instead of 1 each time through
            // the loop, you'll generate the whole waveform in half the samples, resulting in double the
            // frequency.
            int b = ((8191 & z) * samplesPerStep) >> 16;
            samplesPerStep -= 1;
            unsigned char outputByte = (unsigned char)(128 + a - b);

            // Check against reference samples, if available
            static int sampleIndex = 0;
            if (refSamples &&
                validateOutput &&
                sampleIndex < refFileNbytes &&
                outputByte != refSamples[sampleIndex])
            {
                fprintf(stderr, "ERROR: sample mismatch at index %d (generated 0x%02X, expected 0x%02X)\n",
                    sampleIndex, outputByte, refSamples[sampleIndex]);
            }
            sampleIndex += 1;
            if (validateOutput && sampleIndex == refFileNbytes)
                fprintf(stderr, "All samples matched reference values until now\n");

            // Output
            if (sampleCount < OUTPUT_BUFFER_SIZE) {
                outputBuffer[sampleCount++] = outputByte;
            }
        }
    }

    ma_device_uninit(&audioDevice);

    return 0;
}
