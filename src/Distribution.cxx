#include "Distribution.hh"

#include "H5Cpp.h"

#include "h5tools.hh"
#include "Binners.hh"

#include <cassert>

Distribution::Distribution(const H5::DataSet& ds):
  m_binner(0)
{
  const H5::DataSpace space = ds.getSpace();
  int n_dims = space.getSimpleExtentNdims();
  const H5::Attribute axes_attr = ds.openAttribute("axes");
  assert(n_dims == axes_attr.getSpace().getSimpleExtentNpoints());

  // read in axes
  std::vector<H5Axis> axes(n_dims);
  axes_attr.read(get_axis_type(), axes.data());

  // reverse iterate through axes to build binners
  auto itr = axes.rbegin();
  int n_values = itr->n_bins + 2;
  m_binner = new LinBinner(itr->name, itr->n_bins, itr->min, itr->max);
  itr++;
  for (; itr != axes.rend(); itr++) {
    m_binner->add_dimension(
      new LinBinner(itr->name, itr->n_bins, itr->min, itr->max));
    n_values *= (itr->n_bins + 2);
  }

  // read in values
  m_values = std::vector<double>(n_values, 0);
  ds.read(m_values.data(), H5::PredType::NATIVE_DOUBLE);
}

Distribution::~Distribution() {
  delete m_binner;
}

double Distribution::get(const std::map<std::string, double>& point) {
  return m_values.at(m_binner->get_bin(point));
}