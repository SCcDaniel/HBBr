/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2023 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef SDL_vitagles_c_h_
#define SDL_vitagles_c_h_

#include <pib.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "SDL_vitavideo.h"

typedef struct SDL_GLDriverData
{
    EGLDisplay display;
    EGLContext context;
    EGLSurface surface;
    uint32_t swapinterval;
} SDL_GLDriverData;

extern SDL_FunctionPointer VITA_GLES_GetProcAddress(SDL_VideoDevice *_this, const char *proc);
extern int VITA_GLES_MakeCurrent(SDL_VideoDevice *_this, SDL_Window *window, SDL_GLContext context);
extern void VITA_GLES_SwapBuffers(SDL_VideoDevice *_this);

extern int VITA_GLES_SwapWindow(SDL_VideoDevice *_this, SDL_Window *window);
extern SDL_GLContext VITA_GLES_CreateContext(SDL_VideoDevice *_this, SDL_Window *window);

extern int VITA_GLES_LoadLibrary(SDL_VideoDevice *_this, const char *path);
extern void VITA_GLES_UnloadLibrary(SDL_VideoDevice *_this);
extern int VITA_GLES_SetSwapInterval(SDL_VideoDevice *_this, int interval);
extern int VITA_GLES_GetSwapInterval(SDL_VideoDevice *_this, int *interval);

#endif /* SDL_vitagles_c_h_ */
