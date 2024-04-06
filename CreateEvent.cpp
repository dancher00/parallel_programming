#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <Windows.h>
#include <cstdio>

struct header {
	int32_t chunkId;
	int32_t chunkSize;
	int32_t format;
	int32_t subchunk1Id;
	int32_t subchunk1Size;
	int16_t audioFormat;
	int16_t numChannels;
	int32_t sampleRate;
	int32_t byteRate;
	int16_t blockAlign;
	int16_t bitsPerSample;
	int32_t subchunk2Id;
	int32_t subchunk2Size;
};

struct task {
	int from, to;
	int num;
};

const int X = 524 + 3;
const int N = 4 + (X % 5);
int _more = 0, _less = 0;

int16_t* arr;
HANDLE th[N], ev[N];
DWORD tid[N];
task t[N];

DWORD __stdcall f(void* arg) {
	task* t = (task*)arg;
	int more = 0; int less = 0;
	for (int i = t->from; i >= t->to; i -= N)
		sqrt(arr[i] * arr[i]) > 16000 ? more++ : less++;
	WaitForSingleObject(ev[N], INFINITY);
	ResetEvent(ev[N]);
	_more += more;
	_less += less;
	SetEvent(ev[N]);
	SetEvent(ev[t->num]);
	return 0;
};

using namespace std;

int main() {
	//Events
	ev[N] = CreateEvent(NULL, TRUE, TRUE, (LPCWSTR)"test");
	//Danil Belov is onto the 3rd in the list of 524 group
	cout << "X = 524 + 3 = " << X << '\n';
	cout << "X % 4 = " << X % 4 << " [D] " << '\n';
	cout << "N = 4 + (X % 5) = " << N << '\n';
	//-----------------------------------------------------------
	string path;
	cout << "Input the pathway to wav-file: " << '\n';
	cin >> path;
	ifstream inwav(path, ios::binary);

	char subchunk2Id[N] = {};
	while (string(subchunk2Id) != "data")
		inwav.read((char*)&subchunk2Id, 4);

	int32_t subchunk2Size;
	inwav.read((char*)&subchunk2Size, 4);
	int size = subchunk2Size / 2;
	arr = new int16_t[size];
	inwav.read(reinterpret_cast<char*>(arr), size * sizeof(int16_t));

	for (int i = 0; i < N; i++) {
		ev[i] = CreateEvent(NULL, TRUE, FALSE, (LPCWSTR)"test");
		th[i] = CreateThread(NULL, 0, f, (void*)&t[i], 0, &tid[i]);
		t[i].from = size - i;
		t[i].to = 0;
		t[i].num = i;
	}

	WaitForMultipleObjects(N, ev, true, INFINITE);
	WaitForMultipleObjects(N, th, true, INFINITE);
	for (int i = 0; i < N; i++) {
		CloseHandle(th[i]);
		CloseHandle(ev[i]);
	}

	delete[] arr;
	cout << "Numbers that modulo more than 16000: " << _more << '\n';
	cout << "Numbers that modulo less than 16000: " << _less << '\n';
	system("pause");
	return 0;
}
