//*****************************************************************************
//
//	SineTable.c
//  Author   DF
//  COMPILER    -      Keil uVision Version 5.15
//  TARGET      -      Cortex M3
//
//           
//*****************************************************************************
#include "target.h"
//#include <stdlib.h>
#include "stdio.h"
#include "math.h"

//SineTable(double Table, double Radians, double Count)
//Generate a sinetable for a specified number of radians
//We get Count entries starting at angle  sine(Radians/(Count-1))  and proceding to sine(Radians)
//For motion, probably we want a 0.0 entry for Sine(0)
//We generally only need quarter cycle but calculation is fast and why not do whole thing
//We will generate a 180 degree table and reflect at end.
//In C Pi is defined in math.h: #define M_PI 3.14159265358979323846



//So for interpolated cyclical moves, we will move through a table of positions we will be moving at a rate that translates to
//integer ticks. A tick is 1/4096 seconds. All moves will be in segments of an even number of ticks. We envision working up to a speed of at least 200 ticks
//per second but the maximum rate will depend on how fast we can compute segments. 
//for 200 segments per second, each sement will last approx value 20 ticks. So the fastest we could conceivably
//go would be 2000 segments per second for 2 ticks per segment. We could implement an acceleration function for
//rate of motion through the list. We will do it all in floating point as it is all done by the background routine. 
//We just need to keep up with consumption of segments which will occur at rate determined by our rate.
//As we get up to higher rates, our increments in tick time will be very coarse.
//If we need higher speeds for a circular pattern, we could use less steps, say ten for a half cycle.
//Say we limit segments to ten ticks so we have 400 segments per second. If a cycle takes 20 segments,
//we could have a rate of 20 Hz. For a 100 step table we would have 200 segments per cycle giving 2 hz rate.
//We could always use shorter tables to give higher rates. So looks like we will be OK. Lets limit
//dual segments to 10 ticks minimum. This means we compute a new segment every 2.5 milliseconds for a
//400 Hz segment rate. So we will have an acceleration rate for the cyclical move. We can change it on the
//fly. We apply that to velocity. The velocity will be in segments per second with a maximum of 400.
//The acceleration will be a floating point value going very low and maxing out at maybe 400 segments/sec/sec.
//We will work through the table as fast as we can filling the segment FIFO. So if we change Acc, there will
//be a delay before we see it of four dual segments. We have a total FIFO length of 8.
//So when segment generator is running, we will set Acceleration to positive or negative value. We will
//have positiive or negative segment velocities. They will go through list in forward or reverse. 
//We will have a segment start command which will start us generating segments. We will generate segments
//until list is full and then change ServoState for both servos to SS_MOVE. We will be generating segments for at least
//two servos. The segments will be generated in pairs of dual segments at the same tick interval. They
//will hence stay in step. Maybe we just look at one segment list to pace us and assume the other stays in step.
//We just keep looking at acceleration to see if it has been reversed. If we want to be able to slow to a stop,
//we need to indicate this so we don't just keep decelerating and end up going in reverse direction at maximum rate.
//So we have signed Acceleration, Velocity (starting at zero) and a stop flag which causes us to stop processing
//when velocity passes through zero. In fact calling stop will set stop flag and reverse sign of acceleration.
//So we need some commands to control interpolation. We will start with two axes in 90 phase. We will have a quarter cycle
//cosine table with N steps including 0 degrees and 90 degrees. So as angle goes from 0-90, we just go through table for cos.
//For sine, we start at N and work back to zero. Whatever our division of 90 degrees is (say N) we will have N+1 entries.
//We will count from N on up to 4N and wrap to zero, or count from 0 == 4N to 4N-1.
//For cos, we negate values if N > 2N.  For sine, we just start shifted by 90 (that is at N) and increment. We negate the sine value
//when <n or >3N
//As our index goes up, we need to convert it to a table value. We can just switch based on quadrant. 
Quad   SineIndex  CosIndex	Cos Sign
0	   N-(I		    I%N		    1
1					N-(I%N)	   -1
2					I%N		   -1
3							    1
So, we need to advance our selves through table in steps while changing the tick increment.
Each entry in table is a position for our axis. Will compute the segment based on the last 
segment we produced or zero. So for each axis we have a current velocity, starting at zero. This
is in fact the VLast of segment generator. We then generate a dual segment from VLast and last position
to next point. We want to end at the mean of the velocities of the current segment and average velocity of
the next segment. One thing we will know is the length of the segment, that is, the difference between the
starting point and next point.To compute the average velocity of 
When we start out, we will compute the length of current segment in time. It will be some length N based on
acceleration. We
















