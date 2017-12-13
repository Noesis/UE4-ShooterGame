// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Online/ShooterPlayerState.h"
#include "GameDelegates.h"

#include "UObject/PackageReload.h"

//#include "Runtime/RHI/Public/RHICommandlist.h"

#if !UE_BUILD_SHIPPING

#if PLATFORM_PS4
#include "PS4ChunkInstall.h"
static void PlayGoNext()
{
	IPlatformChunkInstall* ChunkInstaller = FPlatformMisc::GetPlatformChunkInstall();
	if (ChunkInstaller)
	{
		ChunkInstaller->DebugStartNextChunk();
	}
}

FAutoConsoleCommand CmdPlayGoNext(
	TEXT("r.PlayGoNext"),
	TEXT("Tell PlayGo to start downloading the next chunk."),
	FConsoleCommandDelegate::CreateStatic(PlayGoNext)
	);
#endif

#endif
#include "ShooterGameState.h"


// respond to requests from a companion app
static void WebServerDelegate(int32 UserIndex, const FString& Action, const FString& URL, const TMap<FString, FString>& Params, TMap<FString, FString>& Response)
{
	if (URL == TEXT("/index.html?scoreboard"))
	{
		FString ScoreboardStr = TEXT("{ \"scoreboard\" : [ ");

		// you shouldn't normally use this method to get a UWorld as it won't always be correct in a PIE context.
		// However, the PS4 companion app server will never run in the Editor.
		UGameEngine* GameEngine = CastChecked<UGameEngine>(GEngine);
		if (GameEngine)
		{
			UWorld* World = GameEngine->GetGameWorld();
			if (World)
			{			
				ULocalPlayer *Player = GEngine->GetFirstGamePlayer(World);
				if (Player)
				{
					// get the shoter game
					AShooterGameState* const GameState = Player->PlayerController->GetWorld()->GetGameState<AShooterGameState>();


					RankedPlayerMap Players;
					GameState->GetRankedMap(0, Players);

					bool bNeedsComma = false;
					for (auto It = Players.CreateIterator(); It; ++It)
					{
						if (bNeedsComma)
						{
							ScoreboardStr += TEXT(" ,");
						}
						ScoreboardStr += FString::Printf(TEXT(" { \"n\" : \"%s\" , \"k\" : \"%d\" , \"d\" : \"%d\" }"), *It.Value()->GetShortPlayerName(), It.Value()->GetKills(), It.Value()->GetDeaths());
						bNeedsComma = true;
					}
				}

				ScoreboardStr += TEXT(" ] }");

				Response.Add(TEXT("Content-Type"), TEXT("text/html; charset=utf-8"));
				Response.Add(TEXT("Body"), ScoreboardStr);
			}
		}
	}
}

static void AssignLayerChunkDelegate(const FAssignLayerChunkMap* ChunkManifest, const FString& Platform, const int32 ChunkIndex, int32& OutChunkLayer)
{
	OutChunkLayer = 0;

	static FString PS4PlatformString(TEXT("PS4"));
	if (Platform.Compare(TEXT("PS4"), ESearchCase::IgnoreCase) == 0)
	{	
		// test dual layer BD50 packaging.
		switch (ChunkIndex)
		{
			case 0:
			case 1:
			default:
				OutChunkLayer = 0;
				break;
			case 2:
				OutChunkLayer = 1;
				break;
		}
	}
}

static void ExtendedSaveGameInfoDelegate(const TCHAR* SaveName, const EGameDelegates_SaveGame Key, FString& Value)
{
	static const int32 MAX_SAVEGAME_SIZE = 100 * 1024;
	switch(Key)
	{
		case EGameDelegates_SaveGame::MaxSize:			
			Value = FString::Printf(TEXT("%i"), MAX_SAVEGAME_SIZE);
			break;
		case EGameDelegates_SaveGame::Title:
			Value = TEXT("ShooterGame");
			break;
		case EGameDelegates_SaveGame::SubTitle:
			Value = TEXT("The Shootening");
			break;
		case EGameDelegates_SaveGame::Detail:
			Value = TEXT("ShooterGame User Settings");
			break;
		default:
			break;
	}
}

static void ReloadHandler( EPackageReloadPhase ReloadPhase, FPackageReloadedEvent* Event)
{
	if ( ReloadPhase == EPackageReloadPhase::PostPackageFixup)
	{
		// reinitialize allthe material instances


		/*{
			// fixup uniform expressions
			UMaterialInterface::RecacheAllMaterialUniformExpressions();
		}

		ENQUEUE_UNIQUE_RENDER_COMMAND(
		FRecreateBoundShaderStates,
		{
			RHIRecreateRecursiveBoundShaderStates();
		});*/


		/*for (TObjectIterator<UMaterialInstance> It; It; ++It)
		{
			UMaterialInstance* Material = *It;
			//Material->InitResources();
			Material->RebuildResource();
		}*/
	}
}

#define EXPERIMENTAL_ENABLEHOTRELOAD 0
static void ReloadPackagesCallback( const TArray<FString>& PackageNames)
{
#if EXPERIMENTAL_ENABLEHOTRELOAD
	TArray<UPackage*> PackagesToReload;
	TArray<UPackage*> MaterialPackagesToReload;
	for (const FString& PackageName : PackageNames)
	{
		UPackage* Package = FindPackage(nullptr, *PackageName);

		if (Package == nullptr)
		{
			// UE_LOG(, Log, TEXT("Unable to find package in memory %s"), *PackageName);
		}
		else
		{
			if ( Package->HasAnyPackageFlags(PKG_ContainsMap || PKG_ContainsMap) )
			{
				continue;
			}
			PackagesToReload.Add(Package);
		}
	}


	// see what's in these packages

	if (PackagesToReload.Num())
	{
		SortPackagesForReload(PackagesToReload);

		TArray<FReloadPackageData> PackagesToReloadData;
		PackagesToReloadData.Empty(PackagesToReload.Num());
		for (UPackage* PackageToReload : PackagesToReload)
		{
			PackagesToReloadData.Emplace(PackageToReload, LOAD_None);
		}

		TArray<UPackage*> ReloadedPackages;

		FDelegateHandle Handle = FCoreUObjectDelegates::OnPackageReloaded.AddStatic(&ReloadHandler);

		FText ErrorMessage;
		GShouldVerifyGCAssumptions = false;
		GUObjectArray.DisableDisregardForGC();

		::ReloadPackages(PackagesToReloadData, ReloadedPackages, 500);

		FCoreUObjectDelegates::OnPackageReloaded.Remove(Handle);
	}
#endif
}

void InitializeShooterGameDelegates()
{
	FGameDelegates::Get().GetWebServerActionDelegate() = FWebServerActionDelegate::CreateStatic(WebServerDelegate);
	FGameDelegates::Get().GetAssignLayerChunkDelegate() = FAssignLayerChunkDelegate::CreateStatic(AssignLayerChunkDelegate);
	FGameDelegates::Get().GetExtendedSaveGameInfoDelegate() = FExtendedSaveGameInfoDelegate::CreateStatic(ExtendedSaveGameInfoDelegate);

	FCoreUObjectDelegates::NetworkFileRequestPackageReload.BindStatic(&ReloadPackagesCallback);
}
