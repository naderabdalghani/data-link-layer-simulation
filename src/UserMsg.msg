packet UserMsg {
    @customize(true);
    int type;	// 0-> from hub to nodes first time, 1-> message to node, 2-> ACK for node, 3-> NACk for node
    string payload;
    int line_nr;
    int line_expected;
    bool waiting;
}
