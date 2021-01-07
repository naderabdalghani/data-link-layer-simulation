package data_link_layer_simulation;

import ned.IdealChannel;

network CentralizedNetwork
{
    parameters:
        int n = default(5);
        @display("bgb=415,315");
    submodules:
        nodes[n]: Node;
        hub: Node;
    connections:
        for i=0..sizeof(nodes)-1 {
            nodes[i].outs++ --> IdealChannel --> hub.ins++;
            hub.outs++ --> IdealChannel --> nodes[i].ins++;
        }
}
