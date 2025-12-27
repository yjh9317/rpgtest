// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/Animation/Notifies/ANS_RPGTrail.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

void UANS_RPGTrail::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp && NiagaraTemplate)
	{
		// 1. 나이아가라 스폰 (무기 소켓에 부착)
		// 주의: 실제로는 MeshComp가 아니라 무기 액터나 무기 컴포넌트를 찾아야 할 수도 있습니다. 
		// 소켓이 캐릭터 메시에 있다면 MeshComp 그대로 사용.
		UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			NiagaraTemplate,
			MeshComp,
			SocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true // AutoDestroy
		);

		// 2. 파라미터 주입
		if (NiagaraComp)
		{
			// 여기서 아까 만든 변수들을 나이아가라로 쏴줍니다.
			//NiagaraComp->SetVariableLinearColor(ParamName_Color, TrailColor);
			NiagaraComp->SetVariableFloat(ParamName_Width, TrailWidth);
			
			// 나중에 NotifyEnd에서 끄기 위해 컴포넌트를 저장하거나 태그를 활용해야 할 수 있습니다.
			// 하지만 보통 SpawnSystemAttached로 생성된 녀석은 NotifyState가 끝날 때 자동으로 사라지지 않으므로,
			// NotifyEnd에서 파괴하거나 Deactivate 하는 로직이 추가로 필요합니다.
			// (간단하게 하려면 컴포넌트에 태그를 붙여서 End에서 찾는 방식을 씁니다)
            
			// 예시: 찾기 위해 태그 설정
			NiagaraComp->ComponentTags.Add(FName("ActiveTrail")); 
		}
	}
}

void UANS_RPGTrail::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp)
	{
		// 생성된 나이아가라를 찾아서 종료
		TArray<USceneComponent*> Children;
		MeshComp->GetChildrenComponents(true, Children);

		for (USceneComponent* Child : Children)
		{
			UNiagaraComponent* NC = Cast<UNiagaraComponent>(Child);
			if (NC && NC->ComponentTags.Contains(FName("ActiveTrail")))
			{
				NC->Deactivate(); // 서서히 사라지게 함 (Destroy 아님)
				NC->ComponentTags.Remove(FName("ActiveTrail")); // 태그 제거
			}
		}
	}
}
