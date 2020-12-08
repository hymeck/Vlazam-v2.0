#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <Windows.h>
#include <fstream>
#include <vector>
#include <string>
#include "Vlazam.h"
#include "../Dependencies/kiss_fft130/tools/kfc.h"
#include "../Dependencies/bass/bass.h"

// ranges of frequencies
const int RANGE_COUNT = 5;
const int RANGE[RANGE_COUNT] = { 40, 80, 120, 180, 300 };
// stuff for recording
char* recBuf = NULL;  // buffer for recording
DWORD recLen;         // size of this buffer
HRECORD rchan = 0;
HSTREAM chan = 0;
BOOL CALLBACK recordingCallback(HRECORD handle, const void* buffer, DWORD length, void* user);

int loadSongs(SongHash* &songsHashes, const char* dirPath, int &numOfSongs) {
	WIN32_FIND_DATA data;
	HANDLE hFind;
	std::vector<char*> vect;
	if ((hFind = FindFirstFile(dirPath, &data)) == INVALID_HANDLE_VALUE) {
		return -1;
	}

	do {
		if (data.cFileName[0] != '.') {
			int buflen = strlen(data.cFileName) + strlen(DB_DIRECTORY_PATH) + 1;
			char* buf = (char*)malloc(buflen);
			strcpy_s(buf, buflen, DB_DIRECTORY_PATH);
			strcat_s(buf, buflen, data.cFileName);
			vect.push_back(buf);
		}
	} while (FindNextFile(hFind, &data) != 0);
	FindClose(hFind);

	numOfSongs = vect.size();
	songsHashes = (SongHash*)malloc(sizeof(SongHash) * numOfSongs);
	if (!songsHashes) {
		return -1;
	}
	//songsHashes = new SongHash[numOfSongs];
	memset(songsHashes, 0, sizeof(SongHash) * numOfSongs);
	
	for (int i = 0; i < numOfSongs; i++) {
		int k = sizeof(long);
		songsHashes[i].size = fileSize(vect[i]) / k;
		songsHashes[i].buffer = (long*)malloc(sizeof(long) * songsHashes[i].size);
		if (!songsHashes[i].buffer) {
			return -1;
		}
		memset(songsHashes[i].buffer, 0, songsHashes[i].size * sizeof(long));
		//std::ifstream file(vect[i].c_str());
		//if (!file.is_open()) {
		//	continue;
		//	// return -1??
		//}
		//file.read(songsHashes[i].buffer, )
		FILE* fp;
		if (fopen_s(&fp, vect[i], "rb")) {
			return -1;
		}
		int res = fread(songsHashes[i].buffer, sizeof(long), songsHashes[i].size, fp);
		// fread(songsHashes[i].buffer, songsHashes[i].size, 1, fp);
		fclose(fp);
	}
	return 0;
}
// returns 0 if success
int changeFileExtension(char *&fileName, const char* newExtension) {
	int len = strlen(fileName);
	int i = len;
	while ((i >= 0) && (fileName[i] != '.')) {
		i--;
	}
	if (i < 0) {
		return -1;
	}
	int newLen = i + strlen(newExtension) + 1;
	char* buf = (char*)malloc(newLen);
	if (!buf) {
		return -1;
	}
	strcpy_s(buf, newLen, fileName);
	strcpy_s(buf + i, newLen, newExtension);
	fileName = buf;

	return 0;
}

// returns -1 if error; otherwise returns 0
int saveToFile(const char* fileName, const char* buf, int bufLen) {
	std::ofstream file(fileName);
	if (!file) {
		return -1;
	}
	file.write(buf, bufLen);

	return 0;
}

long hash(long p1, long p2, long p3, long p4) {
	return  (p1 - (p1 % FUZZ_FACTOR)) +
		(p2 - (p2 % FUZZ_FACTOR)) * 100 +
		(p3 - (p3 % FUZZ_FACTOR)) * 10000 +
		(p4 - (p4 % FUZZ_FACTOR)) * 10000000;
}
 
size_t fileSize(const char* fileName) {
	//											at the end
	std::ifstream file(fileName, std::ifstream::ate | std::ifstream::binary);
	int res = static_cast<int>(file.tellg());
	file.close();
	return res;
}

int getIndex(const int freq) {
	int i = 0;
	const int* range = RANGE;

	while (range[i] < freq) {
		i++;
	}
	return i;
}

char* intToCharptr(long long a) {
	// max long long 9223372036854775807
	const int maxSize = 20;

	char* result = (char*)malloc(maxSize);
	memset(result, 0, maxSize);

	a = abs(a);
	int buf = a % 10;

	do {
		buf = a % 10;
		a /= 10;
		switch (buf) {
		case 0:
			strcat_s(result, maxSize, "0");
			break;
		case 1:
			strcat_s(result, maxSize, "1");
			break;
		case 2:
			strcat_s(result, maxSize, "2");
			break;
		case 3:
			strcat_s(result, maxSize, "3");
			break;
		case 4:
			strcat_s(result, maxSize, "4");
			break;
		case 5:
			strcat_s(result, maxSize, "5");
			break;
		case 6:
			strcat_s(result, maxSize, "6");
			break;
		case 7:
			strcat_s(result, maxSize, "7");
			break;
		case 8:
			strcat_s(result, maxSize, "8");
			break;
		case 9:
			strcat_s(result, maxSize, "9");
			break;
		}
	}
	while (a != 0);

	int j = strlen(result) - 1;
	char bufc;
	for (int i = 0; i < (j / 2) + 1; i++) {
		bufc = result[i];
		result[i] = result[j - i];
		result[j - i] = bufc;
	}

	return result;
}

char* getFileName(const char* fullPath) {
	int len = strlen(fullPath);
	int i = len - 1;
	while ((i >= 0) && (fullPath[i] != '\\') && (fullPath[i] != '/')) {
		i--;
	}
	char* result = (char*)malloc(len - i);
	if (!result) {
		return nullptr;
	}
	strcpy_s(result, len - i, fullPath + i + 1);
	return result;
}

int addToDatabase(const char* fileName) {

	int sizeWithoutHeaders = fileSize(fileName) - 44;
	char* audio = (char*)malloc(sizeof(char) * sizeWithoutHeaders);
	if (!audio) {
		// out of memory
		return -1;
	}

	// read all audio meat
	FILE* fp;
	if (fopen_s(&fp, fileName, "rb")) {
		// Cannot open a file
		return -1;
	}
	fseek(fp, 44, 0);
	int length = fread(audio, 1, sizeWithoutHeaders, fp);
	fclose(fp);

	const int chunkSize = CHUNK_SIZE;
	int sampledChunkCount = length / chunkSize;

	// allocate memory for result complex array
	kiss_fft_cpx** result = (kiss_fft_cpx**)malloc(sizeof(kiss_fft_cpx*) * sampledChunkCount);
	for (int j = 0; j < sampledChunkCount; j++) {
		result[j] = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * chunkSize);
	}
	for (int j = 0; j < sampledChunkCount; j++) {
		for (int i = 0; i < chunkSize; i++) {
			result[j][i].r = 0;
			result[j][i].i = 0;
		}
	}

	for (int j = 0; j < sampledChunkCount; j++) {
		kiss_fft_cpx* complexArray = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * chunkSize);
		if (!complexArray) {
			return -1;
		}

		for (int i = 0; i < chunkSize; i++) {
			kiss_fft_cpx bufCpx;
			bufCpx.r = audio[j * chunkSize + i];
			bufCpx.i = 0;
			complexArray[i] = bufCpx;
		}
		
		kfc_fft(chunkSize, complexArray, result[j]);
	}

	// create digital signature
	// array for recording high values of amplittude
	double** highscores = (double**)malloc(sizeof(double*) * sampledChunkCount);
	if (!highscores) {
		return -1;
	}
	for (int j = 0; j < sampledChunkCount; j++) {
		highscores[j] = (double*)malloc(sizeof(double) * 5);
		if (!highscores[j]) {
			return -1;
		}
	}
	for (int j = 0; j < sampledChunkCount; j++) {
		for (int i = 0; i < RANGE_COUNT; i++) {
			highscores[j][i] = 0;
		}
	}
	// array for recording frequences of this values
	int** points = (int**)malloc(sizeof(int*) * sampledChunkCount);
	if (!points) {
		return -1;
	}
	for (int j = 0; j < sampledChunkCount; j++) {
		points[j] = (int*)malloc(sizeof(int) * RANGE_COUNT);
		if (!points[j]) {
			return -1;
		}
	}
	for (int j = 0; j < sampledChunkCount; j++) {
		for (int i = 0; i < RANGE_COUNT; i++) {
			points[j][i] = 0;
		}
	}

	SongHash songHash;
	songHash.size = sampledChunkCount;
	songHash.buffer = (long*)malloc(sizeof(long) * songHash.size);
	// values of recording sample
	for (int t = 0; t < sampledChunkCount; t++) {
		for (int freq = 40; freq < 300; freq++) {
			double mag = log(abs(result[t][freq].r) + 1);
			int index = getIndex(freq);
			if (mag > highscores[t][index]) {
				highscores[t][index] = mag;
				points[t][index] = freq;
			}
		}
		songHash.buffer[t] = hash(points[t][0], points[t][1], points[t][2], points[t][3]);
	}

	char* buf = getFileName(fileName);
	int outputFileNameLen = strlen(DB_DIRECTORY_PATH) + strlen(buf) + 1;
	char* outputFileName = (char*)malloc(outputFileNameLen);
	if (!outputFileName) {
		return -1;
	}
	strcpy_s(outputFileName, outputFileNameLen, DB_DIRECTORY_PATH);
	strcat_s(outputFileName, outputFileNameLen, buf);
	changeFileExtension(outputFileName, ".bin");

	FILE* fo;
	if (fopen_s(&fo, outputFileName, "wb")) {
		return -1;
	}
	fwrite(songHash.buffer, sampledChunkCount, 1, fo);
	fclose(fo);
	return 0;
}
// returns -1 if error
int startRecording() {
	WAVEFORMATEX* wf;
	if (recBuf) {
		// free old recording
		BASS_StreamFree(chan);
		chan = 0;
		free(recBuf);
		recBuf = NULL;
		// close output device before recording incase of half-duplex device
		BASS_Free();
	}
	// allocate initial buffer and make space for WAVE header
	recBuf = (char*)malloc(BUFSTEP);
	memset(recBuf, 0, BUFSTEP);
	recLen = 44;

	// fill the WAVE header
	memcpy(recBuf, "RIFF\0\0\0\0WAVEfmt \20\0\0\0", 20);
	memcpy(recBuf + 36, "data\0\0\0\0", 8);
	wf = (WAVEFORMATEX*)(recBuf + 20);
	wf->wFormatTag = 1;
	wf->nChannels = CHANS;
	wf->wBitsPerSample = 8;
	wf->nSamplesPerSec = FREQ;
	wf->nBlockAlign = wf->nChannels * wf->wBitsPerSample / 8;
	wf->nAvgBytesPerSec = wf->nSamplesPerSec * wf->nBlockAlign;
	// start recording
	rchan = BASS_RecordStart(FREQ, CHANS, BASS_SAMPLE_8BITS, recordingCallback, 0);
	if (!rchan) {
		free(recBuf);
		recBuf = NULL;
		return -1;
	}
	return 0;
}

// buffer the recorded data
BOOL CALLBACK recordingCallback(HRECORD handle, const void* buffer, DWORD length, void* user)
{
	// increase buffer size if needed
	if ((recLen % BUFSTEP) + length >= BUFSTEP) {
		recBuf = (char*)realloc(recBuf, ((recLen + length) / BUFSTEP + 1) * BUFSTEP);
		if (!recBuf) {
			rchan = 0;
			return FALSE; // stop recording
		}
	}
	// buffer the data
	memcpy(recBuf + recLen, buffer, length);
	recLen += length;
	return TRUE; // continue recording
}

// returns -1 if error
int stopRecording() {
	int result = 0;
	if (!BASS_ChannelStop(rchan)) {
		result = -1;
	}
	rchan = 0;

	*(DWORD*)(recBuf + 4) = recLen - 8;
	*(DWORD*)(recBuf + 40) = recLen - 44;

	return result;
}

void BASS_CleanUp() {
	BASS_RecordFree();
	BASS_Free();
}

BOOL initDevice() {
	BASS_RecordFree();
	return BASS_RecordInit(DEFAULT_DEVICE);
}

int playFileWAV(const char* fileName) {
	if (!BASS_Init(DEFAULT_DEVICE, FREQ, BASS_DEVICE_3D, 0, NULL)) {
		return false;
	}
	chan = BASS_StreamCreateFile(FALSE, fileName, 0, 0, 0);
	
	BASS_ChannelPlay(chan, TRUE);
}

int saveRecording(const char* fileName) {
	return saveToFile(fileName, recBuf, recLen);
}