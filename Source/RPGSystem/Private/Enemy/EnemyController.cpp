// EnemyController.cpp

#include "Enemy/EnemyController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "BehaviorTree/BlackboardComponent.h"

AEnemyController::AEnemyController()
{
	// 1. Perception Component 생성
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
	
	// 2. 시각(Sight) 설정
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1000.0f; // 감지 반경
	SightConfig->LoseSightRadius = 1200.0f; // 시야 상실 반경
	SightConfig->PeripheralVisionAngleDegrees = 60.0f; // 시야각
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	AIPerceptionComp->ConfigureSense(*SightConfig);

	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 2000.0f;
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	AIPerceptionComp->ConfigureSense(*HearingConfig);

	// 4. 데미지(Damage) 설정
	DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
	AIPerceptionComp->ConfigureSense(*DamageConfig);

	// 시각을 주 감각으로 설정
	AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

	// 초기 상태
	CurrentState = EEnemyAIState::Idle;
}

void AEnemyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AEnemyController::BeginPlay()
{
	Super::BeginPlay();
	
	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyController::OnTargetDetected);
	}
}

void AEnemyController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	// 감지된 대상이 플레이어인지 확인 (태그나 클래스로 구분)
	// 예: if (!Actor->ActorHasTag("Player")) return;

	// 감각 종류에 따른 처리
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
	{
		// 데미지를 입으면 즉시 추적 또는 공격 태세 전환
		TargetActor = Actor;
		SetState(EEnemyAIState::Chase);
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		// 시각 또는 청각으로 감지 성공
		TargetActor = Actor;
		SetState(EEnemyAIState::Chase);
	}
	else
	{
		// 시야에서 사라짐 -> 일정 시간 후 복귀 로직 등
		// TargetActor = nullptr; // 바로 잃어버리게 할지, 마지막 위치로 갈지는 선택
	}
}

void AEnemyController::SetState(EEnemyAIState NewState)
{
	if (CurrentState == NewState) return;
	
	CurrentState = NewState;

	// 상태 변경 시 초기화 로직 (예: 공격 애니메이션 준비 등)
	switch (CurrentState)
	{
	case EEnemyAIState::Chase:
		// 이동 속도 증가 등
		break;
	case EEnemyAIState::Attack:
		// 공격 로직 시작
		break;
		// ...
	}
}