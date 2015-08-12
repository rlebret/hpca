// Multi-threading utility functions
//
// Copyright (c) 2009 PGXIS - UMR CNRS 8524
// Written by Rémi Lebret <remi@lebret.ch>
//
// This file is part of HPCA.
//
// HPCA is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 3 as
// published by the Free Software Foundation.
//
// HPCA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with HPCA. If not, see <http://www.gnu.org/licenses/>.


#ifndef __USE_GNU
#define  __USE_GNU
#endif
// C headers
#include <stdio.h>
#include <pthread.h>
#include <math.h>
// tax3 header
#include "thread.h"


/** Static function to return the optimal number of threads
 **/
int MultiThread::optimal_nb_thread( const int n_thread
                                  , const int thread_per_cpu
                                  , const int n_element
                                  )
{
  int optimal_number = n_thread;
  // Determine the actual number of processors
  int NUM_PROCS = sysconf(_SC_NPROCESSORS_CONF);
  if ( (n_thread < 0) || ( n_thread > NUM_PROCS) )
  {
    if ( n_element < NUM_PROCS ) optimal_number = n_element ;
    else
    {
      optimal_number = NUM_PROCS;
      if ( optimal_number*thread_per_cpu < n_element ) optimal_number *= thread_per_cpu;
    }
  }
  else if ( (n_thread > 1) && (n_thread <= NUM_PROCS) )
  {
    if ( n_element < n_thread ) optimal_number = n_element;
    else if ( optimal_number*thread_per_cpu < n_element ) optimal_number *= thread_per_cpu;
  }
  else optimal_number = 1;

  // return the optimal number
  return optimal_number;
}

/** Allocate memory to MultiThread object
 **/
MultiThread::MultiThread( const int n_thread
                        , const int thread_per_cpu
                        , bool const force_cpu_affinity
                        , const int n_element
                        , void* that
                        , void* object
                        )
                        : nb_thread_(0)
                        , nb_element_(n_element)
                        , thread_(0)
{
  // get the optimal number of threads
  nb_thread_ = optimal_nb_thread(n_thread, thread_per_cpu, n_element);
  // create Threads
  thread_ = new Thread[nb_thread_];
  for (int i=0; i<nb_thread_; i++)
  {
    thread_[i].id_ = i;
    thread_[i].force_cpu_affinity_ = force_cpu_affinity;
    thread_[i].that = that;
    thread_[i].object = object;
  }
  // update thread id if there is only one thread
  if ( nb_thread_ == 1 ) thread_[0].id_ = -1;
}

/** release memory
 **/
MultiThread::~MultiThread()
{
#ifdef VERBOSE
  printf("MultiThread::delete()\n");
#endif
  // release Threads
  if ( thread_ ) delete[] thread_;
#ifdef VERBOSE
  printf("end MultiThread::delete()\n");
#endif
}

/** launch threads
 **/
void MultiThread::launch( void* (*run)( void*) )
{
  void *status;
  pthread_t p[nb_thread_];
  pthread_attr_t attr;

  /* Initialize and set thread detached attribute */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  for (int i=0; i<nb_thread_; i++)
  {
    // thread creation for one block
    if (pthread_create(&p[i], &attr, run, &thread_[i]) != 0)
      perror("Pthread_create failed");
  }

  /* Free attribute and wait for the other threads */
  pthread_attr_destroy(&attr);
  // join threads
  for (int i=0; i<nb_thread_; i++)
    if (pthread_join(p[i], &status) != 0) perror("Pthread_join failed");
}

/** Launch an equally spaced computation in multithreading
 **/
void MultiThread::linear( void* (*run)( void*) )
{
  // for unique thread
  if (nb_thread_ == 1 )
  {
    thread_[0].set_start(0);
    thread_[0].set_end(nb_element_);
    (*run)(&thread_[0]);
  }
  else // for multi threads
  {
    // number of variables in each thread
    // define remaining elements
    int rm = nb_element_;
    // add the start and the end of each data block
    for (int i=0; i<nb_thread_; i++)
    {
      thread_[i].set_end( rm );
      rm -= floor( rm / (nb_thread_-i) );
      thread_[i].set_start( rm );
      if ( rm==0 ) break;
    }
    // launch the threads
    launch( run );
  }
}

/** Launch an equally spaced computation in multithreading
 *  with pre-defined starting & ending values
 **/
void MultiThread::linear( void* (*run)( void*)
                        , const int start
                        , const int end
                        )
{
  // for unique thread
  if (nb_thread_ == 1 )
  {
    thread_[0].set_start(start);
    thread_[0].set_end(end);
    (*run)(&thread_[0]);
  }
  else // for multi threads
  {
    // number of variables in each thread
    // define remaining elements
    int rm = nb_element_;
    // add the start and the end of each data block
    for (int i=0; i<nb_thread_; i++)
    {
      thread_[i].set_end( rm+start );
      rm -= floor( rm / (nb_thread_-i) );
      thread_[i].set_start( rm+start );
      if (  rm==0 ) break;
    }
    launch( run );
  }
}
