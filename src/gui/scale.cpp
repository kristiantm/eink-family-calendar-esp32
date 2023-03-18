#include "scale.hpp"

int scale::x_resMtp(double x, double x_res)
{
  const double x_resolution_defined = 640.0; 
  return (double)x * (x_res/x_resolution_defined);
}

int scale::y_resMtp(double y, double y_res)
{
  const double y_resolution_defined = 384.0;
  return (double)y * (y_res/y_resolution_defined);
}