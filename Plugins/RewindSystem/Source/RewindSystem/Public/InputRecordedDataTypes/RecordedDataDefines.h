#pragma once

#include "RecordedDataDefines.generated.h"

class UInputRecordComponent;

UENUM(BlueprintType)
enum class ERecordableActionType :uint8
{
	Instant,
	HasDuration
};

UENUM(BlueprintType)
enum class ERecordState :uint8
{
	Idle,
	RecordPause,
	Recording,
	PreviewPause,
	Previewing,
	Rewinding
};

UENUM(BlueprintType)
enum class EActionState : uint8
{
	Inactive,
	Active
};

using ClassIDType = uint16;
using RewindSystemTickType = uint16;
constexpr RewindSystemTickType REWIND_TICK_NONE{ 0 };

USTRUCT(BlueprintType)
struct FRecordedDataObjectHandle
{
	GENERATED_BODY()

	friend class URewindSystemStatics;

	FRecordedDataObjectHandle() = default;

	FRecordedDataObjectHandle(RewindSystemTickType Tick, UInputRecordComponent& InputRecordComponent);

	/** True if GenerateNewHandle was called on this handle */
	inline bool IsValid() const { return Handle != INDEX_NONE && Tick != REWIND_TICK_NONE && InputRecordComponent.IsValid(); }

	/** Sets this to a valid handle */
	void GenerateNewHandle();

	inline RewindSystemTickType GetTick() const { return Tick; };
	inline int32 GetHandle() const { return Handle; };

public:
	TWeakObjectPtr<UInputRecordComponent> InputRecordComponent;

private:

	UPROPERTY()
	int32 Handle{ INDEX_NONE };

	RewindSystemTickType Tick{ REWIND_TICK_NONE };

public:
	bool operator==(const FRecordedDataObjectHandle& Other) const
	{
		const bool bResult{ Handle == Other.Handle /*&& InputRecordComponent == Other.InputRecordComponent*/ };

#ifdef UE_BUILD_SHIPPING
		return bResult;
#else
		return bResult && check(Tick == Other.Tick);
#endif // UE_BUILD_SHIPPING
	}

	bool operator!=(const FRecordedDataObjectHandle& Other) const
	{
		return !operator==(Other);
	}

	/** Operator to expose FRecordedDataObjectHandle serialization to custom serialization functions like NetSerialize overrides. */
	friend FArchive& operator<<(FArchive& Ar, FRecordedDataObjectHandle& Value)
	{
		static_assert(sizeof(FRecordedDataObjectHandle) == 16, "If properties of FGameplayAbilitySpecHandle change, consider updating this operator implementation.");
		Ar << Value.Handle << Value.Tick << Value.InputRecordComponent;
		return Ar;
	}

	friend uint32 GetTypeHash(const FRecordedDataObjectHandle& SpecHandle)
	{
		return ::GetTypeHash(SpecHandle.Handle);
	}

	FString ToString() const
	{
		return IsValid() ? FString("Handle: ") + FString::FromInt(Handle) + FString("\tTick: ") + FString::FromInt(Tick) : TEXT("Invalid");
	}
};

class FTypeInfoPrivate final
{
	template <typename T> friend struct TTypeInfo;

	[[nodiscard]] inline static ClassIDType IDGenerator()
	{
		static ClassIDType NextID{ 1 };
		return NextID++;
	}
};

struct REWINDSYSTEM_API IClassIDObjectBase
{
	virtual ~IClassIDObjectBase() = default;

	[[nodiscard]] virtual ClassIDType GetClassID() const = 0;

	//virtual bool CanHandlePayload() const = 0;
	//virtual bool PrepareToHandlePayload() const = 0;
	//virtual void HandlePayload() const = 0;

	template <typename To>
	[[nodiscard]] friend inline const To* CustomCast(const IClassIDObjectBase* Src)
	{
		if (!Src) return nullptr;

		if (Src->GetClassID() == TTypeInfo<To>::GetClassID())
		{
			return reinterpret_cast<const To*>(Src);
		}

		return nullptr;
	}

	template <typename To>
	[[nodiscard]] friend inline To* CustomCast(IClassIDObjectBase* Src)
	{
		return const_cast<To*>(CustomCast<To>(const_cast<const IClassIDObjectBase*>(Src)));
	}
};

struct REWINDSYSTEM_API IRecordedDataObjectInterface : public IClassIDObjectBase
{
	IRecordedDataObjectInterface();
	explicit IRecordedDataObjectInterface(ERecordableActionType RecordableActionType);
	IRecordedDataObjectInterface(const IRecordedDataObjectInterface&) = delete;
	IRecordedDataObjectInterface& operator=(const IRecordedDataObjectInterface&) = delete;
	IRecordedDataObjectInterface(IRecordedDataObjectInterface&& Other) noexcept = default;
	IRecordedDataObjectInterface& operator=(IRecordedDataObjectInterface&& Other) noexcept
	{
		if (this != &Other)
		{
			Tick = Other.Tick;
			Handle = MoveTemp(Other.Handle);
		}
		return *this;
	}

	virtual bool PrepareToHandleRecordedData() = 0;
	[[nodiscard]] virtual bool ConsumeAndTryHandlePayload(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> RecordedData) = 0;

	virtual bool PrepareToPreview() = 0;
	virtual void Preview(const bool bIsPreview) = 0;

	[[nodiscard]] virtual bool ShouldStopRewindWhenHandleThisUnsuccessful() const = 0;

	RewindSystemTickType Tick{ REWIND_TICK_NONE };
	const ERecordableActionType RecordableActionType{ ERecordableActionType::Instant };
	FRecordedDataObjectHandle Handle;
};

struct REWINDSYSTEM_API FObjectToken
{
	using ObjectTokenType = bool;

	FObjectToken();
	//~FObjectToken() = default;
	//FObjectToken(const FObjectToken&) = default;
	//FObjectToken& operator=(const FObjectToken&) = default;

	//FObjectToken(FObjectToken&&) = delete;
	//FObjectToken& operator=(FObjectToken&&) = delete;

	[[nodiscard]] TWeakPtr<ObjectTokenType, ESPMode::NotThreadSafe> GetToken() const;

private:
	TSharedPtr<ObjectTokenType, ESPMode::NotThreadSafe> Token;
};

struct REWINDSYSTEM_API FNoObjectToken
{
	//FNoObjectToken() = default;
	//~FNoObjectToken() = default;
	//FNoObjectToken(const FNoObjectToken&) = default;
	//FNoObjectToken& operator=(const FNoObjectToken&) = default;

	//FNoObjectToken(FNoObjectToken&&) = delete;
	//FNoObjectToken& operator=(FNoObjectToken&&) = delete;
};

//struct REWINDSYSTEM_API FDelegateObject
//{
//	//FDelegateObject() = default;
//	//~FDelegateObject() = default;
//	//FDelegateObject(const FDelegateObject&) = default;
//	//FDelegateObject& operator=(const FDelegateObject&) = default;
//
//	//FDelegateObject(FDelegateObject&&) = delete;
//	//FDelegateObject& operator=(FDelegateObject&&) = delete;
//
//	FShowWillExecuteOperationDelegate ShowWillExecuteOperationDelegate;
//};
//
//struct REWINDSYSTEM_API FNoDelegateObject
//{
//	//FNoDelegateObject() = default;
//	//~FNoDelegateObject() = default;
//	//FNoDelegateObject(const FNoDelegateObject&) = default;
//	//FNoDelegateObject& operator=(const FNoDelegateObject&) = default;
//
//	//FNoDelegateObject(FNoDelegateObject&&) = delete;
//	//FNoDelegateObject& operator=(FNoDelegateObject&&) = delete;
//};

struct REWINDSYSTEM_API FNoSharedFromThis
{
};