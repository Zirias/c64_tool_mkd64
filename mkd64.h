#ifndef MKD64_H
#define MKD64_H

#include <mkd64/mkd64.h>
#include <mkd64/image.h>
#include "cmdline.h"

int mkd64_init(int argc, char **argv);
int mkd64_run(void);
void mkd64_done(void);

Image *mkd64_image(void);
Cmdline *mkd64_cmdline(void);

#endif

/* vim: et:si:ts=4:sts=4:sw=4
*/
