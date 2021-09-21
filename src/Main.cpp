#include <iostream>
#include <ctime>
#include "Config.hpp" 
#include "SignalProvider.hpp" 

#include "StateMachine.hpp" 

using namespace std;
 
int main(void) {
  Config cfg = LoadConfig("config");
  
  SignalProvider signalprovider;
  StateMachine sm(cfg, signalprovider);

  sm.Run(true);
  sm.Run(true);
  return 0;
}
