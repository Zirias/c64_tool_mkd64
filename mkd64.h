#ifndef MKD64_H
#define MKD64_H

#include <mkd64/mkd64.h>
#include <mkd64/image.h>
#include "cmdline.h"

Mkd64 *Mkd64_init(Mkd64 *self, int argc, char **argv);
int Mkd64_run(Mkd64 *self);
void Mkd64_done(Mkd64 *self);

Image *Mkd64_image(Mkd64 *self);
Cmdline *Mkd64_cmdline(Mkd64 *self);

#endif

/* vim: et:si:ts=4:sts=4:sw=4
*/
