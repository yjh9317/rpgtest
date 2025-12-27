// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/EnvQuery/EnvQueryContext_Target.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Enemy/EnemyController.h" // AEnemyController 헤더 경로 주의
#include "GameFramework/Actor.h"

void UEnvQueryContext_Target::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	// 1. 쿼리를 요청한 주체(AI Controller)를 가져옵니다.
	AActor* QueryOwner = Cast<AActor>(QueryInstance.Owner.Get());
	AEnemyController* EnemyPC = Cast<AEnemyController>(QueryOwner);

	// 만약 Owner가 Pawn이라면 Controller를 통해 가져옵니다.
	if (!EnemyPC)
	{
		APawn* PawnOwner = Cast<APawn>(QueryOwner);
		if (PawnOwner)
		{
			EnemyPC = Cast<AEnemyController>(PawnOwner->GetController());
		}
	}

	// 2. 컨트롤러에서 우리가 저장해둔 TargetActor를 꺼냅니다.
	if (EnemyPC && EnemyPC->GetTargetActor())
	{
		// 3. EQS 시스템에 "이게 그 타겟이야"라고 알려줍니다.
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, EnemyPC->GetTargetActor());
	}
}
