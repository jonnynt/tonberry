// ConsoleTesting.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "MurmurHash2.h"
#include "texturehash.h"
#include <iostream>
#include <ctime>
#include <array>
#include <unordered_set>
#include <unordered_map>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <sstream>
namespace fs = boost::filesystem;
using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::hash;
using std::array;
using std::deque;
using std::set;
using std::unordered_set;
using std::unordered_map;
using std::map;
using std::pair;
using std::min;
using std::max;
using std::max_element;
using std::ofstream;
using std::greater;
using std::list;
using std::stringstream;
using std::ifstream;

using namespace TextureHash::Omzy;
using namespace TextureHash::FNV;
using namespace TextureHash::Murmur2;

const static fs::path FF8_ROOT("C:\\Program Files (x86)\\Steam\\steamapps\\common\\FINAL FANTASY VIII");

// Return path when appended to a_From will resolve to same as a_To
fs::path make_relative(fs::path a_From, fs::path a_To)
{
	a_From = fs::absolute(a_From); a_To = fs::absolute(a_To);
	fs::path ret;
	fs::path::const_iterator itrFrom(a_From.begin()), itrTo(a_To.begin());
	// Find common base
	for (fs::path::const_iterator toEnd(a_To.end()), fromEnd(a_From.end()); itrFrom != fromEnd && itrTo != toEnd && *itrFrom == *itrTo; ++itrFrom, ++itrTo);
	// Navigate backwards in directory to reach previously found base
	for (fs::path::const_iterator fromEnd(a_From.end()); itrFrom != fromEnd; ++itrFrom) {
		if ((*itrFrom) != ".")
			ret /= "..";
	}
	// Now navigate down the directory branch
	for (; itrTo != a_To.end(); ++itrTo)
		ret /= *itrTo;
	return ret;
}

const int DIM_X = 128;
const int DIM_Y = 128;

// hold image paths and data
struct image
{
	fs::path path;
	cv::Rect rect;
	uint64 hash;

	image(fs::path p, cv::Rect r, uint64 h = 0) : path(p), rect(r), hash(h)
	{
		//mat = cv::imread(path.string(), CV_LOAD_IMAGE_COLOR);
	}

	cv::Mat mat()
	{
		cv::Mat in = cv::imread(path.string());
		try {
			return in(rect);
		} catch (cv::Exception) {
			return in;
		}
	}

	char rect_char()
	{
		if (rect.x < DIM_X) {
			if (rect.y < DIM_X)
				return 'a';
			return 'c';
		} else {
			if (rect.y < DIM_Y)
				return 'b';
			return 'd';
		}
	}
};

void get_images(fs::path analysis, deque<image*>& images, unordered_set<uint64>& image_hashes, bool name_is_hash = false)
{
	if (fs::is_regular_file(analysis)) {
		ifstream in;
		in.open(analysis.string().c_str());

		string line;
		while (getline(in, line)) {
			string::size_type semi = line.find(';');
			if (semi == string::npos) continue;

			uint64 hash = _strtoui64(line.substr(0, semi).c_str(), NULL, 10);
			if (hash == _UI64_MAX || hash == 0) continue;
			string path = line.substr(semi + 1);
			boost::erase_all(path, "\"");
			image * img = new image(fs::path(path), cv::Rect(0, 0, DIM_X, DIM_Y), hash);
			images.push_back(img);
			image_hashes.insert(hash);
		}
		return;
	}
	try {
		fs::directory_iterator iter(analysis), end;
		for (; iter != end; iter++) {
			fs::path path = iter->path();
			if (fs::is_directory(path)) {
				/*if (analysis.stem().string() == "mapdata2")
					cout << "Skipping " << path.stem().string() << endl;
				else if (analysis.parent_path().stem().string() == "mapdata2" && !isdigit(path.string().at(path.string().length() - 1)))
					cout << "Skipping " << path.stem().string() << endl;
				else*/ if (path.stem().string() != "collisions")
					get_images(path, images, image_hashes, name_is_hash);						// recursive
			}
			else if (fs::is_regular_file(path) &&
					(boost::iequals(path.extension().string(), ".bmp") ||
					boost::iequals(path.extension().string(), ".png"))) {
				cv::Mat img = cv::imread(path.string());

				// store 1-4 128x128 objects
				// 		|_a_|_b_|
				//		|_c_|_d_|

				if (name_is_hash) {																			// only one obj with hash=name
					cv::Rect rect = cv::Rect(0, 0, min(img.cols, DIM_X), min(img.rows, DIM_Y));
					string fn = path.stem().string();
					uint64 hash = _strtoui64(path.stem().string().c_str(), NULL, 10);
					images.push_back(new image(path, rect, hash));
				} else {
					// a
					cv::Rect rect = cv::Rect(0, 0, min(img.cols, DIM_X), min(img.rows, DIM_Y));
					uint64 hash = Murmur2_Full(img(rect));													// hash the object
					if (image_hashes.insert(hash).second) images.push_back(new image(path, rect, hash));	// only insert unique objects

					// b
					if (img.cols > DIM_X) {
						rect = cv::Rect(DIM_X, 0, min(img.cols - DIM_X, DIM_X), min(img.rows, DIM_Y));
						uint64 hash = Murmur2_Full(img(rect));
						if (image_hashes.insert(hash).second) images.push_back(new image(path, rect, hash));
					}

					// c
					if (img.rows > DIM_Y) {
						rect = cv::Rect(0, DIM_Y, min(img.cols, DIM_X), min(img.rows - DIM_Y, DIM_Y));
						uint64 hash = Murmur2_Full(img(rect));
						if (image_hashes.insert(hash).second) images.push_back(new image(path, rect, hash));
					}

					// d
					if (img.cols > DIM_X && img.rows > DIM_Y) {
						rect = cv::Rect(DIM_X, DIM_Y, min(img.cols - DIM_X, DIM_X), min(img.rows - DIM_Y, DIM_Y));
						uint64 hash = Murmur2_Full(img(rect));
						if (image_hashes.insert(hash).second) images.push_back(new image(path, rect, hash));
					}
				}
			}
		}
	} catch (fs::filesystem_error e) {
		cout << e.what() << endl;
	}
}

typedef long long_dim[DIM_X];
typedef double double_dim[DIM_X];
typedef long long_count[255];

void Analyze_Pixels(fs::path analysis, fs::path dest, bool name_is_hash = false)
{
	bool DO_OLD_ALGO = false;
	bool DO_HIGH_VAR = false;
	bool DO_LOW_MODE = true;
	bool DO_COLANLYZ = false;

	int COLLIDE_COUNT	= 100;

	cout << "Running Image Analysis..." << endl;
	cout << "Reading images...";

	deque<image*> images;
	unordered_set<uint64> image_hashes;
	get_images(analysis, images, image_hashes, name_is_hash);
	
	cout << " found " << images.size() << " images." << endl;

	map<uint64, set<int>> hashmap;
	clock_t start_time, end_time, total_time = 0;
	double avg_time;
	int collisions = 0;
	int unique_hashes = 0;

	if (DO_OLD_ALGO) {
		// hash images using old algorithm
		hashmap.clear();
		total_time = 0;
		for (int i = 0; i < images.size(); i++) {
			cv::Mat img = images[i]->mat();
			uint64 hash;

			start_time = clock();
			hash = Old_Hash_Algorithm_1(img);
			end_time = clock();
			total_time += end_time - start_time;
			hashmap[hash].insert(i);
		}
		avg_time = ((double)total_time) / images.size();

		// count collisions
		collisions = 0;
		unique_hashes = 0;
		for (pair<uint64, set<int>> hashset : hashmap) {
			if (hashset.second.size() < 2) continue;
			unique_hashes++;
			collisions += hashset.second.size() - 1;
		}

		cout << "Omzy:      " << images.size() << " images; " << collisions << " collisions; " << unique_hashes << " hashes; ~" << avg_time << " ms per image" << endl;
	}

	deque<HashCoord> coords;
	
	if (DO_HIGH_VAR) {
		// get sum, mean, and variance of each pixel value in image set
		ofstream out;
		out.open(dest.string());
		out << "y,x,sum,mean,var,mode_count" << endl;


		long_dim* sum = new long_dim[DIM_Y];
		double_dim* mean = new double_dim[DIM_Y];
		double_dim* var = new double_dim[DIM_Y];
		deque<uchar> pixvals[DIM_Y][DIM_X];

		// init to 0
		for (int y = 0; y < DIM_Y; y++) {
			for (int x = 0; x < DIM_X; x++) {
				sum[y][x] = 0;
			}
		}


		// get pixvals and sums
		for (image* img_ptr : images) {
			cv::Mat img = img_ptr->mat();
			for (int y = 0; y < DIM_Y; y++) {
				for (int x = 0; x < DIM_X; x++) {
					cv::Vec3b pixel = (y < img.rows && x < img.cols) ? img.at<cv::Vec3b>(y, x) : cv::Vec3b(0, 0, 0);
					uchar pixval = round((pixel[0] + pixel[1] + pixel[2]) / 3);
					sum[y][x] += pixval;
					pixvals[y][x].push_back(pixval);
				}
			}
		}

		for (int y = 0; y < DIM_Y; y++) {
			for (int x = 0; x < DIM_X; x++) {
				// mean
				mean[y][x] = ((long double)sum[y][x]) / images.size();

				// variance
				var[y][x] = 0;
				for (uchar pixval : pixvals[y][x]) {
					var[y][x] += pow(pixval - mean[y][x], 2);
				}
				var[y][x] /= images.size();

				// max_with_same_pixel

				out << y << "," << x << "," << sum[y][x] << "," << mean[y][x] << "," << var[y][x] << endl;
			}
		}
		out.close();

		// find highest variance in 20x22 blocks
		//int block_width = 20;
		//int block_height = 22;
		// find highest variance in 10x10 blocks
		int block_width = 10;
		int block_height = 10;

		coords.clear();
		for (int y = 2; y < DIM_Y - block_height; y += block_height + 1) {
			for (int x = 2; x < DIM_X - block_width; x += block_width + 1) {
				HashCoord best;
				double high_var = 0;
				for (int r = y; r < y + block_height; r++) {
					for (int c = x; c < x + block_width; c++) {
						if (var[r][c] > high_var) {
							best.x = c;
							best.y = r;
							high_var = var[r][c];
						}
					}
				}
				coords.push_back(best);
				//cout << "(" << best.x << ", " << best.y << "): " << high_var << endl;
			}
		}

		// write high-variance coordinates
		out.open((dest.parent_path() / "analysis_coordinates.csv").string());
		out << endl << "High-Variance Coordinates" << endl;
		out << "y,x" << endl;

		for (HashCoord coord : coords)
			out << coord.y << "," << coord.x << endl;

		out.close();

		// hash images using high-variance coordinates and store collisions
		hashmap.clear();
		total_time = 0;
		for (int i = 0; i < images.size(); i++) {
			cv::Mat img = images[i]->mat();
			uint64 hash;

			start_time = clock();
			hash = FNV_Hash(img, coords);
			end_time = clock();
			total_time += end_time - start_time;
			hashmap[hash].insert(i);
		}
		avg_time = ((double)total_time) / images.size();

		// count collisions
		collisions = 0;
		for (pair<uint64, set<int>> hashset : hashmap) {
			collisions += hashset.second.size() - 1;
		}

		cout << "High-Var:  " << images.size() << " images; " << collisions << " collisions; ~" << avg_time << " ms per image" << endl;

		out << endl;
		out.close();
		delete[] sum;
		delete[] mean;
		delete[] var;
	}

	if (DO_LOW_MODE) {
		// find lowest mode_count in all collision images for LOW_MODE_COUNT iterations
		coords.clear();
		hashmap.clear();
		deque<int> low_modes;

		// get pixcounts for each possible x,y,RGB
		unsigned long* pixcounts = new unsigned long[DIM_Y * DIM_X * 3 * 256];
		for (int y = 0; y < DIM_Y; y++)
			for (int x = 0; x < DIM_X; x++)
				for (int c = 0; c < 3; c++)
					for (int p = 0; p < 256; p++)
						pixcounts[(y * DIM_X * 3 * 256) + (x * 3 * 256) + (c * 256) + p] = 0;

		for (image* img_ptr : images) {
			cv::Mat img = img_ptr->mat();
			for (int y = 0; y < DIM_Y; y++) {
				for (int x = 0; x < DIM_X; x++) {
					cv::Vec3b pixel = (y < img.rows && x < img.cols) ? img.at<cv::Vec3b>(y, x) : cv::Vec3b(0, 0, 0);
					for (int c = 0; c < 3; c++)
						pixcounts[(y * DIM_X * 3 * 256) + (x * 3 * 256) + (c * 256) + pixel[c]]++;
				}
			}
		}

		// find highest variance in 8x8 blocks - total of (128/9)^2 = 14^2 = 196 coordinates
		// find highest variance in 6x6 blocks - total of (128/7)^2 = 18^2 = 324 coordinates
		// generalization: total of (128/(bw+1))*(128/(bh+1)) coordinates
		int block_width = 6;
		int block_height = 6;

		for (int y = 1; y < DIM_Y - block_height; y += block_height + 1) {
			for (int x = 1; x < DIM_X - block_width; x += block_width + 1) {
				HashCoord best;
				unsigned long low_mode = ULONG_MAX;
				//long double low_mode = LDBL_MAX;
				for (int r = y; r < y + block_height; r++) {
					for (int c = x; c < x + block_width; c++) {
						// use overall mode
						unsigned long mode = *max_element(pixcounts + (r * DIM_X * 3 * 256) + (c * 3 * 256),
														  pixcounts + (r * DIM_X * 3 * 256) + (c * 3 * 256) + (3 * 256));
						// use RGB mode average
						//unsigned long mode_red = *max_element(pixcounts + (r * DIM_X * 3 * 256) + (c * 3 * 256),
						//									  pixcounts + (r * DIM_X * 3 * 256) + (c * 3 * 256) + 256);
						//unsigned long mode_green = *max_element(pixcounts + (r * DIM_X * 3 * 256) + (c * 3 * 256) + 256,
						//										pixcounts + (r * DIM_X * 3 * 256) + (c * 3 * 256) + 512);
						//unsigned long mode_blue	= *max_element(pixcounts + (r * DIM_X * 3 * 256) + (c * 3 * 256) + 512,
						//									   pixcounts + (r * DIM_X * 3 * 256) + (c * 3 * 256) + 768);
						//long double mode_avg = ((long double)(mode_red + mode_green + mode_blue)) / 3;
						if (mode < low_mode) {
						//if (mode_avg < low_mode) {
							best.x = c;
							best.y = r;
							low_mode = mode;
							//low_mode = mode_avg;
						}
					}
				}
				coords.push_back(best);
				low_modes.push_back(low_mode);
				//cout << "(" << best.x << ", " << best.y << "): " << high_var << endl;
			}
		}

		// write low mode count coordinates
		ofstream out;
		out.open((dest.parent_path() / "analysis_coordinates.csv").string());
		out << coords.size() << " Lowest |Mode| Coordinates" << endl;
		out << "x,y,mode" << endl;

		for (int i = 0; i < coords.size(); i++)
			out << coords[i].x << "," << coords[i].y << "," << low_modes[i] << endl;

		hashmap.clear();
		total_time = 0;
		for (int i = 0; i < images.size(); i++) {
			cv::Mat img = images[i]->mat();
			uint64 hash;

			start_time = clock();
			hash = Murmur2_Hash(img, coords);
			end_time = clock();
			total_time += end_time - start_time;
			hashmap[hash].insert(i);
		}
		avg_time = ((double)total_time) / images.size();

		// find the differing coordinates in images with hash collisions
		ofstream collout;
		collout.open((dest.parent_path() / "analysis_collisions.csv").string());
		collisions = 0;
		unique_hashes = 0;
		map<HashCoord, int> collision_coords;
		for (pair<uint64, set<int>> hashset : hashmap) {
			if (hashset.second.size() < 2) continue;
			unique_hashes++;
			collisions += hashset.second.size() - 1;

			// write collisions to collout
			collout << hashset.first;
			for (int i : hashset.second) {
				collout << "," << images[i]->path.stem().string() << " (" << Murmur2_Full(images[i]->mat()) << ")";
			}
			collout << endl;
		}
		cout << "Low-Mode (" << coords.size() << "): " << images.size() << " images; " << collisions << " collisions; " << unique_hashes << " hashes; ~" << avg_time << " ms per image" << endl;

		out << endl;
		out.close();
		collout.close();
		delete[] pixcounts;
	}

	if (DO_COLANLYZ) {
		// find the differing coordinates in images with hash collisions
		ofstream collout;
		collout.open((dest.parent_path() / "analysis_collisions.csv").string());
		collisions = 0;
		map<HashCoord, int> collision_coords;
		for (pair<uint64, set<int>> hashset : hashmap) {
			if (hashset.second.size() < 2) continue;

			set<int>::iterator iter = hashset.second.begin();
			while (iter != hashset.second.end()) {
				for (int j : hashset.second) {				// compare each image in the collision set to every other image in the collision set
					if (*iter == j) continue;				// (excluding itself)
					cv::Mat img1 = images[*iter]->mat();
					cv::Mat img2 = images[j]->mat();

					for (int y = 0; y < min(img1.rows, img2.rows); y++) {
						cv::Mat row1 = img1.row(y);
						cv::Mat row2 = img2.row(y);
						for (int x = 0; x < min(row1.cols, row2.cols); x++) {
							cv::Vec3b pixel1 = row1.at<cv::Vec3b>(0, x);
							cv::Vec3b pixel2 = row2.at<cv::Vec3b>(0, x);
							uchar val1 = round((pixel1[0] + pixel1[1] + pixel1[2]) / 3);
							uchar val2 = round((pixel2[0] + pixel2[1] + pixel2[2]) / 3);
							if (val1 != val2)
								collision_coords[{x, y}]++;		// increase collision count for differing pixels
						}
					}
				}
				hashset.second.erase(iter);
				iter = hashset.second.begin();
			}
		}

		// find most common collision coordinates
		// reverse collision_coords to sort coordinates by frequency
		map<int, set<HashCoord>, greater<int>> common_collisions;
		for (pair<HashCoord, int> collision : collision_coords) {
			common_collisions[collision.second].insert(collision.first);
		}

		ofstream out;
		out.open((dest.parent_path() / "analysis_coordinates.csv").string());
		out << "Frequently-Colliding Coordinates" << endl;
		out << "y,x,count" << endl;

		// add 44 most frequent collision_coords to coords
		int count = 0;
		map<int, set<HashCoord>, greater<int>>::iterator collision_iter = common_collisions.begin();
		while (collision_iter != common_collisions.end() && count < 88) {
			set<HashCoord>::iterator coord_iter = collision_iter->second.begin();
			while (coord_iter != collision_iter->second.end() && count < 44) {
				coords.push_back(*coord_iter);
				count++;

				out << coord_iter->y << "," << coord_iter->x << "," << collision_iter->first << endl;

				coord_iter++;
			}
			collision_iter++;
		}

		// hash images using frequently-colliding coords
		hashmap.clear();
		total_time = 0;
		for (int i = 0; i < images.size(); i++) {
			cv::Mat img = images[i]->mat();
			uint64 hash;

			start_time = clock();
			hash = FNV_Hash(img, coords);
			end_time = clock();
			total_time += end_time - start_time;
			hashmap[hash].insert(i);
		}
		avg_time = ((double)total_time) / images.size();

		// count collisions
		collisions = 0;
		for (pair<uint64, set<int>> hashset : hashmap) {
			collisions += hashset.second.size() - 1;
		}

		cout << "Freq-Coll (" << coords.size() << ") : " << images.size() << " images; " << collisions << " collisions; ~" << avg_time << " ms per image" << endl;
		out.close();
		collout.close();
	}

}


void Analyze_Collisions(fs::path analysis, fs::path dest)
{
	cout << "Running Collision Analysis..." << endl;
	cout << "Reading images...";

	deque<image*> images;
	unordered_set<uint64> image_hashes;
	get_images(analysis, images, image_hashes);

	cout << " found " << images.size() << " images." << endl;

	map<uint64, set<int>> hashmap;
	clock_t start_time, end_time, total_time;
	double avg_time;
	long collisions = 0;

	total_time = 0;
	for (int i = 0; i < images.size(); i++) {
		cv::Mat img = images[i]->mat();
		uint64 hash;

		start_time = clock();
		hash = Murmur2_Hash(img);
		end_time = clock();
		total_time += end_time - start_time;
		hashmap[hash].insert(i);
	}
	avg_time = ((double)total_time) / images.size();

	// find the differing coordinates in images with hash collisions
	ofstream collout;
	collout.open(dest.string());
	collisions = 0;
	map<HashCoord, int> collision_coords;
	long coll_pairs = 0;
	long total_differing = 0;
	long total_difference = 0;
	for (pair<uint64, set<int>> hashset : hashmap) {
		if (hashset.second.size() < 2) continue;
		collisions += hashset.second.size() - 1;

		// write collisions to collout
		collout << hashset.first;
		for (int i : hashset.second) {
			collout << "," << i << " - " << make_relative(analysis, images[i]->path.string()).string() << " (rect " << images[i]->rect.x << ", " << images[i]->rect.y  << " )";
		}
		collout << endl;

		set<int>::iterator iter2 = hashset.second.begin();
		set<int>::iterator iter = hashset.second.begin();
		set<int>::iterator end = hashset.second.end();
		end--;
		while (iter != end) {
			iter2++;
			cv::Mat img = images[*iter]->mat();
			while (iter2 != hashset.second.end()) {
				long differing = 0;
				long difference = 0;

				cv::Mat img2 = images[*iter2]->mat();
				deque<HashCoord> differing_coords;
				for (int y = 0; y < min(img.rows, img2.rows); y++) {
					for (int x = 0; x < min(img.cols, img2.cols); x++) {
						cv::Vec3b pixel = img.at<cv::Vec3b>(y, x);
						cv::Vec3b pixel2 = img2.at<cv::Vec3b>(y, x);
						for (int c = 0; c < 3; c++) {
							if (pixel[c] != pixel2[c]) {
								differing++;
								difference += abs(pixel[c] - pixel2[c]);
								differing_coords.push_back({ x, y, c });
							}
						}
					}
				}

				if (differing > 0) {
					coll_pairs++;

					// write comparison image to analysis\collisions
					cv::Mat comparison(max(img.rows, img2.rows), img.cols + img2.cols, img.type());
					img.copyTo(comparison(cv::Rect(0, 0, img.cols, img.rows)));
					img2.copyTo(comparison(cv::Rect(img.cols, 0, img2.cols, img2.rows)));
					string base_name = images[*iter]->path.stem().string() + images[*iter]->rect_char() + "_" +
						images[*iter2]->path.stem().string() + images[*iter2]->rect_char();

					string name = base_name + ".png";
					cv::imwrite(((analysis / "collisions") / name).string(), comparison);

					// write differing pixels to analysis\collisions
					cv::Mat differing_pixels(comparison.size(), comparison.type());
					differing_pixels.setTo(cv::Scalar(0, 0, 0));
					for (HashCoord differing_coord : differing_coords) {
						cv::Vec3b& pixel = differing_pixels.at<cv::Vec3b>(differing_coord.y, differing_coord.x);
						cv::Vec3b& pixel2 = differing_pixels.at<cv::Vec3b>(differing_coord.y, differing_coord.x + img.cols);
						switch (differing_coord.c) {
						case HashCoord::r: pixel[0] = pixel2[0] = 255; break;
						case HashCoord::g: pixel[1] = pixel2[1] = 255; break;
						case HashCoord::b: pixel[2] = pixel2[2] = 255; break;
						}
					}

					name = base_name + "_diff.png";
					cv::imwrite(((analysis / "collisions") / name).string(), differing_pixels);
				}

				stringstream ss;
				ss << *iter << " vs " << *iter2 << ": " << differing <<
					" differing pixels for a total of " << difference <<
					" and an average difference of " << ((differing == 0) ? 0 : ((double)difference) / differing);
				cout << ss.str() << endl;
				collout << ss.str() << endl;
				total_differing += differing;
				total_difference += difference;
				iter2++;
			}
			collout << endl;
			iter2 = ++iter;
		}
	}
	cout << "Low-Mode (" << COORDS_LEN << "):  " << images.size() << " images; " << collisions << " collisions in " << coll_pairs << " pairs; ~" << avg_time << " ms per image" << endl;
	cout << "Differing Pixels: " << total_differing << endl;
	cout << "Total Difference: " << total_difference << endl;
	cout << "Average Difference: " << ((total_differing == 0) ? 0 : (((double)total_difference) / total_differing)) << endl;
	cout << "Differing/CollPair: " << ((coll_pairs == 0) ? 0 : (((double)total_differing) / coll_pairs)) << endl;
	cout << "Difference/CollPair: " << ((coll_pairs == 0) ? 0 : (((double)total_difference) / coll_pairs)) << endl;
}

void Create_Hashmap(fs::path texture_dir, fs::path output_dir, bool append = false)
{
	fs::path hashmap_csv(output_dir / (texture_dir.filename().string() + "_hm.csv"));
	//fs::path collisions_dir(output_dir / (texture_dir.filename().string() + "_coll.csv"));

	ofstream out;
	unsigned int open_mode = ofstream::out;
	if (append) open_mode |= ofstream::app;
	out.open(hashmap_csv.string(), open_mode);

	fs::directory_iterator end;
	for (fs::directory_iterator iter(texture_dir); iter != end; iter++) {
		fs::path path = iter->path();
		if (fs::is_directory(path)) {
			// recursive
			Create_Hashmap(path, output_dir, true);
		} else if (fs::is_regular_file(path) && boost::iequals(path.extension().string(), ".bmp")) {
			cv::Mat img = cv::imread(path.string(), CV_LOAD_IMAGE_COLOR);
			uint64 hash_combined, hash_upper, hash_lower;
			hash_combined = FNV_Hash_Combined_64(img, hash_upper, hash_lower, COORDS, COORDS_LEN, true);
			out << path.stem().string() << "," << hash_combined << "," << hash_upper << "," << hash_lower << endl;
		}
	}
}

void Get_Blank_Hashes(cv::Mat& img)
{
	cv::Mat normal = img(cv::Rect(0, 0, 128, 256));
	cv::Mat nolower = normal.clone();
	cv::Mat noupper = normal.clone();

	for (int y = 0; y < 256; y++) {
		for (int x = 0; x < 128; x++) {
			if (y < 128) {
				noupper.data[noupper.step[0] * y + noupper.step[1] * x + 0] = 0;
				noupper.data[noupper.step[0] * y + noupper.step[1] * x + 1] = 0;
				noupper.data[noupper.step[0] * y + noupper.step[1] * x + 2] = 0;
			} else {
				nolower.data[nolower.step[0] * y + nolower.step[1] * x + 0] = 0;
				nolower.data[nolower.step[0] * y + nolower.step[1] * x + 1] = 0;
				nolower.data[nolower.step[0] * y + nolower.step[1] * x + 2] = 0;
			}
		}
	}

	uint64 upper, lower, combined;

	upper = lower = combined = 0;

	cout << "NORMAL:" << endl;
	combined = Murmur2_Hash(normal, COORDS, COORDS_LEN);

	cout << "  upper:        " << upper << endl;

	cout << "  lower:        " << lower << endl;

	cout << "  combined:     " << combined << endl;

	cout << "NO LOWER:" << endl;
	combined = Murmur2_Hash(nolower, COORDS, COORDS_LEN);

	cout << "  combined:     " << combined << endl;

	cout << "NO UPPER:" << endl;
	combined = Murmur2_Hash(noupper, COORDS, COORDS_LEN);

	cout << "  combined:     " << combined << endl;
}

void Test_Texture_Replacement()
{
	cv::Mat sel_13 = cv::imread("H:\\Game Saves\\Final Fantasy VIII\\Mods\\Berrymapper - Hash Textures\\BerryMapper\\INPUT\\sel_13.bmp", CV_LOAD_IMAGE_COLOR);
	cv::Mat wm_ocean = cv::imread("C:\\Program Files (x86)\\Steam\\steamapps\\common\\FINAL FANTASY VIII\\tonberry\\MCINDUS_Ocean2\\wm\\wm_oceancstlt\\wm_oceancstlt_13.png", CV_LOAD_IMAGE_COLOR);
	cv::Mat wm_ocean_upper = wm_ocean.rowRange(0, wm_ocean.rows / 2);
	cv::Mat wm_ocean_lower = wm_ocean.rowRange(wm_ocean.rows / 2, wm_ocean.rows);

	cv::Mat test(1024, 1024, CV_8UC3);
	// UPSCALE ORIGINAL TEXTURE
	for (int y = 0; y < 1024; y++) {
		for (int x = 0; x < 1024; x++) {
			cv::Vec3b pixel = sel_13.at<cv::Vec3b>(y / 4, x / 4);
			test.data[test.step[0] * y + test.step[1] * x + 0] = pixel[0];
			test.data[test.step[0] * y + test.step[1] * x + 1] = pixel[1];
			test.data[test.step[0] * y + test.step[1] * x + 2] = pixel[2];
		}
	}

	//cv::namedWindow("image", cv::WINDOW_AUTOSIZE);
	//cv::imshow("image", test);
	//cv::waitKey(0);
	cv::imwrite("img_testing\\upscaled.bmp", test);

	// UPSCALE ORIGINAL FLIPPED
	for (int y = 0; y < 1024; y++) {
		for (int x = 0; x < 1024; x++) {
			cv::Vec3b pixel = sel_13.at<cv::Vec3b>(sel_13.rows - 1 - y / 4, x / 4);
			test.data[test.step[0] * y + test.step[1] * x + 0] = pixel[0];
			test.data[test.step[0] * y + test.step[1] * x + 1] = pixel[1];
			test.data[test.step[0] * y + test.step[1] * x + 2] = pixel[2];
		}
	}

	//cv::namedWindow("image", cv::WINDOW_AUTOSIZE);
	//cv::imshow("image", test);
	//cv::waitKey(0);
	cv::imwrite("img_testing\\upscaled_flipped.bmp", test);

	// UPSCALE OCEAN UPPER
	for (int y = 0; y < 1024; y++) {
		for (int x = 0; x < 1024; x++) {
			cv::Vec3b pixel;
			if (y < 512)
				pixel = wm_ocean.at<cv::Vec3b>(y, x);
			else
				pixel = sel_13.at<cv::Vec3b>(y / 4, x / 4);
			test.data[test.step[0] * y + test.step[1] * x + 0] = pixel[0];
			test.data[test.step[0] * y + test.step[1] * x + 1] = pixel[1];
			test.data[test.step[0] * y + test.step[1] * x + 2] = pixel[2];
		}
	}

	//cv::namedWindow("image", cv::WINDOW_AUTOSIZE);
	//cv::imshow("image", test);
	//cv::waitKey(0);
	cv::imwrite("img_testing\\upper.bmp", test);

	// UPSCALE OCEAN UPPER FLIPPED
	cv::Mat* to_use;
	to_use = &wm_ocean;
	for (int y = 0; y < 1024; y++) {
		for (int x = 0; x < 1024; x++) {
			cv::Vec3b pixel;
			if (y < 512)
				pixel = sel_13.at<cv::Vec3b>(sel_13.rows - 1 - y / 4, x / 4);
			else {
				int repl_y = to_use->rows - 1 - y;
				if (repl_y < 0) repl_y += 512;
				pixel = to_use->at<cv::Vec3b>(repl_y, x);
			}
			test.data[test.step[0] * y + test.step[1] * x + 0] = pixel[0];
			test.data[test.step[0] * y + test.step[1] * x + 1] = pixel[1];
			test.data[test.step[0] * y + test.step[1] * x + 2] = pixel[2];
		}
	}

	cv::namedWindow("image", cv::WINDOW_AUTOSIZE);
	cv::imshow("image", test);
	cv::waitKey(0);
	cv::imwrite("img_testing\\full_upper_flipped.bmp", test);

	// UPSCALE OCEAN_UPPER FLIPPED
	to_use = &wm_ocean_upper;
	for (int y = 0; y < 1024; y++) {
		for (int x = 0; x < 1024; x++) {
			cv::Vec3b pixel;
			if (y < 512)
				pixel = sel_13.at<cv::Vec3b>(sel_13.rows - 1 - y / 4, x / 4);
			else {
				int repl_y = to_use->rows - 1 - y;
				if (repl_y < 0) repl_y += 512;
				pixel = to_use->at<cv::Vec3b>(repl_y, x);
			}
			test.data[test.step[0] * y + test.step[1] * x + 0] = pixel[0];
			test.data[test.step[0] * y + test.step[1] * x + 1] = pixel[1];
			test.data[test.step[0] * y + test.step[1] * x + 2] = pixel[2];
		}
	}

	cv::namedWindow("image", cv::WINDOW_AUTOSIZE);
	cv::imshow("image", test);
	cv::waitKey(0);
	cv::imwrite("img_testing\\half_upper_flipped.bmp", test);

	// UPSCALE OCEAN LOWER
	for (int y = 0; y < 1024; y++) {
		for (int x = 0; x < 1024; x++) {
			cv::Vec3b pixel;
			if (y < 512)
				pixel = sel_13.at<cv::Vec3b>(y / 4, x / 4);
			else
				pixel = wm_ocean.at<cv::Vec3b>(y, x);
			test.data[test.step[0] * y + test.step[1] * x + 0] = pixel[0];
			test.data[test.step[0] * y + test.step[1] * x + 1] = pixel[1];
			test.data[test.step[0] * y + test.step[1] * x + 2] = pixel[2];
		}
	}

	cv::namedWindow("image", cv::WINDOW_AUTOSIZE);
	cv::imshow("image", test);
	cv::waitKey(0);
	cv::imwrite("img_testing\\lower.bmp", test);

	// UPSCALE OCEAN LOWER FLIPPED
	to_use = &wm_ocean;
	for (int y = 0; y < 1024; y++) {
		for (int x = 0; x < 1024; x++) {
			cv::Vec3b pixel;
			if (y < 512) {
				int repl_y = to_use->rows - 1 - y;
				//if (repl_y < 0) repl_y += 512;
				pixel = to_use->at<cv::Vec3b>(repl_y, x);
			}
			else
				pixel = sel_13.at<cv::Vec3b>(sel_13.rows - 1 - y / 4, x / 4);
			test.data[test.step[0] * y + test.step[1] * x + 0] = pixel[0];
			test.data[test.step[0] * y + test.step[1] * x + 1] = pixel[1];
			test.data[test.step[0] * y + test.step[1] * x + 2] = pixel[2];
		}
	}

	cv::namedWindow("image", cv::WINDOW_AUTOSIZE);
	cv::imshow("image", test);
	cv::waitKey(0);
	cv::imwrite("img_testing\\full_lower_flipped.bmp", test);

	// UPSCALE OCEAN_LOWER FLIPPED
	to_use = &wm_ocean_lower;
	for (int y = 0; y < 1024; y++) {
		for (int x = 0; x < 1024; x++) {
			cv::Vec3b pixel;
			if (y < 512) {
				int repl_y = to_use->rows - 1 - y;
				//if (repl_y < 0) repl_y += 512;
				pixel = to_use->at<cv::Vec3b>(repl_y, x);
			} else
				pixel = sel_13.at<cv::Vec3b>(sel_13.rows - 1 - y / 4, x / 4);
			test.data[test.step[0] * y + test.step[1] * x + 0] = pixel[0];
			test.data[test.step[0] * y + test.step[1] * x + 1] = pixel[1];
			test.data[test.step[0] * y + test.step[1] * x + 2] = pixel[2];
		}
	}

	cv::namedWindow("image", cv::WINDOW_AUTOSIZE);
	cv::imshow("image", test);
	cv::waitKey(0);
	cv::imwrite("img_testing\\half_lower_flipped.bmp", test);
}

//-------------------------------------------------------------
// converts string to unsigned long long
uint64 StringToUint64(string s)
{
	uint64 sum = 0;

	for (int i = 0; i<s.length(); i++)
		sum = (sum * 10) + (s[i] - '0');

	return sum;
}

#include "..\D3D9CallbackSC2\src\BigInteger.h"

void get_hashmap(fs::path root, unordered_map<uint64, string>& hashmap)
{
	try {
		fs::directory_iterator iter(root), end;
		for (; iter != end; iter++) {
			fs::path path = iter->path();
			if (fs::is_directory(path)) {
				get_hashmap(path, hashmap);										// recursive
			} else if (fs::is_regular_file(path) &&
				(boost::iequals(path.extension().string(), ".csv"))) {
				ifstream in;
				in.open(path.string(), ifstream::in);
				string line = "";

				while (getline(in, line)) {
					string name = line.substr(0, line.find(','));
					string value = line.substr(line.find(',') + 1);
					uint64 hash;
					stringstream ss(value);
					ss >> hash;

					hashmap.insert(pair<uint64, string>(hash, name));
				}
			}
		}
	} catch (fs::filesystem_error e) {
		cout << e.what() << endl;
	}
}

void Test_Hash2_Collisions()
{
	unordered_map<uint64, string> hashmap;
	get_hashmap(FF8_ROOT / "tonberry\\hashmap", hashmap);

	unordered_map<uint64, string>::iterator iter = hashmap.begin();
	for (; iter != hashmap.end(); iter++) {
		cout << iter->first << " -> " << iter->second << endl;
	}

	cout << hashmap.size() << " hashes." << endl;

	ifstream in;
	in.open((FF8_ROOT / "tonberry\\hash2map.csv").string(), ifstream::in);
	string line;

	while (getline(in, line)) {
		string name = line.substr(0, line.find(','));
		string value = line.substr(line.find(',') + 1);

		uint64 hash2 = StringToUint64(value);

		cout << line << " -> " << hash2;
		if (hashmap.count(hash2) > 0) {
			cout << " COLLISION WITH " << hashmap[hash2];
		}
		cout << endl;
	}

}

void Copy_Unique(fs::path root, fs::path dest)
{
	cout << "Reading existing unique images...";

	deque<image*> existing, new_images;
	unordered_set<uint64> image_hashes;
	get_images(dest, existing, image_hashes, true);

	cout << " found " << existing.size() << " existing images." << endl;

	cout << "Reading new images...";
	get_images(root, new_images, image_hashes, false);

	cout << " found " << new_images.size() << " new images." << endl;
	cout << "Copying unique images...";

	uint64 successful = 0;

	for (image * img : new_images) {
		cv::Mat mat = img->mat();
		stringstream ss;
		ss << img->hash << ".bmp";
		string filename = ss.str();
		successful += cv::imwrite((dest / filename).string(), mat);
	}

	cout << " successfully wrote " << successful << " unique images to " << dest.string() << "." << endl;
}

void Find_Similar_Images(image base_img, fs::path root, unsigned max_diff, bool name_is_hash = false)
{
	cout << "Reading images...";

	deque<image*> images;
	unordered_set<uint64> image_hashes;
	get_images(root, images, image_hashes, name_is_hash);

	cout << " found " << images.size() << " images." << endl;
	cout << "Finding images that differ from " << base_img.path.filename() << " by 0 < x <= " << max_diff << " pixels:" << endl;;

	// create root/../similar
	fs::path sim_dir = root.parent_path() / "similar";
	if (!fs::exists(sim_dir))
		fs::create_directory(sim_dir);

	// create root/../similar/<stem>
	sim_dir /= base_img.path.stem().string();
	if (!fs::exists(sim_dir))
		fs::create_directory(sim_dir);

	// copy base_img to root/../similar/<stem>/<filename>
	fs::copy_file(base_img.path, sim_dir / base_img.path.filename(), fs::copy_option::overwrite_if_exists);

	cv::Mat base_mat = base_img.mat();
	unsigned similar = 0;

	for (image * img : images) {
		cv::Mat mat = img->mat();
		unsigned difference = 0;
		unsigned differing = 0;

		deque<HashCoord> diff_coords;
		for (int y = 0; y < DIM_Y && differing <= max_diff; y++) {
			for (int x = 0; x < DIM_X && differing <= max_diff; x++) {
				cv::Vec3b pixela = (y < base_mat.rows && x < base_mat.cols) ? base_mat.at<cv::Vec3b>(y, x) : cv::Vec3b(0, 0, 0);
				cv::Vec3b pixelb = (y < mat.rows && x < mat.cols) ? mat.at<cv::Vec3b>(y, x) : cv::Vec3b(0, 0, 0);
				bool diff = false;
				for (int c = 0; c < 3; c++) {
					if (pixela[c] != pixelb[c]) {
						diff = true;
						difference += abs(pixela[c] - pixelb[c]);
					}
				}
				if (diff) {
					differing++;
					diff_coords.push_back(HashCoord(x, y));
				}
			}
		}

		if (differing > 0 && differing <= max_diff) {
			similar++;
			fs::copy_file(img->path, sim_dir / img->path.filename(), fs::copy_option::overwrite_if_exists);
			cout << "  " << make_relative(root, img->path) << " differs by " << difference << " in " << differing << " pixels (" << (difference/differing) << "/pixel):" << endl;
			for (HashCoord coord : diff_coords) {
				cout << "    " << coord.string() << ": ";
				cv::Vec3b pixela = (coord.y < base_mat.rows && coord.x < base_mat.cols) ? base_mat.at<cv::Vec3b>(coord.y, coord.x) : cv::Vec3b(0, 0, 0);
				cv::Vec3b pixelb = (coord.y < mat.rows && coord.x < mat.cols) ? mat.at<cv::Vec3b>(coord.y, coord.x) : cv::Vec3b(0, 0, 0);
				cout << "(" << (ushort)pixela[0] << ", " << (ushort)pixela[1] << ", " << (ushort)pixela[2] << ") vs. ";
				cout << "(" << (ushort)pixelb[0] << ", " << (ushort)pixelb[1] << ", " << (ushort)pixelb[2] << ")" << endl;
			}
		}
	}

	cout << "Found " << similar << " similar images." << endl;
}

void Write_Unique_To_File(fs::path root, fs::path file)
{
	cout << "Reading images...";

	deque<image*> images;
	unordered_set<uint64> image_hashes;
	get_images(root, images, image_hashes, true);

	cout << " found " << images.size() << " images." << endl;
	cout << "Writing image paths to " << file.string() << "...";

	ofstream out;
	out.open(file.string().c_str());
	for (image * img : images) {
		out << img->hash << ';' << img->path << endl;
	}
	out.close();
	cout << " done!" << endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	// test Murmur2_Combined
	fs::path test_file("E:/Tonberry/Textures/Magic Animations/7167.bmp");
	//uint64 hash_ui, hash_li;
	uint64 hash_cn, hash_ucn, hash_lcn;
	uint64 hash_c, hash_uc, hash_lc;

	cv::Mat img = cv::imread(test_file.string());

	//hash_ui = Murmur2_Hash(img, COORDS, COORDS_LEN);

	//cout << "Upper (Individual):\t" << hash_ui << endl;

	hash_cn = Murmur2_Hash_Combined_Naive(img, hash_ucn, hash_lcn, COORDS, COORDS_LEN);

	cout << "Upper (Naive):\t\t" << hash_ucn << endl;
	cout << "Lower (Naive):\t\t" << hash_lcn << endl;
	cout << "Combined (Naive):\t" << hash_cn << endl;

	hash_c = Murmur2_Hash_Combined(img, hash_uc, hash_lc, COORDS, COORDS_LEN);

	cout << "Upper:\t\t\t" << hash_uc << endl;
	cout << "Lower:\t\t\t" << hash_lc << endl;
	cout << "Combined:\t\t" << hash_c << endl;
	cout << "Match " << ((hash_c == hash_cn) ? "yes" : "no") << endl;

	getchar();
	return 0;

	fs::path debug(FF8_ROOT / "tonberry\\debug");
	//fs::path analysis(debug / "analysis0");
	fs::path analysis("E:/Tonberry/Textures");
	fs::path unique("E:/Tonberry/unique");
	fs::path textures(FF8_ROOT / "textures");

	//Copy_Unique(analysis, unique);
	//Write_Unique_To_File(unique, unique.parent_path() / "unique.txt");
	//image quistis(unique / "14641241333983697.bmp", cv::Rect(0, 0, DIM_X, DIM_Y), 14641241333983697);
	//Find_Similar_Images(quistis, unique, 100, true);

	Analyze_Pixels(fs::path("E:/Tonberry/unique.txt"), unique.parent_path() / "object_analysis.csv", true);
	//Analyze_Collisions(analysis, debug / "collision_analysis.txt");
	//getchar();
	//return 0;

	//fs::path ocean2(FF8_ROOT + "tonberry\\MCINDUS_Ocean2");
	//Create_Hashmap(ocean2 / "original bitmaps", ocean2);
	//return 0;

	//Test_Hash2_Collisions();

	//cv::Mat sel_13 = cv::imread("H:\\Game Saves\\Final Fantasy VIII\\Mods\\Berrymapper - Hash Textures\\BerryMapper\\INPUT\\sel_13.bmp", CV_LOAD_IMAGE_COLOR);
	//cv::Mat sql_78 = cv::imread("H:\\Game Saves\\Final Fantasy VIII\\Mods\\Berrymapper - Hash Textures\\BerryMapper\\INPUT\\sql_78.bmp", CV_LOAD_IMAGE_COLOR);
	//cv::Mat zel_13 = cv::imread("H:\\Game Saves\\Final Fantasy VIII\\Mods\\Berrymapper - Hash Textures\\BerryMapper\\INPUT\\zel_13.bmp", CV_LOAD_IMAGE_COLOR);
	//Get_Blank_Hashes(sel_13);
	//Get_Blank_Hashes(sql_78);
	//Get_Blank_Hashes(zel_13);
	getchar();

	//Test_Texture_Replacement();
	return 0;
	
	deque<cv::Mat> images;
	deque<string> image_names;
	try {
		fs::directory_iterator iter_end;
		for (fs::directory_iterator debug_outer_dir(debug); debug_outer_dir != iter_end; debug_outer_dir++) {
			// [FFVIII]\tonberry\debug\*
			string dirname = debug_outer_dir->path().filename().string();
			// skip files and directories that end in 0
			if (!fs::is_directory(debug_outer_dir->path()) || dirname.substr(dirname.length() -1, 1) == "0") continue;
			for (fs::directory_iterator texture(debug_outer_dir->path()); texture != iter_end; texture++) {
				// [FFVIII]\tonberry\debug_outer_dir\*
				string tex_name = texture->path().string();
				//cout << tex_name << endl;
				cv::Mat img = cv::imread(tex_name, CV_LOAD_IMAGE_COLOR);
				if (img.cols < 1 || img.rows < 1) {
					//cout << "Skipping " << tex_name.substr(debug.string().length()) << " (" << img.cols << "x" << img.rows << ")" << endl;
					continue;
				}
				images.push_back(img);
				image_names.push_back(tex_name.substr(debug.string().length()));
			}
		}
	} catch (fs::filesystem_error e) {
		cout << e.what() << endl;
	}


	ofstream hashout, collisionout;
	hashout.open((debug / "hashes.csv").string());
	collisionout.open((debug / "collisions.csv").string());

	// hash images for collisions.csv output
	map<uint64, set<string>> hashmap;
	//map<string, set<string>> hashmap;
	for (int i = 0; i < images.size(); i++) {
		cv::Mat img = images[i];
		//uint64 hash = Old_Hash_Algorithm_1(img);
		uint64 hash = FNV_Hash(img, COORDS, COORDS_LEN);
		hashmap[hash].insert(image_names[i]);
	}

	// count collisions for collisions.csv output
	int collisions = 0;
	for (pair<uint64, set<string>> hashset : hashmap) {
		if (hashset.second.size() < 2) continue;
		collisions += hashset.second.size() - 1;
		collisionout << hashset.first;
		for (string tex : hashset.second) {
			collisionout << "," << tex;
		}
		collisionout << endl;
	}

	//
	// Algorithm Comparisons
	//

	// hash images using fast hash1 algorithm
	clock_t start_time, end_time, total_time = 0;
	double avg_time;
	hashmap.clear();
	total_time = 0;
	for (int i = 0; i < images.size(); i++) {
		cv::Mat img = images[i];
		start_time = clock();
		uint64 hash = Old_Hash_Algorithm_1(img);
		end_time = clock();
		total_time += end_time - start_time;
		hashmap[hash].insert(image_names[i]);
	}
	avg_time = ((double)total_time) / images.size();

	// count collisions
	collisions = 0;
	for (pair<uint64, set<string>> hashset : hashmap) {
		collisions += hashset.second.size() - 1;
	}

	cout << images.size() << " images; " << collisions << " collisions; ~" << avg_time << " ms per image" << endl;

	// hash images using fast hash1 algorithm
	hashmap.clear();
	total_time = 0;
	for (int i = 0; i < images.size(); i++) {
		cv::Mat img = images[i];
		start_time = clock();
		uint64 hash = Hash_Algorithm_1(img);
		end_time = clock();
		total_time += end_time - start_time;
		hashmap[hash].insert(image_names[i]);
	}
	avg_time = ((double)total_time) / images.size();

	// count collisions
	collisions = 0;
	for (pair<uint64, set<string>> hashset : hashmap) {
		collisions += hashset.second.size() - 1;
	}

	cout << images.size() << " images; " << collisions << " collisions; ~" << avg_time << " ms per image" << endl;

	// hash images using FNV algorithm with hash1 coords
	hashmap.clear();
	total_time = 0;
	for (int i = 0; i < images.size(); i++) {
		cv::Mat img = images[i];
		start_time = clock();
		uint64 hash = FNV_Hash(img, hash1, 64, false);
		end_time = clock();
		total_time += end_time - start_time;
		hashmap[hash].insert(image_names[i]);
	}
	avg_time = ((double)total_time) / images.size();

	// count collisions
	collisions = 0;
	for (pair<uint64, set<string>> hashset : hashmap) {
		collisions += hashset.second.size() - 1;
	}

	cout << images.size() << " images; " << collisions << " collisions; ~" << avg_time << " ms per image" << endl;

	// hash images using FNV algorithm with high-variance coords
	hashmap.clear();
	total_time = 0;
	for (int i = 0; i < images.size(); i++) {
		cv::Mat img = images[i];
		start_time = clock();
		uint64 hash = FNV_Hash(img, COORDS, COORDS_LEN, false);
		end_time = clock();
		total_time += end_time - start_time;
		hashmap[hash].insert(image_names[i]);
	}
	avg_time = ((double)total_time) / images.size();

	// count collisions
	collisions = 0;
	for (pair<uint64, set<string>> hashset : hashmap) {
		collisions += hashset.second.size() - 1;
	}

	cout << images.size() << " images; " << collisions << " collisions; ~" << avg_time << " ms per image" << endl;

	// hash images using FNV algorithm with high-variance coords AND frequently-colliding coords AND RGB averages
	hashmap.clear();
	total_time = 0;
	for (int i = 0; i < images.size(); i++) {
		cv::Mat img = images[i];
		start_time = clock();
		uint64 hash = FNV_Hash(img, COORDS, COORDS_LEN);
		end_time = clock();
		total_time += end_time - start_time;
		hashmap[hash].insert(image_names[i]);
	}
	avg_time = ((double)total_time) / images.size();

	// count collisions
	collisions = 0;
	for (pair<uint64, set<string>> hashset : hashmap) {
		collisions += hashset.second.size() - 1;
	}

	cout << images.size() << " images; " << collisions << " collisions; ~" << avg_time << " ms per image" << endl;

	//cv::Mat sel_13, sql_78, zel_13;
	//uint64 hash_sel_13, hash_sql_78, hash_zel_13;
	//sel_13 = cv::imread("H:\\Game Saves\\Final Fantasy VIII\\Mods\\Berrymapper - Hash Textures\\BerryMapper\\INPUT\\sel_13.bmp", CV_LOAD_IMAGE_COLOR);
	//sql_78 = cv::imread("H:\\Game Saves\\Final Fantasy VIII\\Mods\\Berrymapper - Hash Textures\\BerryMapper\\INPUT\\sql_78.bmp", CV_LOAD_IMAGE_COLOR);
	//zel_13 = cv::imread("H:\\Game Saves\\Final Fantasy VIII\\Mods\\Berrymapper - Hash Textures\\BerryMapper\\INPUT\\zel_13.bmp", CV_LOAD_IMAGE_COLOR);
	//hash_sel_13 = Hash_Algorithm_1(sel_13);
	//hash_sql_78 = Hash_Algorithm_1(sql_78);
	//hash_zel_13 = Hash_Algorithm_1(zel_13);
	//cout << hash_sel_13 << endl;
	//cout << hash_sql_78 << endl;
	//cout << hash_zel_13 << endl;

	//try {
	//	fs::directory_iterator iter_end;
	//	for (fs::directory_iterator tex_outer_dir(textures); tex_outer_dir != iter_end; tex_outer_dir++) {
	//		// [FFVIII]\textures\*
	//		if (!fs::is_directory(tex_outer_dir->path())) continue;
	//		for (fs::directory_iterator tex_inner_dir(tex_outer_dir->path()); tex_inner_dir != iter_end; tex_inner_dir++) {
	//			// [FFVIII]\textures\tex_inner_dir\*
	//			if (!fs::is_directory(tex_inner_dir->path())) continue;
	//			for (fs::directory_iterator files(tex_inner_dir->path()); files != iter_end; files++) {
	//				// [FFVIII]\textures\tex_outer_dir\tex_inner_dir\*.bmp
	//				string pathstring = files->path().string();
	//				cout << pathstring << endl;
	//			}
	//		}
	//	}
	//}
	//catch (fs::fs_error e) {
	//	cout << e.what() << endl;
	//}
	return 0;
}

//
// Old Algorithms
//

uint64_t Hash_Algorithm_slow(const cv::Mat& img)	//hash algorithm that preferences top and left sides
{
	uint64 hash = 0;
	float first_val, val = 0, last_val = 0;
	for (int i = 0; i < 64; i++) {
		int x, y;
		if (i < 32) {
			x = i / 4;
			y = i % 4;
		}
		else if (i < 56) {
			x = (i - 32) / 12;
			y = (i - 32) % 12 + 4;
		}
		else {
			x = (i - 56) / 4 * 3 + 3;
			y = (i - 56) % 4 * 3 + 5;
		}

		x *= BLOCKSIZE;
		y *= BLOCKSIZE;

		// get average pixel value
		if (x < img.cols && y < img.rows) {
			cv::Vec3b pixel = img.at<cv::Vec3b>(y, x);
			val = (pixel[0] + pixel[1] + pixel[2]) / 3;
		}
		else
			val = 0;

		// compare to last value for hash
		if (i > 0) {
			hash <<= 1;
			if (val >= last_val)
				hash |= 1;
		}
		else
			first_val = val;

		// do wrap-around comparison
		if (i == 63 && first_val >= val) {
			hash |= 0x8000000000000000;
		}

		last_val = val;
	}

	return hash;
}

uint64 Hash_Algorithm_1_lessslow(const cv::Mat& img)
{
	int x = 0, y = 0;
	float first_val, val = 0, last_val = 0;
	uint64 hash = 0;
	cv::Vec3b pixel;

	// first_val
	pixel = img.at<cv::Vec3b>(y, x);
	first_val = last_val = (pixel[0] + pixel[1] + pixel[2]) / 3;

	//int i = 0;
	//printf("%d:(%d,%d)\n", i++, x >> 4, y >> 4);

	for (y = 1; y < 4; y++) {
		//for (y = BLOCKSIZE; y < BLOCKSIZE << 2; y += BLOCKSIZE) { // pixvals 1->3

		if (y < img.rows) {
			pixel = img.at<cv::Vec3b>(y, x);
			val = (pixel[0] + pixel[1] + pixel[2]) / 3;
		}
		else
			val = 0;

		//printf("%d:(%d,%d)\n", i++, x >> 4, y >> 4);

		hash <<= 1;
		if (val >= last_val) hash++;
		last_val = val;
	}

	//for (x = 1; x < 8; x++)
	for (x = BLOCKSIZE; x < BLOCKSIZE << 3; x += BLOCKSIZE) // pixvals 4->31
	{
		//for (y = 0; y < 4; y++)
		for (y = 0; y < BLOCKSIZE << 2; y += BLOCKSIZE)
		{
			if (x < img.cols && y < img.rows) {
				pixel = img.at<cv::Vec3b>(y, x);
				val = (pixel[0] + pixel[1] + pixel[2]) / 3;
			}
			else
				val = 0;

			//printf("%d:(%d,%d)\n", i++, x >> 4, y >> 4);

			hash <<= 1;
			if (val >= last_val) hash++;
			last_val = val;
		}
	}

	//for (x = 0; x < 2; x++)
	for (x = 0; x < BLOCKSIZE << 1; x += BLOCKSIZE) // pixvals 32->55
	{
		// for (y = 4; y < 16; y++)
		for (y = BLOCKSIZE << 2; y < BLOCKSIZE << 4; y += BLOCKSIZE)
		{
			if (x < img.cols && y < img.rows) {
				pixel = img.at<cv::Vec3b>(y, x);
				val = (pixel[0] + pixel[1] + pixel[2]) / 3;
			}
			else
				val = 0;

			//printf("%d:(%d,%d)\n", i++, x >> 4, y >> 4);

			hash <<= 1;
			if (val >= last_val) hash++;
			last_val = val;
		}
	}

	//for (x = 3; x <= 6; x += 3)
	for (x = (BLOCKSIZE << 1) + BLOCKSIZE; x <= (BLOCKSIZE << 1) + (BLOCKSIZE << 2); x += (BLOCKSIZE << 1) + BLOCKSIZE) // pixvals 56->63, note +=3
	{
		//for (y = 5; y < 15; y += 3)
		for (y = (BLOCKSIZE << 2) + BLOCKSIZE; y <= (BLOCKSIZE << 1) + (BLOCKSIZE << 2) + (BLOCKSIZE << 3); y += (BLOCKSIZE << 1) + BLOCKSIZE) // note +=3
		{
			if (x < img.cols && y < img.rows) {
				pixel = img.at<cv::Vec3b>(y, x);
				val = (pixel[0] + pixel[1] + pixel[2]) / 3;
			}
			else
				val = 0;

			//printf("%d:(%d,%d)\n", i++, x >> 4, y >> 4);

			hash <<= 1;
			if (val >= last_val) hash++;
			last_val = val;
		}
	}

	if (first_val >= last_val) hash |= 0x8000000000000000;

	return hash;
}