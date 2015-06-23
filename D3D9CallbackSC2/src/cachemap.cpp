#include "Main.h"
#include "cachemap.h"

#define CACHE_DEBUG 0

#if CACHE_DEBUG
string cache_debug_file = "tonberry\\debug\\texture_cache.log";
#endif


TextureCache::TextureCache(unsigned max_size)
{
	this->max_size = max_size;

#if CACHE_DEBUG
	ofstream debug(cache_debug_file, fstream::out | fstream::trunc);
	debug << "CACHE_SIZE: " << max_size << endl << endl << endl;
	debug.close();
#endif

	nh_list = new nhcache_list_t();
	nh_map = new nhcache_map_t();
	handlecache = new handlecache_t();
	reverse_handlecache = new reverse_handlecache_t();

	nh_persist_end = nh_list->end();
}


TextureCache::~TextureCache()
{
	delete nh_list;
	delete nh_map;
	delete handlecache;
	delete reverse_handlecache;
}

bool TextureCache::full()
{
	return (nh_list->size() >= max_size && nh_persist_end == nh_list->end());
}


bool TextureCache::contains(uint64_t hash)
{
	return nh_map->find(hash) != nh_map->end();
}


bool TextureCache::contains(HANDLE replaced)
{
	return handlecache->find(replaced) != handlecache->end();
}


HANDLE TextureCache::at(HANDLE replaced)
{
	handlecache_iter cache_iter = handlecache->find(replaced);
	if (cache_iter == handlecache->end()) return nullptr;

	nhcache_map_iter map_iter = nh_map->find(cache_iter->second);
#if CACHE_DEBUG
	if (map_iter == nh_map->end()) {														// this should never happen
		ofstream debug(cache_debug_file, ofstream::out | ofstream::app);
//		debug << "LOCATION 0" << endl;
		debug << endl << "ERROR: handlecache entry (" << cache_iter->first << ", " << cache_iter->second << ") not in nh_map!" << endl;
		debug.close();
		return nullptr;
	}
#endif

	// move (most-recently-accessed) list item to front of non-persistent portion of nh_list
	nhcache_list_iter item = map_iter->second;
	if (!item->persist) {																	// if item is not persistent
		nh_list->splice(nh_persist_end, *nh_list, item);									// fastupdate() - full method call is excessive
		nh_persist_end = item;
	}

	return map_iter->second->replacement;
}


void TextureCache::map_insert(uint64_t hash, HANDLE replaced, nhcache_list_iter item)
{
#if CACHE_DEBUG
	ofstream debug(cache_debug_file, ofstream::out | ofstream::app);
#endif

	// update nh_map with new list item pointer
	pair<nhcache_map_iter, bool> map_insertion = nh_map->emplace(							// returns iterator to nh_map[hash] and boolean success
		pair<uint64_t, nhcache_list_iter>(hash, item));
	if (!map_insertion.second)																// if nh_map already contained hash, 
		map_insertion.first->second = item;													// change nh_map[hash] to item

	pair<handlecache_iter, bool> cache_insertion = handlecache->insert(						// returns iterator to handlecache[HANDLE] and boolean success
		pair<HANDLE, uint64_t>(replaced, hash));
	if (!cache_insertion.second) {															// if handlecache already contained HANDLE,
		uint64_t old_hash = cache_insertion.first->second;

		if (old_hash == hash) {																// if the entry is the same, then nothing needs to change
#if CACHE_DEBUG
			debug << endl;
			debug.close();
#endif
			return;
		}																					// otherwise, we need to remove the old reverse_handlecache entry

		pair<reverse_handlecache_iter, reverse_handlecache_iter> backpointer_range = reverse_handlecache->equal_range(old_hash);
		reverse_handlecache_iter backpointer = backpointer_range.first;
		for (; backpointer != backpointer_range.second && backpointer != reverse_handlecache->end(); backpointer++)
			if (backpointer->second == replaced) {
				int size_before = reverse_handlecache->size();
				reverse_handlecache->erase(backpointer);
#if CACHE_DEBUG
//				debug << "LOCATION 1" << endl;
				debug << "  Removing (" << backpointer->first << ", " << backpointer->second << ") from reverse_handlecache ";
				debug << "(size: " << size_before << " --> " << reverse_handlecache->size() << ")." << endl;
#endif
				break;
			}
		cache_insertion.first->second = hash;												// change handlecache entry
#if CACHE_DEBUG
//		debug << "LOCATION 2" << endl;
		debug << "  Changing (" << cache_insertion.first->first << ", " << old_hash << ") to ";
		debug << "(" << replaced << ", " << item->hash << ") in handlecache: ";
		debug << "nh_map[" << hash << "] = " << nh_map->at(hash)->replacement << endl;
	} else {
//		debug << "LOCATION 3" << endl;
		debug << "  Adding (" << replaced << ", " << hash << ") to handlecache: ";		// actually already did so in if() above
		debug << "nh_map[" << hash << "] = " << nh_map->at(hash)->replacement << endl;
	}

	int size_before = reverse_handlecache->size();
#else
	}
#endif

	reverse_handlecache->emplace(pair<uint64_t, HANDLE>(hash, replaced));

#if CACHE_DEBUG
//	debug << "LOCATION 4" << endl;
	debug << "  Adding (" << item->hash << ", " << replaced << ") to reverse_handlecache ";
	debug << "(size: " << size_before << " --> " << reverse_handlecache->size() << ")." << endl;
//	pair<reverse_handlecache_iter, reverse_handlecache_iter> backpointer_range = reverse_handlecache->equal_range(hash);
//	reverse_handlecache_iter backpointer = reverse_handlecache->begin();// backpointer_range.first;
//	for (; /*backpointer != backpointer_range.second &&*/ backpointer != reverse_handlecache->end(); backpointer++)
//		debug << "    \t\t\t\t\t\t(" << backpointer->first << ", " << backpointer->second << ")" << endl;

	debug << endl;
	debug.close();
#endif
}


bool TextureCache::update(HANDLE replaced, uint64_t hash)
{
	nhcache_map_iter updated = nh_map->find(hash);	// this line is now needed, we want to check if we update or not 
	if (updated == nh_map->end()) return false;

#if CACHE_DEBUG
	ofstream debug(cache_debug_file, ofstream::out | ofstream::app);
//	debug << "LOCATION 5" << endl;
	debug << "Inserting (" << replaced << " :-> nh_map[" << hash << "] = " << updated->second->replacement << "):" << endl;
#endif

	/* UPDATE NH CACHE ACCESS ORDER */
	nhcache_list_iter item = updated->second;

	// move (most-recently-accessed) list item to front of non-persistent portion of nh_list
	if (!item->persist) {																	// if item is not persistent
		nh_list->splice(nh_persist_end, *nh_list, item);									// fastupdate() - full method call is excessive
		nh_persist_end = item;

#if CACHE_DEBUG
		//	debug << "LOCATION 6" << endl;
		debug << "  Moving (" << item->hash << ", " << item->replacement << ") to nh_persist_end (size: " << nh_list->size() << "): nh_persist_end = ";
		debug << "(" << nh_persist_end->hash << ", " << nh_persist_end->replacement << "): " << ((updated->second == nh_persist_end) ? "true" : "false") << endl;

		debug << "    Distance between nh_persist_end and nh_list->end(): " << distance(nh_persist_end, nh_list->end()) << endl;
#endif
	}
#if CACHE_DEBUG
	debug.close();
#endif

	map_insert(hash, replaced, item);
	return true;
}


void TextureCache::insert(HANDLE replaced, uint64_t hash, HANDLE replacement, bool persist)
{
#if CACHE_DEBUG
	ofstream debug(cache_debug_file, ofstream::out | ofstream::app);
#endif

	nhcache_list_iter item;
	if (persist) {
		nh_list->push_front({ hash, replacement, persist });
		item = nh_list->begin();

#if CACHE_DEBUG
		debug << "Inserting (" << replaced << " :-> (" << hash << ", " << replacement << ")):" << endl;
		debug << "  Inserting (" << hash << ", " << replacement << ") at front of nh_list (size: " << nh_list->size() << "): nh_list->begin() = ";
		debug << "(" << nh_list->begin()->hash << ", " << nh_list->begin()->replacement << ")" << endl;
#endif
	} else {
		nh_persist_end = item = nh_list->insert(nh_persist_end, { hash, replacement, persist });

#if CACHE_DEBUG
		//	debug << "LOCATION 7" << endl;
		debug << "Inserting (" << replaced << " :-> (" << hash << ", " << replacement << "):" << endl;
		debug << "  Inserting (" << hash << ", " << replacement << ") at nh_persist_end (size: " << nh_list->size() << "): nh_persist_end = ";
		debug << "(" << nh_persist_end->hash << ", " << nh_persist_end->replacement << ")" << endl;

		debug << "    Distance between nh_persist_end and nh_list->end(): " << distance(nh_persist_end, nh_list->end()) << endl;
#endif
	}

	/* MAKE SURE NHCACHE IS THE CORRECT SIZE */
	while (nh_list->size() > max_size) {													// "while" for completeness; should never be greater than (max_size + 1)
		// get pointer to last (least recent) list item
		nhcache_list_iter last_elem = nh_list->end();
		--last_elem;

		if (last_elem == nh_persist_end) nh_persist_end = nh_list->end();
#if CACHE_DEBUG
//		debug << "LOCATION 8" << endl;
		debug << "  Removing (" << last_elem->hash << ", " << last_elem->replacement << ") from back of nh_list (size: " << (nh_list->size() - 1) << ")." << endl;
#endif

		// dispose of texture
		((IDirect3DTexture9*)last_elem->replacement)->Release();
		last_elem->replacement = nullptr;

		// if we're going to delete a hash from the nh_map, we need to first remove entries that map to that hash from the handlecache
		nhcache_map_iter to_delete = nh_map->find(last_elem->hash);
		pair<reverse_handlecache_iter, reverse_handlecache_iter> backpointer_range =
			reverse_handlecache->equal_range(last_elem->hash);

		reverse_handlecache_iter backpointer = backpointer_range.first;
		for (; backpointer != backpointer_range.second && backpointer != reverse_handlecache->end(); backpointer++) {
#if CACHE_DEBUG
			//			debug << "LOCATION 9" << endl;
			debug << "    Removing (" << backpointer->second << ", " << backpointer->first << ") from handlecache." << endl;
#endif
			handlecache->erase(backpointer->second);										// remove from handlecache; reverse_handlecache will be removed
		}																					// afterward to preserve iterators in the backpointer_range

#if CACHE_DEBUG
		//		debug << "LOCATION 10" << endl;
		size_t size_before = reverse_handlecache->size();
		size_t num_removed = reverse_handlecache->erase(last_elem->hash);
		debug << "    Removed " << num_removed << " entries from reverse_handlecache (size: " << size_before << " --> " << reverse_handlecache->size() << ")." << endl;
		debug << "  Removing (" << to_delete->first << ", (" << to_delete->second->hash << ", " << to_delete->second->replacement << ")) from nh_map." << endl;
#endif

		// remove from map (this is why the nh_list stores pair<hash, handle>)
		nh_map->erase(to_delete);

		// pop from list
		nh_list->pop_back();
	}
	/* END MAKE SURE NHCACHE IS THE CORRECT SIZE */

#if CACHE_DEBUG
	debug.close();
#endif

	map_insert(hash, replaced, item);
}


void TextureCache::erase(HANDLE replaced)
{
	handlecache_iter iter;
	if ((iter = handlecache->find(replaced)) == handlecache->end()) return;

#if CACHE_DEBUG
	ofstream debug(cache_debug_file, ofstream::out | ofstream::app);
//		debug << "LOCATION 11" << endl;
	debug << "Erasing unused HANDLE " << replaced << ": " << endl;
	debug << "  Removing (" << iter->first << ", " << iter->second << ") from handlecache." << endl;
#endif

	pair<reverse_handlecache_iter, reverse_handlecache_iter> backpointer_range = reverse_handlecache->equal_range(iter->second);
	reverse_handlecache_iter backpointer = backpointer_range.first;
	for (; backpointer != backpointer_range.second && backpointer != reverse_handlecache->end(); backpointer++)
		if (backpointer->second == replaced) {
			int size_before = reverse_handlecache->size();
			reverse_handlecache->erase(backpointer);									// remove matching backpointer from reverse_handlecache
#if CACHE_DEBUG
//				debug << "LOCATION 12" << endl;
			debug << "  Removing (" << backpointer->first << ", " << backpointer->second << ") from reverse_handlecache ";
			debug << "(size: " << size_before << " --> " << reverse_handlecache->size() << ")." << endl;
#endif
			break;
		}

	nhcache_list_iter list_iter = nh_map->find(iter->second)->second;					// move list item to back of nh_list
	if (!list_iter->persist && !reverse_handlecache->count(iter->second)) {				// (if it is not persistent and has no more entries in handlecache)
		if (list_iter == nh_persist_end)
			nh_persist_end++;
		nh_list->splice(nh_list->end(), *nh_list, list_iter);							// so that it will be deleted first when the cache is full
#if CACHE_DEBUG
		debug << "  Moving (" << list_iter->hash << ", " << list_iter->replacement << ") to back of nh_list. (nh_list->end()-1) = ";
		nhcache_list_iter last = nh_list->end();
		last--;
		debug << "(" << last->hash << ", " << last->replacement << ")" << endl;

		debug << "    Distance between nh_persist_end and nh_list->end(): " << distance(nh_persist_end, nh_list->end()) << endl;
#endif
	}

	handlecache->erase(iter);															// remove entry from handlecache

#if CACHE_DEBUG
//		debug << "LOCATION 13" << endl;
	debug << endl;
	debug.close();
#endif
}