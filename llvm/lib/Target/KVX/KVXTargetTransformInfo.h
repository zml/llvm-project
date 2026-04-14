//===-- KVXTargetTransformInfo.h - KVX specific TTI -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file a TargetTransformInfo::Concept conforming object specific
// to the KVX target machine. It uses the target's detailed
// information to provide more precise answers to certain TTI queries,
// while letting the target independent and default TTI
// implementations handle the rest.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_KVX_KVXTARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_KVX_KVXTARGETTRANSFORMINFO_H

#include "KVX.h"
#include "KVXTargetMachine.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/BasicTTIImpl.h"
#include "llvm/CodeGen/TargetLowering.h"
#include <algorithm>

namespace llvm {

class KVXTTIImpl : public BasicTTIImplBase<KVXTTIImpl> {
  using BaseT = BasicTTIImplBase<KVXTTIImpl>;
  using TTI = TargetTransformInfo;

  friend BaseT;

  const TargetMachine *TM;
  const KVXSubtarget *ST;
  const KVXTargetLowering *TLI;
  const KVXSubtarget *getST() const { return ST; }
  const KVXTargetLowering *getTLI() const { return TLI; }
  using BaseT::preferInLoopReduction;

public:
  explicit KVXTTIImpl(const KVXTargetMachine *TM, const Function &F)
      : BaseT(TM, F.getParent()->getDataLayout()), TM(TM),
        ST(TM->getSubtargetImpl(F)), TLI(ST->getTargetLowering()) {}

  void getUnrollingPreferences(Loop *L, ScalarEvolution &SE,
                               TTI::UnrollingPreferences &UP,
                               OptimizationRemarkEmitter *ORE) const override;

  bool isHardwareLoopProfitable(Loop *L, ScalarEvolution &SE,
                                AssumptionCache &AC, TargetLibraryInfo *LibInfo,
                                HardwareLoopInfo &HWLoopInfo) const override;
  bool shouldAddRemainderMetaData() const override;
  bool isLoweredToCall(const Function *F) const override;
  bool isLoweredToCall(const CallInst &CI) const;
  bool isLSRCostLess(const TargetTransformInfo::LSRCost &C1,
                     const TargetTransformInfo::LSRCost &C2) const override;
  unsigned getInliningThresholdMultiplier() const override;

  TargetTransformInfo::PopcntSupportKind
  getPopcntSupport(unsigned IntTyWidthInBit) const override;

  bool shouldExpandReduction(const IntrinsicInst *II) const override;

  bool preferInLoopReduction(unsigned Opcode, Type *Ty,
                             TTI::ReductionFlags Flags) const;

  InstructionCost getArithmeticReductionCost(
      unsigned Opcode, VectorType *Ty, std::optional<FastMathFlags> FMF,
      TTI::TargetCostKind CostKind = TTI::TCK_RecipThroughput) const override;

  bool shouldMaximizeVectorBandwidth(
      TargetTransformInfo::RegisterKind K) const override {
    return true;
  };
  bool enableAggressiveInterleaving(bool LoopHasReductions) const override {
    return !ST->isV1();
  }

  bool enableInterleavedAccessVectorization() const override {
    return !ST->isV1();
  }

  bool isVectorShiftByScalarCheap(Type *Ty) const override { return true; }

  unsigned getNumberOfRegisters(unsigned ClassID) const override;
  TypeSize
  getRegisterBitWidth(TargetTransformInfo::RegisterKind K) const override {
    return TypeSize::getFixed(K == TargetTransformInfo::RGK_Scalar ? 64 : 256);
  }

  const char *getRegisterClassName(unsigned ClassID) const override;

  unsigned getRegisterClassForType(bool Vector, Type *Ty) const override;

  unsigned getMaxInterleaveFactor(ElementCount VF) const override {
    // FIXME: Workaround for VPPlan + InsCombine bugs.
    // Enforcing interleave when no vectorization is done (VF = 1)
    // generates a useless runtime test for this code, for
    // InterleaveCount (IC = 2), as iteration count is known to be
    // a multiple of 2. It also generates redundant code.
    // Appart from that, it creates a useless operation %0 = %index + 0
    // which InstructionCombiner bug breaks the code.
    // Input C code:
    // int my_parityl(unsigned long x) {
    //   int i;
    //   int count = 0;
    //   for (i = 0; i < 8 * sizeof(unsigned long); i++)
    //     if (x & ((unsigned long)1 << i))
    //       count++;
    //   return count & 1;
    // };
    // Once the InstCombine bug is fixed, as well the useless intruction
    // generation in VPlan, we should reconsider the performance impact
    // of this, and perhaps prevent useless code duplication.
    if (VF.isScalar())
      return 1;

    // Not knowing the largest element size is rather limiting,
    // so let's assume we operate with 32 bit values. We would
    // like to operate in 8 x 32 elements per iteration so we
    // compute:
    return std::max(1u, 8u / VF.getKnownMinValue());
  }

  unsigned getRegUsageForType(Type *Ty) const override {
    if (!Ty->isSized())
      return 0;

    unsigned Parts = static_cast<unsigned>(Ty->getPrimitiveSizeInBits() / 64);
    return std::max(1u, Parts);
  }

  InstructionCost
  getVectorInstrCost(unsigned Opcode, Type *Val,
                     TTI::TargetCostKind CostKind, unsigned Index = -1,
                     const Value *Op0 = nullptr,
                     const Value *Op1 = nullptr) const override;

  InstructionCost getVectorInstrCost(const Instruction &I, Type *Val,
                                     TTI::TargetCostKind CostKind,
                                     unsigned Index = -1) const override {
    return getVectorInstrCost(I.getOpcode(), Val, CostKind, Index);
  }

  InstructionCost getScalarizationOverhead(VectorType *Ty,
                                           const APInt &DemandedElts,
                                           bool Insert, bool Extract,
                                           TTI::TargetCostKind CostKind,
                                           bool ForPoisonSrc = true,
                                           ArrayRef<Value *> VL = {}) const override;

  bool isTypeLegal(Type *Ty) const override;

  InstructionCost getArithmeticInstrCost(
      unsigned Opcode, Type *Ty, TTI::TargetCostKind CostKind,
      TTI::OperandValueInfo Op1Info = {TTI::OK_AnyValue, TTI::OP_None},
      TTI::OperandValueInfo Op2Info = {TTI::OK_AnyValue, TTI::OP_None},
      ArrayRef<const Value *> Args = ArrayRef<const Value *>(),
      const Instruction *CxtI = nullptr) const override;


  InstructionCost getIntrinsicInstrCost(const IntrinsicCostAttributes &ICA,
                                        TTI::TargetCostKind CostKind) const override;

  InstructionCost getExtractWithExtendCost(unsigned Opcode, Type *Dst,
                                           VectorType *VecTy, unsigned Index = -1,
                                           TTI::TargetCostKind CostKind =
                                               TTI::TCK_SizeAndLatency) const override;
  InstructionCost
  getCastInstrCost(unsigned Opcode, Type *Dst, Type *Src,
                   TTI::CastContextHint CCH,
                   TTI::TargetCostKind CostKind = TTI::TCK_SizeAndLatency,
                   const Instruction *I = nullptr) const override;

  InstructionCost getCmpSelInstrCost(
      unsigned Opcode, Type *ValTy, Type *CondTy, CmpInst::Predicate VecPred,
      TTI::TargetCostKind CostKind,
      TTI::OperandValueInfo Op1Info = {TTI::OK_AnyValue, TTI::OP_None},
      TTI::OperandValueInfo Op2Info = {TTI::OK_AnyValue, TTI::OP_None},
      const Instruction *I = nullptr) const override;

  InstructionCost getMinMaxReductionCost(Intrinsic::ID IID, VectorType *Ty,
                                         FastMathFlags FMF,
                                         TTI::TargetCostKind CostKind) const override;

  InstructionCost getCFInstrCost(unsigned Opcode, TTI::TargetCostKind CostKind,
                                 const Instruction *I = nullptr) const override;

  unsigned getNumberOfParts(Type *Tp) const override;

private:
  bool isHardwareLoopProfitableCheck(Loop *L, ScalarEvolution &SE) const;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_KVX_KVXTARGETTRANSFORMINFO_H
