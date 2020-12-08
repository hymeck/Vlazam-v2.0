#include "SongHash.h"

SongHash::SongHash() {
	this->buffer = nullptr;
	this->size = 0;
}

SongHash::SongHash(long* bufptr, size_t bufsize) {
	this->buffer = bufptr;
	this->size = bufsize;
}