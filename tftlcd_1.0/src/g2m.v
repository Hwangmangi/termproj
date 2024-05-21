`timescale 1ns / 1ps

//
// TFT-LCD Clock ����
// �Է� Clock�� 2���� ��Ŵ
//

module g2m(
  input clk,
  input rstn,
  output reg opclk);
    
  always@(negedge clk or negedge rstn)
  begin
    if(!rstn)
        opclk <= 0;
    else
        opclk <= ~opclk;
  end
    
endmodule
