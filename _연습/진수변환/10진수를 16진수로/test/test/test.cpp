// test.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include <windows.h>
#include <conio.h>

// 10--> 16함수
void DecimalToHexa()
{
	int decimal = 0;

	fputs("10진수 입력 : ", stdout);
	scanf_s("%d", &decimal);

	char print[20] = { 0, };

	int Index = 0;
	while (1)
	{
		// 16으로 나눈 나머지를 구함
		int mod = decimal % 16;

		// 나머지가 10보다 작으면, 0~9라는 것. 이 때는 문자열 0~9중 하나 입력
		if (mod < 10)
		{
			print[Index] = '0' + mod;
		}

		// 나머지가 10과 같거나 크다면, 10~15라는 것. 이 때는 문자열 a~f중 하나 입력
		// 16 나머지 연산을 했기 때문에, 최대 16이 된다.
		else
		{
			print[Index] = 'A' + mod - 10;
		}

		// 다음 나머지 연산을 위해, 몫 저장
		decimal = decimal / 16;

		Index++;

		if (decimal <= 0)
			break;
	}

	// 출력할 때는, 역순으로 출력한다.
	Index--;
	for (int i = Index; i >= 0; --i)
		printf("%c", print[i]);

	fputs("\n", stdout);

}

// 16--> 10함수
void HexaToDecimal()
{
	char Hexa[40];

	fputs("16진수 입력(최대 40글자) : ", stdout);
	scanf_s("%s", &Hexa, (UINT)sizeof(Hexa));

	// 문자 몇개있는지 구하기
	int Size = strlen(Hexa);

	// 문자열 수 만큼 돌면서 뒤 배열부터 처리하기.
	int Count = 1;
	int Total = 0;
	for (int i = Size-1; i >= 0; --i)
	{
		int Temp = 0;

		// 입력한 값이 0~9사이 라면 (문자지만 아스키코드 비교 시, 값이 숫자라면)
		if ('0' - 1 < Hexa[i] && Hexa[i] < '9'+1)
			Temp = Hexa[i] - '0';

		// 입력한 값이 10~15(a~f) 사이라면
		// 소문자를 적었을 때
		else if ('a' - 1 < Hexa[i] && Hexa[i] < 'z' + 1)
			Temp = (Hexa[i] - 'a') + 10;

		// 대문자를 적었을 떄
		else if ('A' - 1 < Hexa[i] && Hexa[i] < 'Z' + 1)
			Temp = (Hexa[i] - 'A') + 10;

		Total += Temp * Count;
		Count *= 16;
	}

	printf("%d\n", Total);
}

// 10 --> 2함수
void DecimalBinary()
{
	int decimal = 0;

	fputs("10진수 입력 : ", stdout);
	scanf_s("%d", &decimal);

	int Binary[2000] = { 0, };

	int index = 0;
	while (1)
	{
		// 나머지 구한 후 저장
		int mod = decimal % 2;

		Binary[index++] = mod;

		// 몫 구함.
		decimal = decimal / 2;

		// 몫이 1이거나 0이면 끝난것.
		if (decimal == 1 || decimal == 0)
		{	
			Binary[index] = decimal;
			break;
		}
	}

	// 뒤에서부터 출력한다.
	for (int i = index; i >= 0; --i)
		printf("%d", Binary[i]);

	fputs("\n", stdout);
}

// 2 --> 10함수
void BinaryToDecimal()
{
	char Binary[1000];

	fputs("2진수 입력 (최대 200글자. 0 or 1 이외 값 적으면 비정상값 출력됨.) : ", stdout);
	scanf_s("%s", &Binary, (UINT)sizeof(Binary));

	// 문자열 길이
	int Size = strlen(Binary);

	// 문자 수 만큼 돌면서 뒤 배열부터 처리하기.
	int Count = 1;
	int Total = 0;
	for (int i = Size - 1; i >= 0; --i)
	{
		int Temp = Binary[i] - '0';

		if(Temp == 1)
			Total += Temp * Count;

		Count *= 2;
	}

	printf("%d\n", Total);
}

int _tmain()
{
	char a = 0xA6;
	unsigned char b = 0xA6;

	while (1)
	{
		system("cls");

		fputs("0 : 프로그램 종료\n",stdout);
		fputs("1 : 10  ---> 16\n", stdout);
		fputs("2 : 16  ---> 10\n", stdout);
		fputs("3 : 10  ---> 2\n", stdout);
		fputs("4 :  2  ---> 10\n", stdout);

		int input = 0;

		scanf_s("%d", &input);

		if (input == 0)
			break;

		switch (input)
		{
		case 1:
			// 10->16
			DecimalToHexa();
			break;

		case 2:
			// 16->10
			HexaToDecimal();
			break;

		case 3:
			// 10->2
			DecimalBinary();
			break;

		case 4:
			// 2->10
			BinaryToDecimal();
			break;
		}

		fputs("계산 완료\n", stdout);
		_getch();
	}

	fputs("정상종료\n",stdout);
	return 0;
}

