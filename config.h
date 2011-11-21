/*
 * config.h
 *
 * Copyright 2011 Matthew Brush <mbrush@codebrainz.ca>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */
#ifndef CONFIG_H__
#define CONFIG_H__ 1

#include <cv.h>
#include <highgui.h>

#define PROGNAME "digimag"
#define PROGTITLE "Digital Magnifier"
#define COLORS_MAX 16
#define CAMERAS_MAX 16

#if defined(_WIN32) || defined(_WIN64)
#define HAVE_WINDOWS 1
#define DIRSEP "\\"
#elif defined(__unix) || defined(__posix) || defined(__linux)
#define HAVE_UNIX
#define DIRSEP "/"
#elif defined(__APPLE__)
#define HAVE_APPLE
#define DIRSEP "/"
#endif


typedef struct
{
  int version;
  int fullscreen;
  int device;
  int invert;
  char *title;

  int width;
  int height;

  int show_line;
  int line_weight;
  int line_weight_max;
  int line_weight_interval;
  CvScalar line_color;

  int color_pair;
  CvScalar fg_colors[COLORS_MAX];
  CvScalar bg_colors[COLORS_MAX];

  CvCapture *capture;
}
Config;


Config *config_init(void);
void config_save(Config *config);


#endif /* CONFIG_H__ */
