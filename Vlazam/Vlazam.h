#ifndef VLAZAM_H
#define VLAZAM_H

#include "SongHash.h"

#define CHUNK_SIZE 2205
#define FUZZ_FACTOR 2
#define FREQ 44100
#define CHANS 1
#define DEFAULT_DEVICE -1
#define BUFSTEP 100000  // memory allocation input
#define CHUNK_SIZE 2205
#define RECORDED_BUF_FILENAME "RecordedSample.wav"
#define DB_DIRECTORY_PATH "..\\DB\\"

int getIndex(const int freq);
size_t fileSize(const char* fileName);
int saveToFile(const char* fileName, const char* buf, int bufLen);
int loadSongs(SongHash* &songsHashes, const char* dirPath, int& numOfSongs);
long hash(long p1, long p2, long p3, long p4);
char* intToCharptr(long long a);
int addToDatabase(const char* fileName);
int changeFileExtension(char*& fileName, const char* newExtension);
char* getFileName(const char* fullPath);
BOOL initRecordDevice();
int playFileWAV(const char* fileName);
int saveRecording(const char* fileName);
int startRecording();
int stopRecording();
int recognizeSample(char**& resultSongs, int& countSongs);

#endif