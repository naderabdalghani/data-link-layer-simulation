package data_link_layer_simulation;

simple Node
{
    parameters:
        double lambda = default(2.0);
    gates:
        input ins[];
        output outs[];
}