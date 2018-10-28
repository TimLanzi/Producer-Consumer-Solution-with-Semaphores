# Producer-Consumer-Solution-with-Semaphores

A demonstration of the Producer/Consumer problem and a solution to the problem using semaphores. Two threads are created to act as the producer and consumer. These threads then enter their infinite loops to produce and consume items. Semaphores are used as a solution to a deadlock situation that can occur with this problem.

A deadlock situtation could still occur even if semaphores are being used if they are used carelessly. If the mutual exclusion semaphore were not "downed" last and "upped" first, in certain situations deadlock could still occur.
