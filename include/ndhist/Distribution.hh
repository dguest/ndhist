#ifndef DISTRIBUTION_HH
#define DISTRIBUTION_HH

#include <map>
#include <string>
#include <vector>

// _________________________________________________________________________
// forward declares
namespace H5 {
  class DataSet;
}
class IBinner;

// _________________________________________________________________________
class Distribution
{
public:
  Distribution(const H5::DataSet& ds);
  ~Distribution();

  Distribution(const Distribution&);
  Distribution(Distribution&&);
  Distribution& operator=(Distribution);
  void swap(Distribution&);

  double get(const std::map<std::string, double>&) const;
private:
  IBinner* m_binner;
  std::vector<double> m_values;
};

void swap(Distribution&, Distribution&);

#endif
