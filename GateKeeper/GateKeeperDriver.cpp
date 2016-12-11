/* add your code here */
#include <IOKit/IOLib.h>
#include "GateKeeperDriver.hpp"

// This required macro defines the class's constructors, destructors,
// and several other methods I/O Kit requires.
OSDefineMetaClassAndStructors(com_sravan_driver_GateKeeperDriver, IOService)

// Define the driver's superclass.
#define super IOService

bool com_sravan_driver_GateKeeperDriver::init(OSDictionary *dict)
{
    bool result = super::init(dict);
    IOLog("Initializing\n");
    return result;
}

void com_sravan_driver_GateKeeperDriver::free(void)
{
    IOLog("Freeing\n");
    super::free();
}

IOService *com_sravan_driver_GateKeeperDriver::probe(IOService *provider,
                                                   SInt32 *score)
{
    IOService *result = super::probe(provider, score);
    IOLog("Probing\n");
    return result;
}

bool com_sravan_driver_GateKeeperDriver::start(IOService *provider)
{
    bool result = super::start(provider);
    queueManager = new GateKeeperDriverQueueManager;
    if (!queueManager->init() ||
        queueManager->StartListener() != kIOReturnSuccess) {
        queueManager->release();
        queueManager = nullptr;
        return false;
    }
    if (result) {
        // Publish ourselves so clients can find us
        registerService();
    }
    IOLog("Starting\n");
    return result;
}

void com_sravan_driver_GateKeeperDriver::stop(IOService *provider)
{
    IOLog("Stopping\n");
    queueManager->StopListener();
    queueManager->release();
    super::stop(provider);
}

GateKeeperDriverQueueManager *com_sravan_driver_GateKeeperDriver::GetQueueManager() const {
    return queueManager;
}




