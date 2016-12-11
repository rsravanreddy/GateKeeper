# GateKeeper
Gatekeeper is kernel extension based on IOKit which logs every file access in the file system from the kernel which uses kauth and logs it to IOShared queue.
A user level program can connect to a matching driver and consume the shared queue events,this can be used to implement an anti-virus program or an event logger.
### GateKeeper provides two kinds of communication queues
1.Control queue to control the kauth to block/allow access to applications. <br />
2.Log queue which is used by IOKit Driver to log all the file system events. <br />
#### Control queue
Control queue is consumed by IoKit Driver to cache permissions for applications and consumed by kauth module to make decisions. 

#####TODO:
Implement cache to hold application permissions <br />
implement sample user space daemon to consume the queue <br />
