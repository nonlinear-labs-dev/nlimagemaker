/***
  Copyright (c) 2017 Nonlinear Labs GmbH

  Authors: Pascal Huerst <pascal.huerst@gmail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>.
***/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "main.h"

// 1k Alignement
#define  ALIGN 0x400
#define BUF_SIZE 10*1024

static void usage(const char *argv0)
{
	printf("Usage: %s [options]\n", argv0);
	printf("        -k, --kernel <kernel>        Kernel initramfs image (required)\n");
	printf("        -d, --dts <dts>              Device Tree Blob (required)\n");
	printf("        -r, --rootfs <rootfs>        Rootfs tarbal (required)\n");
	printf("        -e, --extract <img>          Image (required)\n");
	printf("        -v, --verbose                Be more verbose\n");
	printf("        -h, --help                   This help output\n");
}

static struct option long_options[] = {
{ "kernel",		required_argument,		0,	'k' },
{ "dts",		required_argument,		0,	'd' },
{ "rootfs",		required_argument,		0,	'r' },
{ "extract",	required_argument,		0,	'e' },
{ "verbose",	no_argument,			0,	'v' },
{ "help",		no_argument,			0,	'h' },
{ NULL,			0,						0,	0	}
};


// Actual HEADER is 6 * 4 Bytes, which is 28 Bytes.
// But we use ALIGN as size.
const static int HEADER_SIZE = ALIGN;

static uint32_t aligned(uint32_t value, uint32_t alignement)
{
	return (((value-1) / alignement) + 1) * alignement;
}

static size_t img_copy(int out, const char *fname)
{
	int fd;
	size_t ret, off = 0;
	char buf[BUF_SIZE];

	fd = open(fname, O_RDONLY);
	if (fd < 0) {
		printf("unable to open %s: %s\n", fname, strerror(errno));
		return fd;
	}

	while ((ret = read(fd, buf, sizeof(buf)))) {
		off += ret;
		write(out, buf, ret);
	}

	close(fd);
	return off;
}

static size_t extract_part(int img_fd, size_t offset, size_t size, const char *fname)
{
	int fd;
	size_t ret, off = 0;
	char buf[BUF_SIZE];

	fd = open(fname, O_WRONLY | O_CREAT, 0664);
	if (fd < 0) {
		printf("unable to open %s: %s\n", fname, strerror(errno));
		return fd;
	}

	lseek(img_fd, offset, SEEK_SET);
	while(size > 0) {
		ret = read(img_fd, buf, size > BUF_SIZE ? BUF_SIZE : size);
		if (ret < 0) {
			printf("unable to read from image: %s\n", strerror(errno));
			close(fd);
			return ret;
		}
		size -= ret;

		ret = write(fd, buf, ret);
		if (ret < 0) {
			printf("unable to write to %s: %s\n", fname, strerror(errno));
			close(fd);
			return ret;
		}
	}
	close(fd);
	return off;
}

static size_t get_size(const char* fname)
{
	struct stat st;
	stat(fname, &st);
	return st.st_size;
}

void print_header(struct header *h)
{
	printf("\n          Size:       Offset:\n");
	printf("  Header: %08X    %08X\n", HEADER_SIZE, 0x0);
	printf("  Kernel: %08X    %08X\n", h->kernel_size, h->kernel_offset);
	printf("  Dts   : %08X    %08X\n", h->dts_size, h->dts_offset);
	printf("  Rootfs: %08X    %08X\n", h->rootfs_size, h->rootfs_offset);
}

static size_t extract(const char* fname)
{
	struct header h = {0};

	int fd, ret;

	fd = open(fname, O_RDONLY);
	if (fd < 0) {
		printf("unable to open %s: %s\n", fname, strerror(errno));
		return fd;
	}

	if ((ret = read(fd, &h, sizeof(struct header))) < 0) {
		printf("unable to read from %s: %s\n", fname, strerror(errno));
		close(fd);
		return ret;
	}

	ret = extract_part(fd, h.kernel_offset, h.kernel_size, "uImage");
	if (ret < 0) {
		printf("can not extract kernel: %s\n", strerror(errno));
		close(fd);
		return ret;
	}

	ret = extract_part(fd, h.dts_offset, h.dts_size, "dtb");
	if (ret < 0) {
		printf("can not extract dts: %s\n", strerror(errno));
		close(fd);
		return ret;
	}

	ret = extract_part(fd, h.rootfs_offset, h.rootfs_size, "rootfs.tar.gz");
	if (ret < 0) {
		printf("can not extract rootfs: %s\n", strerror(errno));
		close(fd);
		return ret;
	}

	close(fd);

	return ret;
}


int main(int argc, char **argv)
{
	int fd_out;
	int ret;
	char *outfile = "/tmp/nonlinear.img";
	char be_verbose = 0;

	char *kernel_file, *dts_file, *rootfs_file;
	struct header h = {0};

	while (1) {
		int option_index = 0;
		int c = getopt_long (argc, argv, "e:k:r:d:v:h",
							 long_options, &option_index);
		if (c < 0)
			break;

		switch (c) {
		case 'e':
			return extract(optarg);
			break;
		case 'k':
			kernel_file = optarg;
			h.kernel_size = get_size(kernel_file);
			break;
		case 'd':
			dts_file = optarg;
			h.dts_size = get_size(dts_file);
			break;
		case 'r':
			rootfs_file = optarg;
			h.rootfs_size = get_size(rootfs_file);
			break;
		case 'v':
			be_verbose = ~0;
			break;
		case 'h':
		default:
			usage(argv[0]);
			return -1;
		}
	}

	h.kernel_offset = aligned(HEADER_SIZE, ALIGN);
	h.dts_offset = h.kernel_offset + aligned(h.kernel_size, ALIGN);
	h.rootfs_offset = h.dts_offset + aligned(h.dts_size, ALIGN);

	if (!kernel_file	|| !dts_file	|| !rootfs_file ||
			!h.kernel_offset|| !h.dts_offset|| !h.rootfs_offset ||
			!h.kernel_size	|| !h.dts_size	|| !h.rootfs_size) {
		usage(argv[0]);
		return -1;
	}

	fd_out = open(outfile,
				  O_RDWR | O_CREAT,
				  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd_out < 0) {
		printf("can not open target file\n");
		return fd_out;
	}

	// Write Header
	lseek(fd_out, 0, SEEK_SET);
	write(fd_out, &h, sizeof(struct header));

	// Write Kernel
	lseek(fd_out, h.kernel_offset, SEEK_SET);
	if ((ret = img_copy(fd_out, kernel_file)) < 0) {
		printf("can not copy kernel into target image: %s\n", strerror(errno));
		close(fd_out);
		return ret;
	}

	// Write DTS
	lseek(fd_out, h.dts_offset, SEEK_SET);
	if ((ret = img_copy(fd_out, dts_file)) < 0) {
		printf("can not copy dts into target image: %s\n", strerror(errno));
		close(fd_out);
		return ret;
	}

	// Write Rootfs
	lseek(fd_out, h.rootfs_offset, SEEK_SET);
	if ((ret = img_copy(fd_out, rootfs_file)) < 0) {
		printf("can not copy rootfs into target image: %s\n", strerror(errno));
		close(fd_out);
		return ret;
	}


	if (be_verbose)
		print_header(&h);

	close(fd_out);

	return EXIT_SUCCESS;
}
