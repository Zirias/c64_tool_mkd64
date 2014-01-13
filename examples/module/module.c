#include <mkd64/common.h>
#include <mkd64/imodule.h>

MKD64_MODULE("example")

#define MODVERSION "1.0"

typedef struct
{
    IModule mod;
    /* add runtime properties here */
} Module;

static void
delete(IModule *this)
{
    Module *mod = (Module *)this;

    /* add destructor code here, free all memory allocated in instance() */

    free(mod);
}

SOEXPORT IModule *
instance(void)
{
    Module *mod = calloc(1, sizeof(Module));
    mod->mod.id = &id;
    mod->mod.free = &delete;

    /* add more runtime methods as needed, see imodule.h and modapi.txt */

    return (IModule *) mod;
}

SOEXPORT const char *
help(void)
{
    return
"examplemod is an mkd64 module.\n";

    /* add description of global options understood by your module */
}

/*
 * use this if your module provides file options:
SOEXPORT const char *
helpFile(void)
{
    return
"  -o [ARGNAME]  This option does something with {ARGNAME}\n";
}
*/

SOEXPORT const char *
versionInfo(void)
{
    return
"example " MODVERSION "\n";

    /* add descriptive text, maybe some kind of build id. */
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
