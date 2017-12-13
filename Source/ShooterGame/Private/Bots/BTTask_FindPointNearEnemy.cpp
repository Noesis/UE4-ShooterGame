// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Bots/BTTask_FindPointNearEnemy.h"
#include "Bots/ShooterAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"


UBTTask_FindPointNearEnemy::UBTTask_FindPointNearEnemy(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
}

EBTNodeResult::Type UBTTask_FindPointNearEnemy::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AShooterAIController* MyController = Cast<AShooterAIController>(OwnerComp.GetAIOwner());
	if (MyController == NULL)
	{
		return EBTNodeResult::Failed;
	}
	
	APawn* MyBot = MyController->GetPawn();
	AShooterCharacter* Enemy = MyController->GetEnemy();
	if (Enemy && MyBot)
	{
		const float SearchRadius = 200.0f;
		const FVector SearchOrigin = Enemy->GetActorLocation() + 600.0f * (MyBot->GetActorLocation() - Enemy->GetActorLocation()).GetSafeNormal();
		FVector Loc(0);
		UNavigationSystem::K2_GetRandomReachablePointInRadius(MyController, SearchOrigin, Loc, SearchRadius);
		if (Loc != FVector::ZeroVector)
		{
			OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID(), Loc);
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}
