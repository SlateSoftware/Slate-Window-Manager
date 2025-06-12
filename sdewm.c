#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <intdef.h>

static bool wm_detecetd = false;

int wm_error_handler(Display* dpy, XErrorEvent* ev)
{
    if (ev->error_code == BadAccess)
        wm_detecetd = true;

    return 0;
}

int main(void) 
{
    Display* display = XOpenDisplay(NULL);
    if (!display)
    {
        fprintf(stderr, "error: cannot open display\n");
        return EXIT_FAILURE;
    }

    Window root = DefaultRootWindow(display);
    XSetErrorHandler(wm_error_handler);
    XSelectInput(display, root, SubstructureRedirectMask);
    XSync(display, false);
    if (wm_detecetd)
    {
        fprintf(stderr, "error: another window manager is already running\n");
        return EXIT_FAILURE;
    }
    printf("sdewm: running\n");
}