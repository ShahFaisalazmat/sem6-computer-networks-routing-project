package campusnetwork;

// StaticRouter: Custom static routing module.
// Receives Ethernet frames directly, handles ARP requests,
// performs IPv4 longest-prefix-match, and forwards packets.
// Uses broadcast MAC for outgoing frames to avoid ARP dependency.
simple StaticRouter
{
    parameters:
        string ipAddress = default("0.0.0.0");
        string macAddress = default("00:00:00:00:00:01");
        string interfaceAddresses = default("");  // semicolon-separated IPs per port, e.g. "192.168.1.1;10.1.1.2"
        string routeTable = default("");         // semicolon-separated routes, e.g. "192.168.2.0/24 via 1; 0.0.0.0/0 via 1"
        int numPorts = default(0);

        @display("i=device/router");
        @class(StaticRouter);

        @signal[packetForwarded](type=long);
        @signal[packetDropped](type=long);

        @statistic[packetForwarded](title="Packets Forwarded"; source=packetForwarded; record=count,vector);
        @statistic[packetDropped](title="Packets Dropped"; source=packetDropped; record=count,vector);

    gates:
        inout ethg[];
}
