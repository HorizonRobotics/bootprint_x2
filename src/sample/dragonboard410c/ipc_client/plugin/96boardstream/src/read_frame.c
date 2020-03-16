/*
 * yavta --  Yet Another V4L2 Test Application
 *
 * Copyright (C) 2005-2010 Laurent Pinchart <laurent.pinchart@ideasonboard.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 */
#ifndef WIN32
#define __STDC_FORMAT_MACROS

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <inttypes.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <sched.h>
#include <termios.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <linux/videodev2.h>

#define ARRAY_SIZE(a)	(sizeof(a)/sizeof((a)[0]))

enum buffer_fill_mode
{
	BUFFER_FILL_NONE = 0,
	BUFFER_FILL_FRAME = 1 << 0,
	BUFFER_FILL_PADDING = 1 << 1,
};

struct buffer
{
	unsigned int idx;
	unsigned int padding[VIDEO_MAX_PLANES];
	unsigned int size[VIDEO_MAX_PLANES];
	void *mem[VIDEO_MAX_PLANES];
};

struct device
{
	int fd;
	int opened;

	enum v4l2_buf_type type;
	enum v4l2_memory memtype;
	unsigned int nbufs;
	struct buffer *buffers;

	unsigned int width;
	unsigned int height;
	uint32_t buffer_output_flags;
	uint32_t timestamp_type;

	unsigned char num_planes;
	struct v4l2_plane_pix_format plane_fmt[VIDEO_MAX_PLANES];

	void *pattern[VIDEO_MAX_PLANES];
	unsigned int patternsize[VIDEO_MAX_PLANES];

	bool write_data_prefix;
};

/* -----------------------------------------------------------------------------
 * Pause Handling
 */

static bool pause_resume;
static struct termios pause_term;
static bool pause_no_term;
static bool pause_term_configured;
static char pause_filename[23];

static void pause_signal_handler(int signal __attribute__((__unused__)))
{
	pause_resume = true;
}

static void pause_wait(void)
{
	int ret;
	int fd;

	fd = open(pause_filename, O_CREAT, 0);
	if (fd != -1)
		close(fd);

	if (pause_no_term) {
		printf("Paused waiting for SIGUSR1\n");
		while (!pause_resume)
			pause();

		goto done;
	}

	printf("Paused waiting for key press or SIGUSR1\n");
	pause_resume = false;

	while (!pause_resume) {
		fd_set rfds;
		char c;

		FD_ZERO(&rfds);
		FD_SET(0, &rfds);

		ret = select(1, &rfds, NULL, NULL, NULL);
		if (ret < 0 && errno != EINTR)
			break;

		if (ret == 1) {
			ret = read(0, &c, 1);
			break;
		}
	}

done:
	unlink(pause_filename);
}

static void pause_cleanup(void)
{
	if (pause_term_configured)
		tcsetattr(0, TCSANOW, &pause_term);

	unlink(pause_filename);
}

static int pause_init(void)
{
	struct sigaction sig_usr1;
	struct termios term;
	int ret;

	sprintf(pause_filename, ".yavta.wait.%u", getpid());

	memset(&sig_usr1, 0, sizeof(sig_usr1));
	sig_usr1.sa_handler = pause_signal_handler;
	ret = sigaction(SIGUSR1, &sig_usr1, NULL);
	if (ret < 0) {
		printf("Unable to install SIGUSR1 handler: %s (%d)\n",
		       strerror(errno), errno);
		return -errno;
	}

	ret = tcgetattr(0, &term);
	if (ret < 0) {
		if (errno == ENOTTY) {
			pause_no_term = true;
			return 0;
		}

		printf("Unable to retrieve terminal attributes: %s (%d)\n",
		       strerror(errno), errno);
		return -errno;
	}

	pause_term = term;
	pause_term_configured = true;

	atexit(pause_cleanup);

	term.c_lflag &= ~ICANON;
	term.c_lflag &= ~ECHO;
	term.c_cc[VMIN] = 0;
	term.c_cc[VTIME] = 0;

	ret = tcsetattr(0, TCSANOW, &term);
	if (ret < 0) {
		printf("Unable to set terminal attributes: %s (%d)\n",
		       strerror(errno), errno);
		return -errno;
	}

	return 0;
}

/* -----------------------------------------------------------------------------
 * Pause Handling
 */

static bool video_is_mplane(struct device *dev)
{
	return dev->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE ||
	       dev->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
}

static bool video_is_meta(struct device *dev)
{
	return dev->type == V4L2_BUF_TYPE_META_CAPTURE ||
	       dev->type == V4L2_BUF_TYPE_META_OUTPUT;
}

static bool video_is_capture(struct device *dev)
{
	return dev->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE ||
	       dev->type == V4L2_BUF_TYPE_VIDEO_CAPTURE ||
	       dev->type == V4L2_BUF_TYPE_META_CAPTURE;
}

static bool video_is_output(struct device *dev)
{
	return dev->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE ||
	       dev->type == V4L2_BUF_TYPE_VIDEO_OUTPUT ||
	       dev->type == V4L2_BUF_TYPE_META_OUTPUT;
}

static struct {
	enum v4l2_buf_type type;
	bool supported;
	const char *name;
	const char *string;
} buf_types[] = {
	{ V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, 1, "Video capture mplanes", "capture-mplane", },
	{ V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE, 1, "Video output", "output-mplane", },
	{ V4L2_BUF_TYPE_VIDEO_CAPTURE, 1, "Video capture", "capture", },
	{ V4L2_BUF_TYPE_VIDEO_OUTPUT, 1, "Video output mplanes", "output", },
	{ V4L2_BUF_TYPE_VIDEO_OVERLAY, 0, "Video overlay", "overlay" },
	{ V4L2_BUF_TYPE_META_CAPTURE, 1, "Meta-data capture", "meta-capture", },
	{ V4L2_BUF_TYPE_META_OUTPUT, 1, "Meta-data output", "meta-output", },
};

static int v4l2_buf_type_from_string(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(buf_types); i++) {
		if (!buf_types[i].supported)
			continue;

		if (strcmp(buf_types[i].string, str))
			continue;

		return buf_types[i].type;
	}

	return -1;
}

static const char *v4l2_buf_type_name(enum v4l2_buf_type type)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(buf_types); ++i) {
		if (buf_types[i].type == type)
			return buf_types[i].name;
	}

	if (type & V4L2_BUF_TYPE_PRIVATE)
		return "Private";
	else
		return "Unknown";
}

static struct v4l2_format_info {
	const char *name;
	unsigned int fourcc;
	unsigned char n_planes;
} pixel_formats[] = {
	{ "RGB332", V4L2_PIX_FMT_RGB332, 1 },
	{ "RGB444", V4L2_PIX_FMT_RGB444, 1 },
	{ "ARGB444", V4L2_PIX_FMT_ARGB444, 1 },
	{ "XRGB444", V4L2_PIX_FMT_XRGB444, 1 },
	{ "RGB555", V4L2_PIX_FMT_RGB555, 1 },
	{ "ARGB555", V4L2_PIX_FMT_ARGB555, 1 },
	{ "XRGB555", V4L2_PIX_FMT_XRGB555, 1 },
	{ "RGB565", V4L2_PIX_FMT_RGB565, 1 },
	{ "RGB555X", V4L2_PIX_FMT_RGB555X, 1 },
	{ "RGB565X", V4L2_PIX_FMT_RGB565X, 1 },
	{ "BGR666", V4L2_PIX_FMT_BGR666, 1 },
	{ "BGR24", V4L2_PIX_FMT_BGR24, 1 },
	{ "RGB24", V4L2_PIX_FMT_RGB24, 1 },
	{ "BGR32", V4L2_PIX_FMT_BGR32, 1 },
	{ "ABGR32", V4L2_PIX_FMT_ABGR32, 1 },
	{ "XBGR32", V4L2_PIX_FMT_XBGR32, 1 },
	{ "RGB32", V4L2_PIX_FMT_RGB32, 1 },
	{ "ARGB32", V4L2_PIX_FMT_ARGB32, 1 },
	{ "XRGB32", V4L2_PIX_FMT_XRGB32, 1 },
	{ "HSV24", V4L2_PIX_FMT_HSV24, 1 },
	{ "HSV32", V4L2_PIX_FMT_HSV32, 1 },
	{ "Y8", V4L2_PIX_FMT_GREY, 1 },
	{ "Y10", V4L2_PIX_FMT_Y10, 1 },
	{ "Y12", V4L2_PIX_FMT_Y12, 1 },
	{ "Y16", V4L2_PIX_FMT_Y16, 1 },
	{ "UYVY", V4L2_PIX_FMT_UYVY, 1 },
	{ "VYUY", V4L2_PIX_FMT_VYUY, 1 },
	{ "YUYV", V4L2_PIX_FMT_YUYV, 1 },
	{ "YVYU", V4L2_PIX_FMT_YVYU, 1 },
	{ "NV12", V4L2_PIX_FMT_NV12, 1 },
	{ "NV12M", V4L2_PIX_FMT_NV12M, 2 },
	{ "NV21", V4L2_PIX_FMT_NV21, 1 },
	{ "NV21M", V4L2_PIX_FMT_NV21M, 2 },
	{ "NV16", V4L2_PIX_FMT_NV16, 1 },
	{ "NV16M", V4L2_PIX_FMT_NV16M, 2 },
	{ "NV61", V4L2_PIX_FMT_NV61, 1 },
	{ "NV61M", V4L2_PIX_FMT_NV61M, 2 },
	{ "NV24", V4L2_PIX_FMT_NV24, 1 },
	{ "NV42", V4L2_PIX_FMT_NV42, 1 },
	{ "YUV420M", V4L2_PIX_FMT_YUV420M, 3 },
	{ "YUV422M", V4L2_PIX_FMT_YUV422M, 3 },
	{ "YUV444M", V4L2_PIX_FMT_YUV444M, 3 },
	{ "YVU420M", V4L2_PIX_FMT_YVU420M, 3 },
	{ "YVU422M", V4L2_PIX_FMT_YVU422M, 3 },
	{ "YVU444M", V4L2_PIX_FMT_YVU444M, 3 },
	{ "SBGGR8", V4L2_PIX_FMT_SBGGR8, 1 },
	{ "SGBRG8", V4L2_PIX_FMT_SGBRG8, 1 },
	{ "SGRBG8", V4L2_PIX_FMT_SGRBG8, 1 },
	{ "SRGGB8", V4L2_PIX_FMT_SRGGB8, 1 },
	{ "SBGGR10_DPCM8", V4L2_PIX_FMT_SBGGR10DPCM8, 1 },
	{ "SGBRG10_DPCM8", V4L2_PIX_FMT_SGBRG10DPCM8, 1 },
	{ "SGRBG10_DPCM8", V4L2_PIX_FMT_SGRBG10DPCM8, 1 },
	{ "SRGGB10_DPCM8", V4L2_PIX_FMT_SRGGB10DPCM8, 1 },
	{ "SBGGR10", V4L2_PIX_FMT_SBGGR10, 1 },
	{ "SGBRG10", V4L2_PIX_FMT_SGBRG10, 1 },
	{ "SGRBG10", V4L2_PIX_FMT_SGRBG10, 1 },
	{ "SRGGB10", V4L2_PIX_FMT_SRGGB10, 1 },
	{ "SBGGR10P", V4L2_PIX_FMT_SBGGR10P, 1 },
	{ "SGBRG10P", V4L2_PIX_FMT_SGBRG10P, 1 },
	{ "SGRBG10P", V4L2_PIX_FMT_SGRBG10P, 1 },
	{ "SRGGB10P", V4L2_PIX_FMT_SRGGB10P, 1 },
	{ "SBGGR12", V4L2_PIX_FMT_SBGGR12, 1 },
	{ "SGBRG12", V4L2_PIX_FMT_SGBRG12, 1 },
	{ "SGRBG12", V4L2_PIX_FMT_SGRBG12, 1 },
	{ "SRGGB12", V4L2_PIX_FMT_SRGGB12, 1 },
	{ "IPU3_SBGGR10", V4L2_PIX_FMT_IPU3_SBGGR10, 1 },
	{ "IPU3_SGBRG10", V4L2_PIX_FMT_IPU3_SGBRG10, 1 },
	{ "IPU3_SGRBG10", V4L2_PIX_FMT_IPU3_SGRBG10, 1 },
	{ "IPU3_SRGGB10", V4L2_PIX_FMT_IPU3_SRGGB10, 1 },
	{ "DV", V4L2_PIX_FMT_DV, 1 },
	{ "MJPEG", V4L2_PIX_FMT_MJPEG, 1 },
	{ "MPEG", V4L2_PIX_FMT_MPEG, 1 },
};

static void list_formats(void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(pixel_formats); i++)
		printf("%s (\"%c%c%c%c\", %u planes)\n",
		       pixel_formats[i].name,
		       pixel_formats[i].fourcc & 0xff,
		       (pixel_formats[i].fourcc >> 8) & 0xff,
		       (pixel_formats[i].fourcc >> 16) & 0xff,
		       (pixel_formats[i].fourcc >> 24) & 0xff,
		       pixel_formats[i].n_planes);
}

static const struct v4l2_format_info *v4l2_format_by_fourcc(unsigned int fourcc)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(pixel_formats); ++i) {
		if (pixel_formats[i].fourcc == fourcc)
			return &pixel_formats[i];
	}

	return NULL;
}

static const struct v4l2_format_info *v4l2_format_by_name(const char *name)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(pixel_formats); ++i) {
		if (strcasecmp(pixel_formats[i].name, name) == 0)
			return &pixel_formats[i];
	}

	return NULL;
}

static const char *v4l2_format_name(unsigned int fourcc)
{
	const struct v4l2_format_info *info;
	static char name[5];
	unsigned int i;

	info = v4l2_format_by_fourcc(fourcc);
	if (info)
		return info->name;

	for (i = 0; i < 4; ++i) {
		name[i] = fourcc & 0xff;
		fourcc >>= 8;
	}

	name[4] = '\0';
	return name;
}

static const struct {
	const char *name;
	enum v4l2_field field;
} fields[] = {
	{ "any", V4L2_FIELD_ANY },
	{ "none", V4L2_FIELD_NONE },
	{ "top", V4L2_FIELD_TOP },
	{ "bottom", V4L2_FIELD_BOTTOM },
	{ "interlaced", V4L2_FIELD_INTERLACED },
	{ "seq-tb", V4L2_FIELD_SEQ_TB },
	{ "seq-bt", V4L2_FIELD_SEQ_BT },
	{ "alternate", V4L2_FIELD_ALTERNATE },
	{ "interlaced-tb", V4L2_FIELD_INTERLACED_TB },
	{ "interlaced-bt", V4L2_FIELD_INTERLACED_BT },
};

static enum v4l2_field v4l2_field_from_string(const char *name)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(fields); ++i) {
		if (strcasecmp(fields[i].name, name) == 0)
			return fields[i].field;
	}

	return -1;
}

static const char *v4l2_field_name(enum v4l2_field field)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(fields); ++i) {
		if (fields[i].field == field)
			return fields[i].name;
	}

	return "unknown";
}

static void video_set_buf_type(struct device *dev, enum v4l2_buf_type type)
{
	dev->type = type;
}

static bool video_has_valid_buf_type(struct device *dev)
{
	return (int)dev->type != -1;
}

static void video_init(struct device *dev)
{
	dev->fd = -1;
	dev->memtype = V4L2_MEMORY_MMAP;
	dev->buffers = NULL;
	dev->type = (enum v4l2_buf_type)-1;
}

static bool video_has_fd(struct device *dev)
{
	return dev->fd != -1;
}

static int video_set_fd(struct device *dev, int fd)
{
	if (video_has_fd(dev)) {
		printf("Can't set fd (already open).\n");
		return -1;
	}

	dev->fd = fd;

	return 0;
}

static int video_open(struct device *dev, const char *devname)
{
	if (video_has_fd(dev)) {
		printf("Can't open device (already open).\n");
		return -1;
	}

	dev->fd = open(devname, O_RDWR);
	if (dev->fd < 0) {
		printf("Error opening device %s: %s (%d).\n", devname,
		       strerror(errno), errno);
		return dev->fd;
	}

	printf("Device %s opened.\n", devname);

	dev->opened = 1;

	return 0;
}

static int video_querycap(struct device *dev, unsigned int *capabilities)
{
	struct v4l2_capability cap;
	unsigned int caps;
	bool has_video;
	bool has_meta;
	bool has_capture;
	bool has_output;
	bool has_mplane;
	int ret;

	memset(&cap, 0, sizeof cap);
	ret = ioctl(dev->fd, VIDIOC_QUERYCAP, &cap);
	if (ret < 0)
		return 0;

	caps = cap.capabilities & V4L2_CAP_DEVICE_CAPS
	     ? cap.device_caps : cap.capabilities;

	has_video = caps & (V4L2_CAP_VIDEO_CAPTURE_MPLANE |
			    V4L2_CAP_VIDEO_CAPTURE |
			    V4L2_CAP_VIDEO_OUTPUT_MPLANE |
			    V4L2_CAP_VIDEO_OUTPUT);
	has_meta = caps & (V4L2_CAP_META_CAPTURE |
			   V4L2_CAP_META_OUTPUT);
	has_capture = caps & (V4L2_CAP_VIDEO_CAPTURE_MPLANE |
			      V4L2_CAP_VIDEO_CAPTURE |
			      V4L2_CAP_META_CAPTURE);
	has_output = caps & (V4L2_CAP_VIDEO_OUTPUT_MPLANE |
			     V4L2_CAP_VIDEO_OUTPUT |
			     V4L2_CAP_META_OUTPUT);
	has_mplane = caps & (V4L2_CAP_VIDEO_CAPTURE_MPLANE |
			     V4L2_CAP_VIDEO_OUTPUT_MPLANE);

	printf("Device `%s' on `%s' (driver '%s') supports%s%s%s%s %s mplanes.\n",
		cap.card, cap.bus_info, cap.driver,
		has_video ? " video," : "",
		has_meta ? " meta-data," : "",
		has_capture ? " capture," : "",
		has_output ? " output," : "",
		has_mplane ? "with" : "without");

	*capabilities = caps;

	return 0;
}

static int cap_get_buf_type(unsigned int capabilities)
{
	if (capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
		return V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	} else if (capabilities & V4L2_CAP_VIDEO_OUTPUT_MPLANE) {
		return V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	} else if (capabilities & V4L2_CAP_VIDEO_CAPTURE) {
		return  V4L2_BUF_TYPE_VIDEO_CAPTURE;
	} else if (capabilities & V4L2_CAP_VIDEO_OUTPUT) {
		return V4L2_BUF_TYPE_VIDEO_OUTPUT;
	} else if (capabilities & V4L2_CAP_META_CAPTURE) {
		return V4L2_BUF_TYPE_META_CAPTURE;
	} else if (capabilities & V4L2_CAP_META_OUTPUT) {
		return V4L2_BUF_TYPE_META_OUTPUT;
	} else {
		printf("Device supports neither capture nor output.\n");
		return -EINVAL;
	}

	return 0;
}

static void video_close(struct device *dev)
{
  printf("video_close\n");
	unsigned int i;

	for (i = 0; i < dev->num_planes; i++)
		free(dev->pattern[i]);

	free(dev->buffers);
	if (dev->opened)
		close(dev->fd);
}

static void video_log_status(struct device *dev)
{
	ioctl(dev->fd, VIDIOC_LOG_STATUS);
}

static int query_control(struct device *dev, unsigned int id,
			 struct v4l2_query_ext_ctrl *query)
{
	struct v4l2_queryctrl q;
	int ret;

	memset(query, 0, sizeof(*query));
	query->id = id;

	ret = ioctl(dev->fd, VIDIOC_QUERY_EXT_CTRL, query);
	if (ret < 0)
		ret = -errno;
	if (!ret || ret == -EINVAL)
		return ret;

	if (ret != -ENOTTY) {
		printf("unable to query control 0x%8.8x: %s (%d).\n",
		       id, strerror(-ret), -ret);
		return ret;
	}

	/*
	 * If VIDIOC_QUERY_EXT_CTRL isn't available emulate it using
	 * VIDIOC_QUERYCTRL.
	 */
	memset(&q, 0, sizeof(q));
	q.id = id;

	ret = ioctl(dev->fd, VIDIOC_QUERYCTRL, &q);
	if (ret < 0) {
		ret = -errno;
		printf("unable to query control 0x%8.8x: %s (%d).\n",
		       id, strerror(-ret), -ret);
		return ret;
	}

	memset(query, 0, sizeof(*query));
	query->id = q.id;
	query->type = q.type;
	memcpy(query->name, q.name, sizeof(query->name));
	query->minimum = q.minimum;
	query->maximum = q.maximum;
	query->step = q.step;
	query->default_value = q.default_value;
	query->flags = q.flags;

	if (q.type == V4L2_CTRL_TYPE_STRING &&
	    !(q.flags & V4L2_CTRL_FLAG_HAS_PAYLOAD)) {
		query->elem_size = q.maximum + 1;
		query->elems = 1;
	}

	return 0;
}

static int get_control(struct device *dev,
		       const struct v4l2_query_ext_ctrl *query,
		       struct v4l2_ext_control *ctrl,
		       unsigned int which)
{
	struct v4l2_ext_controls ctrls;
	struct v4l2_control old;
	int ret;

	memset(&ctrls, 0, sizeof(ctrls));
	memset(ctrl, 0, sizeof(*ctrl));

	ctrls.which = which;
	ctrls.count = 1;
	ctrls.controls = ctrl;

	ctrl->id = query->id;

	if (query->flags & V4L2_CTRL_FLAG_HAS_PAYLOAD) {
		ctrl->size = query->elems * query->elem_size;
		ctrl->ptr = malloc(ctrl->size);
		if (ctrl->ptr == NULL)
			return -ENOMEM;
	}

	ret = ioctl(dev->fd, VIDIOC_G_EXT_CTRLS, &ctrls);
	if (ret != -1)
		return 0;

	if (query->flags & V4L2_CTRL_FLAG_HAS_PAYLOAD)
		free(ctrl->ptr);

	if (query->flags & V4L2_CTRL_FLAG_HAS_PAYLOAD ||
	    query->type == V4L2_CTRL_TYPE_INTEGER64 ||
	    (errno != EINVAL && errno != ENOTTY))
		return -errno;

	old.id = query->id;
	ret = ioctl(dev->fd, VIDIOC_G_CTRL, &old);
	if (ret < 0)
		return -errno;

	ctrl->value = old.value;
	return 0;
}

static int set_control(struct device *dev,
		       const struct v4l2_query_ext_ctrl *query,
		       struct v4l2_ext_control *ctrl)
{
	struct v4l2_ext_controls ctrls;
	struct v4l2_control old;
	int ret;

	memset(&ctrls, 0, sizeof(ctrls));

	ctrls.ctrl_class = V4L2_CTRL_ID2CLASS(ctrl->id);
	ctrls.count = 1;
	ctrls.controls = ctrl;

	ctrl->id = query->id;

	ret = ioctl(dev->fd, VIDIOC_S_EXT_CTRLS, &ctrls);
	if (ret != -1)
		return 0;

	if (query->flags & V4L2_CTRL_FLAG_HAS_PAYLOAD ||
	    query->type == V4L2_CTRL_TYPE_INTEGER64 ||
	    (errno != EINVAL && errno != ENOTTY))
		return -1;

	old.id = ctrl->id;
	old.value = ctrl->value;
	ret = ioctl(dev->fd, VIDIOC_S_CTRL, &old);
	if (ret != -1)
		ctrl->value = old.value;

	return ret;
}

static int video_get_format(struct device *dev)
{
	struct v4l2_format fmt;
	unsigned int i;
	int ret;

	memset(&fmt, 0, sizeof fmt);
	fmt.type = dev->type;

	ret = ioctl(dev->fd, VIDIOC_G_FMT, &fmt);
	if (ret < 0) {
		printf("Unable to get format: %s (%d).\n", strerror(errno),
			errno);
		return ret;
	}

	if (video_is_mplane(dev)) {
		dev->width = fmt.fmt.pix_mp.width;
		dev->height = fmt.fmt.pix_mp.height;
		dev->num_planes = fmt.fmt.pix_mp.num_planes;

		printf("Video format: %s (%08x) %ux%u field %s, %u planes: \n",
			v4l2_format_name(fmt.fmt.pix_mp.pixelformat), fmt.fmt.pix_mp.pixelformat,
			fmt.fmt.pix_mp.width, fmt.fmt.pix_mp.height,
			v4l2_field_name(fmt.fmt.pix_mp.field),
			fmt.fmt.pix_mp.num_planes);

		for (i = 0; i < fmt.fmt.pix_mp.num_planes; i++) {
			dev->plane_fmt[i].bytesperline =
					fmt.fmt.pix_mp.plane_fmt[i].bytesperline;
			dev->plane_fmt[i].sizeimage =
					fmt.fmt.pix_mp.plane_fmt[i].bytesperline ?
						fmt.fmt.pix_mp.plane_fmt[i].sizeimage : 0;

			printf(" * Stride %u, buffer size %u\n",
				fmt.fmt.pix_mp.plane_fmt[i].bytesperline,
				fmt.fmt.pix_mp.plane_fmt[i].sizeimage);
		}
	} else if (video_is_meta(dev)) {
		dev->width = 0;
		dev->height = 0;
		dev->num_planes = 1;

		printf("Meta-data format: %s (%08x) buffer size %u\n",
			v4l2_format_name(fmt.fmt.meta.dataformat), fmt.fmt.meta.dataformat,
					 fmt.fmt.meta.buffersize);
	} else {
		dev->width = fmt.fmt.pix.width;
		dev->height = fmt.fmt.pix.height;
		dev->num_planes = 1;

		dev->plane_fmt[0].bytesperline = fmt.fmt.pix.bytesperline;
		dev->plane_fmt[0].sizeimage = fmt.fmt.pix.bytesperline ? fmt.fmt.pix.sizeimage : 0;

		printf("Video format: %s (%08x) %ux%u (stride %u) field %s buffer size %u\n",
			v4l2_format_name(fmt.fmt.pix.pixelformat), fmt.fmt.pix.pixelformat,
			fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix.bytesperline,
			v4l2_field_name(fmt.fmt.pix_mp.field),
			fmt.fmt.pix.sizeimage);
	}

	return 0;
}

static int video_set_format(struct device *dev, unsigned int w, unsigned int h,
			    unsigned int format, unsigned int stride,
			    unsigned int buffer_size, enum v4l2_field field,
			    unsigned int flags)
{
	struct v4l2_format fmt;
	unsigned int i;
	int ret;

	memset(&fmt, 0, sizeof fmt);
	fmt.type = dev->type;

	if (video_is_mplane(dev)) {
		const struct v4l2_format_info *info = v4l2_format_by_fourcc(format);

		fmt.fmt.pix_mp.width = w;
		fmt.fmt.pix_mp.height = h;
		fmt.fmt.pix_mp.pixelformat = format;
		fmt.fmt.pix_mp.field = field;
		fmt.fmt.pix_mp.num_planes = info->n_planes;
		fmt.fmt.pix_mp.flags = flags;

		for (i = 0; i < fmt.fmt.pix_mp.num_planes; i++) {
			fmt.fmt.pix_mp.plane_fmt[i].bytesperline = stride;
			fmt.fmt.pix_mp.plane_fmt[i].sizeimage = buffer_size;
		}
	} else if (video_is_meta(dev)) {
		fmt.fmt.meta.dataformat = format;
		fmt.fmt.meta.buffersize = buffer_size;
	} else {
		fmt.fmt.pix.width = w;
		fmt.fmt.pix.height = h;
		fmt.fmt.pix.pixelformat = format;
		fmt.fmt.pix.field = field;
		fmt.fmt.pix.bytesperline = stride;
		fmt.fmt.pix.sizeimage = buffer_size;
		fmt.fmt.pix.priv = V4L2_PIX_FMT_PRIV_MAGIC;
		fmt.fmt.pix.flags = flags;
	}

	ret = ioctl(dev->fd, VIDIOC_S_FMT, &fmt);
	if (ret < 0) {
		printf("Unable to set format: %s (%d).\n", strerror(errno),
			errno);
		return ret;
	}

	if (video_is_mplane(dev)) {
		printf("Video format set: %s (%08x) %ux%u field %s, %u planes: \n",
			v4l2_format_name(fmt.fmt.pix_mp.pixelformat), fmt.fmt.pix_mp.pixelformat,
			fmt.fmt.pix_mp.width, fmt.fmt.pix_mp.height,
			v4l2_field_name(fmt.fmt.pix_mp.field),
			fmt.fmt.pix_mp.num_planes);

		for (i = 0; i < fmt.fmt.pix_mp.num_planes; i++) {
			printf(" * Stride %u, buffer size %u\n",
				fmt.fmt.pix_mp.plane_fmt[i].bytesperline,
				fmt.fmt.pix_mp.plane_fmt[i].sizeimage);
		}
	} else if (video_is_meta(dev)) {
		printf("Meta-data format: %s (%08x) buffer size %u\n",
			v4l2_format_name(fmt.fmt.meta.dataformat), fmt.fmt.meta.dataformat,
					 fmt.fmt.meta.buffersize);
	} else {
		printf("Video format set: %s (%08x) %ux%u (stride %u) field %s buffer size %u\n",
			v4l2_format_name(fmt.fmt.pix.pixelformat), fmt.fmt.pix.pixelformat,
			fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix.bytesperline,
			v4l2_field_name(fmt.fmt.pix.field),
			fmt.fmt.pix.sizeimage);
	}

	return 0;
}

static int video_set_framerate(struct device *dev, struct v4l2_fract *time_per_frame)
{
	struct v4l2_streamparm parm;
	int ret;

	memset(&parm, 0, sizeof parm);
	parm.type = dev->type;

	ret = ioctl(dev->fd, VIDIOC_G_PARM, &parm);
	if (ret < 0) {
		printf("Unable to get frame rate: %s (%d).\n",
			strerror(errno), errno);
		return ret;
	}

	printf("Current frame rate: %u/%u\n",
		parm.parm.capture.timeperframe.numerator,
		parm.parm.capture.timeperframe.denominator);

	printf("Setting frame rate to: %u/%u\n",
		time_per_frame->numerator,
		time_per_frame->denominator);

	parm.parm.capture.timeperframe.numerator = time_per_frame->numerator;
	parm.parm.capture.timeperframe.denominator = time_per_frame->denominator;

	ret = ioctl(dev->fd, VIDIOC_S_PARM, &parm);
	if (ret < 0) {
		printf("Unable to set frame rate: %s (%d).\n", strerror(errno),
			errno);
		return ret;
	}

	ret = ioctl(dev->fd, VIDIOC_G_PARM, &parm);
	if (ret < 0) {
		printf("Unable to get frame rate: %s (%d).\n", strerror(errno),
			errno);
		return ret;
	}

	printf("Frame rate set: %u/%u\n",
		parm.parm.capture.timeperframe.numerator,
		parm.parm.capture.timeperframe.denominator);
	return 0;
}

static int video_buffer_mmap(struct device *dev, struct buffer *buffer,
			     struct v4l2_buffer *v4l2buf)
{
	unsigned int length;
	unsigned int offset;
	unsigned int i;

	for (i = 0; i < dev->num_planes; i++) {
		if (video_is_mplane(dev)) {
			length = v4l2buf->m.planes[i].length;
			offset = v4l2buf->m.planes[i].m.mem_offset;
		} else {
			length = v4l2buf->length;
			offset = v4l2buf->m.offset;
		}

		buffer->mem[i] = mmap(0, length, PROT_READ | PROT_WRITE, MAP_SHARED,
				      dev->fd, offset);
		if (buffer->mem[i] == MAP_FAILED) {
			printf("Unable to map buffer %u/%u: %s (%d)\n",
			       buffer->idx, i, strerror(errno), errno);
			return -1;
		}

		buffer->size[i] = length;
		buffer->padding[i] = 0;

		printf("Buffer %u/%u mapped at address %p.\n",
		       buffer->idx, i, buffer->mem[i]);
	}

	return 0;
}

static int video_buffer_munmap(struct device *dev, struct buffer *buffer)
{
	unsigned int i;
	int ret;

	for (i = 0; i < dev->num_planes; i++) {
		ret = munmap(buffer->mem[i], buffer->size[i]);
		if (ret < 0) {
			printf("Unable to unmap buffer %u/%u: %s (%d)\n",
			       buffer->idx, i, strerror(errno), errno);
		}

		buffer->mem[i] = NULL;
	}

	return 0;
}

static int video_buffer_alloc_userptr(struct device *dev, struct buffer *buffer,
				      struct v4l2_buffer *v4l2buf,
				      unsigned int offset, unsigned int padding)
{
	int page_size = getpagesize();
	unsigned int length;
	unsigned int i;
	int ret;

	for (i = 0; i < dev->num_planes; i++) {
		if (video_is_mplane(dev))
			length = v4l2buf->m.planes[i].length;
		else
			length = v4l2buf->length;

		ret = posix_memalign(&buffer->mem[i], page_size,
				     length + offset + padding);
		if (ret < 0) {
			printf("Unable to allocate buffer %u/%u (%d)\n",
			       buffer->idx, i, ret);
			return -ENOMEM;
		}

		buffer->mem[i] += offset;
		buffer->size[i] = length;
		buffer->padding[i] = padding;

		printf("Buffer %u/%u allocated at address %p.\n",
		       buffer->idx, i, buffer->mem[i]);
	}

	return 0;
}

static void video_buffer_free_userptr(struct device *dev, struct buffer *buffer)
{
	unsigned int i;

	for (i = 0; i < dev->num_planes; i++) {
		free(buffer->mem[i]);
		buffer->mem[i] = NULL;
	}
}

static void video_buffer_fill_userptr(struct device *dev, struct buffer *buffer,
				      struct v4l2_buffer *v4l2buf)
{
	unsigned int i;

	if (!video_is_mplane(dev)) {
		v4l2buf->m.userptr = (unsigned long)buffer->mem[0];
		return;
	}

	for (i = 0; i < dev->num_planes; i++)
		v4l2buf->m.planes[i].m.userptr = (unsigned long)buffer->mem[i];
}

static void get_ts_flags(uint32_t flags, const char **ts_type, const char **ts_source)
{
	switch (flags & V4L2_BUF_FLAG_TIMESTAMP_MASK) {
	case V4L2_BUF_FLAG_TIMESTAMP_UNKNOWN:
		*ts_type = "unk";
		break;
	case V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC:
		*ts_type = "mono";
		break;
	case V4L2_BUF_FLAG_TIMESTAMP_COPY:
		*ts_type = "copy";
		break;
	default:
		*ts_type = "inv";
	}
	switch (flags & V4L2_BUF_FLAG_TSTAMP_SRC_MASK) {
	case V4L2_BUF_FLAG_TSTAMP_SRC_EOF:
		*ts_source = "EoF";
		break;
	case V4L2_BUF_FLAG_TSTAMP_SRC_SOE:
		*ts_source = "SoE";
		break;
	default:
		*ts_source = "inv";
	}
}

static int video_alloc_buffers(struct device *dev, int nbufs,
	unsigned int offset, unsigned int padding)
{
	struct v4l2_plane planes[VIDEO_MAX_PLANES];
	struct v4l2_requestbuffers rb;
	struct v4l2_buffer buf;
	struct buffer *buffers;
	unsigned int i;
	int ret;

	memset(&rb, 0, sizeof rb);
	rb.count = nbufs;
	rb.type = dev->type;
	rb.memory = dev->memtype;

	ret = ioctl(dev->fd, VIDIOC_REQBUFS, &rb);
	if (ret < 0) {
		printf("Unable to request buffers: %s (%d).\n", strerror(errno),
			errno);
		return ret;
	}

	printf("%u buffers requested.\n", rb.count);

	buffers = malloc(rb.count * sizeof buffers[0]);
	if (buffers == NULL)
		return -ENOMEM;

	/* Map the buffers. */
	for (i = 0; i < rb.count; ++i) {
		const char *ts_type, *ts_source;

		memset(&buf, 0, sizeof buf);
		memset(planes, 0, sizeof planes);

		buf.index = i;
		buf.type = dev->type;
		buf.memory = dev->memtype;
		buf.length = VIDEO_MAX_PLANES;
		buf.m.planes = planes;

		ret = ioctl(dev->fd, VIDIOC_QUERYBUF, &buf);
		if (ret < 0) {
			printf("Unable to query buffer %u: %s (%d).\n", i,
				strerror(errno), errno);
			return ret;
		}
		get_ts_flags(buf.flags, &ts_type, &ts_source);
		printf("length: %u offset: %u timestamp type/source: %s/%s\n",
		       buf.length, buf.m.offset, ts_type, ts_source);

		buffers[i].idx = i;

		switch (dev->memtype) {
		case V4L2_MEMORY_MMAP:
			ret = video_buffer_mmap(dev, &buffers[i], &buf);
			break;

		case V4L2_MEMORY_USERPTR:
			ret = video_buffer_alloc_userptr(dev, &buffers[i], &buf, offset, padding);
			break;

		default:
			break;
		}

		if (ret < 0)
			return ret;
	}

	dev->timestamp_type = buf.flags & V4L2_BUF_FLAG_TIMESTAMP_MASK;
	dev->buffers = buffers;
	dev->nbufs = rb.count;
	return 0;
}

static int video_free_buffers(struct device *dev)
{
	struct v4l2_requestbuffers rb;
	unsigned int i;
	int ret;

	if (dev->nbufs == 0)
		return 0;

	for (i = 0; i < dev->nbufs; ++i) {
		switch (dev->memtype) {
		case V4L2_MEMORY_MMAP:
			ret = video_buffer_munmap(dev, &dev->buffers[i]);
			if (ret < 0)
				return ret;
			break;
		case V4L2_MEMORY_USERPTR:
			video_buffer_free_userptr(dev, &dev->buffers[i]);
			break;
		default:
			break;
		}
	}

	memset(&rb, 0, sizeof rb);
	rb.count = 0;
	rb.type = dev->type;
	rb.memory = dev->memtype;

	ret = ioctl(dev->fd, VIDIOC_REQBUFS, &rb);
	if (ret < 0) {
		printf("Unable to release buffers: %s (%d).\n",
			strerror(errno), errno);
		return ret;
	}

	printf("%u buffers released.\n", dev->nbufs);

	free(dev->buffers);
	dev->nbufs = 0;
	dev->buffers = NULL;

	return 0;
}

static int video_queue_buffer(struct device *dev, int index, enum buffer_fill_mode fill)
{
	struct v4l2_buffer buf;
	struct v4l2_plane planes[VIDEO_MAX_PLANES];
	int ret;
	unsigned int i;

	memset(&buf, 0, sizeof buf);
	memset(&planes, 0, sizeof planes);

	buf.index = index;
	buf.type = dev->type;
	buf.memory = dev->memtype;

	if (video_is_output(dev)) {
		buf.flags = dev->buffer_output_flags;
		if (dev->timestamp_type == V4L2_BUF_FLAG_TIMESTAMP_COPY) {
			struct timespec ts;

			clock_gettime(CLOCK_MONOTONIC, &ts);
			buf.timestamp.tv_sec = ts.tv_sec;
			buf.timestamp.tv_usec = ts.tv_nsec / 1000;
		}
	}

	if (video_is_mplane(dev)) {
		buf.m.planes = planes;
		buf.length = dev->num_planes;
	}

	if (dev->memtype == V4L2_MEMORY_USERPTR) {
		if (video_is_mplane(dev)) {
			for (i = 0; i < dev->num_planes; i++) {
				buf.m.planes[i].m.userptr = (unsigned long)
					dev->buffers[index].mem[i];
				buf.m.planes[i].length =
					dev->buffers[index].size[i];
			}
		} else {
			buf.m.userptr = (unsigned long)dev->buffers[index].mem[0];
			buf.length = dev->buffers[index].size[0];
		}
	}

	for (i = 0; i < dev->num_planes; i++) {
		if (video_is_output(dev)) {
			if (video_is_mplane(dev))
				buf.m.planes[i].bytesused = dev->patternsize[i];
			else
				buf.bytesused = dev->patternsize[i];

			memcpy(dev->buffers[buf.index].mem[i], dev->pattern[i],
			       dev->patternsize[i]);
		} else {
			if (fill & BUFFER_FILL_FRAME)
				memset(dev->buffers[buf.index].mem[i], 0x55,
				       dev->buffers[index].size[i]);
			if (fill & BUFFER_FILL_PADDING)
				memset(dev->buffers[buf.index].mem[i] +
					dev->buffers[index].size[i],
				       0x55, dev->buffers[index].padding[i]);
		}
	}

	ret = ioctl(dev->fd, VIDIOC_QBUF, &buf);
	if (ret < 0)
		printf("Unable to queue buffer: %s (%d).\n",
			strerror(errno), errno);

	return ret;
}

static int video_enable(struct device *dev, int enable)
{
	int type = dev->type;
	int ret;

	ret = ioctl(dev->fd, enable ? VIDIOC_STREAMON : VIDIOC_STREAMOFF, &type);
	if (ret < 0) {
		printf("Unable to %s streaming: %s (%d).\n",
			enable ? "start" : "stop", strerror(errno), errno);
		return ret;
	}

	return 0;
}

static int video_for_each_control(struct device *dev,
				  int(*callback)(struct device *dev, const struct v4l2_query_ext_ctrl *query))
{
	struct v4l2_query_ext_ctrl query;
	unsigned int nctrls = 0;
	unsigned int id;
	int ret;

	id = 0;
	while (1) {
		id |= V4L2_CTRL_FLAG_NEXT_CTRL | V4L2_CTRL_FLAG_NEXT_COMPOUND;

		ret = query_control(dev, id, &query);
		if (ret == -EINVAL)
			break;
		if (ret < 0)
			return ret;

		id = query.id;

		ret = callback(dev, &query);
		if (ret < 0)
			return ret;

		if (ret)
			nctrls++;
	}

	return nctrls;
}

static void video_query_menu(struct device *dev,
			     const struct v4l2_query_ext_ctrl *query,
			     unsigned int value)
{
	struct v4l2_querymenu menu;
	int ret;

	for (menu.index = query->minimum;
	     menu.index <= (unsigned)query->maximum; menu.index++) {
		menu.id = query->id;
		ret = ioctl(dev->fd, VIDIOC_QUERYMENU, &menu);
		if (ret < 0)
			continue;

		if (query->type == V4L2_CTRL_TYPE_MENU)
			printf("  %u: %.32s%s\n", menu.index, menu.name,
			       menu.index == value ? " (*)" : "");
		else
			printf("  %u: %lld%s\n", menu.index, menu.value,
			       menu.index == value ? " (*)" : "");
	};
}

static void video_print_control_array(const struct v4l2_query_ext_ctrl *query,
				      struct v4l2_ext_control *ctrl)
{
	unsigned int i;

	printf("{");

	for (i = 0; i < query->elems; ++i) {
		switch (query->type) {
		case V4L2_CTRL_TYPE_U8:
			printf("%u", ctrl->p_u8[i]);
			break;
		case V4L2_CTRL_TYPE_U16:
			printf("%u", ctrl->p_u16[i]);
			break;
		case V4L2_CTRL_TYPE_U32:
			printf("%u", ctrl->p_u32[i]);
			break;
		}

		if (i != query->elems - 1)
			printf(", ");
	}

	printf("}");
}

static void video_print_control_value(const struct v4l2_query_ext_ctrl *query,
				      struct v4l2_ext_control *ctrl)
{
	if (query->nr_of_dims == 0) {
		switch (query->type) {
		case V4L2_CTRL_TYPE_INTEGER:
		case V4L2_CTRL_TYPE_BOOLEAN:
		case V4L2_CTRL_TYPE_MENU:
		case V4L2_CTRL_TYPE_INTEGER_MENU:
			printf("%d", ctrl->value);
			break;
		case V4L2_CTRL_TYPE_BITMASK:
			printf("0x%08x", ctrl->value);
			break;
		case V4L2_CTRL_TYPE_INTEGER64:
			printf("%lld", ctrl->value64);
			break;
		case V4L2_CTRL_TYPE_STRING:
			printf("%s", ctrl->string);
			break;
		}

		return;
	}

	switch (query->type) {
	case V4L2_CTRL_TYPE_U8:
	case V4L2_CTRL_TYPE_U16:
	case V4L2_CTRL_TYPE_U32:
		video_print_control_array(query, ctrl);
		break;
	default:
		printf("unsupported type %u", query->type);
		break;
	}
}

static int video_get_control(struct device *dev,
			     const struct v4l2_query_ext_ctrl *query,
			     bool full)
{
	struct v4l2_ext_control ctrl;
	unsigned int i;
	int ret;

	if (query->flags & V4L2_CTRL_FLAG_DISABLED)
		return 0;

	if (query->type == V4L2_CTRL_TYPE_CTRL_CLASS) {
		printf("--- %s (class 0x%08x) ---\n", query->name, query->id);
		return 0;
	}

	if (full) {
		printf("control 0x%08x `%s' min %lld max %lld step %lld default %lld ",
			query->id, query->name, query->minimum, query->maximum,
			query->step, query->default_value);
		if (query->nr_of_dims) {
			for (i = 0; i < query->nr_of_dims; ++i)
				printf("[%u]", query->dims[i]);
			printf(" ");
		}
	} else {
		printf("control 0x%08x ", query->id);
	}

	if (query->type == V4L2_CTRL_TYPE_BUTTON) {
		/* Button controls have no current value. */
		printf("\n");
		return 1;
	}

	printf("current ");

	ret = get_control(dev, query, &ctrl, V4L2_CTRL_WHICH_CUR_VAL);
	if (ret < 0) {
		printf("n/a\n");
		printf("unable to get control 0x%8.8x: %s (%d).\n",
			query->id, strerror(errno), errno);
	} else {
		video_print_control_value(query, &ctrl);
		printf("\n");
	}

	if (query->flags & V4L2_CTRL_FLAG_HAS_PAYLOAD)
		free(ctrl.ptr);

	if (!full)
		return 1;

	if (query->type == V4L2_CTRL_TYPE_MENU ||
	    query->type == V4L2_CTRL_TYPE_INTEGER_MENU)
		video_query_menu(dev, query, ctrl.value);

	return 1;
}

static int __video_get_control(struct device *dev,
			       const struct v4l2_query_ext_ctrl *query)
{
	return video_get_control(dev, query, true);
}

static int video_parse_control_array(const struct v4l2_query_ext_ctrl *query,
				     struct v4l2_ext_control *ctrl,
				     const char *val)
{
	unsigned int i;
	char *endptr;
	__u32 value;

	for ( ; isspace(*val); ++val) { }

	if (*val == '<') {
		/* Read the control value from the given file. */
		ssize_t size;
		int fd;

		val++;
		fd = open(val, O_RDONLY);
		if (fd < 0) {
			printf("unable to open control file `%s'\n", val);
			return -EINVAL;
		}

		size = read(fd, ctrl->ptr, ctrl->size);
		if (size != (ssize_t)ctrl->size) {
			printf("error reading control file `%s' (%s)\n", val,
			       strerror(errno));
			close(fd);
			return -EINVAL;
		}

		close(fd);
		return 0;
	}

	if (*val++ != '{')
		return -EINVAL;

	for (i = 0; i < query->elems; ++i) {
		for ( ; isspace(*val); ++val) { }

		switch (query->type) {
		case V4L2_CTRL_TYPE_U8:
		case V4L2_CTRL_TYPE_U16:
		case V4L2_CTRL_TYPE_U32:
		default:
			value = strtoul(val, &endptr, 0);
			break;
		}

		if (endptr == NULL)
			return -EINVAL;

		switch (query->type) {
		case V4L2_CTRL_TYPE_U8:
			ctrl->p_u8[i] = value;
			break;
		case V4L2_CTRL_TYPE_U16:
			ctrl->p_u16[i] = value;
			break;
		case V4L2_CTRL_TYPE_U32:
			ctrl->p_u32[i] = value;
			break;
		}

		val = endptr;
		for ( ; isspace(*val); ++val) { }
		if (*val != ',')
			break;
		val++;
	}

	if (i < query->elems - 1)
		return -EINVAL;

	for ( ; isspace(*val); ++val) { }
	if (*val++ != '}')
		return -EINVAL;

	for ( ; isspace(*val); ++val) { }
	if (*val++ != '\0')
		return -EINVAL;

	return 0;
}

static void video_set_control(struct device *dev, unsigned int id,
			      const char *val)
{
	struct v4l2_query_ext_ctrl query;
	struct v4l2_ext_control ctrl;
	char *endptr;
	int ret;

	ret = query_control(dev, id, &query);
	if (ret < 0)
		return;

	memset(&ctrl, 0, sizeof(ctrl));

	if (query.nr_of_dims == 0) {
		switch (query.type) {
		case V4L2_CTRL_TYPE_INTEGER:
		case V4L2_CTRL_TYPE_BOOLEAN:
		case V4L2_CTRL_TYPE_MENU:
		case V4L2_CTRL_TYPE_INTEGER_MENU:
		case V4L2_CTRL_TYPE_BITMASK:
			ctrl.value = strtol(val, &endptr, 0);
			if (*endptr != 0) {
				printf("Invalid control value '%s'\n", val);
				return;
			}
			break;
		case V4L2_CTRL_TYPE_INTEGER64:
			ctrl.value64 = strtoll(val, &endptr, 0);
			if (*endptr != 0) {
				printf("Invalid control value '%s'\n", val);
				return;
			}
			break;
		case V4L2_CTRL_TYPE_STRING:
			ctrl.size = query.elem_size;
			ctrl.ptr = malloc(ctrl.size);
			if (ctrl.ptr == NULL)
				return;
			strncpy(ctrl.string, val, ctrl.size);
			break;
		default:
			printf("Unsupported control type\n");
			return;
		}
	} else {
		switch (query.type) {
		case V4L2_CTRL_TYPE_U8:
		case V4L2_CTRL_TYPE_U16:
		case V4L2_CTRL_TYPE_U32:
			ctrl.size = query.elem_size * query.elems;
			ctrl.ptr = malloc(ctrl.size);
			if (ctrl.ptr == NULL)
				return;
			ret = video_parse_control_array(&query, &ctrl, val);
			if (ret < 0) {
				printf("Invalid compound control value '%s'\n", val);
				return;
			}
			break;
		default:
			printf("Unsupported control type %u\n", query.type);
			break;
		}
	}

	ret = set_control(dev, &query, &ctrl);
	if (ret < 0) {
		printf("unable to set control 0x%8.8x: %s (%d).\n",
			id, strerror(errno), errno);
	} else {
		printf("Control 0x%08x set to %s, is ", id, val);

		video_print_control_value(&query, &ctrl);
		printf("\n");
	}

	if ((query.flags & V4L2_CTRL_FLAG_HAS_PAYLOAD) && ctrl.ptr)
		free(ctrl.ptr);
}

static void video_list_controls(struct device *dev)
{
	int ret;

	ret = video_for_each_control(dev, __video_get_control);
	if (ret < 0)
		return;

	if (ret)
		printf("%u control%s found.\n", ret, ret > 1 ? "s" : "");
	else
		printf("No control found.\n");
}

static int video_reset_control(struct device *dev,
			       const struct v4l2_query_ext_ctrl *query)
{
       struct v4l2_ext_control ctrl = { .value = query->default_value, };
       int ret;

	if (query->flags & V4L2_CTRL_FLAG_DISABLED)
		return 0;

	if (query->type == V4L2_CTRL_TYPE_CTRL_CLASS)
		return 0;

	/*
	 * For controls with payloads the default value must be retrieved with
	 * a VIDIOC_G_EXT_CTRLS call. If the V4L2_CTRL_WHICH_DEF_VAL flag isn't
	 * supported by the kernel (it got introduced in v4.5, while controls
	 * with payloads were introduced in v3.17), there isn't much we can do,
	 * so skip resetting the control.
	 */
	if (query->flags & V4L2_CTRL_FLAG_HAS_PAYLOAD) {
		ret = get_control(dev, query, &ctrl, V4L2_CTRL_WHICH_DEF_VAL);
		if (ret < 0)
			return 0;
	}

	set_control(dev, query, &ctrl);

	if (query->flags & V4L2_CTRL_FLAG_HAS_PAYLOAD)
		free(ctrl.ptr);

	return 1;
}

static void video_reset_controls(struct device *dev)
{
	int ret;

	ret = video_for_each_control(dev, video_reset_control);
	if (ret < 0)
		return;

	if (ret)
		printf("%u control%s reset.\n", ret, ret > 1 ? "s" : "");
}

static void video_enum_frame_intervals(struct device *dev, __u32 pixelformat,
	unsigned int width, unsigned int height)
{
	struct v4l2_frmivalenum ival;
	unsigned int i;
	int ret;

	for (i = 0; ; ++i) {
		memset(&ival, 0, sizeof ival);
		ival.index = i;
		ival.pixel_format = pixelformat;
		ival.width = width;
		ival.height = height;
		ret = ioctl(dev->fd, VIDIOC_ENUM_FRAMEINTERVALS, &ival);
		if (ret < 0)
			break;

		if (i != ival.index)
			printf("Warning: driver returned wrong ival index "
				"%u.\n", ival.index);
		if (pixelformat != ival.pixel_format)
			printf("Warning: driver returned wrong ival pixel "
				"format %08x.\n", ival.pixel_format);
		if (width != ival.width)
			printf("Warning: driver returned wrong ival width "
				"%u.\n", ival.width);
		if (height != ival.height)
			printf("Warning: driver returned wrong ival height "
				"%u.\n", ival.height);

		if (i != 0)
			printf(", ");

		switch (ival.type) {
		case V4L2_FRMIVAL_TYPE_DISCRETE:
			printf("%u/%u",
				ival.discrete.numerator,
				ival.discrete.denominator);
			break;

		case V4L2_FRMIVAL_TYPE_CONTINUOUS:
			printf("%u/%u - %u/%u",
				ival.stepwise.min.numerator,
				ival.stepwise.min.denominator,
				ival.stepwise.max.numerator,
				ival.stepwise.max.denominator);
			return;

		case V4L2_FRMIVAL_TYPE_STEPWISE:
			printf("%u/%u - %u/%u (by %u/%u)",
				ival.stepwise.min.numerator,
				ival.stepwise.min.denominator,
				ival.stepwise.max.numerator,
				ival.stepwise.max.denominator,
				ival.stepwise.step.numerator,
				ival.stepwise.step.denominator);
			return;

		default:
			break;
		}
	}
}

static void video_enum_frame_sizes(struct device *dev, __u32 pixelformat)
{
	struct v4l2_frmsizeenum frame;
	unsigned int i;
	int ret;

	for (i = 0; ; ++i) {
		memset(&frame, 0, sizeof frame);
		frame.index = i;
		frame.pixel_format = pixelformat;
		ret = ioctl(dev->fd, VIDIOC_ENUM_FRAMESIZES, &frame);
		if (ret < 0)
			break;

		if (i != frame.index)
			printf("Warning: driver returned wrong frame index "
				"%u.\n", frame.index);
		if (pixelformat != frame.pixel_format)
			printf("Warning: driver returned wrong frame pixel "
				"format %08x.\n", frame.pixel_format);

		switch (frame.type) {
		case V4L2_FRMSIZE_TYPE_DISCRETE:
			printf("\tFrame size: %ux%u (", frame.discrete.width,
				frame.discrete.height);
			video_enum_frame_intervals(dev, frame.pixel_format,
				frame.discrete.width, frame.discrete.height);
			printf(")\n");
			break;

		case V4L2_FRMSIZE_TYPE_CONTINUOUS:
			printf("\tFrame size: %ux%u - %ux%u (",
				frame.stepwise.min_width,
				frame.stepwise.min_height,
				frame.stepwise.max_width,
				frame.stepwise.max_height);
			video_enum_frame_intervals(dev, frame.pixel_format,
				frame.stepwise.max_width,
				frame.stepwise.max_height);
			printf(")\n");
			break;

		case V4L2_FRMSIZE_TYPE_STEPWISE:
			printf("\tFrame size: %ux%u - %ux%u (by %ux%u) (\n",
				frame.stepwise.min_width,
				frame.stepwise.min_height,
				frame.stepwise.max_width,
				frame.stepwise.max_height,
				frame.stepwise.step_width,
				frame.stepwise.step_height);
			video_enum_frame_intervals(dev, frame.pixel_format,
				frame.stepwise.max_width,
				frame.stepwise.max_height);
			printf(")\n");
			break;

		default:
			break;
		}
	}
}

static void video_enum_formats(struct device *dev, enum v4l2_buf_type type)
{
	struct v4l2_fmtdesc fmt;
	unsigned int i;
	int ret;

	for (i = 0; ; ++i) {
		memset(&fmt, 0, sizeof fmt);
		fmt.index = i;
		fmt.type = type;
		ret = ioctl(dev->fd, VIDIOC_ENUM_FMT, &fmt);
		if (ret < 0)
			break;

		if (i != fmt.index)
			printf("Warning: driver returned wrong format index "
				"%u.\n", fmt.index);
		if (type != fmt.type)
			printf("Warning: driver returned wrong format type "
				"%u.\n", fmt.type);

		printf("\tFormat %u: %s (%08x)\n", i,
			v4l2_format_name(fmt.pixelformat), fmt.pixelformat);
		printf("\tType: %s (%u)\n", v4l2_buf_type_name(fmt.type),
			fmt.type);
		printf("\tName: %.32s\n", fmt.description);
		video_enum_frame_sizes(dev, fmt.pixelformat);
		printf("\n");
	}
}

static void video_enum_inputs(struct device *dev)
{
	struct v4l2_input input;
	unsigned int i;
	int ret;

	for (i = 0; ; ++i) {
		memset(&input, 0, sizeof input);
		input.index = i;
		ret = ioctl(dev->fd, VIDIOC_ENUMINPUT, &input);
		if (ret < 0)
			break;

		if (i != input.index)
			printf("Warning: driver returned wrong input index "
				"%u.\n", input.index);

		printf("\tInput %u: %s.\n", i, input.name);
	}

	printf("\n");
}

static int video_get_input(struct device *dev)
{
	__u32 input;
	int ret;

	ret = ioctl(dev->fd, VIDIOC_G_INPUT, &input);
	if (ret < 0) {
		printf("Unable to get current input: %s (%d).\n",
			strerror(errno), errno);
		return ret;
	}

	return input;
}

static int video_set_input(struct device *dev, unsigned int input)
{
	__u32 _input = input;
	int ret;

	ret = ioctl(dev->fd, VIDIOC_S_INPUT, &_input);
	if (ret < 0)
		printf("Unable to select input %u: %s (%d).\n", input,
			strerror(errno), errno);

	return ret;
}

static int video_set_quality(struct device *dev, unsigned int quality)
{
	struct v4l2_jpegcompression jpeg;
	int ret;

	if (quality == (unsigned int)-1)
		return 0;

	memset(&jpeg, 0, sizeof jpeg);
	jpeg.quality = quality;

	ret = ioctl(dev->fd, VIDIOC_S_JPEGCOMP, &jpeg);
	if (ret < 0) {
		printf("Unable to set quality to %u: %s (%d).\n", quality,
			strerror(errno), errno);
		return ret;
	}

	ret = ioctl(dev->fd, VIDIOC_G_JPEGCOMP, &jpeg);
	if (ret >= 0)
		printf("Quality set to %u\n", jpeg.quality);

	return 0;
}

static int video_load_test_pattern(struct device *dev, const char *filename)
{
	unsigned int plane;
	unsigned int size;
	int fd = -1;
	int ret;

	if (filename != NULL) {
		fd = open(filename, O_RDONLY);
		if (fd == -1) {
			printf("Unable to open test pattern file '%s': %s (%d).\n",
				filename, strerror(errno), errno);
			return -errno;
		}
	}

	/* Load or generate the test pattern */
	for (plane = 0; plane < dev->num_planes; plane++) {
		size = dev->buffers[0].size[plane];
		dev->pattern[plane] = malloc(size);
		if (dev->pattern[plane] == NULL) {
			ret = -ENOMEM;
			goto done;
		}

		if (filename != NULL) {
			ret = read(fd, dev->pattern[plane], size);
			if (ret != (int)size && dev->plane_fmt[plane].bytesperline != 0) {
				printf("Test pattern file size %u doesn't match image size %u\n",
					ret, size);
				ret = -EINVAL;
				goto done;
			}
		} else {
			uint8_t *data = dev->pattern[plane];
			unsigned int i;

			if (dev->plane_fmt[plane].bytesperline == 0) {
				printf("Compressed format detected for plane %u and no test pattern filename given.\n"
					"The test pattern can't be generated automatically.\n", plane);
				ret = -EINVAL;
				goto done;
			}

			for (i = 0; i < dev->plane_fmt[plane].sizeimage; ++i)
				*data++ = i;
		}

		dev->patternsize[plane] = size;
	}

	ret = 0;

done:
	if (fd != -1)
		close(fd);

	return ret;
}

static int video_prepare_capture(struct device *dev, int nbufs, unsigned int offset,
				 const char *filename, enum buffer_fill_mode fill)
{
	unsigned int padding;
	int ret;

	/* Allocate and map buffers. */
	padding = (fill & BUFFER_FILL_PADDING) ? 4096 : 0;
	if ((ret = video_alloc_buffers(dev, nbufs, offset, padding)) < 0)
		return ret;

	if (video_is_output(dev)) {
		ret = video_load_test_pattern(dev, filename);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int video_queue_all_buffers(struct device *dev, enum buffer_fill_mode fill)
{
	unsigned int i;
	int ret;

	/* Queue the buffers. */
	for (i = 0; i < dev->nbufs; ++i) {
		ret = video_queue_buffer(dev, i, fill);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static void video_verify_buffer(struct device *dev, struct v4l2_buffer *buf)
{
	struct buffer *buffer = &dev->buffers[buf->index];
	unsigned int plane;
	unsigned int i;

	for (plane = 0; plane < dev->num_planes; ++plane) {
		const uint8_t *data = buffer->mem[plane] + buffer->size[plane];
		unsigned int errors = 0;
		unsigned int dirty = 0;
		unsigned int length;

		if (video_is_mplane(dev))
			length = buf->m.planes[plane].bytesused;
		else
			length = buf->bytesused;

		if (dev->plane_fmt[plane].sizeimage &&
		    dev->plane_fmt[plane].sizeimage != length)
			printf("Warning: bytes used %u != image size %u for plane %u\n",
			       length, dev->plane_fmt[plane].sizeimage, plane);

		if (buffer->padding[plane] == 0)
			continue;

		for (i = 0; i < buffer->padding[plane]; ++i) {
			if (data[i] != 0x55) {
				errors++;
				dirty = i + 1;
			}
		}

		if (errors) {
			printf("Warning: %u bytes overwritten among %u first padding bytes for plane %u\n",
			       errors, dirty, plane);

			dirty = (dirty + 15) & ~15;
			dirty = dirty > 32 ? 32 : dirty;

			for (i = 0; i < dirty; ++i) {
				printf("%02x ", data[i]);
				if (i % 16 == 15)
					printf("\n");
			}
		}
	}
}

static void video_save_image(struct device *dev, struct v4l2_buffer *buf,
			     const char *pattern, unsigned int sequence)
{
	unsigned int size;
	unsigned int i;
	char *filename;
	const char *p;
	bool append;
	int ret = 0;
	int fd;

	size = strlen(pattern);
	filename = malloc(size + 12);
	if (filename == NULL)
		return;

	p = strchr(pattern, '#');
	if (p != NULL) {
		sprintf(filename, "%.*s%06u%s", (int)(p - pattern), pattern,
			sequence, p + 1);
		append = false;
	} else {
		strcpy(filename, pattern);
		append = true;
	}

	fd = open(filename, O_CREAT | O_WRONLY | (append ? O_APPEND : O_TRUNC),
		  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	free(filename);
	if (fd == -1)
		return;

	for (i = 0; i < dev->num_planes; i++) {
		void *data = dev->buffers[buf->index].mem[i];
		unsigned int length;

		if (video_is_mplane(dev)) {
			length = buf->m.planes[i].bytesused;

			if (!dev->write_data_prefix) {
				data += buf->m.planes[i].data_offset;
				length -= buf->m.planes[i].data_offset;
			}

		} else {
			length = buf->bytesused;
		}

		ret = write(fd, data, length);
		if (ret < 0) {
			printf("write error: %s (%d)\n", strerror(errno), errno);
			break;
		} else if (ret != (int)length)
			printf("write error: only %d bytes written instead of %u\n",
			       ret, length);
	}
	close(fd);
}

unsigned int video_buffer_bytes_used(struct device *dev, struct v4l2_buffer *buf)
{
	unsigned int bytesused = 0;
	unsigned int i;

	if (!video_is_mplane(dev))
		return buf->bytesused;

	for (i = 0; i < dev->num_planes; i++)
		bytesused += buf->m.planes[i].bytesused;

	return bytesused;
}

static struct device dev = { 0 };
static unsigned int nframes = 0;
static struct timeval last;
static struct timespec start;
int dequeue_buf(void** data, unsigned int* length, int* buf_index)
{
	struct v4l2_plane planes[VIDEO_MAX_PLANES];
	struct v4l2_buffer buf;
	
	struct timespec ts;
	double fps;
	int ret;

  const char *ts_type, *ts_source;

	/* Dequeue a buffer. */
	memset(&buf, 0, sizeof buf);
	memset(planes, 0, sizeof planes);

	buf.type = dev.type;
	buf.memory = dev.memtype;
	buf.length = VIDEO_MAX_PLANES;
	buf.m.planes = planes;

	ret = ioctl(dev.fd, VIDIOC_DQBUF, &buf);
	if (ret < 0) {
		if (errno != EIO) {
			printf("Unable to dequeue buffer: %s (%d).\n",
				strerror(errno), errno);
			return -1;
		}
		buf.type = dev.type;
		buf.memory = dev.memtype;
	}

	if (video_is_capture(&dev))
		video_verify_buffer(&dev, &buf);


	fps = (buf.timestamp.tv_sec - last.tv_sec) * 1000000
	    + buf.timestamp.tv_usec - last.tv_usec;
	fps = fps ? 1000000.0 / fps : 0.0;

  //输出
  *buf_index = buf.index;
  for (int i = 0; i < dev.num_planes; i++) {
		*data = dev.buffers[buf.index].mem[i];

		if (video_is_mplane(&dev)) {
			*length = buf.m.planes[i].bytesused;

			if (!dev.write_data_prefix) {
				*data += buf.m.planes[i].data_offset;
				*length -= buf.m.planes[i].data_offset;
			}

		} else {
			*length = buf.bytesused;
		}
  }

  unsigned int frame_id = ((((unsigned int)((char*)*data)[0]) & 0x000000FF) << 8) + ((char*)*data)[1];
	clock_gettime(CLOCK_MONOTONIC, &ts);
	get_ts_flags(buf.flags, &ts_type, &ts_source);
	/*printf("%u (%u) [%c] %s %u %u B %ld.%06ld %ld.%06ld %.3f fps ts %s/%s\n", frame_id, buf.index,
		(buf.flags & V4L2_BUF_FLAG_ERROR) ? 'E' : '-',
		v4l2_field_name(buf.field),
		buf.sequence, video_buffer_bytes_used(&dev, &buf),
		buf.timestamp.tv_sec, buf.timestamp.tv_usec,
		ts.tv_sec, ts.tv_nsec/1000, fps,
		ts_type, ts_source);*/

	last = buf.timestamp;

	fflush(stdout);

#if 0
	if (ts.tv_sec == start.tv_sec && ts.tv_nsec == start.tv_nsec) {
		printf("Captured %u frames 0 seconds\n", nframes);
		return 0;
	}

	ts.tv_sec -= start.tv_sec;
	ts.tv_nsec -= start.tv_nsec;
	if (ts.tv_nsec < 0) {
		ts.tv_sec--;
		ts.tv_nsec += 1000000000;
	}

	fps = nframes/(ts.tv_nsec/1000.0+1000000.0*ts.tv_sec)*1000000.0;

	printf("Captured %u frames in %lu.%06lu seconds (%f fps).\n",
		nframes, ts.tv_sec, ts.tv_nsec/1000, fps);
#endif
  return 0;
}

#define V4L_BUFFERS_DEFAULT	8
#define V4L_BUFFERS_MAX		32

void uinit()
{
  /* Stop streaming. */
  video_enable(&dev, 0);
  video_free_buffers(&dev);
  video_close(&dev);
}

int queue_buf(int buf_index)
{
  int ret;
  ret = video_queue_buffer(&dev, buf_index, BUFFER_FILL_NONE);
  if (ret < 0) {
    printf("Unable to requeue buffer: %s (%d).\n", strerror(errno), errno);
    return -1;
  }
  return 0;
}

int init()
{
	struct sched_param sched;
	int ret;

	/* Options parsings */
	const struct v4l2_format_info *info;
	/* Use video capture by default if query isn't done. */
	unsigned int capabilities = V4L2_CAP_VIDEO_CAPTURE;
	int do_file = 0, do_capture = 0;
	int do_set_time_per_frame = 0;
	int do_enum_formats = 0, do_set_format = 0;
	int do_enum_inputs = 0, do_set_input = 0;
	int do_list_controls = 0, do_get_control = 0, do_set_control = 0;
	int do_reset_controls = 0;
	int do_sleep_forever = 0, do_requeue_last = 0;
	int do_rt = 0, do_log_status = 0;
	int no_query = 0, do_queue_late = 0;
	char *endptr;
	int c;

	/* Controls */
	int ctrl_name = 0;
	const char *ctrl_value = NULL;

	/* Video buffers */
	enum v4l2_memory memtype = V4L2_MEMORY_MMAP;
	unsigned int pixelformat = V4L2_PIX_FMT_YUYV;
	unsigned int fmt_flags = 0;
	unsigned int width = 640;
	unsigned int height = 480;
	unsigned int stride = 0;
	unsigned int buffer_size = 0;
	unsigned int nbufs = V4L_BUFFERS_DEFAULT;
	unsigned int input = 0;
	unsigned int skip = 0;
	unsigned int quality = (unsigned int)-1;
	unsigned int userptr_offset = 0;
	unsigned int pause_count = (unsigned int)-1;
	struct v4l2_fract time_per_frame = {1, 25};
	enum v4l2_field field = V4L2_FIELD_ANY;

	/* Capture loop */
	enum buffer_fill_mode fill_mode = BUFFER_FILL_NONE;
	unsigned int delay = 0, nframes = (unsigned int)-1;
	const char *filename = "frame-#.bin";

	unsigned int rt_priority = 1;

	video_init(&dev);

	if (pause_count != (unsigned int)-1) {
		ret = pause_init();
		if (ret < 0)
			return 1;
	}

	if ((fill_mode & BUFFER_FILL_PADDING) && memtype != V4L2_MEMORY_USERPTR) {
		printf("Buffer overrun can only be checked in USERPTR mode.\n");
		return 1;
	}

	if (!do_file)
		filename = NULL;

	if (!video_has_fd(&dev)) {
		ret = video_open(&dev, "/dev/video3");
		if (ret < 0)
			return 1;
	}

	if (!no_query) {
		ret = video_querycap(&dev, &capabilities);
		if (ret < 0)
			return 1;
	}

	ret = cap_get_buf_type(capabilities);
	if (ret < 0)
		return 1;

	if (!video_has_valid_buf_type(&dev))
		video_set_buf_type(&dev, ret);

	dev.memtype = memtype;

	if (do_log_status)
		video_log_status(&dev);

	if (do_get_control) {
		struct v4l2_query_ext_ctrl query;

		ret = query_control(&dev, ctrl_name, &query);
		if (ret == 0)
			video_get_control(&dev, &query, false);
	}

	if (do_set_control)
		video_set_control(&dev, ctrl_name, ctrl_value);

	if (do_list_controls)
		video_list_controls(&dev);

	if (do_reset_controls)
		video_reset_controls(&dev);

	if (do_enum_formats) {
		printf("- Available formats:\n");
		video_enum_formats(&dev, V4L2_BUF_TYPE_VIDEO_CAPTURE);
		video_enum_formats(&dev, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
		video_enum_formats(&dev, V4L2_BUF_TYPE_VIDEO_OUTPUT);
		video_enum_formats(&dev, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);
		video_enum_formats(&dev, V4L2_BUF_TYPE_VIDEO_OVERLAY);
		video_enum_formats(&dev, V4L2_BUF_TYPE_META_CAPTURE);
		video_enum_formats(&dev, V4L2_BUF_TYPE_META_OUTPUT);
	}

	if (do_enum_inputs) {
		printf("- Available inputs:\n");
		video_enum_inputs(&dev);
	}

	if (do_set_input) {
		video_set_input(&dev, input);
		ret = video_get_input(&dev);
		printf("Input %d selected\n", ret);
	}

	/* Set the video format. */
	if (do_set_format) {
		if (video_set_format(&dev, width, height, pixelformat, stride,
				     buffer_size, field, fmt_flags) < 0) {
			video_close(&dev);
			return 1;
		}
	}

	if (!no_query || do_capture)
		video_get_format(&dev);

	/* Set the frame rate. */
	if (do_set_time_per_frame) {
		if (video_set_framerate(&dev, &time_per_frame) < 0) {
			video_close(&dev);
			return 1;
		}
	}

	while (do_sleep_forever)
		sleep(1000);

	/* Set the compression quality. */
	if (video_set_quality(&dev, quality) < 0) {
		video_close(&dev);
		return 1;
	}

	if (video_prepare_capture(&dev, nbufs, userptr_offset, filename, fill_mode)) {
		video_close(&dev);
		return 1;
	}

	if (!do_queue_late && video_queue_all_buffers(&dev, fill_mode)) {
		video_close(&dev);
		return 1;
	}

	if (do_rt) {
		memset(&sched, 0, sizeof sched);
		sched.sched_priority = rt_priority;
		ret = sched_setscheduler(0, SCHED_RR, &sched);
		if (ret < 0)
			printf("Failed to select RR scheduler: %s (%d)\n",
				strerror(errno), errno);
	}

  if (pause_count == 0)
		pause_wait();

	/* Start streaming. */
	ret = video_enable(&dev, 1);
	if (ret < 0)
		return -1;

	if (do_queue_late)
		video_queue_all_buffers(&dev, BUFFER_FILL_NONE);

	clock_gettime(CLOCK_MONOTONIC, &start);
	last.tv_sec = start.tv_sec;
	last.tv_usec = start.tv_nsec / 1000;
  return 0;
}



static void save_image(void *data, int length,
			     const char *pattern, unsigned int sequence)
{
	unsigned int size;
	unsigned int i;
	char *filename;
	const char *p;
	bool append;
	int ret = 0;
	int fd;

	size = strlen(pattern);
	filename = malloc(size + 12);
	if (filename == NULL)
		return;

	p = strchr(pattern, '#');
	if (p != NULL) {
		sprintf(filename, "%.*s%06u%s", (int)(p - pattern), pattern,
			sequence, p + 1);
		append = false;
	} else {
		strcpy(filename, pattern);
		append = true;
	}

	fd = open(filename, O_CREAT | O_WRONLY | (append ? O_APPEND : O_TRUNC),
		  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	free(filename);
	if (fd == -1)
		return;

	ret = write(fd, data, length);
	if (ret < 0) {
		printf("write error: %s (%d)\n", strerror(errno), errno);
	} else if (ret != (int)length)
		printf("write error: only %d bytes written instead of %u\n",
		       ret, length);
    
	close(fd);
}
#else
int init()
{
    //printf("test");
    return 0;
}
#endif
