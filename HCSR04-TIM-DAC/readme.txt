Reads the distance from HCSR04 and drives the LED by DAC with sine wave.
DAC period is determined by the value obtained from HCSR04.

Clock configuration can be seen inside the code, important point is TIM2 time unit should match the HCSR04 output time unit (ms).
DMA is triggered by TIM2 output event. 
Sine wave is passed to DAC-DMA and TIM2 ARR register is being varied with respect to distance value coming from the sensor.

Breathing LED is observed as the sinusoidal wave generated from DAC.  

PE9 -> ECHO PIN on HCSR04
PG2 -> TRIGGER PIN on HCSR04

PA4 -> LED on the breadboard.
