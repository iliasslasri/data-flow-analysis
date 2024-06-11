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

struct WhileRangeValue
{
  WhileConstantKind Kind;
  // modification
  int upperValue;
  int lowerValue;

  WhileRangeValue() : Kind(TOP), upperValue(0), lowerValue(0)
  {
  }

  WhileRangeValue(int value) : Kind(CONSTANT), upperValue(value), lowerValue(value)
  {
  }

  WhileRangeValue(WhileConstantKind kind) : Kind(kind), upperValue(0), lowerValue(0)
  {
  }

  WhileRangeValue(WhileConstantKind kind, int min, int max) : Kind(kind), upperValue(max), lowerValue(min)
  {
  }

  /// operations

   // Overload + operator
  WhileRangeValue operator+(const WhileRangeValue& other) const
  {
    return WhileRangeValue{Kind, lowerValue + other.lowerValue, upperValue + other.upperValue};
  }

  // Overload - operator
  WhileRangeValue operator-(const WhileRangeValue& other) const
  {
    return WhileRangeValue{Kind, lowerValue - other.upperValue, upperValue - other.lowerValue};
  }

  // Overload * operator
  WhileRangeValue operator*(const WhileRangeValue& other) const
  {
    int min = std::min({lowerValue * other.lowerValue, lowerValue * other.upperValue,
                         upperValue * other.lowerValue, upperValue * other.upperValue});
    int max = std::max({lowerValue * other.lowerValue, lowerValue * other.upperValue,
                         upperValue * other.lowerValue, upperValue * other.upperValue});
    return WhileRangeValue{Kind, min, max};
  }

  // Overload / operator
  WhileRangeValue operator/(const WhileRangeValue& other) const
  {
    // Handle division by zero
    if (other.lowerValue == 0 && other.upperValue == 0)
    {
      throw std::runtime_error("Division by zero");
    }

    int min = std::min({lowerValue / other.lowerValue, lowerValue / other.upperValue,
                         upperValue / other.lowerValue, upperValue / other.upperValue});
    int max = std::max({lowerValue / other.lowerValue, lowerValue / other.upperValue,
                         upperValue / other.lowerValue, upperValue / other.upperValue});
    return WhileRangeValue{Kind, min, max};
  }

  // Overload != operator
  bool operator!=(const WhileRangeValue& other) const
  {
    return !(lowerValue == other.lowerValue && upperValue == other.upperValue);
  }

  // Overload < operator
  bool operator<(const WhileRangeValue& other) const
  {
    return upperValue < other.lowerValue;
  }

  // Overload <= operator
  bool operator<=(const WhileRangeValue& other) const
  {
    return upperValue <= other.lowerValue;
  }

};

bool operator==(const WhileRangeValue &a, const WhileRangeValue &b)
{
  return (a.Kind == b.Kind && a.upperValue == b.upperValue && a.lowerValue == b.lowerValue);
}

typedef std::map<int, WhileRangeValue> WhileConstantDomain;

std::ostream &operator<<(std::ostream &s, const WhileRangeValue &v)
{
  switch (v.Kind)
  {
    case TOP:
      return s << FLIGHT_GRAY << "⊤" << CRESET;
    case BOTTOM:
      return s << FRED << "⊥" << CRESET;
    case CONSTANT:
      return s << FGREEN << "[" << v.lowerValue << ":" << v.upperValue << "]" << CRESET;
  };
  abort();
}


struct WhileConstant : public WhileDataFlowAnalysis<WhileConstantDomain>
{
  std::ostream &dump_first(std::ostream &s,
                           const WhileConstantDomain &value) override
  {
    s << "    [";
    bool first = true;
    for(const auto&[idx, c] : value)
    {
      if (!first)
        s << ", ";

      s << "R" << idx << "=" << c;
      first = false;
    }
    return s << "]\n";
  }

  std::ostream &dump_pre(std::ostream &s,
                         const WhileConstantDomain &value) override
  {
    return s;
  }

  std::ostream &dump_post(std::ostream &s,
                          const WhileConstantDomain &value) override
  {
    return dump_first(s, value);
  }

  static void updateRegisterOperand(const WhileInstr &instr, unsigned int idx,
                             WhileConstantDomain &result,
                             WhileRangeValue value)
  {
    const WhileOperand &op = instr.Ops[idx];
    switch (op.Kind)
    {
      case WREGISTER:
        assert(op.ValueOrIndex >= 0);
        result[op.ValueOrIndex] = value;
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

  static WhileRangeValue readDataOperand(const WhileInstr &instr,
                                            unsigned int idx,
                                            const WhileConstantDomain &input)
  {
    const WhileOperand &op = instr.Ops[idx];
    switch (op.Kind)
    {
      case WREGISTER:
      {
        assert(op.ValueOrIndex >= 0);
        auto value = input.find(op.ValueOrIndex);
        if (value == input.end())
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

  WhileConstantDomain transfer(const WhileInstr &instr, const WhileConstantDomain input) override
  {
    WhileConstantDomain result = input;
    const auto &ops = instr.Ops;
    switch(instr.Opc)
    {
      case WBRANCHZ:
      case WBRANCH:
      case WRETURN:
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
        WhileRangeValue a = readDataOperand(instr, 1, input);
        WhileRangeValue b = readDataOperand(instr, 2, input);

        if (a.Kind == CONSTANT && b.Kind == CONSTANT)
          updateRegisterOperand(instr, 0, result, a + b);
        else
          updateRegisterOperand(instr, 0, result, BOTTOM);
        break;
      }
      case WMINUS:
      {
        // Ops: OpD = OpA - OpB
        assert(ops.size() ==  3);
        WhileRangeValue a = readDataOperand(instr, 1, input);
        WhileRangeValue b = readDataOperand(instr, 2, input);

        if (a.Kind == CONSTANT && b.Kind == CONSTANT)
          updateRegisterOperand(instr, 0, result, a - b);
        else
          updateRegisterOperand(instr, 0, result, BOTTOM);
        break;
      }
      case WMULT:
      {
        // Ops: OpD = OpA * OpB
        assert(ops.size() ==  3);
        WhileRangeValue a = readDataOperand(instr, 1, input);
        WhileRangeValue b = readDataOperand(instr, 2, input);

        if (a.Kind == CONSTANT && b.Kind == CONSTANT)
          updateRegisterOperand(instr, 0, result, a * b);
        else
          updateRegisterOperand(instr, 0, result, BOTTOM);
        break;
      }
      case WDIV:
      {
        // Ops: OpD = OpA / OpB
        assert(ops.size() ==  3);
        WhileRangeValue a = readDataOperand(instr, 1, input);
        WhileRangeValue b = readDataOperand(instr, 2, input);

        if (a.Kind == CONSTANT && b.Kind == CONSTANT)
          updateRegisterOperand(instr, 0, result, a / b);
        else
          updateRegisterOperand(instr, 0, result, BOTTOM);
        break;
      }
      case WEQUAL:
      {
        // Ops: OpD = OpA == OpB
        assert(ops.size() ==  3);
        WhileRangeValue a = readDataOperand(instr, 1, input);
        WhileRangeValue b = readDataOperand(instr, 2, input);

        if (a.Kind == CONSTANT && b.Kind == CONSTANT)
          updateRegisterOperand(instr, 0, result, a == b);
        else
          updateRegisterOperand(instr, 0, result, BOTTOM);
        break;
      }
      case WUNEQUAL:
      {
        // Ops: OpD = OpA != OpB
        assert(ops.size() ==  3);
        WhileRangeValue a = readDataOperand(instr, 1, input);
        WhileRangeValue b = readDataOperand(instr, 2, input);

        if (a.Kind == CONSTANT && b.Kind == CONSTANT)
          updateRegisterOperand(instr, 0, result, a != b);
        else
          updateRegisterOperand(instr, 0, result, BOTTOM);
        break;
      }
      case WLESS:
      {
        // Ops: OpD = OpA < OpB
        assert(ops.size() ==  3);
        WhileRangeValue a = readDataOperand(instr, 1, input);
        WhileRangeValue b = readDataOperand(instr, 2, input);

        if (a.Kind == CONSTANT && b.Kind == CONSTANT)
          updateRegisterOperand(instr, 0, result, a < b);
        else
          updateRegisterOperand(instr, 0, result, BOTTOM);
        break;
      }
      case WLESSEQUAL:
      {
        // Ops: OpD = OpA <= OpB
        assert(ops.size() ==  3);
        WhileRangeValue a = readDataOperand(instr, 1, input);
        WhileRangeValue b = readDataOperand(instr, 2, input);

        if (a.Kind == CONSTANT && b.Kind == CONSTANT)
          updateRegisterOperand(instr, 0, result, a <= b);
        else
          updateRegisterOperand(instr, 0, result, BOTTOM);
        break;
      }
    };

    return result;
  }

  static WhileRangeValue join(const WhileRangeValue &a,
                                 const WhileRangeValue &b)
  {
    if (a.Kind == TOP && b.Kind == CONSTANT)
      return b;
    else if (b.Kind == TOP && a.Kind == CONSTANT)
      return a;
    else if (a == b)
      return a;
    else if( a.Kind == CONSTANT && b.Kind == CONSTANT)
    {
      int newLowerValue = std::min(a.lowerValue, b.lowerValue);
      int newUpperValue = std::max(a.upperValue, b.upperValue);
      return WhileRangeValue{a.Kind, newLowerValue, newUpperValue};
    } 
    else if (a.Kind == b.Kind)
      return a;
    else
      return BOTTOM;
  }

  WhileConstantDomain join(std::list<WhileConstantDomain> inputs) override
  {
    WhileConstantDomain result;
    for(WhileConstantDomain r : inputs)
    {
      for(const auto&[idx, value] : r)
      {
        WhileRangeValue &resultvalue = result[idx];
        resultvalue = join(resultvalue, value);
      }
    }

    return result;
  }
};


struct WhileValueRangeAnalysis : public WhileAnalysis
{
  void analyze(const WhileProgram &p) override
  {
    WhileConstant WCRA;
    WCRA.analyze(p);
    WCRA.dump(std::cout, p);
  };

  WhileValueRangeAnalysis() : WhileAnalysis("WVRA",
                                                  "Value Register Analysis")
  {
  }
};

WhileValueRangeAnalysis WVRA;
