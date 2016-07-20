#include "des3.h"
#define ID_OK	0
#define RE_LEN	1
typedef struct {
  uint32_t subkeys[3][32];                     /* subkeys for three operations */
  uint32_t iv[2];                                       /* initializing vector */
  uint32_t originalIV[2];                        /* for restarting the context */
  int32_t encrypt;                                              /* encrypt flag */
} DES3_CBC_CTX;

static uint16_t bytebit[8] = {
	0200, 0100, 040, 020, 010, 04, 02, 01
};
static uint32_t bigbyte[24] = {
	0x800000L, 0x400000L, 0x200000L, 0x100000L,
	0x80000L,  0x40000L,  0x20000L,  0x10000L,
	0x8000L,   0x4000L,   0x2000L,   0x1000L,
	0x800L,    0x400L,    0x200L,    0x100L,
	0x80L,     0x40L,     0x20L,     0x10L,
	0x8L,      0x4L,      0x2L,      0x1L
};

static uint8_t totrot[16] = {
	1, 2, 4, 6, 8, 10, 12, 14, 15, 17, 19, 21, 23, 25, 27, 28
};

static uint8_t pc1[56] = {
	56, 48, 40, 32, 24, 16,  8,      0, 57, 49, 41, 33, 25, 17,
	 9,  1, 58, 50, 42, 34, 26,     18, 10,  2, 59, 51, 43, 35,
	62, 54, 46, 38, 30, 22, 14,      6, 61, 53, 45, 37, 29, 21,
	13,  5, 60, 52, 44, 36, 28,     20, 12,  4, 27, 19, 11,  3
};

static uint8_t pc2[48] = {
	13, 16, 10, 23,  0,  4,  2, 27, 14,  5, 20,  9,
	22, 18, 11,  3, 25,  7, 15,  6, 26, 19, 12,  1,
	40, 51, 30, 36, 46, 54, 29, 39, 50, 44, 32, 47,
	43, 48, 38, 55, 33, 52, 45, 41, 49, 35, 28, 31
};

static uint32_t Spbox[8][64] = {
	{0x01010400L, 0x00000000L, 0x00010000L, 0x01010404L,
	0x01010004L, 0x00010404L, 0x00000004L, 0x00010000L,
	0x00000400L, 0x01010400L, 0x01010404L, 0x00000400L,
	0x01000404L, 0x01010004L, 0x01000000L, 0x00000004L,
	0x00000404L, 0x01000400L, 0x01000400L, 0x00010400L,
	0x00010400L, 0x01010000L, 0x01010000L, 0x01000404L,
	0x00010004L, 0x01000004L, 0x01000004L, 0x00010004L,
	0x00000000L, 0x00000404L, 0x00010404L, 0x01000000L,
	0x00010000L, 0x01010404L, 0x00000004L, 0x01010000L,
	0x01010400L, 0x01000000L, 0x01000000L, 0x00000400L,
	0x01010004L, 0x00010000L, 0x00010400L, 0x01000004L,
	0x00000400L, 0x00000004L, 0x01000404L, 0x00010404L,
	0x01010404L, 0x00010004L, 0x01010000L, 0x01000404L,
	0x01000004L, 0x00000404L, 0x00010404L, 0x01010400L,
	0x00000404L, 0x01000400L, 0x01000400L, 0x00000000L,
	0x00010004L, 0x00010400L, 0x00000000L, 0x01010004L,},
	{0x80108020L, 0x80008000L, 0x00008000L, 0x00108020L,
	0x00100000L, 0x00000020L, 0x80100020L, 0x80008020L,
	0x80000020L, 0x80108020L, 0x80108000L, 0x80000000L,
	0x80008000L, 0x00100000L, 0x00000020L, 0x80100020L,
	0x00108000L, 0x00100020L, 0x80008020L, 0x00000000L,
	0x80000000L, 0x00008000L, 0x00108020L, 0x80100000L,
	0x00100020L, 0x80000020L, 0x00000000L, 0x00108000L,
	0x00008020L, 0x80108000L, 0x80100000L, 0x00008020L,
	0x00000000L, 0x00108020L, 0x80100020L, 0x00100000L,
	0x80008020L, 0x80100000L, 0x80108000L, 0x00008000L,
	0x80100000L, 0x80008000L, 0x00000020L, 0x80108020L,
	0x00108020L, 0x00000020L, 0x00008000L, 0x80000000L,
	0x00008020L, 0x80108000L, 0x00100000L, 0x80000020L,
	0x00100020L, 0x80008020L, 0x80000020L, 0x00100020L,
	0x00108000L, 0x00000000L, 0x80008000L, 0x00008020L,
	0x80000000L, 0x80100020L, 0x80108020L, 0x00108000L,},
	{0x00000208L, 0x08020200L, 0x00000000L, 0x08020008L,
	0x08000200L, 0x00000000L, 0x00020208L, 0x08000200L,
	0x00020008L, 0x08000008L, 0x08000008L, 0x00020000L,
	0x08020208L, 0x00020008L, 0x08020000L, 0x00000208L,
	0x08000000L, 0x00000008L, 0x08020200L, 0x00000200L,
	0x00020200L, 0x08020000L, 0x08020008L, 0x00020208L,
	0x08000208L, 0x00020200L, 0x00020000L, 0x08000208L,
	0x00000008L, 0x08020208L, 0x00000200L, 0x08000000L,
	0x08020200L, 0x08000000L, 0x00020008L, 0x00000208L,
	0x00020000L, 0x08020200L, 0x08000200L, 0x00000000L,
	0x00000200L, 0x00020008L, 0x08020208L, 0x08000200L,
	0x08000008L, 0x00000200L, 0x00000000L, 0x08020008L,
	0x08000208L, 0x00020000L, 0x08000000L, 0x08020208L,
	0x00000008L, 0x00020208L, 0x00020200L, 0x08000008L,
	0x08020000L, 0x08000208L, 0x00000208L, 0x08020000L,
	0x00020208L, 0x00000008L, 0x08020008L, 0x00020200L,},
	{0x00802001L, 0x00002081L, 0x00002081L, 0x00000080L,
	0x00802080L, 0x00800081L, 0x00800001L, 0x00002001L,
	0x00000000L, 0x00802000L, 0x00802000L, 0x00802081L,
	0x00000081L, 0x00000000L, 0x00800080L, 0x00800001L,
	0x00000001L, 0x00002000L, 0x00800000L, 0x00802001L,
	0x00000080L, 0x00800000L, 0x00002001L, 0x00002080L,
	0x00800081L, 0x00000001L, 0x00002080L, 0x00800080L,
	0x00002000L, 0x00802080L, 0x00802081L, 0x00000081L,
	0x00800080L, 0x00800001L, 0x00802000L, 0x00802081L,
	0x00000081L, 0x00000000L, 0x00000000L, 0x00802000L,
	0x00002080L, 0x00800080L, 0x00800081L, 0x00000001L,
	0x00802001L, 0x00002081L, 0x00002081L, 0x00000080L,
	0x00802081L, 0x00000081L, 0x00000001L, 0x00002000L,
	0x00800001L, 0x00002001L, 0x00802080L, 0x00800081L,
	0x00002001L, 0x00002080L, 0x00800000L, 0x00802001L,
	0x00000080L, 0x00800000L, 0x00002000L, 0x00802080L,},
	{0x00000100L, 0x02080100L, 0x02080000L, 0x42000100L,
	0x00080000L, 0x00000100L, 0x40000000L, 0x02080000L,
	0x40080100L, 0x00080000L, 0x02000100L, 0x40080100L,
	0x42000100L, 0x42080000L, 0x00080100L, 0x40000000L,
	0x02000000L, 0x40080000L, 0x40080000L, 0x00000000L,
	0x40000100L, 0x42080100L, 0x42080100L, 0x02000100L,
	0x42080000L, 0x40000100L, 0x00000000L, 0x42000000L,
	0x02080100L, 0x02000000L, 0x42000000L, 0x00080100L,
	0x00080000L, 0x42000100L, 0x00000100L, 0x02000000L,
	0x40000000L, 0x02080000L, 0x42000100L, 0x40080100L,
	0x02000100L, 0x40000000L, 0x42080000L, 0x02080100L,
	0x40080100L, 0x00000100L, 0x02000000L, 0x42080000L,
	0x42080100L, 0x00080100L, 0x42000000L, 0x42080100L,
	0x02080000L, 0x00000000L, 0x40080000L, 0x42000000L,
	0x00080100L, 0x02000100L, 0x40000100L, 0x00080000L,
	0x00000000L, 0x40080000L, 0x02080100L, 0x40000100L,},
	{0x20000010L, 0x20400000L, 0x00004000L, 0x20404010L,
	0x20400000L, 0x00000010L, 0x20404010L, 0x00400000L,
	0x20004000L, 0x00404010L, 0x00400000L, 0x20000010L,
	0x00400010L, 0x20004000L, 0x20000000L, 0x00004010L,
	0x00000000L, 0x00400010L, 0x20004010L, 0x00004000L,
	0x00404000L, 0x20004010L, 0x00000010L, 0x20400010L,
	0x20400010L, 0x00000000L, 0x00404010L, 0x20404000L,
	0x00004010L, 0x00404000L, 0x20404000L, 0x20000000L,
	0x20004000L, 0x00000010L, 0x20400010L, 0x00404000L,
	0x20404010L, 0x00400000L, 0x00004010L, 0x20000010L,
	0x00400000L, 0x20004000L, 0x20000000L, 0x00004010L,
	0x20000010L, 0x20404010L, 0x00404000L, 0x20400000L,
	0x00404010L, 0x20404000L, 0x00000000L, 0x20400010L,
	0x00000010L, 0x00004000L, 0x20400000L, 0x00404010L,
	0x00004000L, 0x00400010L, 0x20004010L, 0x00000000L,
	0x20404000L, 0x20000000L, 0x00400010L, 0x20004010L,},
	{0x00200000L, 0x04200002L, 0x04000802L, 0x00000000L,
	0x00000800L, 0x04000802L, 0x00200802L, 0x04200800L,
	0x04200802L, 0x00200000L, 0x00000000L, 0x04000002L,
	0x00000002L, 0x04000000L, 0x04200002L, 0x00000802L,
	0x04000800L, 0x00200802L, 0x00200002L, 0x04000800L,
	0x04000002L, 0x04200000L, 0x04200800L, 0x00200002L,
	0x04200000L, 0x00000800L, 0x00000802L, 0x04200802L,
	0x00200800L, 0x00000002L, 0x04000000L, 0x00200800L,
	0x04000000L, 0x00200800L, 0x00200000L, 0x04000802L,
	0x04000802L, 0x04200002L, 0x04200002L, 0x00000002L,
	0x00200002L, 0x04000000L, 0x04000800L, 0x00200000L,
	0x04200800L, 0x00000802L, 0x00200802L, 0x04200800L,
	0x00000802L, 0x04000002L, 0x04200802L, 0x04200000L,
	0x00200800L, 0x00000000L, 0x00000002L, 0x04200802L,
	0x00000000L, 0x00200802L, 0x04200000L, 0x00000800L,
	0x04000002L, 0x04000800L, 0x00000800L, 0x00200002L,},
	{0x10001040L, 0x00001000L, 0x00040000L, 0x10041040L,
	0x10000000L, 0x10001040L, 0x00000040L, 0x10000000L,
	0x00040040L, 0x10040000L, 0x10041040L, 0x00041000L,
	0x10041000L, 0x00041040L, 0x00001000L, 0x00000040L,
	0x10040000L, 0x10000040L, 0x10001000L, 0x00001040L,
	0x00041000L, 0x00040040L, 0x10040040L, 0x10041000L,
	0x00001040L, 0x00000000L, 0x00000000L, 0x10040040L,
	0x10000040L, 0x10001000L, 0x00041040L, 0x00040000L,
	0x00041040L, 0x00040000L, 0x10041000L, 0x00001000L,
	0x00000040L, 0x10040040L, 0x00001000L, 0x00041040L,
	0x10001000L, 0x00000040L, 0x10000040L, 0x10040000L,
	0x10040040L, 0x10000000L, 0x00040000L, 0x10001040L,
	0x00000000L, 0x10041040L, 0x00040040L, 0x10000040L,
	0x10040000L, 0x10001000L, 0x10001040L, 0x00000000L,
	0x10041040L, 0x00041000L, 0x00041000L, 0x00001040L,
	0x00001040L, 0x00040040L, 0x10000000L, 0x10041000L,},
};
static void unscrunch(uint8_t *, uint32_t *);
static void scrunch(uint32_t *, uint8_t *);
static void deskey_(uint32_t *, uint8_t *, int32_t);
static void cookey(uint32_t *, uint32_t *, int32_t);
static void desfunc(uint32_t *, uint32_t *);

static void memset_(void* dest_a, int32_t value, int32_t len);
static void memset_(void* dest_a, int32_t value, int32_t len)
{
    uint8_t * dest = dest_a;
    if (len == 0) return;
    do {
        *dest++ = (uint8_t)value;  /* ??? to be unrolled */
    } while (--len != 0);
}


#define	F(l,r,key){\
	work = ((r >> 4) | (r << 28)) ^ *key;\
	l ^= Spbox[6][work & 0x3f];\
	l ^= Spbox[4][(work >> 8) & 0x3f];\
	l ^= Spbox[2][(work >> 16) & 0x3f];\
	l ^= Spbox[0][(work >> 24) & 0x3f];\
	work = r ^ *(key+1);\
	l ^= Spbox[7][work & 0x3f];\
	l ^= Spbox[5][(work >> 8) & 0x3f];\
	l ^= Spbox[3][(work >> 16) & 0x3f];\
	l ^= Spbox[1][(work >> 24) & 0x3f];\
}


static void desfunc(uint32_t *block, uint32_t *ks){
    uint32_t left,right,work;

	left = block[0];
	right = block[1];

	work = ((left >> 4) ^ right) & 0x0f0f0f0f;
	right ^= work;
	left ^= work << 4;
	work = ((left >> 16) ^ right) & 0xffff;
	right ^= work;
	left ^= work << 16;
	work = ((right >> 2) ^ left) & 0x33333333;
	left ^= work;
	right ^= (work << 2);
	work = ((right >> 8) ^ left) & 0xff00ff;
	left ^= work;
	right ^= (work << 8);
	right = (right << 1) | (right >> 31);
	work = (left ^ right) & 0xaaaaaaaa;
	left ^= work;
	right ^= work;
	left = (left << 1) | (left >> 31);

	F(left,right,&ks[0]);
	F(right,left,&ks[2]);
	F(left,right,&ks[4]);
	F(right,left,&ks[6]);
	F(left,right,&ks[8]);
	F(right,left,&ks[10]);
	F(left,right,&ks[12]);
	F(right,left,&ks[14]);
	F(left,right,&ks[16]);
	F(right,left,&ks[18]);
	F(left,right,&ks[20]);
	F(right,left,&ks[22]);
	F(left,right,&ks[24]);
	F(right,left,&ks[26]);
	F(left,right,&ks[28]);
	F(right,left,&ks[30]);

	right = (right << 31) | (right >> 1);
	work = (left ^ right) & 0xaaaaaaaa;
	left ^= work;
	right ^= work;
	left = (left >> 1) | (left  << 31);
	work = ((left >> 8) ^ right) & 0xff00ff;
	right ^= work;
	left ^= work << 8;
	work = ((left >> 2) ^ right) & 0x33333333;
	right ^= work;
	left ^= work << 2;
	work = ((right >> 16) ^ left) & 0xffff;
	left ^= work;
	right ^= work << 16;
	work = ((right >> 4) ^ left) & 0x0f0f0f0f;
	left ^= work;
	right ^= work << 4;

	*block++ = right;
	*block = left;
}

void DES3_CBCInit(DES3_CBC_CTX *context, uint8_t *key, uint8_t *iv, int32_t encrypt){
	context->encrypt = encrypt;
	scrunch(context->iv, iv);
	scrunch(context->originalIV, iv);
	deskey_(context->subkeys[0], encrypt ? key : &key[16], encrypt);
	deskey_(context->subkeys[1], &key[8], !encrypt);
	deskey_(context->subkeys[2], encrypt ? &key[16] : key, encrypt);
}

static int32_t DES3_CBCUpdate(DES3_CBC_CTX *context, uint8_t *output, uint8_t *input, uint32_t len){
    uint32_t inputBlock[2], work[2];
    uint32_t i;

    if(len % 8)                  /* length check */
        return 0;

	for(i = 0; i < len/8; i++) {
		scrunch(inputBlock, &input[8*i]);
		if(context->encrypt == 0) {
			*work = *inputBlock;
			*(work+1) = *(inputBlock+1);
		}
		else {
			*work = *inputBlock ^ *context->iv;
			*(work+1) = *(inputBlock+1) ^ *(context->iv+1);
		}

		desfunc(work, context->subkeys[0]);
		desfunc(work, context->subkeys[1]);
		desfunc(work, context->subkeys[2]);

		if(context->encrypt == 0) {
			*work ^= *context->iv;
			*(work+1) ^= *(context->iv+1);
			*context->iv = *inputBlock;
			*(context->iv+1) = *(inputBlock+1);
		}
		else {
			*context->iv = *(work);
			*(context->iv+1) = *(work+1);
		}
		unscrunch(&output[8*i], work);
	}
	memset_(inputBlock, 0, sizeof(inputBlock));
	memset_(work, 0, sizeof(work));
    return len;
}

void DES3_CBCRestart(DES3_CBC_CTX *context){
	// Restore the original IV
	*context->iv = *context->originalIV;
	*(context->iv+1) = *(context->originalIV+1);
}

static void scrunch(uint32_t *into, uint8_t *outof){
	*into    = (*outof++ & 0xffL) << 24;
	*into   |= (*outof++ & 0xffL) << 16;
	*into   |= (*outof++ & 0xffL) << 8;
	*into++ |= (*outof++ & 0xffL);
	*into    = (*outof++ & 0xffL) << 24;
	*into   |= (*outof++ & 0xffL) << 16;
	*into   |= (*outof++ & 0xffL) << 8;
	*into   |= (*outof   & 0xffL);
}

static void unscrunch(uint8_t *into, uint32_t *outof){
    *into++ = (uint8_t)((*outof >> 24) & 0xffL);
    *into++ = (uint8_t)((*outof >> 16) & 0xffL);
    *into++ = (uint8_t)((*outof >>  8) & 0xffL);
    *into++ = (uint8_t)( *outof++      & 0xffL);
    *into++ = (uint8_t)((*outof >> 24) & 0xffL);
    *into++ = (uint8_t)((*outof >> 16) & 0xffL);
    *into++ = (uint8_t)((*outof >>  8) & 0xffL);
    *into   = (uint8_t)( *outof        & 0xffL);
}

static void deskey_(uint32_t subkeys[32], uint8_t key[8], int32_t encrypt){
    uint32_t kn[32];
	int i, j, l, m, n;
    uint8_t pc1m[56], pcr[56];

	for(j = 0; j < 56; j++) {
		l = pc1[j];
		m = l & 07;
        pc1m[j] = (uint8_t)((key[l >> 3] & bytebit[m]) ? 1 : 0);
	}
	for(i = 0; i < 16; i++) {
		m = i << 1;
		n = m + 1;
		kn[m] = kn[n] = 0L;
		for(j = 0; j < 28; j++) {
			l = j + totrot[i];
			if(l < 28) pcr[j] = pc1m[l];
			else pcr[j] = pc1m[l - 28];
		}
		for(j = 28; j < 56; j++) {
			l = j + totrot[i];
			if(l < 56) pcr[j] = pc1m[l];
			else pcr[j] = pc1m[l - 28];
		}
		for(j = 0; j < 24; j++) {
			if(pcr[pc2[j]])
				kn[m] |= bigbyte[j];
			if(pcr[pc2[j+24]])
				kn[n] |= bigbyte[j];
		}
	}
	cookey(subkeys, kn, encrypt);


	memset_(pc1m, 0, sizeof(pc1m));
	memset_(pcr, 0, sizeof(pcr));
	memset_(kn, 0, sizeof(kn));
}

static void cookey(uint32_t *subkeys, uint32_t *kn, int32_t encrypt){
    uint32_t *cooked, *raw0, *raw1;
    int32_t increment;
    uint32_t i;

	raw1 = kn;
	cooked = encrypt ? subkeys : &subkeys[30];
	increment = encrypt ? 1 : -3;

	for (i = 0; i < 16; i++, raw1++) {
		raw0 = raw1++;
		*cooked    = (*raw0 & 0x00fc0000L) << 6;
		*cooked   |= (*raw0 & 0x00000fc0L) << 10;
		*cooked   |= (*raw1 & 0x00fc0000L) >> 10;
		*cooked++ |= (*raw1 & 0x00000fc0L) >> 6;
		*cooked    = (*raw0 & 0x0003f000L) << 12;
		*cooked   |= (*raw0 & 0x0000003fL) << 16;
		*cooked   |= (*raw1 & 0x0003f000L) >> 4;
		*cooked   |= (*raw1 & 0x0000003fL);
		cooked += increment;
	}
}

static int base64_decode(uint8_t *in,uint8_t *out,int32_t size){
    int32_t i,len;
    uint8_t tmp;
    if(size % 4)
        return 0;
    for(i = 0; i<size;++i){
        tmp = in[i];
        if(tmp >= 'A' &&
                tmp <= 'Z')
            in[i] = tmp - 'A'+0;
        else if(tmp >= 'a' &&
                tmp <= 'z')
            in[i] = tmp - 'a' + 26;
        else if(tmp >= '0' &&
                tmp <= '9')
            in[i] = tmp - '0' + 52;
        else if(tmp == '+')
            in[i] = 62;
        else if(tmp == '/')
            in[i] = 63;
        else
            in[i] = 64;
    }
    len = size >> 2;
    for(i = 0; i<len;++i){
        tmp = in[i*4 + 0x0] << 2;
        tmp |= (in[i*4 + 0x1] >> 4) & 0x3;
        out[i * 3 + 0] = tmp;

        tmp = in[i*4 + 0x1] & 0xf;
        tmp <<= 4;
        tmp |= (in[i*4 + 0x2] >> 2) & 0xf;
        out[i * 3 + 1] = tmp;

        tmp = in[i*4 + 0x2] & 0x3;
        tmp <<= 6;
        tmp |= in[i*4 + 0x3] & 0x3f;
        out[i * 3 + 2] = tmp;
    }
    len *= 3;
    if(in[size - 1] == 64)
        --len;
    if(in[size - 2] == 64)
        --len;
    return len;
}

static int base64_encode(uint8_t *in,uint8_t *out,int32_t size){
    int32_t i,len;
    uint8_t tmp;
    if(size == 0)
        return 0;
    len = ((size +2) / 3) *3;
    for(i = 0; i<(len / 3);++i){
        tmp = in[i*3 + 0x0] >> 2;
        out[i * 4 + 0] = tmp;

        tmp = in[i*3 + 0x0] & 0x3;
        tmp <<= 4;
        tmp |= in[i*3 + 0x1] >> 4;
        out[i * 4 + 1] = tmp;

        tmp = in[i*3 + 0x1] & 0xf;
        tmp <<= 2;
        tmp |= in[i*3 + 0x2] >> 6;
        out[i * 4 + 2] = tmp;

        tmp = in[i*3 + 0x2] & 0x3f;
        out[i * 4 + 3] = tmp;
    }
    if(len > size){
        --i;
        out[i * 4 + 3] = 64;
        if((len - size) <= 1){
            out[i * 4 + 2] &= ~0x3;
        }else{
            out[i * 4 + 2] = 64;
            out[i * 4 + 1] &= ~0xf;
        }
    }
    len /= 3;
    len *= 4;
    for(i = 0; i<len;++i){
        tmp = out[i];
        if(tmp <= 25)
            out[i] = tmp + 'A';
        else if(tmp <= 51)
            out[i] = tmp - 26 + 'a';
        else if(tmp <= 61)
            out[i] = tmp - 52 + '0';
        else if(tmp == 62)
            out[i] = '+';
        else if(tmp == 63)
            out[i] = '/';
        else
            out[i] = '=';
    }
    return len;
}

static void padding_encode(uint8_t *buf,int32_t inLen,int32_t outLen,uint8_t padStyle){
    if(inLen >= outLen)
        return;
    if(padStyle == DES3_PADDING_ZEROS)
        memset(&buf[inLen],0,outLen - inLen);
    else{
        memset(&buf[inLen],outLen - inLen,outLen - inLen);
    }
}
static int32_t padding_decode(uint8_t *buf,int32_t inLen,int32_t outLen,uint8_t padStyle){
    int32_t tmp,i;
    if(buf[inLen] >= outLen)
        return inLen;
    tmp = buf[inLen-1];
    for(i = 0;i < inLen;++i){
        if(buf[inLen - i - 1] != tmp)
            break;
    }
    if(padStyle == DES3_PADDING_ZEROS){
        if(tmp == 0)
            return inLen - i;
        return inLen;
    }
    if(tmp == i)
        return inLen - i;
    return inLen;
}
int32_t des3_encode(DES3_PARA *para){
    uint8_t subKeys[25];
    uint8_t iv[9];
    DES3_CBC_CTX des3;
    int32_t len,maxLen;
    maxLen = ((para->encryptLen + 7) >> 3) << 3;
    maxLen += 2;
    maxLen /= 3;
    maxLen *= 4;
    maxLen += 1;

    memset(subKeys,0,sizeof(subKeys));
    memset(iv,0,sizeof(iv));
    memset(para->outBuf,0,maxLen);

    strncpy(subKeys,para->subKeys,24);
    padding_encode(subKeys,strlen(para->subKeys),24,para->paddStyle);

    strncpy(iv,para->iv,8);
    padding_encode(iv,strlen(para->iv),8,para->paddStyle);

    len = ((para->encryptLen + 7) >> 3) << 3;
    padding_encode(para->inBuf,strlen(para->inBuf),len,para->paddStyle);

    DES3_CBCInit(&des3,subKeys,iv,1);
    len = DES3_CBCUpdate(&des3,para->outBuf,para->inBuf,len);

    memset(para->inBuf,0,maxLen);
    len = base64_encode(para->outBuf,para->inBuf,len);

    memset(para->outBuf,0,maxLen);
    strncpy(para->outBuf,para->inBuf,len);
    return len;
}

int32_t des3_decode(DES3_PARA *para){
    uint8_t subKeys[25];
    uint8_t iv[9];
    DES3_CBC_CTX des3;
    int32_t len,maxLen;
    maxLen = ((para->encryptLen + 7) >> 3) << 3;
    maxLen += 2;
    maxLen /= 3;
    maxLen *= 4;
    maxLen += 1;

    memset(subKeys,0,sizeof(subKeys));
    memset(iv,0,sizeof(iv));
    memset(para->outBuf,0,maxLen);

    strncpy(subKeys,para->subKeys,24);
    padding_encode(subKeys,strlen(para->subKeys),24,para->paddStyle);

    strncpy(iv,para->iv,8);
    padding_encode(iv,strlen(para->iv),8,para->paddStyle);
    len = base64_decode(para->inBuf,para->outBuf,strlen(para->inBuf));
    memset(para->inBuf,0,maxLen);
    DES3_CBCInit(&des3,subKeys,iv,0);
    len = DES3_CBCUpdate(&des3,para->inBuf,para->outBuf,len);
    len = padding_decode(para->inBuf,len,para->encryptLen,para->paddStyle);

    memset(para->outBuf,0,maxLen);
    strncpy(para->outBuf,para->inBuf,len);
    return len;
}


