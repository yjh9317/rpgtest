// RPGEffect.cpp

#include "Status/Effects/RPGEffect.h"

URPGEffect::URPGEffect()
{
    // Set reasonable defaults
    DurationType = EEffectDurationType::Instant;
    Duration = 0.0f;
    Period = 1.0f;
    BaseMagnitude = 0.0f;
    ScalingCoefficient = 0.0f;
    StackPolicy = EEffectStackPolicy::Refresh;
    MaxStacks = 1;
}

float URPGEffect::CalculateMagnitude(float SourceAttributeValue, int32 CurrentStacks) const
{
    // Base magnitude
    float FinalMagnitude = BaseMagnitude;

    // Add attribute scaling if configured
    if (MagnitudeScalingAttribute.IsValid() && ScalingCoefficient > 0.0f)
    {
        FinalMagnitude += SourceAttributeValue * ScalingCoefficient;
    }

    // Apply stack multiplier if using magnitude stacking
    if (StackPolicy == EEffectStackPolicy::StackMagnitude)
    {
        FinalMagnitude *= FMath::Clamp(CurrentStacks, 1, MaxStacks);
    }

    return FinalMagnitude;
}

int32 URPGEffect::GetTotalTicks() const
{
    if (!IsPeriodic() || Period <= 0.0f || Duration <= 0.0f)
    {
        return 0;
    }

    // Calculate how many times the effect will tick
    // Example: 10 second duration, 2 second period = 5 ticks
    return FMath::FloorToInt(Duration / Period);
}

#if WITH_EDITOR
void URPGEffect::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    const FName PropertyName = PropertyChangedEvent.GetPropertyName();

    // Auto-adjust duration to 0 for instant effects
    if (PropertyName == GET_MEMBER_NAME_CHECKED(URPGEffect, DurationType))
    {
        if (DurationType == EEffectDurationType::Instant)
        {
            Duration = 0.0f;
        }
        else if (Duration <= 0.0f)
        {
            Duration = 1.0f; // Set a reasonable default
        }
    }

    // Ensure period doesn't exceed duration for periodic effects
    if (PropertyName == GET_MEMBER_NAME_CHECKED(URPGEffect, Period) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(URPGEffect, Duration))
    {
        if (DurationType == EEffectDurationType::Periodic)
        {
            if (Period > Duration)
            {
                Period = Duration;
            }
            if (Period < 0.1f)
            {
                Period = 0.1f; // Minimum period to prevent performance issues
            }
        }
    }

    // Validate MaxStacks
    if (PropertyName == GET_MEMBER_NAME_CHECKED(URPGEffect, MaxStacks))
    {
        MaxStacks = FMath::Clamp(MaxStacks, 1, 99);
    }
}
#endif
