module sram4096X8Dp ( input wire        clockA,
                                        writeEnableA,
                      input wire [11:0] addressA,
                      input wire [7:0]  dataInA,
                      output reg [7:0]  dataOutA,
                      input wire        clockB,
                                        writeEnableB,
                      input wire [11:0] addressB,
                      input wire [7:0]  dataInB,
                      output reg [7:0]  dataOutB);

  reg [7:0] memory [4095:0];
  
  always @ (posedge clockA)
    begin
      if (writeEnableA == 1'b1) memory[addressA] <= dataInA;
      dataOutA <= memory[addressA];
    end

  always @ (posedge clockB)
    begin
      if (writeEnableB == 1'b1) memory[addressB] <= dataInB;
      dataOutB <= memory[addressB];
    end
endmodule
