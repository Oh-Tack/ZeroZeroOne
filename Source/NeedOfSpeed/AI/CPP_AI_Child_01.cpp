// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_AI_Child_01.h"

#include "ChaosWheeledVehicleMovementComponent.h"

ACPP_AI_Child_01::ACPP_AI_Child_01() 
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp)
	{
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> NewMesh(TEXT("/Game/Resources/McLaren/McLaren_Rig.McLaren_Rig"));
		if (NewMesh.Succeeded())
		{
			MeshComp->SetSkeletalMesh(NewMesh.Object);
		}
	}
}

