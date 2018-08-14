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
 
/**
 * Creates a new FAT32 Filesystem with the specified path into block devices
 * The devices must be exists, or you can build a new devices file by calling create_sparse_files() function
 * @param __pathimage The fullpath into the image devices
 * @param __label Label for new volume filesystem
 * @param __size Size for new filesystem volume
 * @param shouldVerbose Verbose operation? FALSE(0) not verbose, TRUE(1) verbose
 * @param checkBlocksAfter Should check if any bad blocks after creating fs? TRUE/FALSE
 * @return <int> 0 if success, 1 if fail!
 */
int MakeVfatFS(char *__pathimage, char *__label, unsigned long long __size, short shouldVerbose, short checkBlocksAfter);

/**
 * Creates a new EXT4 Filesystem with the specified path into block devices
 * The devices must be exists, or you can build a new devices file by calling create_sparse_files() function
 * @param __pathimage The fullpath into the image devices
 * @param __label Label for new volume filesystem
 * @param __blksize Defining a blocksize of a filesystem, value can be 1024, 2048 or 4096
 * @param __size Size for new filesystem volume
 * @param shouldVerbose Verbose operation? FALSE(0) not verbose, TRUE(1) verbose
 * @return <int> 0 if success, 1 if fail!
 */
int MakeExt4FS(char *__pathimage, char *__label, long __blksize, unsigned long long __size, short shouldVerbose);

