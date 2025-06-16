
race.riscv:     file format elf64-littleriscv


Disassembly of section .text.init:

0000000080000000 <_start>:
    80000000:	4081                	li	ra,0
    80000002:	4101                	li	sp,0
    80000004:	4181                	li	gp,0
    80000006:	4201                	li	tp,0
    80000008:	4281                	li	t0,0
    8000000a:	4301                	li	t1,0
    8000000c:	4381                	li	t2,0
    8000000e:	4401                	li	s0,0
    80000010:	4481                	li	s1,0
    80000012:	4501                	li	a0,0
    80000014:	4581                	li	a1,0
    80000016:	4601                	li	a2,0
    80000018:	4681                	li	a3,0
    8000001a:	4701                	li	a4,0
    8000001c:	4781                	li	a5,0
    8000001e:	4801                	li	a6,0
    80000020:	4881                	li	a7,0
    80000022:	4901                	li	s2,0
    80000024:	4981                	li	s3,0
    80000026:	4a01                	li	s4,0
    80000028:	4a81                	li	s5,0
    8000002a:	4b01                	li	s6,0
    8000002c:	4b81                	li	s7,0
    8000002e:	4c01                	li	s8,0
    80000030:	4c81                	li	s9,0
    80000032:	4d01                	li	s10,0
    80000034:	4d81                	li	s11,0
    80000036:	4e01                	li	t3,0
    80000038:	4e81                	li	t4,0
    8000003a:	4f01                	li	t5,0
    8000003c:	4f81                	li	t6,0
    8000003e:	62f9                	lui	t0,0x1e
    80000040:	3002a073          	csrs	mstatus,t0
    80000044:	4285                	li	t0,1
    80000046:	02fe                	slli	t0,t0,0x1f
    80000048:	0002d863          	bgez	t0,80000058 <_start+0x58>
    8000004c:	4505                	li	a0,1
    8000004e:	00001297          	auipc	t0,0x1
    80000052:	faa2a923          	sw	a0,-78(t0) # 80001000 <tohost>
    80000056:	bfdd                	j	8000004c <_start+0x4c>
    80000058:	00000297          	auipc	t0,0x0
    8000005c:	09028293          	addi	t0,t0,144 # 800000e8 <_start+0xe8>
    80000060:	30529073          	csrw	mtvec,t0
    80000064:	00301073          	fscsr	zero
    80000068:	f0000053          	fmv.w.x	ft0,zero
    8000006c:	f00000d3          	fmv.w.x	ft1,zero
    80000070:	f0000153          	fmv.w.x	ft2,zero
    80000074:	f00001d3          	fmv.w.x	ft3,zero
    80000078:	f0000253          	fmv.w.x	ft4,zero
    8000007c:	f00002d3          	fmv.w.x	ft5,zero
    80000080:	f0000353          	fmv.w.x	ft6,zero
    80000084:	f00003d3          	fmv.w.x	ft7,zero
    80000088:	f0000453          	fmv.w.x	fs0,zero
    8000008c:	f00004d3          	fmv.w.x	fs1,zero
    80000090:	f0000553          	fmv.w.x	fa0,zero
    80000094:	f00005d3          	fmv.w.x	fa1,zero
    80000098:	f0000653          	fmv.w.x	fa2,zero
    8000009c:	f00006d3          	fmv.w.x	fa3,zero
    800000a0:	f0000753          	fmv.w.x	fa4,zero
    800000a4:	f00007d3          	fmv.w.x	fa5,zero
    800000a8:	f0000853          	fmv.w.x	fa6,zero
    800000ac:	f00008d3          	fmv.w.x	fa7,zero
    800000b0:	f0000953          	fmv.w.x	fs2,zero
    800000b4:	f00009d3          	fmv.w.x	fs3,zero
    800000b8:	f0000a53          	fmv.w.x	fs4,zero
    800000bc:	f0000ad3          	fmv.w.x	fs5,zero
    800000c0:	f0000b53          	fmv.w.x	fs6,zero
    800000c4:	f0000bd3          	fmv.w.x	fs7,zero
    800000c8:	f0000c53          	fmv.w.x	fs8,zero
    800000cc:	f0000cd3          	fmv.w.x	fs9,zero
    800000d0:	f0000d53          	fmv.w.x	fs10,zero
    800000d4:	f0000dd3          	fmv.w.x	fs11,zero
    800000d8:	f0000e53          	fmv.w.x	ft8,zero
    800000dc:	f0000ed3          	fmv.w.x	ft9,zero
    800000e0:	f0000f53          	fmv.w.x	ft10,zero
    800000e4:	f0000fd3          	fmv.w.x	ft11,zero
    800000e8:	00000297          	auipc	t0,0x0
    800000ec:	03c28293          	addi	t0,t0,60 # 80000124 <trap_entry>
    800000f0:	30529073          	csrw	mtvec,t0
    800000f4:	00003197          	auipc	gp,0x3
    800000f8:	eac18193          	addi	gp,gp,-340 # 80002fa0 <__global_pointer$>
    800000fc:	00003217          	auipc	tp,0x3
    80000100:	70320213          	addi	tp,tp,1795 # 800037ff <_end+0x3f>
    80000104:	fc027213          	andi	tp,tp,-64
    80000108:	f1402573          	csrr	a0,mhartid
    8000010c:	4589                	li	a1,2
    8000010e:	00b57063          	bgeu	a0,a1,8000010e <_start+0x10e>
    80000112:	00150113          	addi	sp,a0,1
    80000116:	0146                	slli	sp,sp,0x11
    80000118:	9112                	add	sp,sp,tp
    8000011a:	01151613          	slli	a2,a0,0x11
    8000011e:	9232                	add	tp,tp,a2
    80000120:	3100206f          	j	80002430 <_init>

0000000080000124 <trap_entry>:
    80000124:	716d                	addi	sp,sp,-272
    80000126:	e406                	sd	ra,8(sp)
    80000128:	e80a                	sd	sp,16(sp)
    8000012a:	ec0e                	sd	gp,24(sp)
    8000012c:	f012                	sd	tp,32(sp)
    8000012e:	f416                	sd	t0,40(sp)
    80000130:	f81a                	sd	t1,48(sp)
    80000132:	fc1e                	sd	t2,56(sp)
    80000134:	e0a2                	sd	s0,64(sp)
    80000136:	e4a6                	sd	s1,72(sp)
    80000138:	e8aa                	sd	a0,80(sp)
    8000013a:	ecae                	sd	a1,88(sp)
    8000013c:	f0b2                	sd	a2,96(sp)
    8000013e:	f4b6                	sd	a3,104(sp)
    80000140:	f8ba                	sd	a4,112(sp)
    80000142:	fcbe                	sd	a5,120(sp)
    80000144:	e142                	sd	a6,128(sp)
    80000146:	e546                	sd	a7,136(sp)
    80000148:	e94a                	sd	s2,144(sp)
    8000014a:	ed4e                	sd	s3,152(sp)
    8000014c:	f152                	sd	s4,160(sp)
    8000014e:	f556                	sd	s5,168(sp)
    80000150:	f95a                	sd	s6,176(sp)
    80000152:	fd5e                	sd	s7,184(sp)
    80000154:	e1e2                	sd	s8,192(sp)
    80000156:	e5e6                	sd	s9,200(sp)
    80000158:	e9ea                	sd	s10,208(sp)
    8000015a:	edee                	sd	s11,216(sp)
    8000015c:	f1f2                	sd	t3,224(sp)
    8000015e:	f5f6                	sd	t4,232(sp)
    80000160:	f9fa                	sd	t5,240(sp)
    80000162:	fdfe                	sd	t6,248(sp)
    80000164:	34202573          	csrr	a0,mcause
    80000168:	341025f3          	csrr	a1,mepc
    8000016c:	860a                	mv	a2,sp
    8000016e:	23e020ef          	jal	ra,800023ac <handle_trap>
    80000172:	34151073          	csrw	mepc,a0
    80000176:	000022b7          	lui	t0,0x2
    8000017a:	8002829b          	addiw	t0,t0,-2048 # 1800 <buflen.3027+0x17c0>
    8000017e:	3002a073          	csrs	mstatus,t0
    80000182:	60a2                	ld	ra,8(sp)
    80000184:	6142                	ld	sp,16(sp)
    80000186:	61e2                	ld	gp,24(sp)
    80000188:	7202                	ld	tp,32(sp)
    8000018a:	72a2                	ld	t0,40(sp)
    8000018c:	7342                	ld	t1,48(sp)
    8000018e:	73e2                	ld	t2,56(sp)
    80000190:	6406                	ld	s0,64(sp)
    80000192:	64a6                	ld	s1,72(sp)
    80000194:	6546                	ld	a0,80(sp)
    80000196:	65e6                	ld	a1,88(sp)
    80000198:	7606                	ld	a2,96(sp)
    8000019a:	76a6                	ld	a3,104(sp)
    8000019c:	7746                	ld	a4,112(sp)
    8000019e:	77e6                	ld	a5,120(sp)
    800001a0:	680a                	ld	a6,128(sp)
    800001a2:	68aa                	ld	a7,136(sp)
    800001a4:	694a                	ld	s2,144(sp)
    800001a6:	69ea                	ld	s3,152(sp)
    800001a8:	7a0a                	ld	s4,160(sp)
    800001aa:	7aaa                	ld	s5,168(sp)
    800001ac:	7b4a                	ld	s6,176(sp)
    800001ae:	7bea                	ld	s7,184(sp)
    800001b0:	6c0e                	ld	s8,192(sp)
    800001b2:	6cae                	ld	s9,200(sp)
    800001b4:	6d4e                	ld	s10,208(sp)
    800001b6:	6dee                	ld	s11,216(sp)
    800001b8:	7e0e                	ld	t3,224(sp)
    800001ba:	7eae                	ld	t4,232(sp)
    800001bc:	7f4e                	ld	t5,240(sp)
    800001be:	7fee                	ld	t6,248(sp)
    800001c0:	6151                	addi	sp,sp,272
    800001c2:	30200073          	mret

Disassembly of section .text:

0000000080002000 <vprintfmt>:
    80002000:	710d                	addi	sp,sp,-352
    80002002:	eaa2                	sd	s0,336(sp)
    80002004:	e6a6                	sd	s1,328(sp)
    80002006:	e2ca                	sd	s2,320(sp)
    80002008:	fa52                	sd	s4,304(sp)
    8000200a:	f25a                	sd	s6,288(sp)
    8000200c:	ee5e                	sd	s7,280(sp)
    8000200e:	ee86                	sd	ra,344(sp)
    80002010:	fe4e                	sd	s3,312(sp)
    80002012:	f656                	sd	s5,296(sp)
    80002014:	ea62                	sd	s8,272(sp)
    80002016:	e666                	sd	s9,264(sp)
    80002018:	e26a                	sd	s10,256(sp)
    8000201a:	892a                	mv	s2,a0
    8000201c:	84ae                	mv	s1,a1
    8000201e:	8432                	mv	s0,a2
    80002020:	8bb6                	mv	s7,a3
    80002022:	02500a13          	li	s4,37
    80002026:	00000b17          	auipc	s6,0x0
    8000202a:	61ab0b13          	addi	s6,s6,1562 # 80002640 <main+0x6>
    8000202e:	a029                	j	80002038 <vprintfmt+0x38>
    80002030:	c131                	beqz	a0,80002074 <vprintfmt+0x74>
    80002032:	85a6                	mv	a1,s1
    80002034:	0405                	addi	s0,s0,1
    80002036:	9902                	jalr	s2
    80002038:	00044503          	lbu	a0,0(s0)
    8000203c:	ff451ae3          	bne	a0,s4,80002030 <vprintfmt+0x30>
    80002040:	00144683          	lbu	a3,1(s0)
    80002044:	00140a93          	addi	s5,s0,1
    80002048:	8756                	mv	a4,s5
    8000204a:	02000c13          	li	s8,32
    8000204e:	59fd                	li	s3,-1
    80002050:	5cfd                	li	s9,-1
    80002052:	4501                	li	a0,0
    80002054:	05500613          	li	a2,85
    80002058:	45a5                	li	a1,9
    8000205a:	fdd6879b          	addiw	a5,a3,-35
    8000205e:	0ff7f793          	zext.b	a5,a5
    80002062:	00170413          	addi	s0,a4,1
    80002066:	1cf66f63          	bltu	a2,a5,80002244 <vprintfmt+0x244>
    8000206a:	078a                	slli	a5,a5,0x2
    8000206c:	97da                	add	a5,a5,s6
    8000206e:	439c                	lw	a5,0(a5)
    80002070:	97da                	add	a5,a5,s6
    80002072:	8782                	jr	a5
    80002074:	60f6                	ld	ra,344(sp)
    80002076:	6456                	ld	s0,336(sp)
    80002078:	64b6                	ld	s1,328(sp)
    8000207a:	6916                	ld	s2,320(sp)
    8000207c:	79f2                	ld	s3,312(sp)
    8000207e:	7a52                	ld	s4,304(sp)
    80002080:	7ab2                	ld	s5,296(sp)
    80002082:	7b12                	ld	s6,288(sp)
    80002084:	6bf2                	ld	s7,280(sp)
    80002086:	6c52                	ld	s8,272(sp)
    80002088:	6cb2                	ld	s9,264(sp)
    8000208a:	6d12                	ld	s10,256(sp)
    8000208c:	6135                	addi	sp,sp,352
    8000208e:	8082                	ret
    80002090:	4721                	li	a4,8
    80002092:	4785                	li	a5,1
    80002094:	008b8693          	addi	a3,s7,8
    80002098:	08a7c763          	blt	a5,a0,80002126 <vprintfmt+0x126>
    8000209c:	e549                	bnez	a0,80002126 <vprintfmt+0x126>
    8000209e:	000be783          	lwu	a5,0(s7)
    800020a2:	8bb6                	mv	s7,a3
    800020a4:	02e7f9b3          	remu	s3,a5,a4
    800020a8:	2c01                	sext.w	s8,s8
    800020aa:	0054                	addi	a3,sp,4
    800020ac:	4a85                	li	s5,1
    800020ae:	c04e                	sw	s3,0(sp)
    800020b0:	00e7f463          	bgeu	a5,a4,800020b8 <vprintfmt+0xb8>
    800020b4:	aaf9                	j	80002292 <vprintfmt+0x292>
    800020b6:	8aea                	mv	s5,s10
    800020b8:	02e7d7b3          	divu	a5,a5,a4
    800020bc:	0691                	addi	a3,a3,4
    800020be:	001a8d1b          	addiw	s10,s5,1
    800020c2:	02e7f9b3          	remu	s3,a5,a4
    800020c6:	ff36ae23          	sw	s3,-4(a3)
    800020ca:	fee7f6e3          	bgeu	a5,a4,800020b6 <vprintfmt+0xb6>
    800020ce:	019d5963          	bge	s10,s9,800020e0 <vprintfmt+0xe0>
    800020d2:	2c81                	sext.w	s9,s9
    800020d4:	3cfd                	addiw	s9,s9,-1
    800020d6:	85a6                	mv	a1,s1
    800020d8:	8562                	mv	a0,s8
    800020da:	9902                	jalr	s2
    800020dc:	ff9d4ce3          	blt	s10,s9,800020d4 <vprintfmt+0xd4>
    800020e0:	002a9c13          	slli	s8,s5,0x2
    800020e4:	01810ab3          	add	s5,sp,s8
    800020e8:	418a8c33          	sub	s8,s5,s8
    800020ec:	4ca5                	li	s9,9
    800020ee:	a021                	j	800020f6 <vprintfmt+0xf6>
    800020f0:	ffcaa983          	lw	s3,-4(s5)
    800020f4:	1af1                	addi	s5,s5,-4
    800020f6:	05700513          	li	a0,87
    800020fa:	013ce463          	bltu	s9,s3,80002102 <vprintfmt+0x102>
    800020fe:	03000513          	li	a0,48
    80002102:	85a6                	mv	a1,s1
    80002104:	0135053b          	addw	a0,a0,s3
    80002108:	9902                	jalr	s2
    8000210a:	ff5c13e3          	bne	s8,s5,800020f0 <vprintfmt+0xf0>
    8000210e:	b72d                	j	80002038 <vprintfmt+0x38>
    80002110:	03000513          	li	a0,48
    80002114:	85a6                	mv	a1,s1
    80002116:	9902                	jalr	s2
    80002118:	85a6                	mv	a1,s1
    8000211a:	07800513          	li	a0,120
    8000211e:	9902                	jalr	s2
    80002120:	008b8693          	addi	a3,s7,8
    80002124:	4741                	li	a4,16
    80002126:	000bb783          	ld	a5,0(s7)
    8000212a:	8bb6                	mv	s7,a3
    8000212c:	bfa5                	j	800020a4 <vprintfmt+0xa4>
    8000212e:	000bba83          	ld	s5,0(s7)
    80002132:	0ba1                	addi	s7,s7,8
    80002134:	180a8163          	beqz	s5,800022b6 <vprintfmt+0x2b6>
    80002138:	15905763          	blez	s9,80002286 <vprintfmt+0x286>
    8000213c:	02d00793          	li	a5,45
    80002140:	10fc1c63          	bne	s8,a5,80002258 <vprintfmt+0x258>
    80002144:	000ac503          	lbu	a0,0(s5)
    80002148:	c105                	beqz	a0,80002168 <vprintfmt+0x168>
    8000214a:	5c7d                	li	s8,-1
    8000214c:	0009c563          	bltz	s3,80002156 <vprintfmt+0x156>
    80002150:	39fd                	addiw	s3,s3,-1
    80002152:	01898963          	beq	s3,s8,80002164 <vprintfmt+0x164>
    80002156:	85a6                	mv	a1,s1
    80002158:	0a85                	addi	s5,s5,1
    8000215a:	9902                	jalr	s2
    8000215c:	000ac503          	lbu	a0,0(s5)
    80002160:	3cfd                	addiw	s9,s9,-1
    80002162:	f56d                	bnez	a0,8000214c <vprintfmt+0x14c>
    80002164:	ed905ae3          	blez	s9,80002038 <vprintfmt+0x38>
    80002168:	3cfd                	addiw	s9,s9,-1
    8000216a:	85a6                	mv	a1,s1
    8000216c:	02000513          	li	a0,32
    80002170:	9902                	jalr	s2
    80002172:	fe0c9be3          	bnez	s9,80002168 <vprintfmt+0x168>
    80002176:	b5c9                	j	80002038 <vprintfmt+0x38>
    80002178:	4729                	li	a4,10
    8000217a:	bf21                	j	80002092 <vprintfmt+0x92>
    8000217c:	00174683          	lbu	a3,1(a4)
    80002180:	8722                	mv	a4,s0
    80002182:	bde1                	j	8000205a <vprintfmt+0x5a>
    80002184:	85a6                	mv	a1,s1
    80002186:	02500513          	li	a0,37
    8000218a:	9902                	jalr	s2
    8000218c:	b575                	j	80002038 <vprintfmt+0x38>
    8000218e:	4741                	li	a4,16
    80002190:	b709                	j	80002092 <vprintfmt+0x92>
    80002192:	000ba983          	lw	s3,0(s7)
    80002196:	00174683          	lbu	a3,1(a4)
    8000219a:	0ba1                	addi	s7,s7,8
    8000219c:	8722                	mv	a4,s0
    8000219e:	ea0cdee3          	bgez	s9,8000205a <vprintfmt+0x5a>
    800021a2:	8cce                	mv	s9,s3
    800021a4:	59fd                	li	s3,-1
    800021a6:	bd55                	j	8000205a <vprintfmt+0x5a>
    800021a8:	00174683          	lbu	a3,1(a4)
    800021ac:	02d00c13          	li	s8,45
    800021b0:	8722                	mv	a4,s0
    800021b2:	b565                	j	8000205a <vprintfmt+0x5a>
    800021b4:	fffcc793          	not	a5,s9
    800021b8:	97fd                	srai	a5,a5,0x3f
    800021ba:	00fcfcb3          	and	s9,s9,a5
    800021be:	00174683          	lbu	a3,1(a4)
    800021c2:	2c81                	sext.w	s9,s9
    800021c4:	8722                	mv	a4,s0
    800021c6:	bd51                	j	8000205a <vprintfmt+0x5a>
    800021c8:	00174683          	lbu	a3,1(a4)
    800021cc:	03000c13          	li	s8,48
    800021d0:	8722                	mv	a4,s0
    800021d2:	b561                	j	8000205a <vprintfmt+0x5a>
    800021d4:	fd06899b          	addiw	s3,a3,-48
    800021d8:	00174683          	lbu	a3,1(a4)
    800021dc:	8722                	mv	a4,s0
    800021de:	fd06879b          	addiw	a5,a3,-48
    800021e2:	0006881b          	sext.w	a6,a3
    800021e6:	faf5ece3          	bltu	a1,a5,8000219e <vprintfmt+0x19e>
    800021ea:	0705                	addi	a4,a4,1
    800021ec:	0029979b          	slliw	a5,s3,0x2
    800021f0:	00074683          	lbu	a3,0(a4)
    800021f4:	013789bb          	addw	s3,a5,s3
    800021f8:	0019999b          	slliw	s3,s3,0x1
    800021fc:	010989bb          	addw	s3,s3,a6
    80002200:	fd06879b          	addiw	a5,a3,-48
    80002204:	fd09899b          	addiw	s3,s3,-48
    80002208:	0006881b          	sext.w	a6,a3
    8000220c:	fcf5ffe3          	bgeu	a1,a5,800021ea <vprintfmt+0x1ea>
    80002210:	b779                	j	8000219e <vprintfmt+0x19e>
    80002212:	000ba503          	lw	a0,0(s7)
    80002216:	85a6                	mv	a1,s1
    80002218:	0ba1                	addi	s7,s7,8
    8000221a:	9902                	jalr	s2
    8000221c:	bd31                	j	80002038 <vprintfmt+0x38>
    8000221e:	4785                	li	a5,1
    80002220:	008b8a93          	addi	s5,s7,8
    80002224:	00a7c363          	blt	a5,a0,8000222a <vprintfmt+0x22a>
    80002228:	c93d                	beqz	a0,8000229e <vprintfmt+0x29e>
    8000222a:	000bb983          	ld	s3,0(s7)
    8000222e:	0609cb63          	bltz	s3,800022a4 <vprintfmt+0x2a4>
    80002232:	87ce                	mv	a5,s3
    80002234:	8bd6                	mv	s7,s5
    80002236:	4729                	li	a4,10
    80002238:	b5b5                	j	800020a4 <vprintfmt+0xa4>
    8000223a:	00174683          	lbu	a3,1(a4)
    8000223e:	2505                	addiw	a0,a0,1
    80002240:	8722                	mv	a4,s0
    80002242:	bd21                	j	8000205a <vprintfmt+0x5a>
    80002244:	85a6                	mv	a1,s1
    80002246:	02500513          	li	a0,37
    8000224a:	9902                	jalr	s2
    8000224c:	8456                	mv	s0,s5
    8000224e:	b3ed                	j	80002038 <vprintfmt+0x38>
    80002250:	00000a97          	auipc	s5,0x0
    80002254:	548a8a93          	addi	s5,s5,1352 # 80002798 <main+0x15e>
    80002258:	06098d63          	beqz	s3,800022d2 <vprintfmt+0x2d2>
    8000225c:	000ac783          	lbu	a5,0(s5)
    80002260:	cbad                	beqz	a5,800022d2 <vprintfmt+0x2d2>
    80002262:	001a8793          	addi	a5,s5,1
    80002266:	013a8633          	add	a2,s5,s3
    8000226a:	a029                	j	80002274 <vprintfmt+0x274>
    8000226c:	0785                	addi	a5,a5,1
    8000226e:	fff7c703          	lbu	a4,-1(a5)
    80002272:	cb25                	beqz	a4,800022e2 <vprintfmt+0x2e2>
    80002274:	86be                	mv	a3,a5
    80002276:	fef61be3          	bne	a2,a5,8000226c <vprintfmt+0x26c>
    8000227a:	415606bb          	subw	a3,a2,s5
    8000227e:	40dc8cbb          	subw	s9,s9,a3
    80002282:	05904863          	bgtz	s9,800022d2 <vprintfmt+0x2d2>
    80002286:	000ac503          	lbu	a0,0(s5)
    8000228a:	da0507e3          	beqz	a0,80002038 <vprintfmt+0x38>
    8000228e:	5c7d                	li	s8,-1
    80002290:	bd75                	j	8000214c <vprintfmt+0x14c>
    80002292:	4785                	li	a5,1
    80002294:	4a81                	li	s5,0
    80002296:	4d05                	li	s10,1
    80002298:	e397cde3          	blt	a5,s9,800020d2 <vprintfmt+0xd2>
    8000229c:	b591                	j	800020e0 <vprintfmt+0xe0>
    8000229e:	000ba983          	lw	s3,0(s7)
    800022a2:	b771                	j	8000222e <vprintfmt+0x22e>
    800022a4:	85a6                	mv	a1,s1
    800022a6:	02d00513          	li	a0,45
    800022aa:	9902                	jalr	s2
    800022ac:	8bd6                	mv	s7,s5
    800022ae:	413007b3          	neg	a5,s3
    800022b2:	4729                	li	a4,10
    800022b4:	bbc5                	j	800020a4 <vprintfmt+0xa4>
    800022b6:	01905663          	blez	s9,800022c2 <vprintfmt+0x2c2>
    800022ba:	02d00793          	li	a5,45
    800022be:	f8fc19e3          	bne	s8,a5,80002250 <vprintfmt+0x250>
    800022c2:	00000a97          	auipc	s5,0x0
    800022c6:	4d6a8a93          	addi	s5,s5,1238 # 80002798 <main+0x15e>
    800022ca:	02800513          	li	a0,40
    800022ce:	5c7d                	li	s8,-1
    800022d0:	bdb5                	j	8000214c <vprintfmt+0x14c>
    800022d2:	2c01                	sext.w	s8,s8
    800022d4:	3cfd                	addiw	s9,s9,-1
    800022d6:	85a6                	mv	a1,s1
    800022d8:	8562                	mv	a0,s8
    800022da:	9902                	jalr	s2
    800022dc:	fe0c9ce3          	bnez	s9,800022d4 <vprintfmt+0x2d4>
    800022e0:	b75d                	j	80002286 <vprintfmt+0x286>
    800022e2:	415686bb          	subw	a3,a3,s5
    800022e6:	bf61                	j	8000227e <vprintfmt+0x27e>

00000000800022e8 <sprintf_putch.3141>:
    800022e8:	619c                	ld	a5,0(a1)
    800022ea:	00a78023          	sb	a0,0(a5)
    800022ee:	619c                	ld	a5,0(a1)
    800022f0:	0785                	addi	a5,a5,1
    800022f2:	e19c                	sd	a5,0(a1)
    800022f4:	8082                	ret

00000000800022f6 <putchar>:
    800022f6:	04022803          	lw	a6,64(tp) # 40 <buflen.3027>
    800022fa:	00020793          	mv	a5,tp
    800022fe:	97c2                	add	a5,a5,a6
    80002300:	7159                	addi	sp,sp,-112
    80002302:	0018071b          	addiw	a4,a6,1
    80002306:	04e22023          	sw	a4,64(tp) # 40 <buflen.3027>
    8000230a:	00a78023          	sb	a0,0(a5)
    8000230e:	45a9                	li	a1,10
    80002310:	03f10793          	addi	a5,sp,63
    80002314:	fc07f793          	andi	a5,a5,-64
    80002318:	00b50963          	beq	a0,a1,8000232a <putchar+0x34>
    8000231c:	04000593          	li	a1,64
    80002320:	00b70563          	beq	a4,a1,8000232a <putchar+0x34>
    80002324:	4501                	li	a0,0
    80002326:	6165                	addi	sp,sp,112
    80002328:	8082                	ret
    8000232a:	04000593          	li	a1,64
    8000232e:	e38c                	sd	a1,0(a5)
    80002330:	4585                	li	a1,1
    80002332:	e78c                	sd	a1,8(a5)
    80002334:	00020693          	mv	a3,tp
    80002338:	eb94                	sd	a3,16(a5)
    8000233a:	ef98                	sd	a4,24(a5)
    8000233c:	0ff0000f          	fence
    80002340:	fffff697          	auipc	a3,0xfffff
    80002344:	d0068693          	addi	a3,a3,-768 # 80001040 <fromhost>
    80002348:	fffff717          	auipc	a4,0xfffff
    8000234c:	caf73c23          	sd	a5,-840(a4) # 80001000 <tohost>
    80002350:	6298                	ld	a4,0(a3)
    80002352:	df7d                	beqz	a4,80002350 <putchar+0x5a>
    80002354:	fffff717          	auipc	a4,0xfffff
    80002358:	ce073623          	sd	zero,-788(a4) # 80001040 <fromhost>
    8000235c:	0ff0000f          	fence
    80002360:	04022023          	sw	zero,64(tp) # 40 <buflen.3027>
    80002364:	639c                	ld	a5,0(a5)
    80002366:	4501                	li	a0,0
    80002368:	6165                	addi	sp,sp,112
    8000236a:	8082                	ret

000000008000236c <setStats>:
    8000236c:	b00027f3          	csrr	a5,mcycle
    80002370:	00000717          	auipc	a4,0x0
    80002374:	43070713          	addi	a4,a4,1072 # 800027a0 <counters>
    80002378:	e119                	bnez	a0,8000237e <setStats+0x12>
    8000237a:	6314                	ld	a3,0(a4)
    8000237c:	8f95                	sub	a5,a5,a3
    8000237e:	00000697          	auipc	a3,0x0
    80002382:	42f6b123          	sd	a5,1058(a3) # 800027a0 <counters>
    80002386:	b02027f3          	csrr	a5,minstret
    8000238a:	e119                	bnez	a0,80002390 <setStats+0x24>
    8000238c:	6718                	ld	a4,8(a4)
    8000238e:	8f99                	sub	a5,a5,a4
    80002390:	00000717          	auipc	a4,0x0
    80002394:	40f73c23          	sd	a5,1048(a4) # 800027a8 <counters+0x8>
    80002398:	8082                	ret

000000008000239a <tohost_exit>:
    8000239a:	00151793          	slli	a5,a0,0x1
    8000239e:	0017e793          	ori	a5,a5,1
    800023a2:	fffff717          	auipc	a4,0xfffff
    800023a6:	c4f73f23          	sd	a5,-930(a4) # 80001000 <tohost>
    800023aa:	a001                	j	800023aa <tohost_exit+0x10>

00000000800023ac <handle_trap>:
    800023ac:	6785                	lui	a5,0x1
    800023ae:	a7378793          	addi	a5,a5,-1421 # a73 <buflen.3027+0xa33>
    800023b2:	fffff717          	auipc	a4,0xfffff
    800023b6:	c4f73723          	sd	a5,-946(a4) # 80001000 <tohost>
    800023ba:	a001                	j	800023ba <handle_trap+0xe>

00000000800023bc <exit>:
    800023bc:	1141                	addi	sp,sp,-16
    800023be:	e406                	sd	ra,8(sp)
    800023c0:	fdbff0ef          	jal	ra,8000239a <tohost_exit>

00000000800023c4 <abort>:
    800023c4:	10d00793          	li	a5,269
    800023c8:	fffff717          	auipc	a4,0xfffff
    800023cc:	c2f73c23          	sd	a5,-968(a4) # 80001000 <tohost>
    800023d0:	a001                	j	800023d0 <abort+0xc>

00000000800023d2 <printstr>:
    800023d2:	00054783          	lbu	a5,0(a0)
    800023d6:	7159                	addi	sp,sp,-112
    800023d8:	03f10693          	addi	a3,sp,63
    800023dc:	fc06f693          	andi	a3,a3,-64
    800023e0:	c3b9                	beqz	a5,80002426 <printstr+0x54>
    800023e2:	87aa                	mv	a5,a0
    800023e4:	0785                	addi	a5,a5,1
    800023e6:	0007c703          	lbu	a4,0(a5)
    800023ea:	ff6d                	bnez	a4,800023e4 <printstr+0x12>
    800023ec:	8f89                	sub	a5,a5,a0
    800023ee:	04000713          	li	a4,64
    800023f2:	e298                	sd	a4,0(a3)
    800023f4:	4705                	li	a4,1
    800023f6:	e698                	sd	a4,8(a3)
    800023f8:	ea88                	sd	a0,16(a3)
    800023fa:	ee9c                	sd	a5,24(a3)
    800023fc:	0ff0000f          	fence
    80002400:	fffff717          	auipc	a4,0xfffff
    80002404:	c4070713          	addi	a4,a4,-960 # 80001040 <fromhost>
    80002408:	fffff797          	auipc	a5,0xfffff
    8000240c:	bed7bc23          	sd	a3,-1032(a5) # 80001000 <tohost>
    80002410:	631c                	ld	a5,0(a4)
    80002412:	dffd                	beqz	a5,80002410 <printstr+0x3e>
    80002414:	fffff797          	auipc	a5,0xfffff
    80002418:	c207b623          	sd	zero,-980(a5) # 80001040 <fromhost>
    8000241c:	0ff0000f          	fence
    80002420:	629c                	ld	a5,0(a3)
    80002422:	6165                	addi	sp,sp,112
    80002424:	8082                	ret
    80002426:	4781                	li	a5,0
    80002428:	b7d9                	j	800023ee <printstr+0x1c>

000000008000242a <thread_entry>:
    8000242a:	c111                	beqz	a0,8000242e <thread_entry+0x4>
    8000242c:	a001                	j	8000242c <thread_entry+0x2>
    8000242e:	8082                	ret

0000000080002430 <_init>:
    80002430:	00000517          	auipc	a0,0x0
    80002434:	38050513          	addi	a0,a0,896 # 800027b0 <data>
    80002438:	06400593          	li	a1,100
    8000243c:	4621                	li	a2,8
    8000243e:	4701                	li	a4,0
    80002440:	00271793          	slli	a5,a4,0x2
    80002444:	97aa                	add	a5,a5,a0
    80002446:	4394                	lw	a3,0(a5)
    80002448:	2705                	addiw	a4,a4,1
    8000244a:	2685                	addiw	a3,a3,1
    8000244c:	c394                	sw	a3,0(a5)
    8000244e:	4394                	lw	a3,0(a5)
    80002450:	2681                	sext.w	a3,a3
    80002452:	00d5d663          	bge	a1,a3,8000245e <_init+0x2e>
    80002456:	4394                	lw	a3,0(a5)
    80002458:	f9c6869b          	addiw	a3,a3,-100
    8000245c:	c394                	sw	a3,0(a5)
    8000245e:	fec711e3          	bne	a4,a2,80002440 <_init+0x10>
    80002462:	bff1                	j	8000243e <_init+0xe>

0000000080002464 <printhex>:
    80002464:	8082                	ret

0000000080002466 <printf>:
    80002466:	711d                	addi	sp,sp,-96
    80002468:	02810313          	addi	t1,sp,40
    8000246c:	f42e                	sd	a1,40(sp)
    8000246e:	f832                	sd	a2,48(sp)
    80002470:	fc36                	sd	a3,56(sp)
    80002472:	862a                	mv	a2,a0
    80002474:	869a                	mv	a3,t1
    80002476:	00000517          	auipc	a0,0x0
    8000247a:	e8050513          	addi	a0,a0,-384 # 800022f6 <putchar>
    8000247e:	4581                	li	a1,0
    80002480:	ec06                	sd	ra,24(sp)
    80002482:	e0ba                	sd	a4,64(sp)
    80002484:	e4be                	sd	a5,72(sp)
    80002486:	e8c2                	sd	a6,80(sp)
    80002488:	ecc6                	sd	a7,88(sp)
    8000248a:	e41a                	sd	t1,8(sp)
    8000248c:	b75ff0ef          	jal	ra,80002000 <vprintfmt>
    80002490:	60e2                	ld	ra,24(sp)
    80002492:	4501                	li	a0,0
    80002494:	6125                	addi	sp,sp,96
    80002496:	8082                	ret

0000000080002498 <sprintf>:
    80002498:	711d                	addi	sp,sp,-96
    8000249a:	03010313          	addi	t1,sp,48
    8000249e:	f022                	sd	s0,32(sp)
    800024a0:	e42a                	sd	a0,8(sp)
    800024a2:	f832                	sd	a2,48(sp)
    800024a4:	fc36                	sd	a3,56(sp)
    800024a6:	842a                	mv	s0,a0
    800024a8:	862e                	mv	a2,a1
    800024aa:	00000517          	auipc	a0,0x0
    800024ae:	e3e50513          	addi	a0,a0,-450 # 800022e8 <sprintf_putch.3141>
    800024b2:	002c                	addi	a1,sp,8
    800024b4:	869a                	mv	a3,t1
    800024b6:	f406                	sd	ra,40(sp)
    800024b8:	e4be                	sd	a5,72(sp)
    800024ba:	e0ba                	sd	a4,64(sp)
    800024bc:	e8c2                	sd	a6,80(sp)
    800024be:	ecc6                	sd	a7,88(sp)
    800024c0:	ec1a                	sd	t1,24(sp)
    800024c2:	b3fff0ef          	jal	ra,80002000 <vprintfmt>
    800024c6:	67a2                	ld	a5,8(sp)
    800024c8:	00078023          	sb	zero,0(a5)
    800024cc:	6522                	ld	a0,8(sp)
    800024ce:	70a2                	ld	ra,40(sp)
    800024d0:	9d01                	subw	a0,a0,s0
    800024d2:	7402                	ld	s0,32(sp)
    800024d4:	6125                	addi	sp,sp,96
    800024d6:	8082                	ret

00000000800024d8 <memcpy>:
    800024d8:	00c5e7b3          	or	a5,a1,a2
    800024dc:	8fc9                	or	a5,a5,a0
    800024de:	8b9d                	andi	a5,a5,7
    800024e0:	00c506b3          	add	a3,a0,a2
    800024e4:	cf91                	beqz	a5,80002500 <memcpy+0x28>
    800024e6:	962e                	add	a2,a2,a1
    800024e8:	87aa                	mv	a5,a0
    800024ea:	02d57763          	bgeu	a0,a3,80002518 <memcpy+0x40>
    800024ee:	0585                	addi	a1,a1,1
    800024f0:	fff5c703          	lbu	a4,-1(a1)
    800024f4:	0785                	addi	a5,a5,1
    800024f6:	fee78fa3          	sb	a4,-1(a5)
    800024fa:	feb61ae3          	bne	a2,a1,800024ee <memcpy+0x16>
    800024fe:	8082                	ret
    80002500:	fed57fe3          	bgeu	a0,a3,800024fe <memcpy+0x26>
    80002504:	87aa                	mv	a5,a0
    80002506:	05a1                	addi	a1,a1,8
    80002508:	ff85b703          	ld	a4,-8(a1)
    8000250c:	07a1                	addi	a5,a5,8
    8000250e:	fee7bc23          	sd	a4,-8(a5)
    80002512:	fed7eae3          	bltu	a5,a3,80002506 <memcpy+0x2e>
    80002516:	8082                	ret
    80002518:	8082                	ret

000000008000251a <memset>:
    8000251a:	00c567b3          	or	a5,a0,a2
    8000251e:	8b9d                	andi	a5,a5,7
    80002520:	962a                	add	a2,a2,a0
    80002522:	0ff5f593          	zext.b	a1,a1
    80002526:	cb91                	beqz	a5,8000253a <memset+0x20>
    80002528:	87aa                	mv	a5,a0
    8000252a:	02c57a63          	bgeu	a0,a2,8000255e <memset+0x44>
    8000252e:	0785                	addi	a5,a5,1
    80002530:	feb78fa3          	sb	a1,-1(a5)
    80002534:	fef61de3          	bne	a2,a5,8000252e <memset+0x14>
    80002538:	8082                	ret
    8000253a:	00859793          	slli	a5,a1,0x8
    8000253e:	8ddd                	or	a1,a1,a5
    80002540:	01059793          	slli	a5,a1,0x10
    80002544:	8fcd                	or	a5,a5,a1
    80002546:	02079593          	slli	a1,a5,0x20
    8000254a:	8ddd                	or	a1,a1,a5
    8000254c:	fec576e3          	bgeu	a0,a2,80002538 <memset+0x1e>
    80002550:	87aa                	mv	a5,a0
    80002552:	07a1                	addi	a5,a5,8
    80002554:	feb7bc23          	sd	a1,-8(a5)
    80002558:	fec7ede3          	bltu	a5,a2,80002552 <memset+0x38>
    8000255c:	8082                	ret
    8000255e:	8082                	ret

0000000080002560 <strlen>:
    80002560:	00054783          	lbu	a5,0(a0)
    80002564:	cb89                	beqz	a5,80002576 <strlen+0x16>
    80002566:	87aa                	mv	a5,a0
    80002568:	0785                	addi	a5,a5,1
    8000256a:	0007c703          	lbu	a4,0(a5)
    8000256e:	ff6d                	bnez	a4,80002568 <strlen+0x8>
    80002570:	40a78533          	sub	a0,a5,a0
    80002574:	8082                	ret
    80002576:	4501                	li	a0,0
    80002578:	8082                	ret

000000008000257a <strnlen>:
    8000257a:	cd91                	beqz	a1,80002596 <strnlen+0x1c>
    8000257c:	00054783          	lbu	a5,0(a0)
    80002580:	c38d                	beqz	a5,800025a2 <strnlen+0x28>
    80002582:	00b506b3          	add	a3,a0,a1
    80002586:	87aa                	mv	a5,a0
    80002588:	a021                	j	80002590 <strnlen+0x16>
    8000258a:	0007c703          	lbu	a4,0(a5)
    8000258e:	c711                	beqz	a4,8000259a <strnlen+0x20>
    80002590:	0785                	addi	a5,a5,1
    80002592:	fed79ce3          	bne	a5,a3,8000258a <strnlen+0x10>
    80002596:	852e                	mv	a0,a1
    80002598:	8082                	ret
    8000259a:	40a785b3          	sub	a1,a5,a0
    8000259e:	852e                	mv	a0,a1
    800025a0:	8082                	ret
    800025a2:	4581                	li	a1,0
    800025a4:	bfcd                	j	80002596 <strnlen+0x1c>

00000000800025a6 <strcmp>:
    800025a6:	0505                	addi	a0,a0,1
    800025a8:	fff54783          	lbu	a5,-1(a0)
    800025ac:	0585                	addi	a1,a1,1
    800025ae:	fff5c703          	lbu	a4,-1(a1)
    800025b2:	c799                	beqz	a5,800025c0 <strcmp+0x1a>
    800025b4:	fee789e3          	beq	a5,a4,800025a6 <strcmp>
    800025b8:	0007851b          	sext.w	a0,a5
    800025bc:	9d19                	subw	a0,a0,a4
    800025be:	8082                	ret
    800025c0:	4501                	li	a0,0
    800025c2:	bfed                	j	800025bc <strcmp+0x16>

00000000800025c4 <strcpy>:
    800025c4:	87aa                	mv	a5,a0
    800025c6:	0585                	addi	a1,a1,1
    800025c8:	fff5c703          	lbu	a4,-1(a1)
    800025cc:	0785                	addi	a5,a5,1
    800025ce:	fee78fa3          	sb	a4,-1(a5)
    800025d2:	fb75                	bnez	a4,800025c6 <strcpy+0x2>
    800025d4:	8082                	ret

00000000800025d6 <atol>:
    800025d6:	00054783          	lbu	a5,0(a0)
    800025da:	02000713          	li	a4,32
    800025de:	00e79763          	bne	a5,a4,800025ec <atol+0x16>
    800025e2:	0505                	addi	a0,a0,1
    800025e4:	00054783          	lbu	a5,0(a0)
    800025e8:	fee78de3          	beq	a5,a4,800025e2 <atol+0xc>
    800025ec:	fd57871b          	addiw	a4,a5,-43
    800025f0:	0fd77713          	andi	a4,a4,253
    800025f4:	c715                	beqz	a4,80002620 <atol+0x4a>
    800025f6:	00054683          	lbu	a3,0(a0)
    800025fa:	87aa                	mv	a5,a0
    800025fc:	4601                	li	a2,0
    800025fe:	ce85                	beqz	a3,80002636 <atol+0x60>
    80002600:	4501                	li	a0,0
    80002602:	0785                	addi	a5,a5,1
    80002604:	fd06859b          	addiw	a1,a3,-48
    80002608:	00251713          	slli	a4,a0,0x2
    8000260c:	0007c683          	lbu	a3,0(a5)
    80002610:	953a                	add	a0,a0,a4
    80002612:	0506                	slli	a0,a0,0x1
    80002614:	952e                	add	a0,a0,a1
    80002616:	f6f5                	bnez	a3,80002602 <atol+0x2c>
    80002618:	c219                	beqz	a2,8000261e <atol+0x48>
    8000261a:	40a00533          	neg	a0,a0
    8000261e:	8082                	ret
    80002620:	00154683          	lbu	a3,1(a0)
    80002624:	fd378793          	addi	a5,a5,-45
    80002628:	0017b613          	seqz	a2,a5
    8000262c:	00150793          	addi	a5,a0,1
    80002630:	fae1                	bnez	a3,80002600 <atol+0x2a>
    80002632:	4501                	li	a0,0
    80002634:	b7d5                	j	80002618 <atol+0x42>
    80002636:	4501                	li	a0,0
    80002638:	8082                	ret

Disassembly of section .text.startup:

000000008000263a <main>:
    8000263a:	557d                	li	a0,-1
    8000263c:	8082                	ret
