//===- IntrinsicsAIR.td - Defines AIR intrinsics -----------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the AIR-flavoured intrinsic entry points observed in Apple
// Metal LLVM IR. Apple AIR bitcode exposes these as external calls named
// "air.*"; LLVM intrinsics must be named "llvm.*", so these records use the
// canonical "llvm.air.*" spelling while keeping the AIR symbol spelling in the
// final name components.
//
//===----------------------------------------------------------------------===//

def air_global_ptr_ty : LLVMQualPointerType<1>;

class AIRObservedIntrinsic<string air_name, list<LLVMType> ret_types,
                           list<LLVMType> param_types = [],
                           list<IntrinsicProperty> props = []>
    : DefaultAttrsIntrinsic<ret_types, param_types, props,
                            !strconcat("llvm.", air_name)>;

class AIRObservedRawIntrinsic<string air_name, list<LLVMType> ret_types,
                              list<LLVMType> param_types = [],
                              list<IntrinsicProperty> props = []>
    : Intrinsic<ret_types, param_types, props, !strconcat("llvm.", air_name)>;

let TargetPrefix = "air" in {

// Observed AIR symbols:
//   air.fast_sin.f32(float) -> float
//   air.fast_cos.f32(float) -> float
def int_air_fast_sin_f32 :
  AIRObservedIntrinsic<"air.fast_sin.f32", [llvm_float_ty], [llvm_float_ty],
                       [IntrNoMem]>;
def int_air_fast_cos_f32 :
  AIRObservedIntrinsic<"air.fast_cos.f32", [llvm_float_ty], [llvm_float_ty],
                       [IntrNoMem]>;

// Observed AIR symbols:
//   air.simd_any(i1) -> i1
//   air.simd_all(i1) -> i1
def int_air_simd_any :
  AIRObservedIntrinsic<"air.simd_any", [llvm_i1_ty], [llvm_i1_ty],
                       [IntrNoMem, IntrConvergent]>;
def int_air_simd_all :
  AIRObservedIntrinsic<"air.simd_all", [llvm_i1_ty], [llvm_i1_ty],
                       [IntrNoMem, IntrConvergent]>;

// Observed AIR symbols:
//   air.simd_sum.u.i32(i32) -> i32
//   air.simd_min.u.i32(i32) -> i32
//   air.simd_max.u.i32(i32) -> i32
def int_air_simd_sum_u_i32 :
  AIRObservedIntrinsic<"air.simd_sum.u.i32", [llvm_i32_ty], [llvm_i32_ty],
                       [IntrNoMem, IntrConvergent]>;
def int_air_simd_min_u_i32 :
  AIRObservedIntrinsic<"air.simd_min.u.i32", [llvm_i32_ty], [llvm_i32_ty],
                       [IntrNoMem, IntrConvergent]>;
def int_air_simd_max_u_i32 :
  AIRObservedIntrinsic<"air.simd_max.u.i32", [llvm_i32_ty], [llvm_i32_ty],
                       [IntrNoMem, IntrConvergent]>;

// Observed AIR symbols:
//   air.simd_sum.f32(float) -> float
//   air.simd_min.f32(float) -> float
//   air.simd_max.f32(float) -> float
def int_air_simd_sum_f32 :
  AIRObservedIntrinsic<"air.simd_sum.f32", [llvm_float_ty], [llvm_float_ty],
                       [IntrNoMem, IntrConvergent]>;
def int_air_simd_min_f32 :
  AIRObservedIntrinsic<"air.simd_min.f32", [llvm_float_ty], [llvm_float_ty],
                       [IntrNoMem, IntrConvergent]>;
def int_air_simd_max_f32 :
  AIRObservedIntrinsic<"air.simd_max.f32", [llvm_float_ty], [llvm_float_ty],
                       [IntrNoMem, IntrConvergent]>;

// Observed AIR symbols:
//   air.atomic.global.add.u.i32(ptr addrspace(1), i32, i32, i32, i1) -> i32
//   air.atomic.global.load.i32(ptr addrspace(1), i32, i32, i1) -> i32
//   air.atomic.global.store.i32(ptr addrspace(1), i32, i32, i32, i1) -> void
//
// These use Intrinsic instead of DefaultAttrsIntrinsic so the default nosync
// attribute is not inferred for atomic operations.
def int_air_atomic_global_add_u_i32 :
  AIRObservedRawIntrinsic<"air.atomic.global.add.u.i32", [llvm_i32_ty],
                          [air_global_ptr_ty, llvm_i32_ty, llvm_i32_ty,
                           llvm_i32_ty, llvm_i1_ty],
                          [IntrArgMemOnly, NoCapture<ArgIndex<0>>]>;
def int_air_atomic_global_load_i32 :
  AIRObservedRawIntrinsic<"air.atomic.global.load.i32", [llvm_i32_ty],
                          [air_global_ptr_ty, llvm_i32_ty, llvm_i32_ty,
                           llvm_i1_ty],
                          [IntrArgMemOnly, IntrReadMem,
                           NoCapture<ArgIndex<0>>, ReadOnly<ArgIndex<0>>]>;
def int_air_atomic_global_store_i32 :
  AIRObservedRawIntrinsic<"air.atomic.global.store.i32", [],
                          [air_global_ptr_ty, llvm_i32_ty, llvm_i32_ty,
                           llvm_i32_ty, llvm_i1_ty],
                          [IntrArgMemOnly, IntrWriteMem,
                           NoCapture<ArgIndex<0>>, WriteOnly<ArgIndex<0>>]>;
}
