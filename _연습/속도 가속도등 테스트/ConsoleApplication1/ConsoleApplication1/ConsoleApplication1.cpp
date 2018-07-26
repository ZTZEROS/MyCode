// ConsoleApplication1.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include <cmath>

#define FPS		(60)

// ----------------
// 시간 기준 fps 치환 매크로
// ----------------
// 초 (_sec 초에 몇fps인지)
#define SECOND(_sec)	((_sec)*FPS)

// 분 (_min 분에 몇fps인지)
#define MINUTE(_min)	(SECOND(_min)*60)

// 시 (_hour 시간에 몇fps인지)
#define HOUR(_hour)		(MINUTE(_hour)*60)


// ----------------
// M(미터) 기준, 거리 매크로
// ----------------
// m
#define M(_m)		(_m)

// cm
#define CM(_cm)		(M(0.01f * (_cm)))

// mm
#define MM(_mm)		(CM(0.1f * (_mm)))

// km
#define KM(_km)		(M(1000.0f * (_km)))


// --------------
// 속도 매크로
// --------------
// m/s를 입력받아서, 1프레임 당 속도 계산.
// 30m/s는 1초에 30m를 간다는 것
#define M_S(_speed)		(M(_speed) / SECOND(1.0f))

// km/h를 입력받아서 1프레임 당 속도 계산
// 60km/h는 1시간에 60km를 간다는 것.
#define KM_H(_speed)	(KM(_speed) / HOUR(1.0f))


// --------------
// 가속도 매크로
// --------------
// m/s^2를 입력받아, 가속도 계산
// 3.6m/s^2는 '1초동안 속도가 3.6m/s^2만큼 증가'한다는 것.
// [ 3.6m/s^2 = (3.6m / (1s * 1s)) = (3.6m / (1초당 프레임 수 * 1초당 프레임 수)) = (3.6m / (60 * 60)) = (3.6m / (3600)) = 0.001m/s^2 ] 
// 즉, [ 3.6m/s^2 = 1프레임에 0.001m/s^2 씩 빨라진다 ]이다.
#define M_S2(_acc)	(M(_acc) / (SECOND(1.0f) * SECOND(1.0f)))



void dimension_One()
{
	float pos = 0.0f;
	float speed = M_S(30.0f);	// 최초 속도 30.0m/s
	float acce = M_S2(2.0f);	// 가속도 2m/s^2

								// 2초 동안 처리 수행
	for (int i = 0; i < SECOND(2); ++i)
	{
		// 이동 전
		printf("%f초 : ( 좌표 : %f 속도 : %f ) -> ", float(i) / SECOND(1.0f), pos, speed);

		// 이동처리
		speed += acce;	// 속도 증가. [속도 = 가속도 * 시간]. 현재는 매 시간(프레임)마다 더해주고 있으니 [속도 += 가속도]로 계산중
		pos += speed;	// 거리 증가. [거리 = 속도 * 시간].	현재는 매 시간마다 이동처리 하니, [거리 += 속도]로 계산중

						// 이동 후
		printf("(좌표 : %f 속도 : %f )\n", pos, speed);
	}

	// -------------------------
	// 등가속도 운동 공식 (가속도가 일정한 것을 등가속도 운동이라 함)
	// 
	// 속도 = 처음속도 + (가속도 * 시간)	
	// 좌표(1차원) = 처음속도 * 시간 + 1/2 * 가속도 * 시간^2 = (처음속도 * 시간) + (1/2 * 가속도 * 시간 * 시간)
	// ---------------------------	

	// 2초일 때 속도와 거리
	float Time = 120.0f;
	float TestSpeed = M_S(30.0f) + M_S2(2.0f) * Time;
	float TestPos = (M_S(30.0f) * Time) + (M_S2(2.0f) * Time * Time / 2);


	int abc = 10;
}


struct Vector
{
	float x, y;
};

struct Point
{
	float x, y;
};

// 백터 * 스칼라
Vector Mul_Vector_Scalar(Vector v, float s)
{
	Vector r;
	r.x = v.x * s;
	r.y = v.y * s;
	return r;
}

// 백터의 길이를 얻는다.
float Vector_Length(Vector v)
{
	float l = sqrtf(v.x * v.x + v.y * v.y);
	return l;
}

// 백터 정규화
Vector Vector_Normalize(Vector v)
{
	float l = Vector_Length(v);
	Vector n;
	n.x = v.x / l;
	n.y = v.y / l;

	return n;
}

// 백터 길이 설정
Vector Vector_SetLength(Vector v, float length)
{
	v = Vector_Normalize(v);
	v = Mul_Vector_Scalar(v, length);
	return v;
}



void dimension_Two()
{
	// 백터에 속도 30m/s 설정
	Vector vec_seepd = { 1.0f, 1.0f };
	vec_seepd = Vector_SetLength(vec_seepd, M_S(30.0f));
	printf("30m/s --> (%f, %f)\n", vec_seepd.x, vec_seepd.y);


	// 백터어 가속도 2m/s^2설정
	Vector vec_acc = { 10.0f, 0.0f };
	vec_acc = Vector_SetLength(vec_acc, M_S2(2.0f));
	printf("2m/s^2 --> (%f, %f)\n", vec_acc.x, vec_acc.y);	
}

void dimension_Two_Test()
{
	// 처음 좌표
	Point pos = { 0.0f, 0.0f };

	// 처음 속도 (30m/s)
	Vector speed = { 1.0f, 2.0f };
	speed = Vector_SetLength(speed, M_S(30.0f));

	// 가속도 2m/s^2
	Vector acce = { 2.0f, 1.0f };
	acce = Vector_SetLength(acce, M_S2(2.0f));

	// 2초동안 처리 수행
	for (int h = 0; h < SECOND(3); ++h)
	{
		// 이동 전
		printf("%f초 : ( 좌표 : (%f, %f) 속도 : (%f, %f) ) ->\n", float(h) / SECOND(1.0f), pos.x, pos.y, speed.x, speed.y);

		// 이동처리
		speed.x += acce.x;
		speed.y += acce.y;
		
		pos.x += speed.x;
		pos.y += speed.y;

		// 이동 후
		printf("             ( 좌표 : (%f, %f) 속도 : (%f, %f) )\n", pos.x, pos.y, speed.x, speed.y);
	}

	int abc = 10;
}

int _tmain()
{	
	//dimension_One();
	
	//dimension_Two();

	dimension_Two_Test();
	
	
    return 0;
}

