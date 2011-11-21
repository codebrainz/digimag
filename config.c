/*
 * config.c
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

/*
 * TODO: ensure this file is portable
 */

#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <sys/stat.h>
#include "config.h"
#include "ini.h"


static char *color_to_string(CvScalar *color)
{
  char *str_color = calloc(8, sizeof(char));
  snprintf(str_color, 8,
           "#%02x%02x%02x",
           (int) color->val[2],
           (int) color->val[1],
           (int) color->val[0]);
  return str_color;
}


static int string_to_color(const char *str_color, CvScalar *color)
{
  char tmp[7] = { 0 };
  char rs[3] = { 0 };
  char gs[3] = { 0 };
  char bs[3] = { 0 };
  int  r, g, b;

  if (str_color == NULL || strlen(str_color) < 6)
    return -1;

  if (str_color[0] == '#')
    strncpy(tmp, str_color + 1, 6);
  else if (str_color[0] == '0' && (str_color[1] == 'x' || str_color[1] == 'X'))
    strncpy(tmp, str_color + 2, 6);
  else
    strncpy(tmp, str_color, 6);

  if (strlen(tmp) < 6)
    return -1;

  strncpy(rs, tmp, 2);
  strncpy(gs, tmp+2, 2);
  strncpy(bs, tmp+4, 2);

  errno = 0;
  r = strtol(rs, NULL, 16);
  g = strtol(gs, NULL, 16);
  b = strtol(bs, NULL, 16);
  if (errno != 0)
    return -1;

  if (color)
    *color = CV_RGB(r, g, b);

  return 0;
}


static const char *config_get_path(void)
{
  static char config_path[PATH_MAX];

  if (config_path[0] == '\0')
    {
#if defined(HAVE_WINDOWS)
      strncpy(config_path, getenv("APPDATA"), PATH_MAX);
#elif defined(HAVE_APPLE)
      strcpy(config_path, getenv("HOME"));
      strncat(config_path, DIRSEP "Library" DIRSEP "Application Support",
        PATH_MAX - strlen(config_path));
#elif defined(HAVE_UNIX)
      strcpy(config_path, getenv("HOME"));
      strncat(config_path, DIRSEP ".config", PATH_MAX - strlen(config_path));
#else
      strcpy(config_path, ".");
#endif

      strncat(config_path, DIRSEP PROGNAME DIRSEP PROGNAME ".conf",
        PATH_MAX - strlen(config_path));
    }

  return (const char *) config_path;
}


static int on_ini_entry(void *user, const char *section, const char *name,
  const char *value)
{
#define MATCH(s, n) strcasecmp(section, s) == 0 && strcasecmp(name, n) == 0

  Config *config = (Config *) user;

  if (MATCH("general", "version"))
    config->version = atoi(value);
  else if (MATCH("general", "fullscreen"))
    config->fullscreen = atoi(value);
  else if (MATCH("general", "device"))
    config->device = atoi(value);
  else if (MATCH("general", "invert"))
    config->invert = atoi(value);
  else if (MATCH("general", "title"))
    {
      free(config->title);
      config->title = strdup(value);
    }
  else if (MATCH("general", "color_pair"))
    config->color_pair = atoi(value);
  else if (MATCH("line", "show"))
    config->show_line = atoi(value);
  else if (MATCH("line", "weight"))
    config->line_weight = atoi(value);
  else if (MATCH("line", "weight_max"))
    config->line_weight_max = atoi(value);
  else if (MATCH("line", "weight_interval"))
    config->line_weight_interval = atoi(value);
  else if (MATCH("line", "color"))
    string_to_color(value, &(config->line_color));
  else if (strncasecmp(section, "color_", 6) == 0)
    {
      int color_num = atoi(section + 6);

      if (strcasecmp(name, "foreground") == 0)
        string_to_color(value, &(config->fg_colors[color_num]));
      else if (strcasecmp(name, "background") == 0)
        string_to_color(value, &(config->bg_colors[color_num]));
    }
    else
      return 0; /* unknown section/name error */

  return 1; /* success */

#undef MATCH
}


static void config_create(void)
{
  /* FIXME */
  char  command[PATH_MAX + 11] = { 0 };
  char *path = strdup(config_get_path());
  snprintf(command, PATH_MAX + 11, "mkdir -p \"%s\"", dirname(path));
  (void) system(command);
  free(path);
}


Config *config_init(void)
{
  int i;
  static Config conf = { 0 };
  static int initialized = 0;

  if (initialized)
    return &conf;

  conf.version = 1;
  conf.title = strdup(PROGTITLE);
  conf.device = -1;
  conf.line_weight = 4;
  conf.line_weight_max = 20;
  conf.line_weight_interval = 2;
  conf.line_color = CV_RGB(255, 0, 0);
  conf.color_pair = 0;

  /* Some default high-contrast color pairs */
  conf.bg_colors[0] = CV_RGB(0x00, 0x00, 0x00);
  conf.fg_colors[0] = CV_RGB(0xff, 0xff, 0xff);

  conf.bg_colors[1] = CV_RGB(0x00, 0x00, 0xff);
  conf.fg_colors[1] = CV_RGB(0xff, 0xff, 0x00);

  conf.bg_colors[2] = CV_RGB(0x80, 0x00, 0xff);
  conf.fg_colors[2] = CV_RGB(0x80, 0xff, 0x00);

  conf.bg_colors[3] = CV_RGB(0xff, 0x00, 0xff);
  conf.fg_colors[3] = CV_RGB(0x00, 0xff, 0x00);

  conf.bg_colors[4] = CV_RGB(0xff, 0x00, 0x80);
  conf.fg_colors[4] = CV_RGB(0x00, 0xff, 0x80);

  conf.bg_colors[5] = CV_RGB(0xff, 0x00, 0x00);
  conf.fg_colors[5] = CV_RGB(0x00, 0xff, 0xff);

  conf.bg_colors[6] = CV_RGB(0xff, 0x80, 0x00);
  conf.fg_colors[6] = CV_RGB(0x00, 0x80, 0xff);

  /* Fill the rest with Black & White */
  for (i = 7; i < COLORS_MAX; i++)
    {
      conf.bg_colors[i] = CV_RGB(0x00, 0x00, 0x00);
      conf.fg_colors[i] = CV_RGB(0xff, 0xff, 0xff);
    }

  config_create();

  if (ini_parse(config_get_path(), on_ini_entry, &conf) < 0)
    {
      fprintf(stderr,
              "warning: unable to load config file '%s': %s\n",
              config_get_path(),
              strerror(errno));
      return &conf;
    }

  initialized = 1;
  return &conf;
}


void config_save(Config *config)
{
  int   i;
  FILE *fp;
  char *color = NULL;

  fp = fopen(config_get_path(), "w");
  if (fp == NULL)
    {
      fprintf(stderr,
              "warning: unable to save config file '%s': %s\n",
              config_get_path(),
              strerror(errno));
      return;
    }

  color = color_to_string(&(config->line_color));

  fprintf(fp,
          "\n[general]\n"
          "version=%d\n"
          "title=%s\n"
          "fullscreen=%d\n"
          "device=%d\n"
          "invert=%d\n"
          "color_pair=%d\n"
          "\n[line]\n"
          "show=%d\n"
          "weight=%d\n"
          "weight_max=%d\n"
          "weight_interval=%d\n"
          "color=%s\n",
          config->version,
          config->title,
          config->fullscreen,
          config->device,
          config->invert,
          config->color_pair,
          config->show_line,
          config->line_weight,
          config->line_weight_max,
          config->line_weight_interval,
          color);

  free(color);

  for (i = 0; i < COLORS_MAX; i++)
    {
      char *c1, *c2;

      c1 = color_to_string(&(config->fg_colors[i]));
      c2 = color_to_string(&(config->bg_colors[i]));

      fprintf(fp,
              "\n[color_%d]\n"
              "foreground=%s\n"
              "background=%s\n",
              i, c1, c2);

      free(c1);
      free(c2);
    }

  fclose(fp);
}
