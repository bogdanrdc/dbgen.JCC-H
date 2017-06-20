#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "phash.h"

static int pos[64] = /* numbers 0..63 in some random order */
{15,18,54,9,50,37,26,51,41,11,31,7,23,38,28,58,16,57,14,3,36,21,6,34,25,33,53,2,4,40,0,46,1,60,
 30,61,8,43,56,48,42,55,47,59,24,49,20,39,12,63,10,29,19,27,52,45,22,13,35,44,5,32,17,62 };

#define CONVERT_ENDIAN(x) (x)
#if ((1 >> 30) > 0)
#undef CONVERT_ENDIAN
#define CONVERT_ENDIAN(x) (63-(x))
#endif

uint64_t phash(uint64_t key, uint64_t maxval, int maxbit, int inv) {
	uint64_t safeval = ((uint64_t) 1) << maxbit, ret = 0;
	assert(key < maxval);
	if (key < safeval) {
		int i, j;
		for(i=j=0; i<maxbit; i++,j++) {
			while (pos[j] >= maxbit) j++;
			ret |= (1-((key>>CONVERT_ENDIAN(inv?i:pos[j]))&1)) << CONVERT_ENDIAN(inv?pos[j]:i);
		}
	} else {
		ret = maxval - (1 + key - safeval);
	}
	return ret;
}

uint64_t hash(uint64_t key, uint64_t maxval, int maxbit, int inv) {
	if (inv) return phash(key, maxval, maxbit, inv) + 1;
	return phash(key - 1, maxval, maxbit, inv);
}

#ifdef PHASH_STANDALONE
int main(int argc, char** argv) {
	long int key = atol(argv[1]);
	long int maxval = atol(argv[2]);
	int maxbit = log2(maxval);
	uint64_t h = hash(key,maxval,maxbit,0);
	return printf("hash(key=%lld,maxval=%lld, maxbit=%d) = %d (inv = %lld)\n",  key, maxval, maxbit, h, hash(h, maxval, maxbit, 1));
}
#else 
#include "../dss.h"

static uint16_t nations_map[25] = /* mapping between countries and their keys */
{15, 0, 5, 14, 16,	/* AFRICA		(MOROCCO | ALGERIA, ETHIOPIA, KENYA, MOZAMBIQUE) */
24, 1, 2, 3, 17, 	/* AMERICA		(UNITED STATES | ARGENTINA, BRAZIL, CANADA, PERU)*/
18, 9, 12, 21, 8, 	/* ASIA 		(CHINA | INDONESIA, JAPAN, VIETNAM, INDIA)*/
7, 6, 22, 19, 23, 	/* EUROPE 		(GERMANY | FRANCE, RUSSIA, ROMANIA, UNITED KINGDOM*/
4, 10, 11, 13, 20};	/* MIDDLE EAST 	(EGYPT | IRAN, IRAQ, JORDAN, SAUDI ARABIA)*/

uint64_t max_bit_tbl_part = 0;
uint64_t max_bit_tbl_supplier = 0;
uint64_t max_bit_tbl_partsupp = 0;
uint64_t max_bit_tbl_customer = 0;
uint64_t max_bit_tbl_nation = 0;
uint64_t max_bit_tbl_lineitem = 0;
uint64_t max_bit_tbl_region = 0;
uint64_t max_bit_tbl_orders = 0;

void init_skew() {
	int i, j = 0;
	max_bit_tbl_part = (uint64_t) floor(log2((double) (scale * tdefs[PART].base)));
	max_bit_tbl_supplier = (uint64_t) floor(log2((double) (scale * tdefs[SUPP].base)));
	max_bit_tbl_partsupp = (uint64_t) floor(log2((double) (scale * tdefs[PSUPP].base)));
	max_bit_tbl_customer = (uint64_t) floor(log2((double) (scale * tdefs[CUST].base)));
	max_bit_tbl_nation = (uint64_t) floor(log2((double) (scale * tdefs[NATION].base)));
	max_bit_tbl_lineitem = (uint64_t) floor(log2((double) (scale * tdefs[LINE].base)));
	max_bit_tbl_region = (uint64_t) floor(log2((double) (scale * tdefs[REGION].base)));
	max_bit_tbl_orders = (uint64_t) floor(log2((double) (scale * tdefs[ORDER].base)));
}

uint16_t bin_nationkey(uint64_t key, uint64_t tbl_size) {
	long row = key / (0.2 * tbl_size);
	long bin = row * 5;
	long offset = key - (0.18 + row * 0.2) * tbl_size;
	assert(row < 5);
	if (offset > 0 && 0.02 * tbl_size > 0) { 
		offset = (4*offset) / (0.02*tbl_size);
		assert(offset < 4);
		bin += 1 + offset;
	}
	return nations_map[bin];
}
#endif
