//
//  GateKeeperDriverQueueManager.cpp
//  GateKeeperDriver
//
//  Created by sravan rekula on 06/12/2016.
//  Copyright © 2016 Sravan. All rights reserved.
//

#include "GateKeeperDriverQueueManager.hpp"
#include "GateKeeperSharedPayload.h"

#define super OSObject
OSDefineMetaClassAndStructors(GateKeeperDriverQueueManager, OSObject);

bool GateKeeperDriverQueueManager::init() {
    if (!super::init()) return false;
    control_dataqueue_ = IOSharedDataQueue::withEntries(kMaxDecisionQueueEvents,
                                                         sizeof(GateKeeperSharedPayload));
    if (!control_dataqueue_) return kIOReturnNoMemory;
    
    log_dataqueue_ = IOSharedDataQueue::withEntries(kMaxLogQueueEvents,
                                                    sizeof(GateKeeperSharedPayload));
    if (!log_dataqueue_) return kIOReturnNoMemory;
    control_lock_ = IOLockAlloc();
    log_lock_ = IOLockAlloc();

    return true;
}

IOMemoryDescriptor *GateKeeperDriverQueueManager::GetControlMemoryDescriptor() const {
    return control_dataqueue_->getMemoryDescriptor();
}

IOMemoryDescriptor *GateKeeperDriverQueueManager::GetLogMemoryDescriptor() const {
    return log_dataqueue_->getMemoryDescriptor();
}

void GateKeeperDriverQueueManager::SetLogPort(mach_port_t port){
    IOLockLock(log_lock_);
    log_dataqueue_->setNotificationPort(port);
    IOLockUnlock(log_lock_);
}
void GateKeeperDriverQueueManager::SetControlPort(mach_port_t port){
    IOLockLock(control_lock_);
    log_dataqueue_->setNotificationPort(port);
    IOLockLock(control_lock_);
}


void GateKeeperDriverQueueManager::clearMachPort(){
    
}

void GateKeeperDriverQueueManager::free() {
    IOLockFree(log_lock_);
    IOLockFree(control_lock_);
    OSSafeReleaseNULL(control_dataqueue_);
    OSSafeReleaseNULL(log_dataqueue_);
}


kern_return_t GateKeeperDriverQueueManager::StartListener() {
    vnode_listener_ = kauth_listen_scope(KAUTH_SCOPE_VNODE,
                                         vnode_scope_callback,
                                         reinterpret_cast<void *>(this));
    if (!vnode_listener_) return kIOReturnInternalError;
    
    fileop_listener_ = kauth_listen_scope(KAUTH_SCOPE_FILEOP,
                                          fileop_scope_callback,
                                          reinterpret_cast<void *>(this));
    if (!fileop_listener_) return kIOReturnInternalError;
    
    IOLog("Listeners started.");
    
    return kIOReturnSuccess;
}

kern_return_t GateKeeperDriverQueueManager::StopListener() {
    kauth_unlisten_scope(vnode_listener_);
    vnode_listener_ = nullptr;
    
    kauth_unlisten_scope(fileop_listener_);
    fileop_listener_ = nullptr;
    
    // Wait for any active invocations to finish before returning
    do {
        IOSleep(5);
    } while (listener_invocations_);
    
    IOLog("Listeners stopped.");
    
    return kIOReturnSuccess;
}


extern "C" int fileop_scope_callback(
                                     kauth_cred_t credential, void *idata, kauth_action_t action,
                                     uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3) {
    auto sdm = OSDynamicCast(
                             GateKeeperDriverQueueManager, reinterpret_cast<OSObject *>(idata));
    
    vnode_t vp = nullptr;
    char *path = nullptr;
    char *new_path = nullptr;
    
    switch (action) {
        case KAUTH_FILEOP_CLOSE:
            if (!(arg2 & KAUTH_FILEOP_CLOSE_MODIFIED)) return KAUTH_RESULT_DEFER;
            // Intentional fall-through
        case KAUTH_FILEOP_DELETE:
        case KAUTH_FILEOP_EXEC:
            vp = reinterpret_cast<vnode_t>(arg0);
            if (vp && vnode_vtype(vp) != VREG) return KAUTH_RESULT_DEFER;
            path = reinterpret_cast<char *>(arg1);
            break;
        case KAUTH_FILEOP_RENAME:
        case KAUTH_FILEOP_EXCHANGE:
        case KAUTH_FILEOP_LINK:
            path = reinterpret_cast<char *>(arg0);
            new_path = reinterpret_cast<char *>(arg1);
            break;
        default:
            return KAUTH_RESULT_DEFER;
    }
    
    sdm->IncrementListenerInvocations();
    sdm->FileOpCallback(action, vp, path, new_path);
    sdm->DecrementListenerInvocations();
    
    return KAUTH_RESULT_DEFER;
}

extern "C" int vnode_scope_callback(
                                    kauth_cred_t credential, void *idata, kauth_action_t action,
                                    uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3) {
    if (action & KAUTH_VNODE_ACCESS ||
        !(action & KAUTH_VNODE_EXECUTE) ||
        idata == nullptr) {
        return KAUTH_RESULT_DEFER;
    }
    
    auto sdm = OSDynamicCast(
                             GateKeeperDriverQueueManager, reinterpret_cast<OSObject *>(idata));
    
    sdm->IncrementListenerInvocations();
    int result = sdm->VnodeCallback(credential,
                                    reinterpret_cast<vfs_context_t>(arg0),
                                    reinterpret_cast<vnode_t>(arg1),
                                    reinterpret_cast<int *>(arg3));
    sdm->DecrementListenerInvocations();
    return result;
}

void GateKeeperDriverQueueManager::IncrementListenerInvocations() {
    OSIncrementAtomic(&listener_invocations_);
}

void GateKeeperDriverQueueManager::DecrementListenerInvocations() {
    OSDecrementAtomic(&listener_invocations_);
}

int GateKeeperDriverQueueManager::VnodeCallback(const kauth_cred_t cred,
                                        const vfs_context_t ctx,
                                        const vnode_t vp,
                                        int *errno) {
    // Only operate on regular files (not directories, symlinks, etc.).
    if (vnode_vtype(vp) != VREG) return KAUTH_RESULT_DEFER;
    
    // Get ID for the vnode and convert it to a string.
    auto vnode_id = GetVnodeIDForVnode(ctx, vp);
    char vnode_str[MAX_VNODE_ID_STR];
    snprintf(vnode_str, MAX_VNODE_ID_STR, "%llu", vnode_id);
    
    return KAUTH_RESULT_DEFER;

}

void GateKeeperDriverQueueManager::FileOpCallback(
                                          const kauth_action_t action, const vnode_t vp,
                                          const char *path, const char *new_path) {
    if (vp) {
        auto context = vfs_context_create(nullptr);
        auto vnode_id = GetVnodeIDForVnode(context, vp);
        vfs_context_rele(context);
        
        if (action == KAUTH_FILEOP_CLOSE) {
            char vnode_id_str[MAX_VNODE_ID_STR];
            snprintf(vnode_id_str, MAX_VNODE_ID_STR, "%llu", vnode_id);
            //CacheCheck(vnode_id_str);
        } else if (action == KAUTH_FILEOP_EXEC) {
            auto message = NewMessage();
            message->node_id = vnode_id;
            message->action = NOTIFY_EXEC;
            strlcpy(message->path, path, sizeof(message->path));
            
            char vnode_str[MAX_VNODE_ID_STR];
            snprintf(vnode_str, MAX_VNODE_ID_STR, "%llu", vnode_id);
                    
            PostToLogQueue(message);
            delete message;
            return;
        }
    }
    
    // Filter out modifications to locations that are definitely
    // not useful or made by santad.
    if (
        !strprefix(path, "/.") && !strprefix(path, "/dev")) {
        auto message = NewMessage();
        strlcpy(message->path, path, sizeof(message->path));
        if (new_path) strlcpy(message->newpath, new_path, sizeof(message->newpath));
        proc_name((int)message->pid, message->pname, sizeof(message->pname));
        
        switch (action) {
            case KAUTH_FILEOP_CLOSE:
                message->action = NOTIFY_WRITE;
                break;
            case KAUTH_FILEOP_RENAME:
                message->action = NOTIFY_RENAME;
                break;
            case KAUTH_FILEOP_LINK:
                message->action = NOTIFY_LINK;
                break;
            case KAUTH_FILEOP_EXCHANGE:
                message->action = NOTIFY_EXCHANGE;
                break;
            case KAUTH_FILEOP_DELETE:
                message->action = NOTIFY_DELETE;
                break;
            default: delete message; return;
        }
        
        PostToLogQueue(message);
        delete message;
    }
}

bool GateKeeperDriverQueueManager::PostToLogQueue(GateKeeperSharedPayload *message) {
    IOLockLock(log_lock_);
    auto kr = log_dataqueue_->enqueue(message, sizeof(GateKeeperSharedPayload));
    if (!kr) {
        // If enqueue failed, pop an item off the queue and try again.
        uint32_t dataSize = 0;
        log_dataqueue_->dequeue(0, &dataSize);
        kr = log_dataqueue_->enqueue(message, sizeof(GateKeeperSharedPayload));
    }
    IOLockUnlock(log_lock_);

    return kr;
}

