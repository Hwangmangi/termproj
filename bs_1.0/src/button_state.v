module button_state(
input wire rst_n,
input wire clk,

input wire [3:0] control,

output reg [3:0] state);

  reg [3:0] state_out;
    
	parameter   q1=4'b0001,
             q2=4'b0010,
             q3=4'b0011,
             q4=4'b0100,
             q5=4'b0101,
             q6=4'b0110,
             q7=4'b0111,
             q8=4'b1000,
             q9=4'b1001,
             q10=4'b1010,
             end1=4'b1011; 
            
         
reg [3:0] currentstate, nextstate;

always @(currentstate or control)
begin
    case(currentstate)
        q1 : begin
                if((control==4'b0001)||(control==4'b0010)||(control==4'b0100)||(control==4'b1000)) 
                    nextstate=q2;
                else 
                    nextstate=currentstate;
                      
             end
        q2 : begin
                if((control==4'b0001)||(control==4'b0010)||(control==4'b0100)||(control==4'b1000)) 
                    nextstate=q3;
   
                else 
                    nextstate=currentstate;
                     
             end
        q3 : begin
                if((control==4'b0001)||(control==4'b0010)||(control==4'b0100)||(control==4'b1000)) 
                    nextstate=q4;
                else 
                    nextstate=currentstate;
                 
             end
        q4 : begin
                if((control==4'b0001)||(control==4'b0010)||(control==4'b0100)||(control==4'b1000)) 
                    nextstate=q5;
        
                else 
                    nextstate=currentstate;                      
             end
        q5 : begin
                if((control==4'b0001)||(control==4'b0010)||(control==4'b0100)||(control==4'b1000))
                    nextstate=q6;
      
                else 
                    nextstate=currentstate;
                     
             end
        q6 : begin
                if((control==4'b0001)||(control==4'b0010)||(control==4'b0100)||(control==4'b1000)) 
                    nextstate=q7;
  
                else 
                    nextstate=currentstate;
                   
             end
        q7 : begin
                if((control==4'b0001)||(control==4'b0010)||(control==4'b0100)||(control==4'b1000)) 
                    nextstate=q8;
 
                else 
                    nextstate=currentstate;                       
             end
        q8 : begin
                if((control==4'b0001)||(control==4'b0010)||(control==4'b0100)||(control==4'b1000)) 
                    nextstate=q9;
  
                else 
                    nextstate=currentstate;
                       
             end
             
        q9 : begin
                if((control==4'b0001)||(control==4'b0010)||(control==4'b0100)||(control==4'b1000)) 
                    nextstate=q10;

                else 
                    nextstate=currentstate;
             
             end
        q10 : begin
                if((control==4'b0001)||(control==4'b0010)||(control==4'b0100)||(control==4'b1000))
                    nextstate=end1;

                else 
                    nextstate=currentstate;
                  
             end
       end1 : begin
                  if((control==4'b0001)||(control==4'b0010)||(control==4'b0100)||(control==4'b1000)) 
                    nextstate=q1;
       
                else 
                    nextstate=currentstate;                 
             end
                
        default : begin
                    nextstate=currentstate;
                  end
    endcase
end
    always @(posedge clk or negedge rst_n)
    begin
        if(rst_n==1'b0)
            currentstate<=q1;
        else
            currentstate<=nextstate;
            
    end

    always @(posedge clk or negedge rst_n)
    begin
    if (rst_n == 1'b0)
        state <= q1;
    else
        state <= currentstate;
    end
 endmodule
 
              