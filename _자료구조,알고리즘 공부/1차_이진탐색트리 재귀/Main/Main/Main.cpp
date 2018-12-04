// Main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>

// 이진 탐색 알고리즘은, 데이터가 오름차순 혹은 내림차순으로 정렬되어 있어야 함.
int BSearchRecur(int arr[], int first, int last, int target);

int main()
{
	int arr[] = { 1, 3, 5, 7, 9 };

	// 7 찾기
	int idx = BSearchRecur(arr, 0, sizeof(arr) / sizeof(int) - 1, 7);

	// 탐색 결과 표시
	if (idx == -1)
		printf("Search fail..\n");
	else
		printf("Search Index = %d\n", idx);


	// 4 찾기
	idx = BSearchRecur(arr, 0, sizeof(arr) / sizeof(int) - 1, 4);

	// 탐색 결과 표시
	if (idx == -1)
		printf("Search fail..\n");
	else
		printf("Search Index = %d\n", idx);

	return 0;
}

int BSearchRecur(int arr[], int first, int last, int target)
{	
	// 탐색 실패 시 -1 리턴
	if (first > last)
		return -1;

	// 탐색 배열의 중앙을 찾는다.
	int mid = (first + last) / 2;

	// 중앙이 내가 찾던 Target일 경우, Target의 인덱스 리턴
	if (arr[mid] == target)
		return mid;

	// 중앙이 내가 찾던 Target이 아니라면, 좌 우를 뒤져본다.
	// 좌 뒤져보기
	else if (target < arr[mid])
		return BSearchRecur(arr, first, mid - 1, target);

	// 우 뒤져보기
	else
		return BSearchRecur(arr, mid + 1, last, target);
}
