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
 * modified binary search function that only works on node arrays
 * @param value  value to search in the array
 * @param vector  array to make the search
 * @param lim  number of elements in the array
 * @returns the address of the found value in the array, else NULL
 */
cpknode *search_vector(u32 value, cpknode *vector, u32 lim) {
    u32 idx;
    u32 count = lim;
    if(vector) {
        while(lim) {
            idx = lim >> 1;
            if(value >= vector[idx].cpk_offset) {
                if(value > vector[idx].cpk_offset && (idx+1 > count || vector[idx+1].cpk_offset <= value)) {
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
