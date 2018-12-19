## Spectre 

An exploration of the Spectre processor vulnerability. 

### Overview

- `cachetime.c`: a quick look at how we can use timing to determine the presence of particular data in the cache. 
- `flushreload.c`: using the cache flush+reload technique to determine a secret value based on cache timing. 
- `ooe.c`: exploiting out of order execution to read a secret value based on the result of an operation that is never actually executed (at the architectural level).
- `attack.c`: the vanilla Spectre attack on a secret value within the same process. 
- `improved.c`: an improved attack with minimized noise. 
- `full.c`: an extension of the improved attack to read the entirety of a secret value within the same process. 

### Acknowledgements

Adapted from Professor Wenliang Du's original [laboratory exercise](http://www.cis.syr.edu/~wedu/seed/Labs_16.04/System/Spectre_Attack/). 
