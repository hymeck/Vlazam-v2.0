#ifndef SONGHASH_H
#define SONGHASH_H

class SongHash {
public:
	long* buffer;
	size_t size;
	SongHash();
	SongHash(long* bufptr, size_t bufsize);

};

#endif