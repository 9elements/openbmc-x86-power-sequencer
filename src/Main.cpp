#include <iostream>
#include <ctime>
#include "Config.hpp" 
#include "StateMachine.hpp" 

using namespace std;
 
int main(void) {
  Config cfg = LoadConfig("config");
  
  StateMachine sm(cfg);
  sm.Run(true);
  sm.Run(true);
  return 0;
}
