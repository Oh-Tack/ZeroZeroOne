// Fill out your copyright notice in the Description page of Project Settings.


#include "NeedOfSpeed/Public/AI/CPP_McLaren_Rear.h"

UCPP_McLaren_Rear::UCPP_McLaren_Rear()
{
	AxleType = EAxleType::Rear;
	
	WheelRadius = 40.0f;
	WheelWidth = 38.0f;
	WheelMass = 38.0f;
	
	FrictionForceMultiplier = 2.5f;  // 타이어 grip 증가
	MaxSteerAngle = 0.0f;
	
	bAffectedByBrake = true;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
	bAffectedBySteering = false;
	
	
}