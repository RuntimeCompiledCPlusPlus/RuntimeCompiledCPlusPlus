Systems
=======

A system is a piece of functionality that implements the simple ISystem.h interface. 

Each kind of module should have:
 - an abtract interface in the Systems folder (e.g. ILogging.h)
 - a folder for its implementations (e.g. Logging)
 
A system can in principle have multiple interchangable implementations.
Each implementation folder should have:
 - a subfolder for each concrete implementation (e.g. FileLogger)