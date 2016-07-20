#include <stdio.h>
#include <string.h>

typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef int int32_t;
typedef struct{
    uint8_t encryptLen; //待编码长度,最多支持长度为 DES3_MAX_LEN
    uint8_t *inBuf; //待加解密字符串,字符串最后一个字符的值不能小于8
    uint8_t *outBuf; //输出加解密字符串，
    //输入输出缓冲大小不得小于 (((((encryptLen+7) >> 3) << 3) + 2) / 3) *4 +1
    uint8_t *subKeys;  //密钥,当前只支持长度为192位,即24字节,若少于24字节,按填充方式进入处理.
    uint8_t *iv; //密钥偏移量,当前只支持64位,即8字节,若少于8字节,按填充方式进入处理.
#define DES3_ENCRYPSTLYE_CBC 0
    uint8_t encryptStlye;  //加解密方式，当前只支持CBC
#define DES3_PADDING_PKCS7 0
#define DES3_PADDING_ZEROS 1
    uint8_t paddStyle;  //填充方式
}DES3_PARA;

//加密字符串，成功返回加密后的字符串长度，出错，返回0
extern int32_t des3_encode(DES3_PARA *para);
//解密字符串，成功返回解密后的字符串长度，出错，返回0
extern int32_t des3_decode(DES3_PARA *para);
