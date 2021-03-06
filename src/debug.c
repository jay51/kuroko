#include <stdio.h>

#include "debug.h"
#include "vm.h"

void krk_disassembleChunk(FILE * f, KrkFunction * func, const char * name) {
	KrkChunk * chunk = &func->chunk;
	fprintf(f, "[%s from %s]\n", name, chunk->filename->chars);
	for (size_t offset = 0; offset < chunk->count;) {
		offset = krk_disassembleInstruction(f, func, offset);
	}
}

static inline const char * opcodeClean(const char * opc) {
	return &opc[3];
}

#define SIMPLE(opc) case opc: fprintf(f, "%-16s      ", opcodeClean(#opc)); size = 1; break;
#define CONSTANT(opc,more) case opc: { size_t constant = chunk->code[offset + 1]; \
	fprintf(f, "%-16s %4d ", opcodeClean(#opc), (int)constant); \
	krk_printValueSafe(f, chunk->constants.values[constant]); \
	fprintf(f," (type=%s)", krk_typeName(chunk->constants.values[constant])); \
	more; \
	size = 2; break; } \
	case opc ## _LONG: { size_t constant = (chunk->code[offset + 1] << 16) | \
	(chunk->code[offset + 2] << 8) | (chunk->code[offset + 3]); \
	fprintf(f, "%-16s %4d ", opcodeClean(#opc "_LONG"), (int)constant); \
	krk_printValueSafe(f, chunk->constants.values[constant]); \
	fprintf(f," (type=%s)", krk_typeName(chunk->constants.values[constant])); \
	more; size = 4; break; }
#define OPERANDB(opc,more) case opc: { uint32_t operand = chunk->code[offset + 1]; \
	fprintf(f, "%-16s %4d", opcodeClean(#opc), (int)operand); \
	more; size = 2; break; }
#define OPERAND(opc,more) OPERANDB(opc,more) \
	case opc ## _LONG: { uint32_t operand = (chunk->code[offset + 1] << 16) | \
	(chunk->code[offset + 2] << 8) | (chunk->code[offset + 3]); \
	fprintf(f, "%-16s %4d", opcodeClean(#opc "_LONG"), (int)operand); \
	more; fprintf(f,"\n"); \
	size = 4; break; }
#define JUMP(opc,sign) case opc: { uint16_t jump = (chunk->code[offset + 1] << 8) | \
	(chunk->code[offset + 2]); \
	fprintf(f, "%-16s %4d -> %d", opcodeClean(#opc), (int)offset, (int)(offset + 3 sign jump)); \
	size = 3; break; }

#define CLOSURE_MORE \
	KrkFunction * function = AS_FUNCTION(chunk->constants.values[constant]); \
	for (size_t j = 0; j < function->upvalueCount; ++j) { \
		int isLocal = chunk->code[offset++ + 2]; \
		int index = chunk->code[offset++ + 2]; \
		fprintf(f, " (%d %s %d)", (int)offset - 2, isLocal ? "local" : "upvalue", index); \
	}

#define EXPAND_ARGS_MORE \
	fprintf(f, " (%s)", operand == 0 ? "singleton" : (operand == 1 ? "list" : "dict"));

#define LOCAL_MORE \
	if ((short int)operand < (func->requiredArgs)) { \
		fprintf(f, " (%s, arg)", AS_CSTRING(func->requiredArgNames.values[operand])); \
	} else if ((short int)operand < (func->requiredArgs + func->keywordArgs)) { \
		fprintf(f, " (%s, kwarg))", AS_CSTRING(func->keywordArgNames.values[operand-func->requiredArgs])); \
	} else { \
		for (size_t i = 0; i < func->localNameCount; ++i) { \
			if (func->localNames[i].id == operand && func->localNames[i].birthday <= offset && func->localNames[i].deathday >= offset) { \
				fprintf(f, " (%s)", func->localNames[i].name->chars); \
				break; \
			} \
		} \
	}

size_t krk_lineNumber(KrkChunk * chunk, size_t offset) {
	size_t line = 0;
	for (size_t i = 0; i < chunk->linesCount; ++i) {
		if (chunk->lines[i].startOffset > offset) break;
		line = chunk->lines[i].line;
	}
	return line;
}

size_t krk_disassembleInstruction(FILE * f, KrkFunction * func, size_t offset) {
	KrkChunk * chunk = &func->chunk;
	fprintf(f, "%04u ", (unsigned int)offset);
	if (offset > 0 && krk_lineNumber(chunk, offset) == krk_lineNumber(chunk, offset - 1)) {
		fprintf(f, "   | ");
	} else {
		fprintf(f, "%4d ", (int)krk_lineNumber(chunk, offset));
	}
	uint8_t opcode = chunk->code[offset];
	size_t size = 1;

	switch (opcode) {
		SIMPLE(OP_RETURN)
		SIMPLE(OP_ADD)
		SIMPLE(OP_SUBTRACT)
		SIMPLE(OP_MULTIPLY)
		SIMPLE(OP_DIVIDE)
		SIMPLE(OP_NEGATE)
		SIMPLE(OP_MODULO)
		SIMPLE(OP_NONE)
		SIMPLE(OP_TRUE)
		SIMPLE(OP_FALSE)
		SIMPLE(OP_NOT)
		SIMPLE(OP_EQUAL)
		SIMPLE(OP_GREATER)
		SIMPLE(OP_LESS)
		SIMPLE(OP_POP)
		SIMPLE(OP_INHERIT)
		SIMPLE(OP_RAISE)
		SIMPLE(OP_CLOSE_UPVALUE)
		SIMPLE(OP_DOCSTRING)
		SIMPLE(OP_CALL_STACK)
		SIMPLE(OP_BITOR)
		SIMPLE(OP_BITXOR)
		SIMPLE(OP_BITAND)
		SIMPLE(OP_SHIFTLEFT)
		SIMPLE(OP_SHIFTRIGHT)
		SIMPLE(OP_BITNEGATE)
		SIMPLE(OP_INVOKE_GETTER)
		SIMPLE(OP_INVOKE_SETTER)
		SIMPLE(OP_INVOKE_DELETE)
		SIMPLE(OP_INVOKE_GETSLICE)
		SIMPLE(OP_INVOKE_SETSLICE)
		SIMPLE(OP_INVOKE_DELSLICE)
		SIMPLE(OP_SWAP)
		SIMPLE(OP_FINALIZE)
		SIMPLE(OP_IS)
		SIMPLE(OP_POW)
		SIMPLE(OP_CREATE_PROPERTY)
		OPERANDB(OP_DUP,(void)0)
		OPERANDB(OP_EXPAND_ARGS,EXPAND_ARGS_MORE)
		CONSTANT(OP_DEFINE_GLOBAL,(void)0)
		CONSTANT(OP_CONSTANT,(void)0)
		CONSTANT(OP_GET_GLOBAL,(void)0)
		CONSTANT(OP_SET_GLOBAL,(void)0)
		CONSTANT(OP_DEL_GLOBAL,(void)0)
		CONSTANT(OP_CLASS,(void)0)
		CONSTANT(OP_GET_PROPERTY, (void)0)
		CONSTANT(OP_SET_PROPERTY, (void)0)
		CONSTANT(OP_DEL_PROPERTY,(void)0)
		CONSTANT(OP_METHOD, (void)0)
		CONSTANT(OP_CLOSURE, CLOSURE_MORE)
		CONSTANT(OP_IMPORT, (void)0)
		CONSTANT(OP_IMPORT_FROM, (void)0)
		CONSTANT(OP_GET_SUPER, (void)0)
		OPERAND(OP_KWARGS, (void)0)
		OPERAND(OP_SET_LOCAL, LOCAL_MORE)
		OPERAND(OP_GET_LOCAL, LOCAL_MORE)
		OPERAND(OP_SET_UPVALUE, (void)0)
		OPERAND(OP_GET_UPVALUE, (void)0)
		OPERAND(OP_CALL, (void)0)
		OPERAND(OP_INC, (void)0)
		OPERAND(OP_TUPLE, (void)0)
		OPERAND(OP_UNPACK, (void)0)
		JUMP(OP_JUMP,+)
		JUMP(OP_JUMP_IF_FALSE,+)
		JUMP(OP_JUMP_IF_TRUE,+)
		JUMP(OP_LOOP,-)
		JUMP(OP_PUSH_TRY,+)
		JUMP(OP_PUSH_WITH,+)
		SIMPLE(OP_CLEANUP_WITH)
		default:
			fprintf(f, "Unknown opcode: %02x", opcode);
	}

	/* Birthdays - Local names that have become valid from this instruction */
	for (size_t i = 0; i < func->localNameCount; ++i) {
		if (func->localNames[i].birthday >= offset && func->localNames[i].birthday < offset + size) {
			fprintf(f, " +%s", func->localNames[i].name->chars);
		}
	}

	/* Deathdays - Local names that are no longer valid as of this instruction */
	for (size_t i = 0; i < func->localNameCount; ++i) {
		if (func->localNames[i].deathday >= offset && func->localNames[i].deathday < offset + size) {
			fprintf(f, " -%s", func->localNames[i].name->chars);
		}
	}

	fprintf(f,"\n");

	return offset + size;
}

