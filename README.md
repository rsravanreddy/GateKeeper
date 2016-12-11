# GateKeeper
Gatekeeper is based on IOKit which logs every file access in the file system from the kernel which uses kauth and logs it to IOShared queue.
A user level program can connect to a matching driver and consume the shared queue events,this can be used to implement a anti-virus program or a event logger.
#GateKeeper provides two kinds of communication queues
1.Control queue to control the kauth to block/allow access to applications
2.Log queue which is used by IOKit Driver to log all the file system events.
