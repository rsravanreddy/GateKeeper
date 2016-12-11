//
//  GateKeeperSharedPayload.h
//  GateKeeperDriver
//
//  Created by sravan rekula on 06/12/2016.
//  Copyright © 2016 Sravan. All rights reserved.
//

#ifndef GateKeeperSharedPayload_h
#define GateKeeperSharedPayload_h
#include <sys/param.h>

typedef enum {
    NOTIFY_EXEC = 0,
    NOTIFY_WRITE = 1,
    NOTIFY_RENAME = 2,
    NOTIFY_LINK = 3,
    NOTIFY_EXCHANGE = 4,
    NOTIFY_DELETE = 5,
    
    // ERROR
    ACTION_ERROR = 50,
} action_t;

typedef enum {
    QUEUE_CONTROL,
    QUEUE_LOG,
} gateKeeper_qtype_t;

typedef struct GateKeeperSharedPayload {
    uint64_t pid;
    uint64_t node_id;
    uint64_t action;
    char path[MAXPATHLEN];
    char newpath[MAXPATHLEN];
    char pname[MAXPATHLEN];

} GateKeeperSharedPayload;

enum {
    kGateKeeperUserClientPing, //just a sample method
    kNumberOfMethods // Must be last
};

#endif /* GateKeeperSharedPayload_h */
