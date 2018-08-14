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

#include <stdio.h>
//#include <types.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#ifndef __ALL_PMACROS__
#include "all_macros.h"
#endif // __ALL_PMACROS__

#ifndef __BASIC_IO_H__
#include "basic_io.h"
#endif // __BASIC_IO_H__

#ifndef __P_BUFFERS__
#include "p_buffer.h"
#endif // __P_BUFFERS__

#ifndef __STDBOOL_H__
#include "stdbool.h"
#endif // __STDBOOL_H__

#include <dirent.h>
#include "../fs/fs.h"
/*
 * Create and open the FData File based on path. Stored in the FStore data structure
 * @param path The absolute path
 * @return FStore data Structure
 */
FStore *__fdata_openf(const char *path){
	// check the file, not exists create the file
	if(IsFileExists(path)){
		// create file first
		creat(path, 0777);
		// open file
		int fd = open(path, O_RDWR);
		if(fd < 0)
			return NULL;
		// seek to 0 index
		lseek(fd, 0, SEEK_SET);
		// write headers into file
		write(fd, HEADERS, strlen(HEADERS));
		// close file
		close(fd);
		// recursive call function itself
		__fdata_openf(path);
	}
	// open file
	FILE *__res = fopen(path, "r+");
	if(!__res)return NULL;
	// seeking into after headers
	off_t offset = strlen(HEADERS);
	fseek(__res, offset, SEEK_SET);
	// build new FStore
	FStore *fs = (FStore *)malloc(sizeof(FStore));
	fs -> __fdata = __res;
	strcpy(fs -> __fname, path);
	fs -> __first_offset = offset;
	return fs;
}
/*
 * Seek the file into the first position except the headers
 * @param f The FStore Data Structure
 * @noreturn
 */
void __setToFirstOp(FStore *f){
	// seeking into the first operation except the headers
	fseek(f -> __fdata, f -> __first_offset, SEEK_SET);
}
/*
 * Seek the file into the first position.
 * @param f The FStore Data Structure
 * @noreturn
 */
void __setToFirst(FStore *f){
	// seek to first index
	fseek(f -> __fdata, 0, SEEK_SET);
}
/*
 * Seek the file into the end position.
 * @param f The FStore Data Structure
 * @noreturn
 */
void __setToEnd(FStore *f){
	// seek into end
	fseek(f -> __fdata, 0, SEEK_END);
}
/*
 * Gets the list length from a file
 * @param f The FStore Data Structure
 * @return size of a list
 */
size_t __fdataget_length(FStore *f){
	if(!f)return 0;
	// set To first operation
	__setToFirstOp(f);
	// initialize vars
	int x = 0, count = 0;
	// counting a [ chars
	while((x = getc(f -> __fdata)) != -1){
		if(x == '[') count++;
	}
	// set To first operation
	__setToFirstOp(f);
	return count;
}
// function used by __fdatawrite
char *GetFSType(FData *f){
	switch(f -> __fs_declared_type){
		case FS_EXT4:
			return FS_EXT4_STR;
		case FS_VFAT:
			return FS_VFAT_STR;
		default:
			return f -> __fs_type_other;
	}
}
/*
 * Write the data stored in {@link f} into the list that have been included in file that has stored into {@link rd}
 * @param f The pointer to FStore Data Structure
 * @param fd the pointer to FData Data Structure for carry data
 * @noreturn
 */
void __fdatawrite(FStore *f, FData *rd){
	if(!f || !rd)return;
	// dereference file from FStore
	FILE *__file = f -> __fdata;
	size_t len = __fdataget_length(f);
	// seek to end
	__setToEnd(f);
	/**
	 * Data structures
	 * if not mount bind
	 * [Mount_type|Mount_flags|Extra_option|FS_type|DIRSOURCE|DIRTARGET]
	 * if mount bind
	 * [Mount_type|Mount_flags|dirsource|dirtarget]
	 **/
	 if(len)
		 fprintf(__file, "\n");
	 // seek to end
	 __setToEnd(f);
	 // switching mount operation
	 switch(rd -> __op_type){
		case MOUNT_BIND_OP:
			// [Mount_type|Mount_flags|DIRSOURCE|DIRTARGET]
			fprintf(f -> __fdata,"[BIND|%lu|%s|%s]",
								rd -> __flags,
								rd -> __dirsource,
								rd -> __dirtarget
					);
			break;
		case MOUNT_IMAGE_OP:
		{
			char *__fstype = GetFSType(rd);
			if(rd -> __extra_opt[0] == '\0')
				if(!strcmp(__fstype, FS_VFAT_STR)) strcpy(rd -> __extra_opt, FS_VFAT_DEFAULT_OPTION);
				else strcpy(rd -> __extra_opt, FS_EXT4_DEFAULT_OPTION);
			// [Mount_type|Mount_flags|Extra_option|FS_type|DIRSOURCE|DIRTARGET]
			fprintf(f -> __fdata,"[DEVS|%lu|%s|",
								rd -> __flags,
								rd -> __extra_opt
					);
			__setToEnd(f);
			fprintf(f -> __fdata, "%s|%s|%s]",
								__fstype,
								rd -> __dirsource,
								rd -> __dirtarget
					);
		}
			break;
	 }
	 __setToFirstOp(f);
	 return;
}
// for __fdataread_iter operation
int SetFSType(FData *f, char *fs_name){
	if(!f || !fs_name) return -1;
	if(!strcmp(fs_name, FS_EXT4_STR))
		return FS_EXT4;
	else if(!strcmp(fs_name, FS_VFAT_STR))
		return FS_VFAT;
	else 
		return 0;
}
/*
 * Read the file fdata with iteration feature
 * @param f The pointer to FStore Data Structure
 * @param fd the pointer to FData Data Structure for carry data
 * @return <int> SUCCESS while non-zero value returned
 */
int __fdataread_iter(FStore *f, FData *fd){
	if(!f || !fd) return !SUCCESS;
	FILE *__f = f -> __fdata;
	int read;
	int pos;
	int x;
	int op = 0;
	char *_buf;
	while((read = getc(__f)) != -1 && read != '\n'){
		// [Mount_type|Mount_flags|DIRSOURCE|DIRTARGET]
		// device images
		// [Mount_type|Mount_flags|Extra_option|FS_type|DIRSOURCE|DIRTARGET]
		if(read == '['){
			pos = 0;
			_buf = fd -> __extra_opt;
			x = 0;
			continue;
		}
		if(read == '|'){
			switch(pos){
				// mount_type
				case 0:
					_buf[x++] = '\0';
					if(!strcmp(_buf, "BIND"))fd -> __op_type = MOUNT_BIND_OP;
					else if(!strcmp(_buf, "DEVS"))fd -> __op_type = MOUNT_IMAGE_OP;
					else return !SUCCESS;
					break;
				// mount_flags
				case 1:
					_buf[x++] = '\0';
					fd -> __flags = strtoul(_buf, (char **) NULL, 0);	
					// skipping fs type and extra option if operation is MS_BIND
					if(fd -> __op_type == MOUNT_BIND_OP){
						_buf = fd -> __dirsource;
						pos+=2;
					}
					break;
				// extra_option
				case 2:
					_buf[x++] = '\0';
					_buf = fd -> __dirsource;
					break;
				// fs_type
				case 3:
					_buf[x++] = '\0';
					int fs_tp = SetFSType(fd, _buf);
					if(fs_tp) fd -> __fs_declared_type = fs_tp;
					else strcpy(fd -> __fs_type_other, _buf);
					break;
				// dirsource or path into device images
				case 4:
					_buf[x++] = '\0';
					_buf = fd -> __dirtarget;
					break;
			}
			pos++;
			x = 0;
			continue;
		}
		if(read == ']'){
			_buf[x++] = '\0';
			continue;
		}
		_buf[x++] = read;
		op++;
	}
	return (!op)?!SUCCESS:SUCCESS;
}
/*
 * read the fdata with specified position
 * @param f The FStore Data Structure
 * @param fd The FData Data Structure for data carry
 * @param pos Position to be read
 * @return SUCCESS if operation finished normally
 */
int __fdataread_atpos(FStore *f, FData *fd, size_t index){
	if(!f || !fd)return !SUCCESS;
	// get the length of the list
	size_t len = __fdataget_length(f);
	// if pos is greater than length, break operation
	if(index >= len)return !SUCCESS;
	int read;
	fd -> __op_type = 0;
	int x = 0, y = 0, pos = 0;
	__setToFirstOp(f);
	char *_buf;
	while((read = getc(f -> __fdata)) != -1){
		if(read == '\n'){
			if(++y > index)break;
			continue;
		}
		if(index == y){
			// [Mount_type|Mount_flags|Extra_option|DIRSOURCE|DIRTARGET]
			// device images
			// [Mount_type|Mount_flags|Extra_option|FS_type|DIRSOURCE|DIRTARGET]
			if(read == '['){
				pos = 0;
				_buf = fd -> __extra_opt;
				x = 0;
				continue;
			}
			if(read == '|'){
				switch(pos){
					// mount_type
					case 0:
						_buf[x++] = '\0';
						if(!strcmp(_buf, "BIND"))fd -> __op_type = MOUNT_BIND_OP;
						else if(!strcmp(_buf, "DEVS"))fd -> __op_type = MOUNT_IMAGE_OP;
						else return !SUCCESS;
						break;
					// mount_flags
					case 1:
						_buf[x++] = '\0';
						fd -> __flags = strtoul(_buf, (char **) NULL, 0);	
						// skipping fs type and extra option if operation is MS_BIND
						if(fd -> __op_type == MOUNT_BIND_OP){
							_buf = fd -> __dirsource;
							pos+=2;
						}						
						break;
					// extra_option
					case 2:
						_buf[x++] = '\0';
						_buf = fd -> __dirsource;
						
						break;
					// fs_type
					case 3:
						_buf[x++] = '\0';
						int fs_tp = SetFSType(fd, _buf);
						if(fs_tp) fd -> __fs_declared_type = fs_tp;
						else strcpy(fd -> __fs_type_other, _buf);
						break;
					// dirsource or path into device images
					case 4:
						_buf[x++] = '\0';
						_buf = fd -> __dirtarget;
						break;
				}
				pos++;
				x = 0;
				continue;
			}
			if(read == ']'){
				_buf[x++] = '\0';
				continue;
			}
			_buf[x++] = read;
			
		}
	}
	return SUCCESS;
}
/**
 * Check the directory from specified path
 * @param path_to_dirs Path to the Directory 
 * @return <int> while non-zero returned operation fail!
 **/
int CheckDirs(const char *path_to_dirs){
	DIR *dir;
	// try to open a dir, if dir isn't exists return null
	if((dir = opendir(path_to_dirs)) != NULL){
		// close dirs
		closedir(dir);
		return SUCCESS;
	}
	return !SUCCESS;
}
/**
 * This function is used to check the file is exists or not
 * how to use
 * IsFileExists(filename<string>);
 * This method will return 0 if no error occurs if searching the file
 * and the file is exists in directory
 * return 1 if the file isn't exists and the error is occurs if searching the file
 */
int IsFileExists(const char *path){
	struct stat v;
    errno = 0;
    return (stat(path, &v) != 0 && errno == ENOENT) ? 1 : 0;
}
/**
 * Create a sparse files(empty file image) with the path and size specified.
 * @param path The absolute path to the file
 * @param size_files Size files to be created with the off64_t format(byte format -> 1K = 1024 bytes, 1M = 1048576 bytes, etc)
 * @return <int> the condition of file, which returned non-zero is fail to be created
 */
int create_sparse_files(const char *path, off64_t size_files){
	// Exists? cancel operation
	if(!IsFileExists(path))return FILEORDIR_EXISTS;
	// create first file
	creat(path, 0777);
	// open file
	int fd = open(path, O_RDWR);
	// if fd < 0 return operation
	if(fd < 0)
		return CANT_OPENFILE;
	// write sparse file with null character 
	pwrite64(fd, "\0", 1, size_files);
	// close file
	close(fd);
	return SUCCESS;
}
/**
 * Make a new filesystem, with the option specified in {@see BDev}
 * @param dev_opt The BDev structures
 * @return <int> while non-zero returned operation fail!
 **/
int MakeFS(BDev *dev_opt, short verbose){
	// Null? return!
	if(!dev_opt)return !SUCCESS;
	// select the filesystem type
	switch(dev_opt -> __fs_type){
		case FS_VFAT:
			MakeVfatFS(dev_opt -> __dname, dev_opt -> __dlabel, dev_opt -> __dsize, verbose, true);
			break;
		// ext4 fs currently not available
		case FS_EXT4:
			MakeExt4FS(dev_opt -> __dname, dev_opt -> __dlabel, dev_opt -> __blksize, dev_opt -> __dsize, verbose);
			break;
		default:
			return !SUCCESS;
	}
	return SUCCESS;
}
/*
 * Remove the data from the list with the start position into the end position
 * start position from 0 and end position can be length - 1
 * @param f The pointer to FStore Data Structure
 * @param start Start position
 * @param end End position
 * @noreturn
 */
void __fdataremove(FStore *f, size_t start, size_t end){
	if(!f)return;
	int len = __fdataget_length(f);
	if(len == 0 || start > end || end == len)return;
	int _fpos = 0;
	int read;
	__setToFirst(f);
	// gets the offset pointer that pointed to firstOption
	off_t *__firstOp = &(f -> __first_offset);
	sprintf(__p_buffer, "%s.tmp", f -> __fname);
	FILE *tmp = fopen(__p_buffer, "w+");
	FILE *fcurr = f -> __fdata;
	// print the header
	while((read = getc(fcurr)) != '\n')putc(read, tmp);
	putc(read, tmp);
	
	while((read = getc(fcurr)) != -1){
		if(read == '\n'){
			_fpos++;
			//skipping newline if start is positioned from zero
			if(start == 0 && _fpos >= start && _fpos-1 <= end)continue;
			//skipping newline if start is positioned greater than zero and fpos-1 is less than end
			else if(start > 0 && _fpos >= start && _fpos-1 < end)continue;
			// if somewhat else, no skipping and write newline char
			else {
				putc('\n', tmp);
				continue;
			}
		}
		// skipping content from start into end
		if(_fpos >= start &&  _fpos <= end)continue;
		putc(read, tmp);
	}
	fclose(tmp);
	fclose(fcurr);
	rename(__p_buffer, f -> __fname);
	f -> __fdata = fcurr = fopen(f -> __fname, "r+");
	__setToFirstOp(f);
	return;
}
/**
 * Check whether the path is block devices or not
 * @param path The absolute path
 * @return <int> :
 * 		0  if the file isn't exists, regular file or is directory
 * 		1  if the file is block devices
 */
int IsBlockDevices(const char *path){
	struct stat res;
	stat(path, &res);
	if(!errno && S_ISBLK(res.st_mode))return 1;
	return 0;
}
/**
 * Check whether the path is regular file or not
 * @param path The absolute path
 * @return <int> :
 * 		0  if the path isn't exists, is block devices, or is directory 
 * 		1  if the path is Regular file
 */
int IsRegularFile(const char *path){
	struct stat res;
	stat(path, &res);
	if(!errno && S_ISREG(res.st_mode))return 1;
	return 0;
}
