#include "Ustring.h"

#if USE_MINIGUI==0

void Int2Unicode(int number, U16 str[])	//int 到Unicode字符串的转换
{
	U16 tmpstr[20];
	int i,temp,tmpnum=number;

	if(tmpnum==0){
		*str='0';
		str++;
		*str=0;
		return;
	}

	if(tmpnum<0){
		tmpnum=-tmpnum;
		*str='-';
		str++;
	}
	for(i=0;tmpnum;i++){
		temp=tmpnum%10;
		tmpnum/=10;
		tmpstr[i]=temp+'0';
	}
	i--;
	for(;i>=0;i--){
		*str=tmpstr[i];
		str++;
	}
	*str=0;
}

//Unicode字符串到int的转换
int Unicode2Int(U16 str[])
{
	int i,temp=0;;
	if(str[0]=='-'){
		for(i=1;str[i];i++){
			temp*=10;
			if(str[i]>'9' || str[i]<'0')
				break;
			temp+=str[i]-'0';
		}
		temp=-temp;
	}
	else{
		for(i=0;str[i];i++){
			temp*=10;
			if(str[i]>'9' || str[i]<'0')
				break;
			temp+=str[i]-'0';
		}
	}
	return temp;
}

//char类型包括GB编码,到Unicode的编码转换
void strChar2Unicode(U16 ch2[], const char ch1[])
{
	while(*ch1){
		*ch2=*ch1;
		ch2++;
		ch1++;
	}
	*ch2=0;
}

void UstrCpy(U16 ch1[], U16 ch2[])
{
	int i=0;
	if(ch2==NULL){
		ch1[0]=0;
		return;
	}

	while((*ch2)!=0){
		ch1[i++]=(*ch2);
		ch2++;
	}
	ch1[i]=0;
}

#endif //#if USE_MINIGUI==0

