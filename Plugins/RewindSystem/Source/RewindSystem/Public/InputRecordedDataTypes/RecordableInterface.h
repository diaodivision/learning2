// RecordableInterface.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InputRecordedDataTypes/RecordedDataDefines.h"
#include "InputRecordedDataTypes/RecordedDataDelegates.h"
#include "RecordableInterface.generated.h"

//UENUM(BlueprintType)
//enum class ERecordableActionType :uint8
//{
//	Instant,
//	HasDuration
//};
struct IClassIDObjectBase;
enum class ERecordableActionType : uint8;

//using IRecordedDataObjectBase = IClassIDObjectBase;

// This class does not need to be modified.
UINTERFACE(Blueprintable, BlueprintType, MinimalAPI)
class URecordableInterfaceBase : public UInterface
{
	GENERATED_BODY()
};

UINTERFACE(Blueprintable, BlueprintType, MinimalAPI)
class URecordableInterface : public URecordableInterfaceBase
{
	GENERATED_BODY()
};

UINTERFACE(Blueprintable, BlueprintType, MinimalAPI)
class UDurativeRecordableInterface : public URecordableInterfaceBase
{
	GENERATED_BODY()
};

class REWINDSYSTEM_API IRecordableInterfaceBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Recordable")
	ERecordableActionType GetActionType() const;
	virtual ERecordableActionType GetActionType_Implementation() const = 0;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Recordable")
	bool ShouldRecord() const;
	virtual bool ShouldRecord_Implementation() const = 0;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Recordable")
	bool ShouldStopWhenFailToHandleRecordedData() const;
	virtual bool ShouldStopWhenFailToHandleRecordedData_Implementation() const = 0;

	virtual EActionState GetActionState() const = 0;

protected:
	virtual void PreRecord() = 0;
	virtual void Record() = 0;
	virtual void PostRecord(const FRecordedDataObjectHandle& Handle) = 0;
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Recordable", meta = (DisplayName = "PostRecord"))
	void K2_PostRecord(const FRecordedDataObjectHandle& Handle);

	virtual void OnPreview(const bool bIsPreview, const IRecordedDataObjectInterface* InRecordedData) = 0;
};

class REWINDSYSTEM_API IRecordableInterface : public IRecordableInterfaceBase
{
	GENERATED_BODY()

public:
	virtual bool TryHandleRecordedData(IRecordedDataObjectInterface* InRecordedData) = 0;
};

class REWINDSYSTEM_API IDurativeRecordableInterface : public IRecordableInterfaceBase
{
	GENERATED_BODY()

protected:
	//UFUNCTION(BlueprintCallable, Category = "Recordable")
	//virtual void NotifyStartDurativeAction(const IRecordedDataObjectInterface& RecordedData) = 0;
	virtual bool TryHandleRecordedData(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> InRecordedData) = 0;
	virtual void ReleaseRecordedData() = 0;
	virtual void NotifyStartDurativeAction(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> InRecordedData) = 0;
	virtual void NotifyEndDurativeAction() = 0;
};

UCLASS(Blueprintable, BlueprintType, Abstract)
class REWINDSYSTEM_API ADurativeRecordableBase : public AActor, public IDurativeRecordableInterface
{
	GENERATED_BODY()

public:
	virtual FORCEINLINE EActionState GetActionState() const override { return EActionState::Inactive; }
	virtual FORCEINLINE ERecordableActionType GetActionType_Implementation() const { return ERecordableActionType::HasDuration; };
	virtual FORCEINLINE bool ShouldRecord_Implementation() const { return false; };
	virtual FORCEINLINE bool ShouldStopWhenFailToHandleRecordedData_Implementation() const { return true; }

	virtual bool TryHandleRecordedData(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> InRecordedData) override;

protected:
	virtual FORCEINLINE void PreRecord() override {}

	virtual FORCEINLINE void Record() override
	{
		// Example: 

		//if (ShouldRecord())
		//{
		//	URewindSubsystem* System = URewindSystemStatics::GetRewindSubsystem(this);
		//	if (!System) { return; }

		//	TUniquePtr<IRecordedDataObjectInterface> Data = MakeUnique<IRecordedDataObjectInterface>();
		//	if (!GetOwner()) { return; }

		//FRecordedDataObjectHandle RecordedDataObjectHandle = System->Record(MoveTemp(Data), *AvatarActor, [this](const FRecordedDataObjectHandle& Handle) { PostRecord(Handle); });
		//}
		//else
		//{
		//}
	}
	virtual FORCEINLINE void PostRecord(const FRecordedDataObjectHandle& Handle) override { Execute_K2_PostRecord(this, Handle); }
	virtual FORCEINLINE void OnPreview(const bool bIsPreview, const IRecordedDataObjectInterface* InRecordedData) override {};
	virtual void NotifyStartDurativeAction(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> InRecordedData) override;
	virtual void NotifyEndDurativeAction() override;
	virtual void ReleaseRecordedData() override final;

protected:
	//TSharedPtr<FObjectToken::ObjectTokenType, ESPMode::NotThreadSafe> Token;
	TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> RecordedData;
	//UPROPERTY(BlueprintReadWrite, Category = "Recordable")
	//FShowWillExecuteOperationDelegate ShowWillExecuteOperationDelegate;
};