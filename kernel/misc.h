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

#ifndef MISC_H_
#define MISC_H_

#define MODEL_PHAT 0
#define MODEL_SLIM 1
#define MODEL_BRITE 2
#define MODEL_PSPGO 4

#define TRANSLATION_PATH_MS "ms0:/MHP3RD_DATA.BIN"
#define TRANSLATION_PATH_GO "ef0:/MHP3RD_DATA.BIN"

extern int model;
extern volatile int reopen;
extern volatile SceUID transfd;
void reopen_translation();

#endif /* MISC_H_ */
