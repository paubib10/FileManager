<<<<<<< HEAD
#ifndef SEMAFORO_MUTEX_POSIX
#define SEMAFORO_MUTEX_POSIX

=======
 /* semaforo_mutex_posix.h */
>>>>>>> d5d1384dab3b242d5fd11a1059a2082355146f51
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>


<<<<<<< HEAD

=======
>>>>>>> d5d1384dab3b242d5fd11a1059a2082355146f51
#define SEM_NAME "/mymutex" /* Usamos este nombre para el semáforo mutex */
#define SEM_INIT_VALUE 1 /* Valor inicial de los mutex */


sem_t *initSem();
void deleteSem();
void signalSem(sem_t *sem);
<<<<<<< HEAD
void waitSem(sem_t *sem);

#endif //SEMAFORO_MUTEX_POSIX
=======
void waitSem(sem_t *sem);
>>>>>>> d5d1384dab3b242d5fd11a1059a2082355146f51
