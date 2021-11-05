
// 1+...+100の和を求める
	@i	//iはメモリの特定の場所を参照している
	M=1	//i=1
	@sum	//sumはメモリの特定の場所を指している
	M=0	//sum=0
(LOOP)
	@i	
	D=M	//D=i
        @100
        D=D-A   //D=i-100
        @END
        D;JGT	//i>100ならばENDへジャンプ
	@i
	D=M	//D=i
	@sum
	M=M+D	//sum=sum+i
	@i
	M=M+1	//i++
	@LOOP
	0;JMP	//LOOPへジャンプ
(END)
	@END
	0;JMP	//無限ループ
