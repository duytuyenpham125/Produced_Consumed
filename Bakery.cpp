#include "iostream"
#include "stdio.h"
#include "Windows.h"

#define NUMTHREAD 2
#define BUFFERSIZE 6
#define N 100

volatile int buffer[BUFFERSIZE];
volatile int in = 0, out = 0, count = 0;

volatile bool choosing[NUMTHREAD];
volatile int num[NUMTHREAD];

void InitNumChoosing() {
	for (int i = 0; i < NUMTHREAD; i++) {
		num[i] = 0;
		choosing[i] = false;
	}
}

int MaxNum() {
	int max = num[0];
	for (int i = 0; i < NUMTHREAD; i++) {
		if (max < num[1])
			max = num[1];
	}
}

int cmpnum(int numj, int j, int numi, int i) {
	if (numj < numi) {
		return 1;
	}
	if ((numj == numi) && (j < i)) {
		return 1;
	}
	return 0;
}

void Bakery_EnterCriticalSection(int id) {
	int i = id;
	choosing[i] = true;
	num[i] = MaxNum() + 1;
	choosing[i] = false;
	for (int j = 0; j < NUMTHREAD; j++) {
		MemoryBarrier();
		while (choosing[j]);
		MemoryBarrier();
		while ((num[j] != 0) && cmpnum(num[j], j, num[i], i));
	}
}

void Bakery_ExitCriticalSection(int id) {
	int i = id;
	num[i] = 0;
}

typedef struct _param_t {
	int *pArray;
	int num;
}param_t, *pparam_t;

DWORD WINAPI Producer(LPVOID lpParam) {
	int pid = 0;
	pparam_t p = (pparam_t)lpParam;
	int produced;
	int i = 0;
	while (i<p->num)
	{
		while (count == BUFFERSIZE);
		Bakery_EnterCriticalSection(pid);
		buffer[in] = produced;
		count = count + 1;
		in = (in + 1) % BUFFERSIZE;
		Bakery_ExitCriticalSection(pid);
	}
	return 0;
}
DWORD WINAPI Consumer(LPVOID lpParam) {
	int pid = 1;
	pparam_t p = (pparam_t)lpParam;
	int consumed;
	int i = 0;
	while (i<p->num)
	{
		while (count == 0);
		Bakery_EnterCriticalSection(pid);
		consumed = buffer[out];
		count = count - 1;
		out = (out + 1) % BUFFERSIZE;
		p->pArray[i++] = consumed;
		Bakery_ExitCriticalSection(pid);
	}
	return 0;
}

void KhoiTaoMang(int a[], int n) {
	for (int i = 0; i < n; i++) {
		a[i] = i + 1;
	}
}
void XuatMang(int a[], int n) {
	for (int i = 0; i < n; i++) {
		printf("%d", a[i]);
	}
	printf("\n");
}

void main() {
	int a[N], b[N];
	int n = N;
	HANDLE h[NUMTHREAD];
	DWORD dwID[NUMTHREAD];


	InitNumChoosing();


	printf("\n Bakery:\n");
	KhoiTaoMang(a, n);
	pparam_t p = (pparam_t)malloc(sizeof(param_t));
	p->num = n;
	p->pArray = a;
	h[0] = CreateThread(NULL, 0, Producer, (LPVOID)p, 0, &dwID[0]);

	p = (pparam_t)malloc(sizeof(param_t));
	p->num = n;
	p->pArray = b;
	h[1] = CreateThread(NULL, 0, Consumer, (LPVOID)p, 0, &dwID[1]);
	WaitForMultipleObjects(NUMTHREAD, h, TRUE, INFINITE);
	printf("\n Mang A:\n");
	XuatMang(a, n);
	printf("\n Mang B:\n");
	XuatMang(b, n);

}
