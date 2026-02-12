// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_McLaren_Rear.h"

UCPP_McLaren_Rear::UCPP_McLaren_Rear()
{
	AxleType = EAxleType::Rear;
	
	WheelRadius = 40.0f;
	WheelWidth = 38.0f;
	WheelMass = 38.0f;
	
	bAffectedByBrake = true;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
	bAffectedBySteering = false;
}