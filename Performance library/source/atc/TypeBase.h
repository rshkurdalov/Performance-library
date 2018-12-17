// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once

typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long long uint64;
typedef char char8;
typedef char16_t char16;
typedef char32_t char32;
typedef wchar_t wchar;
typedef float float32;
typedef double float64;

namespace atc
{
	template <
		typename ConditionalType,
		bool Condition> struct _ReturnTypeIf
	{
		using Type = ConditionalType;
	};
	template <typename ConditionalType>
	struct _ReturnTypeIf<ConditionalType, false> {};
	template <typename ConditionalType, bool Condition>
	using ReturnTypeIf = typename _ReturnTypeIf<ConditionalType, Condition>::Type;

	template <typename ConvertableType> struct _AppendLValue
	{
		typedef ConvertableType & Type;
	};
	template <typename ConvertableType>
	using AppendLValue = typename _AppendLValue<ConvertableType>::Type;

	template <typename ConvertableType> struct _AppendRValue
	{
		typedef ConvertableType && Type;
	};
	template <typename ConvertableType>
	using AppendRValue = typename _AppendRValue<ConvertableType>::Type;

	template <typename Type> AppendRValue<Type> Declval() {}

	template <
		typename Type1,
		typename Type2,
		typename ...Args> struct _GetResultingType
	{
		static decltype(Declval<bool>() ?
			Declval<Type1>()
			: Declval<decltype(_GetResultingType<Type2, Args...>::Declval())>()) Declval() {}
	};
	template <
		typename Type1,
		typename Type2> struct _GetResultingType<Type1, Type2>
	{
		static decltype(Declval<bool>() ?
			Declval<Type1>() : Declval<Type2>()) Declval() {}
	};
	template <
		typename Type1,
		typename Type2,
		typename ...Args>
		using GetResultingType = typename decltype(_GetResultingType<Type1, Type2, Args...>::Declval());

	template<typename ConvertableType> struct _RemoveReference
	{
		typedef ConvertableType Type;
	};
	template<typename ConvertableType> struct _RemoveReference<ConvertableType &>
	{
		typedef ConvertableType Type;
	};
	template<typename ConvertableType> struct _RemoveReference<ConvertableType &&>
	{
		typedef ConvertableType Type;
	};
	template<typename ConvertableType>
	using RemoveReference = typename _RemoveReference<ConvertableType>::Type;

	template<typename ResultType, typename ValueType>
	constexpr inline ResultType &Reinterpret(ValueType &variable)
	{
		return *reinterpret_cast<ResultType *>(&variable);
	};
	template<typename ResultType, typename ValueType>
	constexpr inline ResultType Reinterpret(ValueType &&variable)
	{
		return *reinterpret_cast<ResultType *>(&variable);
	};

	template <typename Type> struct _IsIntegral
	{
		static constexpr bool value = false;
	};
	template <> struct _IsIntegral<char16>
	{
		static constexpr bool value = true;
	};
	template <> struct _IsIntegral<char32>
	{
		static constexpr bool value = true;
	};
	template <> struct _IsIntegral<int8>
	{
		static constexpr bool value = true;
	};
	template <> struct _IsIntegral<uint8>
	{
		static constexpr bool value = true;
	};
	template <> struct _IsIntegral<int16>
	{
		static constexpr bool value = true;
	};
	template <> struct _IsIntegral<uint16>
	{
		static constexpr bool value = true;
	};
	template <> struct _IsIntegral<int32>
	{
		static constexpr bool value = true;
	};
	template <> struct _IsIntegral<uint32>
	{
		static constexpr bool value = true;
	};
	template <> struct _IsIntegral<int64>
	{
		static constexpr bool value = true;
	};
	template <> struct _IsIntegral<uint64>
	{
		static constexpr bool value = true;
	};
	template <typename Type>
	constexpr bool IsIntegral = _IsIntegral<Type>::value;

	template <typename Type> struct _IsFloating
	{
		static constexpr bool value = false;
	};
	template <> struct _IsFloating<float32>
	{
		static constexpr bool value = true;
	};
	template <> struct _IsFloating<float64>
	{
		static constexpr bool value = true;
	};
	template <typename Type>
	constexpr bool IsFloating = _IsFloating<Type>::value;

	template <typename Type>
	constexpr bool IsScalar = IsIntegral<Type> || IsFloating<Type>;
}
