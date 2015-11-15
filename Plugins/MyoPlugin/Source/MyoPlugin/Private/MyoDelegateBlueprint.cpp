#include "MyoPluginPrivatePCH.h"
#include "MyoDelegateBlueprint.h"

//Events
void MyoDelegateBlueprint::MyoOnConnect(int32 myoId, uint64 timestamp)
{
	if (MyoIsValidId(myoId))
		IMyoInterface::Execute_OnConnect(_interfaceDelegate, InternalControllerForId(myoId));
}
void MyoDelegateBlueprint::MyoOnDisconnect(int32 myoId, uint64 timestamp)
{
	if(MyoIsValidId(myoId))
		IMyoInterface::Execute_OnDisconnect(_interfaceDelegate, InternalControllerForId(myoId));
}
void MyoDelegateBlueprint::MyoOnPair(int32 myoId, uint64 timestamp)
{
	//Ensure our internal array is up to date
	if (myoId>_latestFrame.Num())
		InternalAddController(myoId);

	if(MyoIsValidId(myoId))
		IMyoInterface::Execute_OnPair(_interfaceDelegate, InternalControllerForId(myoId));
}
void MyoDelegateBlueprint::MyoOnUnpair(int32 myoId, uint64 timestamp)
{
	if (MyoIsValidId(myoId))
		IMyoInterface::Execute_OnUnpair(_interfaceDelegate, InternalControllerForId(myoId));
}
void MyoDelegateBlueprint::MyoOnArmMoved(int32 myoId, FVector armAcceleration, FRotator armOrientation, FVector armGyro, MyoPose pose)
{
	if(MyoIsValidId(myoId))
		IMyoInterface::Execute_OnArmMoved(_interfaceDelegate, InternalControllerForId(myoId), armAcceleration, armOrientation, armGyro, pose);
}

void MyoDelegateBlueprint::MyoOnOrientationData(int32 myoId, uint64 timestamp, FRotator rotation)
{
	if(MyoIsValidId(myoId))
		IMyoInterface::Execute_OnOrientationData(_interfaceDelegate, InternalControllerForId(myoId), rotation);
}
void MyoDelegateBlueprint::MyoOnAccelerometerData(int32 myoId, uint64 timestamp, FVector accel)
{
	if(MyoIsValidId(myoId))
		IMyoInterface::Execute_OnAccelerometerData(_interfaceDelegate, InternalControllerForId(myoId), accel);
}
void MyoDelegateBlueprint::MyoOnGyroscopeData(int32 myoId, uint64 timestamp, FVector gyro)
{
	if(MyoIsValidId(myoId))
		IMyoInterface::Execute_OnGyroscopeData(_interfaceDelegate, InternalControllerForId(myoId), gyro);
}
void MyoDelegateBlueprint::MyoOnPose(int32 myoId, uint64 timestamp, int32 pose)
{
	if(MyoIsValidId(myoId))
		IMyoInterface::Execute_OnPose(_interfaceDelegate, InternalControllerForId(myoId), (MyoPose)pose);
}
void MyoDelegateBlueprint::MyoOnArmSync(int32 myoId, uint64 timestamp, int32 arm, int32 direction)
{
	if(MyoIsValidId(myoId))
		IMyoInterface::Execute_OnArmSync(_interfaceDelegate, InternalControllerForId(myoId), (MyoArm)arm, (MyoArmDirection)direction);
}
void MyoDelegateBlueprint::MyoOnArmUnsync(int32 myoId, uint64 timestamp)
{
	if(MyoIsValidId(myoId))
		IMyoInterface::Execute_OnArmUnsync(_interfaceDelegate, InternalControllerForId(myoId));
}
void MyoDelegateBlueprint::MyoOnEmgData(int32 myoId, FMyoEmgData data)
{
	if (MyoIsValidId(myoId))
		IMyoInterface::Execute_OnEmgData(_interfaceDelegate, InternalControllerForId(myoId), data);
}

void MyoDelegateBlueprint::MyoDisabled()
{
	if (IsValidDelegate())
		IMyoInterface::Execute_DeviceDisabled(_interfaceDelegate);
}

//Blueprint functions forward
bool MyoDelegateBlueprint::MyoIsHubEnabled()
{
	return MyoDelegate::MyoIsHubEnabled();
}
void MyoDelegateBlueprint::MyoSetLockingPolicy(MyoLockingPolicy policy)
{
	MyoDelegate::MyoSetLockingPolicy(policy);
}

void MyoDelegateBlueprint::MyoConvertToMyoOrientationSpace(FRotator orientation, FRotator& converted)
{
	MyoConvertToMyoOrientationSpace(orientation, converted);
}

UMyoController* MyoDelegateBlueprint::MyoPrimaryMyo()
{
	int myoId = 0;
	bool available = false;
	MyoPrimaryMyoId(available, myoId);

	return InternalControllerForId(myoId);
}
UMyoController* MyoDelegateBlueprint::MyoLeftMyo()
{
	int myoId = 0;
	bool available = false;
	MyoLeftMyoId(available, myoId);

	return InternalControllerForId(myoId);
}
UMyoController* MyoDelegateBlueprint::MyoRightMyo()
{
	int myoId = 0;
	bool available = false;
	MyoRightMyoId(available, myoId);

	return InternalControllerForId(myoId);
}

//Internal
UMyoController* MyoDelegateBlueprint::InternalAddController(int newId)
{
	//Obtain a pointer of self for use in making the Myo Controllers
	UObject* validUObject = NULL;
	validUObject = Cast<UObject>(ValidSelfPointer);
	UMyoController* controller;
	if (validUObject)
		controller = NewObject<UMyoController>(validUObject, UMyoController::StaticClass());
	else
		controller = NewObject<UMyoController>(UMyoController::StaticClass());	//no ownership
	_latestFrame.Add(controller);

	controller->_myoDelegate = this;
	controller->myoId = newId;

	return controller;
}

UMyoController* MyoDelegateBlueprint::InternalControllerForId(int32 myoId)
{
	if (MyoIsValidId(myoId))
		return _latestFrame[myoId - 1];
	else
		return NULL;
}

void MyoDelegateBlueprint::MyoStartup()
{
	MyoDelegate::MyoStartup();

	//Setup our Controller BP array
	
	for (int i = 0; i < MyoMaxId(); i++)
	{
		InternalAddController(i+1);
	}

	UObject* validUObject = NULL;
	validUObject = Cast<UObject>(ValidSelfPointer);

	//Set self as interface delegate by default
	if (!_interfaceDelegate && validUObject)
		SetInterfaceDelegate(validUObject);
}

void MyoDelegateBlueprint::MyoShutdown()
{
	MyoDelegate::MyoShutdown();
}

void MyoDelegateBlueprint::MyoTick(float DeltaTime)
{
	MyoDelegate::MyoTick(DeltaTime);

	//Sync the Myos with their MyoController data 
	//Sync our array

	for (int i = 0; i < MyoMaxId(); i++)
	{
		UMyoController* controller = _latestFrame[i];
		controller->setFromMyoDeviceData(MyoDelegate::MyoLatestData(i+1));
		controller->myoId = i+1;
	}
}

bool MyoDelegateBlueprint::MyoIsValidId(int32 myoId)
{
	return (IsValidDelegate() && MyoDelegate::MyoIsValidId(myoId));
}

bool MyoDelegateBlueprint::IsValidDelegate()
{
	return (_interfaceDelegate != NULL);
}

void MyoDelegateBlueprint::SetInterfaceDelegate(UObject* newDelegate)
{
	UE_LOG(LogClass, Log, TEXT("InterfaceDelegate passed: %s"), *newDelegate->GetName());

	//Use this format to support both blueprint and C++ form
	if (newDelegate->GetClass()->ImplementsInterface(UMyoInterface::StaticClass()))
	{
		_interfaceDelegate = newDelegate;
	}
	else
	{
		//Try casting as self
		if (ValidSelfPointer->GetClass()->ImplementsInterface(UMyoInterface::StaticClass()))
		{
			_interfaceDelegate = (UObject*)this;
		}
		else
		{
			//If you're crashing its probably because of this setting causing an assert failure
			_interfaceDelegate = NULL;
		}

		//Either way post a warning, this will be a common error
		UE_LOG(LogClass, Log, TEXT("MyoDelegateBlueprint Warning: Delegate is NOT set, did your class implement HydraInterface?"));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("MyoDelegateBlueprint Warning: Delegate is NOT set, did your class implement MyoInterface?"));
	}
}
