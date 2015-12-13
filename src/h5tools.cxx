#include "h5tools.hh"

#include "H5Cpp.h"

// setup data type
H5::CompType get_axis_type() {
  auto stype = H5::StrType(H5::PredType::C_S1, H5T_VARIABLE);
  stype.setCset(H5T_CSET_UTF8);
  auto dtype = H5::PredType::NATIVE_DOUBLE;
  auto itype = H5::PredType::NATIVE_INT;

  H5::CompType type(sizeof(H5Axis));
  type.insertMember("name",   HOFFSET(H5Axis, name  ), stype);
  type.insertMember("n_bins", HOFFSET(H5Axis, n_bins), itype);
  type.insertMember("min",    HOFFSET(H5Axis, min   ), dtype);
  type.insertMember("max",    HOFFSET(H5Axis, max   ), dtype);
  type.insertMember("units",  HOFFSET(H5Axis, units ), stype);
  return type;
}
