/********************************************************/
/*  file : get_input.h					*/
/*							*/
/*  prototypes for get_input functions			*/
/*							*/
/*  Maintained by: Markus Joos, CERN PH/ESS		*/
/********************************************************/

#ifndef _GET_INPUT_H
#define _GET_INPUT_H

#ifdef __cplusplus
extern "C" 
{
#endif

  long getdec(void);
  long getdecd(long defa);
  unsigned long gethex(void);
  unsigned long gethexd(unsigned long defa);
  unsigned long long gethexdll(unsigned long long defa);
  char getfstchar(void);
  void getstrd(char* a, char* defa);
  float getfloatd(float defa);

  int ReadCard_int (char* filename, char* tag, int nr, int* i);
  int ReadCard_uint(char* filename, char* tag, int nr, unsigned int* u);
  int ReadCard_str (char* filename, char* tag, int nr, char* c);

#ifdef __cplusplus
}
#endif

#endif

