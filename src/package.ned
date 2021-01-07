package data_link_layer_simulation;


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
            nodes[i].outs++ --> NoisyChannel --> hub.ins++;
            hub.outs++ --> NoisyChannel --> nodes[i].ins++;
        }
}
