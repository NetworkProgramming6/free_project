#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

struct card {
	char *color;
	int num;
};

void setCard(struct card*);
void mixCard(struct card*,struct card*);
void printAliveCard(int[4],struct card[4]);
void printLineOne(int,char*,int);
void printLineTwo(int,char*,int);
void printLineThree(int,char*,int);
void colorPrint(char*);
void printNullCard();

void setCard(struct card* card)
{
	int i,j;
	for(i=0;i<14;i++)
	{
		card[i].color="red";
		card[i+14].color="yellow";
		card[i+28].color="green";
		card[i+42].color="purple";
	}
	for(j=0;j<43;j+=14)
	{
		for(i=0;i<5;i++)
			card[i+j].num=1;
		for(i=5;i<8;i++)
			card[i+j].num=2;
		for(i=8;i<11;i++)
			card[i+j].num=3;
		for(i=11;i<13;i++)
			card[i+j].num=4;
		for(i=13;i<14;i++)
			card[i+j].num=5;
	}
}

void mixCard(struct card* beforeMix, struct card* afterMix)
{
	int cardFull[56],cnt=0,random;
	for(int i=0;i<56;i++)
		cardFull[i]=-1;
	srand(time(NULL));
	while(1)
	{
		random=rand()%56;
		if(cardFull[random]==-1)
		{
			afterMix[cnt].color=beforeMix[random].color;
			afterMix[cnt].num=beforeMix[random].num;
			cnt++;
			cardFull[random]++;
		}
		else
			;
		if(cnt==56)
			break;
	}
}

void printAliveCard(int alive[4], struct card card[4])
{
	int i;

	printf("┌────────────┐ ┌────────────┐ ┌────────────┐ ┌────────────┐\n");
	
	for(i=0;i<4;i++)
	{
		printLineOne(alive[i],card[i].color,card[i].num);
		printf(" ");
	}
	printf("\n");

	for(i=0;i<4;i++)
		printf("│            │ ");
	printf("\n");
	for(i=0;i<4;i++)
		printf("│            │ ");
	printf("\n");
	
	for(i=0;i<4;i++)
	{
		printLineTwo(alive[i],card[i].color,card[i].num);
		printf(" ");
	}
	printf("\n");

	for(i=0;i<4;i++)
		printf("│            │ ");
	printf("\n");
	for(i=0;i<4;i++)
		printf("│            │ ");
	printf("\n");

	for(i=0;i<4;i++)
	{
		printLineThree(alive[i],card[i].color,card[i].num);
		printf(" ");
	}
	printf("\n");
	printf("└────────────┘ └────────────┘ └────────────┘ └────────────┘\n");
}

void printLineOne(int alive,char* color,int num)
{
	printf("│");
	if(alive>0)
	{
		if(num==2 || num==3 || num==4 || num==5)
			colorPrint(color);
		else //num==1
			printf("  ");
		printf("        ");
		if(num==4 || num==5)
			colorPrint(color);
		else //num==1,2,3
			printf("  ");
		printf("│");
	}
	else //게임오버된 플레이어 카드
	{
		printf("│            │ ");
	}
}
void printLineTwo(int alive,char* color,int num)
{
	printf("│     ");
	if(alive>0)
	{
		if(num==1 || num==3 || num==5)
			colorPrint(color);
		else //num==2,4
			printf("  ");
	}
	else //게임오버된 플레이어 카드
		printf("  ");
	printf("     │");
}
void printLineThree(int alive, char* color, int num)
{
	printf("│");
	if(alive>0)
	{
		if(num==4 || num==5)
			colorPrint(color);
		else //num==1,2,3
			printf("  ");
		printf("        ");
		if(num==2 || num==3 || num==4 || num==5)
			colorPrint(color);
		else //num==1
			printf("  ");
		printf("│");
	}
	else //게임오버된 플레이어 카드
		printf("│            │");
}
void colorPrint(char* color)
{
	if(strcmp(color,"red")==0)
		printf("🍓");
	else if(strcmp(color,"yellow")==0)
		printf("🍋");
	else if(strcmp(color,"green")==0)
		printf("🍈");
	else if(strcmp(color,"purple")==0)
		printf("🍇");
	else
		;
}
void printNullCard()
{
	printf("┌────────────┐ ┌────────────┐ ┌────────────┐ ┌────────────┐\n");
	for(int i=0;i<7;i++)
	{
		printf("│            │ │            │ │            │ │            │\n");
	}
	printf("└────────────┘ └────────────┘ └────────────┘ └────────────┘\n");
}

