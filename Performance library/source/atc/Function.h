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
		static constexpr uint32 ResultSize = sizeof(ResultType);
		static constexpr uint32 ArgsCount = sizeof...(Args);
		static constexpr uint32 ArgsSize = ArgsSize<Args...>::Value;

		void *fptr;
	};
	template <typename ...Args> struct FunctionBase<void, Args...>
	{
		static constexpr bool IsFunction = true;
		static constexpr uint32 ResultSize = 0;
		static constexpr uint32 ArgsCount = sizeof...(Args);
		static constexpr uint32 ArgsSize = ArgsSize<Args...>::Value;

		void *fptr;
	};
	template <typename ResultType> struct FunctionBase<ResultType>
	{
		static constexpr bool IsFunction = true;
		static constexpr uint32 ResultSize = sizeof(ResultType);
		static constexpr uint32 ArgsCount = 0;
		static constexpr uint32 ArgsSize = 0;

		void *fptr;
	};
	template <> struct FunctionBase<void>
	{
		static constexpr bool IsFunction = true;
		static constexpr uint32 ResultSize = 0;
		static constexpr uint32 ArgsCount = 0;
		static constexpr uint32 ArgsSize = 0;

		void *fptr;
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
		static constexpr uint32 CallConvention = CallConventionCdecl;
		ResultType inline operator () (Args... args)
		{
			return ((ResultType (__cdecl *)(Args...))fptr)(args...);
		}
	};
	template <
		typename ResultType,
		typename ...Args> struct FunctionInvoker<ResultType __stdcall (Args...)>
		: FunctionBase<ResultType, Args...>
	{
		static constexpr uint32 CallConvention = CallConventionStdcall;
		ResultType inline operator () (Args... args)
		{
			return ((ResultType (__stdcall *)(Args...))fptr)(args...);
		}
	};
	template <
		typename ResultType,
		typename ...Args> struct FunctionInvoker<ResultType __fastcall (Args...)>
		: FunctionBase<ResultType, Args...>
	{
		static constexpr uint32 CallConvention = CallConventionFastcall;
		ResultType inline operator () (Args... args)
		{
			return ((ResultType(__fastcall *)(Args...))fptr)(args...);
		}
	};
	template <
		typename ResultType,
		typename ...Args> struct FunctionInvoker<ResultType __vectorcall (Args...)>
		: FunctionBase<ResultType, Args...>
	{
		static constexpr uint32 CallConvention = CallConventionVectorcall;
		ResultType inline operator () (Args... args)
		{
			return ((ResultType (__vectorcall *)(Args...))fptr)(args...);
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
				&& ResultSize == AssignableFunction::ResultSize
				&& ArgsSize == AssignableFunction::ArgsSize
				&& CallConvention == AssignableFunction::CallConvention,
				"Function is not compatible");
			fptr = (void *)function;
		}
		template <> void Assign<FunctionType>(FunctionType *function)
		{
			fptr = (void *)function;
		}
		Function() {}
		Function(Function<FunctionType> &object)
		{
			fptr = object.fptr;
		}
		template <typename Type> Function(Type *function)
		{
			Assign<Type>(function);
		}
		void operator=(Function<FunctionType> object)
		{
			fptr = object.fptr;
		}
		template <typename Type> void operator=(Type *function)
		{
			Assign<Type>(function);
		}
	};
}
