/*
 * main.c
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
#include <stdio.h>
#include <string.h>
#include <cv.h>
#include <highgui.h>
#include "config.h"


/*
 * Converts a single channel binary image into a 3 channel colour image
 * with all of the black replaced with black_repl and all of the white
 * replaced for white_repl.
 *
 * Note: img will be released, the return value is a new image.
 */
static IplImage *apply_color_filter(IplImage *img,
                                    CvScalar *black_repl,
                                    CvScalar *white_repl)
{
  IplImage      *output;
  unsigned char *row;
  int            x, y, r, g, b;

  output = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 3);
  cvMerge(img, img, img, NULL, output);
  cvReleaseImage(&img);

  /* Replace black and white with black_repl and white_repl */
  for (y = 0; y < output->height; y++)
    {
      row = &CV_IMAGE_ELEM(output, unsigned char, y, 0);
      for (x = 0; x < output->width * output->nChannels; x += output->nChannels)
        {
          r = row[x];
          g = row[x+1];
          b = row[x+2];
          if ((r+g+b) == 0)
            {
              row[x+0] = (unsigned char) black_repl->val[0];
              row[x+1] = (unsigned char) black_repl->val[1];
              row[x+2] = (unsigned char) black_repl->val[2];
            }
          else
            {
              row[x+0] = (unsigned char) white_repl->val[0];
              row[x+1] = (unsigned char) white_repl->val[1];
              row[x+2] = (unsigned char) white_repl->val[2];
            }
        }
    }

  return output;
}


/* Handles mouse events on the video */
static void on_mouse_callback(int   event,
                              int   x,
                              int   y,
                              int   flags,
                              void *param)
{
  Config *conf = (Config *) param;

  if (!conf)
    return;

  switch (event)
    {
    /* Toggle fullscreen on double-click of the left mouse button */
    case CV_EVENT_LBUTTONDBLCLK:
      conf->fullscreen = !conf->fullscreen;
      cvSetWindowProperty(conf->title, CV_WND_PROP_FULLSCREEN,
        conf->fullscreen ? CV_WINDOW_FULLSCREEN : CV_WINDOW_NORMAL);
      break;
    default:
      break;
    }
}


/* Goes through any devices in order looking for the next valid one.
 * If init is non-zero, the device id won't be incremented since it's
 * either -1 for automatic (default) or it's the value read from the
 * config file. */
static void open_device(Config *conf)
{
  static int attempts = 0;

  if (conf->device >= CAMERAS_MAX)
    conf->device = 0;

  if (conf->capture)
    cvReleaseCapture(&(conf->capture));

  conf->capture = cvCreateCameraCapture(conf->device);
  attempts++;

  if (!conf->capture && attempts <= CAMERAS_MAX)
    {
      conf->device++;
      open_device(conf);
    }
  else if (attempts > CAMERAS_MAX)
    {
      if (!conf->capture)
        cvReleaseCapture(&(conf->capture));
      fprintf(stderr, "error: unable to find any cameras\n");
      exit(EXIT_FAILURE);
    }
  else
    attempts = 0;
}


int main(int argc, char *argv[])
{
  IplImage   *frame, *grey;
  int         line_pos;
  int         quit = 0;
  char        c;
  const char *name;
  void       *handle;
  Config     *conf;

  conf = config_init();
  if (!conf)
    {
      fprintf(stderr, "error: unable to initialize configuration\n");
      exit(EXIT_FAILURE);
    }

  open_device(conf);

  cvNamedWindow(conf->title, 0);
  cvSetMouseCallback(conf->title, on_mouse_callback, conf);
  handle = cvGetWindowHandle(conf->title);
  name = cvGetWindowName(handle);

  if (conf->fullscreen)
    {
      /* Apply initial fullscreen state without toggling. */
      conf->fullscreen = !conf->fullscreen;
      on_mouse_callback(CV_EVENT_LBUTTONDBLCLK, 0, 0, 0, conf);
    }

  while(!quit)
    {
      frame = cvQueryFrame(conf->capture);

      if(!frame)
        break;

      conf->width = (int) cvGetCaptureProperty(conf->capture, CV_CAP_PROP_FRAME_WIDTH);
      conf->height = (int) cvGetCaptureProperty(conf->capture, CV_CAP_PROP_FRAME_HEIGHT);

      /* Grayscale and threshold */
      grey = cvCreateImage(cvSize(conf->width, conf->height), IPL_DEPTH_8U, 1);
      cvCvtColor(frame, grey, CV_RGB2GRAY);
      cvThreshold(grey, grey, -1, 255,
                  (conf->invert ? CV_THRESH_BINARY_INV : CV_THRESH_BINARY) |
                    CV_THRESH_OTSU /* automatically finds the threshold */);

      /* Apply colour filters and then smooth out the edges by blurring */
      frame = apply_color_filter(grey,
                                 &(conf->bg_colors[conf->color_pair]),
                                 &(conf->fg_colors[conf->color_pair]));
      cvSmooth(frame, frame, CV_GAUSSIAN, 3, 3, 0, 0);

      if (conf->show_line)
        {
          /* Draw a line down the centre of the window */
          line_pos = conf->height / 2;
          cvLine(frame,
                 cvPoint(0, line_pos),
                 cvPoint(conf->width, line_pos),
                 conf->line_color,
                 conf->line_weight,
                 CV_AA, 0);
        }

      cvShowImage(conf->title, frame);
      cvReleaseImage(&frame);

      c = cvWaitKey(30);
      switch (c)
        {
        case 27: /* escape */
          quit = 1;
          break;
        case 'd':
          conf->device++;
          open_device(conf);
          break;
        case ' ':
          conf->show_line = !conf->show_line;
          break;
        case 'l':
          if (conf->line_weight > conf->line_weight_max)
            conf->line_weight = 0;
          else
            conf->line_weight += conf->line_weight_interval;
          if (conf->line_weight > 0)
            conf->show_line = 1;
          break;
        case 'f':
          on_mouse_callback(CV_EVENT_LBUTTONDBLCLK, 0, 0, 0, conf);
          break;
        case 'c':
          conf->color_pair++;
          if (conf->color_pair >= COLORS_MAX)
            conf->color_pair = 0;
          break;
        case 'i':
          conf->invert = !conf->invert;
          break;
        default:
          /*fputc(c, stderr);*/
          break;
        }

      /* hack to exit when the window is closed */
      name = cvGetWindowName(handle);
      if (!name || !name[0])
        break;
    }

  cvReleaseCapture(&(conf->capture));
  cvDestroyWindow(conf->title);

  config_save(conf);
  free(conf->title);

  return 0;
}
