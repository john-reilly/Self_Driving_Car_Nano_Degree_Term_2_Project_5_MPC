#include <math.h>
#include <uWS/uWS.h>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include "Eigen-3.3/Eigen/Core"
#include "Eigen-3.3/Eigen/QR"
#include "helpers.h"
#include "json.hpp"
#include "MPC.h"

// for convenience
using nlohmann::json;
using std::string; // becuase of error
using std::vector;

// For converting back and forth between radians and degrees.
constexpr double pi() { return M_PI; }
double deg2rad(double x) { return x * pi() / 180; }
double rad2deg(double x) { return x * 180 / pi(); }


//from Q+A video // this hasData was in helpers.h
//string hasData(string s){
//	auto found_null = s.find("null");
//  auto b1 = s.find_first_of("[");
//  auto b2 = s.rfind("}]");
  
//  if(found_null != string::npos){
//  	return "";
//  }else if (b1 != string::npos && b2 != string::npos){
//  return s.substr(b1,b2 -b1 +2);
//  }
//  return "";
//}

//evaluate a polynomial
//double polyeval(Eigen::VectorXd coeffs, double x){
//double result = 0.0 ;
//  for (int i=0; i < coeffs.size(); i++){
//    result +=coeffs[i] * pow(x,i);
//  }
//  return result;
//}

//fit a ploynomial
// from quiz Lesson 18, Video 9
// Adapted from
// https://github.com/JuliaMath/Polynomials.jl/blob/master/src/Polynomials.jl#L676-L716
//Eigen::VectorXd polyfit(Eigen::VectorXd xvals, Eigen::VectorXd yvals,int order) {
//  assert(xvals.size() == yvals.size());
//  assert(order >= 1 && order <= xvals.size() - 1);
//  Eigen::MatrixXd A(xvals.size(), order + 1);

//  for (int i = 0; i < xvals.size(); i++) {
//    A(i, 0) = 1.0;
//  }

//  for (int j = 0; j < xvals.size(); j++) {
//    for (int i = 0; i < order; i++) {
//      A(j, i + 1) = A(j, i) * xvals(j);
//    }
//  }

//  auto Q = A.householderQr();
//  auto result = Q.solve(yvals);
//  return result;
//}



int main() {
  uWS::Hub h;

  // MPC is initialized here!
  MPC mpc;

  h.onMessage([&mpc](uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length,
                     uWS::OpCode opCode) {
    // "42" at the start of the message means there's a websocket message event.
    // The 4 signifies a websocket message
    // The 2 signifies a websocket event
    string sdata = string(data).substr(0, length);
    std::cout << sdata << std::endl;
    if (sdata.size() > 2 && sdata[0] == '4' && sdata[1] == '2') {
      string s = hasData(sdata);
      if (s != "") {
        auto j = json::parse(s);
        string event = j[0].get<string>();
        if (event == "telemetry") {
          std::cout << "Line 93 Telementry" << std::endl ;
          // j[1] is the data JSON object
          vector<double> ptsx = j[1]["ptsx"];
          vector<double> ptsy = j[1]["ptsy"];
          double px = j[1]["x"];
          double py = j[1]["y"];
          double psi = j[1]["psi"];
          double v = j[1]["speed"];
          //wondering if the file is being read OK
		  std::cout << "Line 101 after inits" << std::endl ;
          //std::cout << "ptsx :" << ptsx << std::endl;
          //std::cout << "ptsy :" << ptsy << std::endl;
          std::cout << "px :" << px << std::endl;
          std::cout << "py :" << py << std::endl;
          std::cout << "v:" << v << std::endl;
          /**
           * TODO: Calculate steering angle and throttle using MPC.
           * Both are in between [-1, 1].
           */
 
          //from Q+A video
          for( int i = 0; i < ptsx.size(); i++)// index error might be the <= here trying "<" ...didn't work reverting// trying again as per advice from Knowledge reply
          {
            //shift car ref angle to 90 degree
            double shift_x = ptsx[i] - px ;
            double shift_y = ptsy[i] - py ;
            
            ptsx[i] = (shift_x * cos(0-psi) - shift_y * sin(0-psi));
            ptsy[i] = (shift_x * sin(0-psi) + shift_y * cos(0-psi));
            
          }
          std::cout << "Line 118 after first loop" << std::endl ;
          double* ptrx = &ptsx[0] ;
          Eigen::Map<Eigen::VectorXd> ptsx_transform(ptrx,6) ;
          
          double* ptry = &ptsy[0] ;
          Eigen::Map<Eigen::VectorXd> ptsy_transform(ptry,6) ;
          
          std::cout << "ptsx_transform size: " << ptsx_transform.size() << std::endl;
          std::cout << "ptsy_transform size: " << ptsy_transform.size() << std::endl;
          auto coeffs = polyfit(ptsx_transform , ptsy_transform,  3) ;
          std::cout << "coeffs :" << coeffs.size() << std::endl;
          
          //calculate cte and epsi
          double cte = polyeval(coeffs , 0);
          
          double epsi = -atan(coeffs[1]);
             
          double steer_value = j[1]["steering_angle"] ;
          double throttle_value = j[1]["throttle"] ;
          
          Eigen::VectorXd state(6);
          state << 0,0,0,v,cte,epsi;
          //maybe zeros and messing it up??
          //state << 1,1,1,v,cte,epsi; //didn't change thing
          std::cout << "Line 136 before solve" << std::endl ;
          std::cout << "State:" << state << std::endl ;
          std::cout << "Coeffs:" << coeffs << std::endl ;
          
          auto vars = mpc.Solve(state,coeffs);
           std::cout << "Line 137 after solve" << std::endl ;
           std::cout << "VARS size" << vars.size()<< std::endl ;
          std::cout << "VARS [1]" << vars[1] <<std::endl ;
          std::cout << "VARS [2]" << vars[2] <<std::endl ;
          std::cout << "VARS [3]" << vars[3] <<std::endl ;
          
          
          vector<double> next_x_vals ;
          vector<double> next_y_vals ;
          
          double poly_inc = 2.5 ;
          int num_points = 25 ;
          
          for(int i = 1 ; i < num_points ; i++)
          {
            next_x_vals.push_back(poly_inc*i);
            next_y_vals.push_back(polyeval(coeffs,poly_inc*i));
          }
          std::cout << "Line 151" << std::endl ;
          vector<double> mpc_x_vals ;
          vector<double> mpc_y_vals ;
          
          for(int i = 2 ; i < vars.size();i++)
          {
            if(i%2 ==0)
            {
              mpc_x_vals.push_back(vars[i]) ;
            }
            else
            {
              mpc_y_vals.push_back(vars[i]) ;
            }
          }
               std::cout << "Line 166" << std::endl ;
          double Lf = 2.67 ;
          
          json msgJson;
          // NOTE: Remember to divide by deg2rad(25) before you send the 
          //   steering value back. Otherwise the values will be in between 
          //   [-deg2rad(25), deg2rad(25] instead of [-1, 1].
          msgJson["steering_angle"] = vars[0]/(deg2rad(25)*Lf );//steer_value;
          msgJson["throttle"] = vars[1] ; //throttle_value;
          //end of Q+A material
      

          // Display the MPC predicted trajectory 

          //vector<double> mpc_x_vals;
          //vector<double> mpc_y_vals;//were commented out//redecaration

          /**
           * TODO: add (x,y) points to list here, points are in reference to 
           *   the vehicle's coordinate system the points in the simulator are 
           *   connected by a Green line
           */

          msgJson["mpc_x"] = mpc_x_vals;
          msgJson["mpc_y"] = mpc_y_vals;

          // Display the waypoints/reference line

          //vector<double> next_x_vals;
          //vector<double> next_y_vals;//also left out?? not in Q+A gave error redeclaration


          /**
           * TODO: add (x,y) points to list here, points are in reference to 
           *   the vehicle's coordinate system the points in the simulator are 
           *   connected by a Yellow line
           */

          msgJson["next_x"] = next_x_vals;
          msgJson["next_y"] = next_y_vals;


          auto msg = "42[\"steer\"," + msgJson.dump() + "]";
          std::cout << msg << std::endl;
          // Latency
          // The purpose is to mimic real driving conditions where
          //   the car does actuate the commands instantly.
          //
          // Feel free to play around with this value but should be to drive
          //   around the track with 100ms latency.
          //
          // NOTE: REMEMBER TO SET THIS TO 100 MILLISECONDS BEFORE SUBMITTING.
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
        }  // end "telemetry" if
      } else {
        // Manual driving
        std::string msg = "42[\"manual\",{}]";
        ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
      }
    }  // end websocket if
  }); // end h.onMessage

  h.onConnection([&h](uWS::WebSocket<uWS::SERVER> ws, uWS::HttpRequest req) {
    std::cout << "Connected!!!" << std::endl;
  });

  h.onDisconnection([&h](uWS::WebSocket<uWS::SERVER> ws, int code,
                         char *message, size_t length) {
    ws.close();
    std::cout << "Disconnected" << std::endl;
  });

  int port = 4567;
  if (h.listen(port)) {
    std::cout << "Listening to port " << port << std::endl;
  } else {
    std::cerr << "Failed to listen to port" << std::endl;
    return -1;
  }
  
  h.run();
}


