#include <iostream>
#include <ctime>
#include "Config.hpp" 
#include "StateMachine.hpp" 

using namespace std;
 
int main(void) {
  Config cfg = LoadConfig("config");
  
  for (int i = 0; i < cfg.Logic.size(); i++) {
    std::cout << "Node " << i << std::endl;
    std::cout << " Name: " << cfg.Logic[i].Name << std::endl;
    std::cout << " Signal: " << cfg.Logic[i].Out.SignalName << std::endl;
  }
  return 0;
}
