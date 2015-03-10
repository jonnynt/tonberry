#ifndef _CACHEMAP_H
#define _CACHEMAP_H

#include "Main.h"
#include <stdint.h>
#include <unordered_set>
#include <unordered_map>

using namespace std;

typedef unsigned long long uint64;

class FieldMap
{
private:
	typedef unordered_set<string> fieldset_t;								// holds field names
	typedef fieldset_t::const_iterator fieldset_iter;						// points to field names

	fieldset_t fieldset;

	struct fieldset_iter_hasher
	{
		size_t operator()(fieldset_iter iter) const
		{						// since fieldset entries are unique, use field to hash fieldset_iter
			return std::hash<string>()(*iter);
		}
	};

	struct fieldset_iter_compare
	{
		bool operator()(const fieldset_iter& lhs, const fieldset_iter& rhs)
		{
			return *lhs == *rhs;											// since fieldset entries are unique, use fields to compare fieldset_iters
		}
	};

	typedef unordered_set<fieldset_iter, fieldset_iter_hasher> fieldset_iter_set_t;	// holds sets of fieldset_iter
	typedef fieldset_iter_set_t::iterator fieldset_iter_set_iter;					// iterates sets of fieldset_iter

	typedef unordered_map<uint64, fieldset_iter_set_t> fieldmap_t;			// maps hashes to a set of matching field names; unordered for O(1) access/insertion, set for collisions
	typedef fieldmap_t::iterator fieldmap_iter;

	fieldmap_t fieldmap;													// maps original texture hash to replacement texture name

public:

	/* count: count matches for a given hash
	  returns: size of fieldmap[hash]
	*/
	size_t count(uint64 hash);

	/* insert: adds hash :-> field to the map
	*/
	void insert(uint64 hash,		// map key
				const string& field	// map value
		);

	/* get_fields: returns a set of fields mapped to the given hash;
		   if the hash does not exist in the map, an empty field is returned
	   returns: true if the given hash is in the map, else false
	*/
	bool get_fields(uint64 hash,					// the hash key
					unordered_set<string>& result	// the set in which to put the matches
		);

	/* get_first_field: gets the first field mapped to the given hash
	  returns: true if the given hash is in the map, else false
	*/
	bool get_first_field(uint64 hash,	// the hash key
						string& result	// the string in which to place the result
		);

	/* get_intersection: returns the intersection of the sets of fields at the given hashes
	   returns: true if the intersection is non-empty, else false
	*/
	bool get_intersection(uint64 hash_1,				// the first hash
						  uint64 hash_2,				// the second hash
						  unordered_set<string>& result	// the set in which to put the intersection
		);

	/* writeMap: writes entire map to an output steram
	*/
	void writeMap(ofstream& out	// output stream to which to write
		);
};

class TextureCache
{

private:

	typedef pair<uint64, HANDLE>						nhcache_item_t;			// associates hashes with newhandles
	typedef list<nhcache_item_t>						nhcache_list_t;			// holds hashes and their associated newhandle in least-recently-accessed order
	typedef nhcache_list_t::iterator					nhcache_list_iter;
	typedef unordered_map<uint64, nhcache_list_iter>	nhcache_map_t;			// maps hashes to an entry in the newhandle list
	typedef nhcache_map_t::iterator						nhcache_map_iter;

	typedef unordered_map<HANDLE, nhcache_map_iter>		handlecache_t;			// maps handles to an entry in the nhcache map
	typedef handlecache_t::iterator						handlecache_iter;

	typedef struct nhcache_map_iter_hasher
	{
		size_t operator()(nhcache_map_iter iter) const
		{
			return iter->first;													// use nhcache entry hash to index reverse_handlecache
		}
	};

	typedef unordered_map<nhcache_map_iter, HANDLE, nhcache_map_iter_hasher> reverse_handlecache_t;	// reverse indexing of handlecache; needed for when values in handlecache are removed from the nhcache
	typedef reverse_handlecache_t::iterator				reverse_handlecache_iter;

	// together these make nhcache:
	nhcache_list_t			nh_list;
	nhcache_map_t			nh_map;

	// handlecache:
	handlecache_t			handlecache;
	reverse_handlecache_t	reverse_handlecache;

	size_t					entries;
	size_t					max_size;

	/*map_insert: insert or update nhcache item pointed to by given hash in the nh_map
	PRECONDITION: item exists in the nh_list
	*/
	void map_insert(uint64 hash,				// nh_map key		- the hash
					nhcache_list_iter item,		// nh_map value		- iterator to the nh_list item
					HANDLE replaced				// handlecache key	- will point to the new entry in nh_map
		);

public:
	TextureCache(unsigned);

	/*find: determine whether a hash is in the nhcache
	  returns: true if hash is in the nh_map, else false
	*/
	bool TextureCache::contains(uint64 hash	// the hash to find
		);

	/*find: determine whether a HANDLE is in the handlecache
	returns: true if HANDLE is in the handlecache map, else false
	*/
	bool TextureCache::contains(HANDLE replaced	// the HANDLE to find
		);


	/*at: access an element in the nhcache
	  returns: a reference to the HANDLE mapped to hash in the nhcache if it exists, or else null
	*/
	HANDLE TextureCache::at(uint64 hash	// the hash key
		);
	
	/*at: access an element in the handlecache
	  returns: a reference to the HANDLE mapped to replaced in the handlecache if it exists, or else null
	*/
	HANDLE TextureCache::at(HANDLE replaced	// the HANDLE key
		);


	/*insert: if nh_map[hash] exists, inserts replaced :-> hash :-> nh_map[hash] into the cache,
			  else does nothing
	*/
	void insert(HANDLE replaced,	// the handlecache key
				uint64 hash			// the nhcache key
		);

	/*insert: inserts replaced :-> hash :-> replacement into the cache
	  PRECONDITIONS:
		- hash is not on the cache
		- replacement has been created
	*/
	void insert(HANDLE replaced,	// in-game texture to be replaced by replacement
				uint64 hash,		// texture hash
				HANDLE replacement	// modded texture handle
	);

	/*erase: removes HANDLE from the cache
	*/
	void erase(HANDLE replaced		// in-game texture to remove from the cache
		);
};

#endif