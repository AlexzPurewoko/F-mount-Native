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

#include <linux/ioctl.h>
#include <linux/loop.h>
#include <fcntl.h>
#include <stdio.h>
#include "loop_ctl.h"
#ifndef __STDBOOL_H__
#include "stdbool.h"
#endif // __STDBOOL_H__
/**
 * Get an unused loop block devices
 * @param buffer The buffer is  contains the path into current unused loop block devices
 * @return loop_num The Loop Number if operation done
 */
int GetUnusedLoop(char *buffer){
	// try to open Loop control, if < 0 indicates not suppport and permission denied!
	int ctl = open(_LOOP_CONTROL_PATH_, O_RDWR);
	// if ctl < 0 permission denied, or kernel is below 3.1
	if(ctl < 0)
		return -1;
	int loop_num = ioctl(ctl, LOOP_CTL_GET_FREE);
	close(ctl);
	// if loop_num < 0 indicates operation Fail!
	if(loop_num < 0)
		return -1;
	if(buffer) sprintf(buffer, _LOOP_DEVICE_PATH_, loop_num);
	return loop_num;
}
/**
 * Fill a loop device with raw image to be mounted
 * @param loop_dev The absolute path into the loop devices
 * @param raw_image_path Raw image absolute path to load into loop devices
 * @return SUCCESS if operation done
 */
int FillLoopDev(const char *loop_dev, const char *raw_image_path){
	if(!loop_dev || !raw_image_path) return !SUCCESS;
	// open a loop devices
	int loop = open(loop_dev, O_RDWR);
	// if loop < 0 ? Fail!
	if(loop < 0)
		return !SUCCESS;
	// open a raw image
	int raw_image_fd = open(raw_image_path, O_RDWR);
	// fail if lt zero
	if(raw_image_fd < 0){
		close(loop);
		return !SUCCESS;
	}
	// operate ioctl, LOOP_SET_FD
	int res = ioctl(loop, LOOP_SET_FD, raw_image_fd);
	//close and indicate the return value
	close(loop);
	close(raw_image_fd);
	return res;
	
}

/**
 * Set a loop file name
 * @param loop_dev The absolute path into the loop devices
 * @param new_loop_names The Raw image absolute path that has been loaded
 * @return SUCCESS if operation done
 */
int SetLoopFileName(const char *loop_dev, const char *new_loop_names){
	// basic checking
	if(!loop_dev || !new_loop_names)return !SUCCESS;
	// open a loop devices
	int loop = open(loop_dev, O_RDWR);
	// fail if lt zero
	if(loop < 0 )
		return !SUCCESS;
	//declare a loop_info64
	struct loop_info64 __loopinfo;
	// get the value of loop_info64
	ioctl(loop, LOOP_GET_STATUS64, &__loopinfo);
	// basic checking for loop_info64
	if(__loopinfo.lo_device == 0 && __loopinfo.lo_number == 0){
		close(loop);
		return !SUCCESS;
	}
	// copy strings, close and return
	strcpy(__loopinfo.lo_file_name, new_loop_names);
	int res = ioctl(loop, LOOP_SET_STATUS64, &__loopinfo);
	close(loop);
	return res >= 0 ? SUCCESS: !SUCCESS;
		
}
/**
 * Add a new Loop devices
 * @param new_loop_num The number of loop devices to be created
 * @return new_loop_num if success or EEXIST if the loop device is already allocated
 * or returned -1 if another fail!
 */
int NewLoopBlockDev(long int new_loop_num){
	// basic checking
	if(new_loop_num < 0) return -1;
	// open loop control
	int lctl = open(_LOOP_CONTROL_PATH_, O_RDWR);
	// if lt 0 ? return -1
	if(lctl < 0) return -1;
	// add!
	int res = ioctl(lctl, LOOP_CTL_ADD, new_loop_num);
	close(lctl);
	return res;
	
}

/**
 * Remove a loop device from specified loop numbers
 * @param loop_num The number of loop devices to be removed
 * @return loop_num if success or EBUSY if the loop device is busy
 * or returned -1 if another fail!
 */
int RemoveLoopDev(long int loop_num){
	// basic checking
	if(loop_num < 0) return -1;
	// open loop control
	int lctl = open(_LOOP_CONTROL_PATH_, O_RDWR);
	// if lt 0 ? return -1
	if(lctl < 0) return -1;
	// add!
	int res = ioctl(lctl, LOOP_CTL_REMOVE, loop_num);
	close(lctl);
	return res;
}
/**
 * Removes a descriptor of raw image from specified path to loop devices
 * @param loop_dev The absolute path into the loop devices
 * @return res The result
 */
int ClearLoopContent(const char *loop_dev){
	// if a loop dev? 
	//if(!(strcmp(loop_dev, _LOOP_DEVICE_PATH_) > 0))return !SUCCESS;
	int loop = open(loop_dev, O_RDWR);
	int res = ioctl(loop, LOOP_CLR_FD);
	close(loop);
	return res;
}
/**
 * Change the descriptor of the raw image in specified loop devices into
 * the new descriptor of new raw image
 * @param loop_dev The absolute path into the loop devices
 * @param raw_image_path Raw image absolute path to load into loop devices
 * @return SUCCESS if operation done
 */
int ChangeLoopContent(const char *loop_dev, const char *new_raw_image){
	if(!loop_dev || !new_raw_image)return !SUCCESS;
	// open a loop dev
	int loop = open(loop_dev, O_RDWR);
	// if lt zero return.
	if(loop < 0) return !SUCCESS;
	int rawImage = open(new_raw_image, O_RDWR);
	// if lt zero return.
	if(rawImage < 0){close(loop); return !SUCCESS;}
	int res = ioctl(loop, LOOP_CHANGE_FD, rawImage);
	close(rawImage);
	close(loop);
	return res < 0 ? !SUCCESS : SUCCESS;
}
void ClrLoopStatus(struct loop_info *__loop_info){
	__loop_info -> lo_number = 0;
	__loop_info -> lo_device = __loop_info -> lo_rdevice = 0;
	__loop_info -> lo_inode = 0;
	__loop_info -> lo_offset = __loop_info -> lo_encrypt_type = __loop_info -> lo_encrypt_key_size = __loop_info -> lo_flags = 0;
	__loop_info -> lo_name[0] = '\0';
	__loop_info -> reserved[0] = '\0';
	__loop_info -> lo_init[0] = __loop_info -> lo_init[1] = 0;
__loop_info -> lo_encrypt_key[0] = '\0';
}
void ClrLoopStatus64(struct loop_info64 *__loop_info){
	__loop_info -> lo_device =	__loop_info -> lo_inode = 	__loop_info -> lo_rdevice = __loop_info -> lo_offset = __loop_info -> lo_sizelimit = 0;
 __loop_info -> lo_number= __loop_info -> lo_encrypt_type= __loop_info -> lo_encrypt_key_size =__loop_info -> lo_flags = 0;
 __loop_info -> lo_file_name[0] = '\0';
 __loop_info -> lo_crypt_name[0] = '\0';
 __loop_info -> lo_encrypt_key[0] = '\0';
 __loop_info -> lo_init[0] = __loop_info -> lo_init[1] = 0;
}
/**
 * Get a loop devices states 
 * @param loop_dev The absolute path into the loop devices
 * @param __loop_info The pointer to the loop_info structures
 * @return SUCCESS if operation done
 */
int GetLoopStatus(const char *loop_dev, struct loop_info *__loop_info){
	if(!loop_dev || !__loop_info) return !SUCCESS;
	ClrLoopStatus(__loop_info);
	int loop = open(loop_dev, O_RDWR);
	if(loop < 0)return !SUCCESS;
	int res = ioctl(loop, LOOP_GET_STATUS, __loop_info);
	close(loop);
	return res < 0 ? !SUCCESS:SUCCESS;
}
/**
 * Get a loop devices states 
 * @param loop_dev The absolute path into the loop devices
 * @param __loop_info The pointer to the loop_info64 structures
 * @return SUCCESS if operation done
 */
int GetLoopStatus64(const char *loop_dev, struct loop_info64 *__loop_info){
	if(!loop_dev || !__loop_info) return !SUCCESS;
	ClrLoopStatus64(__loop_info);
	int loop = open(loop_dev, O_RDWR);
	if(loop < 0)return !SUCCESS;
	int res = ioctl(loop, LOOP_GET_STATUS64,__loop_info);
	close(loop);
	return res < 0 ? !SUCCESS:SUCCESS;
}

/**
 * Gets the total of loop available count
 * @return <int> The total of available loop
 **/
int GetTotalAvailableLoop(){
	char buf[32];
	sprintf(buf, _LOOP_DEVICE_PATH_, 0);
	// try to open loop0
	int fd = open(buf, O_RDWR);
	if(fd < 0)return -1;
	int count = 0;
	close(fd);
	while(true){
		sprintf(buf, _LOOP_DEVICE_PATH_, count);
		fd = open(buf, O_RDWR);
		if(fd < 0)return count;
		close(fd);
		count++;
	}
	return 0;
}
