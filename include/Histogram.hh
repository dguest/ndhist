#ifndef HISTOGRAM_H
#define HISTOGRAM_H

namespace hist { 
  // Histogram::fill(...) will throw a std::range_error if it gets nan
  // set this flag to simply count the number of nan. 
  const unsigned eat_nan = 1u << 0; 
}

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
  Histogram(int n_bins, double low, double high, std::string units = "", 
	    unsigned flags = 0); 
  Histogram(const std::vector<Axis>&, unsigned flags = 0); 
  Histogram(const Histogram&); 
  Histogram& operator=(Histogram); 
  ~Histogram(); 
  void fill(const std::map<std::string, double>&, double weight = 1); 
  void fill(const std::vector<double>&, double weight = 1); 
  void fill(double value, double weight = 1); 
  void write_to(H5::CommonFG& file, std::string name, int deflate = 7) const; 
private: 
  typedef std::vector<Axis> Axes;
  void init(const std::vector<Axis>&, unsigned); 
  template<typename T> void safe_fill(T, double); 
  void dim_atr(H5::DataSet& target, unsigned number, const Axis& dim) const; 
  int get_chunk_size(int) const; 
  void check_dimensions(const std::vector<Axis>&); 
  IBinner* m_binner; 
  std::vector<Axis> m_dimsensions; 
  std::vector<double> m_values; 
  std::vector<int> m_chunking; 
  int m_n_nan; 
  bool m_eat_nan; 
}; 

#endif //HISTOGRAM_H
