                       /*  TRAFFIC LIGHT  */
//----------------------------------------------------------------------
// KHAI BAO THU VIEN

#include <mysql.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <time.h>
#include <wiringPiSPI.h>

//----------------------------------------------------------------------
// XAC DINH MOT SO CHAN VA KENH SPI

#define R1 33 
#define Y1 35
#define G1 37

#define R2 36
#define Y2 38
#define G2 40

#define R3 11
#define Y3 13
#define G3 15

#define channel 0

//----------------------------------------------------------------------
// DAT BIEN

// Dat bien cho mysql
MYSQL *conn;
MYSQL_RES *res;
MYSQL_ROW row;

char *server = "192.168.223.202";
char *user = "nguyen";
char *password = "08102001"; /* set me first */
char *database = "traffic_light";

// Dat bien cho cac cot den
int dem1=0,	check1=0, red1, green1;
int dem2=0,	check2=0, red2, green2;
int dem3=0,	check3=0, red3, green3;
int x,	y,	z;
int mode1,	mode2,	mode3;
char color1[5],	color2[5],	color3[5];

// Dat bien cho thoi gian
int t;

//----------------------------------------------------------------------
//                CAC HAM CON TRONG CHUONG TRINH

// Ham dem so cho tung cot den
int dem_so(int mode,int red, int green, char mau[5], int dem, int check,int den)
{
	if(mode==0)
	{
		if(dem<=0) 
		{
			switch (check)
			{
				case 0:
					dem=red;	check=1;
					sprintf(mau,"Do");
					printf("Trang thai den %d: %s\n",den,mau);
					break;
				case 1:
					dem=green;	check=2;
					sprintf(mau,"Xanh");
					printf("Trang thai den %d: %s\n",den,mau);
					break;
				case 2:
					dem=3;	check=0;
					sprintf(mau,"Vang");
					printf("Trang thai den %d: %s\n",den,mau);
					break;
			}
		}
		dem--;
	}
	else
	{
		dem=0;
		if(strcmp(mau,"Do")==0) check=1;
		else if(strcmp(mau,"Vang")==0) check=0;
		else if(strcmp(mau,"Xanh")==0)	check=2;
	}
	switch(den)
	{
		case 1:
			dem1=dem;	check1=check;	
			break;
		case 2:
			dem2=dem;	check2=check;
			break;
		case 3:
			dem3=dem;	check3=check;
			break;
	}
	return dem;
}

// Ham lien quan den mysql (nhan data va gui data)
void Mode_of_light(int den)
{
	int mode;
	char a[200];
	sprintf(a,"select * from data where STT=%d",den);
	mysql_query(conn, a);
	res = mysql_store_result(conn);
	while (row = mysql_fetch_row(res)) mode=atoi(row[4]);  
	switch(den)
	{
		case 1:
			mode1=mode;
			break;
		case 2:
			mode2=mode;
			break;
		case 3:
			mode3=mode;
			break;
	}  
}

void nhan_data_tu_database(int mode,int den)
{
	int red,green;
	char a[200],color[5];
	if(mode==0)
	{ 
		sprintf(a,"select * from data where STT=%d",den);
		mysql_query(conn, a);
		res = mysql_store_result(conn);
		while (row = mysql_fetch_row(res)) 
		{
			red=atoi(row[1]);
			green=atoi(row[3]);
		}
		switch(den)
		{
			case 1:
				red1=red;	green1=green;	mode1=mode;
				break;
			case 2:
				red2=red;	green2=green;	mode2=mode;
				break;
			case 3:
				red3=red;	green3=green;	mode3=mode;
				break;
		}
	}
	else
	{
		sprintf(a,"select * from status where Den=%d",den);
		mysql_query(conn, a);
		res = mysql_store_result(conn);
		while (row = mysql_fetch_row(res)) 
		{
			sprintf(color,row[1]);
		}
		switch(den)
		{
			case 1:
				sprintf(color1,color);
				break;
			case 2:
				sprintf(color2,color);
				break;
			case 3:
				sprintf(color3,color);
				break;
		}
	}
}

void gui_data_len_database(int mode,int den)
{
	if(mode==0)
	{
		int a;
		char color[5];
		switch(den)
		{
			case 1:
				sprintf(color,color1);	a=x;
				break;
			case 2:
				sprintf(color,color2);	a=y;
				break;
			case 3:
				sprintf(color,color3);	a=z;
				break;
		}
		char sql[200];
		sprintf(sql,"update status set Mau='%s', Time=%d, Mode=%d where Den=%d",color,a,mode,den);
		mysql_query(conn, sql);
	}
}

// Cac ham lien qua den MAX7219
void send_data(uint8_t address, uint8_t value)
{
	uint8_t data[2];
	data[0]=address;
	data[1]=value;
	wiringPiSPIDataRW(channel, data, 2);
}

void Init_max7219(void)
{
	// decode mode: 0x09FF
	send_data(0x09,0xFF);
	
	// intensity: 0x0A07
	send_data(0x0A,0x07);
	
	// scan limit
	send_data(0x0B, 7);
	
	// no shutdown, display test off
	send_data(0x0C,1);
	send_data(0x0F,0);
}

void hienthi()
{
	send_data(1,dem1%10);
	send_data(2,dem1/10);
	send_data(3,0x0A);
	send_data(4,dem2%10);
	send_data(5,dem2/10);
	send_data(6,0x0A);
	send_data(7,dem3%10);
	send_data(8,dem3/10);
}

// Ham hien thi den
void hienthiden(int check,int den)
{
	int R,Y,G;
	switch(den)
	{
		case 1:
			R=R1;	Y=Y1;	G=G1;
			break;
		case 2:
			R=R2;	Y=Y2;	G=G2;
			break;
		case 3:
			R=R3;	Y=Y3;	G=G3;
			break;
	}
	switch(check)
	{
		case 0:
			digitalWrite(G,0);
			digitalWrite(R,0);
			digitalWrite(Y,1);
			break;
		case 1:
			digitalWrite(Y,0);
			digitalWrite(G,0);
			digitalWrite(R,1);
			break;
		case 2:
			digitalWrite(R,0);
			digitalWrite(Y,0);
			digitalWrite(G,1);
			break;
	}
}

//Setup IO cho raspberrypi
void Setup_IO(int R,int Y, int G)
{
	pinMode(R,OUTPUT);
	pinMode(Y,OUTPUT);
	pinMode(G,OUTPUT);
	
	digitalWrite(R,0);
	digitalWrite(Y,0);
	digitalWrite(G,0);
}

//----------------------------------------------------------------------
//                     CHUONG TRINH CHINH

int main(void)
{
	// Ket noi mysql
	conn = mysql_init(NULL);
	mysql_real_connect(conn,server,user,password,database,0,NULL,0); 
	
	// Set up I/O Raspberry Pi
	wiringPiSetupPhys();
	Setup_IO(R1,Y1,G1);
	Setup_IO(R2,Y2,G2);
	Setup_IO(R3,Y3,G3);
	
	// Set up SPI Raspberry Pi
	wiringPiSPISetup(channel, 8000000);
	
	// Set up MAX7219
	Init_max7219();
    
    while(1)
    {
		if(millis()-t>=1000)
		{
			// Set lai thoi gian
			t=millis();
			
			// Doc mode den tu database
			Mode_of_light(1);
			Mode_of_light(2);
			Mode_of_light(3);
			
			// Doc  data tu database
			nhan_data_tu_database(mode1,1); 
			nhan_data_tu_database(mode2,2); 
			nhan_data_tu_database(mode3,3); 
			
			// Xu ly
			x=dem_so(mode1,red1,green1,color1,dem1,check1,1);
			y=dem_so(mode2,red2,green2,color2,dem2,check2,2);
			z=dem_so(mode3,red3,green3,color3,dem3,check3,3);
			 
			// gui data len database
			gui_data_len_database(mode1,1);
			gui_data_len_database(mode2,2);
			gui_data_len_database(mode3,3);
			
			// Hien thi len MAX7219
			hienthi();
			
			// Hien thi den
			hienthiden(check1,1);
			hienthiden(check2,2);
			hienthiden(check3,3);
			
			// Hien thi len terminal
			printf(" Den 1: %d \t Den 2: %d  \t Den 3: %d\n",x,y,z);
			
		}
	}
	
    mysql_close(conn);
    return 0;
}
