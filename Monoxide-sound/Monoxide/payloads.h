#pragma once
#include "monoxide.h"

#define TIMER_DELAY 100
#define PAYLOAD_MS 10000
#define PAYLOAD_TIME ( PAYLOAD_MS / TIMER_DELAY )

#define SineWave(t, freq, sampleCount) FastSine(2.f * 3.1415f * ((FLOAT)(freq) * (FLOAT)(t) / (FLOAT)(sampleCount)))
#define SquareWave(t, freq, sampleCount) (((BYTE)(2.f * (FLOAT)(freq) * ((t) / (FLOAT)(sampleCount))) % 2) == 0 ? 1.f : -1.f)
#define TriangleWave(t, freq, sampleCount) (4.f * ( FLOAT )fabs(((FLOAT)(t) / ((FLOAT)(sampleCount) / (FLOAT)(freq))) - floor(((FLOAT)(t) / ((FLOAT)(sampleCount) / (FLOAT)(freq)))) - .5f) - 1.f)
#define SawtoothWave(t, freq, sampleCount) (fmod(((FLOAT)(t) / (FLOAT)(sampleCount)), (1.f / (FLOAT)(freq))) * (FLOAT)(freq) * 2.f - 1.f)

VOID
WINAPI
BeepEx(
	_In_ INT nWaveformIndex,
	_In_ INT nDuration,
	_In_ PSHORT psFreq,
	_In_ PFLOAT pfVolume
);

VOID
WINAPI
AudioPayloadThread( VOID );

VOID
WINAPI
AudioSequenceThread(
	_In_ PAUDIO_SEQUENCE_PARAMS pAudioParams
);

VOID
WINAPI
ExecuteAudioSequence(
	_In_ INT nSamplesPerSec,
	_In_ INT nSampleCount,
	_In_ AUDIO_SEQUENCE pAudioSequence,
	_In_opt_ AUDIOSEQUENCE_OPERATION pPreSynthOp,
	_In_opt_ AUDIOSEQUENCE_OPERATION pPostSynthOp
);

#define SYNTH_LENGTH 16
AUDIO_SEQUENCE AudioSequence1;
AUDIO_SEQUENCE AudioSequence2;
AUDIO_SEQUENCE AudioSequence3;
AUDIO_SEQUENCE AudioSequence4;
AUDIO_SEQUENCE AudioSequence5;
AUDIO_SEQUENCE AudioSequence6;
AUDIO_SEQUENCE AudioSequence7;
AUDIO_SEQUENCE AudioSequence8;
AUDIO_SEQUENCE AudioSequence9;
AUDIO_SEQUENCE AudioSequence10;
AUDIO_SEQUENCE AudioSequence11;
AUDIO_SEQUENCE AudioSequence12;
AUDIO_SEQUENCE AudioSequence13;
AUDIO_SEQUENCE AudioSequence14;
AUDIO_SEQUENCE AudioSequence15;
AUDIO_SEQUENCE AudioSequence16;
AUDIO_SEQUENCE FinalAudioSequence;
AUDIO_SEQUENCE_PARAMS pAudioSequences[ 25 ];