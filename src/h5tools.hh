#ifndef H5_TOOLS_HH
#define H5_TOOLS_HH

namespace H5 {
  class CompType;
}

// axis structure which is safe to store in HDF5
struct H5Axis {
  const char* name;
  int n_bins;
  double min;
  double max;
  const char* units;
};

H5::CompType get_axis_type();

#endif
