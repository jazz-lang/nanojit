/**
* This file defines a C API for NanoJIT.
* Copyright (C) 2017 Dibyendu Majumdar
*/
#ifndef __nanojit_extra__
#define __nanojit_extra__

#include <stdbool.h>
#include <stdint.h>

/*
* Currently only X86-64 is being tested.
*/
#define NANOJIT_64BIT

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NJXLIns *NJXLInsRef;

/**
* The Jit Context defines a container for the Jit machinery and
* also acts as the repository of the compiled functions. The Jit
* Context must be kept alive as long as any functions within it are
* needed. Deleting the Jit Context will also delete all compiled
* functions managed by the context.
*/
typedef struct NJXContext *NJXContextRef;

/**
* A Function Builder is used to generate code for a single
* C function equivalent. Once the code is generated by invoking
* finalize on the builder, the builder itself can be destroyed as the
* compiled function lives on in the associated Jit Context.
*/
typedef struct NFXFunctionBuilder *NJXFunctionBuilderRef;

/**
* Nanojit function parameter types are is a 64-bit quantities
* on a 64-bit machine
*/
typedef int64_t NJXParamType;

/**
* Calling ABI - but believe it doesn't make a difference on
* X86-64. 
*/
enum NJXCallAbiKind {
  NJX_CALLABI_FASTCALL,
  NJX_CALLABI_THISCALL,
  NJX_CALLABI_STDCALL,
  NJX_CALLABI_CDECL
};

/*
* Maximum number of arguments that can be
* accepted by a JITed function.
*/
enum {
#ifdef _WIN64
  NJXMaxArgs = 4
#else
  NJXMaxArgs = 6
#endif
};

/*
* These types are used to define function argument
* types. See NJX_create_function_builder() below for further
* notes.
*/
enum NJXValueKind {
  NJXValueKind_I = 1, // int32_t
#ifdef NANOJIT_64BIT
  NJXValueKind_Q = 3, // uint64_t
#endif
  NJXValueKind_D = 4, // double
  NJXValueKind_F = 5, // single-precision float;
#ifdef NANOJIT_64BIT
  NJXValueKind_P = NJXValueKind_Q, // pointer
#else
  NJXValueKind_P = NJXValueKind_I, // pointer
#endif
};

/*
* Note on NanoJIT types:
* The NanoJIT IR operates on 4 types of values:
* i - integer (32 bit)
* q - quad (64 bit integer)
* f - float (32 bit)
* d - double (64 bit)
* On 64-bit architecture pointers are synonymous with quads,
* i.e. a pointer is just a quad integer. There is no separate
* pointer type.
*/


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

/*
* Registers an externally defined C function.
* Note that such functions can only accept upto 8 parameters
* and the only supported calling convention is C calling
* convention.
*/
extern bool NJX_register_C_function(NJXContextRef context, const char *name,
                                    void *fptr, enum NJXValueKind return_type,
                                    const enum NJXValueKind *args, int argc);

/**
* Returns a Jit compiled function looking it up by name.
* The pointer must be cast to the correct signature.
* Returns nullptr if function not found.
*/
extern void *NJX_get_function_by_name(NJXContextRef, const char *name);

/**
* Creates a new FunctionBuilder object. The builder is used to construct the
* code that will go into one function. Once the function has been defined,
* machine code is generated by calling finalize(). After the function is
* compiled the builder object can be thrown away - the compiled function
* will live as long as the owning Jit Context lives.
* If optimize flag is true then NanoJit's CSE and Expr filters are enabled.
* *** IMPORTANT ***
* Note that a limitation of NanoJIT is that the function can only
* accept integer or pointer parameters on X64 architecture. Furthermore
* the number of parameters is limited to 4 on WIN64 and 6 on UNIX platforms
* as these are the integer/pointer registers used by these OSes on
* X64 platform. If you specify unsupported number of arguments then
* an error will be reported and this function will fail.
*/
extern NJXFunctionBuilderRef NJX_create_function_builder(
    NJXContextRef context, const char *name, enum NJXValueKind return_type,
    const enum NJXValueKind *args, int argc, int optimize);

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

/* Return float */
extern NJXLInsRef NJX_retf(NJXFunctionBuilderRef fn, NJXLInsRef result);

/**
* Adds a quad return instruction.
*/
extern NJXLInsRef NJX_retq(NJXFunctionBuilderRef fn, NJXLInsRef result);

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
* Gets a function parameter. The number and types of parameters
* of a function are specified in NJX_create_function_builder().
* This api retrieves a suitable cast parameter value.
*/
extern NJXLInsRef NJX_get_parameter(NJXFunctionBuilderRef fn, int i);

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

/* Subtract */
extern NJXLInsRef NJX_subi(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_subq(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_subd(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_subf(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);

/* Multiply */
extern NJXLInsRef NJX_muli(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_mulq(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_muld(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_mulf(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);

/* Divide */
extern NJXLInsRef NJX_divi(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_divq(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_divd(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_divf(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);

/* Modulus */
extern NJXLInsRef NJX_modi(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_modq(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);

/* Negate */
extern NJXLInsRef NJX_negq(NJXFunctionBuilderRef fn, NJXLInsRef q);
extern NJXLInsRef NJX_negi(NJXFunctionBuilderRef fn, NJXLInsRef q);
extern NJXLInsRef NJX_negf(NJXFunctionBuilderRef fn, NJXLInsRef q);
extern NJXLInsRef NJX_negd(NJXFunctionBuilderRef fn, NJXLInsRef q);

/* Bitwise Operators */
extern NJXLInsRef NJX_notq(NJXFunctionBuilderRef fn, NJXLInsRef q);
extern NJXLInsRef NJX_noti(NJXFunctionBuilderRef fn, NJXLInsRef q);
extern NJXLInsRef NJX_andi(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_andq(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_ori(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);
extern NJXLInsRef NJX_orq(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);
extern NJXLInsRef NJX_xori(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_xorq(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_lshi(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_lshq(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_rshi(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_rshq(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_rshui(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                            NJXLInsRef rhs);
extern NJXLInsRef NJX_rshuq(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                            NJXLInsRef rhs);

/**
* Comparisons. Note that there is no api for testing "not equal to". 
* You can call the eq?() api twice to get not equal.
*/
extern NJXLInsRef NJX_eqi(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);
extern NJXLInsRef NJX_eqq(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);
extern NJXLInsRef NJX_eqd(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);
extern NJXLInsRef NJX_eqf(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);

extern NJXLInsRef NJX_lti(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);
extern NJXLInsRef NJX_lei(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);
extern NJXLInsRef NJX_ltui(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_leui(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_ltq(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);
extern NJXLInsRef NJX_leq(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);
extern NJXLInsRef NJX_ltuq(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_leuq(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_ltd(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);
extern NJXLInsRef NJX_led(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);
extern NJXLInsRef NJX_ltf(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);
extern NJXLInsRef NJX_lef(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);

extern NJXLInsRef NJX_gti(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);
extern NJXLInsRef NJX_gei(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);
extern NJXLInsRef NJX_gtui(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_geui(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_gtq(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);
extern NJXLInsRef NJX_geq(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);
extern NJXLInsRef NJX_gtuq(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_geuq(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                           NJXLInsRef rhs);
extern NJXLInsRef NJX_gtd(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);
extern NJXLInsRef NJX_ged(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);
extern NJXLInsRef NJX_gtf(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);
extern NJXLInsRef NJX_gef(NJXFunctionBuilderRef fn, NJXLInsRef lhs,
                          NJXLInsRef rhs);

/**
* Conversions. u here means unsigned, c means character.
*/
extern NJXLInsRef NJX_i2q(NJXFunctionBuilderRef fn, NJXLInsRef q);
extern NJXLInsRef NJX_ui2uq(NJXFunctionBuilderRef fn, NJXLInsRef q);
extern NJXLInsRef NJX_q2i(NJXFunctionBuilderRef fn, NJXLInsRef q);
extern NJXLInsRef NJX_q2d(NJXFunctionBuilderRef fn, NJXLInsRef q);
extern NJXLInsRef NJX_i2d(NJXFunctionBuilderRef fn, NJXLInsRef q);
extern NJXLInsRef NJX_i2f(NJXFunctionBuilderRef fn, NJXLInsRef q);
extern NJXLInsRef NJX_ui2d(NJXFunctionBuilderRef fn, NJXLInsRef q);
extern NJXLInsRef NJX_ui2f(NJXFunctionBuilderRef fn, NJXLInsRef q);
extern NJXLInsRef NJX_f2d(NJXFunctionBuilderRef fn, NJXLInsRef q);
extern NJXLInsRef NJX_d2f(NJXFunctionBuilderRef fn, NJXLInsRef q);
extern NJXLInsRef NJX_d2i(NJXFunctionBuilderRef fn, NJXLInsRef q);
extern NJXLInsRef NJX_f2i(NJXFunctionBuilderRef fn, NJXLInsRef q);
extern NJXLInsRef NJX_d2q(NJXFunctionBuilderRef fn, NJXLInsRef q);

/**
* Inserts a label at current position, no code is emitted for this
*/
extern NJXLInsRef NJX_add_label(NJXFunctionBuilderRef fn);

/**
* Allocates 'size' bytes on the stack. Load and store instructions
* can be used to access memory allocated.
*/
extern NJXLInsRef NJX_alloca(NJXFunctionBuilderRef fn, int32_t size);

/**
* Inserts an unconditional jump - 'to' can be NULL and set later
* using NJX_set_jmp_target().
*/
extern NJXLInsRef NJX_br(NJXFunctionBuilderRef fn, NJXLInsRef to);

/**
* Inserts a conditional branch - jump targets can be NULL and set later
* using NJX_set_jmp_target().
*/
extern NJXLInsRef NJX_cbr_true(NJXFunctionBuilderRef fn, NJXLInsRef cond,
                               NJXLInsRef to);
extern NJXLInsRef NJX_cbr_false(NJXFunctionBuilderRef fn, NJXLInsRef cond,
                                NJXLInsRef to);

/**
* Assigns a value based on the condition - similar to C's ?: operator.
* If use_cmov is true, then emit CMOV assembly instruction
* Note that the condition must be a comparison operator or
* the literal 0 or 1. Ensure that both operands are the same time
* else there will be an assertion failure
*/
extern NJXLInsRef NJX_choose(NJXFunctionBuilderRef fn, NJXLInsRef cond,
                          NJXLInsRef iftrue, NJXLInsRef iffalse, bool use_cmov);

/**
* Generates a C switch like instruction, where branches
* are taken based on the value of an integer index.
* size says how many possible values index can take
* Note that each of the caller must set the
* jump targets for each value using NJX_set_switch_target().
* ISSUE: There is no way to specify a default case
* ISSUE: Switch values must start at 0 and be consecutive
*/
extern NJXLInsRef NJX_switch(NJXFunctionBuilderRef fn, NJXLInsRef index,
                             int32_t size);

/**
* Sets the jump target for a switch instruction.
*/
extern void NJX_set_switch_target(NJXLInsRef switchins, uint32_t index,
                                  NJXLInsRef target);

/**
* Sets the target of a jump instruction
* target should be a label instruction
* and jmp should be the jump instruction returned by NJX_br(), NJX_cbr_true() or
* NJX_cbr_false().
*/
extern void NJX_set_jmp_target(NJXLInsRef jmp, NJXLInsRef target);

/* Loads, here c means character, u means unsigned, s means short */
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

/* Stores - here s means short, c means character */
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

/*
* Insert liveness information.
* The register allocator computes virtual register liveness as it runs, while it
* is scanning LIR bottom-up. To prevent the allocator from thinking a
* register is available when it is not, following actions are needed:
*
* a) Mark function parameters as live after all code is emitted for the
* function.
*
* b) If a virtual register is being defined before the loop entry
* point, and used inside a loop, then it's live range must cover the whole loop.
* the frontend compiler must insert LIR_live at the loop jumps (back edges)
* to extend the live range.
*
* Example: Suppose you have a jump to block B. LIR_live for B's live-in
* registers, just before the jump, should be added (only needed for backwards
* jumps though). Note that if the jump is in B1 and the target is B2, you
* need LIR_live for B2's live-in registers.
*/
extern NJXLInsRef NJX_liveq(NJXFunctionBuilderRef fn, NJXLInsRef);
extern NJXLInsRef NJX_livei(NJXFunctionBuilderRef fn, NJXLInsRef);
extern NJXLInsRef NJX_livef(NJXFunctionBuilderRef fn, NJXLInsRef);
extern NJXLInsRef NJX_lived(NJXFunctionBuilderRef fn, NJXLInsRef);

/*
Insert calls - note maximum number of arguments is 8 on X86-64 platforms
(details TBC) The function being called must have previously been compiled
in Nanojit or declared as external (API TODO). If the function is not
found then the instruction will fail and nullptr will be returned. At
present the caller decides the Abi and the arguments - I think this is
problematic as we should already have this when the function is defined,
and not expect caller to provide.
*/
extern NJXLInsRef NJX_callv(NJXFunctionBuilderRef fn, const char *funcname,
                            enum NJXCallAbiKind abi, int nargs,
                            NJXLInsRef args[]);
extern NJXLInsRef NJX_calli(NJXFunctionBuilderRef fn, const char *funcname,
                            enum NJXCallAbiKind abi, int nargs,
                            NJXLInsRef args[]);
extern NJXLInsRef NJX_callq(NJXFunctionBuilderRef fn, const char *funcname,
                            enum NJXCallAbiKind abi, int nargs,
                            NJXLInsRef args[]);
extern NJXLInsRef NJX_callf(NJXFunctionBuilderRef fn, const char *funcname,
                            enum NJXCallAbiKind abi, int nargs,
                            NJXLInsRef args[]);
extern NJXLInsRef NJX_calld(NJXFunctionBuilderRef fn, const char *funcname,
                            enum NJXCallAbiKind abi, int nargs,
                            NJXLInsRef args[]);

/* 
* Inserts a comment, the supplied string must be valid as long as the 
* function builder is live, as otherwise there will memory fault when 
* the builder tries to access the comment.
*/
extern NJXLInsRef NJX_comment(NJXFunctionBuilderRef fn, const char *s);

/**
* Completes the function, and assembles the code.
* If assembly is successful then the generated code is saved in the parent
* Context object by fragment name. The pointer to executable function is
* returned. Note that the pointer is valid only until the NanoJitContext
* is valid, as all functions are destroyed when the Context ends.
*/
extern void *NJX_finalize(NJXFunctionBuilderRef fn);

#ifdef __cplusplus
}
#endif

#endif