/*
 *  MHP3patch kernel module
 *
 *  Copyright (C) 2011  Codestation
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LOGGER_H_
#define LOGGER_H_

//#include <string.h>

#ifdef DEBUG

#define LOGFILE "ms0:/divapatch.log"

int sprintf(char *str, const char *format, ...);

extern char _buffer_log[256];

int kwrite(const char *path, void *buffer, SceSize buflen);

#define kprintf(format, ...) do { \
    sprintf(_buffer_log, "%s: "format, __func__, ## __VA_ARGS__); \
    kwrite(LOGFILE, _buffer_log, strlen(_buffer_log)); \
} while(0)

int kwrite(const char *path, void *buffer, SceSize buflen);

#else

#define kprintf(format, ...)
#define kwrite(a, b, c)

#endif

#endif /* LOGGER_H_ */
