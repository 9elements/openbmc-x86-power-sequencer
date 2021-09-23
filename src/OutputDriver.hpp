
#ifndef _OUTPUTDRIVER_HPP__
#define _OUTPUTDRIVER_HPP__

using namespace std;

// The output driver applies the signal level to the physical device.
// All output drivers cache the signal level and apply changes when Apply() is invoked.
class OutputDriver {
public:
	virtual void Apply(void) = 0;
};

#endif