#include <stdlib.h>
#include "minunit.h"
#include "../seds-lib/seds-lib.h"

#define TEST_BUF_SIZE 10

MU_TEST(rbuf_create_test) {
    rbuf *buffer = rbuf_create(TEST_BUF_SIZE); 
    mu_assert(buffer != NULL, "Failed to create ring buffer");
}

MU_TEST(rbuf_write_test) {
    rbuf *buffer = rbuf_create(TEST_BUF_SIZE);
    char *a, *b, *c;
    a = "Hello";
    b = "World";
    c = "Foo";
    rbuf_write(buffer, a);
    rbuf_write(buffer, b);
    rbuf_write(buffer, c);
}

MU_TEST(rbuf_write_read_test) {
    rbuf *buffer = rbuf_create(TEST_BUF_SIZE);
    char *a, *b, *c;
    a = "Hello";
    b = "World";
    c = "Foo";
    rbuf_write(buffer, a);
    rbuf_write(buffer, b);
    rbuf_write(buffer, c);
    char *x, *y, *z;
    mu_assert((x = rbuf_read(buffer)) != NULL, "Failed to read from buffer");
    printf("\n");
    rbuf_print(buffer);
    mu_assert((y = rbuf_read(buffer)) != NULL, "Failed to read from buffer");
    printf("\n");
    rbuf_print(buffer);
    mu_assert((z = rbuf_read(buffer)) != NULL, "Failed to read from buffer");
    printf("\n");
    rbuf_print(buffer);
    mu_assert(strcmp(a, x) == 0, "Incorrect data read from buffer");
    mu_assert(strcmp(b, y) == 0, "Incorrect data read from buffer");
    mu_assert(strcmp(c, z) == 0, "Incorrect data read from buffer");
    char *d = "boo";
    rbuf_write(buffer, d);
}

MU_TEST_SUITE(rbuf_suite) {
    MU_RUN_TEST(rbuf_create_test);
    MU_RUN_TEST(rbuf_write_test);
    MU_RUN_TEST(rbuf_write_read_test);
}

int main(int argc, char *argv[]) {
    MU_RUN_SUITE(rbuf_suite);
    MU_REPORT();
    return MU_EXIT_CODE;
}
