#include "FPSBombActor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
AFPSBombActor::AFPSBombActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	RootComponent = MeshComp;

	ExplodeDelay = 2.0F;
}

void AFPSBombActor::Explode()
{
	UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionTemplate, GetActorLocation());

	FCollisionObjectQueryParams QueryParams;
	QueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);

	FCollisionShape CollShape;
	CollShape.SetSphere(500.0f);

	TArray<FOverlapResult> OutOverlaps;
	GetWorld()->OverlapMultiByObjectType(OutOverlaps, GetActorLocation(), FQuat::Identity, QueryParams, CollShape);

	for (FOverlapResult Result : OutOverlaps)
	{
		UPrimitiveComponent* Overlap = Result.GetComponent();
		if (Overlap && Overlap->IsSimulatingPhysics())
		{
			UMaterialInstanceDynamic* MatInst = Overlap->CreateAndSetMaterialInstanceDynamic(0);
			if (MatInst)
			{
				MatInst->SetVectorParameterValue(FName("Base Color"), TargetColor);
			}
		}
	}

	Destroy();
}

// Called when the game starts or when spawned
void AFPSBombActor::BeginPlay()
{
	Super::BeginPlay();
	
	FTimerHandle Explode_TimerHandle;
	GetWorldTimerManager().SetTimer(Explode_TimerHandle, this, &AFPSBombActor::Explode, ExplodeDelay);

	MaterialInst = MeshComp->CreateAndSetMaterialInstanceDynamic(0);
	if (MaterialInst)
	{
		CurrentColor = MaterialInst->K2_GetVectorParameterValue("Color");
	}

	TargetColor = FLinearColor::MakeRandomColor();
}

// Called every frame
void AFPSBombActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MaterialInst)
	{
		float Progress = (GetWorld()->TimeSeconds - CreationTime) / ExplodeDelay;

		FLinearColor NewColor = FLinearColor::LerpUsingHSV(CurrentColor, TargetColor, Progress);

		MaterialInst->SetVectorParameterValue(FName("Base Color"), NewColor);
	}
}

