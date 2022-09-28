/*
 * Original code from
 * https://github.com/olebole/wcstools/blob/52b63bbae90ace3cc4ac0fc86ca148b440bf593d/imresize.c By
 * Jessica Mink, Harvard-Smithsonian Center for Astrophysics
 */

#ifndef IMRESIZE_H
#define IMRESIZE_H

extern "C" {
void imresize(char *name, int resizeFactor, double ghwidth, int kernelSize = 3);
}
#endif // IMRESIZE_H
