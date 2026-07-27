#pragma once

#include "InputRecordedDataTemplates.h"

using FLocationData = struct FRecordedLocationData;
using FRotationData = struct FRecordedRotationData;

struct FRecordedLocationPayloadBase
{
	TWeakObjectPtr<AActor> Actor;
	FVector OldLocation;
	FVector NewLocation;
};

struct FRecordedRotationPayloadBase
{
	TWeakObjectPtr<AActor> Actor;
	FRotator OldRotation;
	FRotator NewRotation;
	//TFunction<void()> OnRotationUpdatedCallback
};

struct REWINDSYSTEM_API FRecordedLocationData : public TRecordedDataBase<ERecordableActionType::Instant, FRecordedLocationData, FRecordedLocationPayloadBase>
{
	//virtual bool CanHandlePayload() const override;
	//virtual bool PrepareToHandlePayload() const override;
	//virtual void HandlePayload() const override;
	inline virtual bool IsPayloadValid() const override { return Payload.Actor.IsValid(); }

	virtual bool PrepareToHandleRecordedData() override;
	virtual bool ConsumeAndTryHandlePayload(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> RecordedData) override;

	virtual bool PrepareToPreview() override;
	virtual void Preview(const bool bIsPreview) override;
	//virtual void PreviewOnHandlePayload(bool bIsPreview);
	virtual bool ShouldStopRewindWhenHandleThisUnsuccessful() const override;

	//virtual void HandleData() const override {}
	//FVector Payload;
};

struct REWINDSYSTEM_API FRecordedRotationData : public TRecordedDataBase<ERecordableActionType::Instant, FRecordedRotationData, FRecordedRotationPayloadBase>
{
	//virtual bool CanHandlePayload() const override;
	//virtual bool PrepareToHandlePayload() const override;
	// 
	//virtual void HandlePayload() const override;
	//inline bool IsPayloadValid() const { return true; }
	inline virtual bool IsPayloadValid() const override { return Payload.Actor.IsValid(); }

	virtual bool PrepareToHandleRecordedData() override;
	virtual bool ConsumeAndTryHandlePayload(TSharedPtr<IRecordedDataObjectInterface, ESPMode::NotThreadSafe> RecordedData) override;

	virtual bool PrepareToPreview() override;
	virtual void Preview(const bool bIsPreview) override;

	//virtual void PreviewOnHandlePayload(bool bIsPreview);
	virtual bool ShouldStopRewindWhenHandleThisUnsuccessful() const override;
	//FRotator Payload;
};