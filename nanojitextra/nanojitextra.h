#ifndef __nanojit_extra__
#define __nanojit_extra__

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  NJXOpIntKind = 1,
  NJXOpQuadKind = 2,
  NJXOpPtrKind = 2,
  NJXOpDoubleKind = 3,
  NJXOpFloatKind = 4,
} NJXOpKind;

typedef struct NJXLIns *NJXLInsRef;

/**
* The Jit Context defines a container for the Jit machinery and
* also acts as the repository of the compiled functions. The Jit
* Context must be kept alive as long as any functions within it are
* needed. Deleting the Jit Context will also delete all compiled
* functions managed by the context.
*/
typedef struct NJXContext *NJXContextRef;

typedef struct NFXFunctionBuilder *NJXFunctionBuilderRef;

typedef int64_t NJXParamType;

/**
* Creates a Jit Context. If versbose is true then during code
* generation verbose output is generated.
*/
extern NJXContextRef NJX_create_context(int verbose);

/**
* Destroys the Jit Context. Note that all compiled functions
* managed by this context will die at this point.
*/
extern void NJX_destroy_context(NJXContextRef);

/**
* Returns a compiled function looking it up by name.
* The point must be cast to the correct signature.
*/
extern void *NJX_get_function_by_name(const char *name);

/**
* Creates a new FunctionBuilder object. The builder is used to construct the
* code that will go into one function. Once the function has been defined,
* it can be compiled by calling finalize(). After the function is compiled the
* builder object can be thrown away - the compiled function will live as long as
* the
* owning Jit Context lives.
* If optimize flag is true then NanoJit's CSE and Expr filters are enabled.
*/
extern NJXFunctionBuilderRef NJX_create_function_builder(NJXContextRef context,
                                                         const char *name,
                                                         int optimize);

/**
* Destroys the FunctionBuilder object. Note that this will not delete the
* compiled function created using this builder - as the compiled function lives
* in the owning Jit Context.
*/
extern void NJX_destroy_function_builder(NJXFunctionBuilderRef);

/**
* Adds an integer return instruction.
*/
extern NJXLInsRef NJX_reti(NJXFunctionBuilderRef fn, NJXLInsRef result);

/**
* Adds a double return instruction.
*/
extern NJXLInsRef NJX_retd(NJXFunctionBuilderRef fn, NJXLInsRef result);

/**
* Adds a quad return instruction.
*/
extern NJXLInsRef NJX_retq(NJXFunctionBuilderRef fn, NJXLInsRef result);

extern NJXLInsRef NJX_ret(NJXFunctionBuilderRef fn);

/**
* Creates an int32 constant
*/
extern NJXLInsRef NJX_immi(NJXFunctionBuilderRef fn, int32_t i);

/**
* Creates an int64 constant
*/
extern NJXLInsRef NJX_immq(NJXFunctionBuilderRef fn, int64_t q);

/**
* Creates a double constant
*/
extern NJXLInsRef NJX_immd(NJXFunctionBuilderRef fn, double d);

/**
* Creates a float constant
*/
extern NJXLInsRef NJX_immf(NJXFunctionBuilderRef fn, float f);

/**
* Adds a function parameter - the parameter size is always the
* default register size I think - so on a 64-bit machine it will be
* quads whereas on 32-bit machines it will be words. Caller must
* handle this and convert to type needed.
* This also means that only primitive values and pointers can be
* used as function parameters.
*/
extern NJXLInsRef NJX_insert_parameter(NJXFunctionBuilderRef fn);

/**
* Integer add
*/
extern NJXLInsRef NJX_addi(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_addq(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);

/**
* Floating point add
*/
extern NJXLInsRef NJX_addd(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_addf(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);

/**
* Tests lhs == rhs, result is 1 or 0
*/
extern NJXLInsRef NJX_eqi(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);
extern NJXLInsRef NJX_eqq(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);
extern NJXLInsRef NJX_eqd(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);
extern NJXLInsRef NJX_eqf(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);

/**
* Converts a quad to an int
*/
extern NJXLInsRef NJX_q2i(NJXFunctionBuilderRef fn, NJXLInsRef q);

/**
* Inserts a label at current position, no code is emitted for this
*/
extern NJXLInsRef NJX_add_label(NJXFunctionBuilderRef fn);

/**
* Allocates 'size' bytes on the stack
*/
extern NJXLInsRef NJX_alloca(NJXFunctionBuilderRef fn, int32_t size);

/**
* Inserts an unconditional jump - to can be NULL and set later
*/
extern NJXLInsRef NJX_br(NJXFunctionBuilderRef fn, NJXLInsRef to);

/**
* Inserts a conditional branch - jump targets can be NULL and set later
*/
extern NJXLInsRef NJX_cbr_true(NJXFunctionBuilderRef fn, NJXLInsRef cond,
                               NJXLInsRef to);
extern NJXLInsRef NJX_cbr_false(NJXFunctionBuilderRef fn, NJXLInsRef cond,
                                NJXLInsRef to);

/**
* Sets the target of a jump instruction
* target should be a label instruction
* and jmp should be the jump instruction returned by NJX_br(), NJX_cbr_true() or
* NJX_cbr_false().
*/
extern void NJX_set_jmp_target(NJXLInsRef jmp, NJXLInsRef target);

extern NJXLInsRef NJX_load_c2i(NJXFunctionBuilderRef fn, NJXLInsRef ptr,
                               int32_t offset);
extern NJXLInsRef NJX_load_uc2ui(NJXFunctionBuilderRef fn, NJXLInsRef ptr,
                                 int32_t offset);
extern NJXLInsRef NJX_load_s2i(NJXFunctionBuilderRef fn, NJXLInsRef ptr,
                               int32_t offset);
extern NJXLInsRef NJX_load_us2ui(NJXFunctionBuilderRef fn, NJXLInsRef ptr,
                                 int32_t offset);
extern NJXLInsRef NJX_load_i(NJXFunctionBuilderRef fn, NJXLInsRef ptr,
                             int32_t offset);
extern NJXLInsRef NJX_load_q(NJXFunctionBuilderRef fn, NJXLInsRef ptr,
                             int32_t offset);
extern NJXLInsRef NJX_load_f(NJXFunctionBuilderRef fn, NJXLInsRef ptr,
                             int32_t offset);
extern NJXLInsRef NJX_load_d(NJXFunctionBuilderRef fn, NJXLInsRef ptr,
                             int32_t offset);
extern NJXLInsRef NJX_load_f2d(NJXFunctionBuilderRef fn, NJXLInsRef ptr,
                               int32_t offset);

extern NJXLInsRef NJX_store_i2c(NJXFunctionBuilderRef fn, NJXLInsRef value,
                                NJXLInsRef ptr, int32_t offset);
extern NJXLInsRef NJX_store_i2s(NJXFunctionBuilderRef fn, NJXLInsRef value,
                                NJXLInsRef ptr, int32_t offset);
extern NJXLInsRef NJX_store_i(NJXFunctionBuilderRef fn, NJXLInsRef value,
                              NJXLInsRef ptr, int32_t offset);
extern NJXLInsRef NJX_store_q(NJXFunctionBuilderRef fn, NJXLInsRef value,
                              NJXLInsRef ptr, int32_t offset);
extern NJXLInsRef NJX_store_d(NJXFunctionBuilderRef fn, NJXLInsRef value,
                              NJXLInsRef ptr, int32_t offset);
extern NJXLInsRef NJX_store_f(NJXFunctionBuilderRef fn, NJXLInsRef value,
                              NJXLInsRef ptr, int32_t offset);

/**
* Tests the type of an instruction
*/
extern bool NJX_is_i(NJXLInsRef ins);
extern bool NJX_is_q(NJXLInsRef ins);
extern bool NJX_is_d(NJXLInsRef ins);
extern bool NJX_is_f(NJXLInsRef ins);

/**
* Completes the function, and assembles the code.
* If assembly is successful then the generated code is saved in the parent
* Context object
* by fragment name. The pointer to executable function is returned. Note that
* the pointer is
* valid only until the NanoJitContext is valid, as all functions are destroyed
* when the
* Context ends.
*/
extern void *NJX_finalize(NJXFunctionBuilderRef fn);

#ifdef __cplusplus
}
#endif

#endif