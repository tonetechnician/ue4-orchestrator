/* -*- mode: c; tab-width: 4; indent-tabs-mode: nil; -*- */

#include "UE4Orchestrator.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UE4OrchestratorPrivate.generated.h"

#pragma once

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRunMRPDelegate);

////////////////////////////////////////////////////////////////////////////////

UCLASS(BlueprintType)
class UE4ORCHESTRATOR_API URCHTTP : public UObject, public FTickableGameObject
{
    GENERATED_UCLASS_BODY()
    virtual ~URCHTTP();

  public:

    void Init();

    /*
     *  FTickableObject interface.
     */
    virtual void    Tick(float dt)                  override;
    virtual TStatId GetStatId()             const   override;
    virtual bool    IsTickable()            const { return true; }
    virtual bool    IsTickableWhenPaused()  const { return true; }
    virtual bool    IsTickableInEditor()    const { return true; }

    /*
     *  UObject interface.
     */
    virtual void Serialize(FArchive& ar) override;
    virtual void PostLoad() override;
    virtual void PostInitProperties() override;

    UFUNCTION(
        BlueprintPure,
        Category = "Persistence",
        meta = (
            DisplayName = "Get Savegame Manager",
            Keywords = "SavegameManager"))
    URCHTTP* GetURCHTTP();
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& evt) override;
#endif

    void SetPollInterval(int v);

  private:
    UPROPERTY(Transient)
    URCHTTP* httpInstance;
    struct mg_mgr         mgr;
    struct mg_connection* conn;

    /*
     *  The poll_interval dictates how often we wish to poll.  If
     *  this is set to 0 (default), it will invoke a poll of
     *  `poll_ms` (default=1ms) each time this plugin gets a tick.
     *  However, if this is set to N (N > 0), this will only call
     *  poll once every N ticks.
     */
    int poll_interval;
    int poll_ms;

    struct mg_bind_opts bind_opts;
    const char* err_str;

    /*
     *  Pak file.
     */
    //FPakPlatformFile *PakFileMgr;

  public:

    UFUNCTION(BlueprintPure, Category = "Beamm|Classes|Http Server")
    static URCHTTP* Get();

    UPROPERTY(BlueprintAssignable, Category = "Beamm|Events|HTTP Events")
    FRunMRPDelegate RunMRP;
    //UFUNCTION()
    //int MountPakFile(const FString& PakPath, bool bLoadContent);

    ///*
    // *  TODO: LoadObject should probably be renamed to LoadObjectPak() or
    // *        something to that effect.
    // */
    //UFUNCTION()
    //UObject* LoadObject(const FString& ObjectPath);

    //UFUNCTION()
    //int UnloadObject(const FString& ObjectPath);

    //UFUNCTION()
    //void GarbageCollect();

    //UFUNCTION()
    //void FinishAllShaderCompilation();

    /*
     * Frame End
     */
    //UFUNCTION()
    //virtual void GameRenderSync();
    
};
