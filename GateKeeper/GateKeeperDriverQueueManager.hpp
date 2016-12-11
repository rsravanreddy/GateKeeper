//
//  GateKeeperDriverQueueManager.hpp
//  GateKeeperDriver
//
//  Created by sravan rekula on 06/12/2016.
//  Copyright © 2016 Sravan. All rights reserved.
//

#include <IOKit/IODataQueueShared.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOMemoryDescriptor.h>
#include <IOKit/IOSharedDataQueue.h>
#include <libkern/c++/OSDictionary.h>
#include <sys/kauth.h>
#include "GateKeeperSharedPayload.h"
#include <sys/proc.h>
#include <sys/vnode.h>

#define MAX_VNODE_ID_STR 21  // digits in UINT64_MAX + 1 for NULL-terminator


#ifndef GateKeeperDriverQueueManager_hpp
#define GateKeeperDriverQueueManager_hpp

class GateKeeperDriverQueueManager:public OSObject {
    OSDeclareDefaultStructors(GateKeeperDriverQueueManager);
public:
    bool init() override;
    void free() override;

    void clearMachPort();
    
    void SetLogPort(mach_port_t port);
    void SetControlPort(mach_port_t port);
    
    IOMemoryDescriptor *GetControlMemoryDescriptor() const;
    IOMemoryDescriptor *GetLogMemoryDescriptor() const;
    IOLock *control_lock_;
    IOLock *log_lock_;
    kern_return_t StartListener();
    kern_return_t StopListener();

    
    int32_t listener_invocations_;
    
    kauth_listener_t vnode_listener_;
    kauth_listener_t fileop_listener_;

    void IncrementListenerInvocations();
    void DecrementListenerInvocations();
    int VnodeCallback(const kauth_cred_t cred, const vfs_context_t ctx,
                      const vnode_t vp, int *errno);
    void FileOpCallback(kauth_action_t action, const vnode_t vp,
                        const char *path, const char *new_path);

    static inline uint64_t GetVnodeIDForVnode(const vfs_context_t ctx,
                                              const vnode_t vp) {
        struct vnode_attr vap;
        VATTR_INIT(&vap);
        VATTR_WANTED(&vap, va_fsid);
        VATTR_WANTED(&vap, va_fileid);
        vnode_getattr(vp, &vap, ctx);
        return vap.va_fileid;
    }
    
    static inline GateKeeperSharedPayload *NewMessage() {
        auto message = new GateKeeperSharedPayload;
        message->pid = proc_selfpid();
        return message;
    }
    
    bool PostToLogQueue(GateKeeperSharedPayload *message);
    
private:
    
    IOSharedDataQueue *control_dataqueue_;
    IOSharedDataQueue *log_dataqueue_;
    static const uint32_t kMaxDecisionQueueEvents = 512;
    
    ///
    ///  The maximum number of messages can be kept
    ///  in the logging data queue at any time.
    ///
    static const uint32_t kMaxLogQueueEvents = 1024;

};


extern "C" int vnode_scope_callback(
                                    kauth_cred_t credential, void *idata, kauth_action_t action,
                                    uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3);
extern "C" int fileop_scope_callback(
                                     kauth_cred_t credential, void *idata, kauth_action_t action,
                                     uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3);

#endif /* GateKeeperDriverQueueManager_hpp */
