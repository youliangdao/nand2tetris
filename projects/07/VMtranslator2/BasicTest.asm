// push constant
@10
D=A
@SP
A=M
M=D
@SP
M=M+1
@SP
M=M-1
@0
D=A
@LCL
D=D+M
@R13
M=D
@SP
A=M
D=M
@R13
A=M
M=D
// push constant
@21
D=A
@SP
A=M
M=D
@SP
M=M+1
// push constant
@22
D=A
@SP
A=M
M=D
@SP
M=M+1
@SP
M=M-1
@2
D=A
@ARG
D=D+M
@R13
M=D
@SP
A=M
D=M
@R13
A=M
M=D
@SP
M=M-1
@1
D=A
@ARG
D=D+M
@R13
M=D
@SP
A=M
D=M
@R13
A=M
M=D
// push constant
@36
D=A
@SP
A=M
M=D
@SP
M=M+1
@SP
M=M-1
@6
D=A
@THIS
D=D+M
@R13
M=D
@SP
A=M
D=M
@R13
A=M
M=D
// push constant
@42
D=A
@SP
A=M
M=D
@SP
M=M+1
// push constant
@45
D=A
@SP
A=M
M=D
@SP
M=M+1
@SP
M=M-1
@5
D=A
@THAT
D=D+M
@R13
M=D
@SP
A=M
D=M
@R13
A=M
M=D
@SP
M=M-1
@2
D=A
@THAT
D=D+M
@R13
M=D
@SP
A=M
D=M
@R13
A=M
M=D
// push constant
@510
D=A
@SP
A=M
M=D
@SP
M=M+1
@SP
M=M-1
@SP
A=M
D=M
@R11
M=D
// push symbol LCL 0
@0
D=A
@LCL
D=D+M
@R13
M=D
A=M
D=M
@SP
A=M
M=D
@SP
M=M+1
// push symbol THAT 5
@5
D=A
@THAT
D=D+M
@R13
M=D
A=M
D=M
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
// push symbol ARG 1
@1
D=A
@ARG
D=D+M
@R13
M=D
A=M
D=M
@SP
A=M
M=D
@SP
M=M+1
// BinaryOperation M-D
@SP
M=M-1
A=M
D=M
@SP
M=M-1
A=M
M=M-D
@SP
M=M+1
// push symbol THIS 6
@6
D=A
@THIS
D=D+M
@R13
M=D
A=M
D=M
@SP
A=M
M=D
@SP
M=M+1
// push symbol THIS 6
@6
D=A
@THIS
D=D+M
@R13
M=D
A=M
D=M
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
// BinaryOperation M-D
@SP
M=M-1
A=M
D=M
@SP
M=M-1
A=M
M=M-D
@SP
M=M+1
@R11
D=M
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