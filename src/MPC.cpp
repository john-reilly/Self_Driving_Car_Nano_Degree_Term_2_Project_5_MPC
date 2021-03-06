





#include "MPC.h"
#include <cppad/cppad.hpp>
#include <cppad/ipopt/solve.hpp>
#include <iostream>
#include <string>
#include <vector>
#include "Eigen-3.3/Eigen/Core"

using CppAD::AD;
using Eigen::VectorXd;

/**
 * TODO: Set the timestep length and duration
 */
//from Q+A video




size_t N = 10;//10;
double dt = 0.05; //0.1

// This value assumes the model presented in the classroom is used.
//
// It was obtained by measuring the radius formed by running the vehicle in the
//   simulator around in a circle with a constant steering angle and velocity on
//   a flat terrain.
//
// Lf was tuned until the the radius formed by the simulating the model
//   presented in the classroom matched the previous radius.
//
// This is the length from front to CoG that has a similar radius.
const double Lf = 2.67;
//from Q+A video
double ref_cte = 0 ;
double ref_epsi = 0 ;
double ref_v = 100 ;

size_t x_start = 0;
size_t y_start = x_start + N;
size_t psi_start = y_start + N;
size_t v_start = psi_start + N;
size_t cte_start = v_start + N;
size_t epsi_start = cte_start + N;
size_t delta_start = epsi_start + N;
size_t a_start = delta_start + N -1;
//end Q+A video

class FG_eval {
 public:
  // Fitted polynomial coefficients
  VectorXd coeffs;
  FG_eval(VectorXd coeffs) { this->coeffs = coeffs; }

  typedef CPPAD_TESTVECTOR(AD<double>) ADvector;
  void operator()(ADvector& fg, const ADvector& vars) {
    /**
     * TODO: implement MPC
     * `fg` is a vector of the cost constraints, `vars` is a vector of variable 
     *   values (state & actuators)
     * NOTE: You'll probably go back and forth between this function and
     *   the Solver function below.
     */


    //from Q+A video
    fg[0] = 0;
   ////////////////
    // This is the cost variables for below listed here for convience
    //reference state costs
    // I am using Q+ levels then doubleing and then halving them on first set of tests
    double cte_ref_state_cost = 1000 ; //1000//4000 // 1000 //2000 in Q+A
    double epsi_ref_state_cost = 1000 ;//1000 //4000 //2000 in Q+A
    double v_ref_state_cots = 1 ; //1 in Q+A
    //Acuators Cost
    double delta_cost = 2500 ;//50 in Q+A
    double a_cost = 150 ; //50 in Q+A
    //Acuators Cost for time ahead t + 1
    double delta_plus_cost = 10000;//1//250000 // 200 in Q+A
    double a_plus_cost = 150; //1000;//1//5000 // 10 in Q+A
      
    for(int i = 0; i < N ; i++)
    {//2000 below is high weight
      fg[0] += cte_ref_state_cost*CppAD::pow(vars[cte_start + i] - ref_cte,2);//was 2000
      fg[0] += epsi_ref_state_cost*CppAD::pow(vars[epsi_start + i] - ref_epsi,2);//was 2000
      fg[0] += v_ref_state_cots * CppAD::pow(vars[v_start + i] - ref_v,2); //had 1 instead of i
      
    }
    
    for(int i = 0; i < N -1; i++)
    {
      fg[0] += delta_cost * CppAD::pow(vars[delta_start + i],2);
      fg[0] += a_cost * CppAD::pow(vars[a_start + i],2);
    }
    
    for(int i = 0; i < N -2; i++)
    {
      fg[0] += delta_plus_cost * CppAD::pow(vars[delta_start + i + 1] - vars[delta_start + i] , 2);//200
      fg[0] += a_plus_cost * CppAD::pow(vars[a_start + i + 1] - vars[a_start + i],2);//10
      
    }
    
    //constraints
    fg[1 + x_start] = vars[x_start];
    fg[1 + y_start] = vars[y_start];
    fg[1 + psi_start] = vars[psi_start];
    fg[1 + v_start] = vars[v_start];
    fg[1 + cte_start] = vars[cte_start];
    fg[1 + epsi_start] = vars[epsi_start];
    //for rest of constraints
    
    for(int i = 0; i < N -1;i++)
    {
      AD<double> x1 = vars[x_start + i + 1];
      AD<double> y1 = vars[y_start + i + 1];
      AD<double> psi1 = vars[psi_start + i + 1];
      AD<double> v1 = vars[v_start + i + 1];
      AD<double> cte1 = vars[cte_start + i + 1];
      AD<double> epsi1 = vars[epsi_start + i + 1];
      
      // at time t
      AD<double> x0 = vars[x_start + i];
      AD<double> y0 = vars[y_start + i];
      AD<double> psi0 = vars[psi_start + i];
      AD<double> v0 = vars[v_start + i];
      AD<double> cte0 = vars[cte_start + i];
      AD<double> epsi0 = vars[epsi_start + i];
      
      AD<double> delta0 = vars[delta_start + i];
      AD<double> a0 = vars[a_start + i];
      AD<double> f0 = coeffs[0] + coeffs[1] * x0 + coeffs[2] * x0 * x0 + coeffs[3] * x0 * x0 * x0 ;   
      // changing i to one as from advice from knowledge
      
      AD<double> psides0 = CppAD::atan(3*coeffs[3] * x0 * x0 + 2 * coeffs[2] * x0 + coeffs[1]);
     //hmmm not sure about above 2 lines
      
         
       //changing 2 for 1 to try to fix index ewrror next 5 lines

      fg[2 + x_start + i] = x1 - (x0 + v0 * CppAD::cos(psi0) * dt);
      fg[2 + y_start + i] = y1 - (y0 + v0 * CppAD::sin(psi0) * dt);
      fg[2 + psi_start + i] = psi1 - (psi0 - v0 * delta0/Lf * dt);
      fg[2 + v_start + i] = v1 - (v0 + a0 *dt);
      //fg[2 + cte_start + i ] = epsi1 - ((psi0 - psides0) - v0 * delta0 / Lf * dt);//wrong!!
      fg[2 + cte_start + i ] = cte1 - ((f0 - y0) +(v0 * CppAD::sin(epsi0) * dt));
      //am I missing this 
      fg[2 + epsi_start + i] = epsi1 - ((psi0 - psides0) - v0 / Lf * delta0 * dt);

      
      
  
                                       
    }
    
    
  }
};

//
// MPC class definition implementation.
//
MPC::MPC() {}
MPC::~MPC() {}
//this line is different in Q+A and might be casuing the Solve index problem also changed header file
//changing back  vars seems emptry once solve working
//std::vector<double> MPC::Solve(const VectorXd &state, const VectorXd &coeffs) {
std::vector<double> MPC::Solve(Eigen::VectorXd state,  Eigen::VectorXd coeffs){
  std::cout << "line 137 in MPC SOLVE" << std::endl ;
  bool ok = true;
  typedef CPPAD_TESTVECTOR(double) Dvector;
//from Q+A
  
  double x = state[0];
  double y = state[1];
  double psi = state[2];
  double v = state[3];
  double cte = state[4];
  double epsi = state[5];
  std::cout << "line 148 in MPC SOLVE" << std::endl ;
  //end Q+A
  /**
   * TODO: Set the number of model variables (includes both states and inputs).
   * For example: If the state is a 4 element vector, the actuators is a 2
   *   element vector and there are 10 timesteps. The number of variables is:
   *   4 * 10 + 2 * 9
   *///Q+A video
 


size_t n_vars = N * 6 + (N -1) *2; //from Q+A//0;
  /**
   * TODO: Set the number of constraints
   */



  size_t n_constraints = N * 6;//from Q+A//0;
std::cout << "line 161 in MPC SOLVE" << std::endl ;
  // Initial value of the independent variables.
  // SHOULD BE 0 besides initial state.
  Dvector vars(n_vars);
  for (int i = 0; i < n_vars; i++) {//aaarrrgggghhh I had ++i instead of i++ gave wierd index error
    vars[i] = 0;
  }
std::cout << "line 168 in MPC SOLVE" << std::endl ;
  Dvector vars_lowerbound(n_vars);
  Dvector vars_upperbound(n_vars);
  /**
   * TODO: Set lower and upper limits for variables.
   */




  std::cout << "line 161 in MPC SOLVE" << std::endl ;
  //from Q+A
  for(int i = 0; i<delta_start;i++)
  {
    vars_lowerbound[i] = -1.0e19;
    vars_upperbound[i] = 1.0e19;
  }

  std::cout << "line 181 in MPC SOLVE" << std::endl ;
//also from Q+A
  //delta -25 to 25 degrees values in radians
  for(int i = delta_start; i < a_start;i++)
  {
    vars_lowerbound[i] = -0.436332 * Lf;
    vars_upperbound[i] =  0.436332 * Lf;
  }

  std::cout << "line 189 in MPC SOLVE" << std::endl ;
  //acclereration.deceleration upper and lower limit
  for(int i = a_start; i< n_vars; i++)
  {
    vars_lowerbound[i] = -1;
    vars_upperbound[i] = 1;
  }

  std::cout << "line 196 in MPC SOLVE" << std::endl ;
  // Lower and upper limits for the constraints
  // Should be 0 besides initial state.
  Dvector constraints_lowerbound(n_constraints);
  Dvector constraints_upperbound(n_constraints);
  std::cout << "line 201 in MPC SOLVE" << std::endl ;
  for (int i = 0; i < n_constraints; i++) { // ++i instead of i++ again argggghhhh
    constraints_lowerbound[i] = 0;
    constraints_upperbound[i] = 0;
  }
std::cout << "line 206 in MPC SOLVE" << std::endl ;
  //also from Q+A
  constraints_lowerbound[x_start] = x;
  constraints_lowerbound[y_start] = y;
  constraints_lowerbound[psi_start] = psi;
  constraints_lowerbound[v_start] = v;
  constraints_lowerbound[cte_start] = cte;
  constraints_lowerbound[epsi_start] = epsi;
  std::cout << "line 214 in MPC SOLVE" << std::endl ;
  constraints_upperbound[x_start] = x;
  constraints_upperbound[y_start] = y;
  constraints_upperbound[psi_start] = psi;
  constraints_upperbound[v_start] = v;
  constraints_upperbound[cte_start] = cte;
  constraints_upperbound[epsi_start] = epsi;
  
  std::cout << "line 222 in MPC SOLVE" << std::endl ;
  
  // object that computes objective and constraints
  FG_eval fg_eval(coeffs);
std::cout << "line 226 in MPC SOLVE" << std::endl ;
  // NOTE: You don't have to worry about these options
  // options for IPOPT solver
  std::string options;
  // Uncomment this if you'd like more print information
  options += "Integer print_level  0\n";
  // NOTE: Setting sparse to true allows the solver to take advantage
  //   of sparse routines, this makes the computation MUCH FASTER. If you can
  //   uncomment 1 of these and see if it makes a difference or not but if you
  //   uncomment both the computation time should go up in orders of magnitude.
  options += "Sparse  true        forward\n";
  options += "Sparse  true        reverse\n";
  // NOTE: Currently the solver has a maximum time limit of 0.5 seconds.
  // Change this as you see fit.
  options += "Numeric max_cpu_time          0.5\n"; //maybe change in relation to N can get too long
std::cout << "line 241 in MPC SOLVE" << std::endl ;
  // place to return solution
  CppAD::ipopt::solve_result<Dvector> solution;
std::cout << "line 244 in MPC SOLVE" << std::endl ;
  // solve the problem
  //cout for all paramenteres here to try and find one causeing index problem
  std::cout << "options size:" << options.size() << std::endl;
  std::cout << "vars size :" << vars.size() << std::endl;
  std::cout << "vars_lowerbound size:" << vars_lowerbound.size() << std::endl;
  std::cout << "vars upperbound size :" << vars_upperbound.size() << std::endl;
  std::cout << "constraints_lowerbound size :" << constraints_lowerbound.size() << std::endl;
  std::cout << "constraints_upperbound size :" << constraints_upperbound.size() << std::endl;
  //std::cout << "fgeval size :" << fg_eval.size() << std::endl;//no size() for this made error.....
  //std::cout << "solution size :" << solution.size() << std::endl;//no size() for this
  
  CppAD::ipopt::solve<Dvector, FG_eval>(
      options, vars, vars_lowerbound, vars_upperbound, constraints_lowerbound,
      constraints_upperbound, fg_eval, solution);
std::cout << "line 249 in MPC SOLVE" << std::endl ;
  // Check some of the solution values
  ok &= solution.status == CppAD::ipopt::solve_result<Dvector>::success;
std::cout << "line 252 in MPC SOLVE" << std::endl ;
  // Cost
  auto cost = solution.obj_value;
  std::cout << "Cost " << cost << std::endl;

  /**
   * TODO: Return the first actuator values. The variables can be accessed with
   *   `solution.x[i]`.
   *
   * {...} is shorthand for creating a vector, so auto x1 = {1.0,2.0}
   *   creates a 2 element double vector.
   */
  //from Q+A video




  std::vector<double> result;
  
  result.push_back(solution.x[delta_start]);
  result.push_back(solution.x[a_start]);
  // i want to check solution here
  // hmmm solution is comming back all zeros
   for(int i = 0; i < N-1; i ++)
  {
   std::cout << "Solution:" << i << " " << solution.x[ i] << std::endl ;
   
  }
  for(int i = 0; i < N-1; i ++)
  {
    result.push_back(solution.x[x_start + i + 1]);
    result.push_back(solution.x[y_start + i + 1]);
  }
  
  return result; //{};
}
