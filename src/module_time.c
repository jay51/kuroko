/**
 * Currently just sleep().
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#include "vm.h"
#include "value.h"
#include "object.h"

#define S(c) (krk_copyString(c,sizeof(c)-1))

/**
 * system.sleep(seconds)
 */
static KrkValue _time_sleep(int argc, KrkValue argv[]) {
	if (argc < 1) {
		krk_runtimeError(vm.exceptions->argumentError, "sleep: expect at least one argument.");
		return BOOLEAN_VAL(0);
	}

	/* Accept an integer or a floating point. Anything else, just ignore. */
	unsigned int usecs = (IS_INTEGER(argv[0]) ? AS_INTEGER(argv[0]) :
	                      (IS_FLOATING(argv[0]) ? AS_FLOATING(argv[0]) : 0)) *
	                      1000000;

	usleep(usecs);

	return BOOLEAN_VAL(1);
}

static KrkValue _time_time(int argc, KrkValue argv[]) {
	time_t out = time(NULL);
	return FLOATING_VAL(out); /* TODO actually support subsecond values */
}

KrkValue krk_module_onload_time(void) {
	KrkInstance * module = krk_newInstance(vm.baseClasses->moduleClass);
	/* Store it on the stack for now so we can do stuff that may trip GC
	 * and not lose it to garbage colletion... */
	krk_push(OBJECT_VAL(module));

	krk_defineNative(&module->fields, "sleep", _time_sleep);
	krk_defineNative(&module->fields, "time", _time_time);

	/* Pop the module object before returning; it'll get pushed again
	 * by the VM before the GC has a chance to run, so it's safe. */
	assert(AS_INSTANCE(krk_pop()) == module);
	return OBJECT_VAL(module);
}

