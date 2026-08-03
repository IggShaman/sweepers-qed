* aspects
** cpu throttling due to heating
** cpu may have a turbo mode - accelerates temporarily, until it is heated up
   too much => perf results depend on initial cpu temperature, as well as the
   outside environment's state (e.g. faster cooling in a cold room)
** cpus can have different kinds of core-s (efficiency vs performance);
   efficiency cores may have two HT-s, so either use all HT-s or none - e.g. run
   one thread per one efficiency core.

