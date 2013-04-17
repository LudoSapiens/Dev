/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_WINDOWS_H
#define BASE_WINDOWS_H

/**
 * Includes <windows.h>, then undefines a bunch of stuff we don't want.
 */

#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
//#  undef CONST // This one is required for Direct3D.
#  undef DELETE
#  undef DIFFERENCE
#  undef IN
#  undef OUT

#else
#  error Trying to include <windows.h> on non-Windows platform.
#endif

#endif //BASE_WINDOWS_H
