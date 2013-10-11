#include "mini_stdint.h"

#define WEAK __attribute__((weak))

extern "C" {

WEAK void halide_shutdown_thread_pool() {
}

WEAK int (*halide_custom_do_task)(int (*)(int, uint8_t *), int, uint8_t *);
WEAK void halide_set_custom_do_task(int (*f)(int (*)(int, uint8_t *), int, uint8_t *)) {
    halide_custom_do_task = f;
}

WEAK int (*halide_custom_do_par_for)(int (*)(int, uint8_t *), int, int, uint8_t *);
WEAK void halide_set_custom_do_par_for(int (*f)(int (*)(int, uint8_t *), int, int, uint8_t *)) {
    halide_custom_do_par_for = f;
}

WEAK int halide_do_task(int (*f)(int, uint8_t *), int idx, uint8_t *closure) {
    if (halide_custom_do_task) {
        return (*halide_custom_do_task)(f, idx, closure);
    } else {
        return f(idx, closure);
    }
}

WEAK int halide_do_par_for(int (*f)(int, uint8_t *), int min, int size, uint8_t *closure) {
    if (halide_custom_do_par_for) {
        return (*halide_custom_do_par_for)(f, min, size, closure);
    }

    for (int x = min; x < min + size; x++) {
        int result = halide_do_task(f, x, closure);
        if (result) {
            return result;
        }
    }
    return 0;
}

}
