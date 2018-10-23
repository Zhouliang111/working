#ifndef __pdu_decode_h
#define __pdu_decode_h



//½âÂëº¯Êý
int PDUdcode_7bit(const unsigned char *Src, char *Dst, int n_Srclength);
int PDUdcode_8bit(const unsigned char* Src, char* Dst, int n_Srclength);
int PDUdcode_Unicode(const unsigned char* Src, char* Dst, int n_Srclength);
int PDU_SerializeNumbers(const char* Src, char* Dst, int n_Srclength);
int PDU_StringtoBytes(const char* Src, unsigned char* Dst, int n_Srclength);
int PDU_Decode(const char* Src);



#endif

