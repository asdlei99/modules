#include "des3.h"
uint8_t secretKey[] = "123456781234567812345678";
uint8_t iv[] = "01234567";

static void test_des3(uint8_t *tmp,uint8_t paddingStyle){
    uint8_t out[65];
    uint8_t in[65];
    int32_t len;
    DES3_PARA des3Para;
    memset(in,0,sizeof(in));
    strncpy(in,tmp,64);
    des3Para.inBuf = in;
    des3Para.encryptStlye = DES3_ENCRYPSTLYE_CBC;
    if(paddingStyle == 0)
        des3Para.paddStyle = DES3_PADDING_PKCS7;
    else
        des3Para.paddStyle = DES3_PADDING_ZEROS;
    des3Para.iv = iv;
    des3Para.subKeys = secretKey;
    des3Para.outBuf = out;
    des3Para.encryptLen = 24;
    len = des3_encode(&des3Para);
    if(len)
        printf("encode:%s\n",des3Para.outBuf);
    else
         printf("encode:err\n");
    des3Para.inBuf = out;
    memset(in,0,sizeof(in));
    des3Para.outBuf = in;
    len = des3_decode(&des3Para);
    if(len)
        printf("encode:%s\n",des3Para.outBuf);
    else
         printf("encode:err\n");
}

int main(int argc,char *argv[]){
    printf("UINT1  size : %d\n",sizeof(uint8_t));
    printf("UINT2  size : %d\n",sizeof(uint16_t));
    printf("UINT4  size : %d\n",sizeof(uint32_t));
    printf("SINT1  size : %d\n",sizeof(int32_t));
    if(argc < 2){
        printf("please input\n");
        return 0;
    }
    test_des3((uint8_t *)argv[1],0);
    test_des3((uint8_t *)argv[1],1);

    return 0;
}
