// push constant
@7
D=A
@SP
A=M
M=D
@SP
M=M+1
// push constant
@8
D=A
@SP
A=M
M=D
@SP
M=M+1
// BinaryOperation D+M
@SP
M=M-1
A=M
D=M
@SP
M=M-1
A=M
M=D+M
@SP
M=M+1
