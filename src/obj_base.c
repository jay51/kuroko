#include <string.h>
#include "vm.h"
#include "value.h"
#include "memory.h"
#include "util.h"

static KrkValue _type_init(int argc, KrkValue argv[]) {
	if (argc != 2) return krk_runtimeError(vm.exceptions->argumentError, "type() takes 1 argument");
	return OBJECT_VAL(krk_getType(argv[1]));
}

/* Class.__base__ */
static KrkValue krk_baseOfClass(int argc, KrkValue argv[]) {
	if (!IS_CLASS(argv[0])) return krk_runtimeError(vm.exceptions->typeError, "expected class");
	return AS_CLASS(argv[0])->base ? OBJECT_VAL(AS_CLASS(argv[0])->base) : NONE_VAL();
}

/* Class.__name */
static KrkValue krk_nameOfClass(int argc, KrkValue argv[]) {
	if (!IS_CLASS(argv[0])) return krk_runtimeError(vm.exceptions->typeError, "expected class");
	return AS_CLASS(argv[0])->name ? OBJECT_VAL(AS_CLASS(argv[0])->name) : NONE_VAL();
}

/* Class.__file__ */
static KrkValue krk_fileOfClass(int argc, KrkValue argv[]) {
	if (!IS_CLASS(argv[0])) return krk_runtimeError(vm.exceptions->typeError, "expected class");
	return AS_CLASS(argv[0])->filename ? OBJECT_VAL(AS_CLASS(argv[0])->filename) : NONE_VAL();
}

/* Class.__doc__ */
static KrkValue krk_docOfClass(int argc, KrkValue argv[]) {
	if (!IS_CLASS(argv[0])) return krk_runtimeError(vm.exceptions->typeError, "expected class");
	return AS_CLASS(argv[0])->docstring ? OBJECT_VAL(AS_CLASS(argv[0])->docstring) : NONE_VAL();
}

/* Class.__str__() (and Class.__repr__) */
static KrkValue _class_to_str(int argc, KrkValue argv[]) {
	if (!IS_CLASS(argv[0])) return krk_runtimeError(vm.exceptions->typeError, "expected class");
	char * tmp = malloc(sizeof("<type ''>") + AS_CLASS(argv[0])->name->length);
	size_t l = sprintf(tmp, "<type '%s'>", AS_CLASS(argv[0])->name->chars);
	KrkString * out = krk_copyString(tmp,l);
	free(tmp);
	return OBJECT_VAL(out);
}

void _createAndBind_type(void) {
	ADD_BASE_CLASS(vm.baseClasses->typeClass, "type", vm.baseClasses->objectClass);
	krk_defineNative(&vm.baseClasses->typeClass->methods, ":__base__", krk_baseOfClass);
	krk_defineNative(&vm.baseClasses->typeClass->methods, ":__file__", krk_fileOfClass);
	krk_defineNative(&vm.baseClasses->typeClass->methods, ":__doc__", krk_docOfClass);
	krk_defineNative(&vm.baseClasses->typeClass->methods, ":__name__", krk_nameOfClass);
	krk_defineNative(&vm.baseClasses->typeClass->methods, ".__init__", _type_init);
	krk_defineNative(&vm.baseClasses->typeClass->methods, ".__str__", _class_to_str);
	krk_defineNative(&vm.baseClasses->typeClass->methods, ".__repr__", _class_to_str);
	krk_finalizeClass(vm.baseClasses->typeClass);
	vm.baseClasses->typeClass->docstring = S("Obtain the object representation of the class of an object.");
}
