
#include "macro.h"
#include"lib.h"
//#include"ip_addr.h"
#define MAX_NET_PORT	0x01
#define DEFAULT_IP		((192)|(168<<8)|(0<<16)|(1<<24))
#define DEFAULT_MASK	((255)|(255<<8)|(255<<16)|(0<<24))
#define DEFAULT_GATE	((192)|(168<<8)|(0<<16)|(254<<24))

#define IP4_ADDRESS32(ipaddr, a,b,c,d) (ipaddr) = ((U32)(d & 0xff) << 24) | ((U32)(c & 0xff) << 16) | \
                                                         ((U32)(b & 0xff) << 8) | (U32)(a & 0xff)

U8 defaultMacAddr[]={0,0xe0,0x4c,0x73,0x50,0x81}; 
extern U8 EmbeddedMac[6];
//U32 ipaddr,gwaddr,ipmask;
U32 ipaddr32,ipmaskaddr32,ipgateaddr32;
U8 Mac[6];  //全局变量，保存所存取的地址

 
extern U32 tx_tmpbuf[];

void ShowMacAddress();
void ShowIPAddress();
void ShowGwAddress();
void ShowMask();

void SetDefaultAddress()
{
	U8 *p;
	U32 tmpipaddr,tmpipmask,tmpgwaddr;
	
	tmpipaddr=DEFAULT_IP;
	tmpipmask=DEFAULT_MASK;
	tmpgwaddr=DEFAULT_GATE;

	printk("Set current Net Mac address is: %x %x %x %x %x %x\n", defaultMacAddr[0],
		defaultMacAddr[1],defaultMacAddr[2],defaultMacAddr[3],defaultMacAddr[4],defaultMacAddr[5]);

	p=(U8*)&tmpipaddr;
	printk("Set current IP address is: %d.%d.%d.%d\n", p[0],p[1],p[2],p[3]);
	p=(U8*)&tmpipmask;
	printk("Set current Subnet mask is: %d.%d.%d.%d\n", p[0],p[1],p[2],p[3]);
	p=(U8*)&tmpgwaddr;
	printk("Set current Gate way IP address is: %d.%d.%d.%d\n", p[0],p[1],p[2],p[3]);
	printk("Are you sure?[y/n]\n");

	if(Uart_Getch1(0)!='y')
		return;

	memcpy(EmbeddedMac,defaultMacAddr,sizeof(EmbeddedMac));
	p=(U8*)&tmpipaddr;
	IP4_ADDRESS32(ipaddr32,p[0],p[1],p[2],p[3]);
	p=(U8*)&tmpipmask;
	IP4_ADDRESS32(ipmaskaddr32,p[0],p[1],p[2],p[3]);
	p=(U8*)&tmpgwaddr;
	IP4_ADDRESS32(ipgateaddr32,p[0],p[1],p[2],p[3]);
	memcpy(Mac, EmbeddedMac, 6);

}

void SetNetWork()
{
	while(1){
		char rev;

  loop: printk("\n");
		printk("                ******************************\n");
		printk("                *  Set NetWork Address Menu  *\n");
		printk("                *----------------------------*\n");
		printk("                *  P(p)>>Mac address         *\n");
		printk("                *  I(i)>>ip  address         *\n");
		printk("                *  M(m)>>Subnet mask         *\n");
		printk("                *  G(g)>>Gate way ip address *\n");
		printk("                *  D(d)>>Set default address *\n");
		printk("                *  Q(q)>>Quit                *\n");
		printk("                ******************************\n");

		rev=Uart_Getch1(0);
		switch(rev){
		case 'P':
		case 'p':
			ShowMacAddress();
			break;
		case 'I':
		case 'i':
			ShowIPAddress();
			break;
		case 'M':
		case 'm':
			ShowMask();
			break;
		case 'G':
		case 'g':
			ShowGwAddress();
			break;
		case 'D':
		case 'd':
			SetDefaultAddress();
			break;
		case 'Q':
		case 'q':
			return;
		}
	}
}

#define SETIP4_ADDRESS(ipaddr, a,b,c,d) (ipaddr) = ((U32)(a & 0xff) << 24) | ((U32)(b & 0xff) << 16) | \
                                                         ((U32)(c & 0xff) << 8) | (U32)(d & 0xff)
                                                    

U8 String2IPAddress(char str[], U32* paddr)
{
	/*ip address format: xxx.xxx.xxx.xxx
	U32 paddr 高字节在前*/
	char* ptext=str;
	int tmp[4],i,num;

	for(i=0;i<4;i++){
		tmp[i]=0;
		while(*ptext && *ptext !='.'){
			tmp[i]*=10;
			num=(*ptext)-'0';
			if(num<0 || num>9)
				return 0;
			tmp[i]+=num;
			ptext++;
		}
		ptext++;
	}
	SETIP4_ADDRESS(*paddr, tmp[3], tmp[2], tmp[1], tmp[0]);
	return 1;
}

U8 String2MacAddress(char str[], U8 Macaddr[])
{
	char* ptext=str;
	int i,num;

	for(i=0;i<6;i++){
		Macaddr[i]=0;
		while(*ptext && *ptext !=' '){
			Macaddr[i]<<=4;
			if(((*ptext)>='0' && (*ptext)<='9')){
				num=(*ptext)-'0';
			}
			else if(((*ptext)>='a' && (*ptext)<='f')){
				num=(*ptext)-'a'+10;
			}
			else{
				return 0;
			}

			Macaddr[i]|=num;
			ptext++;
		}
		ptext++;
	}
	return 1;
}

void ShowMacAddress()
{
	char ch;
	static char str[100];
	U8 NewMac[6];
	printk("Current Net Mac address is: %x %x %x %x %x %x\n", EmbeddedMac[0],
		EmbeddedMac[1],EmbeddedMac[2],EmbeddedMac[3],EmbeddedMac[4],EmbeddedMac[5]);
	printk("Do you want to change the Mac address?[y/n]\n");

	ch=Uart_Getch1(0);
	if(ch!='y' && ch!='Y')
		return;
	printk("Please input the New Mac Address (FF FF FF FF FF FF)\n");
	Uart_GetString1(0,str);

	if(!String2MacAddress(str, NewMac)){
		printk("Wrong Address!\n");
		return;
	}

	memcpy(EmbeddedMac,NewMac, 6);
	printk("New Mac Address is set.\n");
	printk("Current Net Mac address is: %x %x %x %x %x %x\n", EmbeddedMac[0],
		EmbeddedMac[1],EmbeddedMac[2],EmbeddedMac[3],EmbeddedMac[4],EmbeddedMac[5]);
		memcpy(Mac, EmbeddedMac, 6);
		
}

void ShowIPAddress()
{
	char ch;
	static char str[100];
	U8 *p=(U8*)&ipaddr32;
	//U8 *p=(U8*)&ipaddr;
	U32 NewIpaddr;

	printk("Current IP address is: %d.%d.%d.%d\n", p[0],p[1],p[2],p[3]);
	printk("Do you want to change the IP address?[y/n]\n");

	ch=Uart_Getch1(0);
	if(ch!='y' && ch!='Y')
		return;
	printk("Please input the New IP Address (xxx.xxx.xxx.xxx)\n");
	Uart_GetString1(0,str);

	if(!String2IPAddress(str, &NewIpaddr)){
		printk("Wrong IP Address!\n");
		return;
	}

	ipaddr32=NewIpaddr;
	printk("New IP Address is set.\n");
	printk("Current Net IP address is: %d.%d.%d.%d\n", p[0],p[1],p[2],p[3]);
	//IP4_ADDRESS32(ipaddr32,p[0],p[1],p[2],p[3]);
	
}

void ShowGwAddress()
{
	char ch;
	static char str[100];
	U8 *p=(U8*)&ipgateaddr32;
	//U8 *p=(U8*)&gwaddr;
	U32 NewIpaddr;

	printk("Current Gate way IP address is: %d.%d.%d.%d\n", p[0],p[1],p[2],p[3]);
	printk("Do you want to change the Gate way IP address?[y/n]\n");

	ch=Uart_Getch1(0);
	if(ch!='y' && ch!='Y')
		return;
	printk("Please input the New Gate way IP Address (xxx.xxx.xxx.xxx)\n");
	Uart_GetString1(0,str);

	if(!String2IPAddress(str, &NewIpaddr)){
		printk("Wrong Gate way IP Address!\n");
		return;
	}

	ipgateaddr32=NewIpaddr;
	printk("New Gate way IP Address is set.\n");
	printk("Current Gate way IP address is: %d.%d.%d.%d\n", p[0],p[1],p[2],p[3]);
	//IP4_ADDRESS32(ipgateaddr32,p[0],p[1],p[2],p[3]);
	
}

void ShowMask()
{
	char ch;
	static char str[100];
	U8 *p=(U8*)&ipmaskaddr32;
	//U8 *p=(U8*)&ipmask;
	
	U32 NewIpaddr;

	printk("Current Subnet mask is: %d.%d.%d.%d\n", p[0],p[1],p[2],p[3]);
	printk("Do you want to change the Subnet mask?[y/n]\n");

	ch=Uart_Getch1(0);
	if(ch!='y' && ch!='Y')
		return;
	printk("Please input the New Subnet mask(xxx.xxx.xxx.xxx)\n");
	Uart_GetString1(0,str);

	if(!String2IPAddress(str, &NewIpaddr)){
		printk("Wrong Subnet mask!\n");
		return;
	}

	ipmaskaddr32=NewIpaddr;
	printk("New Subnet mask is set.\n");
	printk("Current Subnet mask is: %d.%d.%d.%d\n", p[0],p[1],p[2],p[3]);
	//IP4_ADDRESS32(ipmaskaddr32,p[0],p[1],p[2],p[3]);
}
U32 Get_ipaddr(void)
{
	return ipaddr32;
}
U32 Get_maskaddr(void)
{
	return ipmaskaddr32;
}
U32 Get_gwaddr(void)
{
	return ipgateaddr32;
}
U8 *Get_mac(void)
{
return Mac;
}
