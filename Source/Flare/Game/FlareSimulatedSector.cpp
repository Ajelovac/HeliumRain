
#include "../Flare.h"
#include "FlareSimulatedSector.h"
#include "FlareGame.h"
#include "FlareWorld.h"
#include "FlareFleet.h"
#include "../Spacecrafts/FlareSimulatedSpacecraft.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareSimulatedSector::UFlareSimulatedSector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PersistentStationIndex = 0;
}

void UFlareSimulatedSector::Load(const FFlareSectorDescription* Description, const FFlareSectorSave& Data, const FFlareSectorOrbitParameters& OrbitParameters)
{
	Game = Cast<UFlareWorld>(GetOuter())->GetGame();


	SectorData = Data;
	SectorDescription = Description;
	SectorOrbitParameters = OrbitParameters;
	SectorShips.Empty();
	SectorStations.Empty();
	SectorSpacecrafts.Empty();
	SectorFleets.Empty();

	for (int i = 0 ; i < SectorData.SpacecraftIdentifiers.Num(); i++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = Game->GetGameWorld()->FindSpacecraft(SectorData.SpacecraftIdentifiers[i]);
		if (Spacecraft->IsStation())
		{
			SectorStations.Add(Spacecraft);
		}
		else
		{
			SectorShips.Add(Spacecraft);
		}
		SectorSpacecrafts.Add(Spacecraft);
		Spacecraft->SetCurrentSector(this);
	}


	for (int i = 0 ; i < SectorData.FleetIdentifiers.Num(); i++)
	{
		UFlareFleet* Fleet = Game->GetGameWorld()->FindFleet(SectorData.FleetIdentifiers[i]);
		SectorFleets.Add(Fleet);
		Fleet->SetCurrentSector(this);
	}
}

FFlareSectorSave* UFlareSimulatedSector::Save()
{
	SectorData.SpacecraftIdentifiers.Empty();
	SectorData.FleetIdentifiers.Empty();

	for (int i = 0 ; i < SectorSpacecrafts.Num(); i++)
	{
		SectorData.SpacecraftIdentifiers.Add(SectorSpacecrafts[i]->GetImmatriculation());
	}

	for (int i = 0 ; i < SectorFleets.Num(); i++)
	{
		SectorData.FleetIdentifiers.Add(SectorFleets[i]->GetIdentifier());
	}

	return &SectorData;
}


UFlareSimulatedSpacecraft* UFlareSimulatedSector::CreateStation(FName StationClass, UFlareCompany* Company, FVector TargetPosition, FRotator TargetRotation, FName AttachPoint)
{
	FFlareSpacecraftDescription* Desc = Game->GetSpacecraftCatalog()->Get(StationClass);

	if (!Desc)
	{
		Desc = Game->GetSpacecraftCatalog()->Get(FName(*("station-" + StationClass.ToString())));
	}

	if (Desc)
	{
		return CreateShip(Desc, Company, TargetPosition, TargetRotation, AttachPoint);
	}
	return NULL;
}

UFlareSimulatedSpacecraft* UFlareSimulatedSector::CreateShip(FName ShipClass, UFlareCompany* Company, FVector TargetPosition)
{
	FFlareSpacecraftDescription* Desc = Game->GetSpacecraftCatalog()->Get(ShipClass);

	if (!Desc)
	{
		Desc = Game->GetSpacecraftCatalog()->Get(FName(*("ship-" + ShipClass.ToString())));
	}

	if (Desc)
	{
		return CreateShip(Desc, Company, TargetPosition);
	}
	else
	{
		FLOGV("CreateShip failed: Unkwnon ship %s", *ShipClass.ToString());
	}

	return NULL;
}

UFlareSimulatedSpacecraft* UFlareSimulatedSector::CreateShip(FFlareSpacecraftDescription* ShipDescription, UFlareCompany* Company, FVector TargetPosition, FRotator TargetRotation, FName AttachPoint)
{
	UFlareSimulatedSpacecraft* Spacecraft = NULL;

	// Default data
	FFlareSpacecraftSave ShipData;
	ShipData.Location = TargetPosition;
	ShipData.Rotation = TargetRotation;
	ShipData.LinearVelocity = FVector::ZeroVector;
	ShipData.AngularVelocity = FVector::ZeroVector;
	ShipData.SpawnMode = EFlareSpawnMode::Spawn;
	Game->Immatriculate(Company, ShipDescription->Identifier, &ShipData);
	ShipData.Identifier = ShipDescription->Identifier;
	ShipData.Heat = 600 * ShipDescription->HeatCapacity;
	ShipData.PowerOutageDelay = 0;
	ShipData.PowerOutageAcculumator = 0;
	ShipData.AttachPoint = AttachPoint;
	ShipData.IsAssigned = false;

	FName RCSIdentifier;
	FName OrbitalEngineIdentifier;

	// Size selector
	if (ShipDescription->Size == EFlarePartSize::S)
	{
		RCSIdentifier = FName("rcs-piranha");
		OrbitalEngineIdentifier = FName("engine-octopus");
	}
	else if (ShipDescription->Size == EFlarePartSize::L)
	{
		RCSIdentifier = FName("rcs-rift");
		OrbitalEngineIdentifier = FName("pod-surtsey");
	}
	else
	{
		// TODO
	}

	for (int32 i = 0; i < ShipDescription->RCSCount; i++)
	{
		FFlareSpacecraftComponentSave ComponentData;
		ComponentData.ComponentIdentifier = RCSIdentifier;
		ComponentData.ShipSlotIdentifier = FName(*("rcs-" + FString::FromInt(i)));
		ComponentData.Damage = 0.f;
		ShipData.Components.Add(ComponentData);
	}

	for (int32 i = 0; i < ShipDescription->OrbitalEngineCount; i++)
	{
		FFlareSpacecraftComponentSave ComponentData;
		ComponentData.ComponentIdentifier = OrbitalEngineIdentifier;
		ComponentData.ShipSlotIdentifier = FName(*("engine-" + FString::FromInt(i)));
		ComponentData.Damage = 0.f;
		ShipData.Components.Add(ComponentData);
	}

	for (int32 i = 0; i < ShipDescription->GunSlots.Num(); i++)
	{
		FFlareSpacecraftComponentSave ComponentData;
		ComponentData.ComponentIdentifier = Game->GetDefaultWeaponIdentifier();
		ComponentData.ShipSlotIdentifier = ShipDescription->GunSlots[i].SlotIdentifier;
		ComponentData.Damage = 0.f;
		ComponentData.Weapon.FiredAmmo = 0;
		ShipData.Components.Add(ComponentData);
	}

	for (int32 i = 0; i < ShipDescription->TurretSlots.Num(); i++)
	{
		FFlareSpacecraftComponentSave ComponentData;
		ComponentData.ComponentIdentifier = Game->GetDefaultTurretIdentifier();
		ComponentData.ShipSlotIdentifier = ShipDescription->TurretSlots[i].SlotIdentifier;
		ComponentData.Turret.BarrelsAngle = 0;
		ComponentData.Turret.TurretAngle = 0;
		ComponentData.Weapon.FiredAmmo = 0;
		ComponentData.Damage = 0.f;
		ShipData.Components.Add(ComponentData);
	}

	for (int32 i = 0; i < ShipDescription->InternalComponentSlots.Num(); i++)
	{
		FFlareSpacecraftComponentSave ComponentData;
		ComponentData.ComponentIdentifier = ShipDescription->InternalComponentSlots[i].ComponentIdentifier;
		ComponentData.ShipSlotIdentifier = ShipDescription->InternalComponentSlots[i].SlotIdentifier;
		ComponentData.Damage = 0.f;
		ShipData.Components.Add(ComponentData);
	}

	// Init pilot
	ShipData.Pilot.Identifier = "chewie";
	ShipData.Pilot.Name = "Chewbaca";

	// Init company
	ShipData.CompanyIdentifier = Company->GetIdentifier();

	// Create the ship
	Spacecraft = Company->LoadSpacecraft(ShipData);
	if (Spacecraft->IsStation())
	{
		SectorStations.Add(Spacecraft);
	}
	else
	{
		SectorShips.Add(Spacecraft);
	}
	SectorSpacecrafts.Add(Spacecraft);

	Spacecraft->SetCurrentSector(this);

	FLOGV("UFlareSimulatedSector::CreateShip : Created ship '%s' at %s", *Spacecraft->GetImmatriculation().ToString(), *TargetPosition.ToString());

	// TODO remove automatic fleet creation
	if (!Spacecraft->IsStation())
	{
		UFlareFleet* NewFleet = Company->CreateFleet("Automatic fleet", Spacecraft->GetCurrentSector());
		NewFleet->AddShip(Spacecraft);
		// If the ship is in the player company, select the new fleet
		if (Game->GetPC()->GetCompany() == Company)
		{
			Game->GetPC()->SelectFleet(NewFleet);
		}
	}

	return Spacecraft;
}

void UFlareSimulatedSector::CreateAsteroid(int32 ID, FName Name, FVector Location)
{
	if (ID >= Game->GetAsteroidCatalog()->Asteroids.Num())
	{
		FLOGV("Astroid create fail : Asteroid max ID is %d", Game->GetAsteroidCatalog()->Asteroids.Num() -1);
		return;
	}

	FFlareAsteroidSave Data;
	Data.AsteroidMeshID = ID;
	Data.Identifier = Name;
	Data.LinearVelocity = FVector::ZeroVector;
	Data.AngularVelocity = FMath::VRand() * FMath::FRandRange(-1.f,1.f);
	Data.Scale = FVector(1,1,1) * FMath::FRandRange(0.9,1.1);
	Data.Rotation = FRotator(FMath::FRandRange(0,360), FMath::FRandRange(0,360), FMath::FRandRange(0,360));
	Data.Location = Location;

	SectorData.AsteroidData.Add(Data);
}

void UFlareSimulatedSector::AddFleet(UFlareFleet* Fleet)
{
	SectorFleets.Add(Fleet);

	for (int ShipIndex = 0; ShipIndex < Fleet->GetShips().Num(); ShipIndex++)
	{
		Fleet->GetShips()[ShipIndex]->SetCurrentSector(this);
		SectorShips.AddUnique(Fleet->GetShips()[ShipIndex]);
		SectorSpacecrafts.AddUnique(Fleet->GetShips()[ShipIndex]);
	}
}

void UFlareSimulatedSector::DisbandFleet(UFlareFleet* Fleet)
{
	if (SectorFleets.Remove(Fleet) == 0)
	{
		FLOGV("ERROR: RetireFleet fail. Fleet '%s' is not in sector '%s'", *Fleet->GetFleetName(), *GetSectorName().ToString())
		return;
	}
}

void UFlareSimulatedSector::RetireFleet(UFlareFleet* Fleet)
{
	FLOGV("UFlareSimulatedSector::RetireFleet %s", *Fleet->GetFleetName());
	if (SectorFleets.Remove(Fleet) == 0)
	{
		FLOGV("ERROR: RetireFleet fail. Fleet '%s' is not in sector '%s'", *Fleet->GetFleetName(), *GetSectorName().ToString())
		return;
	}

	for (int ShipIndex = 0; ShipIndex < Fleet->GetShips().Num(); ShipIndex++)
	{
		Fleet->GetShips()[ShipIndex]->SetCurrentSector(NULL);
		if (RemoveSpacecraft(Fleet->GetShips()[ShipIndex]) == 0)
		{
			FLOGV("ERROR: RetireFleet fail. Ship '%s' is not in sector '%s'", *Fleet->GetShips()[ShipIndex]->GetImmatriculation().ToString(), *GetSectorName().ToString())
		}
	}
}

int UFlareSimulatedSector::RemoveSpacecraft(UFlareSimulatedSpacecraft* Spacecraft)
{
	SectorSpacecrafts.Remove(Spacecraft);
	return SectorShips.Remove(Spacecraft);
}

void UFlareSimulatedSector::SetShipToFly(UFlareSimulatedSpacecraft* Ship)
{
	SectorData.LastFlownShip = Ship->GetImmatriculation();
}


/*----------------------------------------------------
	Getters
----------------------------------------------------*/

FText UFlareSimulatedSector::GetSectorName() const
{
	if (SectorData.GivenName.ToString().Len())
	{
		return SectorData.GivenName;
	}
	else if (SectorDescription->Name.ToString().Len())
	{
		return SectorDescription->Name;
	}
	else
	{
		return FText::FromString(GetSectorCode());
	}
}

FText UFlareSimulatedSector::GetSectorDescription() const
{
	return SectorDescription->Description;
}

FString UFlareSimulatedSector::GetSectorCode() const
{
	// TODO cache
	return SectorOrbitParameters.CelestialBodyIdentifier.ToString() + "-" + FString::FromInt(SectorOrbitParameters.Altitude) + "-" + FString::FromInt(SectorOrbitParameters.Phase);
}

EFlareSectorFriendlyness::Type UFlareSimulatedSector::GetSectorFriendlyness(UFlareCompany* Company) const
{
	if (!Company->HasVisitedSector(this))
	{
		return EFlareSectorFriendlyness::NotVisited;
	}

	if (SectorSpacecrafts.Num() == 0)
	{
		return EFlareSectorFriendlyness::Neutral;
	}

	int HostileSpacecraftCount = 0;
	int NeutralSpacecraftCount = 0;
	int FriendlySpacecraftCount = 0;

	for (int SpacecraftIndex = 0 ; SpacecraftIndex < SectorSpacecrafts.Num(); SpacecraftIndex++)
	{
		UFlareCompany* OtherCompany = SectorSpacecrafts[SpacecraftIndex]->GetCompany();

		if (OtherCompany == Company)
		{
			FriendlySpacecraftCount++;
		}
		else if (OtherCompany->GetHostility(Company) == EFlareHostility::Hostile)
		{
			HostileSpacecraftCount++;
		}
		else
		{
			NeutralSpacecraftCount++;
		}
	}

	if (FriendlySpacecraftCount > 0 && HostileSpacecraftCount > 0)
	{
		return EFlareSectorFriendlyness::Contested;
	}

	if (FriendlySpacecraftCount > 0)
	{
		return EFlareSectorFriendlyness::Friendly;
	}
	else if (HostileSpacecraftCount > 0)
	{
		return EFlareSectorFriendlyness::Hostile;
	}
	else
	{
		return EFlareSectorFriendlyness::Neutral;
	}
}

bool UFlareSimulatedSector::CanBuildStation(FFlareSpacecraftDescription* StationDescription, UFlareCompany* Company)
{
	// Check money cost
	if(Company->GetMoney() < StationDescription->Cost)
	{
		return false;
	}

	// First, it need a free cargo
	bool HasFreeCargo = false;
	for(int ShipIndex = 0; ShipIndex < SectorShips.Num(); ShipIndex++)
	{
		UFlareSimulatedSpacecraft* Ship = SectorShips[ShipIndex];

		if(Ship->GetCompany() != Company)
		{
			continue;
		}

		if(Ship->GetDescription()->CargoBayCount == 0)
		{
			// Not a cargo
			continue;
		}

		if(Ship->IsAssignedToSector())
		{
			// Not available to build
			continue;
		}

		HasFreeCargo = true;
		break;
	}

	if(!HasFreeCargo)
	{
		return false;
	}

	// Compute total available resources
	TArray<FFlareCargo> AvailableResources;
	for(int SpacecraftIndex = 0; SpacecraftIndex < SectorSpacecrafts.Num(); SpacecraftIndex++)
	{
		UFlareSimulatedSpacecraft* Spacecraft = SectorSpacecrafts[SpacecraftIndex];

		if(Spacecraft->GetCompany() != Company)
		{
			continue;
		}

		TArray<FFlareCargo>& CargoBay = Spacecraft->GetCargoBay();
		for(int CargoIndex = 0; CargoIndex < CargoBay.Num(); CargoIndex++)
		{
			FFlareCargo* Cargo = &(CargoBay[CargoIndex]);

			if(!Cargo->Resource)
			{
				continue;
			}

			bool NewResource = true;
			for(int AvailableResourceIndex = 0; AvailableResourceIndex < AvailableResources.Num(); AvailableResourceIndex++)
			{
				if(AvailableResources[AvailableResourceIndex].Resource == Cargo->Resource)
				{
					AvailableResources[AvailableResourceIndex].Quantity += Cargo->Quantity;
					NewResource = false;
					break;
				}
			}

			if(NewResource)
			{
				FFlareCargo NewResourceCargo;
				NewResourceCargo.Resource = Cargo->Resource;
				NewResourceCargo.Quantity = Cargo->Quantity;
				AvailableResources.Add(NewResourceCargo);
			}
		}
	}

	// Check resource cost
	for(int ResourceIndex = 0; ResourceIndex < StationDescription->ResourcesCost.Num(); ResourceIndex++)
	{
		FFlareFactoryResource* FactoryResource = &StationDescription->ResourcesCost[ResourceIndex];
		bool ResourceFound = false;

		for(int AvailableResourceIndex = 0; AvailableResourceIndex < AvailableResources.Num(); AvailableResourceIndex++)
		{
			if(AvailableResources[AvailableResourceIndex].Resource == &(FactoryResource->Resource->Data))
			{
				ResourceFound = true;
				if(AvailableResources[AvailableResourceIndex].Quantity < FactoryResource->Quantity)
				{
					return false;
				}
				break;
			}
		}

		if(!ResourceFound)
		{
			return false;
		}
	}

	return true;
}

bool UFlareSimulatedSector::BuildStation(FFlareSpacecraftDescription* StationDescription, UFlareCompany* Company)
{
	if(!CanBuildStation(StationDescription, Company))
	{
		FLOGV("Fail to buid station '%s' for company '%s'", *StationDescription->Identifier.ToString(), *Company->GetIdentifier().ToString());
		return false;
	}

	// Pay station cost
	Company->TakeMoney(StationDescription->Cost);

	// Take resource cost
	for(int ResourceIndex = 0; ResourceIndex < StationDescription->ResourcesCost.Num(); ResourceIndex++)
	{
		FFlareFactoryResource* FactoryResource = &StationDescription->ResourcesCost[ResourceIndex];
		uint32 ResourceToTake = FactoryResource->Quantity;
		FFlareResourceDescription* Resource = &(FactoryResource->Resource->Data);


		// First take from ships
		for(int ShipIndex = 0; ShipIndex < SectorShips.Num() && ResourceToTake > 0; ShipIndex++)
		{
			UFlareSimulatedSpacecraft* Ship = SectorShips[ShipIndex];

			if(Ship->GetCompany() != Company)
			{
				continue;
			}

			ResourceToTake -= Ship->TakeResources(Resource, ResourceToTake);
		}

		if(ResourceToTake == 0)
		{
			continue;
		}

		// Then take useless resources from station
		ResourceToTake -= TakeUselessRessouce(Company, Resource, ResourceToTake);

		if(ResourceToTake == 0)
		{
			continue;
		}

		// Finally take from all stations
		for(int StationIndex = 0; StationIndex < SectorStations.Num() && ResourceToTake > 0; StationIndex++)
		{
			UFlareSimulatedSpacecraft* Station = SectorShips[StationIndex];

			if(Station->GetCompany() != Company)
			{
				continue;
			}

			ResourceToTake -= Station->TakeResources(Resource, ResourceToTake);
		}

		if(ResourceToTake > 0)
		{
			FLOG("WARNING ! Fail to take resource cost for build station a station but CanBuild test succeded");
		}
	}

	CreateStation(StationDescription->Identifier, Company, FVector::ZeroVector);

	return true;
}

void UFlareSimulatedSector::SimulateTransport(int64 Duration)
{
	// TODO transport capacity
	uint32 TransportCapacityPerHour = 10;

	FLOGV("SimulateTransport Duration=%lld TransportCapacityPerHour=%u", Duration, TransportCapacityPerHour)

	if(TransportCapacityPerHour == 0)
	{
		// No transport
		return;
	}

	uint32 TransportCapacity = Duration * TransportCapacityPerHour / 3600.;

	// TODO Store ouput resource from station in overflow to storage

	FLOGV("Initial TransportCapacity=%u", TransportCapacity);

	if(PersistentStationIndex >= SectorStations.Num())
	{
		PersistentStationIndex = 0;
	}

	FLOGV("PersistentStationIndex=%d", PersistentStationIndex);

	// TODO 4 pass:
	// a first one with the exact quantity
	// the second with the double
	// a third with slot alignemnt
	// a 4th with inactive stations


	for (int32 CountIndex = 0 ; CountIndex < SectorStations.Num(); CountIndex++)
	{
		UFlareSimulatedSpacecraft* Station = SectorStations[PersistentStationIndex];
		PersistentStationIndex++;
		if(PersistentStationIndex >= SectorStations.Num())
		{
			PersistentStationIndex = 0;
		}

		FLOGV("Check station %s needs:", *Station->GetImmatriculation().ToString());


		for(int32 FactoryIndex = 0; FactoryIndex < Station->GetFactories().Num(); FactoryIndex++)
		{
			UFlareFactory* Factory = Station->GetFactories()[FactoryIndex];

			FLOGV("  Factory %s : IsActive=%d IsNeedProduction=%d", *Factory->GetDescription()->Name.ToString(), Factory->IsActive(),Factory->IsNeedProduction());

			if(!Factory->IsActive() || !Factory->IsNeedProduction())
			{
				FLOG("    No resources needed");
				// No resources needed
				break;
			}

			for(int32 ResourceIndex = 0; ResourceIndex < Factory->GetInputResourcesCount(); ResourceIndex++)
			{
				FFlareResourceDescription* Resource = Factory->GetInputResource(ResourceIndex);
				uint32 StoredQuantity = Station->GetCargoBayResourceQuantity(Resource);
				uint32 NeededQuantity = Factory->GetInputResourceQuantity(ResourceIndex);
				uint32 StorageCapacity = Station->GetCargoBayFreeSpace(Resource);
				FLOGV("    Resource %s : StoredQuantity=%u NeededQuantity=%u StorageCapacity=%u", *Resource->Name.ToString(), StoredQuantity, NeededQuantity, StorageCapacity);

				if(StoredQuantity < NeededQuantity * 2)
				{
					// Do transfert
					uint32 QuantityToTransfert = FMath::Min(TransportCapacity, NeededQuantity * 2 - StoredQuantity);
					// TODO check the slot size : for helium quantity to transfert will be 2 but must be 100 (1 slot)
					QuantityToTransfert = FMath::Min(StorageCapacity, QuantityToTransfert);
					uint32 TakenResources = TakeUselessRessouce(Station->GetCompany(), Resource, QuantityToTransfert);
					Station->GiveResources(Resource, TakenResources);
					TransportCapacity -= TakenResources;
					FLOGV("      Do transfet : QuantityToTransfert=%u TakenResources=%u TransportCapacity=%u", QuantityToTransfert, TakenResources, TransportCapacity);

					if(TransportCapacity == 0)
					{
						break;
					}
				}
			}

			if(TransportCapacity == 0)
			{
				break;
			}
		}

		if(TransportCapacity == 0)
		{
			break;
		}
	}

	FLOGV("SimulateTransport end TransportCapacity=%u", TransportCapacity);
}

uint32 UFlareSimulatedSector::TakeUselessRessouce(UFlareCompany* Company, FFlareResourceDescription* Resource, uint32 QuantityToTake)
{
	uint32 RemainingQuantityToTake = QuantityToTake;
	// First pass: take from station with factory that output the resource
	for (int32 StationIndex = 0 ; StationIndex < SectorStations.Num() && RemainingQuantityToTake > 0; StationIndex++)
	{
		UFlareSimulatedSpacecraft* Station = SectorStations[StationIndex];

		if( Station->GetCompany() != Company)
		{
			continue;
		}

		for(int32 FactoryIndex = 0; FactoryIndex < Station->GetFactories().Num(); FactoryIndex++)
		{
			UFlareFactory* Factory = Station->GetFactories()[FactoryIndex];
			if(Factory->HasOutputResource(Resource))
			{
				uint32 TakenQuatity = Station->TakeResources(Resource, RemainingQuantityToTake);
				RemainingQuantityToTake -= TakenQuatity;
				break;
			}
		}
	}

	// Second pass: take from storage station
	// TODO

	// Third pass: take from station with factory that don't input the resources
	for (int32 StationIndex = 0 ; StationIndex < SectorStations.Num() && RemainingQuantityToTake > 0; StationIndex++)
	{
		UFlareSimulatedSpacecraft* Station = SectorStations[StationIndex];
		bool NeedResource = false;

		if( Station->GetCompany() != Company)
		{
			continue;
		}

		for(int32 FactoryIndex = 0; FactoryIndex < Station->GetFactories().Num(); FactoryIndex++)
		{
			UFlareFactory* Factory = Station->GetFactories()[FactoryIndex];
			if(Factory->HasInputResource(Resource))
			{
				NeedResource =true;
				break;
			}
		}

		if(!NeedResource)
		{
			uint32 TakenQuatity = Station->TakeResources(Resource, RemainingQuantityToTake);
			RemainingQuantityToTake -= TakenQuatity;
		}
	}

	// 4th pass: take from station with factory that don't input the resources
	for (int32 StationIndex = 0 ; StationIndex < SectorStations.Num() && RemainingQuantityToTake > 0; StationIndex++)
	{
		UFlareSimulatedSpacecraft* Station = SectorStations[StationIndex];
		bool NeedResource = false;

		if( Station->GetCompany() != Company)
		{
			continue;
		}

		for(int32 FactoryIndex = 0; FactoryIndex < Station->GetFactories().Num(); FactoryIndex++)
		{
			UFlareFactory* Factory = Station->GetFactories()[FactoryIndex];
			if(Factory->IsActive() && Factory->IsNeedProduction() && Factory->HasInputResource(Resource))
			{
				NeedResource =true;
				break;
			}
		}

		if(!NeedResource)
		{
			uint32 TakenQuatity = Station->TakeResources(Resource, RemainingQuantityToTake);
			RemainingQuantityToTake -= TakenQuatity;
		}
	}

	return QuantityToTake - RemainingQuantityToTake;
}
