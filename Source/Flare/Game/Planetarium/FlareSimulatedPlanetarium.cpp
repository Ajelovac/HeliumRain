#include "../../Flare.h"
#include "FlareSimulatedPlanetarium.h"
#include "../FlareGame.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSimulatedPlanetarium::UFlareSimulatedPlanetarium(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void UFlareSimulatedPlanetarium::Load()
{
	Game = Cast<UFlareWorld>(GetOuter())->GetGame();
	Sun.Sattelites.Empty();


	// Init the sun
	Sun.Name = "Sun";
	Sun.Identifier = "star-sun";
	Sun.Mass = 2.472e30;
	Sun.Radius = 739886*2; // TODO fix in spec
	Sun.RotationVelocity = 0;
	Sun.OrbitDistance = 0;
	Sun.RelativeLocation = FVector::ZeroVector;
	Sun.AbsoluteLocation = FVector::ZeroVector;
	Sun.RotationAngle = 0;

	// Small Hot planet
	FFlareCelestialBody SmallHotPlanet;
	SmallHotPlanet.Name = "Small Hot Planet";
	SmallHotPlanet.Identifier = "planet-shp";
	SmallHotPlanet.Mass = 330e21;
	SmallHotPlanet.Radius = 2439;
	SmallHotPlanet.RotationVelocity = 0.0003;
	SmallHotPlanet.OrbitDistance = 57000000;
	Sun.Sattelites.Add(SmallHotPlanet);

	// Nema

	FFlareCelestialBody Nema;
	{
		Nema.Name = "Nema";
		Nema.Identifier = "planet-nema";
		Nema.Mass = 8.421e26;
		Nema.Radius = 69586;
		Nema.RotationVelocity = 0.0037254354102635744;
		Nema.OrbitDistance = 110491584;

		FFlareCelestialBody NemaMoon1;
		NemaMoon1.Name = "Nema Moon 1";
		NemaMoon1.Identifier = "moon-nema1";
		NemaMoon1.Mass = 1.3e23;
		NemaMoon1.Radius = 2600;
		NemaMoon1.RotationVelocity = -0.03;
		NemaMoon1.OrbitDistance = 320000;
		Nema.Sattelites.Add(NemaMoon1);

		FFlareCelestialBody NemaMoon2;
		NemaMoon2.Name = "Nema Moon 2";
		NemaMoon2.Identifier = "moon-nema2";
		NemaMoon2.Mass = 5.3e23;
		NemaMoon2.Radius = 4600;
		NemaMoon2.RotationVelocity = 0.003;
		NemaMoon2.OrbitDistance = 571000;
		Nema.Sattelites.Add(NemaMoon2);

		FFlareCelestialBody NemaMoon3;
		NemaMoon3.Name = "Nema Moon 3";
		NemaMoon3.Identifier = "moon-nema3";
		NemaMoon3.Mass = 0.9e23;
		NemaMoon3.Radius = 2000;
		NemaMoon3.RotationVelocity = 0.05;
		NemaMoon3.OrbitDistance = 870000;
		Nema.Sattelites.Add(NemaMoon3);
	}
	Sun.Sattelites.Add(Nema);
}


FFlareCelestialBody* UFlareSimulatedPlanetarium::FindCelestialBody(FString BodyIdentifier)
{
	return FindCelestialBody(&Sun, BodyIdentifier);
}

FFlareCelestialBody* UFlareSimulatedPlanetarium::FindCelestialBody(FFlareCelestialBody* Body, FString BodyIdentifier)
{
	if(Body->Identifier == BodyIdentifier)
	{
		return Body;
	}

	for(int SatteliteIndex = 0; SatteliteIndex < Body->Sattelites.Num(); SatteliteIndex++)
	{
		FFlareCelestialBody* CelestialBody = &Body->Sattelites[SatteliteIndex];
		FFlareCelestialBody* Result = FindCelestialBody(CelestialBody, BodyIdentifier);
		if(Result)
		{
			return Result;
		}
	}

	return NULL;
}

FFlareCelestialBody UFlareSimulatedPlanetarium::GetSnapShot(int64 Time)
{
	ComputeCelestialBodyLocation(NULL, &Sun, Time);
	return Sun;
}

FVector UFlareSimulatedPlanetarium::GetRelativeLocation(FFlareCelestialBody* ParentBody, int64 Time, float OrbitDistance, float Mass, float InitialPhase)
{
	// TODO extract the constant
	float G = 6.674e-11; // Gravitational constant

	float MassSum = ParentBody->Mass + Mass;
	float OrbitalVelocity = FMath::Sqrt(G * ((MassSum) / (1000 * OrbitDistance)));

	float OrbitalCircumference = 2 * PI * 1000 * OrbitDistance;
	int64 RevolutionTime = (int64) (OrbitalCircumference / OrbitalVelocity);

	int64 CurrentRevolutionTime = Time % RevolutionTime;

	float Phase = (360 * (float) CurrentRevolutionTime / (float) RevolutionTime) + InitialPhase;


	FVector RelativeLocation = OrbitDistance * FVector(FMath::Cos(FMath::DegreesToRadians(Phase)),
			0,
			FMath::Sin(FMath::DegreesToRadians(Phase)));


	return RelativeLocation;
}


void UFlareSimulatedPlanetarium::ComputeCelestialBodyLocation(FFlareCelestialBody* ParentBody, FFlareCelestialBody* Body, int64 Time)
{
	if(ParentBody)
	{
	/*	// TODO extract the constant
		float G = 6.674e-11; // Gravitational constant

		float MassSum = ParentBody->Mass + Body->Mass;
		float Distance = 1000 * Body->OrbitDistance;
		float SquareVelocity = G * (MassSum / Distance);
		float FragmentedOrbitalVelocity = FMath::Sqrt(SquareVelocity);

		float OrbitalVelocity = FMath::Sqrt(G * ((ParentBody->Mass + Body->Mass) / (1000 * Body->OrbitDistance)));

		OrbitalVelocity = FragmentedOrbitalVelocity;

		float OrbitalCircumference = 2 * PI * 1000 * Body->OrbitDistance;
		int64 RevolutionTime = (int64) (OrbitalCircumference / OrbitalVelocity);

		int64 CurrentRevolutionTime = Time % RevolutionTime;

		float Phase = 360 * (float) CurrentRevolutionTime / (float) RevolutionTime;
*/
		Body->RelativeLocation = GetRelativeLocation(ParentBody, Time, Body->OrbitDistance, Body->Mass, 0);
		Body->AbsoluteLocation = ParentBody->AbsoluteLocation + Body->RelativeLocation;
		/*Body->RelativeLocation = Body->OrbitDistance * FVector(FMath::Cos(FMath::DegreesToRadians(Phase)),
				FMath::Sin(FMath::DegreesToRadians(Phase)),
				0);*/
	}

	Body->RotationAngle = Body->RotationVelocity * Time;

	for(int SatteliteIndex = 0; SatteliteIndex < Body->Sattelites.Num(); SatteliteIndex++)
	{
		FFlareCelestialBody* CelestialBody = &Body->Sattelites[SatteliteIndex];
		ComputeCelestialBodyLocation(Body, CelestialBody, Time);
	}
}
