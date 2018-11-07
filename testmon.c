#include <nSystem.h>

#define EXITO 0
#define FRACASO 1
#define LIBRE 0
#define TOMADO 1

void dormir(int pausa) {
  int ini= nGetTime();
  int duracion;
  nSleep(pausa);
  duracion= nGetTime()-ini;
  if (duracion<pausa)
    nFatalError("dormir", "Algo no funciona bien, dormir(%d) tomo solo %d milisegs.\n", pausa, duracion);
}

int TestProc1(nMonitor mon, int pausa, int* pverif) {
  nEnter(mon);
  if (*pverif==TOMADO)
    nFatalError("TestProc1", "no se cumple la exclusion mutua\n");
  *pverif= TOMADO;
  dormir(pausa);
  *pverif= LIBRE;
  nExit(mon);
  return 0;
}

int TestProc2(nMonitor mon, int pausa1, int timeout, int pausa2, int* pverif, int *pstate) {
  int ini, dt1, dt2, dt3, dt4;
  ini= nGetTime();
  nEnter(mon);
  dt1= nGetTime()-ini;
  if (*pverif==TOMADO)
    nFatalError("TestProc2", "no se cumple la exclusion mutua\n");
  *pverif= TOMADO;
  dormir(pausa1);
  dt2= nGetTime()-ini;
  *pverif= LIBRE;
  nWaitTimeout(mon, timeout);
  dt3= nGetTime()-ini;
  if (timeout>0 && !*pstate && nGetTime()-ini<timeout)
    nFatalError("TestProc2", "no se cumplio el timeout de %d\n", timeout);
  if (*pverif==TOMADO)
    nFatalError("TestProc2", "no se cumple la exclusion mutua\n");
  *pverif= TOMADO;
  dormir(pausa2);
  dt4= nGetTime()-ini;
  *pverif= LIBRE;
  nExit(mon);
  nPrintf("TestProc2 %d %d %d %d\n", dt1, dt2, dt3, dt4);
  return 0;
}

int TestProc3(nMonitor mon, int pausa, int* pverif) {
  nEnter(mon);
  if (*pverif==TOMADO)
    nFatalError("TestProc3", "no se cumple la exclusion mutua\n");
  *pverif= TOMADO;
  dormir(pausa);
  nNotifyAll(mon);
  dormir(pausa);
  *pverif= LIBRE;
  nExit(mon);
  return 0;
}

int TestProc4(nMonitor mon, int iter, int* pverif) {
  int i;
  while (iter--) {
    nEnter(mon);
    if (*pverif==TOMADO)
      nFatalError("TestProc4", "no se cumple la exclusion mutua\n");
    *pverif= TOMADO;
    for(i=0; i<10000;i++)
      ;
    *pverif= LIBRE;
    nExit(mon);
  }
  return 0;
}

int TestTimeouts() {
  nTask t1, t2;
  int i;
  for (i=0; i<10; i++) {
    int pausa= 100;
    int ini= nGetTime();
    int duracion;
    int verif= LIBRE;
    int stat= FALSE;
    nMonitor mon= nMakeMonitor();
    t1= nEmitTask(TestProc2, mon, pausa, 10*pausa, pausa, &verif, &stat);
    t2= nEmitTask(TestProc2, mon, pausa, 10*pausa, pausa, &verif, &stat);
    dormir(pausa);
    nWaitTask(t1); nWaitTask(t2);
    duracion= nGetTime()-ini;
    nPrintf("duracion= %d\n", duracion);
    if ( duracion < 13*pausa ) {
      nFatalError("nMain", "*** El test toma menos tiempo que el requerido.  Test no aprobado\n");
      break;
    }
    if ( duracion < 14*pausa ) {
      break;
    }
    if (i==9)
      nFatalError("nMain", "*** El test toma mas tiempo que el necesario.  Test no aprobado.\n");
    else
      nPrintf("El test toma mas tiempo que el necesario.  Se hara otro intento.\n");
    nDestroyMonitor(mon);
  }
  return 0;
}

int TestNoTimeouts() {
  int ntasks= 10;
  int pausa= 20;
  nTask* tasks= (nTask*)nMalloc(ntasks*sizeof(nTask));
  nMonitor mon= nMakeMonitor();
  int duracion;
  int verif= LIBRE;
  int stat= FALSE;
  int i, ini;
  for (i=0; i<ntasks; i++) {
    tasks[i]= nEmitTask(TestProc2, mon, pausa, ntasks*pausa*10, pausa, &verif,
 &stat);
  }
  dormir(ntasks*pausa*2);
  nEnter(mon);
  ini= nGetTime();
  stat= TRUE;
  nNotifyAll(mon);
  nExit(mon);
  for (i= 0; i<ntasks; i++) {
    nWaitTask(tasks[i]);
  }
  duracion= nGetTime()-ini;
  if (duracion>ntasks*pausa*2)
    nFatalError("nMain", "*** Las tareas se despiertan en mas tiempo que el necesario.  Test no aprobado.\n"); 
  nFree(tasks);
  nDestroyMonitor(mon);
  nPrintf("Test aprobado\n");

  return 0;
}

int nMain(int argc, char** argv) {
  nTask t1, t2, t3;
  int i;

  nPrintf("Los primeros tests funcionan en modo non-preemptive\n");
  nSetTimeSlice(0);

  {
    int pausa= 100;
    int ini= nGetTime();
    int duracion;
    int verif= LIBRE;
    nMonitor mon= nMakeMonitor();
    nPrintf("Test de exclusion mutua\n");
    t1= nEmitTask(TestProc1, mon, pausa, &verif);
    t2= nEmitTask(TestProc1, mon, pausa, &verif);
    t3= nEmitTask(TestProc1, mon, pausa, &verif);
    nWaitTask(t1); nWaitTask(t2); nWaitTask(t3);
    duracion= nGetTime()-ini;
    nPrintf("duracion= %d\n", duracion);
    if ( duracion<pausa*3 )
      nFatalError("nMain", "*** El tiempo es inferior al esperado\n");
    else
      nPrintf("Test aprobado\n");
    nDestroyMonitor(mon);
  }

  for (i=0; i<10; i++) {
    int pausa= 100;
    int ini= nGetTime();
    int duracion;
    int verif1= LIBRE, verif2= LIBRE, verif3= LIBRE;
    nMonitor mon1= nMakeMonitor();
    nMonitor mon2= nMakeMonitor();
    nMonitor mon3= nMakeMonitor();
    nPrintf("Test de no exclusion mutua\n");
    nPrintf("Tareas que piden monitores distintos, deben obtenerlos en paralelo\n");
    t1= nEmitTask(TestProc1, mon1, pausa, &verif1);
    t2= nEmitTask(TestProc1, mon2, pausa, &verif2);
    t3= nEmitTask(TestProc1, mon3, pausa, &verif3);
    nWaitTask(t1);   nWaitTask(t2);   nWaitTask(t3);
    duracion= nGetTime()-ini;
    nPrintf("duracion= %d\n", duracion);
    if ( duracion<=pausa+20) {
      nPrintf("Test aprobado\n");
      break;
    }
    if (i==9)
      nPrintf("*** El test toma mas tiempo del necesario.  Test no aprobado.\n");
    else
      nPrintf("El test toma mas tiempo del necesario.  Se hara otro intento.\n");
    nDestroyMonitor(mon1);
    nDestroyMonitor(mon2);
    nDestroyMonitor(mon3);
  }

  for (i=0; i<10; i++) {
    int pausa= 100;
    int ini= nGetTime();
    int duracion;
    int verif= LIBRE;
    int stat= FALSE;
    nMonitor mon= nMakeMonitor();
    nPrintf("Test de wait/notifyAll\n");
    t1= nEmitTask(TestProc2, mon, pausa, -1, pausa, &verif, &stat);
    t2= nEmitTask(TestProc2, mon, pausa, -1, pausa, &verif, &stat);
    dormir(pausa);
    t3= nEmitTask(TestProc3, mon, pausa, &verif);
    nWaitTask(t1); nWaitTask(t2); nWaitTask(t3);
    duracion= nGetTime()-ini;
    nPrintf("duracion= %d\n", duracion);
    if ( duracion < 6*pausa ) {
      nFatalError("nMain", "*** El test toma menos tiempo que el requerido.  Test no aprobado\n");
      break;
    }
    if ( duracion < 7*pausa ) {
      break;
    }
    if (i==9)
      nFatalError("nMain", "*** El test toma mas tiempo que el necesario.  Test no aprobado.\n");
    else
      nPrintf("El test toma mas tiempo que el necesario.  Se hara otro intento.\n");
    nDestroyMonitor(mon);
  }

#   define N 4

  {
    nTask tasks[N];
    int i;
    nPrintf("Test de timeouts\n");
    nPrintf("Cuando se vence el timeout, la tarea debe continuar\n");
    for (i= 0; i<N; i++) {
      tasks[i]= nEmitTask(TestTimeouts);
      dormir(10);
    }
    for (i= 0; i<N; i++)
      nWaitTask(tasks[i]);
    nPrintf("Test aprobado\n");
  }

  {
    nTask tasks[N];
    int i;
    nPrintf("Test de no timeouts\n");
    nPrintf("Cuando la condicion se verifica antes que se cumpla el timeout, la tarea debe continuar\n");
    for (i= 0; i<N; i++) {
      tasks[i]= nEmitTask(TestNoTimeouts);
      dormir(10);
    }
    for (i= 0; i<N; i++)
      nWaitTask(tasks[i]);
    nPrintf("Test aprobado\n");
  }

  nPrintf("El ultimo test funciona en modo preemptive\n");
  nSetTimeSlice(1);

  {
    int ntasks= 4000;
    int nmons= 200;
    nTask* tasks= (nTask*)nMalloc(ntasks*sizeof(nTask));
    nMonitor* mons= (nMonitor*)nMalloc(nmons*sizeof(nMonitor));
    int* verifs= (int*)nMalloc(nmons*sizeof(int));
    int j;
    int nfracasos= 0;
    nPrintf("Test de robustez\n");
    for (j= 0; j<nmons; j++) {
      mons[j]= nMakeMonitor();
      verifs[j]= 0;
    }
    for (j= 0; j<ntasks; j++)
      tasks[j]= nEmitTask(TestProc4, mons[j%nmons], 100, &verifs[j%nmons]);
    for (j= 0; j<ntasks; j++)
      nfracasos+= nWaitTask(tasks[j]);
    if (nfracasos>0)
      nPrintf("*** Se detectaron %d problemas de exclusion mutua.  Test no aprobado\n");
    else
      nPrintf("Test aprobado\n");
    for (j= 0; j<nmons; j++)
      nDestroyMonitor(mons[j]);
    nFree(tasks); nFree(mons); nFree(verifs);
  }

  return 0;
}
