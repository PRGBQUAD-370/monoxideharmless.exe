#include "monoxide.h"

VOID
GetRandomPath(
	_Inout_ PWSTR szRandom,
	_In_ INT nLength
)
{
	for ( INT i = 0; i < nLength; i++ )
	{
		szRandom[ i ] = ( WCHAR )( Xorshift32( ) % ( 0x9FFF - 0x4E00 + 1 ) + 0x4E00 );
	}
}

VOID
WINAPI
CursorDraw( VOID )
{
	CURSORINFO curInf = { sizeof( CURSORINFO ) };

	for ( ;; )
	{
		GetCursorInfo( &curInf );

		for ( INT i = 0; i < ( INT )( Xorshift32( ) % 5 + 1 ); i++ )
		{
			DrawIcon( hdcDesktop, Xorshift32( ) % ( rcScrBounds.right - rcScrBounds.left - GetSystemMetrics( SM_CXCURSOR ) ) - rcScrBounds.left,
				Xorshift32( ) % ( rcScrBounds.bottom - rcScrBounds.top - GetSystemMetrics( SM_CYCURSOR ) ) - rcScrBounds.top, curInf.hCursor );
		}
		DestroyCursor( curInf.hCursor );
		Sleep( Xorshift32( ) % 11 );
	}
}