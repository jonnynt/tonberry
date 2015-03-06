#include "Main.h"
#include "cachemap.h"

HashCache::HashCache(unsigned cache_size){
	_lrulist.clear();
	_cachedata.clear();
	_entries=0;
	_cachesize=cache_size;
}

void HashCache::insert(uint64_t key, string val){
	//añade al frente de la lista
	_lrulist.push_front(std::make_pair(key, val));
	//y al mapa de cache
	_cachedata[ key ] = _lrulist.begin();
	_entries++;
	
	if(_entries > _cachesize){

		_cachedata.erase( _lrulist.back().first ); //ident con key
		_lrulist.pop_back();
		_entries--;

	}
}

HashCache::iterator HashCache::find(uint64_t key){
	CacheUMap::iterator it = _cachedata.find(key);
	if(it != _cachedata.end()) return it->second;
	return _lrulist.end();
}

//actualiza un elemento y lo mueve al principio
void HashCache::update(LRUList::iterator it){
	_lrulist.push_front(make_pair(it->first,it->second));
	_cachedata[ it->first ] = _lrulist.begin();
	_lrulist.erase(it);
}

//_________________________________________________

TextureCache::TextureCache(unsigned cache_size){
	entries_ = 0;
	cache_size_=cache_size;
}

void TextureCache::insert(uint64_t hash, HANDLE replaced, HANDLE replacement){
	// we know hash is not in the nhcache, so add hash->tempnewhandle to nhcache
	nh_list_.push_front(lru_item_t(hash, replacement));                         // O(1) in linked list
	nh_map_[hash] = nh_list_.begin();                                              // O(1) in unordered_map
    entries_++;                
	
	/* MAKE SURE NHCACHE IS THE CORRECT SIZE */
	while (entries_ > cache_size_) {                // I use "while" for completeness but this should only ever loop once
		// get pointer to last (least recent) list item
		lru_list_iter last_elem = nh_list_.end();             // O(1) in linked list
		--last_elem;
                        
		// dispose of texture
		((IDirect3DTexture9*) last_elem->second)->Release();
                        
		// remove from map (this is why the nh_list stores pair<hash, handle>)
		nh_map_.erase(last_elem->first);         // O(1) in unordered_map
		// pop from list
		nh_list_.pop_back();                      // O(1) in linked list
		entries_--;
	}
	/* END MAKE SURE NHCACHE IS THE CORRECT SIZE */
                    
                    
	handlecache_[replaced] = nh_map_.find(hash);                 // the way we deal with handlecache does not change
 

}


//actualiza un elemento y lo mueve al principio
bool TextureCache::update(uint64_t hash, HANDLE replaced){
	lru_map_iter ptr_to_replacement = nh_map_.find(hash);

	if( entries_ > 0 && ptr_to_replacement != nh_map_.end() ){ //casos 2 y 3
            
		/* UPDATE NH CACHE ACCESS ORDER */
		lru_list_iter ptr_to_nh_list = ptr_to_replacement->second;
                
		// move (most-recently-accessed) list item to front of nh_list
		nh_list_.push_front(*ptr_to_nh_list);                                      // O(1) in linked list
		nh_list_.erase(ptr_to_nh_list);                                            // O(1) in linked list
                
		// update nh_map with new list item pointer
		nh_map_[hash] = nh_list_.begin();                                           // O(1) in unordered_map
		/* END UPDATE NH CACHE */
              
		handlecache_[replaced] = nh_map_.find(hash);            // need to point this at the new, updated entry in nh_map (with the new list location)
		
		return true;											//hash in caché: replace handle and return success
	}
	return false;												//didn't found hash (nhcache==0 or simply didn't found it)
}

bool TextureCache::setTexture(HANDLE replaced, IDirect3DTexture9** newtexture){

	handlecache_iter ptr_to_replacement = handlecache_.find(replaced);
	if ( entries_ > 0 && ptr_to_replacement != handlecache_.end())   //SurfaceHandles[j] found on handlecache -> will return true
	{
		*newtexture = (IDirect3DTexture9*)ptr_to_replacement->second->second->second; //ptr_to_replacement->second =                 pointer to nh_map
																						//ptr_to_replacement->second->second =         pointer to nh_list
		if(*newtexture){							//ptr_to_replacement->second->second->second = pointer to replacement HANDLE
			return true;
		}
	}
	return false;	//SurfaceHandles[j] doesn't match with our cache -> returns false
}