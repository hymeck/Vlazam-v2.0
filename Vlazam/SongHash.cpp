#include "SongHash.h"

SongHash::SongHash() {
	this->buffer = nullptr;
	this->songName = nullptr;
	this->size = 0;
}

SongHash::SongHash(long* bufptr, char* songname, size_t bufsize) {
	this->buffer = bufptr;
	this->songName = songname;
	this->size = bufsize;
}