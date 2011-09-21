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

u32 *search_vector(u32 value, u32 *vector, u32 lim) {
    u32 idx;
    u32 count = lim;
    if(vector) {
        while(lim) {
            idx = lim >> 1;
            if(value >= vector[idx]) {
                if(value > vector[idx] && (idx+1 > count || vector[idx+1] <= value)) {
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
