#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXSEND 33
#define MAXRECV 137

void showPage1(void){			//显示第一个界面
	printf("Welcome to Weather Forecast Demo Program !\n");
	printf("Please input City Name in Chinese pinyin(e.g. nanjing or beijing)\n");
	printf("(q)cls,(#)exit\n");
}
void showPage2(void){			//显示第二个界面
	printf("Please enter the given number to query\n");
	printf("1.today\n2.three days from today\n3.custom day by yourself\n");
	printf("(r)back,(q)cls,(#)exit\n");
}

void weaExp(char i,char* weather){		//将报文中的数据转换成对应的天气含义
	switch(i){
		case 0x00: strcpy(weather,"shower"); break;
		case 0x01: strcpy(weather,"clear"); break;
		case 0x02: strcpy(weather,"cloudy"); break;
		case 0x03: strcpy(weather,"rain"); break;
		case 0x04: strcpy(weather,"fog"); break;
	}
}

void printWeather(char* data){		//根据data打印出相关天气信息
	if(data[0] == 'D')		//无法提供当天天气
		printf("Sorry, no given day's weather information for city nanjing!\n");
	else if(data[0] == 'C'){		//可以提供当天天气
		int year = (data[32]&0x000000ff)*256 + (data[33]&0x000000ff);	//!!!!!!
		printf("City: %s  Today is %d/%d/%d  Weather information is as follows:\n",data+2,year,data[34],data[35]);
		if(data[1] == 'A'){		//提供某一天天气
			char weather[8];
			weaExp(data[37],weather);
			if(data[36] == 0x01){
				printf("Today's Weather is: %s Wind-level: %d Temp:%d\n",weather,data[38],data[39]);
			}
			else{
				printf("The %dth day's Weather is: %s Wind-level: %d Temp:%d\n",data[36],weather,data[38],data[39]);
			}
		}
		else if(data[1] == 'B' && data[36] == 0x03){		//提供三天内的天气
			char weather1[8],weather2[8],weather3[8];
			weaExp(data[37],weather1);
			weaExp(data[40],weather2);
			weaExp(data[43],weather3);
			printf("The 1th day's Weather is: %s Wind-level: %d Temp:%d\n",weather1,data[38],data[39]);
			printf("The 2th day's Weather is: %s Wind-level: %d Temp:%d\n",weather2,data[41],data[42]);
			printf("The 3th day's Weather is: %s Wind-level: %d Temp:%d\n",weather3,data[44],data[45]);
		}
	}
}

int main(int argc,char **argv){
	int sockfd;
	struct sockaddr_in servaddr;
	char sendline[MAXSEND], recvline[MAXRECV];
	char city[10];
	char charOfDay[2];

	if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){		//创建socket
		perror("socket created error\n");
		exit(1);
	}

	memset(&servaddr, 0, sizeof(servaddr));			//设置服务器套接字地址
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("114.212.191.33");
	servaddr.sin_port = htons(5050);

	if( connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ){	//连接服务器
		perror("connect error\n");
	}	
	
	system("clear");
	showPage1();
	
	while(1){
		scanf("%s",city);		//读取用户输入的城市
		if( city[0] == '#' && strlen(city) == 1 ){		//exit
			close(sockfd);
			exit(0);
		}
		else if( city[0] == 'q' && strlen(city)==1 ){		//cls
			system("clear");
			showPage1();
		}
		else{			
			memset(sendline, 0, MAXSEND);
			strcpy(&sendline[2], city);
			send(sockfd, sendline, MAXSEND, 0);	//将城市信息send到服务器
			if(recv(sockfd, recvline, MAXRECV, 0) == 0){	//receive服务器返回的数据	
				perror("receive error\n");
				exit(1);
			}
			if(recvline[0] == 'B'){		//服务器不可以提供此城市的天气信息
				printf("Sorry, Server does not have weather information for city %s!\n",city);
				showPage1();
			}
			else if(recvline[0] == 'A'){		//服务器可以提供此城市的天气信息
				system("clear");
				showPage2();
				while(1){
					
					scanf("%s",charOfDay);		//读取用户选择的功能
					if(charOfDay[0] == 'r' && strlen(charOfDay) == 1){		//cls
						system("clear");
						showPage1();
						break;
					}
					else if(charOfDay[0] == 'q' && strlen(charOfDay) == 1){		//back
						system("clear");
						showPage2();
					}
					else if(charOfDay[0] == '#' && strlen(charOfDay) == 1){		//exit
						close(sockfd);
						exit(0);
					}
					else if(charOfDay[0] == '1' && strlen(charOfDay) == 1){		//today
						memset(sendline, 0, MAXSEND);
						sendline[0] = 0x01;
						sendline[1] = 'A';
						strcpy(&sendline[2], city);
						sendline[32] = 0x01;
						send(sockfd, sendline, MAXSEND, 0);
						if(recv(sockfd, recvline, MAXRECV, 0) == 0){
							perror("receive error\n");
							exit(1);
						}
						//读取到服务器返回的天气信息，并分析打印出来
						printWeather(recvline);
					}
					else if(charOfDay[0] == '2' && strlen(charOfDay) == 1){		//three days from today
						memset(sendline, 0, MAXSEND);
						sendline[0] = 0x01;
						sendline[1] = 'B';
						strcpy(&sendline[2], city);
						sendline[32] = 0x03;
						send(sockfd, sendline, MAXSEND, 0);
						if(recv(sockfd, recvline, MAXRECV, 0) == 0){
							perror("receive error\n");
							exit(1);
						}
						//读取到服务器返回的天气信息，并分析打印出来
						printWeather(recvline);
					}
					else if(charOfDay[0] == '3' && strlen(charOfDay) == 1){		//custom day by yourself
						printf("Please enter the day number(below 10, e.g. 1 means today):");
						scanf("%s",charOfDay);
						memset(sendline, 0, MAXSEND);
						sendline[0] = 0x01;
						sendline[1] = 'A';
						strcpy(&sendline[2], city);
						while(strlen(charOfDay) != 1 || charOfDay[0] == '0'){		//判断是否输入的值在1-10之间
							printf("Please enter the day number(below 10, e.g. 1 means today):");
							scanf("%s",charOfDay);
						}
						sendline[32] = charOfDay[0] - 48;
						send(sockfd, sendline, MAXSEND, 0);
						if(recv(sockfd, recvline, MAXRECV, 0) == 0){
							perror("receive error\n");
							exit(1);
						}
						//读取到服务器返回的天气信息，并分析打印出来
						printWeather(recvline);
					}
					else{
						printf("input error!\n");
					}
				}
			}
		}
	}
	
}
