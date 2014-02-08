#ifndef HISTOGRAM_H
#define HISTOGRAM_H

namespace hist { 
  // Histogram::fill(...) will throw a std::range_error if it gets nan
  // set this flag to simply count the number of nan. 
  const unsigned eat_nan = 1u << 0; 

  // Save a second histogram weighted by weight*2. (for statistical error)
  // Second histogram has default name <name>Wt2. 
  const unsigned wt2     = 1u << 1; 

  // Save using the "old" <axis name>_<property> attribute naming scheme. 
  // (the "new" way saves arrays of values, keyed by <property>, 
  // indexed by the axis number)
  const unsigned flat_attributes = 1u << 2; 
}

namespace H5 { 
  class CommonFG; 
  class DataSet; 
} 

#include <vector>
#include <string> 
#include <map>
#include <initializer_list>

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
  Histogram(const std::initializer_list<Axis>&, unsigned flags = 0); 
  Histogram(const std::vector<Axis>&, unsigned flags = 0); 
  Histogram(const Histogram&); 
  Histogram& operator=(Histogram); 
  ~Histogram(); 
  friend void swap(Histogram&, Histogram&); 
  void fill(const std::map<std::string, double>&, double weight = 1); 
  void fill(const std::vector<double>&, double weight = 1); 
  void fill(const std::initializer_list<double>&, double weight = 1); 
  void fill(double value, double weight = 1); 
  void set_wt_ext(const std::string& ext); 
  void write_to(H5::CommonFG& file, 
		const std::string& name, int deflate = 7) const; 
private: 
  typedef std::vector<Axis> Axes;
  void write_internal(
    H5::CommonFG& file, const std::string& name, int deflate, 
    const std::vector<double>& values) const; 
  template<typename T> void safe_fill(T, double); 
  int get_chunk_size(int) const; 
  IBinner* m_binner; 
  std::vector<Axis> m_dimsensions; 
  std::vector<double> m_values; 
  std::vector<int> m_chunking; 
  int m_n_nan; 
  bool m_eat_nan; 
  bool m_old_serialization; 
  std::vector<double>* m_wt2; 
  std::string m_wt2_ext; 
}; 

#endif //HISTOGRAM_H
