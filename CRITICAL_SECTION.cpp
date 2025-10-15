#include "iostream"
#include "stdio.h"
#include "Windows.h"

#define NUMTHREAD 2
#define BUFFERSIZE 6
#define N 100

volatile int buffer[BUFFERSIZE];
volatile int in = 0, out = 0, count = 0;

CRITICAL_SECTION cs;
typedef struct _param_t {
	int *pArray;
	int num;
}param_t, *pparam_t;

DWORD WINAPI Producer(LPVOID lpParam) {
	pparam_t p = (pparam_t)lpParam;
	int produced;
	int i = 0;
	while (i<p->num)
	{
		while (count == BUFFERSIZE);
		EnterCriticalSection(&cs);
		produced = p->pArray[i++];
		buffer[in] = produced;
		count = count + 1;
		in = (in + 1) % BUFFERSIZE;
		LeaveCriticalSection(&cs);
	}
	return 0;
}
DWORD WINAPI Consumer(LPVOID lpParam) {
	pparam_t p = (pparam_t)lpParam;
	int consumed;
	int i = 0;
	while (i<p->num)
	{
		while (count == 0);
		EnterCriticalSection(&cs);
		consumed = buffer[out];
		count = count - 1;
		out = (out + 1) % BUFFERSIZE;
		p->pArray[i++] = consumed;
		LeaveCriticalSection(&cs);
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

	InitializeCriticalSection(&cs);

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
	DeleteCriticalSection(&cs);
}
