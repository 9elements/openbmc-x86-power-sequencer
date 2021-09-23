**Input / Output drivers **:

The input/output driver connect real hardware to the x86 power sequencers
internal logic. 

** List of drivers:**

The following input drivers are implemented:
  - GPIO
  - NULL

The following output drivers are implemented:
  - GPIO
  - NULL


** Rules:**

An input driver must fetch the hardware state and call the `SetLevel()` method
of the connected signal. The method can be called asynchronously.

An output driver must apply the current signal level only at call of the`Apply()`
method. All `Apply()` methods are called at the same time to prevent glitches.

An output driver implements the abstract calls `OutputDriver`.


** GPIO driver: **
The GPIO driver requires the GPIO pin to be present in the current system.
A failure of finding the pin will terminate the application.

** Null driver: **
The null driver doesn't need real hardware and should be used for testing only.