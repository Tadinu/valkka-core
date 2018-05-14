#ifndef framefifo_HEADER_GUARD
#define framefifo_HEADER_GUARD
/*
 * framefifo.h : Thread safe system of fifo and a stack
 * 
 * Copyright 2017, 2018 Valkka Security Ltd. and Sampsa Riikonen.
 * 
 * Authors: Sampsa Riikonen <sampsa.riikonen@iki.fi>
 * 
 * This file is part of the Valkka library.
 * 
 * Valkka is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>
 *
 */

/** 
 *  @file    framefifo.h
 *  @author  Sampsa Riikonen
 *  @date    2017
 *  @version 0.4.0 
 *  
 *  @brief   Thread safe system of fifo and a stack
 */ 

#include "common.h"
#include "frame.h"
#include "logging.h"

/** Describes the stack structure and fifo behaviour for a FrameFifo
 * 
 * @ingroup queues_tag
 */
struct FrameFifoContext {                                                                                                                                       // <pyapi>
  FrameFifoContext() : n_basic(20), n_avpkt(0), n_avframe(0), n_yuvpbo(0), n_setup(20), n_signal(20), flush_when_full(DEFAULT_FRAMEFIFO_FLUSH_WHEN_FULL) {}     // <pyapi>
  FrameFifoContext(int n_basic, int n_avpkt, int n_avframe, int n_yuvpbo, int n_setup, int n_signal, bool flush_when_full) :                                    // <pyapi>
  n_basic(n_basic), n_avpkt(n_avpkt), n_avframe(n_avframe), n_yuvpbo(n_yuvpbo), n_setup(n_setup), n_signal(n_signal), flush_when_full(flush_when_full) {}       // <pyapi>
  int n_basic;     ///< data at payload                                                                                                                         // <pyapi>
  int n_avpkt;     ///< data at ffmpeg avpkt                                                                                                                    // <pyapi>
  int n_avframe;   ///< data at ffmpeg av_frame and ffmpeg av_codec_context                                                                                     // <pyapi>
  int n_yuvpbo;    ///< data at yuvpbo struct                                                                                                                   // <pyapi>
  int n_setup;     ///< setup data                                                                                                                              // <pyapi>
  int n_signal;    ///< signal to AVThread or OpenGLThread                                                                                                      // <pyapi>
  bool flush_when_full; ///< Flush when filled                                                                                                                  // <pyapi>
};                                                                                                                                                              // <pyapi>


/** A thread-safe combination of a fifo (first-in-first-out) queue and an associated stack.
 * 
 * Frame instances are placed into FrameFifo with FrameFifo::writeCopy that draws a Frame from the stack and performs a copy of the frame.
 * 
 * If no frames are available, an "overflow" occurs.  The behaviour at overflow event can be defined (see FrameFifoContext).
 * 
 * When Frame has been used, it should be returned to the FrameFifo by calling FrameFifo::recycle.  This returns the Frame to the stack.
 * 
 * @ingroup queues_tag
 */
class FrameFifo {                                                                                   // <pyapi>

public:                                                                                             // <pyapi>
  FrameFifo(const char *name, FrameFifoContext ctx =FrameFifoContext()); ///< Default ctor          // <pyapi>
  virtual ~FrameFifo();                                                  ///< Default virtual dtor  // <pyapi>

protected:
  std::string      name;
  FrameFifoContext ctx;   ///< Parameters defining the stack and overflow behaviour
  
protected: // reservoir, stack & fifo queue
  std::map<FrameClass,Reservoir>  reservoirs;   ///< The actual frames
  std::map<FrameClass,Stack>      stacks;       ///< Pointers to the actual frames, sorted by FrameClass
  Fifo                            fifo;         ///< The fifo queue
  
protected: // mutex synchro
  std::mutex mutex;                         ///< The Lock
  std::condition_variable condition;        ///< The Event/Flag
  std::condition_variable ready_condition;  ///< The Event/Flag for FrameFifo::ready_mutex
    
protected:
  virtual void recycle_(Frame* f);                        ///< Return Frame f back into the stack.  Update target_size if necessary
  
public:
  virtual bool writeCopy(Frame* f, bool wait=false);     ///< Take a frame "ftmp" from the stack, copy contents of "f" into "ftmp" and insert "ftmp" into the beginning of the fifo (i.e. perform "copy-on-insert").  The size of "ftmp" is also checked and set to target_size, if necessary.  If wait is set to true, will wait until there are frames available in the stack.
  virtual Frame* read(unsigned short int mstimeout=0);   ///< Pop a frame from the end of the fifo and return the frame to the reservoir stack
  virtual void recycle(Frame* f);                        ///< Like FrameFifo::recycle_ but with mutex protection
  virtual void recycleAll();                             ///< Recycle all frames from fifo back to stack (make a "flush")
  virtual void dumpStacks();    ///< Dump frames in the stacks
  virtual void dumpFifo();      ///< Dump frames in the fifo
  virtual void diagnosis();     ///< Print a resumen of fifo and stack usage
  bool isEmpty();               ///< Tell if fifo is empty
};                                                                      // <pyapi>


#endif
