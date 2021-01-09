package data_link_layer_simulation;


channel NoisyChannel
{
    parameters:
        double delayMin = default(1.0);
        double delayMax = default(5.0);
        double delayProbability = default(50.0);
        double discardingProbability = default(50.0);
}
