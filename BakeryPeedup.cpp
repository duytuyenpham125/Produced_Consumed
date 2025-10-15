#include"iostream"
#include "stdio.h"
#include "Windows.h"

using namespace std;
typedef struct _data_ {
	char SP;
	double giatri;
	int SoCoreN;
	int SoLanTang;
	double S;
	double P;
	double N;
	double Speedup;
	int SoCoreTang;;
	double newSpeedup;

}data_t, *pdata_t;

bool DocTapTin(char *filename, data_t p[], int n)
{
	FILE*f = fopen(filename, "rt");
	if (!f)
		printf("khong the mo tap tin %s\n", filename), exit(0);
	char header[_MAX_PATH];
	fgets(header, _MAX_PATH, f);
	int i = 0;
	printf(header);
	while ((!feof(f)) && (i < n))
	{
		fscanf(f, "%c%lf%d%d", &(p[i].SP), &(p[i].giatri), &(p[i].SoCoreN), &(p[i].SoCoreTang));
		fgets(header, _MAX_PATH, f);
		printf("%2d %c %0.2lf %3d %3d\n", i, p[i].SP, p[i].giatri, p[i].SoCoreN, p[i].SoCoreTang);
		i++;
	}
	fclose(f);
	return true;
}
char * SVRIPAddress[] = {	"172.17.14.54",
							"172.17.14.54",
							"172.17.14.54",
							"172.17.14.54", };
int NumSVR = sizeof(SVRIPAddress) / sizeof(SVRIPAddress[0]);
u_short port = 2;
typedef struct _param_t {
	pdata_t pdata;
	int Init;
	int Step;
	int Num;
	char * SVRIP;
	u_short port;
}param_t, *pparam_t;

typedef struct _request_t {
	char SP;
	double giatri;
	int SoCoreN;
	int Solantang;
}request_t,*prequest_t;

typedef struct _reply_t {
	double S;
	double P;
	double SpeedUp;
	int SoCoreTang;
	double NewSpeedup;
}reply_t,*preply_t;

int InitializeWinsock() {
	WSADATA wsa;
	printf("\nInitializing Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed");
		exit(-1);
		return 0;
	}
	printf("Initialized.");
	return 1;
}

SOCKET SocketConnect(char* strServerIPAddress, u_short port) {
	SOCKET s;
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket : %d", WSAGetLastError());
		exit(-1);
	}

	printf("Socket created.\n");
	struct sockaddr_in server;
	server.sin_addr.s_addr = inet_addr(strServerIPAddress);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	// Connect to remote server
	if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
		puts("connect error");
		return 1;
	}

	puts("Connected");
	return s;

}
void TinhSpeedupDataTCP(pdata_t p, char* SVRIPAddress, u_short port) {
	request_t req;
	req.SP = p->SP;
	req.giatri = p->giatri;
	req.SoCoreN = p->SoCoreN;
	req.Solantang = p->SoLanTang;

	SOCKET svrsocket = SocketConnect(SVRIPAddress, port);
	send(svrsocket, (char*)&req, sizeof(req), 0);

	reply_t rep;
	recv(svrsocket, (char*)&rep, sizeof(rep), 0);

	p->S = rep.S;
	p->P = rep.P;
	p->Speedup = rep.SpeedUp;
	p->SoCoreTang = rep.SoCoreTang;
	p->newSpeedup = rep.NewSpeedup;

	closesocket(svrsocket);
}

DWORD WINAPI TinhSpeedUpThreadProc(LPVOID lpParam)
{
	printf("ThreadID=%ld", GetCurrentThread());
	pparam_t p = (pparam_t)lpParam;
	for (int i = p->Init; i < p->Num; i += p->Step) {
		TinhSpeedupDataTCP(p->pdata + i, p->SVRIP, p->port);
	}
	return 0;
}

void TinhSpeedupMangDataMT(pdata_t p, int n, int NumThread)
{
	HANDLE*h = (HANDLE*)malloc(NumThread*sizeof(HANDLE));
	DWORD*dwID = (DWORD*)malloc(NumThread*sizeof(DWORD));
	pparam_t param = (pparam_t)malloc(NumThread*sizeof(pparam_t));

	for (int i = 0; i < NumThread; i++) {
		param[i].pdata = p;
		param[i].Init = i;
		param[i].Step = NumThread;
		param[i].Num = n;
		param[i].SVRIP = SVRIPAddress[i%NumSVR];
		param[i].port = port;
		h[i] = CreateThread(NULL, 0, TinhSpeedUpThreadProc, (LPVOID)&param[i], 0, &dwID[i]);
	}
	WaitForMultipleObjects(NumThread, h, TRUE, INFINITE);
}



int TangSoCore(double CurrentSpeedup, int NumCore, int ntime)
{
	if (ntime <= 0)
		return 0;
	double S = TinhSFromSpeedUp(CurrentSpeedup, NumCore);
	double LimitSpeedup = CurrentSpeedup * ntime;
	int i = NumCore;
	double Speedup = CurrentSpeedup;
	while (Speedup < LimitSpeedup)
	{
		i++;
		Speedup = TinhSpeedupSN(S, i);
	}

	return i - NumCore;
}
double TinhSFromSpeedUp(double Speedup, int NumCore)
{
	double S = 0.0;
	int N = NumCore;
	S = (N - Speedup) / (Speedup*(N - 1.0));
	return S;
}
double TinhS(double P)
{
	double S = 1.0 - P;
	return S;
}
double TinhP(double S)
{
	double P = 1.0 - S;
	return P;
}
double TinhSpeedupSN(double S, int N)
{
	double Speedup = 0.0;
	if ((S >= 0.0) && (S <= 1.0) && (N > 0))
	{
		Speedup = 1.0 / (S + (1.0 - S) / N);
	}
}
double TinhSpeedupPN(double P, int N)
{
	double Speedup = 0.0;
	double S = TinhS(P);
	Speedup = TinhSpeedupSN(S, N);
	return Speedup;
}

void TestSpeedupSN(double S, int N)
{
	double P = 0.25;
	int N = 2;
	double Speedup = TinhSpeedupSN(S, N);
	printf("S=%0.2f N=%d Speedup=%.2f\n", S, N, Speedup);

}
void TestSpeedupPN()
{
	double P = 0.75;
	int N = 2;
	double Speedup = TinhSpeedupPN(P, N);
	printf("S=%0.2f P=%0.2f N=%d Speedup=%.2f\n", TinhS(P), P, N, Speedup);
}
void TestTinhSFormSpeedUp()
{
	double Speedup = 1.6;
	int N = 2;
	double S = TinhSFromSpeedUp(Speedup, N);
	printf("S=%0.2f\n", S);
	int ntime = 2;
	int SoCoreTang = TangSoCore(Speedup, N, ntime);
	printf("So core tang them =%d\n", SoCoreTang);
	double NewSpeedup = TinhSpeedupSN(S, N + SoCoreTang);
	printf("OldSpeedup=%0.2f NewSpeedup=%0.2f\n", Speedup, NewSpeedup);
}

void XuatData(pdata_t p, int n)
{
	printf("TT,SP,giatri,SoCoreN,Solantang,S,P,Speedup,SoCoreTang,NewSpeedup\n");
	for (int i = 0; i < n; i++)
	{
		printf("%3d, %c , %0.2lf , %3d, %3d ", i, p[i].SP, p[i].giatri, p[i].SoCoreN, p[i].SoLanTang);
		printf("%0.2lf , %0.2lf , %4.2lf, %3d ,%4.2lf", p[i].S, p[i].P, p[i].Speedup, p[i].SoCoreTang, p[i].newSpeedup);
	}
}
void XuatDataTapTin(char*filename, pdata_t p, int n)
{
	FILE*f = fopen(filename, "wt");
	if (!f)
		printf("Khong the tao file %s\n", filename), exit(0);
	printf("TT,SP,giatri,SoCoreN,Solantang,S,P,Speedup,SoCoreTang,NewSpeedup\n");
	for (int i = 0; i < n; i++)
	{
		printf("%3d, %c , %0.2lf , %3d, %3d ", i, p[i].SP, p[i].giatri, p[i].SoCoreN, p[i].SoLanTang);
		printf("%0.2lf , %0.2lf , %4.2lf, %3d ,%4.2lf", p[i].S, p[i].P, p[i].Speedup, p[i].SoCoreTang, p[i].newSpeedup);
	}

}


bool OpenExcel(char*filename)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	char cmdLine[_MAX_PATH] = "D:/Speedup.csv";
	strcat(cmdLine, filename);
	if (!CreateProcess(NULL,
		cmdLine,
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi))
	{
		printf("CreateProcess failed(%d).\n", GetLastError());
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return true;
}

void TinhSpeedupData(pdata_t p)
{
	if (p->SP == 'S')
		p->S = p->giatri;
	
	else
	{
		p->P = p->giatri;
		p->S = TinhS(p->P);
	}
	p->Speedup = TinhSpeedupSN(p->S, p->SoCoreN);
	p->SoCoreTang = TangSoCore(p->Speedup, p->SoCoreN, p->SoLanTang);
	p->newSpeedup = TinhSpeedupSN(p->S, p->SoCoreN + p->SoCoreTang);
}

void TinhSpeedupMangData(pdata_t p, int n)
{
	for (int i = 0; i < n; i++)
	{
		TinhSpeedupData(p + i);
	}
}
void main(int argc, char*argv[])
{
	InitializeWinsock();
	char * filename = "d:\\Speedup.txt";
	char *filenamecsv = "d:\\Speedup.txt.csv";
	int SoPhanTuMang = 4;
	

	pdata_t pdata = (pdata_t)malloc(SoPhanTuMang*sizeof(data_t));
	DocTapTin(filename, pdata, SoPhanTuMang);
	int NumThread = 4;
	TinhSpeedupMangDataMT(pdata, SoPhanTuMang,NumThread);
	
	XuatDataTapTin(filenamecsv, pdata, SoPhanTuMang);
	OpenExcel(filenamecsv);

}
