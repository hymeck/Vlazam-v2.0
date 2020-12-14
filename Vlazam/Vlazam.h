#ifndef VLAZAM_H
#define VLAZAM_H

#include <Windows.h>
#include <fstream>
#include <vector>
#include "../Dependencies/kiss_fft130/tools/kfc.h"
#include "../Dependencies/bass/bass.h"
#include "SongHash.h"

#define CHUNK_SIZE 2205
// how can i manage this?
#define FUZZ_FACTOR 3
#define FREQ 44100
#define CHANS 1
#define DEFAULT_DEVICE -1
#define BUFSTEP 100000  // memory allocation input
#define CHUNK_SIZE 2205
#define RECORDED_BUF_FILENAME "RecordedSample.wav"
#define DB_DIRECTORY_PATH "..\\DB\\"
// this is max difference between max_count_of_collisions and possible_count_of_collisions
// so we can return few songs
#define DELTA_COMPARE 0.5

int getIndex(const int freq);
size_t fileSize(const char* fileName);
int saveToFile(const char* fileName, const char* buf, int bufLen);
int loadSongs(SongHash* &songsHashes, const char* dirPath, int& numOfSongs);
long hash(long p1, long p2, long p3, long p4);
char* intToCharptr(long long a);
int addToDatabase(const char* fileName);
int changeFileExtension(char*& fileName, const char* newExtension);
char* getFileName(const char* fullPath);
int playFileWAV(const char* fileName);
int saveRecording(const char* fileName);
int startRecording();
int stopRecording();
int recognizeSample(std::vector<char*> &resultSongs);
void waitTillPlaying();
int BassDllInit();
int BassDllCleanup();
int getSampleHash(const char* sampleFileName, SongHash& songHash);

#endif