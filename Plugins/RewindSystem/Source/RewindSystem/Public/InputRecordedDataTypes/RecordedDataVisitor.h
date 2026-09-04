#pragma once

#include "RecordedDataDefines.h"
#include <utility>
#include <type_traits>

//namespace RecordedDataVisitorPrivate
//{
//	// ���Ͳ�����ȡ����
//	template <typename T>
//	struct GetLambdaParam;
//
//	// ƥ�䳣�� Lambda (operator() �� const ��)
//	template <typename ClassType, typename ReturnType, typename ParamType>
//	struct GetLambdaParam<ReturnType(ClassType::*)(ParamType) const> {
//		using Type = ParamType;
//	};
//
//	// ƥ��� mutable �� Lambda (operator() ���� const)
//	template <typename ClassType, typename ReturnType, typename ParamType>
//	struct GetLambdaParam<ReturnType(ClassType::*)(ParamType)> {
//		using Type = ParamType;
//	};
//
//	template <typename LambdaToken>
//	struct GetFirstArgument {
//		// ���޸� 1��ʹ�� std::decay_t �Ƴ� LambdaToken ���������Σ�ȷ������ȷ��ȡ�� operator()
//		using CleanToken = std::decay_t<LambdaToken>;
//		using Type = typename GetLambdaParam<decltype(&CleanToken::operator())>::Type;
//	};
//
//	// ��ȡ������Ŀ���ࣨ���۴������ָ�뻹�����ã�
//	template <typename T>
//	struct ExtractTargetClass {
//		using WithoutRef = std::remove_reference_t<T>;
//		using WithoutPtr = std::remove_pointer_t<WithoutRef>;
//		// �������޸ġ�ȷ�� TargetClass ���� const���� CustomCast �ڲ����� static_cast
//		using Type = std::remove_const_t<WithoutPtr>;
//	};
//}
//
//namespace RecordedDataVisitor
//{
//	// 1. overloaded ������ʩ
//	template<class... Ts> struct TOverloaded : Ts... { using Ts::operator()...; };
//	template<class... Ts> TOverloaded(Ts...) -> TOverloaded<Ts...>;
//
//	// 3. Visit �ĺ���ʵ�� (�ǳ����汾)
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
//				// ���޸� 2���������䣺��� Lambda ����ָ�룬��ָ�룻����������ã������ô���ȥ
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
//	// Visit �ĺ���ʵ�� (�����汾)
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
//			if (auto* CastedPtr = CustomCast<TargetClass>(BasePtr))
//			{
//				// ���޸� 2���������䣺���� Lambda �������;����� CastedPtr ���� *CastedPtr
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
//	// Visit �ĺ���ʵ�� (�� Lambda �汾)
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
//			if (auto* CastedPtr = CustomCast<TargetClass>(BasePtr))
//			{
//				// ���޸� 2���������䣺���� Lambda �������;����� CastedPtr ���� *CastedPtr
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
//	// Visit �ĺ���ʵ�� (�� Lambda �汾, ����)
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
//			if (auto* CastedPtr = CustomCast<TargetClass>(BasePtr))
//			{
//				// ���޸� 2���������䣺���� Lambda �������;����� CastedPtr ���� *CastedPtr
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

// Overloaded ģʽƥ�乤��
template<class... Ts> struct TOverloaded : Ts... { using Ts::operator()...; };
template<class... Ts> TOverloaded(Ts...) -> TOverloaded<Ts...>;

namespace RecordedDataVisitorPrivate
{
	// ���Ͳ�����ȡ����
	template <typename T>
	struct GetLambdaParam;

	// ƥ����ͨ Lambda (operator() �� const)
	template <typename ClassType, typename ReturnType, typename ParamType>
	struct GetLambdaParam<ReturnType(ClassType::*)(ParamType) const> {
		using Type = ParamType;
	};

	// ƥ��� mutable �� Lambda (operator() ���� const)
	template <typename ClassType, typename ReturnType, typename ParamType>
	struct GetLambdaParam<ReturnType(ClassType::*)(ParamType)> {
		using Type = ParamType;
	};

	template <typename LambdaToken>
	struct GetFirstArgument {
		// ʹ�� std::decay_t �Ƴ� LambdaToken ���������Σ�ȷ������ȷ��ȡ�� operator()
		using CleanToken = std::decay_t<LambdaToken>;
		using Type = typename GetLambdaParam<decltype(&CleanToken::operator())>::Type;
	};

	// ��ȡ������Ŀ���ࣨ���۴������ָ�뻹�����ã�
	template <typename T>
	struct ExtractTargetClass {
		using WithoutRef = std::remove_reference_t<T>;
		using WithoutPtr = std::remove_pointer_t<WithoutRef>;
		// ȷ�� TargetClass ���� const���� CustomCast �ڲ����� static_cast
		using Type = std::remove_const_t<WithoutPtr>;
	};

	template <typename TBasePtr, typename F>
	bool TryCastAndCallSingle(F& SingleVisitor, TBasePtr* BasePtr)
	{
		static_assert(std::is_base_of_v<IRecordedDataObjectInterface, std::remove_const_t<TBasePtr>>);

		using RawParamType = typename GetFirstArgument<decltype(SingleVisitor)>::Type;
		using TargetClass = typename ExtractTargetClass<RawParamType>::Type;

		// ����ԭ�е� const ���Խ���ת��
		if (auto* CastedPtr = CustomCast<TargetClass>(BasePtr))
		{
			// �������䣺���� Lambda �������;�����ָ�뻹������
			if constexpr (std::is_pointer_v<std::remove_reference_t<RawParamType>>)
			{
				SingleVisitor(CastedPtr);
			}
			else
			{
				SingleVisitor(*CastedPtr);
			}
			return true; // �ɹ�����
		}
		return false; // δƥ��ɹ�
	}
}

namespace RecordedDataVisitor
{
	// 1. �� Lambda (TOverloaded) �汾 ���� ֧�� const �ͷ� const
	template <typename... Fs, typename TBasePtr>
	void Visit(TOverloaded<Fs...> Visitor, TBasePtr* BasePtr)
	{
		static_assert(std::is_base_of_v<IRecordedDataObjectInterface, std::remove_const_t<TBasePtr>>, "BasePtr must derive from IRecordedDataObjectInterface");

		if (!BasePtr) return;

		bool bHandled = false;
		// �����۵�����ʽ����ÿһ�� Lambda��һ�� bHandled Ϊ true �������·������ִ��ǿת��
		(
			(bHandled || (bHandled = RecordedDataVisitorPrivate::TryCastAndCallSingle(static_cast<Fs&>(Visitor), BasePtr))),
			...
			);
	}

	// 2. �� Lambda �汾 ���� ֧�� const �ͷ� const
	// ʹ�� std::enable_if_t �������� TOverloaded �������壨��ֻ��һ�� Lambda ʱ��
	template <typename F, typename TBasePtr,
		typename = std::enable_if_t<!std::is_base_of_v<IRecordedDataObjectInterface, std::remove_pointer_t<F>>>>
		void Visit(F Visitor, TBasePtr* BasePtr)
	{
		static_assert(std::is_base_of_v<IRecordedDataObjectInterface, std::remove_const_t<TBasePtr>>, "BasePtr must derive from IRecordedDataObjectInterface");

		if (!BasePtr) return;

		RecordedDataVisitorPrivate::TryCastAndCallSingle(static_cast<F&>(Visitor), BasePtr);
	}
}