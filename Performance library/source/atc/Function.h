// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "atc\TypeBase.h"

namespace atc
{
	template <typename Arg, typename ...Args> struct ArgsSize
	{
		static constexpr uint32 Value = sizeof(Arg) + ArgsSize<Args...>::Value;
	};
	template <typename Arg> struct ArgsSize<Arg>
	{
		static constexpr uint32 Value = sizeof(Arg);
	};

	template <
		typename ResultType,
		typename ...Args> struct FunctionBase
	{
		static constexpr bool IsFunction = true;
		static constexpr uint32 FunctionResultSize = sizeof(ResultType);
		static constexpr uint32 FunctionArgsCount = sizeof...(Args);
		static constexpr uint32 FunctionArgsSize = ArgsSize<Args...>::Value;

		void *Ptr;
	};
	template <typename ...Args> struct FunctionBase<void, Args...>
	{
		static constexpr bool IsFunction = true;
		static constexpr uint32 FunctionResultSize = 0;
		static constexpr uint32 FunctionArgsCount = sizeof...(Args);
		static constexpr uint32 FunctionArgsSize = ArgsSize<Args...>::Value;

		void *Ptr;
	};

	enum CallConvention
	{
		CallConventionCdecl,
		CallConventionStdcall,
		CallConventionFastcall,
		CallConventionVectorcall
	};

	template <typename FunctionType> struct FunctionInvoker
	{
		static constexpr bool IsFunction = false;
		static_assert(IsFunction, "Given type is not a function");
	};
	template <
		typename ResultType,
		typename ...Args> struct FunctionInvoker<ResultType __cdecl (Args...)>
		: FunctionBase<ResultType, Args...>
	{
		static constexpr CallConvention FunctionCallConvention = CallConventionCdecl;
		ResultType inline operator () (Args... args)
		{
			return ((ResultType (__cdecl *)(Args...))Ptr)(args...);
		}
	};
	template <
		typename ResultType,
		typename ...Args> struct FunctionInvoker<ResultType __stdcall (Args...)>
		: FunctionBase<ResultType, Args...>
	{
		static constexpr CallConvention FunctionCallConvention = CallConventionStdcall;
		ResultType inline operator () (Args... args)
		{
			return ((ResultType (__stdcall *)(Args...))Ptr)(args...);
		}
	};
	template <
		typename ResultType,
		typename ...Args> struct FunctionInvoker<ResultType __fastcall (Args...)>
		: FunctionBase<ResultType, Args...>
	{
		static constexpr CallConvention FunctionCallConvention = CallConventionFastcall;
		ResultType inline operator () (Args... args)
		{
			return ((ResultType(__fastcall *)(Args...))Ptr)(args...);
		}
	};
	template <
		typename ResultType,
		typename ...Args> struct FunctionInvoker<ResultType __vectorcall (Args...)>
		: FunctionBase<ResultType, Args...>
	{
		static constexpr CallConvention FunctionCallConvention = CallConventionVectorcall;
		ResultType inline operator () (Args... args)
		{
			return ((ResultType (__vectorcall *)(Args...))Ptr)(args...);
		}
	};

	template <typename FunctionType> struct Function
		: FunctionInvoker<FunctionType>
	{
		template <typename Type> void Assign(Type *function)
		{
			typedef Function<Type> AssignableFunction;
			static_assert(
				AssignableFunction::IsFunction
				&& FunctionResultSize == AssignableFunction::FunctionResultSize
				&& FunctionArgsSize == AssignableFunction::FunctionArgsSize
				&& FunctionCallConvention == AssignableFunction::FunctionCallConvention,
				"Function is not compatible");
			Ptr = (void *)function;
		}
		template <> void Assign<FunctionType>(FunctionType *function)
		{
			Ptr = (void *)function;
		}
		Function() {}
		template <typename Type> Function(Type *function)
		{
			Assign<Type>(function);
		}
		template <typename Type> void operator=(Type *function)
		{
			Assign<Type>(function);
		}
	};
}
