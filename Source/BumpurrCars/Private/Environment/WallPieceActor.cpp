// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/WallPieceActor.h"

#include "Characters/BumperCarPawn.h"

#include "NiagaraFunctionLibrary.h"
#include "GameFramework/ForceFeedbackEffect.h"
#include <Kismet/GameplayStatics.h>

DEFINE_LOG_CATEGORY(LogWallPiece);

AWallPieceActor::AWallPieceActor()
{
	PrimaryActorTick.bCanEverTick = true;

	WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
	SetRootComponent(WallMesh);
}

bool AWallPieceActor::IsWallOpen()
{
	return bIsWallOpen;
}

void AWallPieceActor::OpenWall()
{
	if (!ActorHasTag("OpenableWall"))
	{
		UE_LOG(LogWallPiece, Error, TEXT("Wall does not have 'OpenableWall' tag!"));
		return;
	}

	if (bIsWallOpen)
	{
		//already open do nothing
		return;
	}
	
	UE_LOG(LogWallPiece, Log, TEXT("Wall is preparing to open"));
	bIsWallOpen = true;

	if (PreOpenMaterial)
	{
		WallMesh->SetMaterial(0, PreOpenMaterial);
	}

	//delay the opening logic
	GetWorldTimerManager().SetTimer(
		DelayHandle,
		this,
		&AWallPieceActor::FinishOpenWall,
		PreOpenDelay
	);
}

void AWallPieceActor::CloseWall()
{
	if (!ActorHasTag("OpenableWall"))
	{
		UE_LOG(LogWallPiece, Warning, TEXT("Wall does not have 'OpenableWall' tag!"));
		return;
	}

	if (bIsWallOpen)
	{
		UE_LOG(LogWallPiece, Log, TEXT("Closed Wall"));

		bIsWallOpen = false;

		WallMesh->SetVisibility(true);
		WallMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		if (DefaultMaterial)
		{
			WallMesh->SetMaterial(0, DefaultMaterial);
		}
	}
}

void AWallPieceActor::BeginPlay()
{
	Super::BeginPlay();

	WallMesh->OnComponentHit.AddDynamic(this, &AWallPieceActor::OnComponentHit);
}

void AWallPieceActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWallPieceActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	Super::EndPlay(EndPlayReason);
}

void AWallPieceActor::FinishOpenWall()
{
	WallMesh->SetVisibility(false);
	WallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UE_LOG(LogWallPiece, Log, TEXT("Wall is open"));
}

void AWallPieceActor::OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (auto Car{ Cast<ABumperCarPawn>(OtherActor) }; Car)
	{
		FVector const Forward{ Car->GetActorForwardVector().GetSafeNormal() };
		auto const RevHit{ FHitResult::GetReversedHit(Hit) };
		FVector const WallNormal{ RevHit.ImpactNormal.GetSafeNormal() };
		float const FacingDot{ static_cast<float>(FVector::DotProduct(Forward, -WallNormal)) };

		//UE_LOG(LogTemp, Error, TEXT("Hit Wall"));


		if (FacingDot > PushBackFacingThreshold)
		{
			Car->ResetAcceleration();
			if (Car->GetEstimatedPreCollisionVelocity().Size() <= VelThresholdForPushback * 100.f and Car->HasControl())
			{
				//UE_LOG(LogTemp, Error, TEXT("Launch back"));

				float const Force{ static_cast<float>(VelThresholdForPushback * 100.f) * PushBackModifier };

				//UE_LOG(LogTemp, Error, TEXT("Launch back Force: %f"), Force);

				FVector const PushDir{ RevHit.ImpactNormal.GetSafeNormal() };
				FVector const PushVector{ PushDir * Force };

				Car->Launch(PushVector, true, Hit.ImpactPoint);

				//FVector const ArrowEnd{ Hit.ImpactPoint + PushVector };
				//float const ArrowSize{ 20.f };

				//DrawDebugDirectionalArrow(
				//	GetWorld(),
				//	Hit.ImpactPoint,
				//	ArrowEnd,
				//	ArrowSize,
				//	FColor::Red,
				//	false,
				//	2.f,
				//	0,
				//	4.f);
			}
		}

		if (OnWallHitNiagaraSystem)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				OnWallHitNiagaraSystem,
				Hit.ImpactPoint,
				Hit.ImpactNormal.Rotation()
			);
		}

		APlayerController* PlayerController{ Cast<APlayerController>(Car->GetController()) };
		if (PlayerController && ForceFeedbackEffect)
		{
			PlayerController->ClientPlayForceFeedback(ForceFeedbackEffect);
		}

		const float CurrentTime{ static_cast<float>(GetWorld()->GetTimeSeconds()) };
		if (WallBumpSound && (CurrentTime - LastBumpSoundTime > BumpSoundCooldown))
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), WallBumpSound, Hit.ImpactPoint);
			LastBumpSoundTime = CurrentTime;
		}
	}
}
