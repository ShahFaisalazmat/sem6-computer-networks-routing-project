package campusnetwork;

// TrafficGen: Custom UDP traffic generator.
// Supports CBR and Bursty traffic profiles.
// Gate labels match INET's UdpSocket wiring convention so that
// StandardHost can automatically connect socketIn/socketOut to the Udp module.

simple TrafficGen like inet.applications.contract.IApp
{
    parameters:
        string trafficMode   = default("cbr");     // "cbr" or "bursty"
        string destAddress   = default("");        // destination hostname or IP
        int    destPort      = default(5000);
        int    localPort     = default(4000);
        int    packetSize @unit(B) = default(512B);
        double sendInterval @unit(s) = default(0.1s);  // CBR: gap between packets
        int    burstSize     = default(10);             // Bursty: packets per burst
        double burstGap @unit(s)  = default(1s);       // Bursty: silence between bursts
        double startTime @unit(s) = default(20s);
        double stopTime  @unit(s) = default(170s);

        @display("i=block/source");
        @class(TrafficGen);

        @signal[packetSent](type=long);
        @signal[packetReceived](type=long);
        @signal[endToEndDelay](type=simtime_t);

        @statistic[packetSent](title="Packets Sent"; source=packetSent; record=count,vector);
        @statistic[packetReceived](title="Packets Received"; source=packetReceived; record=count,vector);
        @statistic[endToEndDelay](title="End-to-End Delay"; source=endToEndDelay; record=mean,max,vector; unit=s);

    gates:
        input  socketIn  @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);
}
