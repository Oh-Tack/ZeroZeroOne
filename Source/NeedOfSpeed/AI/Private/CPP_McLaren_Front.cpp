// Fill out your copyright notice in the Description page of Project Settings.


#include "NeedOfSpeed/AI/Public/CPP_McLaren_Front.h"

UCPP_McLaren_Front::UCPP_McLaren_Front()
{
	AxleType = EAxleType::Front;
	
	WheelRadius = 37.5f;
	WheelWidth = 30.0f;
	WheelMass = 30.0f;
	
	
	MaxSteerAngle = 50.0f;
	
	bAffectedByBrake = true;
	bAffectedByHandbrake = false;
	bAffectedByEngine = false;
	bAffectedBySteering = true;
}