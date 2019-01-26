#ifndef MPC_H
#define MPC_H

#include <vector>
#include "Eigen-3.3/Eigen/Core"

class MPC {
 public:
  MPC();

  virtual ~MPC();

  // Solve the model given an initial state and polynomial coefficients.
  // Return the first actuations.
 //no & or const in video and maybe causing index error
  //std::vector<double> Solve(const Eigen::VectorXd &state,  const Eigen::VectorXd &coeffs);
  std::vector<double> Solve(Eigen::VectorXd state,  Eigen::VectorXd coeffs);
};

#endif  // MPC_H
