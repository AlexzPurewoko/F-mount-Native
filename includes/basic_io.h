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

#ifndef __BASIC_IO_H__
#define __BASIC_IO_H__
#include <stdio.h>
//#include <types.h>
#ifndef __ALL_PMACROS__
#include "all_macros.h"
#endif // __ALL_PMACROS__
/* 
 * struct for carry a mount parameters at least [BUFFER_SIZE * 3 + 4 + 8 + 2] bytes memory used
 */
typedef struct _fmountdata_ {
	// locate a directory source or path into image devices
	char __dirsource[BUFFER_SIZE];
	// locate a directory target to mount
	char __dirtarget[BUFFER_SIZE];
	// extra mount option flags
	// such as fmask, dmask and other
	char __extra_opt[BUFFER_SIZE];
	// determine a filesystem type
	// like ntfs, vfat, exfat and other supported by system itself
	// if no support fs, and if the system supported it 
	char __fs_type_other[16];
	// for supported fs included in application
	int __fs_declared_type;
	// determine an operation type, like MS_BIND, or other
	short __op_type;
	// determine a mount flags, flags can be sets more than one separated by pipe(|)
	unsigned long __flags;
}FData;

/*
 * For carrying a file, must be defined! Currently not used
 */
 
typedef struct __file_store {
	// Current file data
	FILE *__fdata;
	// Current file name
	char __fname[BUFFER_SIZE];
	// Offset pointed from the first operation
	// Ignores the header
	off_t __first_offset;
}FStore;

/**
 * For build new image devices, not used in other operation
 **/
typedef struct __BuildDevices {
	// The fullpath to the new images file
	char __dname[BUFFER_SIZE];
	// label for new devices
	char __dlabel[16];
	// block device size, default set to BLKSIZE_DEFAULT
	int __blksize;
	// Store the device size in bytes
	off64_t __dsize;
	// Store the type of filesystem to the new blockdevices
	int __fs_type;
}BDev;
// for buffer any operation

/*
 * Create and open the FData File based on path. Stored in the FStore data structure
 * @param path The absolute path
 * @return FStore data Structure
 */
FStore *__fdata_openf(const char *path); //
/*
 * Seek the file into the first position except the headers
 * @param f The FStore Data Structure
 * @noreturn
 */
void __setToFirstOp(FStore *f); //
/*
 * Seek the file into the first position.
 * @param f The FStore Data Structure
 * @noreturn
 */
void __setToFirst(FStore *f); //
/*
 * Seek the file into the end position.
 * @param f The FStore Data Structure
 * @noreturn
 */
void __setToEnd(FStore *f); //
/*
 * Gets the list length from a file
 * @param f The FStore Data Structure
 * @return size of a list
 */
size_t __fdataget_length(FStore *f); //
/*
 * read the fdata with specified position
 * @param f The FStore Data Structure
 * @param fd The FData Data Structure for data carry
 * @param pos Position to be read
 * @return SUCCESS if operation finished normally
 */
int __fdataread_atpos(FStore *f, FData *fd, size_t pos); //
/*
 * Read the file fdata with iteration feature
 * @param f The pointer to FStore Data Structure
 * @param fd the pointer to FData Data Structure for carry data
 * @return <int> SUCCESS while non-zero value returned
 */
int __fdataread_iter(FStore *f, FData *fd); //
/*
 * Remove the data from the list with the start position into the end position
 * start position from 0 and end position can be length - 1
 * @param f The pointer to FStore Data Structure
 * @param start Start position
 * @param end End position
 * @noreturn
 */
void __fdataremove(FStore *f, size_t start, size_t end);
/*
 * Write the data stored in {@link f} into the list that have been included in file that has stored into {@link rd}
 * @param f The pointer to FStore Data Structure
 * @param fd the pointer to FData Data Structure for carry data
 * @noreturn
 */
void __fdatawrite(FStore *f, FData *rd); //
/**
 * This function is used to check the file is exists or not
 * how to use
 * IsFileExists(filename<string>);
 * This method will return 0 if no error occurs if searching the file
 * and the file is exists in directory
 * return 1 if the file isn't exists and the error is occurs if searching the file
 */
int IsFileExists(const char *path); //
/**
 * Create a sparse files(empty file image) with the path and size specified.
 * @param path The absolute path to the file
 * @param size_files Size files to be created with the off64_t format(byte format -> 1K = 1024 bytes, 1M = 1048576 bytes, etc)
 * @return <int> the condition of file, which returned non-zero is fail to be created
 */
int create_sparse_files(const char *path, off64_t size_files); //

/**
 * Check the directory from specified path
 * @param path_to_dirs Path to the Directory 
 * @return <int> while non-zero returned operation fail!
 **/
int CheckDirs(const char *path_to_dirs); //
/**
 * Make a new filesystem, with the option specified in {@see BDev}
 * @param dev_opt The BDev structures
 * @return <int> while non-zero returned operation fail!
 **/
int MakeFS(BDev *dev_opt, short verbose);
// for __fdataread_iter operation
int SetFSType(FData *f, char *fs_name); //
// function used by __fdatawrite
char *GetFSType(FData *f); //
/**
 * Check whether the path is block devices or not
 * @param path The absolute path
 * @return <int> :
 * 		0  if the file isn't exists, regular file or is directory
 * 		1  if the file is block devices
 */
int IsBlockDevices(const char *path);
/**
 * Check whether the path is regular file or not
 * @param path The absolute path
 * @return <int> :
 * 		0  if the path isn't exists, is block devices, or is directory 
 * 		1  if the path is Regular file
 */
int IsRegularFile(const char *path);

/***************************************************************/
/********** mount_op.c ***********/
/**
 * Mount specific data stored in FData structures
 * @param __op_data The Pointer into FData Structures
 * @return <int> while non-zero returned operation fails!
 */
int __mount_operations(FData *__op_data);
int __mount_with_loop_dev(FData *__op_data);
/***************************************************************/

/***************************************************************/
/********** is_mounted.c ***********/
/**
 * Check if the filesystem is already mounted or not
 * @param __op_data The Pointer into FData Structures
 * @return <int> 0 if not mounted and 1 if is mounted
 */
int IsMounted(FData *__op_data);
/***************************************************************/
#endif // __BASIC_IO_H__
