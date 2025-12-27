// Fill out your copyright notice in the Description page of Project Settings.


#include "Command/Components/CommandBufferComponent.h"

#include "Combat/Action/BaseAction.h"
#include "Combat/Action/Components/ActionComponent.h"
#include "Command/InputMappingData.h"

UCommandBufferComponent::UCommandBufferComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f;
}

bool UCommandBufferComponent::HasBufferedAction(const FGameplayTag& ActionTag) const
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (const FBufferedCommand& Command : CommandBuffer)
    {
        if (Command.ActionTag == ActionTag && !Command.IsExpired(CurrentTime, BufferWindow))
        {
            return true;
        }
    }
    return false;
}

float UCommandBufferComponent::GetLastInputTime(const FGameplayTag& ActionTag) const
{
    for (int32 i = CommandBuffer.Num() - 1; i >= 0; i--)
    {
        if (CommandBuffer[i].ActionTag == ActionTag)
        {
            return CommandBuffer[i].Timestamp;
        }
    }
    return -1.0f;
}

bool UCommandBufferComponent::ConsumeBufferedCommand(const FGameplayTag& ActionTag)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = CommandBuffer.Num() - 1; i >= 0; i--)
    {
        if (CommandBuffer[i].ActionTag == ActionTag && 
            !CommandBuffer[i].IsExpired(CurrentTime, BufferWindow))
        {
            CommandBuffer.RemoveAt(i);
            return true;
        }
    }
    return false;
}

FBufferedCommand UCommandBufferComponent::GetLastBufferedCommand(const FGameplayTag& ActionTag) const
{
    for (int32 i = CommandBuffer.Num() - 1; i >= 0; i--)
    {
        if (CommandBuffer[i].ActionTag == ActionTag)
            return CommandBuffer[i];
    }
    return FBufferedCommand();
}

bool UCommandBufferComponent::HasRecentInput(const FGameplayTag& ActionTag, float TimeWindow) const
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (const FBufferedCommand& Command : CommandBuffer)
    {
        if (Command.ActionTag == ActionTag)
        {
            float TimeSinceInput = CurrentTime - Command.Timestamp;
            if (TimeSinceInput <= TimeWindow)
                return true;
        }
    }
    return false;
}

void UCommandBufferComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (DefaultMappingData)
    {
        InitializeFromDataAsset(DefaultMappingData);
    }

    APlayerController* MyPlayerController = Cast<APlayerController>(GetOwner());
    if (!MyPlayerController) return; // 안전 체크 추가

    APawn* ControlledPawn = MyPlayerController->GetPawn();
    if(ControlledPawn)
    {
        // IComponentManager* ComponentManager = Cast<IComponentManager>(ControlledPawn); 
        // if(ComponentManager)
        // {
        //     CachedActionComponent = ComponentManager->GetActionComponent();    
        // }
    }
}

void UCommandBufferComponent::TickComponent(float DeltaTime, ELevelTick TickType, 
                                         FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    CleanExpiredCommands();

    if (HasBufferedCommand())
    {
        ProcessNextBufferedCommand();
    }
}

void UCommandBufferComponent::BufferCommand(const FGameplayTag& InputTag, 
                                       const FVector& InputDir, float Magnitude)
{
    FGameplayTag ActionTag = InputTag;
    FGameplayTag* MappedAction = CommandToActionMapping.Find(InputTag);
    if (!MappedAction)
    {
        return;
    }
    ActionTag = *MappedAction;
    
    if (ShouldExecuteImmediately(ActionTag))
    {
        CachedActionComponent->ExecuteAction(ActionTag);
        return;
    }
    
    FBufferedCommand NewCommand;
    NewCommand.ActionTag = InputTag;  
    NewCommand.MappedActionTag = ActionTag;
    NewCommand.Timestamp = GetWorld()->GetTimeSeconds();
    NewCommand.InputDirection = InputDir;
    NewCommand.InputMagnitude = Magnitude;
    CommandBuffer.Add(NewCommand);

    while (CommandBuffer.Num() > MaxBufferSize)
    {
       CommandBuffer.RemoveAt(0);
    }
}

bool UCommandBufferComponent::ProcessNextBufferedCommand()
{
    if (!CachedActionComponent || CommandBuffer.Num() == 0)
        return false;
    
    for (int32 i = CommandBuffer.Num() - 1; i >= 0; i--)
    {
        if (TryExecuteAction(CommandBuffer[i]))
        {
            CommandBuffer.RemoveAt(i);
            return true;
        }
    }
    
    return false;
}


bool UCommandBufferComponent::TryExecuteAction(const FBufferedCommand& Command)
{
    if (!CachedActionComponent)
        return false;

    FGameplayTag ActionToExecute = Command.MappedActionTag.IsValid() ? Command.MappedActionTag : Command.ActionTag;
    
    if (UBaseAction* Action = CachedActionComponent->GetAction(ActionToExecute))
    {
        return CachedActionComponent->ExecuteAction(ActionToExecute);
    }
    
    return false;
}

bool UCommandBufferComponent::ShouldExecuteImmediately(const FGameplayTag& ActionTag) const
{
    if (!CachedActionComponent)
        return false;
    
    // 동일한 액션이 이미 활성이면 버퍼링만
    UBaseAction* Action = CachedActionComponent->GetAction(ActionTag);
    if (Action && Action->IsActive())
        return false;
    
    return CachedActionComponent->GetActiveActionCount() == 0;
}

void UCommandBufferComponent::InitializeFromDataAsset(UInputMappingData* MappingData)
{
    if (!MappingData)
        return;
    
    CommandToActionMapping = MappingData->InputToActionMap;
}

void UCommandBufferComponent::AddCommandMapping(const FGameplayTag& InputTag, const FGameplayTag& ActionTag)
{
    if (!InputTag.IsValid() || !ActionTag.IsValid())
        return;
    
    CommandToActionMapping.Add(InputTag, ActionTag);
}

void UCommandBufferComponent::RemoveCommandMapping(const FGameplayTag& InputTag)
{
    if (CommandToActionMapping.Remove(InputTag) > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Removed mapping for: %s"), *InputTag.ToString());
    }
}

void UCommandBufferComponent::ClearAllMappings()
{
    CommandToActionMapping.Empty();
}

void UCommandBufferComponent::SwapMappingData(UInputMappingData* NewMappingData)
{
    if (!NewMappingData)
        return;
    
    ClearAllMappings();
    
    InitializeFromDataAsset(NewMappingData);
}

FGameplayTag UCommandBufferComponent::GetMappedAction(const FGameplayTag& InputTag) const
{
    if (const FGameplayTag* MappedTag = CommandToActionMapping.Find(InputTag))
    {
        return *MappedTag;
    }
    return InputTag; // 매핑 없으면 원본 반환
}

void UCommandBufferComponent::CleanExpiredCommands()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    CommandBuffer.RemoveAll([CurrentTime, this](const FBufferedCommand& Command)
    {
        return Command.IsExpired(CurrentTime, BufferWindow);
    });
}


