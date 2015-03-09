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

void TextureCache::insert(uint64_t hash, HANDLE replaced, HANDLE replacement, unordered_map<uint64_t,string> * hashmap){
	// we know hash is not in the nhcache, so add hash->tempnewhandle to nhcache
	nh_list_.push_front(lru_item_t(hash, replacement));                         // O(1) in linked list
	std::pair<lru_map_iter, bool> insertion = nh_map_.insert(std::pair<uint64_t, lru_list_iter>(hash, nh_list_.begin()));   
	if(!insertion.second) insertion.first->second = nh_list_.begin();           // O(1) in unordered_map
    entries_++;     

	/* MAKE SURE NHCACHE IS THE CORRECT SIZE */
	while (entries_ > cache_size_) {                // I use "while" for completeness but this should only ever loop once
		// get pointer to last (least recent) list item
		lru_list_iter last_elem = nh_list_.end();             // O(1) in linked list
		--last_elem;
                        
		// dispose of texture
		((IDirect3DTexture9*) last_elem->second)->Release();

		// if we're going to delete an nh_map entry, we need to remove it from handlecache as well
		lru_map_iter to_delete = nh_map_.find(last_elem->first);
		reverse_handlecache_iter backpointer = reverse_handlecache_.find(to_delete);
                        
		if (backpointer != reverse_handlecache_.end()) {        // is it even possible for handlecache to not contain to_delete?
			handlecache_.erase(backpointer->second);            // remove from both maps
			reverse_handlecache_.erase(backpointer);
		}
                        
		// remove from map (this is why the nh_list stores pair<hash, handle>)
		nh_map_.erase(to_delete);                                                 // O(1) in unordered_map

		// pop from list
		nh_list_.pop_back();                      // O(1) in linked list
		entries_--;
	}
	/* END MAKE SURE NHCACHE IS THE CORRECT SIZE */
	reverse_handlecache_iter checkpointer = reverse_handlecache_.find(insertion.first);
	if(checkpointer != reverse_handlecache_.end()){//found
		handlecache_.erase(checkpointer->second);	//we have to grant that any iterator will be lost in the way
	}
	handlecache_[replaced] = insertion.first;                 // the way we deal with handlecache does not change
	reverse_handlecache_[insertion.first] = replaced;
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
		ptr_to_replacement->second = nh_list_.begin();

		/* END UPDATE NH CACHE */
		reverse_handlecache_iter checkpointer = reverse_handlecache_.find(ptr_to_replacement);
		if(checkpointer != reverse_handlecache_.end()){//found
			handlecache_.erase(checkpointer->second);	//we have to grant that any iterator will be lost in the way
		}
		handlecache_[replaced] = ptr_to_replacement;
		reverse_handlecache_[ptr_to_replacement] = replaced;

		return true;											//hash in caché: replace handle and return success
	}
	return false;												//didn't found hash (nhcache==0 or simply didn't found it)
}

bool TextureCache::setTexture(HANDLE replaced, IDirect3DTexture9** newtexture, unordered_map<uint64_t,string> * hashmap){

   // ofstream checkfile;
   // checkfile.open ("tonberry/tests/cachetests.csv", ofstream::out | ofstream::app);


	handlecache_iter ptr_to_replacement = handlecache_.find(replaced);
	if ( /*entries_ > 0 &&*/ ptr_to_replacement != handlecache_.end())   //SurfaceHandles[j] found on handlecache -> will return true
	{
		*newtexture = (IDirect3DTexture9*)ptr_to_replacement->second->second->second; //ptr_to_replacement->second =                 pointer to nh_map
		//checkfile << "\nSetting texture (" << hashmap->find(ptr_to_replacement->second->first)->second << ")" << flush;
																							//ptr_to_replacement->second->second =         pointer to nh_list
		if(*newtexture){							//ptr_to_replacement->second->second->second = pointer to replacement HANDLE
			///*HERE WE NEED TO UPDATE THE LRU LIST*/
			//lru_map_iter ptr_to_hash = ptr_to_replacement->second;
			//lru_list_iter ptr_to_nh_list = ptr_to_replacement->second->second;
			//
			//nh_list_.push_front(*ptr_to_nh_list);
			//nh_list_.erase(ptr_to_nh_list);

			//nh_map_[ptr_to_hash->first] = nh_list_.begin();

			///*UPDATED NH CACHE*/
			//handlecache_[replaced] = ptr_to_hash;
			//reverse_handlecache_[ptr_to_hash] = replaced;
			//checkfile << "\nRefreshed texture, LRU Tex: " << hashmap->find(nh_list_.back().first)->second << flush << endl;
			return true;
		}
	}
	return false;	//SurfaceHandles[j] doesn't match with our cache -> returns false
}