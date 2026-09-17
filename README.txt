===========================================
Project: Simulation and Performance Analysis of Routing Protocols using OMNeT++
Course: Computer Networks – Spring 2026
Section: SE
Submission Date: 8th May, 2026
===========================================

TEAM MEMBERS:
- Hamad Khan (23i-0058) - Static Routing Module
- Shah Faisal (23i-0058) - RIP Simulation
- Ammad Ashraf (22i-2470) - OSPF Simulation

===========================================
HOW TO RUN THE SIMULATIONS:
===========================================

Prerequisites:
- OMNeT++ 6.x with INET 4.5+ installed
- Set OMNeT++ environment variables

Steps to run:

1. STATIC ROUTING SCENARIOS:
   - Open OMNeT++ IDE
   - Import the project folder
   - Select Configuration: "Static_CBR" or "Static_Bursty"
   - Run Simulation

2. RIP SCENARIOS:
   - Select Configuration: "RIP_CBR" or "RIP_Bursty"
   - Run Simulation

3. OSPF SCENARIOS:
   - Select Configuration: "OSPF_CBR" or "OSPF_Bursty"
   - Run Simulation

Alternative: Command Line
-------------------------
opp_run -l ../../inet4/src/INET -n .:../../inet4/src -u Cmdenv -c RIP_CBR omnetpp.ini

===========================================
FILE STRUCTURE:
===========================================

Core Simulation Files:
- CampusNetwork.ned : Main topology file
- StaticRouter.ned & .cc/.h : Custom static routing implementation
- TrafficGen.ned & .cc/.h : Custom traffic generator
- LinkController.ned & .cc/.h : Link failure controller
- omnetpp.ini : All configuration parameters
- static_routes.xml : Static routing table entries

Result Files:
- *.vec : Vector results (time-series data)
- *.sca : Scalar results (statistical summaries)

===========================================
KEY PARAMETERS:
===========================================

Link Failure:
- Failure Start: t = 60 seconds
- Failure End: t = 120 seconds
- Affected Link: seRouter <-> coreRouter

Traffic Profiles:
- CBR: 512B packets, 0.1s interval
- Bursty: 10 packets back-to-back, 1s silence

===========================================
CONTACT:
===========================================
For any issues, contact any group member.
===========================================