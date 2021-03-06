#pragma once

#include <stdio.h>
#include "kuroko.h"

typedef struct Obj KrkObj;
typedef struct ObjString KrkString;

typedef enum {
	VAL_NONE,
	VAL_BOOLEAN,
	VAL_INTEGER,
	VAL_FLOATING,
	VAL_HANDLER,
	VAL_OBJECT,
	VAL_KWARGS,
	/* More here later */
} KrkValueType;

typedef struct {
	unsigned short type;
	unsigned short target;
} KrkJumpTarget;

typedef struct {
	KrkValueType type;
	union {
		char boolean;
		krk_integer_type integer;
		double  floating;
		KrkJumpTarget handler;
		KrkObj *   object;
	} as;
} KrkValue;

#define BOOLEAN_VAL(value)  ((KrkValue){VAL_BOOLEAN, {.boolean = value}})
#define NONE_VAL(value)     ((KrkValue){VAL_NONE,    {.integer = 0}})
#define INTEGER_VAL(value)  ((KrkValue){VAL_INTEGER, {.integer = value}})
#define FLOATING_VAL(value) ((KrkValue){VAL_FLOATING,{.floating = value}})
#define HANDLER_VAL(ty,ta)  ((KrkValue){VAL_HANDLER, {.handler = (KrkJumpTarget){.type = ty, .target = ta}}})
#define OBJECT_VAL(value)   ((KrkValue){VAL_OBJECT,  {.object = (KrkObj*)value}})
#define KWARGS_VAL(value)   ((KrkValue){VAL_KWARGS,  {.integer = value}})

#define AS_BOOLEAN(value)   ((value).as.boolean)
#define AS_INTEGER(value)   ((value).as.integer)
#define AS_FLOATING(value)  ((value).as.floating)
#define AS_HANDLER(value)   ((value).as.handler)
#define AS_OBJECT(value)    ((value).as.object)

#define IS_BOOLEAN(value)   ((value).type == VAL_BOOLEAN)
#define IS_NONE(value)      ((value).type == VAL_NONE)
#define IS_INTEGER(value)   ((value).type == VAL_INTEGER)
#define IS_FLOATING(value)  ((value).type == VAL_FLOATING)
#define IS_HANDLER(value)   ((value).type == VAL_HANDLER)
#define IS_OBJECT(value)    ((value).type == VAL_OBJECT)
#define IS_KWARGS(value)    ((value).type == VAL_KWARGS)

#define IS_TRY_HANDLER(value)  (IS_HANDLER(value) && AS_HANDLER(value).type == OP_PUSH_TRY)
#define IS_WITH_HANDLER(value) (IS_HANDLER(value) && AS_HANDLER(value).type == OP_PUSH_WITH)

typedef struct {
	size_t capacity;
	size_t count;
	KrkValue * values;
} KrkValueArray;

extern void krk_initValueArray(KrkValueArray * array);
extern void krk_writeValueArray(KrkValueArray * array, KrkValue value);
extern void krk_freeValueArray(KrkValueArray * array);
extern void krk_printValue(FILE * f, KrkValue value);
extern void krk_printValueSafe(FILE * f, KrkValue value);
extern int krk_valuesEqual(KrkValue a, KrkValue b);
extern int krk_valuesSame(KrkValue a, KrkValue b);

