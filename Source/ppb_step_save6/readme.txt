This is a save of PPBSTEP with the first working version of circular rotation
of platform with X and Y on stepper 3 and stepper 4. This version used
one timer2 interrupt with varying period for speed control. On each
interrupt, decision was made whether to step both X and Y steppers based
on additions of fractional step value to individual X and Y accumulators.
For each accumulator step, was taken when accumulator exceeded 1 step and
the full step value was then subtracted from the accumulator. This processed
segments which had a data structure to specify how many interrupts, and
the increment applied for X and Y. Two strucures existed and were alternately
filled by background and consumed by ISR.

New version will also use segments, with ping-pong control structures, but
the control structures will contain StepCount and 16-bit period like
standard stepper control for S1 and S2. There will be one ping-pong control
set for X and one for Y. The segments will specify a fixed interval in
milliseconds, (likely 5). The segments will be timed by Timer2 overflow
interrupt. When a new segment is started, each output compare register will
be loaded with a small value so that each overflow will occur shortly thereafter
and take the first step for X and Y assuming StepCount is non-zero.
Direction will be set when the Timer2 overflow starts the segment as
direction must be constant for each segment.

So this is a new wrinkle for the segment generator, that segment must have
constant direction for both X and Y. Maybe the problem goes away, in that
we are always just looking at current position (position at end of previously
calculated segment) and projecting position at end of segment. If a direction
shift occurs during segment, we just flatten the trajectory or truncate
a peak. So for instance a segment which has X going from -5 to +5 and reversing
to go back to -5  with 20 total steps in X would end up with zero steps
in X

So for new circle generation, I want to specify radius in steps, Rev/Sec * 1000.
So RPSX1K of 1 means .001 revolutions per second or 1000 seconds per rev.
So RPSX1K of 200 means .2 revolutions per second or 5 seconds per rev.
So RPSX1K of 500 means .5 revolutions per second or 2 seconds per rev.
Seems about right. Also want to be able to change rate and radius while active.
Assuming segments last TS = .005 seconds.
So when starting at initial radius and rate, I set X = Radius, Y = 0.
First segment is from there to the point at end of initial angle which is
PhiSeg = (2*Pi)*(RPS*TS).
For each segment calculation, we compute the angle increment PhiSeg and
add it to old current angle to get new current angle which with Radius
allows us to calculate endpoint of segment. We then see how many
steps there are in the segment. We then use the XSteps and YSteps values
to calculate the period.

Period is in increments of 4 USec. One complete
cycle to Timer2 takes 256 clocks or .001024. Five cycles takes .00512 seconds.
So our segment time needs to be in increments of .001024 seconds.

So, the total clocks for a nominal 5 msec segment is 5*256 = 1280. We divide
this by number of steps to get the clocks per segment.   We have set 10 as
minimum clocks resulting in 40 USec min period or 25000 Hz as maximum rate.
So for maximum rate we have 128 steps maximum in a segment. If we double
segment time, we get 256 steps maximum.

At a low step rate of say 250 Hz, we are at 1/100 of maximum speed. That
means we would have 1.28 steps in a segment.

Our ISR implementation limits clocks per step to maximum of 65535 for a time
of .26214 seconds or  3.81 steps oer second.  For slow rotation rates
we can have no steps for a number of segments. At boundary of 1 step
per segment, we would be stepping at 195.3125 steps per second
