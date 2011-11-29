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
#include <iostream>
#include <fstream>

using namespace std;

char buffer[256];

int main(int argc, char **argv) {
    if(argc < 2) {
        cout << "Usage: divaconvert <translation_file>.txt" << endl;
        return 1;
    }
	FILE *file_in = fopen(argv[1], "rb");
	if(!file_in) {
	    cout << "Cannot open " << argv[1] << endl;
		return 1;
	}

	strcpy(buffer, argv[1]);
	char *fileext = strrchr(buffer, '.');
	strcpy(fileext, ".bin");
	FILE *file_out = fopen(buffer, "wb");
	if(!file_out) {
		cout << "Error while creating " << buffer << endl;
		return 1;
	}

	strcpy(buffer, argv[1]);
	fileext = strrchr(buffer, '.');
	strcpy(fileext, ".tmp");
	FILE *file_tmp = fopen(buffer, "wb+");
	if(!file_tmp) {
		cout << "Error while creating " << buffer << endl;
		return 1;
	}
	map<string, size_t> lst;
	size_t count = 0;
	size_t table_size = 0;
	size_t index_size = 4;
	size_t offset = 0;
	size_t padding = 0;
	int line_count = 0;
	fwrite(&count, 4, 1, file_out);
	while(fgets(buffer, 256, file_in)) {
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
        fwrite(&addr, 4, 1, file_out);

		line_count++;

		map<string,size_t>::const_iterator it = lst.find(str);
		if(it == lst.end()) {
		    lst[str] = offset;
		    fwrite(&offset, 4 ,1, file_out);
	        fwrite(str, len, 1, file_tmp);
	        padding = 4 - (len % 4);
	        table_size += len + padding;
	        offset += len + padding;
	        if(padding) {
	            memset(buffer, 0, 16);
	            fwrite(buffer, padding, 1, file_tmp);
	        }
		} else {
		    fwrite(&lst[str], 4 ,1, file_out);
		}
		count++;
		index_size += 8;
	}
	fseek(file_out, 0, SEEK_SET);
	fwrite(&count, 4, 1, file_out);
	fseek(file_out, 0, SEEK_END);
	if(index_size % 16)
		padding = 16 - (index_size % 16);
	if(padding) {
        memset(buffer, 0, 16);
        fwrite(buffer, padding, 1, file_out);
    }

	padding = 0;
	if(table_size % 16)
		padding = 16 - (table_size % 16);
	if(padding) {
        memset(buffer, 0, 16);
        fwrite(buffer, padding, 1, file_tmp);
    }

	size_t size = (size_t)ftell(file_tmp);
	fseek(file_tmp, 0, SEEK_SET);
	count = sizeof(buffer);

	cout << "String block size: " << size << " bytes" << endl;

	while(size) {
	    if(size < (int)sizeof(buffer)) {
	        count = size;
	    }
	    fread(buffer, count, 1, file_tmp);
	    fwrite(buffer, count, 1, file_out);
	    size -= count;
	}

	cout << "Total lines: " << line_count << endl;
	cout << "Unique lines: " << (int)lst.size() << endl;

	// close all files
	fclose(file_out);
	fclose(file_tmp);
    fclose(file_in);

    // delete tmp file
    strcpy(buffer, argv[1]);
    fileext = strrchr(buffer, '.');
    strcpy(fileext, ".tmp");
	remove(buffer);

    strcpy(buffer, argv[1]);
    fileext = strrchr(buffer, '.');
    strcpy(fileext, ".bin");
	printf("%s has been created succefully\n", buffer);
	return 0;
}
