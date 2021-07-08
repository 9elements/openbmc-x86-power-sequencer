# x86 power sequencer

To be used on OpenBMC enabled platforms that do not use a CPLD for
x86 power state handling.

# PCH power states

Those power states are from PCH point of view. The BMC is always in S0.

## G3
 - Transition to S0 (to S5 if AFTERG3_EN is set).

## S5 - Power off. DRAM off.
 - Transistion to S0 on wake event. If no wake events are supported
 - Must transition to G3 for S5->S0 if PWRBTN# not supported.

## S4 - Power off. DRAM off. Wake sources powered.
 - If not advertised to OS same as S5 and Wake sources powered off.

## S3 - Power off. DRAM on. Wake sources powered.
 - Transisition to S0 on wake event.

## S0 - All Power on.
 - Transition to S3 on user request.
 - Transition to S5 on user request.

# Config file

