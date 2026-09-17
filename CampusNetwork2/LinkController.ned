package campusnetwork;

// LinkController: triggers SE-to-Core link failure at t=60s
// and recovery at t=120s by directly manipulating OMNeT++ gates.
// Has no network gates — it only uses self-messages and getModuleByPath().
simple LinkController
{
    parameters:
        bool enabled = default(true);
        @display("i=block/control");
        @class(LinkController);

    gates:
        // No network connections — uses self-scheduled messages only
}
