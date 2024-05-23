module button_state(
input wire rst_n,
input wire clk,

input wire [3:0] control,

output reg [3:0] state);

parameter   POC = 4'b0000, //piece or captive, ?궡 留먯쓣 ??吏곸씪嫄댁? ?룷濡쒕?? 諛곗튂?븷嫄댁? ?꽑?깮
            WP = 4'b0001, //which piece, ?뼱?뼡 留먯쓣 ?꽑?깮?븷吏?
            WC = 4'b0010, //which captive, ?뼱?뼡 ?룷濡쒕?? ?꽑?깮?븷吏?
            PD = 4'b0101, //piece direction, ?궡 留먯쓣 ?뼱?뒓 諛⑺뼢?쑝濡? ??吏곸씪吏?
            CP = 4'b0110; //captive position, ?룷濡쒕?? ?뼱?뵒?뿉?떎媛? 諛곗튂瑜? ?븷寃껋씤吏?
            
            
reg [3:0] currentstate, nextstate;

always @(currentstate or control)
begin
    case(currentstate)
        POC : begin
                if(control==4'b1000) // 4踰덉㎏ 鍮꾪듃媛? 踰꾪듉0?씠?씪媛??젙
                    nextstate = WP;
                else if(control==4'b0100) // 3踰덉㎏ 鍮꾪듃媛? 踰꾪듉 1?씠?씪 媛??젙
                    nextstate = WC;
                else
                    nextstate = currentstate;
                //state = nextstate; // 異쒕젰?쑝濡? ?쁽?옱 state瑜? 諛쏅뒗?떎
                    
               end
        WP : begin
                if((|control)==1)
                    nextstate=PD;
                else 
                    nextstate=currentstate;
                //state = nextstate;
                    
              end
        PD : begin
                if((|control)==1)
                    nextstate=POC;
                else 
                    nextstate=currentstate;
                //state = nextstate;
              end
        WC : begin
                if((|control)==1)
                    nextstate=CP;
                else 
                    nextstate=currentstate;
                //state = nextstate;
                    
              end
        CP : begin
                if((|control)==1)
                    nextstate=POC;
                else 
                    nextstate=currentstate;
                //state = nextstate;
              end
        default : begin
                    nextstate=POC;
                  end
    endcase
end
    always @(posedge clk or negedge rst_n)
    begin
        if(rst_n==1'b0)
            currentstate<=POC;
        else
            currentstate<=nextstate;
            
    end
    always @(posedge clk or negedge rst_n)
    begin
    if (rst_n == 1'b0)
        state <= POC;
    else
        state <= currentstate;
    end
 endmodule
 
              