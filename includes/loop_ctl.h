/*
 * Copyright (c) 2018 Alexzander Purwoko Widiantoro <purwoko908@gmail.com>
 *
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __LOOP_CTL_H__
#define __LOOP_CTL_H__

#include <linux/ioctl.h>
#include <linux/loop.h>
// since kernel 3.1, function has been added, but occasionally we need to define it
#ifndef LOOP_CTL_ADD
# define LOOP_CTL_ADD		0x4C80
# define LOOP_CTL_REMOVE	0x4C81
# define LOOP_CTL_GET_FREE	0x4C82
#endif //LOOP_CTL_ADD

#ifndef __ALL_PMACROS__
#include "all_macros.h"
#endif // __ALL_PMACROS__

// Since kernel 3.1, kernel has been add "/dev/loop-control" to perform any important function
#define _LOOP_CONTROL_PATH_ "/dev/loop-control"
// You can specified the _LOOP_DEVICE_PATH_ macros refer to the loop path
#define _LOOP_DEVICE_PATH_ "/dev/block/loop%d"
/**
 * Get an unused loop block devices
 * @param buffer The buffer is  contains the path into current unused loop block devices
 * @return loop_num The Loop Number if operation done
 */
int GetUnusedLoop(char *buffer);
/**
 * Fill a loop device with raw image to be mounted
 * @param loop_dev The absolute path into the loop devices
 * @param raw_image_path Raw image absolute path to load into loop devices
 * @return SUCCESS if operation done
 */
int FillLoopDev(const char *loop_dev, const char *raw_image_path);
/**
 * Set a loop file name
 * @param loop_dev The absolute path into the loop devices
 * @param new_loop_names The Raw image absolute path that has been loaded
 * @return SUCCESS if operation done
 */
int SetLoopFileName(const char *loop_dev, const char *new_loop_names);
/**
 * Add a new Loop devices
 * @param new_loop_num The number of loop devices to be created
 * @return new_loop_num if success or EEXIST if the loop device is already allocated
 * or returned -1 if another fail!
 */
int NewLoopBlockDev(long int new_loop_num);
/**
 * Remove a loop device from specified loop numbers
 * @param loop_num The number of loop devices to be removed
 * @return loop_num if success or EBUSY if the loop device is busy
 * or returned -1 if another fail!
 */
int RemoveLoopDev(long int loop_num);
/**
 * Removes a descriptor of raw image from specified path to loop devices
 * @param loop_dev The absolute path into the loop devices
 * @return res the Result
 */
int ClearLoopContent(const char *loop_dev);
/**
 * Change the descriptor of the raw image in specified loop devices into
 * the new descriptor of new raw image
 * @param loop_dev The absolute path into the loop devices
 * @param raw_image_path Raw image absolute path to load into loop devices
 * @return SUCCESS if operation done
 */
int ChangeLoopContent(const char *loop_dev, const char *new_raw_image);
/**
 * Get a loop devices states 
 * @param loop_dev The absolute path into the loop devices
 * @param __loop_info The pointer to the loop_info structures
 * @return SUCCESS if operation done
 */
int GetLoopStatus(const char *loop_dev, struct loop_info *__loop_info);
/**
 * Get a loop devices states 
 * @param loop_dev The absolute path into the loop devices
 * @param __loop_info The pointer to the loop_info64 structures
 * @return SUCCESS if operation done
 */
int GetLoopStatus64(const char *loop_dev, struct loop_info64 *__loop_info);
/**
 * Gets the total of loop available count
 * @return <int> The total of available loop
 **/
int GetTotalAvailableLoop();
#endif //__LOOP_CTL_H__
