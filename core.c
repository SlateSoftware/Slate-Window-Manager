#include "core.h"

FILE* logstream = NULL;

bool core__init_log_stream(void)
{
    logstream = fopen("sdewm.log", "w");
    if (!logstream)
        return false;

    else
    {
        fprintf(logstream, "== Slate Window Manager, v%s ==\n", SDEWM_VERSION);
        return true;
    }
}

void core__close_log_stream(void)
{
    fprintf(logstream, "== Quitting SDEWM ==\n");
    fclose(logstream);
}