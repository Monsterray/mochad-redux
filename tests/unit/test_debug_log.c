#include "global.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
    FILE *capture = tmpfile();
    int original_stderr;

    assert(capture != NULL);
    original_stderr = dup(STDERR_FILENO);
    assert(original_stderr >= 0);
    assert(dup2(fileno(capture), STDERR_FILENO) >= 0);

    dbprintf_set_enabled(0);
    assert(dbprintf("hidden\n") == 0);
    fflush(stderr);
    assert(ftell(capture) == 0);

    dbprintf_set_enabled(1);
    assert(dbprintf("visible\n") > 0);
    fflush(stderr);
    assert(ftell(capture) > 0);

    assert(dup2(original_stderr, STDERR_FILENO) >= 0);
    close(original_stderr);
    fclose(capture);
    return 0;
}
