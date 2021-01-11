package data_link_layer_simulation;

simple Node
{
    parameters:
        double duplicatingProbability = default(50.0);
    gates:
        input ins[];
        output outs[];
}