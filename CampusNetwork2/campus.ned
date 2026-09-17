package campusnetwork;

import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;

network CampusNetwork
{
    parameters:
        @display("bgb=900,700");

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=800,50");
        }

        linkController: LinkController {
            @display("p=800,150");
        }

        // Gate index map (backbone connections listed FIRST below):
        //   coreRouter  ethg[0]=seRouter  ethg[1]=eeRouter  ethg[2]=bbaRouter
        //               ethg[3]=adminRouter  ethg[4]=dnsServer  ethg[5]=appServer
        //   seRouter    ethg[0]=coreRouter   ethg[1..3]=seHost1..3
        //   eeRouter    ethg[0]=coreRouter   ethg[1..3]=eeHost1..3
        //   bbaRouter   ethg[0]=coreRouter   ethg[1..3]=bbaHost1..3
        //   adminRouter ethg[0]=coreRouter   ethg[1..3]=adminHost1..3
        coreRouter: Router {
            @display("p=450,350;i=device/router");
        }

        seRouter: Router {
            @display("p=200,200;i=device/router");
        }
        eeRouter: Router {
            @display("p=700,200;i=device/router");
        }
        bbaRouter: Router {
            @display("p=200,500;i=device/router");
        }
        adminRouter: Router {
            @display("p=700,500;i=device/router");
        }

        dnsServer: StandardHost {
            @display("p=350,150;i=device/server");
        }
        appServer: StandardHost {
            @display("p=550,150;i=device/server");
        }

        seHost1: StandardHost { @display("p=50,100;i=device/pc"); }
        seHost2: StandardHost { @display("p=150,100;i=device/pc"); }
        seHost3: StandardHost { @display("p=250,100;i=device/pc"); }

        eeHost1: StandardHost { @display("p=600,100;i=device/pc"); }
        eeHost2: StandardHost { @display("p=700,100;i=device/pc"); }
        eeHost3: StandardHost { @display("p=800,100;i=device/pc"); }

        bbaHost1: StandardHost { @display("p=50,600;i=device/pc"); }
        bbaHost2: StandardHost { @display("p=150,600;i=device/pc"); }
        bbaHost3: StandardHost { @display("p=250,600;i=device/pc"); }

        adminHost1: StandardHost { @display("p=600,600;i=device/pc"); }
        adminHost2: StandardHost { @display("p=700,600;i=device/pc"); }
        adminHost3: StandardHost { @display("p=800,600;i=device/pc"); }

    connections:
        // === BACKBONE FIRST — ensures ethg[0] = backbone link on every router ===
        seRouter.ethg++    <--> { datarate=100Mbps; delay=2ms; per=1e-3; } <--> coreRouter.ethg++;
        eeRouter.ethg++    <--> { datarate=100Mbps; delay=2ms; per=1e-3; } <--> coreRouter.ethg++;
        bbaRouter.ethg++   <--> { datarate=100Mbps; delay=2ms; per=1e-3; } <--> coreRouter.ethg++;
        adminRouter.ethg++ <--> { datarate=100Mbps; delay=2ms; per=1e-3; } <--> coreRouter.ethg++;

        // Server links
        dnsServer.ethg++ <--> { datarate=100Mbps; delay=1ms; } <--> coreRouter.ethg++;
        appServer.ethg++ <--> { datarate=100Mbps; delay=1ms; } <--> coreRouter.ethg++;

        // SE access links
        seHost1.ethg++ <--> { datarate=10Mbps; delay=1ms; } <--> seRouter.ethg++;
        seHost2.ethg++ <--> { datarate=10Mbps; delay=1ms; } <--> seRouter.ethg++;
        seHost3.ethg++ <--> { datarate=10Mbps; delay=1ms; } <--> seRouter.ethg++;

        // EE access links
        eeHost1.ethg++ <--> { datarate=10Mbps; delay=1ms; } <--> eeRouter.ethg++;
        eeHost2.ethg++ <--> { datarate=10Mbps; delay=1ms; } <--> eeRouter.ethg++;
        eeHost3.ethg++ <--> { datarate=10Mbps; delay=1ms; } <--> eeRouter.ethg++;

        // BBA access links
        bbaHost1.ethg++ <--> { datarate=10Mbps; delay=1ms; } <--> bbaRouter.ethg++;
        bbaHost2.ethg++ <--> { datarate=10Mbps; delay=1ms; } <--> bbaRouter.ethg++;
        bbaHost3.ethg++ <--> { datarate=10Mbps; delay=1ms; } <--> bbaRouter.ethg++;

        // Admin access links
        adminHost1.ethg++ <--> { datarate=10Mbps; delay=1ms; } <--> adminRouter.ethg++;
        adminHost2.ethg++ <--> { datarate=10Mbps; delay=1ms; } <--> adminRouter.ethg++;
        adminHost3.ethg++ <--> { datarate=10Mbps; delay=1ms; } <--> adminRouter.ethg++;
}
