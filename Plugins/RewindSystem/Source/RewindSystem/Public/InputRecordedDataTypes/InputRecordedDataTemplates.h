#pragma once

#include <concepts>
#include <type_traits>
#include "Misc/CoreMiscDefines.h"
#include "RecordedDataDefines.h"
#include "Templates/SharedPointer.h"
//using IRecordedDataObjectBase = struct IClassIDObjectBase;

template <typename T>
struct TTypeInfo final
{
	TTypeInfo() = delete;
	~TTypeInfo() = delete;
	UE_NONCOPYABLE(TTypeInfo)

		inline static ClassIDType GetClassID()
	{
		const static ClassIDType ClassID{ FTypeInfoPrivate::IDGenerator() };

		return ClassID;
	}
};

class FClassIDObjectBasePrivate final
{
	FClassIDObjectBasePrivate() = delete;
	~FClassIDObjectBasePrivate() = delete;
	UE_NONCOPYABLE(FClassIDObjectBasePrivate);

	//template <ERecordableActionType ActionType, typename Derived, typename T> requires CValidatablePayload<Derived, T>
	//template <ERecordableActionType ActionType, typename Derived, typename T>
	//	requires (std::is_same_v<T, void> || requires (const Derived & Object) { { Object.IsPayloadValid() } -> std::same_as<bool>; })
	//friend struct TRecordedDataBase;
	template <ERecordableActionType ActionType, typename Derived, typename T> friend struct TRecordedDataBaseCommon;

	template<typename T>
	struct TRecordedDataStorage
	{
		T Payload;
	};

	template <>
	struct TRecordedDataStorage<void> {};
};

template <ERecordableActionType ActionType, typename Derived, typename T = void>
struct TRecordedDataBaseCommon :
	public FClassIDObjectBasePrivate::TRecordedDataStorage<T>,
	//public IClassIDObjectBase,
	public IRecordedDataObjectInterface,
	public std::conditional_t<ActionType == ERecordableActionType::HasDuration, FObjectToken, FNoObjectToken>
	//public std::conditional_t<ActionType == ERecordableActionType::HasDuration, FDelegateObject, FNoDelegateObject>,
	//public std::conditional_t<ActionType == ERecordableActionType::HasDuration, TSharedFromThis<Derived, ESPMode::NotThreadSafe>, FNoSharedFromThis>
{
	TRecordedDataBaseCommon() : IRecordedDataObjectInterface(ActionType) {}

	virtual ClassIDType GetClassID() const override
	{
		const static ClassIDType ClassID{ TTypeInfo<Derived>::GetClassID() };

		return ClassID;
	}
};

template <ERecordableActionType ActionType, typename Derived, typename T = void>
struct TRecordedDataBase : public TRecordedDataBaseCommon<ActionType, Derived, T>
{
	virtual bool IsPayloadValid() const = 0;
};

template <ERecordableActionType ActionType, typename Derived>
struct TRecordedDataBase<ActionType, Derived, void> : public TRecordedDataBaseCommon<ActionType, Derived, void>
{
};