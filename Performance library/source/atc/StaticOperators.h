// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "atc\TypeBase.h"

namespace atc
{
#define EmitAssignOperator(Name, Operation) \
template < \
	typename Arg1, \
	typename Arg2> inline auto Name(Arg1 &arg1, Arg2 arg2) \
		-> decltype(Operation(arg1, arg2)) \
	{ \
		return Operation(arg1, arg2); \
	}
#define PrimitiveMove(arg1, arg2) (arg1 = arg2)
	EmitAssignOperator(Move, PrimitiveMove);
#undef PrimitiveMove
#define PrimitiveAdd(arg1, arg2) (arg1 += arg2)
	EmitAssignOperator(Add, PrimitiveAdd);
#undef PrimitiveAdd
#define PrimitiveSub(arg1, arg2) (arg1 -= arg2)
	EmitAssignOperator(Sub, PrimitiveSub);
#undef PrimitiveSub
#define PrimitiveMul(arg1, arg2) (arg1 *= arg2)
	EmitAssignOperator(Mul, PrimitiveMul);
#undef PrimitiveMul
#define PrimitiveDiv(arg1, arg2) (arg1 /= arg2)
	EmitAssignOperator(Div, PrimitiveDiv);
#undef PrimitiveDiv
#define PrimitiveModDiv(arg1, arg2) (arg1 %= arg2)
	EmitAssignOperator(ModDiv, PrimitiveModDiv);
#undef PrimitiveModDiv
#define PrimitiveAnd(arg1, arg2) (arg1 &= arg2)
	EmitAssignOperator(And, PrimitiveAnd);
#undef PrimitiveAnd
#define PrimitiveOr(arg1, arg2) (arg1 |= arg2)
	EmitAssignOperator(Or, PrimitiveOr);
#undef PrimitiveOr
#define PrimitiveXor(arg1, arg2) (arg1 ^= arg2)
	EmitAssignOperator(Xor, PrimitiveXor);
#undef PrimitiveXor
#define PrimitiveLsh(arg1, arg2) (arg1 <<= arg2)
	EmitAssignOperator(Lsh, PrimitiveLsh);
#undef PrimitiveLsh
#define PrimitiveRsh(arg1, arg2) (arg1 >>= arg2)
	EmitAssignOperator(Rsh, PrimitiveRsh);
#undef PrimitiveRsh
#define PrimitiveLogicalAnd(arg1, arg2) (arg1 = arg1 && arg2)
	EmitAssignOperator(LogicalAnd, PrimitiveLogicalAnd);
#undef PrimitiveLogicalAnd
#define PrimitiveLogicalOr(arg1, arg2) (arg1 = arg1 || arg2)
	EmitAssignOperator(LogicalOr, PrimitiveLogicalOr);
#undef PrimitiveLogicalOr
#undef EmitAssignOperator

#define EmitUnaryAssignOperator(Name, Operation) \
template <typename Arg> inline auto Name(Arg &arg) \
		-> decltype(Operation(arg)) \
	{ \
		return Operation(arg); \
	}
#define PrimitiveInc(arg) (arg++)
	EmitUnaryAssignOperator(Inc, PrimitiveInc);
#undef PrimitiveInc
#define PrimitivePreInc(arg) (++arg)
	EmitUnaryAssignOperator(PreInc, PrimitivePreInc);
#undef PrimitivePreInc
#define PrimitiveDec(arg) (arg--)
	EmitUnaryAssignOperator(Dec, PrimitiveDec);
#undef PrimitiveDec
#define PrimitivePreDec(arg) (--arg)
	EmitUnaryAssignOperator(PreDec, PrimitivePreDec);
#undef PrimitivePreDec
#define PrimitiveIncrement(arg) (arg++)
	EmitUnaryAssignOperator(Increment, PrimitiveIncrement);
#undef PrimitiveIncrement
#define PrimitiveNegate(arg) (arg = -arg)
	EmitUnaryAssignOperator(Negate, PrimitiveNegate);
#undef PrimitiveNegate
#define PrimitiveInvert(arg) (arg = ~arg)
	EmitUnaryAssignOperator(Invert, PrimitiveInvert);
#undef PrimitiveInvert
#define PrimitiveLogicalInvert(arg) (arg = !arg)
	EmitUnaryAssignOperator(LogicalInvert, PrimitiveLogicalInvert);
#undef PrimitiveLogicalInvert
#undef EmitUnaryAssignOperator

#define EmitValueOperator(Name, Operation) \
template < \
	typename Arg1, \
	typename Arg2> constexpr inline auto Name(Arg1 arg1, Arg2 arg2) \
		-> RemoveReference<decltype(Operation(arg1, arg2))> \
	{ \
		return Operation(arg1, arg2); \
	} \
template < \
	typename Arg, \
	typename ...Args> constexpr inline auto Name(Arg arg, Args... args) \
		-> RemoveReference<decltype(Operation(arg, Name<Args...>(args...)))> \
	{ \
		return Operation(arg, Name<Args...>(args...)); \
	}
#define PrimitiveMin(arg1, arg2) ((arg1) < (arg2) ? (arg1) : (arg2))
	EmitValueOperator(Min, PrimitiveMin);
#undef PrimitiveMin
#define PrimitiveMax(arg1, arg2) ((arg1) < (arg2) ? (arg2) : (arg1))
	EmitValueOperator(Max, PrimitiveMax);
#undef PrimitiveMax
#define PrimitiveSum(arg1, arg2) (arg1 + arg2)
	EmitValueOperator(Sum, PrimitiveSum);
#undef PrimitiveSum
#define PrimitiveDiff(arg1, arg2) (arg1 - arg2)
	EmitValueOperator(Diff, PrimitiveDiff);
#undef PrimitiveDiff
#define PrimitiveProduct(arg1, arg2) (arg1 * arg2)
	EmitValueOperator(Product, PrimitiveProduct);
#undef PrimitiveProduct
#define PrimitiveQuotient(arg1, arg2) (arg1 / arg2)
	EmitValueOperator(Quotient, PrimitiveQuotient);
#undef PrimitiveQuotient
#define PrimitiveModQuotient(arg1, arg2) (arg1 % arg2)
	EmitValueOperator(ModQuotient, PrimitiveModQuotient);
#undef PrimitiveModQuotient
#define PrimitiveAndProduct(arg1, arg2) (arg1 & arg2)
	EmitValueOperator(AndProduct, PrimitiveAndProduct);
#undef PrimitiveAndProduct
#define PrimitiveOrProduct(arg1, arg2) (arg1 | arg2)
	EmitValueOperator(OrProduct, PrimitiveOrProduct);
#undef PrimitiveOrProduct
#define PrimitiveXorProduct(arg1, arg2) (arg1 ^ arg2)
	EmitValueOperator(XorProduct, PrimitiveXorProduct);
#undef PrimitiveXorProduct
#define PrimitiveLshProduct(arg1, arg2) (arg1 << arg2)
	EmitValueOperator(LshProduct, PrimitiveLshProduct);
#undef PrimitiveLshProduct
#define PrimitiveRshProduct(arg1, arg2) (arg1 >> arg2)
	EmitValueOperator(RshProduct, PrimitiveRshProduct);
#undef PrimitiveRshProduct
#define PrimitiveLogicalAndProduct(arg1, arg2) (arg1 && arg2)
	EmitValueOperator(LogicalAndProduct, PrimitiveLogicalAndProduct);
#undef PrimitiveLogicalAndProduct
#define PrimitiveLogicalOrProduct(arg1, arg2) (arg1 || arg2)
	EmitValueOperator(LogicalOrProduct, PrimitiveLogicalOrProduct);
#undef PrimitiveLogicalOrProduct
#undef EmitValueOperator

#define EmitUnaryValueOperator(Name, Operation) \
template <typename Arg> constexpr inline auto Name(Arg arg) \
		-> decltype(Operation(arg)) \
	{ \
		return Operation(arg); \
	}
#define PrimitiveNegative(arg) (-arg)
	EmitUnaryValueOperator(Negative, PrimitiveNegative);
#undef PrimitiveNegative
#define PrimitiveInvertion(arg) (~arg)
	EmitUnaryValueOperator(Invertion, PrimitiveInvertion);
#undef PrimitiveInvertion
#define PrimitiveNot(arg) (!arg)
	EmitUnaryValueOperator(Not, PrimitiveNot);
#undef PrimitiveNot
#undef EmitUnaryValueOperator

#define EmitComparisonOperator(Name, Operation) \
template < \
	typename Arg1, \
	typename Arg2> constexpr inline auto Name(Arg1 arg1, Arg2 arg2) \
		-> decltype(Operation(arg1, arg2)) \
	{ \
		return Operation(arg1, arg2); \
	}
#define PrimitiveEqual(arg1, arg2) (arg1 == arg2)
	EmitComparisonOperator(Equal, PrimitiveEqual);
#undef PrimitiveEqual
#define PrimitiveNotEqual(arg1, arg2) (arg1 != arg2)
	EmitComparisonOperator(NotEqual, PrimitiveNotEqual);
#undef PrimitiveNotEqual
#define PrimitiveLess(arg1, arg2) (arg1 < arg2)
	EmitComparisonOperator(Less, PrimitiveLess);
#undef PrimitiveLess
#define PrimitiveGreater(arg1, arg2) (arg1 > arg2)
	EmitComparisonOperator(Greater, PrimitiveGreater);
#undef PrimitiveGreater
#define PrimitiveLessEqual(arg1, arg2) (arg1 <= arg2)
	EmitComparisonOperator(LessEqual, PrimitiveLessEqual);
#undef PrimitiveLess
#define PrimitiveGreaterEqual(arg1, arg2) (arg1 >= arg2)
	EmitComparisonOperator(GreaterEqual, PrimitiveGreaterEqual);
#undef PrimitiveGreaterEqual
#undef EmitComparisonOperator

	template <typename LeftTy, typename RightTy>
	inline void Swap(LeftTy &left, RightTy &right)
	{
		LeftTy temp = static_cast<LeftTy &&>(left);
		left = static_cast<RightTy &&>(right);
		right = static_cast<LeftTy &&>(temp);
	}

	// Static operator ForEach
	template <
		uint32 IterationCount,
		typename TupleOperator,
		typename ...Args>
		inline auto ForEach(
			TupleOperator tupleOperator,
			Args... args)
		-> ReturnTypeIf<void, IterationCount != 1>
	{
		tupleOperator(*(args)...);
		ForEach<IterationCount - 1>(
			tupleOperator,
			(args + 1)...);
	}
	template <
		uint32 IterationCount,
		typename TupleOperator,
		typename ...Args>
		inline auto ForEach(
			TupleOperator tupleOperator,
			Args... args)
		-> ReturnTypeIf<void, IterationCount == 1>
	{
		tupleOperator(*(args)...);
	}

	// Static operator Assign
	template <
		uint32 IterationCount,
		typename AssigneeIterator,
		typename AssignableIterator>
		inline ReturnTypeIf<void, IterationCount != 1> Assign(
			AssigneeIterator assigneeIterator,
			const AssignableIterator assignableIterator)
	{
		*assigneeIterator =
			static_cast<RemoveReference<
			decltype(*assigneeIterator)>>(*assignableIterator);
		Assign<IterationCount - 1>(
			assigneeIterator + 1,
			assignableIterator + 1);
	}
	template <
		uint32 IterationCount,
		typename AssigneeIterator,
		typename AssignableIterator>
		inline ReturnTypeIf<void, IterationCount == 1> Assign(
			AssigneeIterator assigneeIterator,
			const AssignableIterator assignableIterator)
	{
		*assigneeIterator =
			static_cast<RemoveReference<
			decltype(*assigneeIterator)>>(*assignableIterator);
	}

	// Static tuple operator Assign
	template <
		uint32 IterationCount,
		typename TupleOperator,
		typename AssigneeIterator,
		typename ...Args>
		inline ReturnTypeIf<void, IterationCount != 1> Assign(
			TupleOperator tupleOperator,
			AssigneeIterator assigneeIterator,
			const Args... args)
	{
		*assigneeIterator =
			static_cast<RemoveReference<
			decltype(*assigneeIterator)>>(tupleOperator(*args...));
		Assign<IterationCount - 1>(
			tupleOperator,
			assigneeIterator + 1,
			(args + 1)...);
	}
	template <
		uint32 IterationCount,
		typename TupleOperator,
		typename AssigneeIterator,
		typename ...Args>
		inline ReturnTypeIf<void, IterationCount == 1> Assign(
			TupleOperator tupleOperator,
			AssigneeIterator assigneeIterator,
			const Args... args)
	{
		*assigneeIterator =
			static_cast<RemoveReference<
			decltype(*assigneeIterator)>>(tupleOperator(*args...));
	}

	// Operator Accumulate
	template <
		typename ValueType,
		typename AccumulativeOperator,
		typename Iterator>
		inline ValueType Accumulate(
			AccumulativeOperator accumulativeOperator,
			ValueType initialValue,
			Iterator iteratorFirst,
			Iterator iteratorLast)
	{
		while (iteratorFirst != iteratorLast)
			accumulativeOperator(initialValue, *iteratorFirst++);
		return initialValue;
	}

	// Tuple operator Accumulate
	template <
		typename ValueType,
		typename TupleOperator,
		typename AccumulativeOperator,
		typename ...Args>
		inline ValueType Accumulate(
			TupleOperator tupleOperator,
			AccumulativeOperator accumulativeOperator,
			ValueType initialValue,
			uint32 iterationCount,
			Args... args)
	{
		while (iterationCount-- != 0)
			accumulativeOperator(initialValue, tupleOperator(*iterator++));
	}

	// Static operator Accumulate
	template <
		uint32 IterationCount,
		typename ValueType,
		typename AccumulativeOperator,
		typename Iterator>
		inline auto Accumulate(
			AccumulativeOperator accumulativeOperator,
			ValueType initialValue,
			Iterator iterator)
		-> ReturnTypeIf<ValueType, IterationCount != 1>
	{
		return Accumulate<
			IterationCount - 1,
			ValueType,
			AccumulativeOperator,
			Iterator>(
				accumulativeOperator,
				accumulativeOperator(
					initialValue,
					*iterator),
					iterator + 1);
	}
	template <
		uint32 IterationCount,
		typename ValueType,
		typename AccumulativeOperator,
		typename Iterator>
		inline auto Accumulate(
			AccumulativeOperator accumulativeOperator,
			ValueType initialValue,
			Iterator iterator)
		-> ReturnTypeIf<ValueType, IterationCount == 1>
	{
		return accumulativeOperator(
			initialValue,
			*iterator);
	}

	// Static tuple operator Accumulate
	template <
		uint32 IterationCount,
		typename ValueType,
		typename TupleOperator,
		typename AccumulativeOperator,
		typename ...Args>
		inline auto Accumulate(
			TupleOperator tupleOperator,
			AccumulativeOperator accumulativeOperator,
			ValueType initialValue,
			Args... args)
		-> ReturnTypeIf<ValueType, IterationCount != 1 && sizeof...(Args) != 0>
	{
		return Accumulate<
			IterationCount - 1,
			ValueType,
			TupleOperator,
			AccumulativeOperator,
			Args...>(
				tupleOperator,
				accumulativeOperator,
				accumulativeOperator(
					initialValue,
					tupleOperator(*(args)...)),
					(args + 1)...);
	}
	template <
		uint32 IterationCount,
		typename ValueType,
		typename TupleOperator,
		typename AccumulativeOperator,
		typename ...Args>
		inline auto Accumulate(
			TupleOperator tupleOperator,
			AccumulativeOperator accumulativeOperator,
			ValueType initialValue,
			Args... args)
		-> ReturnTypeIf<ValueType, IterationCount == 1 && sizeof...(Args) != 0>
	{
		return accumulativeOperator(
			initialValue,
			tupleOperator(*(args)...));
	}

	// Static tuple operator TestProductAnd
	template <
		uint32 IterationCount,
		typename TupleOperator,
		typename ...Args>
		inline auto TestProductAnd(
			TupleOperator tupleOperator,
			Args... args)
		-> ReturnTypeIf<bool, IterationCount != 1>
	{
		return tupleOperator(*(args)...) ?
			TestProductAnd<IterationCount-1>(tupleOperator, (args + 1)...) : false;
	}
	template <
		uint32 IterationCount,
		typename TupleOperator,
		typename ...Args>
		inline auto TestProductAnd(
			TupleOperator tupleOperator,
			Args... args)
		-> ReturnTypeIf<bool, IterationCount == 1>
	{
		return tupleOperator(*(args)...);
	}

	// Static tuple operator TestProductOr
	template <
		uint32 IterationCount,
		typename TupleOperator,
		typename ...Args>
		inline auto TestProductOr(
			TupleOperator tupleOperator,
			Args... args)
		-> ReturnTypeIf<bool, IterationCount != 1>
	{
		return tupleOperator(*(args)...) ?
			true : TestProductOr<IterationCount - 1>(tupleOperator, (args + 1)...);
	}
	template <
		uint32 IterationCount,
		typename TupleOperator,
		typename ...Args>
		inline auto TestProductOr(
			TupleOperator tupleOperator,
			Args... args)
		-> ReturnTypeIf<bool, IterationCount == 1>
	{
		return tupleOperator(*(args)...);
	}
}
