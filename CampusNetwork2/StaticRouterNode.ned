package campusnetwork;

// StaticRouterNode: compound module wrapping the custom StaticRouter.
// Can be used as an alternative to INET's Router in the static scenario.
// It connects StaticRouter directly to ethg gates (raw Ethernet mode).
module StaticRouterNode
{
    parameters:
        @networkNode;
        @display("i=device/router");
        int numEthInterfaces = default(0);
    gates:
        inout ethg[numEthInterfaces];
    submodules:
        staticRouter: StaticRouter {
            numPorts = sizeof(ethg);
            @display("p=400,150");
        }
    connections:
        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> staticRouter.ethg[i];
        }
}
