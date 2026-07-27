#pragma once

#include "RecordedDataDefines.h"
#include <utility>
#include <type_traits>

//namespace RecordedDataVisitorPrivate
//{
//	// 泛型参数提取工具
//	template <typename T>
//	struct GetLambdaParam;
//
//	// 匹配常规 Lambda (operator() 是 const 的)
//	template <typename ClassType, typename ReturnType, typename ParamType>
//	struct GetLambdaParam<ReturnType(ClassType::*)(ParamType) const> {
//		using Type = ParamType;
//	};
//
//	// 匹配带 mutable 的 Lambda (operator() 不是 const)
//	template <typename ClassType, typename ReturnType, typename ParamType>
//	struct GetLambdaParam<ReturnType(ClassType::*)(ParamType)> {
//		using Type = ParamType;
//	};
//
//	template <typename LambdaToken>
//	struct GetFirstArgument {
//		// 【修复 1】使用 std::decay_t 移除 LambdaToken 的引用修饰，确保能正确获取其 operator()
//		using CleanToken = std::decay_t<LambdaToken>;
//		using Type = typename GetLambdaParam<decltype(&CleanToken::operator())>::Type;
//	};
//
//	// 提取真正的目标类（无论传入的是指针还是引用）
//	template <typename T>
//	struct ExtractTargetClass {
//		using WithoutRef = std::remove_reference_t<T>;
//		using WithoutPtr = std::remove_pointer_t<WithoutRef>;
//		// 【核心修改】确保 TargetClass 不带 const，让 CustomCast 内部好做 static_cast
//		using Type = std::remove_const_t<WithoutPtr>;
//	};
//}
//
//namespace RecordedDataVisitor
//{
//	// 1. overloaded 基础设施
//	template<class... Ts> struct TOverloaded : Ts... { using Ts::operator()...; };
//	template<class... Ts> TOverloaded(Ts...) -> TOverloaded<Ts...>;
//
//	// 3. Visit 的核心实现 (非常量版本)
//	template <typename... Fs>
//	void Visit(TOverloaded<Fs...> Visitor, IRecordedDataObjectInterface* BasePtr)
//	{
//		if (!BasePtr) return;
//
//		bool bHandled = false;
//
//		auto TryCastAndCall = [&bHandled](auto& SingleVisitor) {
//			if (bHandled) return;
//
//			using RawParamType = typename RecordedDataVisitorPrivate::GetFirstArgument<decltype(SingleVisitor)>::Type;
//			using TargetClass = typename RecordedDataVisitorPrivate::ExtractTargetClass<RawParamType>::Type;
//
//			if (auto* CastedPtr = CustomCast<TargetClass>(BasePtr))
//			{
//				// 【修复 2】智能适配：如果 Lambda 期望指针，传指针；如果期望引用，解引用传过去
//				if constexpr (std::is_pointer_v<std::remove_reference_t<RawParamType>>)
//				{
//					SingleVisitor(CastedPtr);
//				}
//				else
//				{
//					SingleVisitor(*CastedPtr);
//				}
//				bHandled = true;
//			}
//			};
//
//		(TryCastAndCall(static_cast<Fs&>(Visitor)), ...);
//	}
//
//	// Visit 的核心实现 (常量版本)
//	template <typename... Fs>
//	void Visit(TOverloaded<Fs...> Visitor, const IRecordedDataObjectInterface* BasePtr)
//	{
//		if (!BasePtr) return;
//
//		bool bHandled = false;
//
//		auto TryCastAndCall = [&](auto& SingleVisitor) {
//			if (bHandled) return;
//
//			using RawParamType = typename RecordedDataVisitorPrivate::GetFirstArgument<decltype(SingleVisitor)>::Type;
//			using TargetClass = typename RecordedDataVisitorPrivate::ExtractTargetClass<RawParamType>::Type;
//
//			UE_LOG(LogTemp, Error, TEXT("Visit BasePtr->GetClassID() %d"), BasePtr->GetClassID());
//
//			if (auto* CastedPtr = CustomCast<TargetClass>(BasePtr))
//			{
//				// 【修复 2】智能适配：根据 Lambda 参数类型决定传 CastedPtr 还是 *CastedPtr
//				if constexpr (std::is_pointer_v<std::remove_reference_t<RawParamType>>)
//				{
//					SingleVisitor(CastedPtr);
//				}
//				else
//				{
//					SingleVisitor(*CastedPtr);
//				}
//				bHandled = true;
//			}
//			};
//
//		(TryCastAndCall(static_cast<Fs&>(Visitor)), ...);
//	}
//
//	// Visit 的核心实现 (单 Lambda 版本)
//	template <typename F>
//	void Visit(F Visitor, IRecordedDataObjectInterface* BasePtr)
//	{
//		if (!BasePtr) return;
//
//		bool bHandled = false;
//
//		auto TryCastAndCall = [&](auto& SingleVisitor) {
//			if (bHandled) return;
//
//			using RawParamType = typename RecordedDataVisitorPrivate::GetFirstArgument<decltype(SingleVisitor)>::Type;
//			using TargetClass = typename RecordedDataVisitorPrivate::ExtractTargetClass<RawParamType>::Type;
//
//			UE_LOG(LogTemp, Error, TEXT("Visit BasePtr->GetClassID() %d"), BasePtr->GetClassID());
//
//			if (auto* CastedPtr = CustomCast<TargetClass>(BasePtr))
//			{
//				// 【修复 2】智能适配：根据 Lambda 参数类型决定传 CastedPtr 还是 *CastedPtr
//				if constexpr (std::is_pointer_v<std::remove_reference_t<RawParamType>>)
//				{
//					SingleVisitor(CastedPtr);
//				}
//				else
//				{
//					SingleVisitor(*CastedPtr);
//				}
//				bHandled = true;
//			}
//			};
//
//		(TryCastAndCall(static_cast<F&>(Visitor)));
//	}
//
//	// Visit 的核心实现 (单 Lambda 版本, 常量)
//	template <typename F>
//	void Visit(F Visitor, const IRecordedDataObjectInterface* BasePtr)
//	{
//		if (!BasePtr) return;
//
//		bool bHandled = false;
//
//		auto TryCastAndCall = [&](auto& SingleVisitor) {
//			if (bHandled) return;
//
//			using RawParamType = typename RecordedDataVisitorPrivate::GetFirstArgument<decltype(SingleVisitor)>::Type;
//			using TargetClass = typename RecordedDataVisitorPrivate::ExtractTargetClass<RawParamType>::Type;
//
//			UE_LOG(LogTemp, Error, TEXT("Visit BasePtr->GetClassID() %d"), BasePtr->GetClassID());
//
//			if (auto* CastedPtr = CustomCast<TargetClass>(BasePtr))
//			{
//				// 【修复 2】智能适配：根据 Lambda 参数类型决定传 CastedPtr 还是 *CastedPtr
//				if constexpr (std::is_pointer_v<std::remove_reference_t<RawParamType>>)
//				{
//					SingleVisitor(CastedPtr);
//				}
//				else
//				{
//					SingleVisitor(*CastedPtr);
//				}
//				bHandled = true;
//			}
//			};
//
//		(TryCastAndCall(static_cast<F&>(Visitor)));
//	}
//}

// Overloaded 模式匹配工具
template<class... Ts> struct TOverloaded : Ts... { using Ts::operator()...; };
template<class... Ts> TOverloaded(Ts...) -> TOverloaded<Ts...>;

namespace RecordedDataVisitorPrivate
{
	// 泛型参数提取工具
	template <typename T>
	struct GetLambdaParam;

	// 匹配普通 Lambda (operator() 是 const)
	template <typename ClassType, typename ReturnType, typename ParamType>
	struct GetLambdaParam<ReturnType(ClassType::*)(ParamType) const> {
		using Type = ParamType;
	};

	// 匹配带 mutable 的 Lambda (operator() 不是 const)
	template <typename ClassType, typename ReturnType, typename ParamType>
	struct GetLambdaParam<ReturnType(ClassType::*)(ParamType)> {
		using Type = ParamType;
	};

	template <typename LambdaToken>
	struct GetFirstArgument {
		// 使用 std::decay_t 移除 LambdaToken 的引用修饰，确保能正确获取其 operator()
		using CleanToken = std::decay_t<LambdaToken>;
		using Type = typename GetLambdaParam<decltype(&CleanToken::operator())>::Type;
	};

	// 提取真正的目标类（无论传入的是指针还是引用）
	template <typename T>
	struct ExtractTargetClass {
		using WithoutRef = std::remove_reference_t<T>;
		using WithoutPtr = std::remove_pointer_t<WithoutRef>;
		// 确保 TargetClass 不带 const，让 CustomCast 内部好做 static_cast
		using Type = std::remove_const_t<WithoutPtr>;
	};

	template <typename TBasePtr, typename F>
	bool TryCastAndCallSingle(F& SingleVisitor, TBasePtr* BasePtr)
	{
		static_assert(std::is_base_of_v<IRecordedDataObjectInterface, std::remove_const_t<TBasePtr>>);

		using RawParamType = typename GetFirstArgument<decltype(SingleVisitor)>::Type;
		using TargetClass = typename ExtractTargetClass<RawParamType>::Type;

		 //UE_LOG(LogTemp, Error, TEXT("Visit BasePtr->GetClassID() %d"), BasePtr->GetClassID());

		// 保持原有的 const 属性进行转换
		if (auto* CastedPtr = CustomCast<TargetClass>(BasePtr))
		{
			// 智能适配：根据 Lambda 参数类型决定传指针还是引用
			if constexpr (std::is_pointer_v<std::remove_reference_t<RawParamType>>)
			{
				SingleVisitor(CastedPtr);
			}
			else
			{
				SingleVisitor(*CastedPtr);
			}
			return true; // 成功处理
		}
		return false; // 未匹配成功
	}
}

namespace RecordedDataVisitor
{
	// 1. 多 Lambda (TOverloaded) 版本 —— 支持 const 和非 const
	template <typename... Fs, typename TBasePtr>
	void Visit(TOverloaded<Fs...> Visitor, TBasePtr* BasePtr)
	{
		static_assert(std::is_base_of_v<IRecordedDataObjectInterface, std::remove_const_t<TBasePtr>>, "BasePtr must derive from IRecordedDataObjectInterface");

		if (!BasePtr) return;

		bool bHandled = false;
		// 利用折叠表达式遍历每一个 Lambda，一旦 bHandled 为 true 则后续短路（不再执行强转）
		(
			(bHandled || (bHandled = RecordedDataVisitorPrivate::TryCastAndCallSingle(static_cast<Fs&>(Visitor), BasePtr))),
			...
			);
	}

	// 2. 单 Lambda 版本 —— 支持 const 和非 const
	// 使用 std::enable_if_t 避免它与 TOverloaded 产生歧义（当只传一个 Lambda 时）
	template <typename F, typename TBasePtr,
		typename = std::enable_if_t<!std::is_base_of_v<IRecordedDataObjectInterface, std::remove_pointer_t<F>>>>
		void Visit(F Visitor, TBasePtr* BasePtr)
	{
		static_assert(std::is_base_of_v<IRecordedDataObjectInterface, std::remove_const_t<TBasePtr>>, "BasePtr must derive from IRecordedDataObjectInterface");

		if (!BasePtr) return;

		RecordedDataVisitorPrivate::TryCastAndCallSingle(static_cast<F&>(Visitor), BasePtr);
	}
}