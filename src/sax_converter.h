/*
 * sax_converter.h
 *
 *  Created on: 6 Jul 2016
 *      Author: thachln
 */

#ifndef SAX_CONVERTER_H_
#define SAX_CONVERTER_H_


#include <iostream>
#include <cmath>
#include <string>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <ctime>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
//#include "common.h"

class SAX
{
private:

	double *break_points;
	bool divisible;


	int MAX;
	int numerosity_reduction;

public:
	int window_size;
	int step_size;
	int word_length;
	int alphabet_size;
	// numerosity reduction strategy
	static const int NONE_NR = 0;
	static const int BACK2BACK_NR = 1;
	static const int UNIQUE_SET_NR = 2;

	static const int TEST_NORMALIZE = 1;


	// Constructor
	SAX(){
		step_size = 1;
		MAX = 10000;
		window_size = 167;
		word_length = 8;
		alphabet_size = 7;
		compute_break_points();
		if (window_size % word_length == 0){
			divisible = true;
		} else {
			divisible = false;
		}
		numerosity_reduction = NONE_NR;
	}

	SAX(int w, int a){
		init(-1,w,a,1,NONE_NR);
	}

	SAX(int N, int w, int a){
		init(N,w,a,1,BACK2BACK_NR);
		if (window_size % word_length == 0){
			divisible = true;
		} else {
			divisible = false;
		}

	}

	SAX(int N, int w, int a, int nr_strategy){
		init(N,w,a,1,nr_strategy);
		if (window_size % word_length == 0){
			divisible = true;
		} else {
			divisible = false;
		}

	}

	SAX(int N, int w, int a, int _step_size, int nr_strategy){
		init(N,w,a,_step_size,nr_strategy);
		if (window_size % word_length == 0){
			divisible = true;
		} else {
			divisible = false;
		}

	}

	// Destructor
	~SAX(){

		free(break_points);
	}

	void init(int N, int w, int a, int _step_size, int nr_strategy){
		window_size = N;
		word_length = w;
		alphabet_size = a;
		step_size = _step_size;
		numerosity_reduction = nr_strategy;
		MAX = 10000;
		compute_break_points();
	}

	void string_to_numeric_vector(char* timeseries, char* delimiter, std::vector<double>& numeric_ts){
		// end pointer of the timeseries char array
		char *stre = timeseries + strlen (timeseries);
		// end pointer of the delimiter char array
		char *dele = delimiter + strlen (delimiter);
		int size = 0;
		while (++size < MAX) {
			// find the first delimiter
			char *n = std::find_first_of (timeseries, stre, delimiter, dele);

			numeric_ts.push_back(atof(timeseries));
			if (n == stre) break;
			timeseries = n + 1;
		}
	}

	std::vector<double> timeseries_to_PAA(std::vector<double> &timeseries){
		std::vector<double> PAA;

		//normalization
		double mean_ts = 0.0;
		double var_ts = 0.0;
		double sum_ts = 0.0;
		double sumsq_ts = 0.0;

		for (auto v: timeseries){
			sum_ts += v;
			sumsq_ts += v*v;
		}
		mean_ts = sum_ts / timeseries.size();
		var_ts = sumsq_ts / timeseries.size() - mean_ts*mean_ts;

		int paai = 0;
		int scaled_index = 0;
		PAA.push_back(0);
		for (int i = 0; i < timeseries.size();i++){
			double normed = (timeseries[i] - mean_ts)/sqrt(var_ts);
			//std::cout << normed << " ";
			int count = word_length;
			while(scaled_index < timeseries.size() && count > 0){
				PAA[paai] += normed;
				scaled_index++;
				count--;
			}
			if (scaled_index == timeseries.size()){
				scaled_index = 0;
				PAA[paai] = PAA[paai] / timeseries.size();
				PAA.push_back(0);
				paai++;
			}
			if (count > 0){
				while(scaled_index < timeseries.size() && count > 0){
					PAA[paai] += normed;
					scaled_index++;
					count--;
				}
			}
		}
		PAA.pop_back();
		//std::cout<< std::endl;

		//for (auto v: PAA){
		//	std::cout << v << " ";
		//}
		//std::cout<< std::endl;

		return PAA;
	}

	// compute split
	double entropy(int begin, int end, std::vector<std::pair<double,double>> &sorted_list){
		std::map<double,int> frequencies;
		double entropy = 0;
		for (int i = begin; i <= end; i++){
			frequencies[sorted_list[i].second]++;
		}
		for (std::pair<double,int> p : frequencies){
			double freq = static_cast<double>(p.second)/(end - begin + 1);
			entropy += freq * log2(freq);
		}
		entropy *= -1;
		return entropy;
	}


	void compute_split(int begin, int end, std::vector<std::pair<double,double>> &sorted_list, int &cut_index, int depth = 0){
		//std::cout << "Find split number " << cut_index << std::endl;

		//double p_entropy = entropy(begin,end,sorted_list);
		double min_entropy = 9999;
		int split_index = 0;

		for (int i = begin;i < end;i++){
			double current_entropy = ((i-begin+1)*entropy(begin,i,sorted_list)/(end-begin + 1) + (end-i)*entropy(i+1,end,sorted_list)/(end - begin +1));
			if (min_entropy > current_entropy){
				split_index = i;
				min_entropy = current_entropy;
				//	std::cout << "Update entropy : " << min_entropy << " and current split: " << split_index << std::endl;
			}
		}
		double split = (sorted_list[split_index].first + sorted_list[split_index + 1].first) / 2;
		//std::cout << split_index << " : " << min_entropy << " : " << split << std::endl;
		break_points[cut_index] = split;
		cut_index++;
		if (cut_index >= (alphabet_size - 1) || depth >= (log2(alphabet_size) - 1)){
			return;
		}
		compute_split(begin, split_index, sorted_list,cut_index,depth+1);
		if (cut_index >= (alphabet_size - 1) || depth >= (log2(alphabet_size) - 1)){
			return;
		}
		compute_split(split_index + 1, end, sorted_list,cut_index,depth+1);

	}

	// Test: compute breaks based on information gain
	void compute_ig_break_points(std::string traindata){


		std::ifstream myfile (traindata);

		// vector contains all Piecewise Aggregate Approximation (aka. means of segments) values
		//std::vector<double> points;
		//std::vector<double> labels;
		std::vector<std::pair<double,double>> flatten_ts;

		std::string line;
		//std::cout << "Start reading from file:" << std::endl;
		while(std::getline(myfile,line)){

			std::istringstream iss(line);
			std::vector<double> ts;

			double label,tmp;
			iss >> label;

			// for z-normalization
			//double mean_ts = 0.0;
			//double var_ts = 0.0;
			//double sum_ts = 0.0;
			//double sumsq_ts = 0.0;

			while(iss >> tmp){
				//flatten_ts.push_back(std::pair<double,double>(tmp,label));
				ts.push_back(tmp);
				//sum_ts += tmp;
				//sumsq_ts += tmp*tmp;
				//std::cout << ts.size() << std::endl;
				if(ts.size() == window_size){
					for ( auto v: timeseries_to_PAA(ts)){
						//flatten_ts.push_back(std::pair<double,double>((v - mean_ts)/sqrt(var_ts),label));
						flatten_ts.push_back(std::pair<double,double>(v,label));
					}
					ts.erase(ts.begin());
				}


			}

			//mean_ts = sum_ts / ts.size();
			//var_ts = sumsq_ts / ts.size() - mean_ts*mean_ts;


		}
		//std::cout << flatten_ts.size() << std::endl;
		sort(flatten_ts.begin(),flatten_ts.end());

		//for(auto item : flatten_ts) {
		//        std::cout << item.first << std::endl;
		//}

		// find splits
		break_points = (double*)malloc (sizeof(double)*(alphabet_size-1));
		int begin = 0;
		int end = flatten_ts.size() - 1;
		//int cut_index = (alphabet_size - 1) / 2;
		int cut_index = 0;


		compute_split(0,flatten_ts.size() - 1, flatten_ts,cut_index);
		std::sort(break_points,break_points + alphabet_size - 1);


		//std::cout << break_points[0] << " " << break_points[1] << " " << break_points[2] << std::endl;


		// allocate memory for break_points


	}

	void compute_PAA_equidepth_break_points_from_file(std::string traindata, char* delimiter){
		int MAXSIZE = 100000;
		int max_line = 20;

		char *del = " ";
		char *line = new char[MAXSIZE];
		std::ifstream myfile (traindata);

		// vector contains all Piecewise Aggregate Approximation (aka. means of segments) values
		std::vector<double> PAAs;



		int line_no = 0;

		// read first max_line lines and push the values to a vector
		if (myfile.is_open()){
			while (myfile.getline (line, MAXSIZE) && ++line_no < max_line){
				std::vector<double> numeric_ts;
				// time series begin after the first delimiter as the first value is the label
				char *timeseries = std::strchr(line,*del)+1;

				//				char *label = (char*) malloc(timeseries - line);
				//				std::strncpy(label,line,timeseries-line-1);
				//				label[timeseries - line-1] = '\0';
				// push the time series to the double vector
				string_to_numeric_vector(timeseries, delimiter, numeric_ts);

				// convert time series to PAA
				for (int cur_pos = 0; cur_pos < numeric_ts.size() - window_size + 1; cur_pos += step_size){
					int window_end = cur_pos + window_size - 1;

					// calculate mean and std
					double mean_wd = 0.0;
					double var_wd = 0.0;
					double sum_wd = 0.0;
					double sumsq_wd = 0.0;
					for (int i = cur_pos; i <= window_end; i++){
						sum_wd += timeseries[i];
						sumsq_wd += timeseries[i]*timeseries[i];
					}


					mean_wd = sum_wd / window_size;
					var_wd = sumsq_wd / window_size - mean_wd*mean_wd;
					//std_wd = sqrt(var_wd / window_size - mean_wd*mean_wd);

					// z-normalize
					// padding data for the indivisible-length time series
					double subsection[window_size*word_length];
					for (int i = cur_pos; i <= window_end; i++){
						double normalized_value;
						if (TEST_NORMALIZE){
							normalized_value = (timeseries[i] - mean_wd);
							if (var_wd > 0 && !isNearlyEqualToZero(var_wd)){
								normalized_value = normalized_value / sqrt(var_wd);
							}
						} else {
							normalized_value = timeseries[i];
						}
						//	std::cout << normalized_value << std::endl;
						//				}
						PAAs.push_back(normalized_value);
						for (int j = (i - cur_pos)*word_length;j < (i - cur_pos)*word_length + word_length; j++){
							subsection[j] = normalized_value;
						}
					}


					for (int i = 0; i < word_length; i++){
						double PAA = 0.0;
						for (int j = window_size*i; j < window_size*(i + 1); j++){
							PAA += subsection[j];
						}
						//PAAs.push_back(PAA / window_size);

					}
				}



			}
			myfile.close();

		} else {
			std::cout << "Invalid File Input." << std::endl;
			return;
		}


		// allocate memory for break_points
		break_points = (double*)malloc (sizeof(double)*(alphabet_size-1));

		// sort the data
		double swap;
		for (int i = 0; i < (PAAs.size() - 1); i++){
			for (int j = 0; j < PAAs.size() - i - 1; j++){
				if (PAAs[j] > PAAs[j+1]){
					swap = PAAs[j];
					PAAs[j] = PAAs[j + 1];
					PAAs[j+1] = swap;
				}
			}
		}


		// find the beak points
		double step = PAAs.size()*1.0/alphabet_size;
		double pos = step;
		int a = 0;
		while (pos < PAAs.size()-1){
			break_points[a] = (PAAs[int(pos)] + PAAs[int(pos) + 1])/2;
			std::cout << break_points[a] << std::endl;
			a++;
			pos += step;
		}

	}
	// TEST: train equidepth break points
	// only read the first 10 lines from data
	void compute_equidepth_break_points_from_file(std::string traindata, char* delimiter){
		int MAXSIZE = 100000;
		int max_line = 20;

		char *del = " ";
		char *line = new char[MAXSIZE];
		std::ifstream myfile (traindata);

		std::vector<double> numeric_ts;


		int line_no = 0;

		// read first max_line lines and push the values to a vector
		if (myfile.is_open()){
			while (myfile.getline (line, MAXSIZE) && ++line_no < max_line){
				// time series begin after the first delimiter as the first value is the label
				char *timeseries = std::strchr(line,*del)+1;

				//				char *label = (char*) malloc(timeseries - line);
				//				std::strncpy(label,line,timeseries-line-1);
				//				label[timeseries - line-1] = '\0';
				// push the time series to the double vector
				string_to_numeric_vector(timeseries, delimiter, numeric_ts);


			}
			myfile.close();

		} else {
			std::cout << "Invalid File Input." << std::endl;
			return;
		}


		// allocate memory for break_points
		break_points = (double*)malloc (sizeof(double)*(alphabet_size-1));

		// sort the data
		double swap;
		for (int i = 0; i < (numeric_ts.size() - 1); i++){
			for (int j = 0; j < numeric_ts.size() - i - 1; j++){
				if (numeric_ts[j] > numeric_ts[j+1]){
					swap = numeric_ts[j];
					numeric_ts[j] = numeric_ts[j + 1];
					numeric_ts[j+1] = swap;
				}
			}
		}

		// normalize data

		double sum_ts = 0.0;
		double sumsq_ts = 0.0;

		for (int i = 0; i < numeric_ts.size(); i++){
			sum_ts += numeric_ts[i];
			sumsq_ts += numeric_ts[i]*numeric_ts[i];
		}
		double mean_ts = sum_ts / numeric_ts.size();
		double var_ts = sumsq_ts / numeric_ts.size() - mean_ts*mean_ts;
		for (int i = 0; i < numeric_ts.size(); i++){
			numeric_ts[i] = (numeric_ts[i] - mean_ts) / sqrt(var_ts);
		}



		// find the beak points
		double step = numeric_ts.size()*1.0/alphabet_size;
		double pos = step;
		int a = 0;
		while (pos < numeric_ts.size()-1){
			break_points[a] = (numeric_ts[int(pos)] + numeric_ts[int(pos) + 1])/2;
			std::cout << break_points[a] << std::endl;
			a++;
			pos += step;
		}

	}

	/*
	void compute_equidepth_break_points(char* timeseries, char* delimiter){
		char *stre = timeseries + strlen (timeseries);
		char *dele = delimiter + strlen (delimiter);
		int size = 1;
		double swap;
		std::vector<double> numeric_ts;
		while (size < MAX) {
			//std::cout << atof(timeseries) << std::endl;
			char *n = std::find_first_of (timeseries, stre, delimiter, dele);
			//std::cout << atof(timeseries) << std::endl;
			numeric_ts.push_back(atof(timeseries));
			++size;
			if (n == stre) break;
			timeseries = n + 1;
		}
		// ??
		if (window_size < 0){
			window_size = numeric_ts.size();
		}

		break_points = (double*)malloc (sizeof(double)*(alphabet_size-1));

		for (int i = 0; i < (numeric_ts.size() - 1); i++){
			for (int j = 0; j < numeric_ts.size() - i - 1; j++){
				if (numeric_ts[j] > numeric_ts[j+1]){
					swap = numeric_ts[j];
					numeric_ts[j] = numeric_ts[j + 1];
					numeric_ts[j+1] = swap;
				}
			}
		}
		//for (int i = 0; i < numeric_ts.size(); i++){
		//	std::cout << numeric_ts[i] << " ";
		//}
		std::cout << std::endl;
		double step = numeric_ts.size()*1.0/alphabet_size;
		double pos = step;
		int a = 0;
		while (pos < numeric_ts.size()-1){
			break_points[a] = (numeric_ts[int(pos)] + numeric_ts[int(pos) + 1])/2;
			a++;
			pos += step;
		}
	}
	 */
	void compute_break_points(){
		break_points = (double*)malloc (sizeof(double)*(alphabet_size-1));

		//double bps[alphabet_size - 1];
		switch (alphabet_size)
		{
		case 2:{
			double bps[1] = { 0.0 };
			std::copy(bps,bps + alphabet_size - 1, break_points);
			break;
		}
		case 3:{
			double bps[2] = { -0.430727299295, 0.430727299295 };
			std::copy(bps,bps + alphabet_size - 1, break_points);
			break;
		}
		case 4:{
			double bps[3] = { -0.674489750196, 0.0, 0.674489750196 };
			std::copy(bps,bps + alphabet_size - 1, break_points);
			break;
		}
		case 5:{
			double bps[4] = { -0.841621233573, -0.253347103136, 0.253347103136, 0.841621233573 };
			std::copy(bps,bps + alphabet_size - 1, break_points);
			break;
		}
		case 6:{
			double bps[5] = { -0.967421566102, -0.430727299295, 0.0, 0.430727299295, 0.967421566102 };
			std::copy(bps,bps + alphabet_size - 1, break_points);
			break;
		}
		case 7:{
			double bps[6] = { -1.06757052388, -0.565948821933, -0.180012369793, 0.180012369793, 0.565948821933, 1.06757052388 };
			std::copy(bps,bps + alphabet_size - 1, break_points);
			break;}
		case 8:{
			double bps[7] = { -1.15034938038, -0.674489750196, -0.318639363964, 0.0, 0.318639363964, 0.674489750196, 1.15034938038 };
			std::copy(bps,bps + alphabet_size - 1, break_points);
			break;}
		case 9:{
			double bps[8] = { -1.22064034885, -0.764709673786, -0.430727299295, -0.139710298882, 0.139710298882, 0.430727299295, 0.764709673786, 1.22064034885 };
			std::copy(bps,bps + alphabet_size - 1, break_points);
			break;}
		case 10:{
			double bps[9] = { -1.28155156554, -0.841621233573, -0.524400512708, -0.253347103136, 0.0, 0.253347103136, 0.524400512708, 0.841621233573, 1.28155156554 };
			std::copy(bps,bps + alphabet_size - 1, break_points);
			break;}
		case 11:{
			double bps[10] = { -1.33517773612, -0.908457868537, -0.604585346583, -0.348755695517, -0.114185294321, 0.114185294321, 0.348755695517, 0.604585346583, 0.908457868537, 1.33517773612 };
			std::copy(bps,bps + alphabet_size - 1, break_points);
			break;}
		case 12:{
			double bps[11] = { -1.3829941271, -0.967421566102, -0.674489750196, -0.430727299295, -0.210428394248, 0.0, 0.210428394248, 0.430727299295, 0.674489750196, 0.967421566102, 1.3829941271 };
			std::copy(bps,bps + alphabet_size - 1, break_points);
			break;}
		case 13:{
			double bps[12] = { -1.42607687227, -1.02007623279, -0.736315917376, -0.502402223373, -0.293381232121, -0.0965586152896, 0.0965586152896, 0.293381232121, 0.502402223373, 0.736315917376, 1.02007623279, 1.42607687227 };
			std::copy(bps,bps + alphabet_size - 1, break_points);
			break;}
		case 14:{
			double bps[13] = { -1.46523379269, -1.06757052388, -0.791638607743, -0.565948821933, -0.366106356801, -0.180012369793, 0.0, 0.180012369793, 0.366106356801, 0.565948821933, 0.791638607743, 1.06757052388, 1.46523379269 };
			std::copy(bps,bps + alphabet_size - 1, break_points);
			break;}
		case 15:{
			double bps[14] = { -1.50108594604, -1.11077161664, -0.841621233573, -0.62292572321, -0.430727299295, -0.253347103136, -0.0836517339071, 0.0836517339071, 0.253347103136, 0.430727299295, 0.62292572321, 0.841621233573, 1.11077161664, 1.50108594604 };
			std::copy(bps,bps + alphabet_size - 1, break_points);
			break;}
		case 16:{
			double bps[15] = { -1.53412054435, -1.15034938038, -0.887146559019, -0.674489750196, -0.488776411115, -0.318639363964, -0.15731068461, 0.0, 0.15731068461, 0.318639363964, 0.488776411115, 0.674489750196, 0.887146559019, 1.15034938038, 1.53412054435 };
			std::copy(bps,bps + alphabet_size - 1, break_points);
			break;}
		case 17:{
			double bps[16] = {-1.56, -1.19, -0.93, -0.72, -0.54, -0.38, -0.22, -0.07, 0.07, 0.22, 0.38, 0.54, 0.72, 0.93, 1.19, 1.56};
			std::copy(bps,bps + alphabet_size - 1, break_points);
			break;}
		case 18:{
			double bps[17] = {-1.59, -1.22, -0.97, -0.76, -0.59, -0.43, -0.28, -0.14, 0.0, 0.14, 0.28, 0.43, 0.59, 0.76, 0.97, 1.22, 1.59};
			std::copy(bps,bps + alphabet_size - 1, break_points);
			break;}
		case 19:{
			double bps[18] = {-1.62, -1.25, -1.0, -0.8, -0.63, -0.48, -0.34, -0.2, -0.07, 0.07, 0.2, 0.34, 0.48, 0.63, 0.8, 1.0, 1.25, 1.62};
			std::copy(bps,bps + alphabet_size - 1, break_points);
			break;}
		case 20:{
			double bps[19] = {-1.64, -1.28, -1.04, -0.84, -0.67, -0.52, -0.39, -0.25, -0.13, 0.0, 0.13, 0.25, 0.39, 0.52, 0.67, 0.84, 1.04, 1.28, 1.64};
			std::copy(bps,bps + alphabet_size - 1, break_points);
			break;}
		default:
			std::cout << "" << std::endl;
			break;
		}
		/*
17{ -1.56472647136, -1.18683143276, -0.928899491647, -0.721522283982, -0.541395085129, -0.377391943829, -0.22300783094, -0.0737912738083, 0.0737912738083, 0.22300783094, 0.377391943829, 0.541395085129, 0.721522283982, 0.928899491647, 1.18683143276, 1.56472647136 };
18{ -1.59321881802, -1.22064034885, -0.967421566102, -0.764709673786, -0.58945579785, -0.430727299295, -0.282216147063, -0.139710298882, 0.0, 0.139710298882, 0.282216147063, 0.430727299295, 0.58945579785, 0.764709673786, 0.967421566102, 1.22064034885, 1.59321881802 };
{ -1.61985625864, -1.25211952027, -1.00314796766, -0.80459638036, -0.63364000078, -0.479505653331, -0.336038140372, -0.199201324789, -0.0660118123758, 0.0660118123758, 0.199201324789, 0.336038140372, 0.479505653331, 0.63364000078, 0.80459638036, 1.00314796766, 1.25211952027, 1.61985625864 };
{ -1.64485362695, -1.28155156554, -1.03643338949, -0.841621233573, -0.674489750196, -0.524400512708, -0.385320466408, -0.253347103136, -0.125661346855, 0.0, 0.125661346855, 0.253347103136, 0.385320466408, 0.524400512708, 0.674489750196, 0.841621233573, 1.03643338949, 1.28155156554, 1.64485362695 };
{ -1.66839119395, -1.30917171679, -1.06757052388, -0.876142849247, -0.712443032389, -0.565948821933, -0.430727299295, -0.302980448056, -0.180012369793, -0.0597170997853, 0.0597170997853, 0.180012369793, 0.302980448056, 0.430727299295, 0.565948821933, 0.712443032389, 0.876142849247, 1.06757052388, 1.30917171679, 1.66839119395 };
{ -1.69062162958, -1.33517773612, -1.09680356209, -0.908457868537, -0.747858594763, -0.604585346583, -0.472789120992, -0.348755695517, -0.229884117579, -0.114185294321, 0.0, 0.114185294321, 0.229884117579, 0.348755695517, 0.472789120992, 0.604585346583, 0.747858594763, 0.908457868537, 1.09680356209, 1.33517773612, 1.69062162958 };
{ -1.71167530651, -1.35973738394, -1.12433823157, -0.938814316877, -0.781033811523, -0.640666889919, -0.511936213871, -0.391196258189, -0.275921063108, -0.164210777079, -0.0545189148481, 0.0545189148481, 0.164210777079, 0.275921063108, 0.391196258189, 0.511936213871, 0.640666889919, 0.781033811523, 0.938814316877, 1.12433823157, 1.35973738394, 1.71167530651 };
		 */


	}

	bool isNearlyEqualToZero(double x)
	{
		const double epsilon = 0; /* some small number such as 1e-5 */;
		return std::abs(x) <= epsilon;
	}

	std::vector<std::string> timeseries2SAX(std::vector<double> &timeseries){

		char alphabet[26] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};;
		// length of the time series
		int ts_length = timeseries.size();
		std::vector<std::string>  sax_seqc;
		// sliding windows

		for (int cur_pos = 0; cur_pos < ts_length - window_size + 1; cur_pos += step_size){
			//std::cout << ts_length << std::endl;
			int window_end = cur_pos + window_size - 1;

			// calculate mean and std
			double mean_wd = 0.0;
			double var_wd = 0.0;
			double sum_wd = 0.0;
			double sumsq_wd = 0.0;
			for (int i = cur_pos; i <= window_end; i++){
				sum_wd += timeseries[i];
				sumsq_wd += timeseries[i]*timeseries[i];
			}

			//			if (cur_pos == 0){
			//				for (int i = cur_pos; i <= window_end; i++){
			//					sum_wd += timeseries[i];
			//					sumsq_wd += timeseries[i]*timeseries[i];
			//				}
			//			} else {
			//				sum_wd += timeseries[window_end] - timeseries[cur_pos - 1];
			//				sumsq_wd += timeseries[window_end]*timeseries[window_end] - timeseries[cur_pos - 1]*timeseries[cur_pos - 1];
			//			}
			mean_wd = sum_wd / window_size;
			var_wd = sumsq_wd / window_size - mean_wd*mean_wd;
			//std_wd = sqrt(var_wd / window_size - mean_wd*mean_wd);

			// z-normalize
			// padding data for the indivisible-length time series
			double subsection[window_size*word_length];
			for (int i = cur_pos; i <= window_end; i++){
				double normalized_value;
				if (TEST_NORMALIZE){
					normalized_value = (timeseries[i] - mean_wd);
					if (var_wd > 0 && !isNearlyEqualToZero(var_wd)){
						normalized_value = normalized_value / sqrt(var_wd);
					}
				} else {
					normalized_value = timeseries[i];
				}
				//	std::cout << normalized_value << std::endl;
				//				}

				for (int j = (i - cur_pos)*word_length;j < (i - cur_pos)*word_length + word_length; j++){
					subsection[j] = normalized_value;
				}
			}
			//char *column[10];
			//	tokenize (example, del, column, 2);
			//	for (int i = 0; i < 5;i++){
			//		std::cout << column[i] << std::endl;
			//	}(
			// to characters
			std::string sax_word = "";
			for (int i = 0; i < word_length; i++){
				double PAA = 0.0;
				int bin = 0;
				for (int j = window_size*i; j < window_size*(i + 1); j++){
					PAA += subsection[j];
				}
				PAA = PAA / window_size;
				for (int j = 0; j < alphabet_size - 1;j++){
					if (PAA >= break_points[j]){
						bin++;
					}
				}
				sax_word += alphabet[bin];
			}

			//sax_seqc += "";

			switch(numerosity_reduction){
			case BACK2BACK_NR:
				if (sax_seqc.empty() || (sax_seqc.back() != sax_word)){
					sax_seqc.push_back(sax_word);
				}
				break;
			case UNIQUE_SET_NR:
			{
				bool first_appear = true;
				for (int i = 0; i < sax_seqc.size();i++){
					if (sax_seqc[i] == sax_word){
						first_appear = false;
						break;
					}
				}
				if (first_appear){
					sax_seqc.push_back(sax_word);
				}
				break;
			}
			default: // also NONE_NR
				sax_seqc.push_back(sax_word);
				break;
			}
		}
		return sax_seqc;
	}

	std::vector<std::string> timeseries2SAX(std::vector<double> &timeseries, char char_start){

		//char alphabet[26] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};;
		// length of the time series
		int ts_length = timeseries.size();
		std::vector<std::string>  sax_seqc;
		// sliding windows

		for (int cur_pos = 0; cur_pos < ts_length - window_size + 1; cur_pos += step_size){
			//std::cout << ts_length << std::endl;
			int window_end = cur_pos + window_size - 1;

			// calculate mean and std
			double mean_wd = 0.0;
			double var_wd = 0.0;
			double sum_wd = 0.0;
			double sumsq_wd = 0.0;
			for (int i = cur_pos; i <= window_end; i++){
				sum_wd += timeseries[i];
				sumsq_wd += timeseries[i]*timeseries[i];
			}

			//			if (cur_pos == 0){
			//				for (int i = cur_pos; i <= window_end; i++){
			//					sum_wd += timeseries[i];
			//					sumsq_wd += timeseries[i]*timeseries[i];
			//				}
			//			} else {
			//				sum_wd += timeseries[window_end] - timeseries[cur_pos - 1];
			//				sumsq_wd += timeseries[window_end]*timeseries[window_end] - timeseries[cur_pos - 1]*timeseries[cur_pos - 1];
			//			}
			mean_wd = sum_wd / window_size;
			var_wd = sumsq_wd / window_size - mean_wd*mean_wd;
			//std_wd = sqrt(var_wd / window_size - mean_wd*mean_wd);

			// z-normalize
			// padding data for the indivisible-length time series
			double subsection[window_size*word_length];
			for (int i = cur_pos; i <= window_end; i++){
				double normalized_value;
				if (TEST_NORMALIZE){
					normalized_value = (timeseries[i] - mean_wd);
					if (var_wd > 0 && !isNearlyEqualToZero(var_wd)){
						normalized_value = normalized_value / sqrt(var_wd);
					}
				} else {
					normalized_value = timeseries[i];
				}
				//	std::cout << normalized_value << std::endl;
				//				}

				for (int j = (i - cur_pos)*word_length;j < (i - cur_pos)*word_length + word_length; j++){
					subsection[j] = normalized_value;
				}
			}
			//char *column[10];
			//	tokenize (example, del, column, 2);
			//	for (int i = 0; i < 5;i++){
			//		std::cout << column[i] << std::endl;
			//	}(
			// to characters
			std::string sax_word = "";
			for (int i = 0; i < word_length; i++){
				double PAA = 0.0;
				int bin = 0;
				for (int j = window_size*i; j < window_size*(i + 1); j++){
					PAA += subsection[j];
				}
				PAA = PAA / window_size;
				for (int j = 0; j < alphabet_size - 1;j++){
					if (PAA >= break_points[j]){
						bin++;
					}
				}
				sax_word += char_start + bin;
			}

			//sax_seqc += "";

			switch(numerosity_reduction){
			case BACK2BACK_NR:
				if (sax_seqc.empty() || (sax_seqc.back() != sax_word)){
					sax_seqc.push_back(sax_word);
				}
				break;
			case UNIQUE_SET_NR:
			{
				bool first_appear = true;
				for (int i = 0; i < sax_seqc.size();i++){
					if (sax_seqc[i] == sax_word){
						first_appear = false;
						break;
					}
				}
				if (first_appear){
					sax_seqc.push_back(sax_word);
				}
				break;
			}
			default: // also NONE_NR
				sax_seqc.push_back(sax_word);
				break;
			}
		}
		return sax_seqc;
	}

	char* timeseries2SAX(char* timeseries, char* delimiter){

		char *stre = timeseries + strlen (timeseries);
		char *dele = delimiter + strlen (delimiter);
		int size = 1;
		std::vector<double> numeric_ts;
		while (size < MAX) {
			//std::cout << atof(timeseries) << std::endl;
			char *n = std::find_first_of (timeseries, stre, delimiter, dele);
			//std::cout << atof(timeseries) << std::endl;
			numeric_ts.push_back(atof(timeseries));
			++size;
			if (n == stre) break;
			timeseries = n + 1;
		}
		if (window_size < 0){
			window_size = numeric_ts.size();
		}
		//numeric_ts.push_back(atof(timeseries));

		std::vector<std::string> sax = timeseries2SAX(numeric_ts);


		unsigned int length_of_return = (word_length+1)*sax.size();
		char *return_array = (char*) malloc(sizeof(char)*length_of_return);
		for (unsigned int i = 0; i < sax.size(); i++){
			//std::cout << sax[i] << " ";
			for (int j = 0; j < word_length;j++){
				return_array[i*(word_length+1) + j] = sax[i][j];
			}
			return_array[i*(word_length +1)+word_length] = ' ';
		}
		return_array[length_of_return-1] = '\0';

		return return_array;
	}


	std::vector<std::string> timeseries2vectorSAX(std::string timeseries, std::string delimiter){

		std::vector<double> numeric_ts;
		size_t pos = 0;
		std::string token;

		while ((pos = timeseries.find(delimiter)) != std::string::npos) {
			token = timeseries.substr(0, pos);
			//std::cout << token << " ";
			numeric_ts.push_back(atof(token.c_str()));
			timeseries.erase(0, pos + delimiter.length());
		}
		if (!timeseries.empty()){
			numeric_ts.push_back(atof(timeseries.c_str()));
		}

		//std::vector<std::string> sax = timeseries2SAX(numeric_ts);
		return timeseries2SAX(numeric_ts);
	}

	std::vector<std::string> timeseries2vectorSAX(std::string timeseries, std::string delimiter, char char_start){

		std::vector<double> numeric_ts;
		size_t pos = 0;
		std::string token;

		while ((pos = timeseries.find(delimiter)) != std::string::npos) {
			token = timeseries.substr(0, pos);
			//std::cout << token << " ";
			numeric_ts.push_back(atof(token.c_str()));
			timeseries.erase(0, pos + delimiter.length());
		}
		if (!timeseries.empty()){
			numeric_ts.push_back(atof(timeseries.c_str()));
		}

		//std::vector<std::string> sax = timeseries2SAX(numeric_ts);
		return timeseries2SAX(numeric_ts, char_start);
	}


	std::string timeseries2SAX(std::string timeseries, std::string delimiter){
		std::string return_sax_str = "";
		//				std::vector<double> numeric_ts;
		//				size_t pos = 0;
		//				std::string token;
		//
		//				while ((pos = timeseries.find(delimiter)) != std::string::npos) {
		//				    token = timeseries.substr(0, pos);
		//				    //std::cout << token << " ";
		//				    numeric_ts.push_back(atof(token.c_str()));
		//				    timeseries.erase(0, pos + delimiter.length());
		//				}
		//				if (!timeseries.empty()){
		//					numeric_ts.push_back(atof(timeseries.c_str()));
		//				}

		std::vector<std::string> sax = timeseries2vectorSAX(timeseries, delimiter);
		//				std::vector<std::string> sax = timeseries2SAX(numeric_ts);

		for (std::string ss:sax){
			return_sax_str = return_sax_str + ss + " ";
		}
		return_sax_str.pop_back();

		return return_sax_str;


	}

	// concatenate multiple SAX representation
	std::string timeseries2multiSAX(std::string timeseries, std::string delimiter, int min_ws, int max_ws, double step){
		std::string return_sax_str = "";
		char char_start = 'a';

		for (int ws = min_ws; ws <= max_ws; ws += step){
			window_size = ws;
			for (std::string ss:timeseries2vectorSAX(timeseries, delimiter,char_start)){
				return_sax_str = return_sax_str + ss + " ";
			}
			//char_start += alphabet_size;

		}
		return_sax_str.pop_back();
		return return_sax_str;
	}

	// find the pattern in the symbolic representation of the time series
	// add corresponding score to return vector
	// return vector has the same length as the time series
	void detect_patterns(std::vector<double> timeseries,std::string pattern, double pt_score, std::vector<double> &accu_score){

		//accu_score.resize(timeseries.size());
		//std::fill(accu_score.begin(), accu_score.end(), 0.0);

		double break_points[4] = { -0.674489750196, 0.0, 0.674489750196 };


		char alphabet[26] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};;
		// length of the time series
		int ts_length = timeseries.size();
		std::vector<std::string>  sax_seqc;
		// sliding windows

		for (int cur_pos = 0; cur_pos < ts_length - window_size + 1; cur_pos += step_size){
			//std::cout << ts_length << std::endl;
			int window_end = cur_pos + window_size - 1;

			// calculate mean and std
			double mean_wd = 0.0;
			double var_wd = 0.0;
			double sum_wd = 0.0;
			double sumsq_wd = 0.0;
			for (int i = cur_pos; i <= window_end; i++){
				sum_wd += timeseries[i];
				sumsq_wd += timeseries[i]*timeseries[i];
			}


			mean_wd = sum_wd / window_size;
			var_wd = sumsq_wd / window_size - mean_wd*mean_wd;
			//std_wd = sqrt(var_wd / window_size - mean_wd*mean_wd);

			// z-normalize
			// padding data for the indivisible-length time series
			double subsection[window_size*word_length];
			for (int i = cur_pos; i <= window_end; i++){
				double normalized_value;
				if (TEST_NORMALIZE){
					normalized_value = (timeseries[i] - mean_wd);
					if (var_wd > 0 && !isNearlyEqualToZero(var_wd)){
						normalized_value = normalized_value / sqrt(var_wd);
					}
				} else {
					normalized_value = timeseries[i];
				}
				//	std::cout << normalized_value << std::endl;
				//				}

				for (int j = (i - cur_pos)*word_length;j < (i - cur_pos)*word_length + word_length; j++){
					subsection[j] = normalized_value;
				}
			}

			// to characters
			std::string sax_word = "";
			for (int i = 0; i < word_length; i++){
				double PAA = 0.0;
				int bin = 0;
				for (int j = window_size*i; j < window_size*(i + 1); j++){
					PAA += subsection[j];
				}
				PAA = PAA / window_size;
				for (int j = 0; j < alphabet_size - 1;j++){
					if (PAA >= break_points[j]){
						bin++;
					}
				}
				sax_word += alphabet[bin];
			}

			//sax_seqc += "";
			// NOTE: should be replaced with more efficient string matching algorithm

			size_t pos = 0;
			//std::cout << sax_word << ":" << cur_pos << std::endl;
			while(pos < word_length){
				pos = sax_word.find(pattern,pos);

				if (pos == std::string::npos){
					break;
				} else {
					//std::cout << "found:" << pattern << " at " << pos << std::endl;
					double stt = cur_pos + pos*window_size*1.0/word_length; // starting pst of the subsequence in the time series
					double end = stt + pattern.size()*window_size*1.0/word_length - 1; // end pst of the subsequence in the time series
					int ceil_stt = std::ceil(stt);
					int fl_end = std::floor(end);
					//std::cout << sax_word.size() << ":" << pattern.size() << std::endl;
					//std::cout << ceil_stt << ":" << fl_end << std::endl;
					//if (fl_end >= accu_score.size()){
					//	std::cout<< stt << " " << end << " ";
					//	std::cout << ceil_stt << " " << fl_end << " ";
					//}
					for (int j = ceil_stt; j <= fl_end;j++){
						accu_score[j] += pt_score; // add score for the subsequence of time series
						//if (accu_score[j] > 1000)
						//		std::cout << accu_score[j] << " ";
					}
					if (ceil_stt > 0){
						accu_score[ceil_stt - 1] += (ceil_stt - stt)*pt_score; // add score to the leftmost of the subsequence
					}
					if(fl_end < timeseries.size() - 1){
						accu_score[fl_end + 1] += (end - fl_end)*pt_score; // add score to the rightmost of the subsequence
					}
					pos++;
				}
			}





		}


		//return accu_score;
	}

	// find the pattern in the symbolic representation of the time series
	// add corresponding score to return vector
	// return vector has the same length as the time series
	// normalize score: i.e. spread the score over the found segments
	void detect_patterns_and_normalize_score(std::vector<double> timeseries,std::string pattern, double pt_score, std::vector<double> &accu_score){

		//accu_score.resize(timeseries.size());
		//std::fill(accu_score.begin(), accu_score.end(), 0.0);

		double break_points[4] = { -0.674489750196, 0.0, 0.674489750196 };


		char alphabet[26] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};;
		// length of the time series
		int ts_length = timeseries.size();
		std::vector<std::string>  sax_seqc;
		// sliding windows
		std::set<int> marked; // store positions in time series corresponded to the pattern

		for (int cur_pos = 0; cur_pos < ts_length - window_size + 1; cur_pos += step_size){
			//std::cout << ts_length << std::endl;
			int window_end = cur_pos + window_size - 1;

			// calculate mean and std
			double mean_wd = 0.0;
			double var_wd = 0.0;
			double sum_wd = 0.0;
			double sumsq_wd = 0.0;
			for (int i = cur_pos; i <= window_end; i++){
				sum_wd += timeseries[i];
				sumsq_wd += timeseries[i]*timeseries[i];
			}


			mean_wd = sum_wd / window_size;
			var_wd = sumsq_wd / window_size - mean_wd*mean_wd;
			//std_wd = sqrt(var_wd / window_size - mean_wd*mean_wd);

			// z-normalize
			// padding data for the indivisible-length time series
			double subsection[window_size*word_length];
			for (int i = cur_pos; i <= window_end; i++){
				double normalized_value;
				if (TEST_NORMALIZE){
					normalized_value = (timeseries[i] - mean_wd);
					if (var_wd > 0 && !isNearlyEqualToZero(var_wd)){
						normalized_value = normalized_value / sqrt(var_wd);
					}
				} else {
					normalized_value = timeseries[i];
				}
				//	std::cout << normalized_value << std::endl;
				//				}

				for (int j = (i - cur_pos)*word_length;j < (i - cur_pos)*word_length + word_length; j++){
					subsection[j] = normalized_value;
				}
			}

			// to characters
			std::string sax_word = "";
			for (int i = 0; i < word_length; i++){
				double PAA = 0.0;
				int bin = 0;
				for (int j = window_size*i; j < window_size*(i + 1); j++){
					PAA += subsection[j];
				}
				PAA = PAA / window_size;
				for (int j = 0; j < alphabet_size - 1;j++){
					if (PAA >= break_points[j]){
						bin++;
					}
				}
				sax_word += alphabet[bin];
			}

			//sax_seqc += "";
			// NOTE: should be replaced with more efficient string matching algorithm

			size_t pos = 0;
			//std::cout << sax_word << ":" << cur_pos << std::endl;
			while(pos < word_length){
				pos = sax_word.find(pattern,pos);

				if (pos == std::string::npos){
					break;
				} else {
					//std::cout << "found:" << pattern << " at " << pos << std::endl;
					double stt = cur_pos + pos*window_size*1.0/word_length; // starting pst of the subsequence in the time series
					double end = stt + pattern.size()*window_size*1.0/word_length - 1; // end pst of the subsequence in the time series
					int ceil_stt = std::ceil(stt);
					int fl_end = std::floor(end);
					//std::cout << sax_word.size() << ":" << pattern.size() << std::endl;
					//std::cout << ceil_stt << ":" << fl_end << std::endl;
					//if (fl_end >= accu_score.size()){
					//	std::cout<< stt << " " << end << " ";
					//	std::cout << ceil_stt << " " << fl_end << " ";
					//}
					for (int j = ceil_stt; j <= fl_end;j++){
						marked.insert(j);
						//accu_score[j] += pt_score; // add score for the subsequence of time series
						//if (accu_score[j] > 1000)
						//		std::cout << accu_score[j] << " ";
					}
					if (ceil_stt > 0){
						marked.insert(ceil_stt - 1);
						//accu_score[ceil_stt - 1] += (ceil_stt - stt)*pt_score; // add score to the leftmost of the subsequence
					}
					if(fl_end < timeseries.size() - 1){
						marked.insert(fl_end + 1);
						//accu_score[fl_end + 1] += (end - fl_end)*pt_score; // add score to the rightmost of the subsequence
					}
					pos++;
				}
			}





		}

		for (int pos : marked){
			accu_score[pos] += pt_score / marked.size();
		}

		//return accu_score;
	}

	void printBreakPoints(){
		for (int i = 0; i < alphabet_size - 1; i++){
			std::cout << break_points[i] << std::endl;
		}
	}

};



#endif /* SAX_CONVERTER_H_ */
