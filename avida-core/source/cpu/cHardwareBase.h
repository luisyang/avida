/*
 *  cHardwareBase.h
 *  Avida
 *
 *  Called "hardware_base.hh" prior to 11/17/05.
 *  Copyright 1999-2011 Michigan State University. All rights reserved.
 *  Copyright 1999-2003 California Institute of Technology.
 *
 *
 *  This file is part of Avida.
 *
 *  Avida is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 *  Avida is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License along with Avida.
 *  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef cHardwareBase_h
#define cHardwareBase_h

#include "avida/core/InstructionSequence.h"
#include "avida/output/Types.h"
#include "avida/systematics/Types.h"

#include <cassert>
#include <climits>
#include <iostream>

class cAvidaContext;
class cMutation;
class cOrganism;
class cString;
class cWorld;

using namespace std;
using namespace Avida;


class cHardwareBase
{
protected:
  cWorld* m_world;
  cOrganism* m_organism;            // Organism using this hardware.
  cInstSet* m_inst_set;             // Instruction set being used.

  HardwareTracerPtr m_tracer;        // Set this if you want execution traced.
  
  // --------  Instruction Costs  ---------
  Apto::Array<int> m_inst_ft_cost;
  Apto::Array<double> m_inst_res_cost;
  Apto::Array<double> m_inst_bonus_cost;
  Apto::Array<int> m_thread_inst_cost;
  Apto::Array<int> m_thread_inst_post_cost;
  Apto::Array<int> m_active_thread_costs;
  Apto::Array<int> m_active_thread_post_costs;

  struct {
    bool m_has_any_costs:1;
    bool m_has_costs:1;
    bool m_has_ft_costs:1;
    bool m_has_res_costs:1;
    bool m_has_post_costs:1;
    bool m_has_bonus_costs:1;
  };
  
  // --------  Base Hardware Feature Support  ---------
  Apto::Array<int, Apto::Smart> m_ext_mem;
  bool m_implicit_repro_active;
  

private:
  cString null_str;

public:
  cHardwareBase(cWorld* world, cOrganism* in_organism, cInstSet* inst_set);
  virtual ~cHardwareBase() { ; }
  
  // --------  Organism  ---------
  cOrganism* GetOrganism() { return m_organism; }
  const cInstSet& GetInstSet() const { return *m_inst_set; }


  // --------  Core Functionality  --------
  void Reset(cAvidaContext& ctx);
  virtual bool SingleProcess(cAvidaContext& ctx, bool speculative = false) = 0;
  virtual void ProcessBonusInst(cAvidaContext& ctx, const Instruction& inst) = 0;

  int Divide_DoMutations(cAvidaContext& ctx, double mut_multiplier = 1.0, const int maxmut = INT_MAX);
  bool Divide_TestFitnessMeasures(cAvidaContext& ctx);
  
  // --------  Helper methods  --------
  virtual int GetType() const = 0;
  virtual bool SupportsSpeculative() const = 0;
  
  
  virtual void PrintStatus(std::ostream& fp) = 0;
  void SetupExtendedMemory(const Apto::Array<int, Apto::Smart>& ext_mem) { m_ext_mem = ext_mem; }
  
  // --------  Stack Manipulation...  --------
  virtual int GetStack(int depth = 0, int stack_id = -1, int in_thread = -1) const = 0;
  virtual int GetCurStack(int in_thread_id = -1) const { (void)in_thread_id; return -1; }
  virtual int GetNumStacks() const = 0;
  
  
  // --------  Head Manipulation (including IP)  --------
  virtual const cHeadCPU& GetHead(int head_id) const = 0;
  virtual cHeadCPU& GetHead(int head_id) = 0;
  virtual const cHeadCPU& GetHead(int head_id, int thread) const = 0;
  virtual cHeadCPU& GetHead(int head_id, int thread) = 0;
  virtual int GetNumHeads() const = 0;
  
  virtual const cHeadCPU& IP() const = 0;
  virtual cHeadCPU& IP() = 0;
  virtual const cHeadCPU& IP(int thread) const = 0;
  virtual cHeadCPU& IP(int thread) = 0;
  
  
  // --------  Memory Manipulation  --------
  virtual const InstMemSpace& GetMemory() const = 0;
  virtual InstMemSpace& GetMemory() = 0;
  virtual int GetMemSize() const = 0;
  virtual const InstMemSpace& GetMemory(int value) const = 0;
  virtual InstMemSpace& GetMemory(int value) = 0;
  virtual int GetMemSize(int value) const = 0;
  virtual int GetNumMemSpaces() const = 0;
  
  const Apto::Array<int, Apto::Smart>& GetExtendedMemory() const { return m_ext_mem; }
  
  
  // --------  Register Manipulation  --------
  virtual int GetRegister(int reg_id) const = 0;
  virtual int GetNumRegisters() const = 0;
  
  
  // --------  Thread Manipulation  --------
  virtual Systematics::UnitPtr ThreadGetOwner() = 0;

  virtual int GetNumThreads() const = 0;
  virtual int GetCurThread() const = 0;
  
  // --------  Mutation  --------
  virtual int PointMutate(cAvidaContext& ctx, double override_mut_rate = 0.0);

  
  // --------  State Transfer  --------
  virtual void InheritState(cHardwareBase&) { ; }
  
  
protected:
  void ResizeCostArrays(int new_size);

  // --------  Core Execution Methods  --------
  bool SingleProcess_PayPreCosts(cAvidaContext& ctx, const Instruction& cur_inst, const int thread_id);
  void SingleProcess_PayPostResCosts(cAvidaContext& ctx, const Instruction& cur_inst);
  void SingleProcess_SetPostCPUCosts(cAvidaContext& ctx, const Instruction& cur_inst, const int thread_id);
  bool IsPayingActiveCost(cAvidaContext& ctx, const int thread_id);
  virtual void internalReset() = 0;
	virtual void internalResetOnFailedDivide() = 0;
  
  
  // --------  No-Operation Instruction  --------
  bool Inst_Nop(cAvidaContext& ctx);  // A no-operation instruction that does nothing! 
  
  
  // --------  Implicit Repro Check/Instruction  -------- @JEB
  inline void CheckImplicitRepro(cAvidaContext& ctx, bool exec_last_inst = false)
    { if (m_implicit_repro_active) checkImplicitRepro(ctx, exec_last_inst); }
  virtual bool Inst_Repro(cAvidaContext& ctx);

  
  // --------  Mutation Helper Methods  --------
  bool doUniformMutation(cAvidaContext& ctx, InstructionSequence& genome);
  void doUniformCopyMutation(cAvidaContext& ctx, cHeadCPU& head);
  void doSlipMutation(cAvidaContext& ctx, InstructionSequence& genome, int from = -1);
  void doTransMutation(cAvidaContext& ctx, InstructionSequence& genome, int from = -1);
  

  // --------  Organism Execution Property Calculation  --------
  virtual int calcExecutedSize(const int parent_size);
  virtual int calcCopiedSize(const int parent_size, const int child_size) = 0;  
  
  
  // --------  Division Support Methods  --------
  bool Divide_CheckViable(cAvidaContext& ctx, const int parent_size, const int child_size, bool using_repro = false);
  unsigned Divide_DoExactMutations(cAvidaContext& ctx, double mut_multiplier = 1.0, const int pointmut = INT_MAX);
  bool Divide_TestFitnessMeasures1(cAvidaContext& ctx);
  

private:
  void checkImplicitRepro(cAvidaContext& ctx, bool exec_last_inst = false);
};


#endif