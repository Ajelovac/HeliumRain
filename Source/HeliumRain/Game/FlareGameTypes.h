#pragma once

#include "../Flare.h"
#include "FlareFleet.h"
#include "FlareTradeRoute.h"
#include "../Spacecrafts/FlareSpacecraftTypes.h"
#include "FlareGameTypes.generated.h"

#define AI_MAX_STATION_PER_SECTOR 30

class AFlareSpacecraft;

/*----------------------------------------------------
	General gameplay enums
----------------------------------------------------*/

/** Hostility status */
UENUM()
namespace EFlareHostility
{
	enum Type
	{
		Hostile,
		Neutral,
		Friendly,
		Owned
	};
}

/** Hostility status */
UENUM()
namespace EFlareBudget
{
	enum Type
	{
		Military,
		Station,
		Technology,
		Trade,
		None
	};
}

/** Combat groups */
UENUM()
namespace EFlareCombatGroup
{
	enum Type
	{
		AllMilitary,
		Capitals,
		Fighters,
		Civilan
	};
}

/** Resource price context */
UENUM()
namespace EFlareResourcePriceContext
{
	enum Type
	{
		Default, /** Default price */
		FactoryInput, /** Price selling to a factory needing the resource */
		FactoryOutput, /** Price buying the resource to a factory */
		HubInput, /** Price selling to a hub needing the resource */
		HubOutput, /** Price buying the resource to a hub */
		ConsumerConsumption, /** Price selling to a the people */
		MaintenanceConsumption, /** Price selling to company using maintenance */
	};
}


struct FFlareResourceUsage
{
public:
	bool HasAnyUsage() const;

	bool HasUsage(EFlareResourcePriceContext::Type Usage) const;

	void AddUsage(EFlareResourcePriceContext::Type Usage);
private:
	TArray<EFlareResourcePriceContext::Type> Usages;
};

/** Combat tactics */
UENUM()
namespace EFlareCombatTactic
{
	enum Type
	{
		AttackAll,
		ProtectMe,
		AttackMilitary,
		AttackStations,
		AttackCivilians,
		StandDown
	};
}

/** Sector knowledge status */
UENUM()
namespace EFlareSectorKnowledge
{
	enum Type
	{
		Unknown, /** The existence of this sector is unknown */
		Known, /** The sector is visible on the map but its content is unknown */
		Visited /** The sector is visited, all static structure are visible */
	};
}

/** Technology type for user display */
UENUM()
namespace EFlareTechnologyCategory
{
	enum Type
	{
		General, /** All-purpose tech */
		Economy, /** Economy tech */
		Military /** Military tech */
	};
}


/*----------------------------------------------------
	General gameplay data
----------------------------------------------------*/

/** Company sector knowledge data */
USTRUCT()
struct FFlareCompanySectorKnowledge
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Save)
	FName SectorIdentifier;

	UPROPERTY(EditAnywhere, Category = Save)
	TEnumAsByte<EFlareSectorKnowledge::Type> Knowledge;
};

/** Technology description */
USTRUCT()
struct FFlareTechnologyDescription
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Content)
	FName Identifier;

	UPROPERTY(EditAnywhere, Category = Content)
	int32 ResearchCost;

	UPROPERTY(EditAnywhere, Category = Content)
	int32 Level;

	UPROPERTY(EditAnywhere, Category = Content)
	FText Name;

	UPROPERTY(EditAnywhere, Category = Content)
	FText Description;

	UPROPERTY(EditAnywhere, Category = Content)
	TEnumAsByte<EFlareTechnologyCategory::Type> Category;
};

/** Scannable description */
USTRUCT()
struct FFlareScannableDescription
{
	GENERATED_USTRUCT_BODY()
		
	UPROPERTY(EditAnywhere, Category = Content)
	FName Identifier;

	UPROPERTY(EditAnywhere, Category = Content)
	FText Name;

	UPROPERTY(EditAnywhere, Category = Content)
	FText Description;
};

/** Company reputation save data */
USTRUCT()
struct FFlareCompanyReputationSave
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Save)
	FName CompanyIdentifier;

	UPROPERTY(EditAnywhere, Category = Save)
	float Reputation;

};

/** Company AI save data */
USTRUCT()
struct FFlareCompanyAISave
{
	GENERATED_USTRUCT_BODY()

	int64 BudgetTechnology;
	int64 BudgetMilitary;
	int64 BudgetStation;
	int64 BudgetTrade;

	/* Modify AttackThreshold */
	float Caution;
	float Pacifism;

	FName ResearchProject;
};

/** Transaction type*/
UENUM()
namespace EFlareTransactionLogEntry
{
	enum Type
	{
		ManualResourcePurchase = 0,
		ManualResourceSell,
		TradeRouteResourcePurchase,
		TradeRouteResourceSell,
		FactoryWages,
		CancelFactoryWages,
		StationConstructionFees,
		StationUpgradeFees,
		UpgradeShipPart,
		OrderShip,
		CancelOrderShip,
		OrderShipAdvance,
		PeoplePurchase,
		InitialMoney,
		PayRepair,
		PayRefill,
		PaidForRepair, // AI only
		PaidForRefill, // AI only
		SendTribute,
		ReceiveTribute, // AI only
		RecoveryFees,
		ScrapGain,
		Cheat, // cheat !
		QuestReward,
		MutualAssistance, // AI only
		ScrapCost, // AI only

		TYPE_COUNT
	};
}

class UFlareFactory;

USTRUCT()
struct FFlareTransactionLogEntry
{
	GENERATED_USTRUCT_BODY()

	static FFlareTransactionLogEntry LogUpgradeShipPart(UFlareSimulatedSpacecraft* Spacecraft);
	static FFlareTransactionLogEntry LogOrderShip(UFlareSimulatedSpacecraft* Shipyard, FName OrderShipClass);
	static FFlareTransactionLogEntry LogCancelOrderShip(UFlareSimulatedSpacecraft* Shipyard, FName OrderShipClass);
	static FFlareTransactionLogEntry LogShipOrderAdvance(UFlareSimulatedSpacecraft* Shipyard, FName Company, FName ShipClass);
	static FFlareTransactionLogEntry LogFactoryWages(UFlareFactory* Factory);
	static FFlareTransactionLogEntry LogCancelFactoryWages(UFlareFactory* Factory);
	static FFlareTransactionLogEntry LogPeoplePurchase(UFlareSimulatedSpacecraft* Station, FFlareResourceDescription* Resource, int32 Quantity);
	static FFlareTransactionLogEntry LogInitialMoney();
	static FFlareTransactionLogEntry LogBuyResource(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource, int32 GivenResources, UFlareTradeRoute* TradeRoute);
	static FFlareTransactionLogEntry LogSellResource(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource, int32 GivenResources, UFlareTradeRoute* TradeRoute);
	static FFlareTransactionLogEntry LogPayMaintenance(UFlareSimulatedSpacecraft* SellerSpacecraft, int32 TakenQuantity, bool ForRepair);
	static FFlareTransactionLogEntry LogPaidForMaintenance(UFlareSimulatedSpacecraft* SellerSpacecraft, UFlareCompany* Company, int32 TakenQuantity, bool ForRepair);
	static FFlareTransactionLogEntry LogSendTribute(UFlareCompany* Company);
	static FFlareTransactionLogEntry LogReceiveTribute(UFlareCompany* Company);
	static FFlareTransactionLogEntry LogRecoveryFees();
	static FFlareTransactionLogEntry LogBuildStationFees(FFlareSpacecraftDescription* StationDescription, UFlareSimulatedSector* Sector);
	static FFlareTransactionLogEntry LogUpgradeStationFees(UFlareSimulatedSpacecraft* Station);
	static FFlareTransactionLogEntry LogScrapCost(UFlareCompany* Company);
	static FFlareTransactionLogEntry LogScrapGain(UFlareSimulatedSpacecraft* ScrappedShip, UFlareCompany* Company);
	static FFlareTransactionLogEntry LogMutualAssistance();
	static FFlareTransactionLogEntry LogCheat();
	static FFlareTransactionLogEntry LogQuestReward(UFlareCompany* Company);

	int64 Date;
	int64 Amount;
	EFlareTransactionLogEntry::Type Type;
	FName Spacecraft;
	FName Sector;
	FName OtherCompany;
	FName OtherSpacecraft;
	FName Resource;
	int32 ResourceQuantity;
	FName ExtraIdentifier1;
	FName ExtraIdentifier2;

	static FText GetCategoryDescription(EFlareTransactionLogEntry::Type Type);

	UFlareCompany* GetOtherCompany(AFlareGame* Game) const;
	UFlareSimulatedSector* GetSector(AFlareGame* Game) const;
	UFlareSimulatedSpacecraft* GetSpacecraft(AFlareGame* Game) const;
	FText GetComment(AFlareGame* Game) const;
};

/** Game save data */
USTRUCT()
struct FFlareCompanySave
{
	GENERATED_USTRUCT_BODY()

	/** Save identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;

	/** Index of the company description in the catalog, or -1 if player */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CatalogIdentifier;

	/** Money money money / Always funny / In a rich men's world */
	UPROPERTY(EditAnywhere, Category = Save)
	int64 Money;

	/** Hostile companies */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FName> HostileCompanies;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareSpacecraftSave> ShipData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareSpacecraftSave> ChildStationData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareSpacecraftSave> StationData;

	UPROPERTY(VisibleAnywhere, Category = Save)
	TArray<FFlareSpacecraftSave> DestroyedSpacecraftData;

	/** Company fleets */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareFleetSave> Fleets;

	/** Company trade routes */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareTradeRouteSave> TradeRoutes;

	UPROPERTY(EditAnywhere, Category = Save)
	int32 FleetImmatriculationIndex;

	UPROPERTY(EditAnywhere, Category = Save)
	int32 TradeRouteImmatriculationIndex;

	/** List of known or visited sectors */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareCompanySectorKnowledge> SectorsKnowledge;

	/** Company AI data */
	UPROPERTY(EditAnywhere, Category = Save)
	FFlareCompanyAISave AI;

	/** Player reputation */
	UPROPERTY(EditAnywhere, Category = Save)
	float PlayerReputation;

	/** Value of all company assets */
	UPROPERTY(EditAnywhere, Category = Save)
	int64 CompanyValue;

	/** Date of last tribute given to the player */
	UPROPERTY(EditAnywhere, Category = Save)
	int64 PlayerLastTributeDate;

	/** Date of last peace with the player */
	UPROPERTY(EditAnywhere, Category = Save)
	int64 PlayerLastPeaceDate;

	/** Date of last war with the player */
	UPROPERTY(EditAnywhere, Category = Save)
	int64 PlayerLastWarDate;

	/** Unlocked technologies */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FName> UnlockedTechnologies;

	/** Science amount */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 ResearchAmount;

	/** Research inflation ratio*/
	UPROPERTY(EditAnywhere, Category = Save)
	float ResearchRatio;

	/** Research inflation ratio*/
	UPROPERTY(EditAnywhere, Category = Save)
	int32 ResearchSpent;

	/** List of capture target */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FName> CaptureOrders;

	/** List of company transactions */
	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FFlareTransactionLogEntry> TransactionLog;

	/** Quantity of damage deal to others companies */
	UPROPERTY(EditAnywhere, Category = Save)
	float Retaliation;
};

/** Catalog data */
USTRUCT()
struct FFlareCompanyDescription
{
	GENERATED_USTRUCT_BODY()

	/** Name */
	UPROPERTY(EditAnywhere, Category = Company)
	FText Name;

	/** Short name */
	UPROPERTY(EditAnywhere, Category = Company)
	FName ShortName;

	/** Name */
	UPROPERTY(EditAnywhere, Category = Company)
	FText Description;

	/** Emblem */
	UPROPERTY(EditAnywhere, Category = Company)
	UTexture2D* Emblem;

	/** Billboard */
	UPROPERTY(EditAnywhere, Category = Company)
	TArray<UTexture2D*> Billboards;

	/** Base color in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	FLinearColor CustomizationBasePaintColor;

	/** Paint color in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	FLinearColor CustomizationPaintColor;

	/** Overlay color in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	FLinearColor CustomizationOverlayColor;

	/** Lights color in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	FLinearColor CustomizationLightColor;

	/** Pattern index in the customization catalog */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 CustomizationPatternIndex;

};

/** Incoming event description */
USTRUCT()
struct FFlareIncomingEvent
{
	GENERATED_USTRUCT_BODY()

	/** Event text */
	UPROPERTY(EditAnywhere, Category = Save)
	FText Text;

	/** Days until event */
	UPROPERTY(EditAnywhere, Category = Content)
	int64 RemainingDuration;
};


/*----------------------------------------------------
	Basic structures
----------------------------------------------------*/

struct CompanyValue
{
	int64 MoneyValue;
	int64 StockValue;
	int64 ShipsValue;
	int64 ArmyValue;
	int32 ArmyTotalCombatPoints;
	int32 ArmyCurrentCombatPoints;

	int64 StationsValue;

	/** Ships + Stations*/
	int64 SpacecraftsValue;

	/** Money + Spacecrafts + Stock */
	int64 TotalValue;
};

struct WarTargetIncomingFleet
{
	int64 TravelDuration;
	int32 ArmyCombatPoints;
};

struct WarTarget
{
	UFlareSimulatedSector* Sector;
	int32 EnemyArmyCombatPoints;
	int32 EnemyArmyLCombatPoints;
	int32 EnemyArmySCombatPoints;
	int64 EnemyStationCount;
	int64 EnemyCargoCount;
	int32 OwnedArmyCombatPoints;
	int32 OwnedArmyAntiSCombatPoints;
	int32 OwnedArmyAntiLCombatPoints;
	int64 OwnedStationCount;
	int64 OwnedCargoCount;
	int64 OwnedMilitaryCount;
	TArray<WarTargetIncomingFleet> WarTargetIncomingFleets; // List player company fleets
	TArray<UFlareCompany*> ArmedDefenseCompanies;
};


/*----------------------------------------------------
	Low-level tools
----------------------------------------------------*/

/** Game save data */
USTRUCT()
struct FFlareFloatBuffer
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Save)
	int32 MaxSize;

	UPROPERTY(EditAnywhere, Category = Save)
	int32 WriteIndex;

	UPROPERTY(EditAnywhere, Category = Save)
	TArray<float> Values;


	void Init(int32 Size);

	void Resize(int32 Size);

	void Append(float NewValue);

	float GetValue(int32 Age);

	float GetMean(int32 StartAge, int32 EndAge);
};


USTRUCT()
struct FVectorArray
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<FVector> Entries;
};

USTRUCT()
struct FNameArray
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<FName> Entries;
};


USTRUCT()
struct FPtr
{
	GENERATED_USTRUCT_BODY()

	void* Entry;
};

/** Generic storage system */
USTRUCT()
struct FFlareBundle
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Save)
	TMap<FName, float> FloatValues;

	UPROPERTY(EditAnywhere, Category = Save)
	TMap<FName, int32> Int32Values;

	UPROPERTY(EditAnywhere, Category = Save)
	TMap<FName, FTransform> TransformValues;

	UPROPERTY(EditAnywhere, Category = Save)
	TMap<FName, FVectorArray> VectorArrayValues;

	UPROPERTY(EditAnywhere, Category = Save)
	TMap<FName, FNameArray> NameArrayValues;

	UPROPERTY(EditAnywhere, Category = Save)
	TMap<FName, FName> NameValues;

	UPROPERTY(EditAnywhere, Category = Save)
	TMap<FName, FString> StringValues;

	UPROPERTY(EditAnywhere, Category = Save)
	TArray<FName> Tags;

	UPROPERTY(EditAnywhere, Category = Save)
	TMap<FName, FPtr> PtrValues;

	bool HasFloat(FName Key) const;
	bool HasInt32(FName Key) const;
	bool HasTransform(FName Key) const;
	bool HasVectorArray(FName Key) const;
	bool HasName(FName Key) const;
	bool HasNameArray(FName Key) const;
	bool HasString(FName Key) const;
	bool HasTag(FName Tag) const;
	bool HasPtr(FName Tag) const;


	float GetFloat(FName Key, float Default = 0.f) const;
	int32 GetInt32(FName Key, int32 Default = 0) const;
	FTransform GetTransform(FName Key, const FTransform Default = FTransform::Identity) const;
	TArray<FVector> GetVectorArray(FName Key) const;
	FName GetName(FName Key) const;
	TArray<FName> GetNameArray(FName Key) const;
	FString GetString(FName Key) const;
	void* GetPtr(FName Key) const;

	FFlareBundle& PutFloat(FName Key, float Value);
	FFlareBundle& PutInt32(FName Key, int32 Value);
	FFlareBundle& PutTransform(FName Key, const FTransform Value);
	FFlareBundle& PutVectorArray(FName Key, const TArray<FVector> Value);
	FFlareBundle& PutName(FName Key, FName Value);
	FFlareBundle& PutNameArray(FName Key, const TArray<FName> Value);
	FFlareBundle& PutString(FName Key, FString Value);
	FFlareBundle& PutTag(FName Tag);
	FFlareBundle& PutPtr(FName Key, void* Value);

	void Clear();
};

struct DamageCause
{
	DamageCause();
	DamageCause(EFlareDamage::Type DamageTypeParam);
	DamageCause(UFlareSimulatedSpacecraft* SpacecraftParam, EFlareDamage::Type DamageTypeParam);
	DamageCause(UFlareCompany* CompanyParam, EFlareDamage::Type DamageTypeParam);

	UFlareSimulatedSpacecraft* Spacecraft;
	UFlareCompany* Company;
	EFlareDamage::Type DamageType;
	bool ManualTurret = false;
};


/*----------------------------------------------------
	Helper class
----------------------------------------------------*/

UCLASS()
class HELIUMRAIN_API UFlareGameTypes : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/** Get the group name */
	static FText GetCombatGroupDescription(EFlareCombatGroup::Type Type);

	/** Get the tactic name */
	static FText GetCombatTacticDescription(EFlareCombatTactic::Type Type);

	/** Get the icon for this combat group */
	static const FSlateBrush* GetCombatGroupIcon(EFlareCombatGroup::Type Type);


};
