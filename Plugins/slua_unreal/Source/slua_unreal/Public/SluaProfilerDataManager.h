// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ProfileDataDefine.h"
#include "Modules/ModuleManager.h"
#include "Containers/Queue.h"
#include "Containers/Array.h"
#include "Containers/Ticker.h"
#include "Serialization/BufferArchive.h"
#include "HAL/Runnable.h"

class SLUA_UNREAL_API FProfileDataProcessRunnable : public FRunnable
{
public:
    FProfileDataProcessRunnable();

    void StopStreamingRunnable();

    // Begin FRunnable overrides
    virtual bool Init() override;
    virtual uint32 Run() override;

    //接收性能数据
    void ReceiveProfileData(int hookEvent, int64 time, int lineDefined, const FString& funcName, const FString& shortSrc);
    //接收内存数据
    void ReceiveMemoryData(int hookEvent, TArray<NS_SLUA::LuaMemInfo>& memInfoList);

    //用外部数据存储数据
    void SaveDataWithData(int inCpuViewBeginIndex, int inMemViewBeginIndex,ProfileNodeArrayArray& inProfileData, const MemNodeInfoList& inLuaMemNodeList, FString inSavePath = "");

    //存储数据
    void SaveData();
    //解压数据
    void LoadData(const TArray<uint8>& inFileData, int& inCpuViewBeginIndex, int& inMemViewBeginIndex, ProfileNodeArrayArray& inProfileData, MemNodeInfoList& inLuaMemNodeList);

    //每帧组合以及发送数据
    bool Tick(slua::LuaState* LS);
    //清除数据
    void OnClearDataWithCallBack(TFunction<void()>&& Callback);

    void ClearData();

    bool bIsRecording = false;
private:
    FRunnableThread* WorkerThread;

    bool RunnableStart = false;

    int cpuViewBeginIndex;
    int memViewBeginIndex;
    float lastLuaTotalMemSize;

    ProfileNodeArrayArray allProfileData;
    TArray<TSharedPtr<FunctionProfileNode>> profileRootArr;
    TQueue<TSharedPtr<FunctionProfileNode>, EQueueMode::Mpsc> funcProfilerNodeQueue;
    TSharedPtr<FProflierMemNode> lastLuaMemNode;
    MemNodeInfoList allLuaMemNodeList;

    MemoryFrameQueue memoryQueue;
    ProfileNodePtr funcProfilerRoot;
    ProfileCallInfoArray profilerStack;
    MemoryFramePtr currentMemory;
    LuaMemInfoMap memoryInfo;

    void PreProcessData(TSharedPtr<FunctionProfileNode> funcInfoRoot, TMap<int64, NS_SLUA::LuaMemInfo>& memoryInfoMap, MemoryFramePtr memoryFrame);

    void CollectMemoryNode(TMap<int64, NS_SLUA::LuaMemInfo>& memoryInfoMap, MemoryFramePtr memoryFrame);

    void RestartMemoryStatistis();
    void initLuaMemChartList();
    void ClearCurProfiler();

    //压缩存储及解压部分 BEGIN
    void SerializeSave(FBufferArchive* inBufferArchive, int inCpuViewBeginIndex, int inMemViewBeginIndex, ProfileNodeArrayArray& inProfileData, const MemNodeInfoList& inLuaMemNodeList);
    void DeserializeCompressedLoad(FBufferArchive* inBufAr, int& inCpuViewBeginIndex, int& inMemViewBeginIndex, ProfileNodeArrayArray& inProfileData, MemNodeInfoList& inLuaMemNodeList);
    //压缩存储及解压部分 END
};

class SLUA_UNREAL_API SluaProfilerDataManager
{
public:

    static void StartManager();
    static void StopManager();

    //接收性能数据
    static void ReceiveProfileData(int hookEvent, int64 time, int lineDefined, const FString& funcName, const FString& shortSrc);
    //接收内存数据
	static void ReceiveMemoryData(int hookEvent, TArray<NS_SLUA::LuaMemInfo>& memInfoList);

    //用外部数据存储数据
    static void SaveDataWithData(int inCpuViewBeginIndex, int inMemViewBeginIndex, ProfileNodeArrayArray& inProfileData, const MemNodeInfoList& inLuaMemNodeList, FString inSavePath = "");

    //开始录制
    static void BeginRecord();

    //结束录制
    static void EndRecord();

    //存储数据
    static void SaveData();
    //解压数据
    static void LoadData(const TArray<uint8>& inFileData, int& inCpuViewBeginIndex, int& inMemViewBeginIndex, ProfileNodeArrayArray& inProfileData, MemNodeInfoList& inLuaMemNodeList);

    //清除数据
    static void OnClearDataWithCallBack(TFunction<void()>&& Callback);

    static void ClearData();

    static void WatchBegin(const FString& fileName, int32 lineDefined, const FString& funcName, double nanoseconds, ProfileNodePtr funcProfilerRoot, ProfileCallInfoArray& profilerStack);
    static void WatchEnd(const FString& fileName, int32 lineDefined, const FString& functionName, double nanoseconds, ProfileCallInfoArray& profilerStack);
    static void CoroutineBegin(int32 lineDefined, const FString& funcName, double nanoseconds, ProfileNodePtr funcProfilerRoot, ProfileCallInfoArray& profilerStack);
    static void CoroutineEnd(double nanoseconds, ProfileCallInfoArray& profilerStack);

    static void InitProfileNode(TSharedPtr<FunctionProfileNode>& funcNode, const FLuaFunctionDefine& funcDefine, int32 layerIdx);
    static void AddToParentNode(TSharedPtr<FunctionProfileNode> patentNode, TSharedPtr<FunctionProfileCallInfo> callInfo);
    
    static FProfileNameSet* ProfileNameSet;
private:
    static FProfileDataProcessRunnable* ProcessRunnable;
};
