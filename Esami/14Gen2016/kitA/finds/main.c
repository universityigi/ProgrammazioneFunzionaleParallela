// =====================================================================
//  main.c
// =====================================================================

//  Author:         (c) 2016 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        January 13, 2016

//  Last changed:   $Date: 2016/01/09 15:13:07 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.00 $


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clut.h"
#include "finds.h"

#define PROG_NAME "finds.cl"


// ---------------------------------------------------------------------
// finds_host
// ---------------------------------------------------------------------
// sequential CPU version executed on host

void finds_host(const char* pattern, const char* text, char* out, int n,  
                double* t) {

    int p = strlen(pattern);
    int i, m = n - p + 1;

    // get initial time
    double start = clut_get_real_time();

    for (i=0; i<m; i++)
        out[i] = memcmp(text+i, pattern, p) == 0;

    for (; i<n; i++) out[i] = 0;

    // get elapsed time
    *t = clut_get_real_time() - start;
}

// ---------------------------------------------------------------------
// count_non_zero
// ---------------------------------------------------------------------
// count number of non-zeros

int count_non_zero(const char* str, int n) {
    int i, s = 0;
    for (i=0; i<n; i++) s += str[i] != 0;
    return s;
}


// ---------------------------------------------------------------------
// get_file_size
// ---------------------------------------------------------------------
// compute file size

int get_file_size(FILE* f){
    int size;
    int curr_pos = ftell(f);
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, curr_pos, SEEK_SET);
    return size;
}


// ---------------------------------------------------------------------
// run_test
// ---------------------------------------------------------------------
// perform test

void run_test(const char* pattern, const char* text, int n, 
              clut_device* dev, char* msg) {

    double t, t_ok;
    
    char* out    = malloc(n);
    char* out_ok = malloc(n);
    if (out_ok == NULL || out == NULL) clut_panic("out of memory");

    // compute on host
    finds_host(pattern, text, out_ok, n, &t_ok);

    // compute on device
    finds(pattern, text, out, n, dev, &t);

    // validate our results
    printf("%s\n", msg);
    printf("    CPU took %.3f msec\n",    t_ok*1E03);
    printf("    Device took %.3f msec\n", t*1E03);
    printf("    Pattern \"%s\" found %d times on CPU\n", 
                    pattern, count_non_zero(out_ok, n));
    printf("    Pattern \"%s\" found %d times on device\n", 
                    pattern, count_non_zero(out, n));
    printf("    Correctness test: %s\n",  
                    memcmp(out, out_ok, n)==0 ? 
					"PASSED" : "FAILED");

    // cleanup
	free(out);
	free(out_ok);
}


// ---------------------------------------------------------------------
// main
// ---------------------------------------------------------------------
int main(int argc, char** argv) {

    clut_device  dev;               // device structure
    char*        in_path;           // input image pathname
    char*        pattern;           // string pattern
    char*        text;              // input text
    int          n;                 // input text size

    // args count check
    if (argc < 3) {
        printf("Usage: %s <input-file> <pattern-string>\n", argv[0]);
        return 1;
    }

    // set parameters
    in_path   = argv[1];
    pattern   = argv[2];
    
    if (strlen(pattern) == 0) clut_panic("empty pattern");

    // open device
    clut_open_device(&dev, PROG_NAME);

    // load text file
    printf("Opening text file: %s\n", in_path);
    FILE* f = fopen(in_path, "rb");
    if (f == NULL) clut_panic("failed to load input text file");
    n = get_file_size(f);
    text = malloc(n);
    if (text == NULL) clut_panic("failed to allocate text block");
    if (n != fread(text, 1, n, f)) clut_panic("failed to read text");
    printf("Loaded text file of %d bytes\n", n);
    fclose(f);

    // print device info
    clut_print_device_info(&dev);

    // print text excerpt
    printf("------\n");
    fwrite(text, 1, 256, stdout);
    printf(" [...] \n------\n");

    // run test
    run_test((const char*)pattern, (const char*)text, n, 
             &dev, "Running test");

    clut_close_device(&dev);
    
    return 0;
}


// Copyright (C) 2016 Camil Demetrescu
  
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
  
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
// USA
