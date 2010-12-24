/*
 *  MHP3patch kernel module
 *
 *  Copyright (C) 2010  Codestation
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

#ifndef BOOTSPLASH_H_
#define BOOTSPLASH_H_

#include <psptypes.h>

void init_display();
void clear_display(u32 color);
void fadeout_display();
int draw_image(const char *path);
int write_bitmap(void *frame_addr, int format, const char *file);
int read_8888_data(void *frame, void *pixel_data);

#endif /* BOOTSPLASH_H_ */
