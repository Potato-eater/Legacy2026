# **Team Legacy Team Description Paper**
## Western Australia State Competition

We aren't really required to write a TDP anymore, but anyway here it is.
Stuff in this document will probably be identical to our poster submission

## Team Name
### Legacy

## Team members and roles
- Shing (Team Captain/programmer)
    - Started robotics in 2021.
    - Currently in year 12.
- Cruz (Structual designer/Mechanic)
    - Started robotics in 2022.
    - Currently in year 11.

## interesting/innovative features of the robot's design or engineering
- Infrared Sensor
    - After having 4 years experience of dealing with pre-made infrared sensors, like the Hitechnic and Building Block Studio irseeker, we decided to design and build our own sensor.
    - We used 16 TSSP94038 IR receivers.
    - Using vector maths, we can do interpolation using each sensor's data
    - the outcome is better than any other sensors we had seen.
- Line (colour) sensor
    - 1 rule that makes standard soccer and lightweight soccer different is that the robot is not allowed to move outside of the white lines, or else it would be treated as "damaged".
    - Our solution is to create our own colour sensor board to detect the line.
    - We used 16 TCRT5000 sensors, in a ring configuration. This means the robot can sense the line equally well no matter which way the robot is currently facing.
- Camera
    - Most teams would make their robot kick the ball straight forward. They can get a goal if they are lucky, but the ball ends up getting kicked out of bounds most of the time. 
    - Using a camera, the robot can identify the colour of the goal and aim directly towards it. it would even avoid opponent goal-keeper robots and aim into the empty spaces.
- Main PCB
    - The centre "brain" of the robot. it includes the teensy 4.1 and connection ports to all sensors and devices.
    - this modular design allows any parts to be swapped, and the rest of the robot can still remain independent.
- Positioning
    - In order for the robot to be good, it must know where its current location is.
    - Most teams would use distance sensors like ultrasonics or lasers. However, they would almost never work as intended, as the opponents robots would cause interference.
    - We are using the Sparkfun Optical Tracking Odometry Sensor (OTOS). It movements based on the floor and calculates the robot's current position.
    - it eliminates the problem of interference from other robots, but error can slowly build up overtime. Therefore correction algorithms are needed.
- Wireless Communication
    - in order to make the two robots coordinate efficently, they must have the capability for direct communcation.
    - Instead of a separate bluetooth module, we actually use the line sensor's esp-32-s2 chip to achieve the same objective.
    - the main mcu would send over data to the line sensor pcb. The esp32-s2 module would then relay the exact same data to the other robot.
    - This significantly increased speed and reliability.
- omni-directional drive system
    - The robot must be able to move in any direction at any time. Sometimes it might even need to rotate and move at the same time.
    - We put our motors into an X configuration, with omni-wheels directly mounted onto each motor.
    - each omni-wheel is equipped with rollers perpendicular to its axis of rotation. This eliminates friction when the robot is moving in a direction different from the wheel's axis of rotation.
- dribbler
    - ## The *ONLY* exception to rule `4.6.3`
    - **Legal** way to get control of the ball.
    - by spinning rubber wheels with a motor, we can create a backspin on the ball into the robot's body. Therefore having a high level of control of the ball.

## Challenge Encountered
Shing: I think the hardest challenge is dealing with *ourselves*. 1st part is balancing robotics and academics. Both requires a lot of time, and it can be very stressful to think about the amount of work you need to do. The way to solve this is to split every task into very very small steps. Eventually, you would realise you've done everything.

Another challenge is finding ways to improve the robot. Achieving what other teams has already done is the bare minimum. This is why we would often look at our robots from previous years and videos of robots in the internationals competition, and reflect on each of them. 
t's important to look at winners' robots, but its also important to look at the people who lost.

We often asks questions to like: 
- what problems this currently have?
- What makes a team's robot good/bad?
- Why is this design good/bad? 
- How was this design achieved?
- What can we do/avoid in our robot? 
 
## What we learned through participation:
- BBC is our enemy. Defeat them.
- Do not just work on your robot by yourself. Find help when you need. You can always ask experts for opinion (ask intelligent questions though, they are not ChatGPT).
- Don't be afraid to make friends with other teams. You guys already have the same hobby, so its usually easy to get onto the same wavelength.
