#include "Main.h"
#include "cachemap.h"


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
	this->max_size	= max_size;
}


void TextureCache::map_insert(uint64 hash, nhcache_list_iter item, HANDLE replaced)
{
	// update nh_map with new list item pointer
	std::pair<nhcache_map_iter, bool> insertion = nh_map.insert(							// returns iterator to nh_map[hash_used] and boolean success
		std::pair<uint64, nhcache_list_iter>(hash, item));		
	if (!insertion.second)																	// if nh_map already contained hash, 
		insertion.first->second = nh_list.begin();											// change nh_map[hash] to nh_list.begin()

	handlecache[replaced] = insertion.first;												// insert in both handlecache maps
	reverse_handlecache[insertion.first] = replaced;
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
	handlecache_iter iter = handlecache.find(replaced);

	if (iter == handlecache.end()) return NULL;

	return iter->second->second->second;
}


void TextureCache::insert(HANDLE replaced, uint64 hash)
{
	nhcache_map_iter updated = nh_map.find(hash);
	if (updated == nh_map.end()) return;

	/* UPDATE NH CACHE ACCESS ORDER */
	nhcache_list_iter ptr_to_nh_list = updated->second;

	// move (most-recently-accessed) list item to front of nh_list
	nh_list.erase(ptr_to_nh_list);
	nh_list.push_front(*ptr_to_nh_list);

	/* END UPDATE NH CACHE ACCESS ORDER */

	// erase now-invalidate handles from handlecache
	reverse_handlecache_iter checkpointer = reverse_handlecache.find(updated);
	if (checkpointer != reverse_handlecache.end()) {										// when we moved the nh_list_item, we invalidated an entry in handlecache
		handlecache.erase(checkpointer->second);
		reverse_handlecache.erase(checkpointer);
	}

	map_insert(hash, nh_list.begin(), replaced);
}


void TextureCache::insert(HANDLE replaced, uint64 hash, HANDLE replacement)
{
	nh_list.push_front(nhcache_item_t(hash, replacement));

	/* MAKE SURE NHCACHE IS THE CORRECT SIZE */
	while (nh_list.size() > max_size) {														// "while" for completeness but this should only ever loop once
		// get pointer to last (least recent) list item
		nhcache_list_iter last_elem = nh_list.end();
		--last_elem;

		// dispose of texture
		((IDirect3DTexture9*)last_elem->second)->Release();
		last_elem->second = NULL;

		// if we're going to delete an nh_map entry, we need to remove it from handlecache as well
		nhcache_map_iter to_delete = nh_map.find(last_elem->first);
		reverse_handlecache_iter backpointer = reverse_handlecache.find(to_delete);

		if (backpointer != reverse_handlecache.end()) {										// is it even possible for handlecache to not contain to_delete?
			handlecache.erase(backpointer->second);											// remove from both maps
			reverse_handlecache.erase(backpointer);
		}

		// remove from map (this is why the nh_list stores pair<hash, handle>)
		nh_map.erase(to_delete);
		nhcache_map_iter temp_iter = nh_map.find(to_delete->first);

		// pop from list
		nh_list.pop_back();
	}
	/* END MAKE SURE NHCACHE IS THE CORRECT SIZE */

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