#ifndef HISTOGRAM_H
#define HISTOGRAM_H

namespace H5 { 
  class CommonFG; 
  class DataSet; 
} 

#include <vector>
#include <string> 
#include <map>


class IBinner; 

struct Axis
{
  std::string name;
  int n_bins; 
  double low; 
  double high; 
  std::string units; 
};


class Histogram
{
public: 
  Histogram(int n_bins, double low, double high, std::string units = ""); 
  Histogram(const std::vector<Axis>&); 
  Histogram(const Histogram&); 
  Histogram& operator=(const Histogram&); 
  ~Histogram(); 
  void fill(const std::map<std::string, double>, double weight = 1); 
  void fill(const std::vector<double>&, double weight = 1); 
  void fill(std::vector<double>&, double weight = 1); 
  void fill(double value, double weight = 1); 
  void write_to(H5::CommonFG& file, std::string name) const; 
private: 
  void init(const std::vector<Axis>&); 
  void dim_atr(H5::DataSet& target, unsigned number, const Axis& dim) const; 
  int get_chunk_size(int) const; 
  IBinner* m_binner; 
  std::vector<Axis> m_dimsensions; 
  std::vector<double> m_values; 
  std::vector<int> m_chunking; 
}; 

#endif //HISTOGRAM_H
