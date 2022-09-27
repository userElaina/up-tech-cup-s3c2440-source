#include "tty.h"
#include "stdio.h"
#undef uchar
#define uchar unsigned char

void print_space(uchar n);

void enter();

void print_move_n(int n);

void print_chinese(uchar *data,uchar n);
void set_chinese_enter(void);
void set_chinese_end(void);
void print(uchar *data,uchar n);
void set_extend(uchar n);
//void tab();

void print_EAN();
