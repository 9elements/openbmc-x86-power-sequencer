#include <iostream>
#include <ctime>
#include "Config.hpp" 
#include "SignalProvider.hpp" 

#include "StateMachine.hpp" 

using namespace std;
 
int main(void) {
  Config cfg = LoadConfig("config");
  
  SignalProvider signalprovider(cfg);
  StateMachine sm(cfg, signalprovider);

  sm.Validate();

  sm.Run(); // Run never returns
  return 0;
}
