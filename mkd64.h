#ifndef MKD64_H
#define MKD64_H

#include <mkd64/mkd64.h>
#include <mkd64/image.h>
#include "cmdline.h"

Mkd64 *Mkd64_init(Mkd64 *this, int argc, char **argv);
int Mkd64_run(Mkd64 *this);
void Mkd64_done(Mkd64 *this);

Image *Mkd64_image(Mkd64 *this);
Cmdline *Mkd64_cmdline(Mkd64 *this);

#endif

/* vim: et:si:ts=4:sts=4:sw=4
*/
