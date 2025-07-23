#ifndef UTIL_HEADER
#define UTIL_HEADER

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <unistd.h>

#include "unif01.h"
#include "bbattery.h"

unsigned int random31() {
    return (unsigned int)random();
}

unsigned int random32() {
    return ((unsigned int)random() << 1) ^ (random() & 1);
}

unsigned int random32v2() {
    unsigned int h = random() & 0xFFFF;
    unsigned int l = random() & 0xFFFF;
    return (h << 16) | l;
}

double random_double() {
    return (double)random() / 2147483648.0;
}

void smallcrush_bits(char * name, unsigned int (*gen)(void)) {
    unif01_Gen *g;
    g = unif01_CreateExternGenBits(name, gen);
    bbattery_SmallCrush(g);
    unif01_DeleteExternGenBits(g);
}

void crush_bits(char * name, unsigned int (*gen)(void)) {
    unif01_Gen *g;
    g = unif01_CreateExternGenBits(name, gen);
    bbattery_Crush(g);
    unif01_DeleteExternGenBits(g);
}

void bigcrush_bits(char * name, unsigned int (*gen)(void)) {
    unif01_Gen *g;
    g = unif01_CreateExternGenBits(name, gen);
    bbattery_BigCrush(g);
    unif01_DeleteExternGenBits(g);
}


void smallcrush_double(char * name, double (*gen)(void)) {
    unif01_Gen *g;
    g = unif01_CreateExternGen01(name, gen);
    bbattery_SmallCrush(g);
    unif01_DeleteExternGen01(g);
}

void crush_double(char * name, double (*gen)(void)) {
    unif01_Gen *g;
    g = unif01_CreateExternGen01(name, gen);
    bbattery_Crush(g);
    unif01_DeleteExternGen01(g);
}

void bigcrush_double(char * name, double (*gen)(void)) {
    unif01_Gen *g;
    g = unif01_CreateExternGen01(name, gen);
    bbattery_BigCrush(g);
    unif01_DeleteExternGen01(g);
}

void seed() {
    srandom((unsigned int) time(NULL));
}

#endif
