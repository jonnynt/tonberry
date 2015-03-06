#ifndef _CACHEMAP_H
#define _CACHEMAP_H

#include "Main.h""
#include <stdint.h>
#include <unordered_map>

using namespace std;

class HashCache{
public:
	typedef std::pair<uint64_t,string> cacheElem;
	typedef std::list<cacheElem> LRUList;
	typedef std::unordered_map<uint64_t,list<cacheElem>::iterator> CacheUMap;
	typedef LRUList::iterator iterator;
	HashCache(unsigned);
	void insert(uint64_t, string);
	iterator find(uint64_t);
	void update(LRUList::iterator);
	inline cacheElem at(LRUList::iterator it){
		return *it;
	}
	inline iterator begin(){
		return _lrulist.begin();
	}
	inline iterator end(){
		return _lrulist.end();
	}
	inline unsigned size(){
		return _entries;
	}
private:
	unsigned	_entries;
	unsigned	_cachesize;
	LRUList		_lrulist;
	CacheUMap	_cachedata;
};

class TextureCache{
public:
	typedef pair<uint64_t, HANDLE> lru_item_t;                     //I'm guessing this maps the hash to the replacing handle
	typedef list<lru_item_t> lru_list_t;
	typedef lru_list_t::iterator lru_list_iter;
	typedef unordered_map<uint64_t, lru_list_iter> lru_map_t;        // unordered for fast access
	typedef lru_map_t::iterator lru_map_iter;
	typedef unordered_map<HANDLE, lru_map_iter> handlecache_t;      // unordered for fast access
	typedef handlecache_t::iterator handlecache_iter;

	TextureCache(unsigned);
	//inline: faster when compiled
	inline void eraseHandle(HANDLE h){
		handlecache_.erase(h);
	}

	/*update: move mru to the front of the list,
	  returns: true if it found the hash and updated cache as consequence	
	*/
	bool update(uint64_t	hash,		//texture's hash
				HANDLE		replaced	//original game's handle
	);	

	/*inserts new hash->replacement mapping and a replaced handle entry.
	  PRECONDITION:	you have to assure that "hash" is not on chache yet:
	  check "bool update(hash, replaced)".
	*/
	void insert(uint64_t	hash,		//texture's hash
				HANDLE		replaced,	//original game's handle
				HANDLE		replacement	//modded texture's handle
				);

	/*sets the texture replacement for "replaced" if exists.
	  returns:			true if exists a valid handle replacement for the surfacehandle.
						false if it doesn't exists or it's invalid.
	*/
	bool setTexture(HANDLE				replaced,	//original game's handle
					IDirect3DTexture9**	newtexture	//pointer to the replacement texture
		);

private:
	// TOGETHER these make nhcache:
	lru_list_t		nh_list_;        // newhandle list stores pair of hash and newhandle
	lru_map_t		nh_map_;          // newhandle map:        hash  :-->  h_list pointe

	// handlecache:
	handlecache_t	handlecache_;  // handle cache:        replaced HANDLE  :-->  pointer to nhcache entry
	unsigned		cache_size_;
	unsigned		entries_;
};

#endif