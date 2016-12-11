/* add your code here */
/* add your code here */

#include <IOKit/IOService.h>
#include <IOKit/IODataQueueShared.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOMemoryDescriptor.h>
#include <IOKit/IOSharedDataQueue.h>
#include <libkern/c++/OSDictionary.h>
#include "GateKeeperDriverQueueManager.hpp"

class com_sravan_driver_GateKeeperDriver : public IOService
{
    OSDeclareDefaultStructors(com_sravan_driver_GateKeeperDriver)
public:
    virtual bool init(OSDictionary *dictionary = 0) override;
    virtual void free(void) override;
    virtual IOService *probe(IOService *provider, SInt32 *score) override;
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    
    GateKeeperDriverQueueManager *queueManager;
    GateKeeperDriverQueueManager *GetQueueManager() const;
};
