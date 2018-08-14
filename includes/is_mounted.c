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
 #include <fcntl.h>
 #include <sys/stat.h>
 #include <sys/ioctl.h>
 #include <linux/loop.h>
 #include <errno.h>
 #ifndef __BASIC_IO_H__
 #include "basic_io.h"
 #endif /*__BASIC_IO_H__*/
 
 #ifndef __ALL_PMACROS__
#include "all_macros.h"
#endif // __ALL_PMACROS__

#ifndef __P_BUFFERS__
#include "p_buffer.h"
#endif // __P_BUFFERS__
 /**
 * Check if the filesystem is already mounted or not
 * @param __op_data The Pointer into FData Structures
 * @return <int> 0 if not mounted and 1 if is mounted
 */
 
int IsMounted(FData *__op_data){
	if(!__op_data)return 0;
	char *bf = __p_buffer;
	char *s = __op_data -> __dirsource;
	char *t = __op_data -> __dirtarget;
	short *optype = &(__op_data -> __op_type);
	struct stat sbuf, sbuf2;
	switch(*optype){
		case MOUNT_BIND_OP:
		{
			stat(s, &sbuf);
			stat(t, &sbuf2);
			// compare 2 buffers
			if(sbuf.st_dev == sbuf2.st_dev && sbuf.st_ino == sbuf2.st_ino)
				return 1;
			
		}
		return 0;
		case MOUNT_IMAGE_OP:
		{
			// check whether is block devices
			if(IsBlockDevices(s)){
				stat(s, &sbuf);
				stat(t, &sbuf2);
				// compare sbuf.st_rdev and sbuf2.st_dev
				if(sbuf.st_rdev == sbuf2.st_dev)
					return 1;
			}
			else if(IsRegularFile(s)){
				errno = 0;
				stat(s, &sbuf);
				if(errno)
					return 0;
				struct loop_info __loop_info;
				long loop_fd;
				unsigned long loop_pos = 0;
				// compare value lo_device and  in loop 
				while(1){
					sprintf(bf, "/dev/block/loop%lu", loop_pos);
					loop_fd = open(bf, O_RDWR);
					if(loop_fd < 0)
						return 0;
					ioctl(loop_fd, LOOP_GET_STATUS, &__loop_info);
					close(loop_fd);
					if(sbuf.st_dev == __loop_info.lo_device)
						return 1;
					loop_pos++;
				}
			}
		}
		return 0;
		default:
			return 0;
	}
	// 
	
}
