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
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <linux/loop.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <conio.h>
#include <linux/ioctl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "includes/basic_io.h"
#include "includes/loop_ctl.h"
#include "includes/arrconv.h"

#define SETUP_MODE_ADD 0x0bef
#define SETUP_MODE_REMOVE 0x0fbe
#define SETUP_MODE_NEW_FS 0x0efb
#define SETUP_MODE_MOUNT_ALL 0x0ae
#define SETUP_MODE_LISTS 0x0ea
#define SETUP_MODE_EJECT 0x0bee
#define OP_MODE_ADD 0xfaa
#define OP_MODE_NEW_FS 0xafa
#define OP_MODE_MOUNT_LIST 0xaaf
#define OP_MODE_REMOVE_LIST 0xcaf
#define OP_MODE_EJECT 0xffc
#define VERSION "3.0"
/*
	 * Usage models
	 * fmount --help
	 * fmount --eject-all
	 * fmount --eject-list [,,,,,n]
	 * fmount --verbose --setup-[]
	 * fmount --mount-all --verbose
	 * fmount --list
	 * fmount --verbose --add --source [] --target [] [--bind | --fs-mount] --fs-type [ext4 | vfat] --extra-option []
	 * fmount --mount [,,,n] --verbose
	 * fmount --newfs --verbose --source [] --fs-type [ext4 | vfat] --size [(KBYTES)] --label []
	 * fmount --remove-list [,,,,,,,,n] --verbose 
*/
/****** GLOBAL VARIABLES ******/
	int selected_modes = 0;
	int setup_modes = 0;
	int verbose = 0;
	int mode_op = 0;
	int mount_modes = 0;
	int fstype = 0;
	unsigned long size = 0;
	char *extra_buf;
	static FData fdt;
	static BDev bdv;
	char *source = fdt.__dirsource;
	char *target = fdt.__dirtarget;
	char *label = bdv.__dlabel;
	char *extra_opt = fdt.__extra_opt;
/****** ---------------- ******/
// return non-zero values indicates an error
char *check_vars_and_select_modes(){
	switch(setup_modes){
		case SETUP_MODE_ADD:
		case SETUP_MODE_REMOVE:
		case SETUP_MODE_NEW_FS:
		case SETUP_MODE_MOUNT_ALL:
		case SETUP_MODE_LISTS:
		case SETUP_MODE_EJECT:
			break;
		default:
			switch(mode_op){
				case OP_MODE_ADD:
				{
					if(!source)
						return ERROR_SOURCE_IS_NULL;
					else if(!target)
						return ERROR_TARGET_IS_NULL;
					else if(!mount_modes)
						return ERROR_MOUNT_MODES_NOT_RECOGNIZED;
					else if(!fstype)
						return ERROR_FSTYPE_NOT_RECOGNIZED;
				}
				break;
				case OP_MODE_NEW_FS:
				{
					if(!source)
						return ERROR_SOURCE_IS_NULL;
					else if(!fstype)
						return ERROR_FSTYPE_NOT_RECOGNIZED;
					else if(!size)
						return ERROR_SIZE_IS_ZERO;
					
				}
				break;
				case OP_MODE_MOUNT_LIST:
				case OP_MODE_EJECT:
				case OP_MODE_REMOVE_LIST:
				{
					if(!extra_buf)
						return ERROR_LIST_IS_NULL;
				}
				break;
				default:
					return ERROR_MODE_NOT_RECOGNIZED;
			}
	}
	selected_modes = (setup_modes)?setup_modes:mode_op;
	return SUCCESS_STR;
}
void set_to_null_vars(){
	source[0] = '\0';
	target[0] = '\0';
	label[0]  = '\0';
	extra_opt[0] = '\0';
}
void check_optarg(){
	if(optarg[0] == '-' || optarg[1] == '-'){
		fprintf(stderr, ERROR_COLORS "Unknown value '%s'! This is an option values! Consider try Command '--help' for more information!" ANSI_COLOR_RESET, optarg);
		exit(1);
	}
}

void usages(){
	char *op[] = {
		"-h, --help",
		"-v, --verbose",
		"-a, --setup-add",
		"-r, --setup-remove",
		"-f, --setup-new-fs",
		"-R, --mount-all",
		"-U, --eject-all",
		"-l, --list",
		"-A, --add",
		"-n, --newfs",
		"-b, --bind",
		"-F, --fs-mount",
		"-m, --mount",
		"-d, --remove-list",
		"-u, --eject-list",
		"-s, --source",
		"-T, --target",
		"-t, --fs-type",
		"-z, --size",
		"-o, --extra-options",
		"-L, --label"
	};
	char *desc[] = {
		"Shows this help",
		"Verbose an operation",
		"Wizard to add new mount configuration into the lists",
		"Wizard to remove a mount configuration from lists",
		"Wizard to create a new Filesystem, supported EXT4 and VFAT(FAT32)",
		"Mount all configuration in lists",
		"Unmount all configuration in a list",
		"Shows a list of mount configuration",
		"Add a mount configuration into the lists. The"ANSI_COLOR_RESET" "OP_COLOR"'--source', '--target', ['--bind' or '--fs-mount'], '--fs-type' (""'--fs-mount'"" only), and '--extra-options'"ANSI_COLOR_RESET" "ANSI_COLOR_CYAN"option must be specified!",
		"Create a new fs raw images. The"ANSI_COLOR_RESET" "OP_COLOR"'--source', '--fs-type', '--size', and '--label'"ANSI_COLOR_RESET" "ANSI_COLOR_CYAN"option must be specified",
		"Bind folders. Flags for mount configuration",
		"Mount raw images / block devices into folders. Flags for mount configuration",
		"Similar to"ANSI_COLOR_RESET" "OP_COLOR"'--mount-all'"ANSI_COLOR_RESET""ANSI_COLOR_CYAN", but with number filtering. Number that not in the list values(separated with commas) were be ignored",
		"Remove a mount configuration with the position specified in the list, start from zero. Can be multiple delete, but must separated with commas(,)",
		"Unmount a mount configuration with the position specified in the list, start from zero. Can be multiple, but must be separated with commas(,)",
		"Path to Source files / directories. Must be absolute path!",
		"Path to Target directories. Must be absolute path!",
		"The Filesystem type. For"ANSI_COLOR_RESET" "OP_COLOR"'--newfs'"ANSI_COLOR_RESET" "ANSI_COLOR_CYAN"option, only supported vfat (fat32) and ext4, but another is system dependent",
		"Size of new Raw images, must be type in KILOBYTES",
		"Extra options for mount configuration",
		"Label for new raw fs images"
	};
	printf( ANSI_COLOR_GREEN "F-Mount -v" VERSION " By Alexzander Purwoko Widiantoro <purwoko908@gmail.com> , Licensed under GPLv3" ANSI_COLOR_RESET );
	printf( ANSI_COLOR_GREEN "\nBuild Time :" ANSI_COLOR_RESET " " ANSI_COLOR_YELLOW "%s at %s" ANSI_COLOR_RESET , __DATE__, __TIME__);
	printf( ANSI_COLOR_GREEN "\n\nF-Mount is used to mount any folder, raw fs images, and blockdevices from the list and creates a new raw fs images. If you had unmount mountpoint dirs that has been using this app to mount, you must using commands"ANSI_COLOR_RESET" "OP_COLOR"'fmount --eject-all' or 'fmount --eject-list [,,,,n]'"ANSI_COLOR_RESET" "ANSI_COLOR_GREEN"to unmount!, ANOTHER COMMANDS IS PROHIBITED! " ANSI_COLOR_RESET );
	printf( ANSI_COLOR_GREEN "\nPlease be carefull and read this help before using." ANSI_COLOR_RESET );
	printf( ANSI_COLOR_GREEN "\n\n\nFull Usages : " ANSI_COLOR_RESET );
	printf( ANSI_COLOR_YELLOW "fmount -varfRUlhAnbF [-m LIST_NUMBERS] [-d LIST_NUMBERS] [-u EJECT_LIST] [-s SOURCE] [-T TARGET] [-t FS_TYPES] [-z SIZE_KBYTES] [-o MOUNT_OPTIONS] [-L LABEL]" ANSI_COLOR_RESET );
	printf("\n\n" ANSI_COLOR_GREEN "Details :" ANSI_COLOR_RESET);
	
	int x = 0;
	
	for(; x < 19; x++){
		printf("\n" ANSI_COLOR_YELLOW "\t%s" ANSI_COLOR_RESET "\t : " ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET, op[x], desc[x]);
	}
	printf("\n");
}
void setup_add(){
	FStore *file = __fdata_openf(PATH_CONF);
	if(!file){
		ERR(ERROR_COLORS "Couldn't open a file!\n" ANSI_COLOR_RESET);
		exit(1);
	}
	clrscr();
	
	printf(RUN_COLOR"Type a source file/folder : "ANSI_COLOR_RESET);
	fgets(source, BUFFER_SIZE, stdin);
	source[strlen(source) - 1] = '\0';
	
	printf(RUN_COLOR"Type a folder target : "ANSI_COLOR_RESET);
	fgets(target, BUFFER_SIZE, stdin);
	target[strlen(target) - 1] = '\0';

	printf(RUN_COLOR"Select a mount operation : \n1. Bind\n2. Filesystem Mount\nType a number(1/2)? : "ANSI_COLOR_RESET);
	int x;
	scanf("%d", &x);
	if(!x || x < 0 || x > 2){
		if(verbose){
			ERR(ERROR_COLORS "Bad number operation, select 1 or 2 instead\n" ANSI_COLOR_RESET);
		}
		fclose(file -> __fdata);
		free(file);
		exit(1);
	}
	fdt.__op_type = (x == 1)? MOUNT_BIND_OP : MOUNT_IMAGE_OP;
	getch();
	if(x == 1)goto bind;
	printf(RUN_COLOR"Types filesytem type (ext4/vfat/other) : "ANSI_COLOR_RESET);
	fgets(fdt.__fs_type_other, 16, stdin);
	fdt.__fs_type_other[strlen(fdt.__fs_type_other) - 1] = '\0';
	fdt.__fs_declared_type = SetFSType(&fdt, fdt.__fs_type_other);
	
	printf(RUN_COLOR"Mount options(separated with commas) : "ANSI_COLOR_RESET);
	fgets(extra_opt, BUFFER_SIZE, stdin);
	extra_opt[strlen(extra_opt) - 1] = '\0';
	
	bind:
	__fdatawrite(file, &fdt);
	fclose(file -> __fdata);
	free(file);
	set_to_null_vars();
	printf(RUN_COLOR"Try again? (y/n) : "ANSI_COLOR_RESET);
	if(getch() == 'y')setup_add();
	printf("\n");
	exit(0);
}

void _nmlist(FStore *f){
	if(!f)return;
	clrscr();
	
	/**
	 * Data structures
	 * if not mount bind
	 * [Mount_type|Mount_flags|Extra_option|FS_type|DIRSOURCE|DIRTARGET]
	 * if mount bind
	 * [Mount_type|Mount_flags|dirsource|dirtarget]
	 **/
	// No Type source target flags extra_opt fstype
	// | [] RED
	// No Cyan -
	// Type YELLOW - 
	// source GREEN -
	// target magenta -
	// flags BLUE -
	// extra_opt YELLOW -
	// fstype Cyan -
	char *fs_other = fdt. __fs_type_other;
	int *fs_type = &(fdt. __fs_declared_type);
	short *op_type = &(fdt. __op_type);
	unsigned long *flags = &(fdt. __flags);
	
	printf( ANSI_COLOR_BLUE "-------- LIST FOR AVAILABLE MOUNT DIRS/FILES --------" ANSI_COLOR_RESET );
	printf(
		"\n" ANSI_COLOR_RED "[" ANSI_COLOR_RESET " " ANSI_COLOR_CYAN "No" ANSI_COLOR_RESET 
		" " ANSI_COLOR_RED "|" ANSI_COLOR_RESET " " ANSI_COLOR_YELLOW "Type" ANSI_COLOR_RESET 
		" " ANSI_COLOR_RED "|" ANSI_COLOR_RESET " " ANSI_COLOR_GREEN "Source" ANSI_COLOR_RESET 
		" " ANSI_COLOR_RED "|" ANSI_COLOR_RESET " " ANSI_COLOR_MAGENTA "Target" ANSI_COLOR_RESET 
		" " ANSI_COLOR_RED "|" ANSI_COLOR_RESET " " ANSI_COLOR_BLUE "flagsnum" ANSI_COLOR_RESET 
		" " ANSI_COLOR_RED "|" ANSI_COLOR_RESET " " ANSI_COLOR_YELLOW "extra_opt" ANSI_COLOR_RESET 
		" " ANSI_COLOR_RED "|" ANSI_COLOR_RESET " " ANSI_COLOR_CYAN "fstype" ANSI_COLOR_RESET 
		" " ANSI_COLOR_RED "]" ANSI_COLOR_RESET
	);
	__setToFirstOp(f);
	size_t x = 0, len = __fdataget_length(f);
	__setToFirstOp(f);
	for(; x < len; x++){
		__fdataread_iter(f, &fdt);
		switch(*op_type){
			case MOUNT_IMAGE_OP:{
				char *fs = GetFSType(&fdt);
				printf(
					"\n" ANSI_COLOR_RED "[" ANSI_COLOR_RESET " " ANSI_COLOR_CYAN "%d" ANSI_COLOR_RESET  // no
					" " ANSI_COLOR_RED "|" ANSI_COLOR_RESET " " ANSI_COLOR_YELLOW "DEVS" ANSI_COLOR_RESET // type
					" " ANSI_COLOR_RED "|" ANSI_COLOR_RESET " " ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET // source
					,x
					,source
				);
				printf(
					" " ANSI_COLOR_RED "|" ANSI_COLOR_RESET " " ANSI_COLOR_MAGENTA "%s" ANSI_COLOR_RESET // target
					" " ANSI_COLOR_RED "|" ANSI_COLOR_RESET " " ANSI_COLOR_BLUE "%lu" ANSI_COLOR_RESET // flags
					,target
					,*flags
				);
				printf(
					" " ANSI_COLOR_RED "|" ANSI_COLOR_RESET " " ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET // extra_opt
					" " ANSI_COLOR_RED "|" ANSI_COLOR_RESET " " ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET //fstype
					" " ANSI_COLOR_RED "]" ANSI_COLOR_RESET
					,extra_opt
					,fs
				);
			}
			break;
			case MOUNT_BIND_OP:
			{
				printf(
					"\n" ANSI_COLOR_RED "[" ANSI_COLOR_RESET " " ANSI_COLOR_CYAN "%d" ANSI_COLOR_RESET  // no
					" " ANSI_COLOR_RED "|" ANSI_COLOR_RESET " " ANSI_COLOR_YELLOW "BIND" ANSI_COLOR_RESET // type
					" " ANSI_COLOR_RED "|" ANSI_COLOR_RESET " " ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET // source
					" " ANSI_COLOR_RED "|" ANSI_COLOR_RESET " " ANSI_COLOR_MAGENTA "%s" ANSI_COLOR_RESET // target
					" " ANSI_COLOR_RED "|" ANSI_COLOR_RESET " " ANSI_COLOR_BLUE "%lu" ANSI_COLOR_RESET // flags
					" " ANSI_COLOR_RED "|" ANSI_COLOR_RESET " " ANSI_COLOR_YELLOW "NaN" ANSI_COLOR_RESET // extra_opt
					" " ANSI_COLOR_RED "|" ANSI_COLOR_RESET " " ANSI_COLOR_CYAN "NaN" ANSI_COLOR_RESET //fstype
					" " ANSI_COLOR_RED "]" ANSI_COLOR_RESET,
					x,
					source,
					target,
					*flags
				);
			}
			break;
		}
	}
	printf("\n" ANSI_COLOR_BLUE "-----------------------------------------------------" ANSI_COLOR_RESET "\n");
}

void setup_remove(){
	FStore *file = __fdata_openf(PATH_CONF);
	if(!file){
		ERR(ERROR_COLORS "Couldn't open a file!\n" ANSI_COLOR_RESET);
		exit(1);
	}
	clrscr();
	_nmlist(file);
	int start, end;
	printf(RUN_COLOR"Type an index number (start) : "ANSI_COLOR_RESET);
	scanf("%d", &start);
	printf(RUN_COLOR"Type an index number (end) [can same with start] : "ANSI_COLOR_RESET);
	scanf("%d", &end);
	__setToFirstOp(file);
	__fdataremove(file, start, end);
	fclose(file -> __fdata);
	free(file);
	set_to_null_vars();
	printf(RUN_COLOR"Try again ? (y/n) : "ANSI_COLOR_RESET);
	getch();
	if(getch() == 'y')setup_remove();
	printf("\n");
	exit(0);
}
void setup_show_list(){
	FStore *file = __fdata_openf(PATH_CONF);
	if(!file){
		ERR(ERROR_COLORS "Couldn't open a file!\n" ANSI_COLOR_RESET);
		exit(1);
	}
	clrscr();
	_nmlist(file);
	fclose(file -> __fdata);
	free(file);
	exit(0);
}///
void setup_new_fs(){
	FStore *file = __fdata_openf(PATH_CONF);
	if(!file){
		ERR(ERROR_COLORS "Couldn't open a file!\n" ANSI_COLOR_RESET);
		exit(1);
	}
	clrscr();
	
	printf(RUN_COLOR"Type an absolute path to create raw image : "ANSI_COLOR_RESET);
	fgets(source, BUFFER_SIZE, stdin);
	source[strlen(source) - 1] = '\0';
	
	printf(RUN_COLOR"Type a label for new images : "ANSI_COLOR_RESET);
	fgets(label, 16, stdin);
	label[strlen(label) - 1] = '\0';

	printf(RUN_COLOR"Select a Filesystem types : \n1. ext4\n2. vfat (fat32)\nType a number(1/2)? : "ANSI_COLOR_RESET);
	int x;
	scanf("%d", &x);
	if(!x || x < 0 || x > 2){
		if(verbose){
			ERR(ERROR_COLORS "Bad number operation, select 1 or 2 instead!\n" ANSI_COLOR_RESET);
		}
		exit(1);
	}
	fstype = (x == 1)? FS_EXT4 : FS_VFAT;
	getch();
	printf(RUN_COLOR"Input a image sizes (in KBYTES)! : "ANSI_COLOR_RESET);
	scanf("%lu", &size);
	printf(RUN_COLOR"Processing...\n"ANSI_COLOR_RESET);
	// create filesystem
	opmode_new_fs();
	
	fclose(file -> __fdata);
	free(file);
	getch();
	printf(RUN_COLOR"Try again? (y/n) : "ANSI_COLOR_RESET);
	if(getch() == 'y')setup_add();
	printf("\n");
	exit(0);
}
void setup_mount_all(){
	FStore *file = __fdata_openf(PATH_CONF);
	if(!file){
		ERR(ERROR_COLORS "Couldn't open a file!\n" ANSI_COLOR_RESET);
		exit(1);
	}
	size_t len = __fdataget_length(file);
	__setToFirstOp(file);
	int x = 0;
	for(; x < len; x++){
		__fdataread_iter(file, &fdt);
		if(!IsMounted(&fdt)){
			int v = __mount_operations(&fdt);
			if(verbose){
				printf("Mount at [source " ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET "] on [target " ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET "] ...", source, target);
				if(!v)
					printf(ANSI_COLOR_CYAN "SUCCESS" ANSI_COLOR_RESET "\n");
				else 
					printf(ANSI_COLOR_RED "FAILED" ANSI_COLOR_RESET "\n");
			}
		}
		else 
			if(verbose)
				switch(fdt.__op_type){
					case MOUNT_BIND_OP:
						printf("The Directory " ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET " has been mounted on " ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET "...\n", source, target);
						break;
					case MOUNT_IMAGE_OP:
						if(IsBlockDevices(source))
							printf("The Block Devices " ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET " has been mounted on " ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET "...\n", source, target);
						else if(IsRegularFile(source))
							printf("The Image Devices " ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET " has been mounted on " ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET "...\n", source, target);
				}
	}
	fclose(file -> __fdata);
	free(file);
}
void setup_eject(){
	FStore *file = __fdata_openf(PATH_CONF);
	if(!file){
		ERR(ERROR_COLORS "Couldn't open a file!\n" ANSI_COLOR_RESET);
		exit(1);
	}
	size_t len = __fdataget_length(file);
	__setToFirstOp(file);
	int x = 0;
	for(; x < len; x++){
		__fdataread_iter(file, &fdt);
		if(IsMounted(&fdt)){
			if(verbose)
				printf("Unmounting at [" ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET "] ...", target);
			switch(fdt.__op_type){
				case MOUNT_BIND_OP:
					if(umount(target) == 0)
						if(verbose)
							printf(ANSI_COLOR_CYAN"SUCCESS\n"ANSI_COLOR_RESET);
					else 
						if(verbose)
							printf(ANSI_COLOR_RED"FAILED\n"ANSI_COLOR_RESET);
				break;
				case MOUNT_IMAGE_OP:
				{
					int res = umount(target);
					if(res){
						if(verbose)
							printf(ANSI_COLOR_RED"FAILED\n"ANSI_COLOR_RESET);
					}
					else
					{
						if(IsBlockDevices(source))
						if(verbose)
						{
							printf(ANSI_COLOR_CYAN"SUCCESS\n"ANSI_COLOR_RESET);
							continue;
						}

						// find a loop devices
						char *bf = extra_buf;
						struct stat sbuf;
						errno = 0;
						stat(source, &sbuf);
						if(errno)
							return 0;
						struct loop_info __loop_info;
						long loop_fd;
						unsigned long loop_pos = 0;
						short cond = 0;
						// compare value lo_device and  in loop 
						while(1){
							sprintf(bf, "/dev/block/loop%lu", loop_pos);
							loop_fd = open(bf, O_RDWR);
							if(loop_fd < 0){
								cond = 0;
								break;
							}
							ioctl(loop_fd, LOOP_GET_STATUS, &__loop_info);
							close(loop_fd);
							if(sbuf.st_dev == __loop_info.lo_device){
								cond = 1;
								break;
							}
							loop_pos++;
						}
						if(cond){
							if(ClearLoopContent(bf) >= 0)
								if(verbose)
									printf(ANSI_COLOR_CYAN"SUCCESS\n"ANSI_COLOR_RESET);
							else
								if(verbose)
									printf(ANSI_COLOR_RED"FAILED\n"ANSI_COLOR_RESET);
						}
						else 
							if(verbose)
							printf(ANSI_COLOR_RED"FAILED\n"ANSI_COLOR_RESET);
					}
				}
			}
		}
		else 
			if(verbose)
				printf("The Directory target (" ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET ") is not mounted...\n", target);
	}
	fclose(file -> __fdata);
	free(file);
}
void opmode_eject_conf(){
	FStore *file = __fdata_openf(PATH_CONF);
	if(!file){
		ERR(ERROR_COLORS "Couldn't open a file!\n" ANSI_COLOR_RESET);
		exit(1);
	}
	int *list_all = __strtointp(extra_buf, ',');
	size_t len = __getlenis(extra_buf, ',');
	int x = 0;
	for(; x < len; x++){
		__fdataread_atpos(file, &fdt, list_all[x]);
		if(IsMounted(&fdt)){
			if(verbose)
				printf("Unmounting at [" ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET "] ...", target);
			switch(fdt.__op_type){
				case MOUNT_BIND_OP:
					if(umount(target) == 0)
						if(verbose)
							printf(ANSI_COLOR_CYAN"SUCCESS\n"ANSI_COLOR_RESET);
					else 
						if(verbose)
							printf(ANSI_COLOR_RED"FAILED\n"ANSI_COLOR_RESET);
				break;
				case MOUNT_IMAGE_OP:
				{
					int res = umount(target);
					if(res){
						if(verbose)
							printf(ANSI_COLOR_RED"FAILED\n"ANSI_COLOR_RESET);
					}
					else
					{
						if(IsBlockDevices(source))
						if(verbose)
						{
							printf(ANSI_COLOR_CYAN"SUCCESS\n"ANSI_COLOR_RESET);
							continue;
						}

						// find a loop devices
						char *bf = extra_buf;
						struct stat sbuf;
						errno = 0;
						stat(source, &sbuf);
						if(errno)
							return 0;
						struct loop_info __loop_info;
						long loop_fd;
						unsigned long loop_pos = 0;
						short cond = 0;
						// compare value lo_device and  in loop 
						while(1){
							sprintf(bf, "/dev/block/loop%lu", loop_pos);
							loop_fd = open(bf, O_RDWR);
							if(loop_fd < 0){
								cond = 0;
								break;
							}
							ioctl(loop_fd, LOOP_GET_STATUS, &__loop_info);
							close(loop_fd);
							if(sbuf.st_dev == __loop_info.lo_device){
								cond = 1;
								break;
							}
							loop_pos++;
						}
						if(cond){
							if(ClearLoopContent(bf) >= 0)
								if(verbose)
									printf(ANSI_COLOR_CYAN"SUCCESS\n"ANSI_COLOR_RESET);
							else
								if(verbose)
									printf(ANSI_COLOR_RED"FAILED\n"ANSI_COLOR_RESET);
						}
						else 
							if(verbose)
							printf(ANSI_COLOR_RED"FAILED\n"ANSI_COLOR_RESET);
					}
				}
			}
		}
		else 
			if(verbose)
				printf("The Directory target (" ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET ") is not mounted...\n", target);
	}
	free(list_all);
	fclose(file -> __fdata);
	free(file);
}
void opmode_add_data(){
	if(verbose)printf( RUN_COLOR "Saving Data..." ANSI_COLOR_RESET );
	FStore *file = __fdata_openf(PATH_CONF);
	if(!file){
		ERR(ERROR_COLORS "Couldn't open a file!\n" ANSI_COLOR_RESET);
		exit(1);
	}
	sleep(1);
	if(mount_modes == MOUNT_IMAGE_OP){
		fdt.__fs_declared_type = fstype;
	}
	fdt.__op_type = mount_modes;
	__fdatawrite(file, &fdt);
	fclose(file -> __fdata);
	free(file);
	if(verbose)printf( RUN_COLOR "SUCCESS\n" ANSI_COLOR_RESET );
}
void opmode_remove_selected_conf(){
	FStore *file = __fdata_openf(PATH_CONF);
	if(!file){
		ERR(ERROR_COLORS "Couldn't open a file!\n" ANSI_COLOR_RESET);
		exit(1);
	}
	int *list_all = __strtointp(extra_buf, ',');
	size_t len = __getlenis(extra_buf, ',');
	int x = 0;
	for(; x < len; x++){
		if(verbose)
			printf( RUN_COLOR "Removing configuration number %d..." ANSI_COLOR_RESET "\n", list_all[x]);
		__fdataremove(file, list_all[x], list_all[x]);
	}
	free(list_all);
	fclose(file -> __fdata);
	free(file);
}
void opmode_mount_selected_list(){
	FStore *file = __fdata_openf(PATH_CONF);
	if(!file){
		ERR(ERROR_COLORS "Couldn't open a file!\n" ANSI_COLOR_RESET);
		exit(1);
	}
	int *list_all = __strtointp(extra_buf, ',');
	size_t len = __getlenis(extra_buf, ',');
	int x = 0;
	for(; x < len; x++){
		__fdataread_atpos(file, &fdt, list_all[x]);
		if(!IsMounted(&fdt)){
			int v = __mount_operations(&fdt);
			if(verbose){
				printf("Mount at [source " ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET "] on [target " ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET "] ...", source, target);
				if(!v)
					printf(ANSI_COLOR_CYAN "SUCCESS" ANSI_COLOR_RESET "\n");
				else 
					printf(ANSI_COLOR_RED "FAILED" ANSI_COLOR_RESET "\n");
			}
		}
		else 
			if(verbose)
				switch(fdt.__op_type){
					case MOUNT_BIND_OP:
						printf("The Directory " ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET " has been mounted on " ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET "...\n", source, target);
						break;
					case MOUNT_IMAGE_OP:
						if(IsBlockDevices(source))
							printf("The Block Devices " ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET " has been mounted on " ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET "...\n", source, target);
						else if(IsRegularFile(source))
							printf("The Image Devices " ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET " has been mounted on " ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET "...\n", source, target);
				}
	}
	free(list_all);
	fclose(file -> __fdata);
	free(file);
}
void opmode_new_fs(){
	if(verbose){
		printf( RUN_COLOR "Building Files..." ANSI_COLOR_RESET "\n");
		if(label[0] == '\0')
			printf(WARNING_COLOR "Warning : Label is empty, will set into default (%s)\n\n" ANSI_COLOR_RESET);
	}
	
	int x = create_sparse_files(source, size * ONE_KBYTES);
	if(x){
		if(verbose){
			ERR(ERROR_COLORS "Couldn't create a sparse files!\n" ANSI_COLOR_RESET);
		}
		exit(1);
	}
	bdv.__blksize 	= BLKSIZE_DEFAULT;
	bdv.__dsize 	= size;
	bdv.__fs_type 	= fstype;
	strcpy(bdv.__dname, source);
	if(label[0] == '\0')
		strcpy(label, DEFAULT_LABEL);
	printf(RUN_COLOR "Making filesystem..." ANSI_COLOR_RESET "\n\n");
	x = MakeFS(&bdv, verbose);
	if(x){
		if(verbose){
			ERR(ERROR_COLORS "Couldn't make a filesytem!\n" ANSI_COLOR_RESET);
		}
	}
	else 
		if(verbose)
			printf("\n" RUN_COLOR "Finished..." ANSI_COLOR_RESET "\n");	
}
void operate(){
	switch(selected_modes){
		case SETUP_MODE_ADD:
			setup_add();
			return;
		case SETUP_MODE_REMOVE:
			setup_remove();
			return;
		case SETUP_MODE_NEW_FS:
			setup_new_fs();
			return;
		case SETUP_MODE_MOUNT_ALL:
			setup_mount_all();
			return;
		case SETUP_MODE_LISTS:
			setup_show_list();
			return;
		case SETUP_MODE_EJECT:
			setup_eject();
			return;
		case OP_MODE_ADD:
			opmode_add_data();
			return;
		case OP_MODE_NEW_FS:
			opmode_new_fs();
			return;
		case OP_MODE_MOUNT_LIST:
			opmode_mount_selected_list();
			return;
		case OP_MODE_REMOVE_LIST:
			opmode_remove_selected_conf();
			return;
		case OP_MODE_EJECT:
			opmode_eject_conf();
	}
	return;
}

int main(const int argc, const char **argv){
	int c = 0;
	int option_index = 0;
	if(argc == 1){
		ERR(ERROR_COMMAND_PROHIBITED)
		exit(1);
	}
	static struct option longopts[] = 
	{
		// flags
		{"verbose", 		no_argument, 0, 'v'},
		// single opts
		{"setup-add", 		no_argument, 0, 'a'},
		{"setup-remove", 	no_argument, 0, 'r'},
		{"setup-new-fs", 	no_argument, 0, 'f'},
		{"mount-all", 		no_argument, 0, 'R'},
		{"eject-all", 		no_argument, 0, 'U'},
		{"list", 			no_argument, 0, 'l'},
		{"help", 			no_argument, 0, 'h'},
		// Mode operations...
		{"add", 			no_argument, 0, 'A'},
		{"newfs", 			no_argument, 0, 'n'},
		{"mount", 			required_argument, 0, 'm'},
		{"remove-list", 	required_argument, 0, 'd'},
		{"eject-list", 		required_argument, 0, 'u'},
		// mount modes
		{"bind", 			no_argument, 0, 'b'},
		{"fs-mount", 		no_argument, 0, 'F'},
		// must declare a flags to use it!
		{"source", 			required_argument, 0, 's'},
		{"target", 			required_argument, 0, 'T'},
		{"fs-type", 		required_argument, 0, 't'},
		{"size", 			required_argument, 0, 'z'},
		{"extra-options", 	optional_argument, 0, 'o'},
		{"label", 			required_argument, 0, 'L'},
		{0, 0, 0, 0}
	};
	
	set_to_null_vars();
	while((c = getopt_long(argc, argv, "varfRUlhAnbFm:d:u:s:T:t:z:o:L:", longopts, &option_index)) != -1){
		switch(c){
			case 'v':
				verbose = 1;
				break;
			case 'a':
				if(!setup_modes && !mode_op)
					setup_modes = SETUP_MODE_ADD;
				break;
			case 'r':
				if(!setup_modes && !mode_op)
					setup_modes = SETUP_MODE_REMOVE;
				break;		
			case 'f':
				if(!setup_modes && !mode_op)
					setup_modes = SETUP_MODE_NEW_FS;
				break;
			case 'R':
				if(!setup_modes && !mode_op)
					setup_modes = SETUP_MODE_MOUNT_ALL;
				break;
			case 'l':
				if(!setup_modes && !mode_op)
					setup_modes = SETUP_MODE_LISTS;
				break;
			case 'h':
			{
				usages();
				exit(0);
			}
			break;
			case 'A':
				if(!setup_modes && !mode_op)
					mode_op = OP_MODE_ADD;
				break;
			case 'n':
				if(!setup_modes && !mode_op)
					mode_op = OP_MODE_NEW_FS;
				break;
				
			case 'b':
				if(!setup_modes && mode_op == OP_MODE_ADD && !mode_op)
					mount_modes = MOUNT_BIND_OP;
			case 'F':
				if(!setup_modes && mode_op == OP_MODE_ADD && !mode_op)
					mount_modes = MOUNT_IMAGE_OP;
			
			case 'm':
				if(!setup_modes && !mode_op){
					check_optarg();
					mode_op = OP_MODE_MOUNT_LIST;
					extra_buf = optarg;
				}
			
			case 'd':
				if(!setup_modes && !mode_op){
					check_optarg();
					mode_op = OP_MODE_REMOVE_LIST;
					extra_buf = optarg;
				}
				break;
				
			case 's':
				if(!setup_modes && (mode_op == OP_MODE_ADD || mode_op == OP_MODE_NEW_FS)){
					check_optarg();
					strcpy(source, optarg);
				}
				break;
			case 'T':
				if(!setup_modes && mode_op == OP_MODE_ADD){
					check_optarg();
					strcpy(target, optarg);
				}
				break;
			case 't':
				if(!setup_modes && (mode_op == OP_MODE_ADD || mode_op == OP_MODE_NEW_FS)){
					check_optarg();
					if(!strcmp(FS_EXT4_STR, optarg))
						fstype = FS_EXT4;
					else if(!strcmp(FS_VFAT_STR, optarg))
						fstype = FS_VFAT;
					else fstype = 0;
				}
				break;
			case 'z':
				if(!setup_modes && mode_op == OP_MODE_NEW_FS){
					check_optarg();
					size = strtoul(optarg, (char **) NULL, 0);
				}
				break;
			case 'o':
				if(!setup_modes && mode_op == OP_MODE_ADD){
					check_optarg();
					if(optarg)
						strcpy(extra_opt, optarg);
				}
				break;
			case 'L':
				if(!setup_modes && mode_op == OP_MODE_NEW_FS){
					check_optarg();
					strcpy(label, optarg);
				}
				break;
			case 'u':
				if(!setup_modes && !mode_op){
					check_optarg();
					mode_op = OP_MODE_EJECT;
					extra_buf = optarg;
				}
				break;
			case 'U':
				if(!setup_modes && !mode_op)
					setup_modes = SETUP_MODE_EJECT;
				break;
			default:
				ERR(ERROR_COMMAND_PROHIBITED)
				exit(1);
		}
	}
	char *ck = check_vars_and_select_modes();
	if(ck){
		ERR(ck)
		exit(1);
	}
	operate();
	return 0;
}
