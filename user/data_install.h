/*
 *  MHP3patch user module: data install patcher
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

#ifndef DATA_INSTALL_H_
#define DATA_INSTALL_H_

extern SceUID sema;
extern SceSize data_start;
extern unsigned int patch_count;
extern unsigned int patch_size[32];

void register_install(const char *file, SceUID fd);
void unregister_install(SceUID fd);
int read_install(SceUID fd, void *data, SceSSize size);
void fill_install_tables(SceUID fd);

#endif /* DATA_INSTALL_H_ */
