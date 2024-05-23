module vending_machine(
input wire rst_n,
input wire clk,

input wire [3:0] control,

output reg [3:0] state);

parameter   POC = 4'b0000, //piece or captive, 내 말을 움직일건지 포로를 배치할건지 선택
            WP = 4'b0001, //which piece, 어떤 말을 선택할지
            WC = 4'b0010, //which captive, 어떤 포로를 선택할지
            PD = 4'b0101, //piece direction, 내 말을 어느 방향으로 움직일지
            CP = 4'b0110; //captive position, 포로를 어디에다가 배치를 할것인지
            
            
reg [3:0] currentstate, nextstate;

always @(currentstate or control)
begin
    case(currentstate)
        POC : begin
                if(control==4'b1000) // 4번째 비트가 버튼0이라가정
                    nextstate = WP;
                else if(control==4'b0100) // 3번째 비트가 버튼 1이라 가정
                    nextstate = WC;
                else
                    nextstate = currentstate;
                state = nextstate; // 출력으로 현재 state를 받는다
                    
               end
        WP : begin
                if((&control)==1)
                    nextstate=PD;
                else 
                    nextstate=currentstate;
                state = nextstate;
                    
              end
        PD : begin
                if((&control)==1)
                    nextstate=POC;
                else 
                    nextstate=currentstate;
                state = nextstate;
              end
        WC : begin
                if((&control)==1)
                    nextstate=CP;
                else 
                    nextstate=currentstate;
                state = nextstate;
                    
              end
        CP : begin
                if((&control)==1)
                    nextstate=POC;
                else 
                    nextstate=currentstate;
                state = nextstate;
              end
        default : begin
                    nextstate=IDLE;
                    beverage=1'b0;
                  end
    endcase
end
    always @(posedge clk or negedge rst_n)
    begin
        if(rst_n==1'b0)
            currentstate<=IDLE;
        else
            currentstate<=nextstate;
    end
 endmodule
 
              