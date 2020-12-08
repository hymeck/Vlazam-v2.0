#ifndef SONGHASH_H
#define SONGHASH_H

class SongHash {
public:
	long* buffer;
	char* songName;
	size_t size;
	SongHash();
	SongHash(long* bufptr, char* songname, size_t bufsize);

};

#endif