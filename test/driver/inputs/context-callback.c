#include "context-callback.h"

void run_callback(void (*callback)(void)) {
    callback();
}
