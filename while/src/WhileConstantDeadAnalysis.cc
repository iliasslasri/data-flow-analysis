// This file is part of While, an educational programming language and program
// analysis framework.
//
//   Copyright 2023 Florian Brandner
//
// While is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// While is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// While. If not, see <https://www.gnu.org/licenses/>.
//
// Contact: florian.brandner@telecom-paris.fr
//

// A simple analysis determining whether symbolic registers contain a constant
// value.

#include "WhileAnalysis.h"
#include "WhileLang.h"
#include "WhileCFG.h"
#include "WhileColor.h"


enum WhileConstantKind
{
  TOP,
  BOTTOM,
  CONSTANT
};

struct WhileConstantValue
{
  WhileConstantKind Kind;
  int Value;

  WhileConstantValue() : Kind(TOP), Value(0)
  {
  }

  WhileConstantValue(int value) : Kind(CONSTANT), Value(value)
  {
  }

  WhileConstantValue(WhileConstantKind kind) : Kind(kind), Value(0)
  {
  }
};

typedef std::map<int, WhileConstantValue> WhileConstantDomain;

enum WhileReachability
{
  REACHABLE,
  DEAD
};

struct WhileConstantDeadValue {
  WhileConstantDomain WConstant;
  WhileReachability WReachability = DEAD; // Reach. of current bb
  WhileReachability WReachabilityTakenBranch = DEAD;
};

bool operator==(const WhileConstantValue &a, const WhileConstantValue &b)
{
  return (a.Kind == b.Kind && a.Value == b.Value);
}

bool operator!=(const WhileConstantValue &a, const WhileConstantValue &b)
{
  return !(a== b);
}


std::ostream &operator<<(std::ostream &s, const WhileConstantValue &v)
{
  switch (v.Kind)
  {
    case TOP:
      return s << FLIGHT_GRAY << "⊤" << CRESET;
    case BOTTOM:
      return s << FRED << "⊥" << CRESET;
    case CONSTANT:
      return s << FGREEN << v.Value << CRESET;
  };
  abort();
}

extern std::ostream &operator<<(std::ostream &s, WhileReachability r);

bool operator==(const WhileConstantDeadValue &a, const WhileConstantDeadValue &b)
{
  return ( a.WConstant == b.WConstant &&
   a.WReachability == b.WReachability &&
   a.WReachabilityTakenBranch == b.WReachabilityTakenBranch);
}

bool operator!=(const WhileConstantDeadValue &a, const WhileConstantDeadValue &b)
{
  return !(a == b);
}

struct WhileConstant : public WhileDataFlowAnalysis<WhileConstantDeadValue>
{
  std::ostream &dump_first(std::ostream &s,
                           const WhileConstantDeadValue &value) override
  {
    s << "    [";
    bool first = true;
    for(const auto&[idx, c] : value.WConstant)
    {
      if (!first)
        s << ", ";

      s << "R" << idx << "=" << c;
      first = false;
    }
    s << "]";
    s << "  Reachability:  [";

    // Information on reachability
    switch (value.WReachability)
    {
      case DEAD:
       s << FRED << "DEAD";
       break;
      case REACHABLE:
       s << FGREEN << "REACHABLE";
       break;
    };
    s << CRESET;
    s << ", w/ branch :";
    switch (value.WReachabilityTakenBranch)
    {
      case DEAD:
       s << FRED << "DEAD";
       break;
      case REACHABLE:
       s << FGREEN << "REACHABLE";
       break;
    };
    s << CRESET;
    return s << "]\n";
  }

  std::ostream &dump_pre(std::ostream &s,
                         const WhileConstantDeadValue &value) override
  {
    return s;
  }

  std::ostream &dump_post(std::ostream &s,
                          const WhileConstantDeadValue &value) override
  {
    return dump_first(s, value);
  }

  static void updateRegisterOperand(const WhileInstr &instr, unsigned int idx,
                             WhileConstantDeadValue &result,
                             WhileConstantValue value)
  {
    const WhileOperand &op = instr.Ops[idx];
    switch (op.Kind)
    {
      case WREGISTER:
        assert(op.ValueOrIndex >= 0);
        result.WConstant[op.ValueOrIndex] = value;
        return;

      case WFRAMEPOINTER:
      case WIMMEDIATE:
      case WBLOCK:
      case WFUNCTION:
      case WUNKNOWN:
        assert("Operand is not a register.");
    }
    abort();
  }

  static WhileConstantValue readDataOperand(const WhileInstr &instr,
                                            unsigned int idx,
                                            const WhileConstantDeadValue &input)
  {
    const WhileOperand &op = instr.Ops[idx];
    switch (op.Kind)
    {
      case WREGISTER:
      {
        assert(op.ValueOrIndex >= 0);
        auto value = input.WConstant.find(op.ValueOrIndex);
        if (value == input.WConstant.end())
          return BOTTOM; // register undefined
        else
          return value->second;
      }
      case WIMMEDIATE:
        return op.ValueOrIndex;

      case WFRAMEPOINTER:
          return BOTTOM;

      case WBLOCK:
      case WFUNCTION:
      case WUNKNOWN:
        assert("Operand is not a data value.");
    }
    abort();
  }

  WhileConstantDeadValue transfer(const WhileInstr &instr, const WhileConstantDeadValue input) override
  {
    WhileConstantDeadValue result = input;
    const auto &ops = instr.Ops;
    bool temp_reach = false;
    // any code after an unreachable branch or return instruction remains unreachable
    if(input.WReachability == DEAD && input.WReachabilityTakenBranch == DEAD){
      result.WReachability = DEAD;
    } else {
      temp_reach = true;
    }
    switch(instr.Opc)
    {
      case WBRANCHZ:
        if(temp_reach){
          WhileConstantValue cond = readDataOperand(instr, 0, input);
          if (cond.Kind == CONSTANT) {
            if (cond.Value == 1){
              result.WReachabilityTakenBranch = DEAD;
              result.WReachability = REACHABLE;
            } else {
              result.WReachability = DEAD;
              result.WReachabilityTakenBranch = REACHABLE;
            }
          } else {
            result.WReachability = REACHABLE;
            result.WReachabilityTakenBranch = REACHABLE;
          }
        }
        break;
      case WBRANCH:
        result.WReachability = DEAD;
        result.WReachabilityTakenBranch = REACHABLE;
        break;
      case WRETURN:
        result.WReachability = DEAD;
        result.WReachabilityTakenBranch = DEAD;
        break;
      case WSTORE:
        // do not write symbolic registers
        break;

      case WCALL:
      {
        // Ops: Fun Opd = Arg1, Arg2, ... ArgN
        assert(ops.size() > 2);
        updateRegisterOperand(instr, 1, result, BOTTOM);
        break;
      }

      case WLOAD:
      {
        // Ops: OpD = [BaseAddress + Offset]
        assert(ops.size() ==  3);
        updateRegisterOperand(instr, 0, result, BOTTOM);
        break;
      }

      case WPLUS:
      {
        // Ops: OpD = OpA + OpB
        assert(ops.size() ==  3);
        WhileConstantValue a = readDataOperand(instr, 1, input);
        WhileConstantValue b = readDataOperand(instr, 2, input);

        if (a.Kind == CONSTANT && b.Kind == CONSTANT)
          updateRegisterOperand(instr, 0, result, a.Value + b.Value);
        else
          updateRegisterOperand(instr, 0, result, BOTTOM);
        break;
      }
      case WMINUS:
      {
        // Ops: OpD = OpA - OpB
        assert(ops.size() ==  3);
        WhileConstantValue a = readDataOperand(instr, 1, input);
        WhileConstantValue b = readDataOperand(instr, 2, input);

        if (a.Kind == CONSTANT && b.Kind == CONSTANT)
          updateRegisterOperand(instr, 0, result, a.Value - b.Value);
        else
          updateRegisterOperand(instr, 0, result, BOTTOM);
        break;
      }
      case WMULT:
      {
        // Ops: OpD = OpA * OpB
        assert(ops.size() ==  3);
        WhileConstantValue a = readDataOperand(instr, 1, input);
        WhileConstantValue b = readDataOperand(instr, 2, input);

        if (a.Kind == CONSTANT && b.Kind == CONSTANT)
          updateRegisterOperand(instr, 0, result, a.Value * b.Value);
        else
          updateRegisterOperand(instr, 0, result, BOTTOM);
        break;
      }
      case WDIV:
      {
        // Ops: OpD = OpA / OpB
        assert(ops.size() ==  3);
        WhileConstantValue a = readDataOperand(instr, 1, input);
        WhileConstantValue b = readDataOperand(instr, 2, input);

        if (a.Kind == CONSTANT && b.Kind == CONSTANT)
          updateRegisterOperand(instr, 0, result, a.Value / b.Value);
        else
          updateRegisterOperand(instr, 0, result, BOTTOM);
        break;
      }
      case WEQUAL:
      {
        // Ops: OpD = OpA == OpB
        assert(ops.size() ==  3);
        WhileConstantValue a = readDataOperand(instr, 1, input);
        WhileConstantValue b = readDataOperand(instr, 2, input);

        if (a.Kind == CONSTANT && b.Kind == CONSTANT)
          updateRegisterOperand(instr, 0, result, a.Value == b.Value);
        else
          updateRegisterOperand(instr, 0, result, BOTTOM);
        break;
      }
      case WUNEQUAL:
      {
        // Ops: OpD = OpA != OpB
        assert(ops.size() ==  3);
        WhileConstantValue a = readDataOperand(instr, 1, input);
        WhileConstantValue b = readDataOperand(instr, 2, input);

        if (a.Kind == CONSTANT && b.Kind == CONSTANT)
          updateRegisterOperand(instr, 0, result, a.Value != b.Value);
        else
          updateRegisterOperand(instr, 0, result, BOTTOM);
        break;
      }
      case WLESS:
      {
        // Ops: OpD = OpA < OpB
        assert(ops.size() ==  3);
        WhileConstantValue a = readDataOperand(instr, 1, input);
        WhileConstantValue b = readDataOperand(instr, 2, input);

        if (a.Kind == CONSTANT && b.Kind == CONSTANT)
          updateRegisterOperand(instr, 0, result, a.Value < b.Value);
        else
          updateRegisterOperand(instr, 0, result, BOTTOM);
        break;
      }
      case WLESSEQUAL:
      {
        // Ops: OpD = OpA <= OpB
        assert(ops.size() ==  3);
        WhileConstantValue a = readDataOperand(instr, 1, input);
        WhileConstantValue b = readDataOperand(instr, 2, input);

        if (a.Kind == CONSTANT && b.Kind == CONSTANT)
          updateRegisterOperand(instr, 0, result, a.Value <= b.Value);
        else
          updateRegisterOperand(instr, 0, result, BOTTOM);
        break;
      }
    };

    return result;
  }

  static WhileConstantValue join(const WhileConstantValue &a,
                                 const WhileConstantValue &b)
  {
    if (a.Kind == TOP)
      return b;
    else if (b.Kind == TOP)
      return a;
    else if (a == b)
      return a;
    else
      return BOTTOM;
  }

  WhileConstantDeadValue join(std::list<WhileConstantDeadValue> inputs) override
  {
    WhileConstantDeadValue result;
    if( inputs.empty() ){
      result.WReachability = DEAD;
    } else {
      
    for(WhileConstantDeadValue r : inputs)
    {
      for(const auto&[idx, value] : r.WConstant)
      { // TODO : Verify next condition !!! 
        // it appers to be ok, for the example ( the bb {x = 9;} won't be reachable
        // so the value 9, won't be taken into account )
        if ( r.WReachability == REACHABLE || r.WReachabilityTakenBranch == REACHABLE){
          WhileConstantValue &resultvalue = result.WConstant[idx];
          resultvalue = join(resultvalue, value);

          // Reachability
          if(r.WReachability == REACHABLE){
            result.WReachability = REACHABLE;
          }
          if(r.WReachabilityTakenBranch == REACHABLE){
            result.WReachabilityTakenBranch = REACHABLE;
          }
        }
      }
    }
    }

    return result;
  }

  WhileConstantDeadValue join(const WhileBlock *bb) override {
    std::list<WhileConstantDeadValue> bbIn;
    for(const auto &[pred, kind] : bb->Pred){
      if(kind == WFALL_THROUGH && BBOut[pred].WReachability == REACHABLE)
        bbIn.emplace_back(BBOut[pred]);
      else if(kind == WBRANCH_TAKEN && BBOut[pred].WReachabilityTakenBranch == REACHABLE)
        bbIn.emplace_back(BBOut[pred]);
    }
    // indicate reachability of the current basic block bb
    WhileConstantDeadValue wcdv = join(bbIn); 
    // We should just stamp on the entry of bb if we are really in entry
    if(bb->Index == 0){
      wcdv.WReachability = REACHABLE;
    }
    return wcdv;
  }
};


struct WhileConstantDeadValueAnalysis : public WhileAnalysis
{
  void analyze(const WhileProgram &p) override
  {
    WhileConstant WCDA;
    WCDA.analyze(p);
    WCDA.dump(std::cout, p);
  };

  WhileConstantDeadValueAnalysis() : WhileAnalysis("WCDA",
                                                  "Constant Register & Dead Code Analysis")
  {
  }
};

WhileConstantDeadValueAnalysis WCDA;
