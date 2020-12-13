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
char* recBuf = nullptr;  // buffer for recording
DWORD recLen;         // size of this buffer
HRECORD rchan = 0;
HSTREAM chan = 0;
BOOL CALLBACK recordingCallback(HRECORD handle, const void* buffer, DWORD length, void* user);

int loadSongs(SongHash* &songsHashes, const char* dirPath, int &numOfSongs) {
	WIN32_FIND_DATA data;
	HANDLE hFind;

	char* correctDirPath = nullptr;
	if (dirPath[strlen(dirPath) - 1] != '*') {
		correctDirPath = (char*)malloc(sizeof(char) * (strlen(dirPath) + 2));
		strcpy_s(correctDirPath, sizeof(char) * (strlen(dirPath) + 2), dirPath);
		strcat_s(correctDirPath, sizeof(char) * (strlen(dirPath) + 2), "*");
	}
	else {
		correctDirPath = (char*)malloc(sizeof(char) * (strlen(dirPath) + 1));
		strcpy_s(correctDirPath, sizeof(char) * (strlen(dirPath) + 1), dirPath);
	}

	std::vector<char*> vect;
	if ((hFind = FindFirstFile(correctDirPath, &data)) == INVALID_HANDLE_VALUE) {
		int err = GetLastError();
		return -1;
	}

	do {
		if (data.cFileName[0] != '.') {
			int buflen = strlen(data.cFileName) + strlen(DB_DIRECTORY_PATH) + 1;
			char* buf = (char*)malloc(sizeof(char) * buflen);
			strcpy_s(buf, sizeof(char) * buflen, DB_DIRECTORY_PATH);
			strcat_s(buf, sizeof(char) * buflen, data.cFileName);
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
		songsHashes[i].songName = (char*)malloc(strlen(vect[i]) + 1);
		if (!((songsHashes[i].buffer) || (!songsHashes[i].songName))) {
			return -1;
		}
		strcpy_s(songsHashes[i].songName, strlen(vect[i]) + 1, vect[i]);


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

char* getFileNameWithoutExtension(const char* fullPath) {
	int len = strlen(fullPath);
	int i = len - 1;
	int j = len - 1;
	while ((i >= 0) && (fullPath[i] != '\\') && (fullPath[i] != '/')) {
		i--;
	}
	while ((j >= 0) && (j >= i) && (fullPath[j] != '.')) {
		j--;
	}
	char* buf = (char*)malloc(strlen(fullPath) * sizeof(char));
	if (!buf) {
		return nullptr;
	}
	strcpy_s(buf, len + 1, fullPath);
	buf[j] = '\0';

	char* result = (char*)malloc(j - i + 1);
	if (!result) {
		return nullptr;
	}
	strcpy_s(result, len - i + 1, buf + i + 1);
	return result;
}

int addToDatabase(const char* fileName) {

	SongHash songHash;
	getSampleHash(fileName, songHash);

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
	int err;
	if (err = fopen_s(&fo, outputFileName, "wb")) {
		return -1;
	}
	fwrite(songHash.buffer, songHash.size, 1, fo);
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
		recBuf = nullptr;
		// close output device before recording in case of half-duplex device
		// BASS_Free();
	}
	// allocate initial buffer and make space for WAVE header
	recBuf = (char*)malloc(BUFSTEP);
	if (!recBuf) {
		return -1;
	}

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
		char *temp = (char*)realloc(recBuf, ((recLen + length) / BUFSTEP + 1) * BUFSTEP);
		if (!temp) {
			rchan = 0;
			free(recBuf);
			return FALSE; // stop recording
		}
		recBuf = temp;
	}
	// buffer the data
	memcpy(recBuf + recLen, buffer, length);
	recLen += length;
	return TRUE; // continue recording
}

// returns 0 if ok
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

int playFileWAV(const char* fileName) {
	chan = BASS_StreamCreateFile(FALSE, fileName, 0, 0, 0);
	
	BASS_ChannelPlay(chan, TRUE);
	return 0;
}

void waitTillPlaying() {
	while ((BASS_ChannelIsActive(chan) == BASS_ACTIVE_PLAYING)) {
		Sleep(100);
	}
}

int BassDllInit() {
	if (!BASS_Init(DEFAULT_DEVICE, FREQ, BASS_DEVICE_3D, 0, NULL)) {
		return BASS_ErrorGetCode();
	}	
	if (!BASS_RecordInit(DEFAULT_DEVICE)) {
		return BASS_ErrorGetCode();
	}
	return 0;
}

int BassDllCleanup() {
	if (BASS_IsStarted()) {
		if (!BASS_Free()) {
			return BASS_ErrorGetCode();
		}
		if (!BASS_RecordFree()) {
			return BASS_ErrorGetCode();
		}
	}
	return 0;
}

int saveRecording(const char* fileName) {
	int res = saveToFile(fileName, recBuf, recLen);
	free(recBuf);
	recBuf = nullptr;
	return res;
}

int recognizeSample(char**& resultSongs, int& countSongs) {

	// todo
	// 1. getSongsFromDBCount
	// 2. loadSong

	SongHash songHash;
	getSampleHash(RECORDED_BUF_FILENAME, songHash);

	SongHash* songs = nullptr;
	int songsCount = 0;
	int res = loadSongs(songs, DB_DIRECTORY_PATH, songsCount);
	if (res == -1) {
		return -1;
	}
	int* collisions = (int*)malloc(sizeof(int) * songsCount);
	if (!collisions) {
		return -1;
	}
	memset(collisions, 0, sizeof(int) * songsCount);
	for (int i = 0; i < songsCount; i++) {
		for (int z = 0; z < songHash.size; z++) {
			for (int w = 0; w < songHash.size; w++) {
				if (songs[i].buffer[z] == songHash.buffer[w]) {
					collisions[i]++;
				}
			}
		}
	}
	
	int maxIndex = 0;
	int maxValue = collisions[maxIndex];
	for (int i = 1; i < songsCount; i++) {
		if (collisions[i] > collisions[maxIndex]) {
			maxIndex = i;
			maxValue = collisions[maxIndex];
		}
	}
	// todo: return more than 1 song, if delta = 50%
	int suitableCount = 1;
	char** suitableSongNames = (char**)malloc(suitableCount);
	if (!suitableSongNames) {
		return -1;
	}
								// sizeof(char*) * ??
	suitableSongNames[0] = getFileNameWithoutExtension(songs[maxIndex].songName);
	resultSongs = suitableSongNames;
	countSongs = suitableCount;
	
	free(songs);

	return 0;
}

int getSampleHash(const char* sampleFileName, SongHash &songHash) {
	int sizeWithoutHeaders = fileSize(sampleFileName) - 44;
	//char* audio = (char*)malloc(sizeof(char) * sizeWithoutHeaders);
	char* audio = new char[sizeWithoutHeaders];
	if (!audio) {
		// out of memory
		return -1;
	}

	// read all audio meat
	FILE* fp;
	if (fopen_s(&fp, sampleFileName, "rb")) {
		// Cannot open a file
		return -1;
	}
	// miss headers
	fseek(fp, 44, 0);
	int length = fread(audio, 1, sizeWithoutHeaders, fp);
	fclose(fp);

	const int chunkSize = CHUNK_SIZE;
	int sampledChunkCount = length / chunkSize;

	// allocate memory for result(OUTPUT) complex array
	//kiss_fft_cpx** result = (kiss_fft_cpx**)malloc(sizeof(kiss_fft_cpx*) * sampledChunkCount);
	kiss_fft_cpx** result = new kiss_fft_cpx * [sampledChunkCount];
	for (int j = 0; j < sampledChunkCount; j++) {
		//result[j] = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * chunkSize);
		result[j] = new kiss_fft_cpx[chunkSize];
	}
	for (int j = 0; j < sampledChunkCount; j++) {
		for (int i = 0; i < chunkSize; i++) {
			result[j][i].r = 0;
			result[j][i].i = 0;
		}
	}

	// allocate memory for result(INPUT) complex array
	// kiss_fft_cpx* complexArray = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * chunkSize);
	kiss_fft_cpx* complexArray = new kiss_fft_cpx[chunkSize];
	if (!complexArray) {
		return -1;
	}
	// and initialize it
	for (int j = 0; j < sampledChunkCount; j++) {
		// kiss_fft_cpx* complexArray = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * chunkSize);
		kiss_fft_cpx* complexArray = new kiss_fft_cpx[chunkSize];
		if (!complexArray) {
			return -1;
		}
		// and initialize it
		for (int i = 0; i < chunkSize; i++) {
			kiss_fft_cpx bufCpx;
			bufCpx.r = audio[j * chunkSize + i];
			bufCpx.i = 0;
			complexArray[i] = bufCpx;
		}

		// performing fast fourie transform
		kfc_fft(chunkSize, complexArray, result[j]);
	}
	delete[] complexArray;
	delete[] audio;
	// ends with Fourie Transform

	// now create sing signature
	// array for recording high values of amplittude
	//double** highscores = (double**)malloc(sizeof(double*) * sampledChunkCount);
	double** highscores = new double* [sampledChunkCount];
	if (!highscores) {
		return -1;
	}
	for (int j = 0; j < sampledChunkCount; j++) {
		//highscores[j] = (double*)malloc(sizeof(double) * 5);
		highscores[j] = new double[RANGE_COUNT];
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
	//int** points = (int**)malloc(sizeof(int*) * sampledChunkCount);
	int** points = new int* [sampledChunkCount];
	if (!points) {
		return -1;
	}
	for (int j = 0; j < sampledChunkCount; j++) {
		//points[j] = (int*)malloc(sizeof(int) * RANGE_COUNT);
		points[j] = new int[RANGE_COUNT];
		if (!points[j]) {
			return -1;
		}
	}
	for (int j = 0; j < sampledChunkCount; j++) {
		for (int i = 0; i < RANGE_COUNT; i++) {
			points[j][i] = 0;
		}
	}
	// now create SongHash object and initialize it
	songHash.size = sampledChunkCount;
	//songHash.buffer = (long*)malloc(sizeof(long) * songHash.size);
	songHash.buffer = new long[songHash.size];
	//songHash.songName = (char*)malloc(strlen(RECORDED_BUF_FILENAME) + 1);
	songHash.songName = new char[strlen(RECORDED_BUF_FILENAME) + 1];
	if (!((songHash.buffer) || (!songHash.songName))) {
		return -1;
	}
	strcpy_s(songHash.songName, strlen(RECORDED_BUF_FILENAME) + 1, RECORDED_BUF_FILENAME);
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

	// free memory
	for (int j = 0; j < sampledChunkCount; j++) {
		delete[] result[j];
	}
	delete[] result;

	for (int j = 0; j < sampledChunkCount; j++) {
		delete[] highscores[j];
	}
	delete[] highscores;

	return 0;
}