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

#ifndef __ALL_PMACROS__
#define __ALL_PMACROS__
#include <stdio.h>

#define ANSI_COLOR_RED "\x1b[31m" 
#define ANSI_COLOR_GREEN "\x1b[32m" 
#define ANSI_COLOR_YELLOW "\x1b[33m" 
#define ANSI_COLOR_BLUE "\x1b[34m" 
#define ANSI_COLOR_MAGENTA "\x1b[35m" 
#define ANSI_COLOR_CYAN "\x1b[36m" 
#define ANSI_COLOR_RESET "\x1b[0m"

#define RUN_COLOR ANSI_COLOR_GREEN
#define OP_COLOR ANSI_COLOR_YELLOW

#define BUFFER_SIZE 256
#define MOUNT_BIND_OP 0xa
#define MOUNT_IMAGE_OP 0xf
#define FS_EXT4 0x00fe2
#define FS_VFAT 0x00ef3
#define FS_EXT4_STR "ext4"
#define FS_VFAT_STR "vfat"
//#define FS_NTF
// Must be Modified!
#define FOLDER_MODE_T 0764
#define FS_VFAT_DEFAULT_OPTION "rw,dirsync,nosuid,nodev,exec,atime,diratime,uid=1000,gid=1023,fmask=0002,dmask=0002,allow_utime=0020"
#define FS_EXT4_DEFAULT_OPTION "rw,suid,dev,relatime"
////^^^^^^////
#define ONE_KBYTES 1024
#define ONE_MBYTES (ONE_KBYTES * ONE_KBYTES)
#define ONE_GBYTES (ONE_MBYTES * ONE_KBYTES)
#define BLKSIZE_DEFAULT 4096
#define BLKSIZE_8K 8192
#define FILEORDIR_EXISTS 0x000e2
#define CANT_OPENFILE 0x000c2
#define SUCCESS 0x0
#define NOT_EXISTS 0x00ae

#define HEADERS "FMountDataList-DONOTDELETE! @APW\n"
#define DEFAULT_LABEL "FImages"
#define PATH_CONF "/sdcard/.fmountdata.list"
// warning runtime
#define WARNING_COLOR ANSI_COLOR_MAGENTA
// errors runtime
#define SUCCESS_STR NULL
#define ERROR_HEADER "Missing Value in"
#define ERROR_FOOTER "option! Consider try option '--help' for more information!"
#define ERROR_COLORS ANSI_COLOR_YELLOW
#define ERROR_SOURCE_IS_NULL ERROR_COLORS ERROR_HEADER " '--source' " ERROR_FOOTER ANSI_COLOR_RESET
#define ERROR_TARGET_IS_NULL ERROR_COLORS ERROR_HEADER " '--target' " ERROR_FOOTER ANSI_COLOR_RESET
#define ERROR_MOUNT_MODES_NOT_RECOGNIZED ERROR_COLORS "Missing '--bind' or '--fs-mount' " ERROR_FOOTER ANSI_COLOR_RESET
#define ERROR_FSTYPE_NOT_RECOGNIZED ERROR_COLORS ERROR_HEADER " '--fs-type' " ERROR_FOOTER ANSI_COLOR_RESET
#define ERROR_SIZE_IS_ZERO ERROR_COLORS ERROR_HEADER " '--size' " ERROR_FOOTER ANSI_COLOR_RESET
#define ERROR_LIST_IS_NULL ERROR_COLORS ERROR_HEADER " '--list' " ERROR_FOOTER ANSI_COLOR_RESET
#define ERROR_MODE_NOT_RECOGNIZED ERROR_COLORS "Missing " ERROR_FOOTER ANSI_COLOR_RESET
#define ERROR_COMMAND_PROHIBITED ERROR_COLORS "Operation not permitted! Consider try option '--help' for usages!" ANSI_COLOR_RESET
#define ERR(errors) \
	fprintf(stderr, ANSI_COLOR_RED "Error : " ANSI_COLOR_RESET "%s\n" , errors);
/////////////////////	
#endif // __ALL_PMACROS__
