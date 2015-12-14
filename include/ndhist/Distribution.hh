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
  // TODO: make copy and assignment operator work
  Distribution(Distribution&) = delete;
  Distribution& operator=(Distribution) = delete;

  double get(const std::map<std::string, double>&);
private:
  IBinner* m_binner;
  std::vector<double> m_values;
};

#endif
