#include "test_ifc_lib_gen.h"
#include "test_ifc_lib_symphony.h"
#include "test_ifc_lib_nomac.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmockery.h"


int main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    UnitTest tests[] = {
        GEN_IFC_LIB_TESTS
        SYMPHONY_IFC_LIB_TESTS
        NOMAC_IFC_LIB_TESTS
    };

    return run_tests(tests);
}
