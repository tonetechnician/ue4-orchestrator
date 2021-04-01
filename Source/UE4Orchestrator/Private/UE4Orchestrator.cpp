/* -*- mode: c; tab-width: 4; indent-tabs-mode: nil; -*- */

/*
 *  UE4Orchestrator.h acts as the PCH for this project and must be the
 *  very first file imported.
 */
#include "UE4Orchestrator.h"


#define LOCTEXT_NAMESPACE "FUE4OrchestratorModule"

void FUE4OrchestratorModule::StartupModule()
{
}

void FUE4OrchestratorModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUE4OrchestratorModule, UE4Orchestrator)

#include <vector>
#include <string>

// UE4
#include "CoreMinimal.h"
//#include "IPlatformFilePak.h"
//#include "Runtime/Core/Public/HAL/FileManagerGeneric.h"
//#include "StreamingNetworkPlatformFile.h"
//#include "Runtime/AssetRegistry/Public/AssetRegistryModule.h"
//#include "Runtime/Core/Public/Misc/WildcardString.h"
//#include "Runtime/Engine/Classes/Engine/StreamableManager.h"
//#include "Runtime/Engine/Classes/Engine/AssetManager.h"
//#include "Runtime/Engine/Public/ShaderCompiler.h"
#include "Runtime/Engine/Public/UnrealEngine.h"

#if WITH_EDITOR
#  include "LevelEditor.h"
#  include "Editor.h"
#  include "Editor/LevelEditor/Public/ILevelViewport.h"
#  include "Editor/LevelEditor/Public/LevelEditorActions.h"
#  include "Editor/UnrealEd/Public/LevelEditorViewport.h"
#endif

#include "UE4OrchestratorPrivate.h"

// HTTP server
#include "mongoose.h"

////////////////////////////////////////////////////////////////////////////////

DEFINE_LOG_CATEGORY(LogUE4Orc);

////////////////////////////////////////////////////////////////////////////////

static void
debugFn(FString payload)
{
    return;
}

////////////////////////////////////////////////////////////////////////////////

//int
//URCHTTP::MountPakFile(const FString& pakPath, bool bLoadContent)
//{
//    int ret = 0;
//    IPlatformFile *originalPlatform = &FPlatformFileManager::Get().GetPlatformFile();
//
//    // Check to see if the file exists first
//    if (!originalPlatform->FileExists(*pakPath))
//    {
//        LOG("PakFile %s does not exist", *pakPath);
//        return -1;
//    }
//
//    // The pak reader is now the current platform file
//    FPlatformFileManager::Get().SetPlatformFile(*PakFileMgr);
//
//    // Get the mount point from the Pak meta-data
//    FPakFile PakFile(PakFileMgr, *pakPath, false);
//    FString MountPoint = PakFile.GetMountPoint();
//
//    // Determine where the on-disk path is for the mountpoint and register it
//    FString PathOnDisk = FPaths::ProjectDir() / MountPoint;
//    FPackageName::RegisterMountPoint(MountPoint, PathOnDisk);
//
//    FString MountPointFull = PathOnDisk;
//    FPaths::MakeStandardFilename(MountPointFull);
//
//    LOG("Mounting at %s and registering mount point %s at %s", *MountPointFull, *MountPoint, *PathOnDisk);
//    if (PakFileMgr->Mount(*pakPath, 0, *MountPointFull))
//    {
//        if (UAssetManager* Manager = UAssetManager::GetIfValid())
//        {
//            Manager->GetAssetRegistry().SearchAllAssets(true);
//            if (bLoadContent)
//            {
//                TArray<FString> FileList;
//                PakFile.FindFilesAtPath(FileList, *PakFile.GetMountPoint(), true, false, true);
//
//                // Iterate over the collected files from the pak
//                for (auto asset : FileList)
//                {
//                    FString Package, BaseName, Extension;
//                    FPaths::Split(asset, Package, BaseName, Extension);
//                    FString ModifiedAssetName = Package / BaseName + "." + BaseName;
//
//                    LOG("Trying to load %s as %s ", *asset, *ModifiedAssetName);
//                    Manager->GetStreamableManager().LoadSynchronous(ModifiedAssetName, true, nullptr);
//                }
//            }
//        }
//        else
//        {
//            LOG("Asset manager not valid!", NULL);
//            ret = -1; goto exit;
//        }
//    }
//    else
//    {
//        LOG("mount failed!", NULL);
//        ret = -1; goto exit;
//    }
//
//  exit:
//    // Restore the platform file
//    FPlatformFileManager::Get().SetPlatformFile(*originalPlatform);
//
//    return ret;
//}
//
//UObject*
//URCHTTP::LoadObject(const FString& assetPath)
//{
//    UObject* ret = nullptr;
//    IPlatformFile *originalPlatform = &FPlatformFileManager::Get().GetPlatformFile();
//
//    if (PakFileMgr == nullptr)
//    {
//        LOG("Failed to create platform file %s", T("PakFile"));
//        return ret;
//    }
//
//    // The pak reader is now the current platform file.
//    FPlatformFileManager::Get().SetPlatformFile(*PakFileMgr);
//    UAssetManager* Manager = UAssetManager::GetIfValid();
//
//    ret = FindObject<UStaticMesh>(ANY_PACKAGE, *assetPath);
//    if (Manager && ret == nullptr)
//        ret = Manager->GetStreamableManager().LoadSynchronous(assetPath, false, nullptr);
//
//    // Reset the platform file.
//    FPlatformFileManager::Get().SetPlatformFile(*originalPlatform);
//
//    return ret;
//}
//
///*
// *  TODO: Deprecate this function.
// */
//int
//URCHTTP::UnloadObject(const FString& assetPath)
//{
//    GarbageCollect();
//    return 0;
//}
//
//void
//URCHTTP::GarbageCollect()
//{
//    CollectGarbage(RF_NoFlags, true);
//}
//
//void
//URCHTTP::FinishAllShaderCompilation()
//{
//    if (GShaderCompilingManager)
//        GShaderCompilingManager->FinishAllCompilation();
//}
//
//void
//URCHTTP::GameRenderSync()
//{
//    static FFrameEndSync FrameEndSync;
//    /* Do a full sync without allowing a frame of lag */
//    FrameEndSync.Sync(false);
//}

////////////////////////////////////////////////////////////////////////////////
static struct mg_serve_http_opts s_http_server_opts;

// HTTP responses.
const mg_str_t STATUS_OK              = mg_mk_str("OK\r\n");
const mg_str_t STATUS_ERROR           = mg_mk_str("ERROR\r\n");
const mg_str_t STATUS_TRY_AGAIN       = mg_mk_str("TRY AGAIN\r\n");
const mg_str_t STATUS_NOT_SUPPORTED   = mg_mk_str("NOT SUPPORTED\r\n");
const mg_str_t STATUS_NOT_IMPLEMENTED = mg_mk_str("NOT IMPLEMENTED\r\n");
const mg_str_t STATUS_BAD_ACTION      = mg_mk_str("BAD ACTION\r\n");
const mg_str_t STATUS_BAD_ENTITY      = mg_mk_str("BAD ENTITY\r\n");

// HTTP query responses.
const mg_str_t STATUS_TRUE            = mg_mk_str("TRUE\r\n");
const mg_str_t STATUS_FALSE           = mg_mk_str("FALSE\r\n");

// Helper to match a list of URIs.
template<typename... Strings> bool
matches_any(mg_str_t* s, Strings... args)
{
    std::vector<std::string> items = {args...};
    for (int i = 0; i < items.size(); i++)
        if (mg_vcmp(s, items[i].c_str()) == 0)
            return true;
    return false;
}

static void response_success(struct mg_connection* nc, void* response_data) {
    UE_LOG(LogUE4Orc, Warning, TEXT("RESPONDING TO REQUEST"));
    //AHttpServer* LogUE4Orc = (AHttpServer*)nc->user_data;
    //mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
    //mg_printf(nc, "%s", "Transfer-Encoding: chunked\r\n\r\n");
    //mg_printf_http_chunk(nc, "{ \"status\": \"screenshot taken\" }");
    //mg_connection* c;
    mg_send_http_chunk(nc, "", 0); /* Send empty chunk, the end of response */
    //mg_mgr* mgr = httpServerActor->GetMongooseManager();
    //for (c = mg_next(mgr, NULL); c != NULL; c = mg_next(mgr, c)) {
    //    if (c->sock == httpServerActor->activeSocket) {
    //        const char* response = (char*)response_data;
    //        mg_printf_http_chunk(c, response);
    //        mg_send_http_chunk(c, response, sizeof(response));
    //        mg_send_http_chunk(c, "", 0); /* Send empty chunk, the end of response */
    //        httpServerActor->mongooseActive = true;
    //        break;
    //    }
    //}
}

static TMap<FString, FString> parseURIQueryParams(TArray<FString> queryParams) {
    TMap<FString, FString> keyValuePairs;
    for (int i = 0; i < queryParams.Num(); i++) {
        FString key, value;
        queryParams[i].Split("=", &key, &value);
        UE_LOG(LogUE4Orc, Warning, TEXT("Got Key: %s"), *key);
        UE_LOG(LogUE4Orc, Warning, TEXT("Got Value: %s"), *value);
        keyValuePairs.Add(key, value);
    }

    return keyValuePairs;
}

static FVector parseVectorFromJSONString(FString stringData) {
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(stringData);
    FString x, y, z;
    // Deserialize the json data given Reader and the actual object to deserialize
    if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
        TArray<FString> keys;

        JsonObject->Values.GetKeys(keys);
        UE_LOG(LogUE4Orc, Warning, TEXT("JSON Keys: %s %s %s"), *keys[0], *keys[1], *keys[2]);

        // Get the value of the json object by field name
        x = JsonObject->GetStringField("x");
        y = JsonObject->GetStringField("y");
        z = JsonObject->GetStringField("z");

        UE_LOG(LogUE4Orc, Warning, TEXT("Parsed Vector: %s %s %s"), *x, *y, *z);
    }

    return FVector(FCString::Atof(*x), FCString::Atof(*y), FCString::Atof(*z));
}

static FRotator parseRotatorFromJSONString(FString stringData) {
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(stringData);
    FString roll, pitch, yaw;
    // Deserialize the json data given Reader and the actual object to deserialize
    if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
        // Get the value of the json object by field name
        roll = JsonObject->GetStringField("roll");
        pitch = JsonObject->GetStringField("pitch");
        yaw = JsonObject->GetStringField("yaw");

        UE_LOG(LogUE4Orc, Warning, TEXT("Parsed Rotator: %s %s %s"), *roll, *pitch, *yaw);
    }

    return FRotator(FCString::Atof(*pitch), FCString::Atof(*yaw), FCString::Atof(*roll));
}

////////////////////////////////////////////////////////////////////////////////

static void
ev_handler(struct mg_connection* conn, int ev, void *ev_data)
{
    if (ev != MG_EV_HTTP_REQUEST)
        return;

    http_message_t* hm       = (http_message_t *)ev_data;
    mg_str_t        rspMsg    = STATUS_ERROR;
    int             rspStatus = 404;
    URCHTTP*        server    = URCHTTP::Get();
    URCHTTP* httpServerActor = (URCHTTP*)conn->user_data;

    switch (ev) {
    case MG_EV_HTTP_REQUEST:
        if (mg_vcmp(&hm->method, "GET") == 0) {
            if (mg_vcmp(&hm->uri, "/api/v1/sum") == 0) {
                //handle_sum_call(nc, hm); /* Handle RESTful call */
                UE_LOG(LogUE4Orc, Warning, TEXT("HIT THE SERVER, NOW DO SOME STUFF!"));
            }
            else if (mg_vcmp(&hm->uri, "/takePicture") == 0) {
                UE_LOG(LogUE4Orc, Warning, TEXT("Take screenshot"));
                //FString query(hm->query_string.p, hm->query_string.len);

                //query.Split(" ", &query, NULL);
                //query = query.Replace(TEXT("%22"), TEXT("\""));
                //query = query.Replace(TEXT("%20"), TEXT(" "));

                ////FString query = hm->query_string
                //UE_LOG(LogUE4Orc, Warning, TEXT("%s"), *query);

                //TArray<FString> queryParams;
                //query.ParseIntoArray(queryParams, TEXT("&"), true);

                //TMap<FString, FString> keyValuePairs = parseURIQueryParams(queryParams);

                //FString defaultSize = "";
                //FString* size = keyValuePairs.Find("size");
                //FString* position = keyValuePairs.Find("position");
                //FString* rotation = keyValuePairs.Find("rotation");
                //FVector positionVector(0, 0, 0);
                //FRotator rotationRotator(0, 0, 0);
                //positionVector = parseVectorFromJSONString(*position);
                //rotationRotator = parseRotatorFromJSONString(*rotation);

                //UE_LOG(LogUE4Orc, Warning, TEXT("%d %d %d"), positionVector.X, positionVector.Y, positionVector.Z);
                //UE_LOG(LogUE4Orc, Warning, TEXT("%d %d %d"), rotationRotator.Euler().X, rotationRotator.Euler().Y, rotationRotator.Euler().Z);

                //if (size == nullptr) {
                //    size = &defaultSize;
                //}

                //if (position != nullptr) {
                //    positionVector = parseVectorFromJSONString(*position);
                //}
                //else {

                //}

                //if (rotation != nullptr) {
                //    rotationRotator = parseRotatorFromJSONString(*rotation);
                //}
                //else {

                //}
                httpServerActor->RunMRP.Broadcast();
                //httpServerActor->OnTakeScreenshot.Broadcast(*size, positionVector, rotationRotator);
                mg_printf(conn, "%s", "HTTP/1.1 200 OK\r\nConnection: Keep-Alive\r\nKeep-Alive: timeout=10, max=1000\r\nTransfer-Encoding: chunked\r\n\r\n");
                //mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nConnection: Keep-Alive\r\nKeep-Alive: timeout=10, max=1000\r\nContent-Disposition: inline; filename=\"test.png\"\r\nContent-Type: image/png\r\nTransfer-Encoding: chunked\r\n\r\n");
                mg_send_http_chunk(conn, "Ho\r\n", 4); /* Send empty chunk, the end of response */

                mg_send_http_chunk(conn, "", 0); /* Send empty chunk, the end of response */
                //httpServerActor->activeSocket = nc->sock;
            }
            else if (mg_vcmp(&hm->uri, "/loadGLB") == 0) {
                UE_LOG(LogUE4Orc, Warning, TEXT("Load GLB"));
                FString url("https://dpnc85okkpizu.cloudfront.net/creators/22/3d_asset_result/58132201c9a84591a252d38280b1a179.glb?Expires=1616323151&Signature=Zrne7OEGrGZLmB~CI~DjcU0U7~eeDc3IEOTcGZRoDqOC7a2lZyVS2GxDYmTf0wl~5RjWylTZ8jphr7J1TBLxDgDQGYzl2Y~j8KiaXnHF~u-1O~owUpcW5PQxuWvNMIw5CNejMVRJiAxYiTVqsm7jIyAzJEm9518yMJtls51Ce5WRWwhpJAaHNjHi2z2Y8cdSdzgELuTiOPU5VkjEVz7O85igBx8KX2IrNiMH4VMNBeI9PPNTprgfT3cl~e0KIx519opt3cUEFOMs6Ri2H8SwcnQdXPnm-4Q7MEZP1JYMpa5uHgr~LJI92ZViJeV3HeCGLJwhdgMHgLAKGUfWMbkbfQ__&Key-Pair-Id=K2W8MF1BYDOO1C");
                UE_LOG(LogUE4Orc, Warning, TEXT("%s"), *url);
                //httpServerActor->OnLoadGLBDelegate.Broadcast(url);
                //mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nConnection: Keep-Alive\r\nKeep-Alive: timeout=10, max=1000\r\nTransfer-Encoding: chunked\r\n\r\n");
                //httpServerActor->activeSocket = nc->sock;

                //response_success(nc, "{ \"status\": \"screenshot taken\" }");
                //response_success(nc, "{ \"status\": \"screenshot taken\" }");
                //mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
                //mg_printf_http_chunk(nc, "{ \"status\": \"glb loaded\" }");
                //mg_send_http_chunk(nc, "", 0); /* Send empty chunk, the end of response */
            }
            else {
                mg_serve_http(conn, hm, s_http_server_opts); /* Serve static content */
            }
        }
        break;
    case MG_EV_HTTP_REPLY:
        UE_LOG(LogUE4Orc, Warning, TEXT("HTTP REPLY"));
        break;
    case MG_EV_CLOSE:
        UE_LOG(LogUE4Orc, Warning, TEXT("Connection closing"));
        break;
    default:
        break;
    }
//#if WITH_EDITOR
//    auto ar = "AssetRegistry";
//    FAssetRegistryModule& AssetRegistry =
//        FModuleManager::LoadModuleChecked<FAssetRegistryModule>(ar);
//
//    /*
//     *  HTTP GET commands
//     */
//    if (mg_vcmp(&msg->method, "GET") == 0)
//    {
//        /*
//         *  HTTP GET /
//         *
//         *  Return "OK"
//         */
//        if (matches_any(&msg->uri, "/"))
//        {
//            goto OK;
//        }
//
//        /*
//         *  HTTP GET /play
//         *
//         *  Trigger a play in the current level.
//         */
//        else if (matches_any(&msg->uri, "/play", "/ue4/play"))
//        {
//            GEditor->PlayMap(NULL, NULL, -1, -1, false);
//            goto OK;
//        }
//
//        /*
//         *  HTTP GET /play
//         *
//         *  Trigger a play in the current level.
//         */
//        else if (matches_any(&msg->uri, "/stop", "/ue4/stop"))
//        {
//            FLvlEditor &Editor =
//                FManager::LoadModuleChecked<FLvlEditor>("LevelEditor");
//
//            if (!Editor.GetFirstActiveViewport().IsValid())
//            {
//                LOG("%s", "ERROR no valid viewport");
//                goto ERROR;
//            }
//
//            if (Editor.GetFirstActiveViewport()->HasPlayInEditorViewport())
//            {
//                FString cmd = "Exit";
//                auto ew = GEditor->GetEditorWorldContext().World();
//                GEditor->Exec(ew, *cmd, *GLog);
//            }
//            goto OK;
//        }
//
//        /*
//         *  HTTP GET /shutdown-now
//         *
//         *  Trigger an editor immediate (possibly unclean) shutdown.
//         */
//        else if (matches_any(&msg->uri, "/shutdown-now", "/ue4/shutdown-now"))
//        {
//            FGenericPlatformMisc::RequestExit(true);
//            goto OK;
//        }
//
//        /*
//         *  HTTP GET /shutdown
//         *
//         *  Trigger an editor shutdown.
//         */
//        else if (matches_any(&msg->uri, "/shutdown", "/ue4/shutdown"))
//        {
//            FGenericPlatformMisc::RequestExit(false);
//            goto OK;
//        }
//
//        /*
//         *  HTTP GET /build
//         *
//         *  Trigger a build all for the current level.
//         */
//        else if (matches_any(&msg->uri, "/build", "/ue4/build"))
//        {
//            FLevelEditorActionCallbacks::Build_Execute();
//            goto OK;
//        }
//
//        /*
//         *  HTTP GET /is_building
//         *
//         *  Returns TRUE if the editor is currently building, FALSE otherwise.
//         */
//        else if (matches_any(&msg->uri, "/is_building", "/ue4/is_building"))
//        {
//            bool ok = FLevelEditorActionCallbacks::Build_CanExecute();
//            goto NOT_IMPLEMENTED;
//        }
//
//        /*
//         *  HTTP GET /list_assets
//         *
//         *  Logs all the assets that are registered with the asset manager.
//         */
//        else if (matches_any(&msg->uri, "/list_assets", "/ue4/list_assets"))
//        {
//            TArray<FAssetData> AssetData;
//            AssetRegistry.Get().GetAllAssets(AssetData);
//            for (auto data : AssetData)
//            {
//                FString path = *(data.PackageName.ToString());
//                FPaths::MakePlatformFilename(path);
//                LOG("%s %s", *(data.PackageName.ToString()), *path);
//            }
//            goto OK;
//        }
//
//        /*
//         *  HTTP GET /assets_idle
//         *
//         *  Returns OK if the importer is idle.  Returns a status code to try
//         *  again otherwise.
//         */
//        else if (matches_any(&msg->uri, "/assets_idle", "/ue4/assets_idle"))
//        {
//            if (AssetRegistry.Get().IsLoadingAssets())
//                goto ONE_MO_TIME;
//            goto OK;
//        }
//
//        /*
//         *  HTTP GET /debug
//         *
//         *  Catch-all debug endpoint.
//         */
//        else if (matches_any(&msg->uri, "/debug", "/ue4/debug"))
//        {
//            debugFn("");
//            goto OK;
//        }
//
//        //else if (matches_any(&msg->uri, "/gc"))
//        //{
//        //    URCHTTP::Get()->GarbageCollect();
//        //    goto OK;
//        //}
//
//        goto BAD_ACTION;
//    }
//
//    /*
//     *  HTTP POST commands
//     */
//    else if (mg_vcmp(&msg->method, "POST") == 0)
//    {
//        FString body;
//        if (msg->body.len > 0)
//        {
//            body = FString::Printf(T("%.*s"), msg->body.len,
//                                   UTF8_TO_TCHAR(msg->body.p));
//        }
//
//        /*
//         *  HTTP POST /poll_interval
//         *
//         *  POST body should contain an int which specifies the polling
//         *  interval.  This value should be positive to delay the polling,
//         *  and can be set to 0 (or negative) to assume default behavior of
//         *  one poll() per tick().
//         */
//        if (matches_any(&msg->uri, "/poll_interval"))
//        {
//            int x = FCString::Atoi(*body);
//            if (x < 0)
//                x = 0;
//            server->SetPollInterval(x);
//            goto OK;
//        }
//
//        /*
//         *  HTTP POST /command
//         *
//         *  POST body should contain the exact console command that is to be
//         *  run in the UE4 console.
//         */
//        else if (matches_any(&msg->uri, "/command", "/ue4/command"))
//        {
//            if (body.Len() > 0)
//            {
//                auto ew = GEditor->GetEditorWorldContext().World();
//                GEditor->Exec(ew, *body, *GLog);
//                goto OK;
//            }
//            goto BAD_ENTITY;
//        }
//
//        /*
//         *  HTTP POST /ue4/loadpak
//         *
//         *  POST body should contain a comma separated list of the following two
//         *  arguments:
//         *  1. Local .pak file path to mount into the engine.
//         *  2. "all" or "none" to indicate if the pak's content should be loaded
//         *
//         */
//        //else if (matches_any(&msg->uri, "/loadpak", "/ue4/loadpak"))
//        //{
//        //    if (body.Len() > 0)
//        //    {
//        //        int32 num_params;
//        //        TArray<FString> pak_options;
//
//        //        FString pakPath;
//
//        //        body.TrimEndInline();
//
//        //        num_params = body.ParseIntoArray(pak_options, T(","), true);
//        //        pakPath = pak_options[0];
//
//        //        if (pakPath.Len() == 0)
//        //            goto ERROR;
//
//        //        if (num_params != 2)
//        //            goto ERROR;
//
//        //        LOG("Mounting pak file: %s", *pakPath);
//
//        //        if (URCHTTP::Get()->MountPakFile(pakPath, pak_options[1] == T("all")) < 0)
//        //            goto ERROR;
//
//        //        goto OK;
//        //    }
//        //    goto BAD_ENTITY;
//        //}
//
//        //else if (matches_any(&msg->uri, "/loadobj", "/ue4/loadobj"))
//        //{
//        //    if (body.Len() > 0)
//        //    {
//        //        body.TrimEndInline();
//
//        //        TArray<FString> objects;
//        //        int32 num_params = body.ParseIntoArray(objects, T(","), true);
//
//        //        if (num_params==1)
//        //        {
//        //            if (URCHTTP::Get()->LoadObject(objects[0]) != nullptr)
//        //                goto OK;
//        //            goto ERROR;
//        //        }
//        //    }
//        //    goto BAD_ENTITY;
//        //}
//
//        //else if (matches_any(&msg->uri, "/unloadobj", "/ue4/unloadobj"))
//        //{
//        //    if (body.Len() > 0)
//        //    {
//        //        body.TrimEndInline();
//
//        //        TArray<FString> objects;
//        //        int32 num_params = body.ParseIntoArray(objects, T(","), true);
//
//        //        if (num_params == 1)
//        //        {
//        //            if (URCHTTP::Get()->UnloadObject(objects[0]) < 0)
//        //                goto ERROR;
//        //            goto OK;
//        //        }
//        //    }
//        //    goto BAD_ENTITY;
//        //}
//
//        /*
//         *  HTTP POST /debug
//         *
//         *  Catch-all debug endpoint.
//         */
//        else if (matches_any(&msg->uri, "/debug", "/ue4/debug"))
//        {
//            debugFn(body);
//            goto OK;
//        }
//        goto BAD_ACTION;
//    }
//#endif // WITH_EDITOR
//
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wunused-label"
//  ERROR:
//    rspMsg    = STATUS_ERROR;
//    rspStatus = 501;
//    goto done;
//
//  BAD_ACTION:
//    rspMsg    = STATUS_BAD_ACTION;
//    rspStatus = 500;
//    goto done;
//
//  BAD_ENTITY:
//    rspMsg    = STATUS_BAD_ENTITY;
//    rspStatus = 422;
//    goto done;
//
//  ONE_MO_TIME:
//    rspMsg    = STATUS_TRY_AGAIN;
//    rspStatus = 416;
//    goto done;
//
//  NOT_IMPLEMENTED:
//    rspMsg    = STATUS_NOT_IMPLEMENTED;
//    rspStatus = 500;
//    goto done;
//
//  OK:
//    rspMsg    = STATUS_OK;
//    rspStatus = 200;
//#pragma GCC diagnostic pop
//
//  done:
//    mg_send_head(conn, rspStatus, rspMsg.len, "Content-Type: text/plain");
//    mg_printf(conn, "%s", rspMsg.p);
}

////////////////////////////////////////////////////////////////////////////////

URCHTTP*
URCHTTP::Get()
{
    UObject *URCHTTP_cdo = URCHTTP::StaticClass()->GetDefaultObject(true);
    return Cast<URCHTTP>(URCHTTP_cdo);
}

URCHTTP*
URCHTTP::GetURCHTTP()
{
    return
        IsValid(httpInstance) ?
        httpInstance :
        httpInstance = NewObject<URCHTTP>(
            this,
            FName("URCHTTP"));
}


////////////////////////////////////////////////////////////////////////////////

URCHTTP::URCHTTP(const FObjectInitializer& oi)
    : Super(oi), poll_interval(0), poll_ms(1)
{
    //// Initialize .pak file reader
    //if (PakFileMgr == nullptr)
    //{
    //    PakFileMgr = new FPakPlatformFile;
    //    PakFileMgr->Initialize(&FPlatformFileManager::Get().GetPlatformFile(), T(""));
    //    PakFileMgr->InitializeNewAsyncIO();
    //}
}

URCHTTP::~URCHTTP()
{
    mg_mgr_free(&mgr);
}

////////////////////////////////////////////////////////////////////////////////

void
URCHTTP::Init()
{
    // Initialize HTTPD server
    mg_mgr_init(&mgr, NULL);
    bind_opts.error_string = &err_str;
    bind_opts.user_data = this;
    conn = mg_bind_opt(&mgr, "18001", ev_handler, bind_opts);
    mg_set_protocol_http_websocket(conn);

    UE_LOG(LogUE4Orc, Warning, TEXT("Bound UE4 Orchestrator to %d"), 18001);
}

void
URCHTTP::SetPollInterval(int v)
{
    poll_interval = v;
}

void
URCHTTP::Tick(float dt)
{
    if (&mgr == nullptr) return;

    static int tick_counter = 0;
    if (tick_counter == 0)
        Init();
    //    mg_set_protocol_http_websocket(conn);

    if (poll_interval == 0 || (tick_counter++ % poll_interval) == 0)
        mg_mgr_poll(&mgr, poll_ms);

    if (tick_counter == 0)
        tick_counter++;
}

TStatId
URCHTTP::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URCHTTP, STATGROUP_Tickables);
}

void
URCHTTP::Serialize(FArchive& ar)
{
    Super::Serialize(ar);
}

void
URCHTTP::PostLoad()
{
    Super::PostLoad();
}

void
URCHTTP::PostInitProperties()
{
    Super::PostInitProperties();
}

#if WITH_EDITOR
void
URCHTTP::PostEditChangeProperty(FPropertyChangedEvent& evt)
{
    Super::PostEditChangeProperty(evt);
}
#endif

////////////////////////////////////////////////////////////////////////////////
