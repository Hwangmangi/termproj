`timescale 1ns/1ps

module tb_button_moore();

reg rst_n;
reg clk;
reg [3:0] control;

wire [3:0] state;

button_moore V0(.rst_n(rst_n), .clk(clk), .control(control), .state(state));


parameter PER100_100MHZ = 10;

always
    #(PER100_100MHZ/2) clk=~clk;
    
initial begin
    rst_n=1'b1;
    clk=1'b0;
    control =4'b0
    
    #40 rst_n=1'b0;
    #20 rst_n=1'b1;
    #40 control=4'b1000;
    #20 control=4'b1000;
    #20 control=4'b1000;
    #40 control=4'b0100;
    #20 control=4'b1000;
    #20 control=4'b1000;
    #200 $stop;
    end
endmodule
