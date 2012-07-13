/*
 *  PJD2Patch patch utilily
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

struct addr_data {
    unsigned int addr;
    unsigned int offset;
}__attribute__((packed));

char buffer[256];

int main(int argc, char **argv) {
    if(argc < 2) {
        printf("Usage: divaextract <translation_file>.bin\n");
        return 1;
    }
	FILE *file_in = fopen(argv[1], "rb");
	if(!file_in) {
	    printf("Cannot open %s\n", argv[1]);
		return 1;
	}

	strcpy(buffer, argv[1]);
	char *fileext = strrchr(buffer, '.');
	strcpy(fileext, ".txt");
	FILE *file_out = fopen(buffer, "wb");
	if(!file_out) {
		printf("Error while creating %s\n", buffer);
		return 1;
	}

	fseek(file_in, 0, SEEK_END);
	size_t file_size = (size_t)ftell(file_in);
	fseek(file_in, 0, SEEK_SET);

	char *filebuf = (char *)malloc(file_size);
	fread(filebuf, file_size, 1, file_in);
    fclose(file_in);

    int entries = *(int *)filebuf;
    int index_size = (entries * 8) + 4;
    index_size += 16 - (index_size % 16);

    printf("entries: %i, index size: %i\n", entries, index_size);

    for(int i = 0; i < entries; i++) {
        struct addr_data *data;

        data = &((struct addr_data *)((char *)filebuf + 4))[i];
        char *str = (filebuf + index_size + data->offset);

        char type;
        if((data->addr & 0xF0000000) == 0xF0000000) {
            type = '!';
        } else if((data->addr & 0xF0000000) == 0xE0000000) {
            type = '^';
        } else if((data->addr & 0xF0000000) == 0xD0000000) {
            type = '$';
        }  else {
            type = '0';
        }

        data->addr &= 0x0FFFFFFF;
        fprintf(file_out, "%cx%08X %s\n", type, data->addr, str);
    }
    fclose(file_out);
	free(filebuf);
	return 0;
}
