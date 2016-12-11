//
//  GateKeeperDriverClient.h
//  GateKeeperDriver
//
//  Created by sravan rekula on 06/12/2016.
//  Copyright © 2016 Sravan. All rights reserved.
//
#include <IOKit/IOService.h>
#include <IOKit/IOUserClient.h>
#include <IOKit/IOLib.h>
#include "GateKeeperDriver.hpp"
#include "GateKeeperDriverQueueManager.hpp"

#ifndef GateKeeperDriverClient_h
#define GateKeeperDriverClient_h

class com_sravan_driver_GateKeeperDriverClient : public IOUserClient
{
    OSDeclareDefaultStructors(com_sravan_driver_GateKeeperDriverClient)
    
private:
    task_t m_task;
    bool m_taskIsAdmin;
    com_sravan_driver_GateKeeperDriver *m_driver;
    
public:
    virtual bool initWithTask(task_t owningTask, void *securityToken, UInt32 type, OSDictionary *properties) override;
    virtual bool start(IOService *provider) override;
    virtual IOReturn clientClose(void) override;
    virtual void stop(IOService *provider) override;
    virtual void free(void) override;
    virtual IOReturn registerNotificationPort(mach_port_t port, UInt32 type, io_user_reference_t refCon) override;
    IOReturn externalMethod(uint32_t selector, IOExternalMethodArguments *args, IOExternalMethodDispatch *dispatch,
                            OSObject *target, void *reference) override;
    IOReturn clientMemoryForType(UInt32 type, IOOptionBits *options, IOMemoryDescriptor **memory) override;
    
private:
    GateKeeperDriverQueueManager *queueManager;
    static const IOExternalMethodDispatch sMethods[kNumberOfMethods];
    static IOReturn sClientPing(OSObject *target, void *reference, IOExternalMethodArguments *args);


};

#endif /* GateKeeperDriverClient_h */

