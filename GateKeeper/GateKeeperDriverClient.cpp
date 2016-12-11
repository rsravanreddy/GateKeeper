//
//  GateKeeperDriverClient.cpp
//  GateKeeperDriver
//
//  Created by sravan rekula on 06/12/2016.
//  Copyright © 2016 Sravan. All rights reserved.
//

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <libkern/OSByteOrder.h>
#include "GateKeeperSharesPayload.h"
#include "GateKeeperDriverClient.h"
#include "GateKeeperDriverQueueManager.hpp"

#define super IOUserClient
OSDefineMetaClassAndStructors(com_sravan_driver_GateKeeperDriverClient, IOUserClient)

const IOExternalMethodDispatch
com_sravan_driver_GateKeeperDriverClient::sMethods[kNumberOfMethods] =
{
    { &com_sravan_driver_GateKeeperDriverClient::sClientPing, 0, sizeof(struct GateKeeperSharedPayload), 0, 0 },
};

IOReturn com_sravan_driver_GateKeeperDriverClient::externalMethod(uint32_t selector, IOExternalMethodArguments *args, IOExternalMethodDispatch *dispatch, OSObject *target, void *reference)
{
    if (selector >= kNumberOfMethods)
        return kIOReturnUnsupported;
    
    dispatch = (IOExternalMethodDispatch *)&sMethods[selector];
    target = this;
    reference = NULL;
    return super::externalMethod(selector, args, dispatch, target, reference);
}

IOReturn com_sravan_driver_GateKeeperDriverClient::sClientPing(OSObject *target, void *reference, IOExternalMethodArguments *args)
{
    IOLog("FlockFlockClient::sClientPing\n");
    //use myself to call into method
    com_sravan_driver_GateKeeperDriverClient *mySelf = (com_sravan_driver_GateKeeperDriverClient *)target;
    return kIOReturnSuccess;
}


bool com_sravan_driver_GateKeeperDriverClient::initWithTask(task_t owningTask, void *securityToken, UInt32 type, OSDictionary *properties)
{
    IOLog("GateKeeperDriverClient::initWithTask client init\n");
    
    if (!owningTask)
        return false;
    
    if (!super::initWithTask(owningTask, securityToken, type, properties))
        return false;
    
    m_task = owningTask;
    m_taskIsAdmin = false;
    
    IOReturn ret = clientHasPrivilege(securityToken, kIOClientPrivilegeAdministrator);
    if (ret == kIOReturnSuccess)
    {
        m_taskIsAdmin = true;
    }
    
    return true;
}

bool com_sravan_driver_GateKeeperDriverClient::start(IOService *provider)
{
    IOLog("GateKeeperDriverClient::start client start\n");
    if (! super::start(provider))
        return false;
    m_driver = OSDynamicCast(com_sravan_driver_GateKeeperDriver, provider);
    queueManager = m_driver->GetQueueManager();
    if (!m_driver)
        return false;
    return true;
}

IOReturn com_sravan_driver_GateKeeperDriverClient::clientClose(void)
{
    IOLog("GateKeeperDriverClient::clientClose client close\n");
    queueManager->clearMachPort();
    terminate();
    return kIOReturnSuccess;
}

IOReturn com_sravan_driver_GateKeeperDriverClient::registerNotificationPort(mach_port_t port, UInt32 type,
                                                                          io_user_reference_t refCon)
{
    IOLog("GateKeeperDriverClient::registerNotificationPort reference: %d\n", (int)refCon);
    switch (type) {
        case QUEUE_CONTROL:
            queueManager->SetLogPort(port);
            break;
        case QUEUE_LOG:
            queueManager->SetLogPort(port);
            break;
        default:
            break;
    }
    return kIOReturnSuccess;
}

void com_sravan_driver_GateKeeperDriverClient::stop(IOService *provider)
{
    super::stop(provider);
}

void com_sravan_driver_GateKeeperDriverClient::free(void)
{
    super::free();
}

IOReturn com_sravan_driver_GateKeeperDriverClient::clientMemoryForType(
                                                UInt32 type, IOOptionBits *options, IOMemoryDescriptor **memory) {
    switch (type) {
        case QUEUE_CONTROL:
            *options = 0;
            *memory = queueManager->GetControlMemoryDescriptor();
            break;
        case QUEUE_LOG:
            *options = 0;
            *memory = queueManager->GetLogMemoryDescriptor();
            break;
        default:
            return kIOReturnBadArgument;
    }
    
    (*memory)->retain();
    
    return kIOReturnSuccess;
}


#define super IOUserClient