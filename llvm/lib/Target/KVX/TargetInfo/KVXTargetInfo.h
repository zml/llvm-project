//===-- KVXTargetInfo.h - KVX Target Implementation ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_KVX_TARGETINFO_KVXTARGETINFO_H
#define LLVM_LIB_TARGET_KVX_TARGETINFO_KVXTARGETINFO_H

namespace llvm {

class Target;

Target &getTheKVXTarget();

} // namespace llvm

#endif // LLVM_LIB_TARGET_KVX_TARGETINFO_KVXTARGETINFO_H
