// Copyright 2017 Andrew Grant
// This file is part of RemoteSession and is freely licensed for commercial and 
// non-commercial use under an MIT license
// See https://github.com/andrewgrant/RemoteSession for more info

#include "Channels/RemoteSessionInputChannel.h"
#include "SlateApplication.h"
#include "Protocol/OSC/BackChannelOSCConnection.h"
#include "Protocol/OSC/BackChannelOSCMessage.h"
#include "MessageHandler/RecordingMessageHandler.h"



FRemoteSessionInputChannel::FRemoteSessionInputChannel(ERemoteSessionChannelMode InRole, TSharedPtr<FBackChannelOSCConnection, ESPMode::ThreadSafe> InConnection)
	: IRemoteSessionChannel(InRole, InConnection)
{

	Connection = InConnection;
	Role = InRole;

	// if sending input replace the default message handler with a recording version, and set us as the
	// handler for that data 
	if (Role == ERemoteSessionChannelMode::Send)
	{
		DefaultHandler = FSlateApplication::Get().GetPlatformApplication()->GetMessageHandler();

		RecordingHandler = MakeShareable(new FRecordingMessageHandler(DefaultHandler.Pin()));

		RecordingHandler->SetRecordingHandler(this);

		FSlateApplication::Get().GetPlatformApplication()->SetMessageHandler(RecordingHandler.ToSharedRef());
	}
	else
	{
		TSharedRef<FGenericApplicationMessageHandler> DestinationHandler = FSlateApplication::Get().GetPlatformApplication()->GetMessageHandler();

		PlaybackHandler = MakeShareable(new FRecordingMessageHandler(DestinationHandler));

		Connection->GetDispatchMap().GetAddressHandler(TEXT("/MessageHandler/")).AddRaw(this, &FRemoteSessionInputChannel::OnRemoteMessage);
	}
	
}

FRemoteSessionInputChannel::~FRemoteSessionInputChannel()
{
	// todo - is this ok? Might other things have changed the handler like we do?
	if (DefaultHandler.IsValid())
	{
		FSlateApplication::Get().GetPlatformApplication()->SetMessageHandler(DefaultHandler.Pin().ToSharedRef());
	}

	// should restore handler? What if something else changed it...
	if (RecordingHandler.IsValid())
	{
		RecordingHandler->SetRecordingHandler(nullptr);
	}
}

FString FRemoteSessionInputChannel::StaticType()
{
	return TEXT("rv.input");
}


void FRemoteSessionInputChannel::SetPlaybackWindow(TWeakPtr<SWindow> InWindow, TWeakPtr<FSceneViewport> InViewport)
{
	PlaybackHandler->SetPlaybackWindow(InWindow, InViewport);
}

void FRemoteSessionInputChannel::Tick(const float InDeltaTime)
{
	
}

void FRemoteSessionInputChannel::RecordMessage(const TCHAR* MsgName, const TArray<uint8>& Data)
{
	if (Connection.IsValid())
	{
		// send as blobs
		FString Path = FString::Printf(TEXT("/MessageHandler/%s"), MsgName);
		FBackChannelOSCMessage Msg(*Path);

		Msg.Write(Data);

		Connection->SendPacket(Msg);
	}
}

void FRemoteSessionInputChannel::OnRemoteMessage(FBackChannelOSCMessage& Message, FBackChannelOSCDispatch& Dispatch)
{
	FString MessageName = Message.GetAddress();
	MessageName.RemoveFromStart(TEXT("/MessageHandler/"));

	TArray<uint8> MsgData;
	Message << MsgData;

	PlaybackHandler->PlayMessage(*MessageName, MsgData);
}
