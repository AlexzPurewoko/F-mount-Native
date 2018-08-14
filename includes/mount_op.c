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

#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <string.h>
#ifndef __BASIC_IO_H__
#include "basic_io.h"
#endif // __BASIC_IO_H__
#ifndef __STDBOOL_H__
#include "stdbool.h"
#endif // __STDBOOL_H__

#ifndef __ALL_PMACROS__
#include "all_macros.h"
#endif //__ALL_PMACROS__

#ifndef __P_BUFFERS__
#include "p_buffer.h"
#endif // __P_BUFFERS__

// return SUCCESS if success
int ChkResources(char *s, char *t, short *__op_type){
	// check the sources
	switch(*__op_type){
		case MOUNT_BIND_OP:
			if(CheckDirs(s))return !SUCCESS;
		break;
		case MOUNT_IMAGE_OP:
			if(!(IsBlockDevices(s) || !IsFileExists(s)))return !SUCCESS;
		break;
		default:
			return !SUCCESS;
	}
	// check the targets
	if(!IsRegularFile(t) && !IsBlockDevices(t))
		if(CheckDirs(t))
			if(mkdir(t, FOLDER_MODE_T) == 0) return SUCCESS;
			else return !SUCCESS;
		else return SUCCESS;
	return !SUCCESS;
}
int __mount_operations(FData *__op_data){
	if(!__op_data)return !SUCCESS;
	// declare a pointer to a data in structures
	char *__dirsource = __op_data -> __dirsource;
	char *__dirtarget = __op_data -> __dirtarget;
	char *__extra_opt = __op_data -> __extra_opt;
	char *__fs_type_other = __op_data -> __fs_type_other;
	int *__fs_declared_type = &(__op_data -> __fs_declared_type);
	short *__op_type = &(__op_data -> __op_type);
	unsigned long *__flags = &(__op_data -> __flags);
	
	// Check the directories/files source and check the directory target
	// if target isn't exists, make it!
	if(ChkResources(__dirsource, __dirtarget, __op_type)) return !SUCCESS;
	
	errno = 0;
	switch(*__op_type){
		case MOUNT_BIND_OP:	
			return mount(__dirsource, __dirtarget, NULL, MS_BIND, NULL) ? errno : SUCCESS;
		case MOUNT_IMAGE_OP:
			if(IsBlockDevices(__dirsource)){
				char *opt = __p_buffer;
				sprintf(opt, "busybox mount -t %s -o %s %s %s"
										, GetFSType(__op_data)
										, __extra_opt
										, __dirsource
										, __dirtarget
						);
				return system(opt);
			}
				//return mount(__dirsource, __dirtarget, GetFSType(__op_data), *__flags, (char *) __extra_opt) ? !SUCCESS : SUCCESS;
			else if(IsRegularFile(__dirsource))
				return __mount_with_loop_dev(__op_data);
		default:
			return !SUCCESS;
	}
}
int __mount_with_loop_dev(FData *__op_data){
	if(!__op_data)return !SUCCESS;
	int total_loop = GetTotalAvailableLoop();
	if(!total_loop)return !SUCCESS;
	char loopblk[32];
	int unused_loop_num = GetUnusedLoop(loopblk);
	// if loop num is full
	if(unused_loop_num < 0){
		// try to make new loop device
		NewLoopBlockDev(total_loop);
		unused_loop_num = GetUnusedLoop(loopblk);
		// if fail again
		if(unused_loop_num < 0)return !SUCCESS;
	}
	// Fill the loop devices
	int fill = FillLoopDev(loopblk, __op_data -> __dirsource);
	if(fill) return !SUCCESS;
	// set the loop filename
	SetLoopFileName(loopblk, __op_data -> __dirsource);
	// change the path into loop
	strcpy(__op_data -> __dirsource, loopblk);
	char *opt = __p_buffer;
	// mount !
	sprintf(opt, "busybox mount -t %s -o %s %s %s", GetFSType(__op_data), __op_data -> __extra_opt, __op_data -> __dirsource, __op_data -> __dirtarget);
	//return __mount_operations(__op_data);
	return system(opt);
}
