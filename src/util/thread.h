// Multi-threading utility functions
//
// Copyright (c) 2015 Idiap Research Institute, http://www.idiap.ch/
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

/**
 * @file        thread.h
 * @author      Remi Lebret
 * @brief       thread public interface.
 */

#ifndef THREAD_H_
#define THREAD_H_

#ifndef __USE_GNU
#define  __USE_GNU
#endif
// C header
#include <cstdio>
#include <sched.h>
#include <unistd.h>
// C++ header
#include <vector>
#include <numeric>

/**
 * 	@ingroup Utility
 * 	@{
 *
 * 	@class Thread
 *
 *	@brief a @c Thread object is created by @c MultiThread.
 * 	It contains a starting point, an ending point and an identifier.
 *  It can also stores two pointers:
 *  - on object where the thread is called
 *  - on object on which apply the thread
 */
class Thread
{
  private:
    /**< first index */
    int start_;
    /**< last index */
    int end_;

  public:
    /**< identifier */
    long id_;
    /**< pointer on object where the thread is called */
    void* that;
    /**< pointer on object on which apply the thread */
    void* object;
    /**< pointer on a secondary object on which apply the thread */
    void* object2;
    /**< force CPU affinity ? */
    bool force_cpu_affinity_;

    /**
     *  @brief Constructor
     *
     *  Create an empty @c Thread
     */
    Thread() : start_(0), end_(0), id_(0)
             , that(0), object(0), object2(0)
    {
#ifdef VERBOSE
  printf("Thread::create()\n");
#endif
    }

    /**
     *  @brief Constructor
     *
     *  Create a unique thread in a multithreading process
     *
     *  @param t the pointer to that
     *  @param o the pointer to object
     *  @param end the ending point
     */
    Thread( void* t, void* o, const int end )
          : start_(0), end_(end)
          , id_(-1), that(t)
          , object(o), object2(0)
    {
#ifdef VERBOSE
  printf("Thread::create(%ld)\n",id_);
#endif
    }

    /**
     *  @brief Destructor
     *
     *  Delete a @c Thread
     */
    ~Thread()
    {
#ifdef VERBOSE
  printf("Thread::delete(%ld)\n",id_);
#endif
    }

    /**
     *  @brief Accessor
     *
     *  Get the starting point.
     *
     *  @return the starting point
     */
    inline const int start() const
    { return start_; }

    /**
     *  @brief Accessor
     *
     *  Get the ending point.
     *
     *  @return the ending point
     */
    inline const int end() const
    { return end_; }

    /**
     *  @brief Accessor
     *
     *  Get the thread ID.
     *
     *  @return the ID
     */
    inline const long id()
    { return id_; }
    /**
     *  @brief Modifier
     *
     *  Set the starting point.
     *
     *  @param start the starting point
     */
    inline void set_start( int start )
    { start_ = start; }

    /**
     *  @brief Modifier
     *
     *  Set the ending point.
     *
     *  @param end the ending point
     */
    inline void set_end( int end )
    { end_ = end; }

    /**
     *  @brief Set a thread to a CPU
     */
    inline void set()
    {
#ifdef __linux
    	if ( force_cpu_affinity_ == true)
    	{    
		   // Determine the actual number of processors
		   int NUM_PROCS = sysconf(_SC_NPROCESSORS_CONF);
		   cpu_set_t mask;
		   /* CPU_ZERO initializes all the bits in the mask to zero. */
		   CPU_ZERO( &mask );
		   /* CPU_SET sets only the bit corresponding to cpu. */
		   CPU_SET( id_%NUM_PROCS, &mask );
		   /* sched_setaffinity returns 0 in success */
		   if( sched_setaffinity( 0, sizeof(mask), &mask ) == -1 )
		     fprintf(stdout,"WARNING: Could not set CPU Affinity, continuing...\n");
        }
#endif  
    }
};


/**
 *  @class MultiThread
 *
 * 	@brief a @c MultiThread is used to run functions
 * 	by using multithreading, i.e. by using several CPU.
 * 	It launch one @c Thread per available CPU.
 *  It needs the number of threads and the number of elements to scatter.
 *  It also needs 2 pointers on object.
 *  @c MultiThread will create an array of @c Thread
 */
class MultiThread
{
  private:
    /**< Number of threads */
    int nb_thread_;
    /**< Number of elements to scatter */
    int nb_element_;
    /**< array of Thread */
    Thread * thread_;

  public:
    /**
     * 	@brief Constructor
     *
     *	Allocate memory for a @c MultiThread object.
     *
     *  @param n_thread the number of thread wanted
     *  @param thread_per_cpu the number of thread per CPU
     *  @param force_cpu_affinity boolean to force CPU affinity
     *  @param n_element the number of variables to analyze
     *  @param that pointer on object where the thread is called
     *  @param object pointer on object on wich apply the thread
     */
    MultiThread( const int n_thread
               , const int thread_per_cpu
               , bool const force_cpu_affinity
               , const int n_element
               , void* that
               , void* object
               );

    /**
     *  @brief Constructor
     *
     *  Allocate memory for a @c MultiThread object.
     *
     *  @param n_thread the number of thread wanted
     *  @param thread_per_cpu the number of thread per CPU
     *  @param force_cpu_affinity boolean to force CPU affinity
     *  @param n_element the number of variables to analyze
     *  @param that pointer on object where the thread is called
     *  @param object pointer on object on which apply the thread
     *  @param object2 pointer on a secondary object on which apply the thread
     */
    MultiThread( const int n_thread
               , const int thread_per_cpu
               , bool const force_cpu_affinity
               , const int n_element
               , void* that
               , void* object
               , void* object2
               );

    /**
     * 	@brief Destructor
     *
     *	Release a @c MultiThread.
     */
    ~MultiThread();

    /**
     *  @brief Accessor
     *
     *  Get the number of threads.
     *
     *  @return the number of threads
     */
    inline const int nb_thread() const
    {  return nb_thread_;  }

    /**
     *  @brief Accessor
     *
     *  Compute the optimal number of threads
     *  and return its.
     *
     *  @param n_thread the number of thread wanted
     *  @param thread_per_cpu the number of thread per CPU
     *  @param n_element the number of variables to analyze
     *  @return the optimal number of threads
     */
    static int optimal_nb_thread( const int n_thread
                                , const int thread_per_cpu
                                , const int n_element
                                );

    /**
     *  @brief Launch a function with threads
     *
     *  @param run the function to run within the threads
     */
    void launch( void* (*run)(void*) );

    /**
     *  @brief Launch an equally spaced computation in multithreading
     *
     *  @param run the function to run within the threads
     */
    void linear( void* (*run)(void*) );

    /**
     *  @brief Launch an equally spaced computation
     *  in multithreading with pre-defined starting & ending values
     *
     *  @param run the function to run within the threads
     *  @param start the starting value
     *  @param end the ending value
     */
    void linear( void* (*run)(void*), const int start, const int end );
};

/** @} */

#endif /* THREAD_H_ */
