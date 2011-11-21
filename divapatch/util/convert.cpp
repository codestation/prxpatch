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

#include <map>
#include <string>

using namespace std;

char buffer[256];

int main(int argc, char **argv) {
    if(argc < 2) {
        printf("Usage: divaconvert <translation_file>.txt\n");
        return 1;
    }
	FILE *fd = fopen(argv[1], "rb");
	if(!fd) {
	    printf("Cannot open %s\n", argv[1]);
		return 1;
	}
	char fileout[64];
	strcpy(fileout, argv[1]);
	strcat(fileout, ".bin");
	FILE *fdout = fopen(fileout, "wb");
	if(!fdout) {
		printf("Error while creating %s\n", fileout);
		return 1;
	}
	strcat(fileout, ".tmp");
	FILE *fdstr = fopen(fileout, "wb+");
	if(!fdout) {
		printf("Error while creating %s\n", fileout);
		return 1;
	}
	map<string, size_t> lst;
	size_t count = 0;
	size_t table_size = 0;
	size_t index_size = 4;
	size_t offset = 0;
	size_t padding = 0;
	int line_count = 0;
	fwrite(&count, 4, 1, fdout);
	while(fgets(buffer, 256, fd)) {
		if(buffer[0] == '#' || buffer[0] == '\n')
			continue;
		buffer[10] = 0;
		char *str = buffer + 11;
		size_t len = strlen(str);
		str[len-1] = 0;
		// embedded offset in instruction
		if(buffer[0] == '!') {
		    buffer[0] = '0';
		    buffer[2] = 'F';
		    // only upper address
		} else if(buffer[0] == '^') {
            buffer[0] = '0';
            buffer[2] = 'E';
            // olny lower address
        } else if(buffer[0] == '$') {
            buffer[0] = '0';
            buffer[2] = 'D';
        }

        long long addr = strtoll(buffer, NULL, 16);
        fwrite(&addr, 4, 1, fdout);

		line_count++;

		map<string,size_t>::const_iterator it = lst.find(str);
		if(it == lst.end()) {
		    lst[str] = offset;
		    fwrite(&offset, 4 ,1, fdout);
	        fwrite(str, len, 1, fdstr);
	        padding = 4 - (len % 4);
	        table_size += len + padding;
	        offset += len + padding;
	        if(padding) {
	            memset(buffer, 0, 16);
	            fwrite(buffer, padding, 1, fdstr);
	        }
		} else {
		    fwrite(&lst[str], 4 ,1, fdout);
		}
		count++;
		index_size += 8;
	}
	fseek(fdout, 0, SEEK_SET);
	fwrite(&count, 4, 1, fdout);
	fseek(fdout, 0, SEEK_END);
	if(index_size % 16)
		padding = 16 - (index_size % 16);
	if(padding) {
        memset(buffer, 0, 16);
        fwrite(buffer, padding, 1, fdout);
    }

	padding = 0;
	if(table_size % 16)
		padding = 16 - (table_size % 16);
	if(padding) {
        memset(buffer, 0, 16);
        fwrite(buffer, padding, 1, fdstr);
    }

	size_t size = (size_t)ftell(fdstr);
	fseek(fdstr, 0, SEEK_SET);
	count = sizeof(buffer);

	printf("Block size: %li bytes\n", size);

	while(size) {
	    if(size < (int)sizeof(buffer)) {
	        count = size;
	    }
	    fread(buffer, count, 1, fdstr);
	    fwrite(buffer, count, 1, fdout);
	    size -= count;
	}

	printf("Total lines: %i\n", line_count);
	printf("Unique lines: %i\n", (int)lst.size());

	strcpy(buffer, argv[1]);
	char *dot = strrchr(buffer, '.');
	strcpy(dot, ".bin");
	fclose(fdout);
	fclose(fdstr);
	remove(fileout);
	fclose(fd);
	strcpy(fileout, argv[1]);
	strcat(fileout, ".bin");
	remove(buffer);
	printf("Created %s\n", buffer);
	rename(fileout,buffer);
	return 0;
}
