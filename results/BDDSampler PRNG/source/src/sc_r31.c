#include "util.h"

int main(void) {
    seed();

    smallcrush_bits("random31", random31);
    /*smallcrush_bits("random32", random32);*/
    /*smallcrush_bits("random32v2", random32v2);*/
    /*smallcrush_double("random_double", random_double);*/

    /*crush_bits("random31", random31);*/
    /*crush_bits("random32", random32);*/
    /*crush_bits("random32v2", random32v2);*/
    /*crush_double("random_double", random_double);*/

    /*bigcrush_bits("random31", random31);*/
    /*bigcrush_bits("random32", random32);*/
    /*bigcrush_bits("random32v2", random32v2);*/
    /*bigcrush_double("random_double", random_double);*/
    return 0;
}
