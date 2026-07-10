#include <windows.h>
#include <mmsystem.h>
#include <math.h>
#include <intrin.h>
#pragma comment(lib, "winmm.lib")
#pragma warning(disable: 4996)

#define PI 3.141592f
#define SYNTH_LENGTH 16

static FLOAT pfSinVals[4096];
DWORD xs = 0;
INT nCounter = 0;

VOID SeedXorshift32(DWORD dwSeed) { xs = dwSeed; }

DWORD Xorshift32(VOID) {
    xs ^= xs << 13;
    xs ^= xs >> 17;
    xs ^= xs << 5;
    return xs;
}

VOID InitializeSine(VOID) {
    INT i;
    for (i = 0; i < 4096; i++)
        pfSinVals[i] = sinf((FLOAT)i / 4096.0f * PI * 2.f);
}

FLOAT FastSine(FLOAT f) {
    INT i = (INT)(f / (2.f * PI) * 4096.0f);
    return pfSinVals[i % 4096];
}

FLOAT FastCosine(FLOAT f) {
    return FastSine(f + PI / 2.f);
}

#define SineWave(t, freq, sampleCount)     FastSine(2.f * PI * (FLOAT)(freq) * (FLOAT)(t) / (FLOAT)(sampleCount))
#define SquareWave(t, freq, sampleCount)   (((BYTE)(2.f * (FLOAT)(freq) * ((t) / (FLOAT)(sampleCount))) % 2) == 0 ? 1.f : -1.f)
#define TriangleWave(t, freq, sampleCount) (4.f * (FLOAT)fabs(((FLOAT)(t) / ((FLOAT)(sampleCount) / (FLOAT)(freq))) - floor(((FLOAT)(t) / ((FLOAT)(sampleCount) / (FLOAT)(freq)))) - .5f) - 1.f)
#define SawtoothWave(t, freq, sampleCount) (fmod(((FLOAT)(t) / (FLOAT)(sampleCount)), (1.f / (FLOAT)(freq))) * (FLOAT)(freq) * 2.f - 1.f)

typedef VOID (WINAPI *AUDIO_SEQUENCE)(INT nSamplesPerSec, INT nSampleCount, PSHORT psSamples);
typedef VOID (WINAPI *AUDIOSEQUENCE_OPERATION)(INT nSamplesPerSec);

typedef struct {
    INT nSamplesPerSec;
    INT nSampleCount;
    AUDIO_SEQUENCE pAudioSequence;
    AUDIOSEQUENCE_OPERATION pPreAudioOp;
    AUDIOSEQUENCE_OPERATION pPostAudioOp;
} AUDIO_SEQUENCE_PARAMS;

AUDIO_SEQUENCE_PARAMS pAudioSequences[25];

VOID WINAPI AudioSequence1(INT nSamplesPerSec, INT nSampleCount, PSHORT psSamples) {
    INT t;
    for (t = 0; t < nSampleCount; t++) {
        INT nFreq = (INT)(FastSine((FLOAT)t / 10.f) * 100.f + 500.f);
        FLOAT fSine = FastSine((FLOAT)t / 10.f) * (FLOAT)nSamplesPerSec;
        psSamples[t] = (SHORT)(TriangleWave(t, nFreq, (FLOAT)nSamplesPerSec * 5.f + fSine) * (FLOAT)SHRT_MAX * .1f) +
                       (SHORT)(SquareWave(t, nFreq, nSampleCount) * (FLOAT)SHRT_MAX * .2f);
    }
}

VOID WINAPI AudioSequence2(INT nSamplesPerSec, INT nSampleCount, PSHORT psSamples) {
    INT t;
    UNREFERENCED_PARAMETER(nSamplesPerSec);
    for (t = 0; t < nSampleCount * 2; t++) {
        BYTE bFreq = (BYTE)((t | t % 255 | t % 257) + (t & t >> 8) + (t * (42 & t >> 10)) + ((t % ((t >> 8 | t >> 16) + 1)) ^ t));
        ((BYTE*)psSamples)[t] = bFreq;
    }
}

VOID WINAPI AudioSequence3(INT nSamplesPerSec, INT nSampleCount, PSHORT psSamples) {
    INT z, y, x;
    UNREFERENCED_PARAMETER(nSamplesPerSec);
    INT nCubeRoot = (INT)cbrtf((FLOAT)nSampleCount) + 1;
    for (z = 0; z < nCubeRoot; z++) {
        for (y = 0; y < nCubeRoot; y++) {
            for (x = 0; x < nCubeRoot; x++) {
                INT nIndex = z * nCubeRoot * nCubeRoot + y * nCubeRoot + x;
                if (nIndex >= nSampleCount) continue;
                INT nFreq = (INT)((FLOAT)(y & z & x) * FastSine((FLOAT)(z * y * x) / 100.f));
                psSamples[nIndex] =
                    (SHORT)(SquareWave(y + z * x, (nFreq % 500) + 1000, nSamplesPerSec) * (FLOAT)SHRT_MAX * .3f) +
                    (SHORT)(SawtoothWave(x | z, (150 - (nFreq % 200) / 4) + 800, nSamplesPerSec) * (FLOAT)SHRT_MAX * .3f) +
                    (SHORT)(TriangleWave((FLOAT)(x & y & z) + (SquareWave(x + y, nFreq % 50, nSamplesPerSec) * nSamplesPerSec),
                        (nFreq % 50) / 10 + 50, nSamplesPerSec) * (FLOAT)SHRT_MAX * .3f);
            }
        }
    }
}

VOID WINAPI AudioSequence4(INT nSamplesPerSec, INT nSampleCount, PSHORT psSamples) {
    INT t;
    UNREFERENCED_PARAMETER(nSamplesPerSec);
    for (t = 0; t < nSampleCount; t++) {
        INT nFreq = (INT)(FastSine((FLOAT)t / (1000.f - t / (nSampleCount / 1000))) * 100.f + 500.f);
        psSamples[t] = (SHORT)(SquareWave(t, nFreq, nSampleCount) * (FLOAT)SHRT_MAX * .1f);
    }
}

VOID WINAPI AudioSequence5(INT nSamplesPerSec, INT nSampleCount, PSHORT psSamples) {
    INT t;
    UNREFERENCED_PARAMETER(nSamplesPerSec);
    for (t = 0; t < nSampleCount; t++) {
        SHORT sFreq = (SHORT)(t * (t >> (t >> 13 & t)));
        psSamples[t] = sFreq;
    }
}

VOID WINAPI AudioSequence6(INT nSamplesPerSec, INT nSampleCount, PSHORT psSamples) {
    INT t;
    UNREFERENCED_PARAMETER(nSamplesPerSec);
    for (t = 0; t < nSampleCount * 2; t++) {
        BYTE bFreq = (BYTE)((t & ((t >> 18) + ((t >> 11) & t))) * t + (((t >> 8 & t) - (t >> 3 & t >> 8 | t >> 16)) & 128));
        ((BYTE*)psSamples)[t] = bFreq;
    }
}

VOID WINAPI AudioSequence7(INT nSamplesPerSec, INT nSampleCount, PSHORT psSamples) {
    INT t;
    UNREFERENCED_PARAMETER(nSamplesPerSec);
    for (t = 0; t < nSampleCount * 2; t++) {
        BYTE bFreq = (BYTE)(((t >> 12 & t >> 8) >> (t >> 20 & t >> 12)) * t);
        ((BYTE*)psSamples)[t] = bFreq;
    }
}

VOID WINAPI AudioSequence8(INT nSamplesPerSec, INT nSampleCount, PSHORT psSamples) {
    INT t;
    UNREFERENCED_PARAMETER(nSamplesPerSec);
    SHORT sRand = (SHORT)Xorshift32();
    for (t = 0; t < nSampleCount; t++) {
        INT nRand = (nSampleCount - t * 2) / 512;
        if (nRand < 24) nRand = 24;
        if (!(Xorshift32() % nRand)) {
            sRand = (SHORT)Xorshift32();
        }
        psSamples[t] = (SHORT)(SawtoothWave(t, sRand, nSampleCount) * (FLOAT)SHRT_MAX * .1f)
            & ~sRand | ((SHORT)Xorshift32() >> 12) +
            (SHORT)(SineWave(Xorshift32() % nSampleCount, nRand ^ sRand, nSampleCount) * (FLOAT)SHRT_MAX * .1f) +
            (SHORT)(TriangleWave(t, 3000, nSampleCount) * (FLOAT)SHRT_MAX);
    }
}

VOID WINAPI AudioSequence9(INT nSamplesPerSec, INT nSampleCount, PSHORT psSamples) {
    INT y, x;
    UNREFERENCED_PARAMETER(nSamplesPerSec);
    INT nSquareRoot = (INT)sqrtf((FLOAT)nSampleCount) + 1;
    for (y = 0; y < nSquareRoot; y++) {
        for (x = 0; x < nSquareRoot; x++) {
            INT nIndex = y * nSquareRoot + x;
            if (nIndex >= nSampleCount) continue;
            INT nFreq = (INT)((FLOAT)(y | x) * FastSine((FLOAT)(y * x) / 1000.f));
            psSamples[nIndex] =
                (SHORT)(SquareWave(y & x, (nFreq % 500) + 1000, nSamplesPerSec) * (FLOAT)SHRT_MAX * .3f);
        }
    }
}

VOID WINAPI AudioSequence10(INT nSamplesPerSec, INT nSampleCount, PSHORT psSamples) {
    INT t;
    UNREFERENCED_PARAMETER(nSamplesPerSec);
    for (t = 0; t < nSampleCount * 2; t++) {
        FLOAT w = powf(2.f, (FLOAT)(t >> 8 & t >> 13));
        BYTE bFreq = (BYTE)((t << ((t >> 1 | t >> 8) ^ (t >> 13)) | (t >> 8 & t >> 16) * t >> 4) + ((t * (t >> 7 | t >> 10)) >> (t >> 18 & t)) + (t * t) / ((t ^ t >> 12) + 1) + ((128 / ((BYTE)w + 1) & t) > 1 ? (BYTE)w * t : -(BYTE)w * (t + 1)));
        ((BYTE*)psSamples)[t] = bFreq;
    }
}

VOID WINAPI AudioSequence11(INT nSamplesPerSec, INT nSampleCount, PSHORT psSamples) {
    INT t;
    UNREFERENCED_PARAMETER(nSamplesPerSec);
    for (t = 0; t < nSampleCount * 2; t++) {
        BYTE bFreq = (BYTE)((t * ((t >> 8 & t >> 3) >> (t >> 16 & t))) + ((t * (t >> 8 & t >> 3)) >> (t >> 16 & t)));
        ((BYTE*)psSamples)[t] = bFreq;
    }
}

VOID WINAPI AudioSequence12(INT nSamplesPerSec, INT nSampleCount, PSHORT psSamples) {
    INT t;
    UNREFERENCED_PARAMETER(nSamplesPerSec);
    for (t = 0; t < nSampleCount; t++) {
        psSamples[t] = (SHORT)(TriangleWave(__rdtsc() % 8, 1500, nSampleCount) * (FLOAT)SHRT_MAX * .3f) |
                       (SHORT)(SquareWave(__rdtsc() % 8, 1000, nSampleCount) * (FLOAT)SHRT_MAX * .3f) + (SHORT)~t + ((SHORT)t >> 2);
    }
}

VOID WINAPI AudioSequence13(INT nSamplesPerSec, INT nSampleCount, PSHORT psSamples) {
    INT t;
    UNREFERENCED_PARAMETER(nSamplesPerSec);
    for (t = 0; t < nSampleCount; t++) {
        psSamples[t] = (SHORT)(SawtoothWave(__rdtsc() % 1500, 1500, nSampleCount) * (FLOAT)SHRT_MAX * .3f) ^
                       ((SHORT)(SawtoothWave(t % 10, t % 1000, nSampleCount) * (FLOAT)SHRT_MAX * .1f) >> 8);
    }
}

VOID WINAPI AudioSequence14(INT nSamplesPerSec, INT nSampleCount, PSHORT psSamples) {
    INT y, x;
    UNREFERENCED_PARAMETER(nSamplesPerSec);
    INT nSquareRoot = (INT)sqrtf((FLOAT)nSampleCount) + 1;
    for (y = 0; y < nSquareRoot; y++) {
        for (x = 0; x < nSquareRoot; x++) {
            INT nIndex = y * nSquareRoot + x;
            if (nIndex >= nSampleCount) continue;
            INT nFreq = (INT)((FLOAT)(y | x) * FastCosine((FLOAT)(y & x) / 10.f));
            psSamples[nIndex] = (SHORT)(SineWave(y + x, (nFreq % 1000) + 1000, nSamplesPerSec) * (FLOAT)SHRT_MAX * .3f);
        }
    }
}

VOID WINAPI AudioSequence15(INT nSamplesPerSec, INT nSampleCount, PSHORT psSamples) {
    INT y, x;
    UNREFERENCED_PARAMETER(nSamplesPerSec);
    INT nSquareRoot = (INT)sqrtf((FLOAT)nSampleCount) + 1;
    for (y = 0; y < nSquareRoot; y++) {
        for (x = 0; x < nSquareRoot; x++) {
            INT nIndex = y * nSquareRoot + x;
            if (nIndex >= nSampleCount) continue;
            INT nFreq = (INT)((FLOAT)(y - x) * FastCosine((FLOAT)(y * x) / 10.f));
            psSamples[nIndex] = (SHORT)(SineWave(y % (x + 1), (nFreq % 100) + 100, nSamplesPerSec) * (FLOAT)SHRT_MAX * .3f);
        }
    }
}

VOID WINAPI AudioSequence16(INT nSamplesPerSec, INT nSampleCount, PSHORT psSamples) {
    INT y, x;
    UNREFERENCED_PARAMETER(nSamplesPerSec);
    INT nSquareRoot = (INT)sqrtf((FLOAT)nSampleCount) + 1;
    for (y = 0; y < nSquareRoot; y++) {
        for (x = 0; x < nSquareRoot; x++) {
            INT nIndex = y * nSquareRoot + x;
            if (nIndex >= nSampleCount) continue;
            INT nFreq = (INT)((FLOAT)(y ^ x) * exp(cosh(atanf((FLOAT)(y | x)) / 10.f)) * 2.f);
            psSamples[nIndex] = (SHORT)(SineWave(y - (x % (y + 1)), (nFreq % 100) + 500, nSamplesPerSec) * (FLOAT)SHRT_MAX * .3f);
        }
    }
}

VOID WINAPI FinalAudioSequence(INT nSamplesPerSec, INT nSampleCount, PSHORT psSamples) {
    INT z, y, x;
    UNREFERENCED_PARAMETER(nSamplesPerSec);
    INT nCubeRoot = (INT)cbrtf((FLOAT)nSampleCount) + 1;
    for (z = 0; z < nCubeRoot; z++) {
        for (y = 0; y < nCubeRoot; y++) {
            for (x = 0; x < nCubeRoot; x++) {
                INT nIndex = z * nCubeRoot * nCubeRoot + y * nCubeRoot + x;
                if (nIndex >= nSampleCount) continue;
                INT nFreq = (INT)((FLOAT)(y & x) * sinf((FLOAT)z / (FLOAT)nCubeRoot + (FLOAT)x + (FLOAT)nCounter * (FLOAT)y) * 2.f);
                psSamples[nIndex] = (SHORT)(SquareWave(nIndex, nFreq, nSamplesPerSec) * (FLOAT)(SHRT_MAX) * .3f);
            }
        }
    }
}

VOID WINAPI ExecuteAudioSequence(
    INT nSamplesPerSec,
    INT nSampleCount,
    AUDIO_SEQUENCE pAudioSequence,
    AUDIOSEQUENCE_OPERATION pPreAudioOp,
    AUDIOSEQUENCE_OPERATION pPostAudioOp)
{
    HANDLE hHeap = GetProcessHeap();
    PSHORT psSamples = (PSHORT)HeapAlloc(hHeap, 0, nSampleCount * 2);
    WAVEFORMATEX waveFormat;
    WAVEHDR waveHdr;
    HWAVEOUT hWaveOut = NULL;

    if (!psSamples) return;

    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = 1;
    waveFormat.nSamplesPerSec = nSamplesPerSec;
    waveFormat.nAvgBytesPerSec = nSamplesPerSec * 2;
    waveFormat.nBlockAlign = 2;
    waveFormat.wBitsPerSample = 16;
    waveFormat.cbSize = 0;

    waveHdr.lpData = (LPSTR)psSamples;
    waveHdr.dwBufferLength = (DWORD)(nSampleCount * 2);
    waveHdr.dwBytesRecorded = 0;
    waveHdr.dwUser = 0;
    waveHdr.dwFlags = 0;
    waveHdr.dwLoops = 0;
    waveHdr.lpNext = NULL;
    waveHdr.reserved = 0;

    waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveFormat, 0, 0, 0);

    if (pPreAudioOp) pPreAudioOp(nSamplesPerSec);
    pAudioSequence(nSamplesPerSec, nSampleCount, psSamples);
    if (pPostAudioOp) pPostAudioOp(nSamplesPerSec);

    waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(waveHdr));
    waveOutWrite(hWaveOut, &waveHdr, sizeof(waveHdr));

    Sleep((DWORD)(nSampleCount * 1000LL / nSamplesPerSec));
    while (!(waveHdr.dwFlags & WHDR_DONE)) Sleep(1);

    waveOutReset(hWaveOut);
    waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(waveHdr));
    waveOutClose(hWaveOut);
    HeapFree(hHeap, 0, psSamples);
}

VOID WINAPI AudioPayloadThread(VOID) {
    for (;;) {
        INT piOrder[SYNTH_LENGTH];
        INT nRandIndex, nNumber;
        INT i;

        for (i = 0; i < SYNTH_LENGTH; i++)
            piOrder[i] = i;

        for (i = 0; i < SYNTH_LENGTH; i++) {
            nRandIndex = Xorshift32() % 16;
            nNumber = piOrder[nRandIndex];
            piOrder[nRandIndex] = piOrder[i];
            piOrder[i] = nNumber;
        }

        for (i = 0; i < SYNTH_LENGTH; i++) {
            ExecuteAudioSequence(
                pAudioSequences[i].nSamplesPerSec,
                pAudioSequences[i].nSampleCount,
                pAudioSequences[i].pAudioSequence,
                pAudioSequences[i].pPreAudioOp,
                pAudioSequences[i].pPostAudioOp);
        }
    }
}

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pszCmdLine, INT nShowCmd) {
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(pszCmdLine);
    UNREFERENCED_PARAMETER(nShowCmd);

    SeedXorshift32((DWORD)__rdtsc());
    InitializeSine();

    pAudioSequences[0].nSamplesPerSec = 48000;  pAudioSequences[0].nSampleCount = 48000 * 30;  pAudioSequences[0].pAudioSequence = AudioSequence1;  pAudioSequences[0].pPreAudioOp = NULL;  pAudioSequences[0].pPostAudioOp = NULL;
    pAudioSequences[1].nSamplesPerSec = 8000;   pAudioSequences[1].nSampleCount = 8000 * 30;   pAudioSequences[1].pAudioSequence = AudioSequence2;  pAudioSequences[1].pPreAudioOp = NULL;  pAudioSequences[1].pPostAudioOp = NULL;
    pAudioSequences[2].nSamplesPerSec = 8000;   pAudioSequences[2].nSampleCount = 8000 * 30;   pAudioSequences[2].pAudioSequence = AudioSequence3;  pAudioSequences[2].pPreAudioOp = NULL;  pAudioSequences[2].pPostAudioOp = NULL;
    pAudioSequences[3].nSamplesPerSec = 16000;  pAudioSequences[3].nSampleCount = 16000 * 30;  pAudioSequences[3].pAudioSequence = AudioSequence4;  pAudioSequences[3].pPreAudioOp = NULL;  pAudioSequences[3].pPostAudioOp = NULL;
    pAudioSequences[4].nSamplesPerSec = 8000;   pAudioSequences[4].nSampleCount = 8000 * 30;   pAudioSequences[4].pAudioSequence = AudioSequence5;  pAudioSequences[4].pPreAudioOp = NULL;  pAudioSequences[4].pPostAudioOp = NULL;
    pAudioSequences[5].nSamplesPerSec = 8000;   pAudioSequences[5].nSampleCount = 8000 * 30;   pAudioSequences[5].pAudioSequence = AudioSequence6;  pAudioSequences[5].pPreAudioOp = NULL;  pAudioSequences[5].pPostAudioOp = NULL;
    pAudioSequences[6].nSamplesPerSec = 12000;  pAudioSequences[6].nSampleCount = 12000 * 30;  pAudioSequences[6].pAudioSequence = AudioSequence7;  pAudioSequences[6].pPreAudioOp = NULL;  pAudioSequences[6].pPostAudioOp = NULL;
    pAudioSequences[7].nSamplesPerSec = 48000;  pAudioSequences[7].nSampleCount = 48000 * 30;  pAudioSequences[7].pAudioSequence = AudioSequence8;  pAudioSequences[7].pPreAudioOp = NULL;  pAudioSequences[7].pPostAudioOp = NULL;
    pAudioSequences[8].nSamplesPerSec = 48000;  pAudioSequences[8].nSampleCount = 48000 * 30;  pAudioSequences[8].pAudioSequence = AudioSequence9;  pAudioSequences[8].pPreAudioOp = NULL;  pAudioSequences[8].pPostAudioOp = NULL;
    pAudioSequences[9].nSamplesPerSec = 8000;   pAudioSequences[9].nSampleCount = 8000 * 30;   pAudioSequences[9].pAudioSequence = AudioSequence10; pAudioSequences[9].pPreAudioOp = NULL;  pAudioSequences[9].pPostAudioOp = NULL;
    pAudioSequences[10].nSamplesPerSec = 8000;  pAudioSequences[10].nSampleCount = 8000 * 30;  pAudioSequences[10].pAudioSequence = AudioSequence11; pAudioSequences[10].pPreAudioOp = NULL; pAudioSequences[10].pPostAudioOp = NULL;
    pAudioSequences[11].nSamplesPerSec = 8000;  pAudioSequences[11].nSampleCount = 8000 * 30;  pAudioSequences[11].pAudioSequence = AudioSequence12; pAudioSequences[11].pPreAudioOp = NULL; pAudioSequences[11].pPostAudioOp = NULL;
    pAudioSequences[12].nSamplesPerSec = 16000; pAudioSequences[12].nSampleCount = 16000 * 30; pAudioSequences[12].pAudioSequence = AudioSequence13; pAudioSequences[12].pPreAudioOp = NULL; pAudioSequences[12].pPostAudioOp = NULL;
    pAudioSequences[13].nSamplesPerSec = 48000; pAudioSequences[13].nSampleCount = 48000 * 30; pAudioSequences[13].pAudioSequence = AudioSequence14; pAudioSequences[13].pPreAudioOp = NULL; pAudioSequences[13].pPostAudioOp = NULL;
    pAudioSequences[14].nSamplesPerSec = 48000; pAudioSequences[14].nSampleCount = 48000 * 30; pAudioSequences[14].pAudioSequence = AudioSequence15; pAudioSequences[14].pPreAudioOp = NULL; pAudioSequences[14].pPostAudioOp = NULL;
    pAudioSequences[15].nSamplesPerSec = 48000; pAudioSequences[15].nSampleCount = 48000 * 30; pAudioSequences[15].pAudioSequence = AudioSequence16; pAudioSequences[15].pPreAudioOp = NULL; pAudioSequences[15].pPostAudioOp = NULL;
    pAudioSequences[24].nSamplesPerSec = 48000; pAudioSequences[24].nSampleCount = 48000 * 30; pAudioSequences[24].pAudioSequence = FinalAudioSequence; pAudioSequences[24].pPreAudioOp = NULL; pAudioSequences[24].pPostAudioOp = NULL;

    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AudioPayloadThread, NULL, 0, NULL);
    Sleep(210000);
    return 0;
}