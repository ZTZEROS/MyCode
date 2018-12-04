// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>

// 하노이 타워
// from에서부터 by를 거쳐 to에 도착
void HanoiTower(int num, char from, char by, char to);

int main()
{
	HanoiTower(3, 'A', 'B', 'C');

	return 0;
}

void HanoiTower(int num, char from, char by, char to)
{
	// 종료조건
	// 이동할 원반이 1개라면.
	if (num == 1)
		printf("원반 1을 %c에서 %c로 이동 \n", from, to);

	// 그게 아니라면 로직 진행
	else
	{
		// 1단계. 작은 원반 n-1개를(가장 아래 원반을 제외한 모든 원반) A에서 B로 이동
		HanoiTower(num - 1, from, to, by);

		// 2단계. 큰 원반 1개를 A에서 C로 이동
		printf("원반 %d을(를) %c에서 %c로 이동 \n", num, from, to);

		// 3단계. B에있는 작은 원반 n-1개를 B에서 C로 이동
		HanoiTower(num - 1, by, from, to);
	}
}