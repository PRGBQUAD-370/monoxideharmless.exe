#pragma once
#include "monoxide.h"

VOID
GetRandomPath(
    _Inout_ PWSTR szRandom,
    _In_ INT nLength
);

VOID
WINAPI
CursorDraw( VOID );