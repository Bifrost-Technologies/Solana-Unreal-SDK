/*
 *  LinkStream
 *  Copyright (c) 2024 Bifrost Inc.
 *  Author: Nathan Martell
 *
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"
#include "Containers/Queue.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "LinkStreamConnection.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FTcpSocketDisconnectDelegate, int32, ConnectionId);
DECLARE_DYNAMIC_DELEGATE_OneParam(FTcpSocketConnectDelegate, int32, ConnectionId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FTcpSocketReceivedMessageDelegate, int32, ConnectionId, UPARAM(ref) TArray<uint8>&, Message);

UCLASS(Blueprintable, BlueprintType)
class LINKSTREAM_API ALinkStreamConnection : public AActor
{
	GENERATED_BODY()
	
public:	
	ALinkStreamConnection();

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Socket")
	void Connect(const FString& ipAddress, int32 port, 
		const FTcpSocketDisconnectDelegate& OnDisconnected, const FTcpSocketConnectDelegate& OnConnected,
		const FTcpSocketReceivedMessageDelegate& OnMessageReceived, int32& ConnectionId);

	UFUNCTION(BlueprintCallable, Category = "Socket")
	void Disconnect(int32 ConnectionId);


	UFUNCTION(BlueprintCallable, Category = "Socket")
	bool SendData(int32 ConnectionId, TArray<uint8> DataToSend);

	
	//UFUNCTION(Category = "Socket")	
	void ExecuteOnConnected(int32 WorkerId, TWeakObjectPtr<ALinkStreamConnection> thisObj);

	//UFUNCTION(Category = "Socket")
	void ExecuteOnDisconnected(int32 WorkerId, TWeakObjectPtr<ALinkStreamConnection> thisObj);

	//UFUNCTION(Category = "Socket")
	void ExecuteOnMessageReceived(int32 ConnectionId, TWeakObjectPtr<ALinkStreamConnection> thisObj);

	/*UFUNCTION(BlueprintPure, meta = (DisplayName = "Append Bytes", CommutativeAssociativeBinaryOperator = "true"), Category = "Socket")
	static TArray<uint8> Concat_BytesBytes(const TArray<uint8>& A, const TArray<uint8>& B);*/

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Append Bytes", CommutativeAssociativeBinaryOperator = "true"), Category = "Socket")
	static TArray<uint8> Concat_BytesBytes(TArray<uint8> A, TArray<uint8> B);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Int To Bytes", CompactNodeTitle = "->", Keywords = "cast convert", BlueprintAutocast), Category = "Socket")
	static TArray<uint8> Conv_IntToBytes(int32 InInt);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "String To Bytes", CompactNodeTitle = "->", Keywords = "cast convert", BlueprintAutocast), Category = "Socket")
	static TArray<uint8> Conv_StringToBytes(const FString& InStr);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Float To Bytes", CompactNodeTitle = "->", Keywords = "cast convert", BlueprintAutocast), Category = "Socket")
	static TArray<uint8> Conv_FloatToBytes(float InFloat);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Byte To Bytes", CompactNodeTitle = "->", Keywords = "cast convert", BlueprintAutocast), Category = "Socket")
	static TArray<uint8> Conv_ByteToBytes(uint8 InByte);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Read Int", Keywords = "read int"), Category = "Socket")
	static int32 Message_ReadInt(UPARAM(ref) TArray<uint8>& Message);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Read Byte", Keywords = "read byte int8 uint8"), Category = "Socket")
	static uint8 Message_ReadByte(UPARAM(ref) TArray<uint8>& Message);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Read Bytes", Keywords = "read bytes"), Category = "Socket")
	static bool Message_ReadBytes(int32 NumBytes, UPARAM(ref) TArray<uint8>& Message, TArray<uint8>& ReturnArray);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Read Float", Keywords = "read float"), Category = "Socket")
	static float Message_ReadFloat(UPARAM(ref) TArray<uint8>& Message);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Read String", Keywords = "read string"), Category = "Socket")
	static FString Message_ReadString(UPARAM(ref) TArray<uint8>& Message, int32 StringLength);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Socket")
	bool isConnected(int32 ConnectionId);

	static void PrintToConsole(FString Str, bool Error);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	int32 SendBufferSize = 16384;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	int32 ReceiveBufferSize = 16384;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	float TimeBetweenTicks = 0.008f;

private:
	TMap<int32, TSharedRef<class FTcpSocketWorker>> TcpWorkers;

	FTcpSocketDisconnectDelegate DisconnectedDelegate;
	FTcpSocketConnectDelegate ConnectedDelegate;
	FTcpSocketReceivedMessageDelegate MessageReceivedDelegate;

	int32 NextConnectionId = 0;
};

class FTcpSocketWorker : public FRunnable, public TSharedFromThis<FTcpSocketWorker>
{

	FRunnableThread* Thread = nullptr;

private:
	class FSocket* Socket = nullptr;
	FString ipAddress;
	int port;
	TWeakObjectPtr<ALinkStreamConnection> ThreadSpawnerActor;
	int32 id;
	int32 RecvBufferSize;
	int32 ActualRecvBufferSize;
	int32 SendBufferSize;
	int32 ActualSendBufferSize;
	float TimeBetweenTicks;
	FThreadSafeBool bConnected = false;

	TQueue<TArray<uint8>, EQueueMode::Spsc> Inbox;
	TQueue<TArray<uint8>, EQueueMode::Spsc> Outbox;

public:

	FTcpSocketWorker(FString inIp, const int32 inPort, TWeakObjectPtr<ALinkStreamConnection> InOwner, int32 inId, int32 inRecvBufferSize, int32 inSendBufferSize, float inTimeBetweenTicks);
	virtual ~FTcpSocketWorker();

	void Start();


	void AddToOutbox(TArray<uint8> Message);


	TArray<uint8> ReadFromInbox();

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

	void SocketShutdown();


	bool isConnected();

private:

	bool BlockingSend(const uint8* Data, int32 BytesToSend);


	FThreadSafeBool bRun = false;

};