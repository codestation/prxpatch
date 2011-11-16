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

#include "search.h"

/**
 * modified binary search function that only works on int arrays
 * @param value  value to search in the array
 * @param vector  array to make the search
 * @param lim  number of elements in the array
 * @returns the address of the found value in the array, else NULL
 */
u32 *search_exact(u32 value, u32 *vector, u32 lim) {
    u32 idx;
    if(vector) {
        while(lim) {
            idx = lim >> 1;
            if(value >= vector[idx]) {
                if(value > vector[idx]) {
                    vector += idx + 1;
                    lim--;
                } else {
                    return vector + idx;
                }
            }
            lim >>= 1;
        }
    }
    return NULL;
}
