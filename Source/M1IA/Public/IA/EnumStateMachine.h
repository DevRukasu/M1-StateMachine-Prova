#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EnumStateMachine.generated.h"

UENUM(BlueprintType)
enum class EAIState : uint8
{
    // Estados neutros
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),

    // Status
    Healing     UMETA(DisplayName = "Healing"),
    Dead        UMETA(DisplayName = "Dead"),
    Retreat     UMETA(DisplayName = "Retreat"),

    // Ataques
    Attack          UMETA(DisplayName = "Attack"),
    RangedAttack    UMETA(DisplayName = "RangedAttack"),

    // Especial (Boss ou Guardião)
    SpecialAttack   UMETA(DisplayName = "SpecialAttack")
};
