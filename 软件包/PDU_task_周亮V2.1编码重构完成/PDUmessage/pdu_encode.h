#ifndef __pdu_encode_h
#define __pdu_encode_h



//���뺯��
//ͷ�ļ��������ü�extern,������Ĭ�����
int PDUencode_7bit(const char* Src, unsigned char* Dst, int n_Srclength);
int PDUencode_8bit(const char* Src, unsigned char* Dst, int n_srclength);
int PDUencode_Unicode(const char* Src, unsigned char* Dst, int n_srclength);
int PDU_InvertNumbers(const char* Src, char* Dst, int n_srclength);
int PDU_BytestoString(const unsigned char* Src, char* Dst, int n_srclength);

int PDU_Encode(char* Dst);
int PDU_Encode_longmessage_NO1(char* Dst);
int PDU_Encode_longmessage_NO2(char* Dst);

#endif



