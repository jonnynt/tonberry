#include "Main.h"
#include "cachemap.h"

#include <fstream>
std::string debug_file = "tonberry\\debug\\texture_cache.log";

size_t FieldMap::count(uint64 hash)
{
	fieldmap_iter iter = fieldmap.find(hash);
	
	return (iter == fieldmap.end()) ? 0 : iter->second.size();
}

void FieldMap::insert(uint64 hash, const string& field)
{
	fieldset_iter ptr_to_field_name = fieldset.insert(field).first;
	fieldmap[hash].insert(ptr_to_field_name);
}

bool FieldMap::get_fields(uint64 hash, unordered_set<string>& result)
{
	fieldmap_iter map_iter;
	if ((map_iter = fieldmap.find(hash)) == fieldmap.end()) return false;

	fieldset_iter_set_iter match_iter;
	for (match_iter = map_iter->second.begin(); match_iter != map_iter->second.end(); match_iter++)
		result.insert(**match_iter);

	return true;
}

bool FieldMap::get_first_field(uint64 hash, string& result)
{
	fieldmap_iter iter;
	if ((iter = fieldmap.find(hash)) == fieldmap.end()) return false;

	result = **(iter->second.begin());

	return true;
}

bool FieldMap::get_intersection(uint64 hash_1, uint64 hash_2, unordered_set<string>& result)
{
	fieldmap_iter iter_1, iter_2;
	fieldset_iter_set_t intersection;

	// search for hash_upper
	if ((iter_1 = fieldmap.find(hash_1)) == fieldmap.end() ||
		(iter_2 = fieldmap.find(hash_2)) == fieldmap.end())
		return false;
	
	set_intersection(iter_1->second.begin(), iter_1->second.end(),
					 iter_2->second.begin(), iter_2->second.end(),
					 inserter(intersection, intersection.end()), fieldset_iter_compare());

	if (intersection.size() == 0) return false;

	fieldset_iter_set_iter iter;
	for (iter = intersection.begin(); iter != intersection.end(); iter++)
		result.insert(**iter);

	return true;
}

void FieldMap::writeMap(ofstream& out)
{
	fieldmap_iter map_iter;
	for (map_iter = fieldmap.begin(); map_iter != fieldmap.end(); map_iter++) {
		out << map_iter->first << ":";
		fieldset_iter_set_iter field_iter;
		for (field_iter = map_iter->second.begin(); field_iter != map_iter->second.end(); field_iter++) {
			out << " " << **field_iter << ";";
		}
		out << endl;
	}
}

TextureCache::TextureCache(unsigned max_size)
{
	entries			= 0;
	this->max_size = max_size;

	ofstream debug(debug_file, fstream::out | fstream::trunc);
	debug << "CACHE_SIZE: " << max_size << endl;
	debug.close();
}

bool TextureCache::contains(uint64 hash)
{
	return nh_map.find(hash) != nh_map.end();
}


bool TextureCache::contains(HANDLE replaced)
{
	return handlecache.find(replaced) != handlecache.end();
}


HANDLE TextureCache::at(uint64 hash)
{
	nhcache_map_iter iter = nh_map.find(hash);

	if (iter == nh_map.end()) return NULL;

	return iter->second->second;
}


HANDLE TextureCache::at(HANDLE replaced)
{
	handlecache_iter cache_iter = handlecache.find(replaced);
	if (cache_iter == handlecache.end()) return NULL;

	nhcache_map_iter map_iter = nh_map.find(cache_iter->second);
	if (map_iter == nh_map.end()) {
		ofstream debug(debug_file);
		debug << endl << "ERROR: handlecache entry (" << cache_iter->first << ", " << cache_iter->second << ") not in nh_map!" << endl;
		debug.close();
		return NULL;
	}

	return map_iter->second->second;
}


void TextureCache::map_insert(uint64 hash, nhcache_list_iter item, HANDLE replaced)
{
	ofstream debug(debug_file, ofstream::out | ofstream::app);


	// update nh_map with new list item pointer
	std::pair<nhcache_map_iter, bool> insertion = nh_map.insert(							// returns iterator to nh_map[hash_used] and boolean success
		std::pair<uint64, nhcache_list_iter>(hash, item));
	if (!insertion.second)																	// if nh_map already contained hash, 
		insertion.first->second = nh_list.begin();											// change nh_map[hash] to nh_list.begin()


	debug << "\tAdding (" << replaced << ", (" << item->first << ") with new nh_map entry to handlecache:";
	handlecache[replaced] = hash;															// insert in both handlecache maps
	debug << " (" << replaced << ", " << hash << "); nh_map[" << hash << "] = " << nh_map[hash]->second << endl;
	debug << "\tAdding (" << item->first << ", (" << replaced << ") with new nh_map entry to reverse_handlecache:";
	reverse_handlecache.insert(pair<uint64, HANDLE>(hash, replaced));
	debug << " (" << hash << ", " << replaced << "); nh_map[" << hash << "] = " << nh_map[hash]->second << endl;
	debug << endl;

	debug.close();
}


void TextureCache::insert(HANDLE replaced, uint64 hash)
{
	ofstream debug(debug_file, ofstream::out | ofstream::app);

	debug << "Inserting (" << replaced << " :-> nh_map[" << hash << "] = " << nh_map[hash]->second << "):" << endl;
	nhcache_map_iter updated = nh_map.find(hash);
	if (updated == nh_map.end()) return;

	/* UPDATE NH CACHE ACCESS ORDER */
	nhcache_list_iter item = updated->second;

	debug << "\tMoving (" << item->first << ", " << item->second << ") to front of nh_list: nh_list.begin() = ";
	// move (most-recently-accessed) list item to front of nh_list
	nh_list.erase(item);
	nh_list.push_front(*item);
	debug << "(" << nh_list.begin()->first << ", " << nh_list.begin()->second << ")" << endl;

	debug.close();
	map_insert(hash, nh_list.begin(), replaced);
}


void TextureCache::insert(HANDLE replaced, uint64 hash, HANDLE replacement)
{
	ofstream debug(debug_file, ofstream::out | ofstream::app);
	debug << "Inserting (" << replaced << " :-> (" << hash << ", " << replacement << "):" << endl;

	debug << "\tInserting (" << hash << ", " << replacement << ") at front of nh_list: nh_list.begin() = ";
	nh_list.push_front(nhcache_item_t(hash, replacement));
	debug << "(" << nh_list.begin()->first << ", " << nh_list.begin()->second << ")" << endl;

	/* MAKE SURE NHCACHE IS THE CORRECT SIZE */
	while (nh_list.size() > max_size) {														// "while" for completeness but this should only ever loop once
		// get pointer to last (least recent) list item
		nhcache_list_iter last_elem = nh_list.end();
		--last_elem;
		debug << "\tRemoving (" << last_elem->first << ", " << last_elem->second << ") from back of nh_list." << endl;

		// dispose of texture
		((IDirect3DTexture9*)last_elem->second)->Release();
		last_elem->second = NULL;

		// if we're going to delete a hash from the nh_map, we need to first remove entries that map to that hash from the handlecache
		nhcache_map_iter to_delete = nh_map.find(last_elem->first);
		pair<reverse_handlecache_iter, reverse_handlecache_iter> backpointer_range =
			reverse_handlecache.equal_range(last_elem->first);

		reverse_handlecache_iter backpointer = backpointer_range.first;
		for (;  backpointer != backpointer_range.second && backpointer != reverse_handlecache.end(); backpointer++) {
			debug << "\tRemoving (" << backpointer->second << ", " << backpointer->first << ") from handlecache." << endl;
			handlecache.erase(backpointer->second);											// remove from handlecache; reverse_handlecache will be removed
		}																					// afterward to preserve iterators in the backpointer_range
		int num_removed = reverse_handlecache.erase(last_elem->first);
		debug << "\tRemoved " << num_removed << " entries from reverse_handlecache." << endl;

		// remove from map (this is why the nh_list stores pair<hash, handle>)
		debug << "\tRemoving (" << to_delete->first << ", (" << to_delete->second->first << ", " << to_delete->second->second << ")) from nh_map." << endl;
		nh_map.erase(to_delete);

		// pop from list
		nh_list.pop_back();
	}
	/* END MAKE SURE NHCACHE IS THE CORRECT SIZE */

	debug.close();
	map_insert(hash, nh_list.begin(), replaced);
}

void TextureCache::erase(HANDLE replaced)
{
	handlecache_iter iter;
	if ((iter = handlecache.find(replaced)) != handlecache.end()) {
		reverse_handlecache.erase(iter->second);											// any old cache entries should be removed from both maps
		handlecache.erase(iter);
	}
}