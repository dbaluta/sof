/*****************************************************************
 * Copyright 2018 NXP
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************/

#include <string.h>
#include <xtensa/hal.h>
#include <xtensa/xtruntime.h>
#include <xtensa/config/core.h>

#define I_CACHE_ATTRIBUTE  0x2224224F     //write back mode
#define D_CACHE_ATTRIBUTE  0x2221224F     //write through mode
#define INT_NUM_MU	7

volatile int x1 = 0x12345678;
volatile int x2 = 0x22446688;
volatile int x3 = 0x87654321;;
volatile int x4 = 0x88888888;
int main(void)
{
	/* set the cache attribute */
	xthal_set_icacheattr(I_CACHE_ATTRIBUTE);
	xthal_set_dcacheattr(D_CACHE_ATTRIBUTE);
	
	if ((int)&x2 == 0x55555555)
		x1 = 0x12341234;
	else
		x1 = 0x12121212;

	if ((int)&x2 == 0x5555555)
		x4 = 0x41414141;
	else
		x4 = 0x42424242;

	xthal_dcache_sync();
	xthal_icache_sync();

	for ( ; ; );
	
	return 0;
}
