// Copyright 2017 Andrew Grant
// Licensed under BSD License 2.0. 
// See https://github.com/andrewgrant/RemoteViewer for more info

#pragma once

#include "CoreMinimal.h"
#include "ProxyMessageHandler.h"

class IRecordingMessageHandlerWriter
{
public:

	virtual void RecordMessage(const TCHAR* MsgName, const TArray<uint8>& Data) = 0;
};

DECLARE_DELEGATE_OneParam(FRecordedMessageDispatch, FArchive&);

class REMOTEVIEWER_API FRecordingMessageHandler : public FProxyMessageHandler, public TSharedFromThis<FRecordingMessageHandler>
{
public:

	FRecordingMessageHandler(const TSharedPtr<FGenericApplicationMessageHandler>& InTargetHandler);

	void SetRecordingHandler(IRecordingMessageHandlerWriter* InOutputWriter);

	void SetConsumeInput(bool bConsume);

	bool IsRecording() const
	{
		return OutputWriter != nullptr;
	}

public:

	virtual bool OnKeyChar(const TCHAR Character, const bool IsRepeat) override;
	virtual bool OnKeyDown(const int32 KeyCode, const uint32 CharacterCode, const bool IsRepeat) override;
	virtual bool OnKeyUp(const int32 KeyCode, const uint32 CharacterCode, const bool IsRepeat) override;
	virtual bool OnTouchStarted(const TSharedPtr< FGenericWindow >& Window, const FVector2D& Location, int32 TouchIndex, int32 ControllerId) override;
	virtual bool OnTouchMoved(const FVector2D& Location, int32 TouchIndex, int32 ControllerId) override;
	virtual bool OnTouchEnded(const FVector2D& Location, int32 TouchIndex, int32 ControllerId) override;

	bool PlayMessage(const TCHAR* Message, const TArray<uint8>& Data);

protected:


	FVector2D ConvertToNormalizedScreenLocation(const FVector2D& Location);
	FVector2D ConvertFromNormalizedScreenLocation(const FVector2D& ScreenLocation);

	void RecordMessage(const TCHAR* MsgName, const TArray<uint8>& Data);

	virtual void PlayOnKeyChar(FArchive& Ar);
	virtual void PlayOnKeyDown(FArchive& Ar);
	virtual void PlayOnKeyUp(FArchive& Ar);
	virtual void PlayOnTouchStarted(FArchive& Ar);
	virtual void PlayOnTouchMoved(FArchive& Ar);
	virtual void PlayOnTouchEnded(FArchive& Ar);

	IRecordingMessageHandlerWriter*		OutputWriter;
	bool								ConsumeInput;

	TMap<FString, FRecordedMessageDispatch> DispatchTable;
};