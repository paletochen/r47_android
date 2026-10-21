// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The C47 Authors


#if !defined(ITEMS_H)
#define ITEMS_H

//This list is generated (manually) from items3.xlsx, EXPORT.H

#define ITM_NULL                       0

// Items from 1 to 127 are 1 byte OP codes
#define ITM_LBL                        1
#define ITM_GTO                        2
#define ITM_XEQ                        3
#define ITM_RTN                        4
#define ITM_ISE                        5
#define ITM_ISG                        6
#define ITM_ISZ                        7
#define ITM_DSE                        8
#define ITM_DSL                        9
#define ITM_DSZ                       10
#define ITM_XEQU                      11
#define ITM_XNE                       12
#define ITM_XEQUP0                    13
#define ITM_XEQUM0                    14
#define ITM_XAEQU                     15
#define ITM_XLT                       16
#define ITM_XLE                       17
#define ITM_XGE                       18
#define ITM_XGT                       19
#define ITM_FC                        20
#define ITM_FS                        21
#define ITM_EVEN                      22
#define ITM_ODD                       23
#define ITM_FPQ                       24
#define ITM_INTQ                      25
#define ITM_CPXQ                      26
#define ITM_MATRIXQ                   27
#define ITM_NANQ                      28
#define ITM_REALQ                     29
#define ITM_SPECQ                     30
#define ITM_STRINGQ                   31
#define ITM_INFQ                      32
#define ITM_PRIME                     33
#define ITM_TOP                       34
#define ITM_ENTER                     35
#define ITM_XexY                      36
#define ITM_DROP                      37
#define ITM_PAUSE                     38
#define ITM_Rup                       39
#define ITM_Rdown                     40
#define ITM_CLX                       41
#define ITM_FILL                      42
#define ITM_INPUT                     43
#define ITM_STO                       44
#define ITM_STOADD                    45
#define ITM_STOSUB                    46
#define ITM_STOMULT                   47
#define ITM_STODIV                    48
#define ITM_COMB                      49
#define ITM_PERM                      50
#define ITM_RCL                       51
#define ITM_RCLADD                    52
#define ITM_RCLSUB                    53
#define ITM_RCLMULT                   54
#define ITM_RCLDIV                    55
#define ITM_CONVG                     56
#define ITM_ENTRY                     57
#define ITM_SQUARE                    58
#define ITM_CUBE                      59
#define ITM_YX                        60
#define ITM_SQUAREROOTX               61
#define ITM_CUBEROOT                  62
#define ITM_XTHROOT                   63
#define ITM_2X                        64
#define ITM_EXP                       65
#define ITM_SINT                      66
#define ITM_10x                       67
#define ITM_LOG2                      68
#define ITM_LN                        69
#define ITM_STOP                      70
#define ITM_LOG10                     71
#define ITM_LOGXY                     72
#define ITM_1ONX                      73
#define ITM_cos                       74
#define ITM_cosh                      75
#define ITM_sin                       76
#define ITM_KEYQ                      77
#define ITM_sinh                      78
#define ITM_tan                       79
#define ITM_tanh                      80
#define ITM_arccos                    81
#define ITM_arcosh                    82
#define ITM_arcsin                    83
#define ITM_arsinh                    84
#define ITM_arctan                    85
#define ITM_artanh                    86
#define ITM_CEIL                      87
#define ITM_FLOOR                     88
#define ITM_GCD                       89
#define ITM_LCM                       90
#define ITM_DECR                      91
#define ITM_INC                       92
#define ITM_IP                        93
#define ITM_FP                        94
#define ITM_ADD                       95
#define ITM_SUB                       96
#define ITM_CHS                       97
#define ITM_MULT                      98
#define ITM_DIV                       99
#define ITM_IDIV                     100
#define ITM_VIEW                     101
#define ITM_MOD                      102
#define ITM_MAX                      103
#define ITM_MIN                      104
#define ITM_MAGNITUDE                105
#define ITM_NEIGHB                   106
#define ITM_NEXTP                    107
#define ITM_XFACT                    108
#define ITM_CONSTpi                  109
#define ITM_CF                       110
#define ITM_SF                       111
#define ITM_FF                       112
#define ITM_M_SQRQ                   113
#define ITM_LITERAL                  114
#define ITM_DEG2                     115
#define ITM_DMS2                     116
#define ITM_GRAD2                    117
#define ITM_MULPI2                   118
#define ITM_RAD2                     119
#define ITM_LINT                     120
#define ITM_RMODE                    121 // sunsetted, can be replaced with a type PTP_NUMBER_8 only, once the >RMODE< has been in effect long enough: 2026-09-20
#define ITM_RMD                      122
#define ITM_LOGICALNOT               123
#define ITM_LOGICALAND               124
#define ITM_LOGICALOR                125
#define ITM_LOGICALXOR               126
#define ITM_Xex                      127


// Items from 128 to ... are 2 byte OP codes
// Constants
#define CST_01                       128
#define CST_02                       129
#define CST_03                       130
#define CST_04                       131
#define CST_05                       132
#define CST_06                       133
#define CST_07                       134
#define CST_08                       135
#define CST_09                       136
#define CST_10                       137
#define CST_11                       138
#define CST_12                       139
#define CST_13                       140
#define CST_14                       141
#define CST_15                       142
#define CST_16                       143
#define CST_17                       144
#define CST_18                       145
#define CST_19                       146
#define CST_20                       147
#define CST_21                       148
#define CST_22                       149
#define CST_23                       150
#define CST_24                       151
#define CST_25                       152
#define CST_26                       153
#define CST_27                       154
#define CST_28                       155
#define CST_29                       156
#define CST_30                       157
#define CST_31                       158
#define CST_32                       159
#define CST_33                       160
#define CST_34                       161
#define CST_35                       162
#define CST_36                       163
#define CST_37                       164
#define CST_38                       165
#define CST_39                       166
#define CST_40                       167
#define CST_41                       168
#define CST_42                       169
#define CST_43                       170
#define CST_44                       171
#define CST_45                       172
#define CST_46                       173
#define CST_47                       174
#define CST_48                       175
#define CST_49                       176
#define CST_50                       177
#define CST_51                       178
#define CST_52                       179
#define CST_53                       180
#define CST_54                       181
#define CST_55                       182
#define CST_56                       183
#define CST_57                       184
#define CST_58                       185
#define CST_59                       186
#define CST_60                       187
#define CST_61                       188
#define CST_62                       189
#define CST_63                       190
#define CST_64                       191
#define CST_65                       192
#define CST_66                       193
#define CST_67                       194
#define CST_68                       195
#define CST_69                       196
#define CST_70                       197
#define CST_71                       198
#define CST_72                       199
#define CST_73                       200
#define CST_74                       201
#define CST_75                       202
#define CST_76                       203
#define CST_77                       204
#define CST_78                       205
#define CST_79                       206
#define ITM_CNST                     207
#define CST_80                       208
#define CST_81                       209
#define CST_82                       210
#define CST_83                       211
#define CST_84                       212
#define ITM_0213                     213
#define ITM_0214                     214
#define ITM_0215                     215
#define ITM_0216                     216
#define ITM_0217                     217
#define ITM_0218                     218
#define ITM_0219                     219


// Conversions
#define ITM_CtoF                     220
#define ITM_FtoC                     221
#define ITM_DBtoPR                   222
#define ITM_FT2toHA                  223
#define ITM_HAtoFT2                  224
#define ITM_DBtoFR                   225
#define ITM_FT2toM2                  226
#define ITM_M2toFT2                  227
#define ITM_PRtoDB                   228
#define ITM_HAtoKM2                  229
#define ITM_KM2toHA                  230
#define ITM_FRtoDB                   231
#define ITM_GLUKtoFZUK               232
#define ITM_FZUKtoGLUK               233
#define ITM_ACtoHA                   234
#define ITM_MLtoIN3                  235
#define ITM_HAtoAC                   236
#define ITM_IN3toML                  237
#define ITM_ACUStoHA                 238
#define ITM_FT3toGLUK                239
#define ITM_HAtoACUS                 240
#define ITM_GLUKtoFT3                241
#define ITM_PAtoATM                  242
#define ITM_ATMtoPA                  243
#define ITM_AUtoM                    244
#define ITM_MtoAU                    245
#define ITM_BARtoPA                  246
#define ITM_PAtoBAR                  247
#define ITM_BTUtoJ                   248
#define ITM_JtoBTU                   249
#define ITM_CALtoJ                   250
#define ITM_JtoCAL                   251
#define ITM_LBFFTtoNM                252
#define ITM_LtoFT3                   253
#define ITM_NMtoLBFFT                254
#define ITM_FT3toL                   255
#define ITM_CWTtoKG                  256
#define ITM_KGtoCWT                  257
#define ITM_FTtoM                    258
#define ITM_MtoFT                    259
#define ITM_FTUStoM                  260
#define ITM_LtoQTUS                  261
#define ITM_QTUStoL                  262
#define ITM_MtoFTUS                  263
#define ITM_IN3toFZUK                264
#define ITM_FZUKtoIN3                265
#define ITM_FZUKtoML                 266
#define ITM_IN3toFZUS                267
#define ITM_MLtoFZUK                 268
#define ITM_FZUStoIN3                269
#define ITM_FZUStoML                 270
#define ITM_FT3toGalUS               271
#define ITM_MLtoFZUS                 272
#define ITM_GalUStoFT3               273
#define ITM_GLUKtoL                  274
#define ITM_LtoGLUK                  275
#define ITM_GLUStoL                  276
#define ITM_LtoGLUS                  277
#define ITM_HPEtoW                   278
#define ITM_WtoHPE                   279
#define ITM_HPMtoW                   280
#define ITM_WtoHPM                   281
#define ITM_HPUKtoW                  282
#define ITM_WtoHPUK                  283
#define ITM_INCHHGtoPA               284
#define ITM_CUPCtoFZUS               285
#define ITM_PAtoINCHHG               286
#define ITM_CUPCtoML                 287
#define ITM_INCHtoMM                 288
#define ITM_MMtoINCH                 289
#define ITM_WHtoJ                    290
#define ITM_JtoWH                    291
#define ITM_KGtoLBS                  292
#define ITM_LBStoKG                  293
#define ITM_GtoOZ                    294
#define ITM_OZtoG                    295
#define ITM_KGtoSCW                  296
#define ITM_CUPUKtoFZUK              297
#define ITM_SCWtoKG                  298
#define ITM_CUPUKtoML                299
#define ITM_KGtoSTO                  300
#define ITM_FZUKtoCUPUK              301
#define ITM_STOtoKG                  302
#define ITM_FZUKtoTBSPUK             303
#define ITM_KGtoST                   304
#define ITM_FZUKtoTSPUK              305
#define ITM_FZUStoCUPC               306
#define ITM_STtoKG                   307
#define ITM_FZUStoTBSPC              308
#define ITM_FZUStoTSPC               309
#define ITM_KGtoLT                   310
#define ITM_KGtoLIANG                311
#define ITM_MLtoCUPC                 312
#define ITM_LTtoKG                   313
#define ITM_LIANGtoKG                314
#define ITM_MLtoCUPUK                315
#define ITM_GtoTRZ                   316
#define ITM_MLtoPINTLQ               317
#define ITM_TRZtoG                   318
#define ITM_MLtoPINTUK               319
#define ITM_LBFtoN                   320
#define ITM_NtoLBF                   321
#define ITM_LYtoM                    322
#define ITM_MtoLY                    323
#define ITM_MMHGtoPA                 324
#define ITM_MLtoQT                   325
#define ITM_PAtoMMHG                 326
#define ITM_MLtoQTUS                 327
#define ITM_MItoKM                   328
#define ITM_KMtoMI                   329
#define ITM_KMtoNMI                  330
#define ITM_NMItoKM                  331
#define ITM_MtoPC                    332
#define ITM_PCtoM                    333
#define ITM_MMtoPOINT                334
#define ITM_MLtoTBSPC                335
#define ITM_MILEtoM                  336
#define ITM_POINTtoMM                337
#define ITM_MLtoTBSPUK               338
#define ITM_MtoMILE                  339
#define ITM_MtoYD                    340
#define ITM_YDtoM                    341
#define ITM_PSItoPA                  342
#define ITM_PAtoPSI                  343
#define ITM_PAtoTOR                  344
#define ITM_MLtoTSPC                 345
#define ITM_TORtoPA                  346
#define ITM_MLtoTSPUK                347
#define ITM_StoYEAR                  348
#define ITM_YEARtoS                  349
#define ITM_CARATtoG                 350
#define ITM_PINTLQtoML               351
#define ITM_JINtoKG                  352
#define ITM_GtoCARAT                 353
#define ITM_PINTUKtoML               354
#define ITM_KGtoJIN                  355
#define ITM_QTtoL                    356
#define ITM_LtoQT                    357
#define ITM_FATHOMtoM                358
#define ITM_QTtoML                   359
#define ITM_NMItoM                   360
#define ITM_MtoFATHOM                361
#define ITM_QTUStoML                 362
#define ITM_MtoNMI                   363
#define ITM_BARRELtoM3               364
#define ITM_TBSPCtoFZUS              365
#define ITM_M3toBARREL               366
#define ITM_HMStoHR                  367
#define ITM_HRtoHMS                  368
#define ITM_TBSPCtoML                369
#define ITM_HECTAREtoM2              370
#define ITM_M2toHECTARE              371
#define ITM_MUtoM2                   372
#define ITM_M2toMU                   373
#define ITM_LItoM                    374
#define ITM_MtoLI                    375
#define ITM_CHItoM                   376
#define ITM_MtoCHI                   377
#define ITM_YINtoM                   378
#define ITM_MtoYIN                   379
#define ITM_CUNtoM                   380
#define ITM_MtoCUN                   381
#define ITM_ZHANGtoM                 382
#define ITM_TBSPUKtoFZUK             383
#define ITM_MtoZHANG                 384
#define ITM_TBSPUKtoML               385
#define ITM_FENtoM                   386
#define ITM_MtoFEN                   387
#define ITM_MI2toKM2                 388
#define ITM_KM2toMI2                 389
#define ITM_NMI2toKM2                390
#define ITM_KM2toNMI2                391
#define ITM_TSPCtoFZUS               392
#define ITM_TSPCtoML                 393
#define ITM_TSPUKtoFZUK              394
#define ITM_TSPUKtoML                395


// Flag, bit, rotation, and logical OPs
#define ITM_FCC                      396
#define ITM_FCS                      397
#define ITM_FCF                      398
#define ITM_FSC                      399
#define ITM_FSS                      400
#define ITM_FSF                      401
#define ITM_LOGICALNAND              402
#define ITM_LOGICALNOR               403
#define ITM_LOGICALXNOR              404
#define ITM_BS                       405
#define ITM_BC                       406
#define ITM_CB                       407
#define ITM_SB                       408
#define ITM_FB                       409
#define ITM_RL                       410
#define ITM_RLC                      411
#define ITM_RR                       412
#define ITM_RRC                      413
#define ITM_SL                       414
#define ITM_SR                       415
#define ITM_ASR                      416
#define ITM_LJ                       417
#define ITM_RJ                       418
#define ITM_MASKL                    419
#define ITM_MASKR                    420
#define ITM_MIRROR                   421
#define ITM_NUMB                     422
#define ITM_SDL                      423
#define ITM_SDR                      424
#define ITM_ZIP                      425
#define ITM_UNZIP                    426
#define ITM_0427                     427
#define ITM_0428                     428
#define ITM_0429                     429
#define ITM_0430                     430
#define ITM_0431                     431
#define ITM_0432                     432



#define ITM_SIGMAPLUS                433
#define ITM_SIGMAMINUS               434
#define ITM_NSIGMA                   435
#define ITM_SIGMAx                   436
#define ITM_SIGMAy                   437
#define ITM_SIGMAx2                  438
#define ITM_SIGMAx2y                 439
#define ITM_SIGMAy2                  440
#define ITM_SIGMAxy                  441
#define ITM_SIGMAlnxy                442
#define ITM_SIGMAlnx                 443
#define ITM_SIGMAln2x                444
#define ITM_SIGMAylnx                445
#define ITM_SIGMAlny                 446
#define ITM_SIGMAln2y                447
#define ITM_SIGMAxlny                448
#define ITM_SIGMAx2lny               449
#define ITM_SIGMAlnyonx              450
#define ITM_SIGMAx2ony               451
#define ITM_SIGMA1onx                452
#define ITM_SIGMA1onx2               453
#define ITM_SIGMAxony                454
#define ITM_SIGMA1ony                455
#define ITM_SIGMA1ony2               456
#define ITM_SIGMAx3                  457
#define ITM_SIGMAx4                  458
#define ITM_0459                     459
#define ITM_0460                     460
#define ITM_0461                     461
#define ITM_0462                     462


// System flags
#define SFL_TDM24                    463 // This label is used to identify the first in the list of flags
#define SFL_YMD                      464
#define SFL_DMY                      465
#define SFL_MDY                      466
#define SFL_CPXRES                   467
#define SFL_CPXj                     468
#define SFL_POLAR                    469
#define SFL_FRACT                    470
#define SFL_PROPFR                   471
#define SFL_DENANY                   472
#define SFL_DENFIX                   473
#define SFL_CARRY                    474
#define SFL_OVERFLOW                 475
#define SFL_LEAD0                    476
#define SFL_ALPHA                    477
#define SFL_alphaCAP                 478
#define SFL_RUNTIM                   479
#define SFL_AMORT_HP12C              480
#define SFL_IMPLOT                   481
#define SFL_TRACE                    482
#define SFL_USER                     483
#define SFL_LOWBAT                   484
#define SFL_SLOW                     485
#define SFL_SPCRES                   486
#define SFL_SSIZE8                   487
#define SFL_QUIET                    488
#define SFL_WRAPEND                  489
#define SFL_MULTx                    490
#define SFL_ENGOVR                   491
#define SFL_GROW                     492
#define SFL_AUTOFF                   493
#define SFL_AUTXEQ                   494
#define SFL_PRTACT                   495
#define SFL_NUMIN                    496
#define SFL_ALPIN                    497
#define SFL_ASLIFT                   498
#define SFL_IGN1ER                   499
#define SFL_INTING                   500
#define SFL_SOLVING                  501
#define SFL_VMDISP                   502
#define SFL_USB                      503
#define SFL_ENDPMT                   504
#define SFL_FRCSRN                   505
#define SFL_HPRP                     506
#define SFL_SBdate                   507
#define SFL_SBtime                   508
#define SFL_SBcr                     509
#define SFL_SBcpx                    510
#define SFL_SBang                    511
#define SFL_SBfrac                   512
#define SFL_SBint                    513
#define SFL_SBmx                     514
#define SFL_SBtvm                    515
#define SFL_SBoc                     516
#define SFL_SBss                     517
#define SFL_SBstpw                   518
#define SFL_SBser                    519
#define SFL_SBprn                    520
#define SFL_SBbatV                   521
#define SFL_SBshfR                   522
#define SFL_HPBASE                   523
#define SFL_2TO10                    524
#define SFL_SH_LONGPRESS             525
#define SFL_WRAPEDG                  526

// Bufferized items
#define ITM_REG_X                    527
#define ITM_REG_Y                    528
#define ITM_REG_Z                    529
#define ITM_REG_T                    530
#define ITM_REG_A                    531
#define ITM_REG_B                    532
#define ITM_REG_C                    533
#define ITM_REG_D                    534
#define ITM_REG_L                    535
#define ITM_REG_I                    536
#define ITM_REG_J                    537
#define ITM_REG_K                    538
#define ITM_INDIRECTION              539
#define ITM_0                        540
#define ITM_1                        541
#define ITM_2                        542
#define ITM_3                        543
#define ITM_4                        544
#define ITM_5                        545
#define ITM_6                        546
#define ITM_7                        547
#define ITM_8                        548
#define ITM_9                        549
#define ITM_A                        550
#define ITM_B                        551
#define ITM_C                        552
#define ITM_D                        553
#define ITM_E                        554
#define ITM_F                        555
#define ITM_G                        556
#define ITM_H                        557
#define ITM_I                        558
#define ITM_J                        559
#define ITM_K                        560
#define ITM_L                        561
#define ITM_M                        562
#define ITM_N                        563
#define ITM_O                        564
#define ITM_P                        565
#define ITM_Q                        566
#define ITM_R                        567
#define ITM_S                        568
#define ITM_T                        569
#define ITM_U                        570
#define ITM_V                        571
#define ITM_W                        572
#define ITM_X                        573
#define ITM_Y                        574
#define ITM_Z                        575
#define ITM_a                        576
#define ITM_b                        577
#define ITM_c                        578
#define ITM_d                        579
#define ITM_e                        580
#define ITM_f                        581
#define ITM_g                        582
#define ITM_h                        583
#define ITM_i                        584
#define ITM_j                        585
#define ITM_k                        586
#define ITM_l                        587
#define ITM_m                        588
#define ITM_n                        589
#define ITM_o                        590
#define ITM_p                        591
#define ITM_q                        592
#define ITM_r                        593
#define ITM_s                        594
#define ITM_t                        595
#define ITM_u                        596
#define ITM_v                        597
#define ITM_w                        598
#define ITM_x                        599
#define ITM_y                        600
#define ITM_z                        601
#define ITM_ALPHA                    602
#define ITM_BETA                     603
#define ITM_GAMMA                    604
#define ITM_DELTA                    605
#define ITM_EPSILON                  606
#define ITM_ZETA                     607
#define ITM_ETA                      608
#define ITM_THETA                    609
#define ITM_IOTA                     610
#define ITM_IOTA_DIALYTIKA           611
#define ITM_KAPPA                    612
#define ITM_LAMBDA                   613
#define ITM_MU                       614
#define ITM_NU                       615
#define ITM_XI                       616
#define ITM_OMICRON                  617
#define ITM_PI                       618
#define ITM_RHO                      619
#define ITM_SIGMA                    620
#define ITM_TAU                      621
#define ITM_UPSILON                  622
#define ITM_UPSILON_DIALYTIKA        623
#define ITM_PHI                      624
#define ITM_CHI                      625
#define ITM_PSI                      626
#define ITM_OMEGA                    627
#define ITM_alpha                    628
#define ITM_beta                     629
#define ITM_gamma                    630
#define ITM_delta                    631
#define ITM_epsilon                  632
#define ITM_zeta                     633
#define ITM_eta                      634
#define ITM_theta                    635
#define ITM_iota                     636
#define ITM_iota_DIALYTIKA           637
#define ITM_kappa                    638
#define ITM_lambda                   639
#define ITM_mu                       640
#define ITM_nu                       641
#define ITM_xi                       642
#define ITM_omicron                  643
#define ITM_pi                       644
#define ITM_rho                      645
#define ITM_sigma                    646
#define ITM_tau                      647
#define ITM_upsilon                  648
#define ITM_upsilon_DIALYTIKA        649
#define ITM_phi                      650
#define ITM_chi                      651
#define ITM_psi                      652
#define ITM_omega                    653
#define ITM_alpha_TONOS              654
#define ITM_epsilon_TONOS            655
#define ITM_eta_TONOS                656
#define ITM_iotaTON                  657
#define ITM_iota_DIALYTIKA_TONOS     658
#define ITM_omicron_TONOS            659
#define ITM_sigma_end                660
#define ITM_upsilon_TONOS            661
#define ITM_upsilon_DIALYTIKA_TONOS  662
#define ITM_omega_TONOS              663
#define ITM_A_MACRON                 664
#define ITM_A_ACUTE                  665
#define ITM_A_BREVE                  666
#define ITM_A_GRAVE                  667
#define ITM_A_DIARESIS               668
#define ITM_A_TILDE                  669
#define ITM_A_CIRC                   670
#define ITM_A_RING                   671
#define ITM_AE                       672
#define ITM_A_OGONEK                 673
#define ITM_C_ACUTE                  674
#define ITM_C_CARON                  675
#define ITM_C_CEDILLA                676
#define ITM_D_STROKE                 677
#define ITM_D_CARON                  678
#define ITM_E_MACRON                 679
#define ITM_E_ACUTE                  680
#define ITM_E_BREVE                  681
#define ITM_E_GRAVE                  682
#define ITM_E_DIARESIS               683
#define ITM_E_CIRC                   684
#define ITM_E_OGONEK                 685
#define ITM_G_BREVE                  686
#define ITM_I_MACRON                 687
#define ITM_I_ACUTE                  688
#define ITM_I_BREVE                  689
#define ITM_I_GRAVE                  690
#define ITM_I_DIARESIS               691
#define ITM_I_CIRC                   692
#define ITM_I_OGONEK                 693
#define ITM_I_DOT                    694
#define ITM_I_DOTLESS                695
#define ITM_L_STROKE                 696
#define ITM_L_ACUTE                  697
#define ITM_L_APOSTROPHE             698
#define ITM_N_ACUTE                  699
#define ITM_N_CARON                  700
#define ITM_N_TILDE                  701
#define ITM_O_MACRON                 702
#define ITM_O_ACUTE                  703
#define ITM_O_BREVE                  704
#define ITM_O_GRAVE                  705
#define ITM_O_DIARESIS               706
#define ITM_O_TILDE                  707
#define ITM_O_CIRC                   708
#define ITM_O_STROKE                 709
#define ITM_OE                       710
#define ITM_S_SHARP                  711
#define ITM_S_ACUTE                  712
#define ITM_S_CARON                  713
#define ITM_S_CEDILLA                714
#define ITM_T_CARON                  715
#define ITM_T_CEDILLA                716
#define ITM_U_MACRON                 717
#define ITM_U_ACUTE                  718
#define ITM_U_BREVE                  719
#define ITM_U_GRAVE                  720
#define ITM_U_DIARESIS               721
#define ITM_U_TILDE                  722
#define ITM_U_CIRC                   723
#define ITM_U_RING                   724
#define ITM_W_CIRC                   725
#define ITM_Y_CIRC                   726
#define ITM_Y_ACUTE                  727
#define ITM_Y_DIARESIS               728
#define ITM_Z_ACUTE                  729
#define ITM_Z_CARON                  730
#define ITM_Z_DOT                    731
#define ITM_a_MACRON                 732
#define ITM_a_ACUTE                  733
#define ITM_a_BREVE                  734
#define ITM_a_GRAVE                  735
#define ITM_a_DIARESIS               736
#define ITM_a_TILDE                  737
#define ITM_a_CIRC                   738
#define ITM_a_RING                   739
#define ITM_ae                       740
#define ITM_a_OGONEK                 741
#define ITM_c_ACUTE                  742
#define ITM_c_CARON                  743
#define ITM_c_CEDILLA                744
#define ITM_d_STROKE                 745
#define ITM_d_APOSTROPHE             746
#define ITM_e_MACRON                 747
#define ITM_e_ACUTE                  748
#define ITM_e_BREVE                  749
#define ITM_e_GRAVE                  750
#define ITM_e_DIARESIS               751
#define ITM_e_CIRC                   752
#define ITM_e_OGONEK                 753
#define ITM_g_BREVE                  754
#define ITM_h_STROKE                 755
#define ITM_i_MACRON                 756
#define ITM_i_ACUTE                  757
#define ITM_i_BREVE                  758
#define ITM_i_GRAVE                  759
#define ITM_i_DIARESIS               760
#define ITM_i_CIRC                   761
#define ITM_i_OGONEK                 762
#define ITM_i_DOT                    763
#define ITM_i_DOTLESS                764
#define ITM_l_STROKE                 765
#define ITM_l_ACUTE                  766
#define ITM_l_APOSTROPHE             767
#define ITM_n_ACUTE                  768
#define ITM_n_CARON                  769
#define ITM_n_TILDE                  770
#define ITM_o_MACRON                 771
#define ITM_o_ACUTE                  772
#define ITM_o_BREVE                  773
#define ITM_o_GRAVE                  774
#define ITM_o_DIARESIS               775
#define ITM_o_TILDE                  776
#define ITM_o_CIRC                   777
#define ITM_o_STROKE                 778
#define ITM_oe                       779
#define ITM_r_CARON                  780
#define ITM_r_ACUTE                  781
#define ITM_s_SHARP                  782
#define ITM_s_ACUTE                  783
#define ITM_s_CARON                  784
#define ITM_s_CEDILLA                785
#define ITM_t_APOSTROPHE             786
#define ITM_t_CEDILLA                787
#define ITM_u_MACRON                 788
#define ITM_u_ACUTE                  789
#define ITM_u_BREVE                  790
#define ITM_u_GRAVE                  791
#define ITM_u_DIARESIS               792
#define ITM_u_TILDE                  793
#define ITM_u_CIRC                   794
#define ITM_u_RING                   795
#define ITM_w_CIRC                   796
#define ITM_x_BAR                    797
#define ITM_x_CIRC                   798
#define ITM_y_BAR                    799
#define ITM_y_CIRC                   800
#define ITM_y_ACUTE                  801
#define ITM_y_DIARESIS               802
#define ITM_z_ACUTE                  803
#define ITM_z_CARON                  804
#define ITM_z_DOT                    805
#define ITM_SPACE                    806
#define ITM_EXCLAMATION_MARK         807
#define ITM_DOUBLE_QUOTE             808
#define ITM_NUMBER_SIGN              809
#define ITM_DOLLAR                   810
#define ITM_PERCENT                  811
#define ITM_AMPERSAND                812
#define ITM_QUOTE                    813
#define ITM_LEFT_PARENTHESIS         814
#define ITM_RIGHT_PARENTHESIS        815
#define ITM_ASTERISK                 816
#define ITM_PLUS                     817
#define ITM_COMMA                    818
#define ITM_MINUS                    819
#define ITM_PERIOD                   820
#define ITM_SLASH                    821
#define ITM_COLON                    822
#define ITM_SEMICOLON                823
#define ITM_LESS_THAN                824
#define ITM_EQUAL                    825
#define ITM_GREATER_THAN             826
#define ITM_QUESTION_MARK            827
#define ITM_AT                       828
#define ITM_LEFT_SQUARE_BRACKET      829
#define ITM_BACK_SLASH               830
#define ITM_RIGHT_SQUARE_BRACKET     831
#define ITM_CIRCUMFLEX               832
#define ITM_UNDERSCORE               833
#define ITM_LEFT_CURLY_BRACKET       834
#define ITM_PIPE                     835
#define ITM_RIGHT_CURLY_BRACKET      836
#define ITM_TILDE                    837
#define ITM_INVERTED_EXCLAMATION_MARK 838
#define ITM_CENT                     839
#define ITM_POUND                    840
#define ITM_YEN                      841
#define ITM_SECTION                  842
#define ITM_OVERFLOW_CARRY           843
#define ITM_LEFT_DOUBLE_ANGLE        844
#define ITM_NOT                      845
#define ITM_DEGREE                   846
#define ITM_PLUS_MINUS               847
#define ITM_MICRO                    848
#define ITM_DOT                      849
#define ITM_RIGHT_DOUBLE_ANGLE       850
#define ITM_ONE_HALF                 851
#define ITM_ONE_QUARTER              852
#define ITM_INVERTED_QUESTION_MARK   853
#define ITM_ETH                      854
#define ITM_CROSS                    855
#define ITM_eth                      856
#define ITM_OBELUS                   857
#define ITM_E_DOT                    858
#define ITM_e_DOT                    859
#define ITM_E_CARON                  860
#define ITM_e_CARON                  861
#define ITM_R_ACUTE                  862
#define ITM_R_CARON                  863
#define ITM_U_OGONEK                 864
#define ITM_u_OGONEK                 865
#define ITM_y_UNDER_ROOT             866
#define ITM_x_UNDER_ROOT             867
#define ITM_SPACE_EM                 868
#define ITM_SPACE_3_PER_EM           869
#define ITM_SPACE_4_PER_EM           870
#define ITM_SPACE_6_PER_EM           871
#define ITM_SPACE_FIGURE             872
#define ITM_SPACE_PUNCTUATION        873
#define ITM_SPACE_HAIR               874
#define ITM_LEFT_SINGLE_QUOTE        875
#define ITM_RIGHT_SINGLE_QUOTE       876
#define ITM_SINGLE_LOW_QUOTE         877
#define ITM_SINGLE_HIGH_QUOTE        878
#define ITM_LEFT_DOUBLE_QUOTE        879
#define ITM_RIGHT_DOUBLE_QUOTE       880
#define ITM_DOUBLE_LOW_QUOTE         881
#define ITM_DOUBLE_HIGH_QUOTE        882
#define ITM_ELLIPSIS                 883
#define ITM_BINARY_ONE               884
#define ITM_EURO                     885
#define ITM_COMPLEX_C                886
#define ITM_PLANCK                   887
#define ITM_PLANCK_2PI               888
#define ITM_NATURAL_N                889
#define ITM_RATIONAL_Q               890
#define ITM_REAL_R                   891
#define ITM_LEFT_ARROW               892
#define ITM_UP_ARROW                 893
#define ITM_RIGHT_ARROW              894
#define ITM_DOWN_ARROW               895
#define ITM_SERIAL_IO                896
#define ITM_RIGHT_SHORT_ARROW        897
#define ITM_LEFT_RIGHT_ARROWS        898
#define ITM_BST_SIGN                 899
#define ITM_SST_SIGN                 900
#define ITM_HAMBURGER                901
#define ITM_UNDO_SIGN                902
#define ITM_FOR_ALL                  903
#define ITM_COMPLEMENT               904
#define ITM_PARTIAL_DIFF             905
#define ITM_THERE_EXISTS             906
#define ITM_THERE_DOES_NOT_EXIST     907
#define ITM_EMPTY_SET                908
#define ITM_INCREMENT                909
#define ITM_NABLA                    910
#define ITM_ELEMENT_OF               911
#define ITM_NOT_ELEMENT_OF           912
#define ITM_CONTAINS                 913
#define ITM_DOES_NOT_CONTAIN         914
#define ITM_BINARY_ZERO              915
#define ITM_PRODUCT                  916
#define ITM_MINUS_PLUS               917
#define ITM_RING                     918
#define ITM_BULLET                   919
#define ITM_SQUARE_ROOT              920
#define ITM_CUBEROOT_SIGN            921
#define ITM_xTH_ROOT                 922
#define ITM_PROPORTIONAL             923
#define ITM_INFINITY                 924
#define ITM_RIGHT_ANGLE              925
#define ITM_IRRATIONAL_I             926
#define ITM_MEASURED_ANGLE           927
#define ITM_DIVIDES                  928
#define ITM_DOES_NOT_DIVIDE          929
#define ITM_PARALLEL_SIGN            930
#define ITM_NOT_PARALLEL             931
#define ITM_AND                      932
#define ITM_OR                       933
#define ITM_INTERSECTION             934
#define ITM_UNION                    935
#define ITM_INTEGRAL_SIGN            936
#define ITM_DOUBLE_INTEGRAL          937
#define ITM_CONTOUR_INTEGRAL         938
#define ITM_SURFACE_INTEGRAL         939
#define ITM_RATIO                    940
#define ITM_CHECK_MARK               941
#define ITM_ASYMPOTICALLY_EQUAL      942
#define ITM_ALMOST_EQUAL             943
#define ITM_COLON_EQUALS             944
#define ITM_CORRESPONDS_TO           945
#define ITM_ESTIMATES                946
#define ITM_NOT_EQUAL                947
#define ITM_IDENTICAL_TO             948
#define ITM_LESS_EQUAL               949
#define ITM_GREATER_EQUAL            950
#define ITM_MUCH_LESS                951
#define ITM_MUCH_GREATER             952
#define ITM_SUN                      953
#define ITM_TRANSPOSED               954
#define ITM_PERPENDICULAR            955
#define ITM_XOR                      956
#define ITM_NAND                     957
#define ITM_NOR                      958
#define ITM_WATCH                    959
#define ITM_HOURGLASS                960
#define ITM_PRINTER                  961
#define ITM_MAT_TL                   962
#define ITM_MAT_ML                   963
#define ITM_MAT_BL                   964
#define ITM_MAT_TR                   965
#define ITM_MAT_MR                   966
#define ITM_MAT_BR                   967
#define ITM_OBLIQUE1                 968
#define ITM_OBLIQUE2                 969
#define ITM_OBLIQUE3                 970
#define ITM_OBLIQUE4                 971
#define ITM_CURSOR                   972
#define ITM_PERIOD34                 973
#define ITM_COMMA34                  974
#define ITM_BATTERY                  975
#define ITM_PGM_BEGIN                976
#define ITM_USER_MODE                977
#define ITM_UK                       978
#define ITM_US                       979
#define ITM_NEG_EXCLAMATION_MARK     980
#define ITM_ex                       981
#define ITM_LF_PIC                   982
#define ITM_SMALL_SQUARE             983
#define ITM_0984                     984
#define ITM_0985                     985
#define ITM_0986                     986
#define ITM_0987                     987
#define ITM_0P                       988
#define ITM_1P                       989
#define ITM_EXPONENT                 990
#define ITM_litre                    991
#define ITM_M_GOTO_ROW               992
#define ITM_M_GOTO_COLUMN            993
#define ITM_SOLVE_VAR                994
#define ITM_EQ_LEFT                  995
#define ITM_EQ_RIGHT                 996
#define ITM_PAIR_OF_PARENTHESES      997
#define ITM_VERTICAL_BAR             998
#define ITM_ALOG_SIGN                999
#define ITM_ROOT_SIGN               1000
#define ITM_TIMER_SYMBOL            1001
#define ITM_Sfdx_VAR                1002
#define ITM_SUP_PLUS                1003
#define ITM_SUP_MINUS               1004
#define ITM_1005                    1005
#define ITM_SUP_INFINITY            1006
#define ITM_SUP_ASTERISK            1007
#define ITM_SUP_0                   1008
#define ITM_SUP_1                   1009
#define ITM_SUP_2                   1010
#define ITM_SUP_3                   1011
#define ITM_SUP_4                   1012
#define ITM_SUP_5                   1013
#define ITM_SUP_6                   1014
#define ITM_SUP_7                   1015
#define ITM_SUP_8                   1016
#define ITM_SUP_9                   1017
#define ITM_SUP_A                   1018
#define ITM_SUP_B                   1019
#define ITM_SUP_C                   1020
#define ITM_SUP_D                   1021
#define ITM_SUP_E                   1022
#define ITM_SUP_F                   1023
#define ITM_SUP_G                   1024
#define ITM_SUP_H                   1025
#define ITM_SUP_I                   1026
#define ITM_SUP_J                   1027
#define ITM_SUP_K                   1028
#define ITM_SUP_L                   1029
#define ITM_SUP_M                   1030
#define ITM_SUP_N                   1031
#define ITM_SUP_O                   1032
#define ITM_SUP_P                   1033
#define ITM_SUP_Q                   1034
#define ITM_SUP_R                   1035
#define ITM_SUP_S                   1036
#define ITM_SUP_T                   1037
#define ITM_SUP_U                   1038
#define ITM_SUP_V                   1039
#define ITM_SUP_W                   1040
#define ITM_SUP_X                   1041
#define ITM_SUP_Y                   1042
#define ITM_SUP_Z                   1043
#define ITM_SUP_a                   1044
#define ITM_SUP_b                   1045
#define ITM_SUP_c                   1046
#define ITM_SUP_d                   1047
#define ITM_SUP_e                   1048
#define ITM_SUP_f                   1049
#define ITM_SUP_g                   1050
#define ITM_SUP_h                   1051
#define ITM_SUP_i                   1052
#define ITM_SUP_j                   1053
#define ITM_SUP_k                   1054
#define ITM_SUP_l                   1055
#define ITM_SUP_m                   1056
#define ITM_SUP_n                   1057
#define ITM_SUP_o                   1058
#define ITM_SUP_p                   1059
#define ITM_SUP_q                   1060
#define ITM_SUP_r                   1061
#define ITM_SUP_s                   1062
#define ITM_SUP_t                   1063
#define ITM_SUP_u                   1064
#define ITM_SUP_v                   1065
#define ITM_SUP_w                   1066
#define ITM_SUP_x                   1067
#define ITM_SUP_y                   1068
#define ITM_SUP_z                   1069
#define ITM_SUB_alpha               1070
#define ITM_SUB_delta               1071
#define ITM_SUB_mu                  1072
#define ITM_SUB_SUN                 1073
#define ITM_SUB_EARTH               1074
#define ITM_SUB_PLUS                1075
#define ITM_SUB_MINUS               1076
#define ITM_SUB_INFINITY            1077
#define ITM_SUB_10                  1078
#define ITM_SUB_E_OUTLINE           1079
#define ITM_SUB_0                   1080
#define ITM_SUB_1                   1081
#define ITM_SUB_2                   1082
#define ITM_SUB_3                   1083
#define ITM_SUB_4                   1084
#define ITM_SUB_5                   1085
#define ITM_SUB_6                   1086
#define ITM_SUB_7                   1087
#define ITM_SUB_8                   1088
#define ITM_SUB_9                   1089
#define ITM_SUB_A                   1090
#define ITM_SUB_B                   1091
#define ITM_SUB_C                   1092
#define ITM_SUB_D                   1093
#define ITM_SUB_E                   1094
#define ITM_SUB_F                   1095
#define ITM_SUB_G                   1096
#define ITM_SUB_H                   1097
#define ITM_SUB_I                   1098
#define ITM_SUB_J                   1099
#define ITM_SUB_K                   1100
#define ITM_SUB_L                   1101
#define ITM_SUB_M                   1102
#define ITM_SUB_N                   1103
#define ITM_SUB_O                   1104
#define ITM_SUB_P                   1105
#define ITM_SUB_Q                   1106
#define ITM_SUB_R                   1107
#define ITM_SUB_S                   1108
#define ITM_SUB_T                   1109
#define ITM_SUB_U                   1110
#define ITM_SUB_V                   1111
#define ITM_SUB_W                   1112
#define ITM_SUB_X                   1113
#define ITM_SUB_Y                   1114
#define ITM_SUB_Z                   1115
#define ITM_SUB_a                   1116
#define ITM_SUB_b                   1117
#define ITM_SUB_c                   1118
#define ITM_SUB_d                   1119
#define ITM_SUB_e                   1120
#define ITM_SUB_f                   1121
#define ITM_SUB_g                   1122
#define ITM_SUB_h                   1123
#define ITM_SUB_i                   1124
#define ITM_SUB_j                   1125
#define ITM_SUB_k                   1126
#define ITM_SUB_l                   1127
#define ITM_SUB_m                   1128
#define ITM_SUB_n                   1129
#define ITM_SUB_o                   1130
#define ITM_SUB_p                   1131
#define ITM_SUB_q                   1132
#define ITM_SUB_r                   1133
#define ITM_SUB_s                   1134
#define ITM_SUB_t                   1135
#define ITM_SUB_u                   1136
#define ITM_SUB_v                   1137
#define ITM_SUB_w                   1138
#define ITM_SUB_x                   1139
#define ITM_SUB_y                   1140
#define ITM_SUB_z                   1141
#define ITM_SUB_pi                  1142
#define ITM_SUP_pi                  1143
#define ITM_CB_OFF                  1144
#define ITM_CB_ON                   1145
#define ITM_RB_OFF                  1146
#define ITM_RB_ON                   1147
#define ITM_ALTERN_CURRENT          1148
#define ITM_ANGLE                   1149
#define ITM_BATTERYHALF             1150
#define ITM_DIA_OFF                 1151
#define ITM_CYCLIC                  1152
#define ITM_DIRECT_CURRENT          1153
#define ITM_DOWN_DASHARROW          1154
#define ITM_EulerE                  1155
#define ITM_INTEGER_Z               1156
#define ITM_LEFT_DASHARROW          1157
#define ITM_NOT_SUBSET_OF           1158
#define ITM_op_i_char               1159
#define ITM_op_j_char               1160
#define ITM_POLAR_char              1161
#define ITM_DIA_ON                  1162
#define ITM_RIGHT_DASHARROW         1163
#define ITM_RIGHT_DOUBLE_ARROW      1164
#define ITM_RIGHT_TACK              1165
#define ITM_1166                    1166
#define ITM_SUBSET_OF               1167
#define ITM_SUM_char                1168
#define ITM_UP_DASHARROW            1169
#define ITM_USB_SYMBOL              1170
#define ITM_LEFT_RIGHT_DOUBLE_ARROW 1171
#define ITM_CR                      1172
#define ITM_TIME_T                  1173
#define ITM_DATE_D                  1174


// Reserved variables
#define VAR_REGX                    1175
#define VAR_REGY                    1176
#define VAR_REGZ                    1177
#define VAR_REGT                    1178
#define VAR_REGA                    1179
#define VAR_REGB                    1180
#define VAR_REGC                    1181
#define VAR_REGD                    1182
#define VAR_REGL                    1183
#define VAR_REGI                    1184
#define VAR_REGJ                    1185
#define VAR_REGK                    1186
#define VAR_1187                    1187
#define VAR_1188                    1188
#define VAR_1189                    1189
#define VAR_1190                    1190
#define VAR_1191                    1191
#define VAR_ACC                     1192
#define VAR_ULIM                    1193
#define VAR_LLIM                    1194
#define VAR_FV                      1195
#define VAR_IPonA                   1196
#define VAR_NPPER                   1197
#define VAR_PPERonA                 1198
#define VAR_PMT                     1199
#define VAR_PV                      1200
#define VAR_1201                    1201
#define VAR_MATA                    1202
#define VAR_MATB                    1203
#define VAR_MATX                    1204
#define VAR_UX                      1205
#define VAR_LX                      1206


// Probability distributions
#define ITM_1207                    1207 // General items / any items spare
#define ITM_BINOMP                  1208
#define ITM_BINOM                   1209
#define ITM_BINOMU                  1210
#define ITM_BINOMM1                 1211
#define ITM_1212                    1212 // General items / any items spare
#define ITM_CAUCHP                  1213
#define ITM_CAUCH                   1214
#define ITM_CAUCHU                  1215
#define ITM_CAUCHM1                 1216
#define ITM_1217                    1217 // General items / any items spare
#define ITM_EXPONP                  1218
#define ITM_EXPON                   1219
#define ITM_EXPONU                  1220
#define ITM_EXPONM1                 1221
#define ITM_1222                    1222 // General items / any items spare
#define ITM_FPX                     1223
#define ITM_FX                      1224
#define ITM_FUX                     1225
#define ITM_FM1P                    1226
#define ITM_1227                    1227 // General items / any items spare
#define ITM_GEOMP                   1228
#define ITM_GEOM                    1229
#define ITM_GEOMU                   1230
#define ITM_GEOMM1                  1231
#define ITM_1232                    1232 // General items / any items spare
#define ITM_HYPERP                  1233
#define ITM_HYPER                   1234
#define ITM_HYPERU                  1235
#define ITM_HYPERM1                 1236
#define ITM_1237                    1237 // General items / any items spare
#define ITM_LGNRMP                  1238
#define ITM_LGNRM                   1239
#define ITM_LGNRMU                  1240
#define ITM_LGNRMM1                 1241
#define ITM_1242                    1242 // General items / any items spare
#define ITM_LOGISP                  1243
#define ITM_LOGIS                   1244
#define ITM_LOGISU                  1245
#define ITM_LOGISM1                 1246
#define ITM_1247                    1247 // General items / any items spare
#define ITM_NBINP                   1248
#define ITM_NBIN                    1249
#define ITM_NBINU                   1250
#define ITM_NBINM1                  1251
#define ITM_1252                    1252 // General items / any items spare
#define ITM_NORMLP                  1253
#define ITM_NORML                   1254
#define ITM_NORMLU                  1255
#define ITM_NORMLM1                 1256
#define ITM_1257                    1257 // General items / any items spare
#define ITM_POISSP                  1258
#define ITM_POISS                   1259
#define ITM_POISSU                  1260
#define ITM_POISSM1                 1261
#define ITM_1262                    1262 // General items / any items spare
#define ITM_TPX                     1263
#define ITM_TX                      1264
#define ITM_TUX                     1265
#define ITM_TM1P                    1266
#define ITM_1267                    1267 // General items / any items spare
#define ITM_WEIBLP                  1268
#define ITM_WEIBL                   1269
#define ITM_WEIBLU                  1270
#define ITM_WEIBLM1                 1271
#define ITM_1272                    1272 // General items / any items spare
#define ITM_chi2Px                  1273
#define ITM_chi2x                   1274
#define ITM_chi2ux                  1275
#define ITM_chi2M1                  1276
#define ITM_1277                    1277 // General items / any items spare
#define ITM_STDNORMLP               1278
#define ITM_STDNORML                1279
#define ITM_STDNORMLU               1280
#define ITM_STDNORMLM1              1281
#define ITM_GEVP                    1282
#define ITM_GEV                     1283
#define ITM_GEVU                    1284
#define ITM_GEVM1                   1285
#define ITM_1286                    1286 // General items / any items spare
#define ITM_PARETOP                 1287
#define ITM_PARETOL                 1288
#define ITM_PARETOU                 1289
#define ITM_PARETOM1                1290
#define ITM_PARETO2P                1291
#define ITM_PARETO2L                1292
#define ITM_PARETO2U                1293
#define ITM_PARETO2M1               1294
#define ITM_1295                    1295
#define ITM_1296                    1296


// Curve fitting
#define ITM_BESTF_OLD               1297
#define ITM_T_EXPF                  1298
#define ITM_T_LINF                  1299
#define ITM_T_LOGF                  1300
#define ITM_T_ORTHOF                1301
#define ITM_T_POWERF                1302
#define ITM_T_GAUSSF                1303
#define ITM_T_CAUCHF                1304
#define ITM_T_PARABF                1305
#define ITM_T_HYPF                  1306
#define ITM_T_ROOTF                 1307
#define ITM_SETALLF                 1308
#define ITM_RSTF                    1309
#define ITM_BESTF                   1310
#define ITM_1311                    1311
#define ITM_1312                    1312


// Menus
#define ITM_1313                    1313 // General items / any items spare (reserved: FIN12C, PGUP, the common paging key of every table screen)
#define ITM_1314                    1314 // General items / any items spare (reserved: FIN12C, PGDN, the common paging key of every table screen)
#define ITM_1315                    1315 // General items / any items spare (reserved: FIN12C, PGTOP, the common paging key of every table screen)
#define ITM_1316                    1316 // General items / any items spare (reserved: FIN12C, PGBTM, the common paging key of every table screen)
#define ITM_RM_HALF_EVEN            1317
#define ITM_RM_HALF_UP              1318
#define ITM_RM_HALF_DOWN            1319
#define ITM_RM_UP                   1320
#define ITM_RM_DOWN                 1321
#define ITM_RM_CEILING              1322
#define ITM_RM_FLOOR                1323
#define ITM_ROUNDMM                 1324
#define ITM_SET_RM                  1325
#define ITM_1326                    1326 // General items / any items spare
#define ITM_SET_42ALPHAX            1327
#define ITM_GET_LPFCT               1328
#define ITM_SET_LPFCT               1329
#define ITM_GET_DPFCT               1330
#define ITM_SET_DPFCT               1331
#define ITM_SET_42ALPHA             1332
#define ITM_GET_42ALPHA             1333
#define ITM_BASEMENU                1334
#define ITM_DRM                     1335
#define ITM_SET_DRM                 1336
#define ITM_GET_DRM                 1337
#define ITM_1338                    1338 // General items / any items spare
#define ITM_1339                    1339 // General items / any items spare
#define ITM_ATEXT                   1340
#define ITM_GET_GRFNT               1341
#define ITM_SET_GRFNT               1342
#define ITM_1343                    1343 // General items / any items spare (reserved: graphic primitives)
#define ITM_1344                    1344 // General items / any items spare (reserved: graphic primitives)
#define ITM_1345                    1345 // General items / any items spare (reserved: graphic primitives)
#define ITM_1346                    1346 // General items / any items spare (reserved: graphic primitives)
#define ITM_1347                    1347 // General items / any items spare (reserved: graphic primitives)
#define ITM_1348                    1348 // General items / any items spare (reserved: graphic primitives)
#define ITM_1349                    1349 // General items / any items spare (reserved: graphic primitives)
#define ITM_1350                    1350 // General items / any items spare (reserved: graphic primitives)
#define ITM_1351                    1351 // General items / any items spare
#define ITM_1352                    1352 // General items / any items spare
#define ITM_1353                    1353 // General items / any items spare
#define ITM_1354                    1354 // General items / any items spare
#define ITM_1355                    1355 // General items / any items spare
#define ITM_1356                    1356 // General items / any items spare
#define ITM_1357                    1357 // General items / any items spare
#define ITM_1358                    1358 // General items / any items spare
#define ITM_1359                    1359 // General items / any items spare
#define ITM_1360                    1360 // General items / any items spare
#define ITM_1361                    1361 // General items / any items spare
#define ITM_1362                    1362 // General items / any items spare
#define ITM_1363                    1363 // General items / any items spare
#define ITM_1364                    1364 // General items / any items spare
#define ITM_1365                    1365 // General items / any items spare
#define ITM_1366                    1366 // General items / any items spare
#define ITM_1367                    1367 // General items / any items spare
#define ITM_1368                    1368 // General items / any items spare
#define ITM_1369                    1369 // General items / any items spare
#define ITM_1370                    1370 // General items / any items spare
#define ITM_1371                    1371 // General items / any items spare
#define ITM_1372                    1372 // General items / any items spare
#define ITM_1373                    1373 // General items / any items spare
#define ITM_1374                    1374 // General items / any items spare
#define ITM_1375                    1375 // General items / any items spare
#define ITM_1376                    1376 // General items / any items spare
#define ITM_1377                    1377 // General items / any items spare
#define ITM_1378                    1378 // General items / any items spare
#define ITM_1379                    1379 // General items / any items spare
#define ITM_1380                    1380 // General items / any items spare
#define ITM_1381                    1381 // General items / any items spare
#define ITM_1382                    1382 // General items / any items spare
#define ITM_1383                    1383 // General items / any items spare
#define ITM_1384                    1384 // General items / any items spare
#define ITM_1385                    1385 // General items / any items spare
#define ITM_1386                    1386 // General items / any items spare
#define ITM_1387                    1387 // General items / any items spare
#define ITM_1388                    1388 // General items / any items spare
#define ITM_1389                    1389 // General items / any items spare
#define ITM_1390                    1390 // General items / any items spare
#define ITM_1391                    1391 // General items / any items spare
#define ITM_1392                    1392 // General items / any items spare
#define ITM_1393                    1393 // General items / any items spare
#define ITM_1394                    1394 // General items / any items spare
#define ITM_1395                    1395 // General items / any items spare
#define ITM_1396                    1396 // General items / any items spare
#define ITM_1397                    1397 // General items / any items spare
#define ITM_1398                    1398 // General items / any items spare
#define ITM_1399                    1399 // General items / any items spare
#define ITM_1400                    1400 // General items / any items spare (reserved: FIN12C, TVM PXN)
#define VAR_Ip                      1401
#define ITM_CLRTVM                  1402
#define ITM_RSTTVM                  1403


#define ITM_1COMPL                  1404
#define ITM_SNAP                    1405
#define ITM_2COMPL                  1406
#define ITM_ABS                     1407
#define ITM_AGM                     1408
#define ITM_AGRAPH                  1409
#define ITM_ALL                     1410
#define ITM_ASSIGN                  1411
#define ITM_BACK                    1412
#define ITM_BATT                    1413
#define ITM_BEEP                    1414
#define ITM_BEGINP                  1415
#define ITM_BN                      1416
#define ITM_BNS                     1417
#define ITM_CASE                    1418
#define ITM_DELALL                  1419
#define ITM_CLCVAR                  1420
#define ITM_CLFALL                  1421
#define ITM_TGLFRT                  1422
#define ITM_CLLCD_XY                1423
#define ITM_CLMENU                  1424
#define ITM_DELP                    1425
#define ITM_DELPALL                 1426
#define ITM_CLREGS                  1427
#define ITM_CLSTK                   1428
#define ITM_CLSIGMA                 1429
#define ITM_STOMAX                  1430
#define ITM_CONJ                    1431
#define ITM_RCLMAX                  1432
#define ITM_CORR                    1433
#define ITM_COV                     1434
#define ITM_BESTFQ                  1435
#define ITM_CROSS_PROD              1436
#define ITM_CXtoRE                  1437
#define ITM_DATE                    1438
#define ITM_DATEto                  1439
#define ITM_DAY                     1440
#define ITM_DBLR                    1441
#define ITM_DBLMULT                 1442
#define ITM_DBLDIV                  1443
#define ITM_DECOMP                  1444
#define ITM_DEG                     1445
#define ITM_LOADEDFILE              1446
#define ITM_SA                      1447
#define ITM_PMT_Beg                 1448
#define ITM_DOT_PROD                1449
#define ITM_DSTACK                  1450
#define ITM_DMS                     1451
#define ITM_CLFIN                   1452
#define ITM_DMY                     1453
#define ITM_DtoJ                    1454
#define ITM_DELITM                  1455
#define ITM_EIGVAL                  1456
#define ITM_EIGVEC                  1457
#define ITM_END                     1458
#define ITM_ENDP                    1459
#define ITM_ENG                     1460
#define ITM_ENORM                   1461
#define ITM_RCLMIN                  1462
#define ITM_EQ_DEL                  1463
#define ITM_EQ_EDI                  1464
#define ITM_EQ_NEW                  1465
#define ITM_ERF                     1466
#define ITM_ERFC                    1467
#define ITM_ERR                     1468
#define ITM_EXITALL                 1469
#define ITM_EXPT                    1470
#define ITM_GET_JUL_GREG            1471
#define ITM_FIB                     1472
#define ITM_FIX                     1473
#define ITM_DISK                    1474
#define ITM_FQX                     1475
#define ITM_FDQX                    1476
#define ITM_FACTORS                 1477
#define ITM_GD                      1478
#define ITM_GDM1                    1479
#define ITM_GRAD                    1480
#define ITM_GETFDIGS                1481
#define ITM_GTOP                    1482
#define ITM_HN                      1483
#define ITM_HNP                     1484
#define ITM_IM                      1485
#define ITM_INDEX                   1486
#define ITM_IXYZ                    1487
#define ITM_IGAMMAP                 1488
#define ITM_IGAMMAQ                 1489
#define ITM_IPLUS                   1490
#define ITM_IMINUS                  1491
#define ITM_JYX                     1492
#define ITM_JPLUS                   1493
#define ITM_JMINUS                  1494
#define ITM_JUL_GREG                1495
#define ITM_MSG                     1496
#define ITM_KEY                     1497
#define ITM_KEYG                    1498
#define ITM_KEYX                    1499
#define ITM_sinc                    1500
#define ITM_KTYP                    1501
#define ITM_LASTX                   1502
#define ITM_LBLQ                    1503
#define ITM_LEAPQ                   1504
#define ITM_Lm                      1505
#define ITM_LmALPHA                 1506
#define ITM_LNBETA                  1507
#define ITM_LNGAMMA                 1508
#define ITM_LOAD                    1509
#define ITM_LOADP                   1510
#define ITM_LOADR                   1511
#define ITM_LOADSS                  1512
#define ITM_LOADSIGMA               1513
#define ITM_LocR                    1514
#define ITM_LocRQ                   1515
#define ITM_LR                      1516
#define ITM_MANT                    1517
#define ITM_MATX                    1518
#define ITM_MEM                     1519
#define ITM_MENU                    1520
#define ITM_MONTH                   1521
#define ITM_MSG_OLD                 1522
#define ITM_MULPI                   1523
#define ITM_MVAR                    1524
#define ITM_M_DELR                  1525
#define ITM_M_DIM                   1526
#define ITM_M_DIMQ                  1527
#define ITM_MDY                     1528
#define ITM_M_EDI                   1529
#define ITM_M_EDIN                  1530
#define ITM_M_GET                   1531
#define ITM_M_GOTO                  1532
#define ITM_M_GROW                  1533
#define ITM_M_INSR                  1534
#define ITM_M_LU                    1535
#define ITM_M_NEW                   1536
#define ITM_M_OLD                   1537
#define ITM_M_PUT                   1538
#define ITM_M_RR                    1539
#define ITM_sincpi                  1540
#define ITM_M_WRAP                  1541
#define ITM_NOP                     1542
#define ITM_OFF                     1543
#define ITM_DROPY                   1544
#define ITM_STOMIN                  1545
#define ITM_PGMINT                  1546
#define ITM_PGMSLV                  1547
#define ITM_PIXEL                   1548
#define ITM_PLOT_SCATR              1549
#define ITM_Pn                      1550
#define ITM_POINT                   1551
#define ITM_LOADV                   1552
#define ITM_POPLR                   1553
#define ITM_REM                     1554
#define ITM_SLVC                    1555
#define ITM_PUTK                    1556
#define ITM_RAD                     1557
#define ITM_GET_DMX                 1558
#define ITM_RAN                     1559
#define ITM_RBR                     1560
#define ITM_RCLCFG                  1561
#define ITM_RCLEL                   1562
#define ITM_RCLIJ                   1563
#define ITM_RCLS                    1564
#define ITM_RDP                     1565
#define ITM_RE                      1566
#define ITM_READP                   1567
#define ITM_RESET                   1568
#define ITM_REtoCX                  1569
#define ITM_REexIM                  1570
#define ITM_DELITM_PROG             1571
#define ITM_DELITM_MENU             1572
#define ITM_DSP                     1573
#define ITM_RNORM                   1574
#define ITM_EX1                     1575
#define ITM_EXPORTP                 1576
#define ITM_RSD                     1577
#define ITM_RSUM                    1578
#define ITM_RTNP1                   1579
#define ITM_R_CLR                   1580
#define ITM_R_COPY                  1581
#define ITM_R_SORT                  1582
#define ITM_R_SWAP                  1583
#define ITM_am                      1584
#define ITM_STDDEVWEIGHTED          1585
#define ITM_SAVE                    1586
#define ITM_SCI                     1587
#define ITM_GETSDIGS                1588
#define ITM_SEED                    1589
#define ITM_WRITEP                  1590
#define ITM_SETCHN                  1591
#define ITM_SETDAT                  1592
#define ITM_SETEUR                  1593
#define ITM_SETIND                  1594
#define ITM_SETJPN                  1595
#define ITM_SETDFLT                 1596
#define ITM_SETTIM                  1597
#define ITM_SETUK                   1598
#define ITM_SETUSA                  1599
#define ITM_SIGN                    1600
#define ITM_SIGNMT                  1601
#define ITM_SIM_EQ                  1602
#define ITM_SKIP                    1603
#define ITM_SLVQ                    1604
#define ITM_SM                      1605
#define ITM_GET_ISM                 1606
#define ITM_SMW                     1607
#define ITM_SOLVE                   1608
#define ITM_SSIZE                   1609
#define ITM_STATUS                  1610
#define ITM_STOCFG                  1611
#define ITM_STOEL                   1612
#define ITM_STOIJ                   1613
#define ITM_LN1X                    1614
#define ITM_STOS                    1615
#define ITM_SUM                     1616
#define ITM_SW                      1617
#define ITM_SXY                     1618
#define ITM_TDISP                   1619
#define ITM_TICKS                   1620
#define ITM_TIME                    1621
#define ITM_TIMER                   1622
#define ITM_Tn                      1623
#define ITM_TONE                    1624
#define ITM_Tex                     1625
#define ITM_ULP                     1626
#define ITM_Un                      1627
#define ITM_UNITV                   1628
#define ITM_UNSIGN                  1629
#define ITM_VARMNU                  1630
#define ITM_VERS                    1631
#define ITM_IDIVR                   1632
#define ITM_WDAY                    1633
#define ITM_WHO                     1634
#define ITM_WM                      1635
#define ITM_WP                      1636
#define ITM_WM1                     1637
#define ITM_WSIZE                   1638
#define ITM_WSIZEQ                  1639
#define ITM_XBAR                    1640
#define ITM_XG                      1641
#define ITM_XW                      1642
#define ITM_XCIRC                   1643
#define ITM_XtoDATE                 1644
#define ITM_XtoALPHA_OLD            1645 // deprecated, only for backward compatibility
#define ITM_M_QR                    1646
#define ITM_YEAR                    1647
#define ITM_YCIRC                   1648
#define ITM_YMD                     1649
#define ITM_Yex                     1650
#define ITM_Zex                     1651
#define ITM_ALPHALENG               1652
#define ITM_XMAX                    1653
#define ITM_XMIN                    1654
#define ITM_ALPHAPOS                1655
#define ITM_ALPHARL                 1656
#define ITM_ALPHARR                 1657
#define ITM_ALPHASL                 1658
#define ITM_ALPHASR                 1659
#define ITM_ALPHAtoX                1660
#define ITM_BETAXY                  1661
#define ITM_gammaXY                 1662
#define ITM_GAMMAXY                 1663
#define ITM_GAMMAX                  1664
#define ITM_YYX                     1665
#define ITM_DELTAPC                 1666
#define ITM_SCATTFACT               1667
#define ITM_SCATTFACTm              1668
#define ITM_SCATTFACTp              1669
#define ITM_zetaX                   1670
#define ITM_PIn                     1671
#define ITM_SIGMAn                  1672
#define ITM_STDDEV                  1673
#define ITM_STDDEVPOP               1674
#define ITM_RANI                    1675
#define ITM_PRINTERX                1676
#define ITM_LASTT                   1677
#define ITM_GETRANGE                1678
#define ITM_M1X                     1679
#define ITM_XMOD                    1680
#define ITM_toDATE                  1681
#define ITM_sn                      1682
#define ITM_cn                      1683
#define ITM_dn                      1684
#define ITM_toHR_OLD                1685 //reserved, not used anymore, removed from testPgms
#define ITM_HRtoTM                  1686
#define ITM_toINT                   1687
#define ITM_HMStoTM                 1688
#define ITM_BASENR                  1689
#define ITM_INTEGRAL_YX             1690   //INT
#define ITM_toREAL                  1691
#define ITM_PCSGM_DPCMN             1692
#define ITM_DPCMEAN                 1693
#define ITM_SHUFFLE                 1694
#define ITM_PC                      1695
#define ITM_PCMRR                   1696
#define ITM_PCT                     1697
#define ITM_PCSIGMA                 1698
#define ITM_PCPMG                   1699
#define ITM_INTEGRAL                1700   //INT
#define ITM_PMOD                    1701
#define ITM_M_DET                   1702
#define ITM_PARALLEL                1703
#define ITM_M_TRANSP                1704
#define ITM_M_INV                   1705
#define ITM_ARG                     1706
#define ITM_PRINTERXY               1707
#define ITM_PRINTERADV              1708
#define ITM_PRINTERCHAR_OLD         1709
#define ITM_PRINTERDLAY             1710
#define ITM_PRINTERLCD              1711
#define ITM_PRINTERMODE             1712
#define ITM_PRINTERPROG             1713
#define ITM_PRINTERR                1714
#define ITM_PRINTERREGS             1715
#define ITM_PRINTERSTK              1716
#define ITM_PRINTERTAB_OLD          1717
#define ITM_PRINTERUSER             1718
#define ITM_PRINTERWIDTH            1719
#define ITM_PRINTERSIGMA            1720
#define ITM_PRINTERHASH_OLD         1721

#define ITM_FBR                     1722

#define ITM_UNDO                    1723
#define ITM_PR                      1724
#define ITM_RS                      1725
#define ITM_Kk                      1726
#define ITM_Ek                      1727
#define ITM_PInk                    1728
#define ITM_USERMODE                1729
#define ITM_CC                      1730
#define ITM_SHIFTf                  1731
#define ITM_SHIFTg                  1732
#define ITM_UP1                     1733
#define ITM_BST                     1734
#define ITM_DOWN1                   1735
#define ITM_SST                     1736
#define ITM_EXIT1                   1737
#define ITM_BACKSPACE               1738
#define ITM_M_DIM_GR                1739
#define ITM_AIM                     1740
#define ITM_dotD                    1741
#define ITM_SHOW                    1742
#define ITM_XMEAN                   1743
#define ITM_FRACT                   1744
#define ITM_VANGLE                  1745
#define ITM_XH                      1746
#define ITM_XRMS                    1747
#define ITM_ACOS                    1748
#define ITM_ASIN                    1749
#define ITM_ATAN                    1750
#define ITM_DET                     1751
#define ITM_INVRT                   1752
#define ITM_TRANS                   1753
#define ITM_iPIn                    1754
#define ITM_iSIGMAn                 1755
#define ITM_PLOT_CENTRL             1756
#define ITM_HIDE_OLD                1757
#define ITM_SMI                     1758
#define ITM_PLOT_ASSESS             1759
#define ITM_PLOT_NXT                1760
#define ITM_PLOT_REV                1761
#define ITM_PLOTZOOM                1762
#define ITM_Fphik                   1763
#define ITM_Ephik                   1764
#define ITM_ZETAphik                1765
#define ITM_GETHIDE                 1766
#define ITM_CALC                    1767
#define ITM_SQRT                    1768
#define ITM_RCL_FV                  1769
#define ITM_RCL_IPonA               1770
#define ITM_RCL_NPPER               1771
#define ITM_RCL_PPERonA             1772
#define ITM_RCL_PMT                 1773
#define ITM_RCL_PV                  1774
#define ITM_atan2                   1775
#define ITM_EFFTOI                  1776
#define ITM_TIMER_0_1S              1777
#define ITM_TIMER_RESET             1778
#define ITM_TIMER_RCL               1779
#define ITM_DELBKUP                 1780
#define ITM_ITOEFF                  1781
#define ITM_TIMER_SIGMA_T           1782
#define ITM_TIMER_SIGMA_L           1783
#define ITM_TIMER_R_T               1784
#define ITM_TIMER_R_L               1785
#define ITM_TIMER_R_S               1786
#define ITM_nBINS                   1787
#define ITM_LOBIN                   1788
#define ITM_HIBIN                   1789
#define ITM_HISTOX                  1790
#define ITM_HISTOY                  1791
#define ITM_HPLOT                   1792
#define ITM_HNORM                   1793
#define ITM_SQRT1PX2                1794



//C47 extensions
#define ITM_op_j_pol                1795
#define ITM_BASE_MYM                1796
#define ITM_G_DOUBLETAP             1797
#define ITM_CPXMULT                 1798
#define ITM_P_ALLREGS               1799
#define ITM_SI_f                    1800
#define ITM_SI_p                    1801
#define ITM_SI_n                    1802
#define ITM_SI_u                    1803
#define ITM_SI_m                    1804
#define ITM_SI_k                    1805
#define ITM_SI_M                    1806
#define ITM_SI_G                    1807
#define ITM_SI_T                    1808
#define ITM_QOPPA                   1809
#define ITM_DIGAMMA                 1810
#define ITM_SAMPI                   1811
#define ITM_EE_D2Y                  1812
#define ITM_EE_Y2D                  1813
#define ITM_EE_A2S                  1814
#define ITM_EE_S2A                  1815
#define ITM_EE_EXP_TH               1816
#define ITM_EE_STO_Z                1817
#define ITM_EE_RCL_Z                1818
#define ITM_EE_STO_V                1819
#define ITM_EE_RCL_V                1820
#define ITM_EE_STO_I                1821
#define ITM_EE_RCL_I                1822
#define ITM_EE_STO_V_I              1823
#define ITM_EE_STO_IR               1824
#define ITM_EE_STO_V_Z              1825
#define ITM_EE_X2BAL                1826
#define ITM_MATX_A                  1827
#define ITM_op_a                    1828
#define ITM_op_a2                   1829
#define ITM_op_j                    1830
#define ITM_2BIN                    1831
#define ITM_2OCT                    1832
#define ITM_2DEC                    1833
#define ITM_2HEX                    1834
#define ITM_WS8                     1835
#define ITM_WS16                    1836
#define ITM_WS32                    1837
#define ITM_WS64                    1838
#define ITM_HR_DEG                  1839
#define ITM_MINUTE                  1840
#define ITM_SECOND                  1841
#define ITM_toTIME                  1842
#define ITM_TIMEto                  1843
#define ITM_COUNTDN_RCL             1844
#define ITM_qoppa                   1845
#define ITM_digamma                 1846
#define ITM_sampi                   1847
#define KEY_COMPLEX                 1848
#define ITM_toPOL2                  1849
#define ITM_toREC2                  1850
#define ITM_CARRY                   1851
#define ITM_OVERFLOW                1852
#define ITM_ERPN                    1853
#define ITM_HOMEx3                  1854
#define ITM_SHTIM                   1855
#define ITM_CB_CPXRES               1856
#define ITM_CB_LEADING_ZERO         1857
#define CHR_case                    1858
#define ITM_BASE_HOME               1859
#define ITM_1860                    1860 // General items / any items spare
#define ITM_MYMx3                   1861
#define ITM_DTtoJ                   1862
#define ITM_JtoDT                   1863
#define ITM_DSPCYCLE                1864
#define ITM_LARGELI                 1865
#define ITM_SIGFIG                  1866
#define ITM_UNIT                    1867
#define ITM_ROUND2                  1868
#define ITM_ROUNDI2                 1869
#define ITM_DMPMNU                  1870
#define ITM_RI                      1871
#define ITM_HASH_JM                 1872
#define ITM_DRG                     1873
#define ITM_CLA                     1874
#define ITM_CLN                     1875
#define ITM_DENANY                  1876
#define ITM_DENFIX                  1877
#define ITM_CASEUP                  1878
#define ITM_CASEDN                  1879
#define ITM_LISTXY                  1880
#define ITM_SUPUP                   1881 // General items / any items spare
#define ITM_SYS_FREE_RAM            1882
#define ITM_SUBDN                   1883 // General items / any items spare
#define ITM_TEST                    1884
#define ITM_GET_TEST_BS             1885
#define ITM_SET_TEST_BS             1886
#define ITM_INP_DEF_DP              1887
#define ITM_PROPFR                  1888
#define ITM_INP_DEF_CPXDP           1889
#define ITM_INP_DEF_43S             1890
#define ITM_INP_DEF_LI              1891
#define ITM_SI_P                    1892
#define KEY_fg                      1893
#define ITM_FROM_USER               1894
#define ITM_HPBASE                  1895
#define ITM_FRCYC                   1896
#define ITM_FGGR                    1897
#define ITM_N_KEY_ALPHA             1898
#define ITM_DREAL                   1899
#define ITM_N_KEY_GSH               1900
#define ITM_1901                    1901 // General items / any items spare
#define ITM_GLUStoFZUS              1902
#define ITM_FZUStoGLUS              1903
#define ITM_N_KEY_USER              1904
#define ITM_LASTERR                 1905
#define ITM_N_KEY_SIGMA             1906
#define ITM_1907                    1907 // General items / any items spare
#define ITM_N_KEY_FSH               1908
#define ITM_ms                      1909
#define ITM_msTo                    1910
#define ITM_ymdTo                   1911
#define ITM_FINISH                  1912
#define ITM_1913                    1913 // General items / any items spare
#define ITM_TO_USER                 1914
#define ITM_N_KEY_NIL               1915
#define ITM_USER_DM42               1916
#define ITM_HPRP                    1917
#define ITM_N_KEY_FGSH              1918
#define ITM_YY_OFF                  1919
#define ITM_1920                    1920 // General items / any items spare
#define ITM_1921                    1921 // General items / any items spare
#define ITM_1922                    1922 // General items / any items spare
#define ITM_1923                    1923 // General items / any items spare
#define ITM_MNUp1                   1924
#define ITM_1925                    1925 // General items / any items spare
#define ITM_T_UP_ARROW              1926
#define ITM_1927                    1927 // General items / any items spare
#define ITM_T_DOWN_ARROW            1928
#define ITM_T_HOME                  1929
#define ITM_T_END                   1930
#define ITM_STKTO3x1                1931
#define ITM_XPARSE                  1932
#define ITM_WRXPALL                 1933
#define ITM_RNG_OLD                 1934
#define ITM_FLGSV                   1935
#define ITM_CPXI                    1936
#define ITM_CPXJ                    1937
#define ITM_SSIZE4                  1938
#define ITM_SSIZE8                  1939
#define ITM_CB_SPCRES               1940
#define ITM_CFG                     1941
#define ITM_CLK12                   1942
#define ITM_CLK24                   1943
#define ITM_MULTCR                  1944
#define ITM_MULTDOT                 1945
#define ITM_POLAR                   1946
#define ITM_RESERVE2                1947
#define ITM_PRTACT                  1948
#define ITM_RECT                    1949
#define ITM_SCIOVR                  1950
#define ITM_ENGOVR                  1951
#define ITM_T_LEFT_ARROW            1952
#define ITM_T_RIGHT_ARROW           1953
#define ITM_T_LLEFT_ARROW           1954
#define ITM_T_RRIGHT_ARROW          1955
#define ITM_RLC1                    1956
#define ITM_RRC1                    1957
#define ITM_KEYMAP                  1958
#define ITM_USER_C47                1959
#define ITM_USER_V47                1960
#define ITM_USER_D47                1961
#define ITM_USER_N47                1962
#define ITM_USER_E47                1963
#define ITM_USER_R47                1964
#define ITM_SLVP                    1965
#define ITM_STKtoV4                 1966
#define ITM_V4toSTK                 1967
#define ITM_stkexV4                 1968
#define ITM_VtoEQ                   1969
#define ITM_RXtoVEC                 1970
#define ITM_XtoPOLY                 1971
#define ITM_VtoSUM0                 1972
#define ITM_1973                    1973
#define ITM_N_KEY_XFACT             1974
#define ITM_N_KEY_DRG               1975
#define ITM_N_KEY_Rup               1976
#define ITM_N_KEY_op_j_pol          1977
#define ITM_N_KEY_SNAP              1978
#define ITM_N_KEY_STOPW             1979
#define ITM_PCURVE                  1980
#define ITM_toPOL_HP                1981
#define ITM_toREC_HP                1982
#define ITM_toPOL_CX                1983
#define ITM_toREC_CX                1984
#define ITM_BCD                     1985
#define ITM_BCD9                    1986
#define ITM_BCD10                   1987
#define ITM_BCDU                    1988
#define ITM_S06                     1989
#define ITM_S08                     1990
#define ITM_S16                     1991
#define ITM_S32                     1992
#define ITM_S64                     1993
#define ITM_U06                     1994
#define ITM_U08                     1995
#define ITM_U16                     1996
#define ITM_U32                     1997
#define ITM_U64                     1998
#define ITM_SL1                     1999
#define ITM_SR1                     2000
#define ITM_RL1                     2001
#define ITM_RR1                     2002
#define ITM_FWORD                   2003
#define ITM_FBYTE                   2004
#define ITM_CLRMOD                  2005
#define ITM_SHOIREP                 2006
#define ITM_SCALE                   2007
#define ITM_TOPHEX                  2008
#define ITM_PLINE                   2009
#define ITM_PCROS                   2010
#define ITM_PBOX                    2011
#define ITM_VECT                    2012
#define ITM_NVECT                   2013
#define ITM_SHOWX                   2014
#define ITM_SHOWY                   2015
#define ITM_DENMAX2_OLDER           2016
#define ITM_SETSIG2_OLD             2017
#define ITM_AVIEW                   2018
#define ITM_GET_RM                  2019
#define ITM_PROMPT                  2020
#define ITM_USER_ARESET             2021
#define ITM_USER_MRESET             2022
#define ITM_USER_KRESET             2023
#define ITM_INTG                    2024
#define ITM_DIFF                    2025
#define ITM_RMS                     2026
#define ITM_SHADE                   2027
#define ITM_2028                    2028 // General items / any items spare
#define CHR_num                     2029
#define CHR_numL                    2030
#define CHR_numU                    2031
#define ITM_EEXCHR                  2032
#define ITM_CLGRF                   2033
#define ITM_MZOOMY                  2034
#define ITM_PZOOMY                  2035
#define ITM_2036                    2036 // General items / any items spare
#define ITM_2037                    2037 // General items / any items spare
#define ITM_SAFERESET               2038
#define ITM_SH_LONGPRESS            2039
#define ITM_PLOT_STAT               2040
#define ITM_3x1TOSTK                2041
#define ITM_PLOTRST                 2042
#define ITM_SYSTEM2                 2043
#define ITM_ACTUSB                  2044
#define ITM_2045                    2045 // General items / any items spare
#define ITM_2046                    2046 // General items / any items spare
#define ITM_2047                    2047 // General items / any items spare
#define ITM_2048                    2048  //blank
#define ITM_2049                    2049  //blank
#define ITM_SIGZEROS                2050
#define ITM_BOLD                    2051
#define ITM_AMORT_HP12C             2052
#define ITM_2TO10                   2053
#define ITM_USER_HRESET             2054
#define ITM_USER_PRESET             2055
#define ITM_IRFRAC                  2056
#define ITM_FGLNOFF                 2057
#define ITM_FGLNLIM                 2058
#define ITM_FGLNFUL                 2059
#define ITM_M124                    2060
#define ITM_F1234                   2061
#define ITM_M1234                   2062
#define ITM_F14                     2063
#define ITM_M14                     2064
#define ITM_F124                    2065
#define ITM_2066                    2066 // General items / any items spare
#define ITM_2067                    2067 // General items / any items spare
#define ITM_2068                    2068 // General items / any items spare
#define ITM_LG_SIGN                 2069
#define ITM_LN_SIGN                 2070
#define ITM_SIN_SIGN                2071
#define ITM_COS_SIGN                2072
#define ITM_TAN_SIGN                2073
#define ITM_MEDIAN                  2074
#define ITM_LOWER_QUARTILE          2075
#define ITM_UPPER_QUARTILE          2076
#define ITM_MAD                     2077
#define ITM_IQR                     2078
#define ITM_SIGMARANGE              2079
#define ITM_2080                    2080 // General items / any items spare
#define ITM_2081                    2081 // General items / any items spare
#define ITM_PERCENTILE              2082
#define ITM_LINPOL                  2083
#define ITM_KNOTtoKMH               2084
#define ITM_KMHtoKNOT               2085
#define ITM_KMHtoMPS                2086
#define ITM_MPStoKMH                2087
#define ITM_RPMtoDEGPS              2088
#define ITM_DEGPStoRPM              2089
#define ITM_MPHtoKMH                2090
#define ITM_KMHtoMPH                2091
#define ITM_MPHtoMPS                2092
#define ITM_MPStoMPH                2093
#define ITM_RPMtoRADPS              2094
#define ITM_RADPStoRPM              2095
#define ITM_DEGtoRAD                2096
#define ITM_RADtoDEG                2097
#define ITM_DEGtoGRAD               2098
#define ITM_GRADtoDEG               2099
#define ITM_GRADtoRAD               2100
#define ITM_RADtoGRAD               2101
#define ITM_2102                    2102 // General items / any items spare
#define ITM_2103                    2103 // General items / any items spare
#define ITM_KtoM                    2104
#define ITM_MtoK                    2105
#define ITM_2106                    2106 // General items / any items spare
#define ITM_2107                    2107 // General items / any items spare
#define ITM_2108                    2108 // General items / any items spare
#define ITM_2109                    2109 // General items / any items spare
#define ITM_TAMMAX                  2110
#define ITM_GAPDOT_L                2111
#define ITM_GAPWIDDOT_L             2112
#define ITM_GAPPER_L                2113
#define ITM_GAPWIDPER_L             2114
#define ITM_GAPCOM_L                2115
#define ITM_GAPWIDCOM_L             2116
#define ITM_GAPAPO_L                2117
#define ITM_GAPNARAPO_L             2118
#define ITM_GAPSPC_L                2119
#define ITM_GAPNARSPC_L             2120
#define ITM_GAPDBLSPC_L             2121
#define ITM_GAPUND_L                2122
#define ITM_GAPNIL_L                2123
#define ITM_GAPDOT_R                2124
#define ITM_GAPWIDDOT_R             2125
#define ITM_GAPPER_R                2126
#define ITM_GAPWIDPER_R             2127
#define ITM_GAPCOM_R                2128
#define ITM_GAPWIDCOM_R             2129
#define ITM_GAPAPO_R                2130
#define ITM_GAPNARAPO_R             2131
#define ITM_GAPSPC_R                2132
#define ITM_GAPNARSPC_R             2133
#define ITM_GAPDBLSPC_R             2134
#define ITM_GAPUND_R                2135
#define ITM_GAPNIL_R                2136
#define ITM_GAPDOT_RX               2137
#define ITM_GAPWIDDOT_RX            2138
#define ITM_GAPPER_RX               2139
#define ITM_GAPWIDPER_RX            2140
#define ITM_GAPCOM_RX               2141
#define ITM_GAPWIDCOM_RX            2142
#define ITM_WDOT                    2143
#define ITM_WPERIOD                 2144
#define ITM_WCOMMA                  2145
#define ITM_NQUOTE                  2146
#define ITM_JUL_GREG_1582           2147
#define ITM_JUL_GREG_1752           2148
#define ITM_JUL_GREG_1873           2149
#define ITM_JUL_GREG_1949           2150
#define ITM_2151                    2151 // General items / any items spare
#define ITM_GRP_HEX                 2152
#define ITM_GRP_BIN                 2153
#define ITM_SETFDIGS                2154
#define ITM_GRP_L                   2155
#define ITM_GRP1_L_OF               2156
#define ITM_GRP1_L                  2157
#define ITM_GRP_R                   2158
#define ITM_GAP_L                   2159
#define ITM_GAP_RX                  2160
#define ITM_GAP_R                   2161
#define ITM_PPLUS                   2162
#define ITM_INCHtoCM                2163
#define ITM_CMtoINCH                2164
#define ITM_op_j_SIGN               2165
#define ITM_poly_SIGN               2166
#define ITM_NMItoMI                 2167
#define ITM_MItoNMI                 2168
#define ITM_FURtoM                  2169
#define ITM_MtoFUR                  2170
#define ITM_FTNtoS                  2171
#define ITM_StoFTN                  2172
#define ITM_FPFtoMPS                2173
#define ITM_MPStoFPF                2174
#define ITM_BRDStoM                 2175
#define ITM_MtoBRDS                 2176
#define ITM_FIRtoKG                 2177
#define ITM_KGtoFIR                 2178
#define ITM_FPFtoKPH                2179
#define ITM_KPHtoFPF                2180
#define ITM_BRDStoIN                2181
#define ITM_INtoBRDS                2182
#define ITM_FIRtoLB                 2183
#define ITM_LBtoFIR                 2184
#define ITM_FPFtoMPH                2185
#define ITM_MPHtoFPF                2186
#define ITM_FPStoKMH                2187
#define ITM_KMHtoFPS                2188
#define ITM_FPStoMPS                2189
#define ITM_MPStoFPS                2190
#define ITM_SCR                     2191
#define ITM_SCRNRM                  2192
#define ITM_SCRSUP                  2193
#define ITM_SCRSUB                  2194
#define ITM_HIDE                    2195
#define ITM_DENMAX2_OLD             2196
#define ITM_SETSIG2                 2197
#define ITM_VOL                     2198
#define ITM_VOLQ                    2199
#define ITM_VOLPLUS                 2200
#define ITM_VOLMINUS                2201
#define ITM_BUZZ                    2202
#define ITM_PLAY                    2203
#define ITM_L100toKML               2204
#define ITM_KMLtoL100               2205
#define ITM_KMLEtoK100K             2206
#define ITM_K100KtoKMLE             2207
#define ITM_K100KtoKMK              2208
#define ITM_KMKtoK100K              2209
#define ITM_L100toMGUS              2210
#define ITM_MGUStoL100              2211
#define ITM_MGEUStoK100M            2212
#define ITM_K100MtoMGEUS            2213
#define ITM_K100KtoK100M            2214
#define ITM_K100MtoK100K            2215
#define ITM_L100toMGUK              2216
#define ITM_MGUKtoL100              2217
#define ITM_MGEUKtoK100M            2218
#define ITM_K100MtoMGEUK            2219
#define ITM_K100MtoMIK              2220
#define ITM_MIKtoK100M              2221
#define ITM_2222                    2222 // General items / any items spare
#define ITM_XEQP1                   2223
#define ITM_RCLP1                   2224
#define ITM_2225                    2225 // General items / any items spare
#define ITM_2226                    2226 // General items / any items spare
#define ITM_2227                    2227 // General items / any items spare
#define ITM_2228                    2228 // General items / any items spare
#define ITM_2229                    2229 // General items / any items spare
#define ITM_2230                    2230 // General items / any items spare
#define ITM_2231                    2231 // General items / any items spare
#define ITM_2232                    2232 // General items / any items spare
#define ITM_2233                    2233 // General items / any items spare
#define ITM_2234                    2234 // General items / any items spare
#define ITM_2235                    2235 // General items / any items spare
#define ITM_YY_DFLT_OLD             2236
#define ITM_YY_TRACK                2237
#define ITM_2238                    2238 // General items / any items spare
#define ITM_CLMALL                  2239
#define ITM_CLVALL                  2240
#define ITM_DELMALL                 2241
#define ITM_DELVALL                 2242
#define ITM_2243                    2243 // General items / any items spare
#define ITM_2244                    2244 // General items / any items spare
#define ITM_YES                     2245
#define ITM_NO                      2246
#define ITM_RCLVEL_OLD              2247
#define ITM_STOVEL_OLD              2248
#define ITM_RCLELPLUS               2249
#define ITM_STOELPLUS               2250

// 64 more system flags
#define SFL_MONIT                   2251 // This label is used to identify the first in the list of flags
#define SFL_FRCYC                   2252
#define SFL_HPCONV                  2253
#define SFL_NUMLOCK                 2254
#define SFL_CPXMULT                 2255
#define SFL_ERPN                    2256
#define SFL_dLrgLI                  2257
#define SFL_IRFRAC                  2258
#define SFL_IRFRQ                   2259
#define SFL_PFX_ALL                 2260
#define SFL_DREAL                   2261
#define SFL_CPXPLOT                 2262
#define SFL_SHOWX                   2263
#define SFL_SHOWY                   2264
#define SFL_PBOX                    2265
#define SFL_PCROS                   2266
#define SFL_PPLUS                   2267
#define SFL_PLINE                   2268
#define SFL_SCALE                   2269
#define SFL_VECT                    2270
#define SFL_NVECT                   2271
#define SFL_US                      2272
#define SFL_MNUp1                   2273
#define SFL_SBwoy                   2274
#define SFL_TOPHEX                  2275
#define SFL_BCD                     2276
#define SFL_PCURVE                  2277
#define SFL_CLX_DROP                2278
#define SFL_BASE_MYM                2279
#define SFL_G_DOUBLETAP             2280
#define SFL_BASE_HOME               2281
#define SFL_MYM_TRIPLE              2282
#define SFL_HOME_TRIPLE             2283
#define SFL_SHFT_4s                 2284
#define SFL_FGLNLIM                 2285
#define SFL_FGLNFUL                 2286
#define SFL_FGGR                    2287
#define SFL_3DPHYS                  2288
#define SFL_3DXYZ                   2289
#define SFL_PRTEN                   2290
#define SFL_NORM                    2291
#define SFL_BOLD                    2292
#define SFL_SIGZEROS                2293
#define SFL_PRMS                    2294
#define SFL_PINTG                   2295
#define SFL_PDIFF                   2296
#define SFL_PSHADE                  2297
#define SFL_SBadm                   2298
#define SFL_AUTOVALID               2299
#define SFL_M_ALL                   2300
#define SFL_PL_2L                   2301
#define SFL_2302                    2302
#define SFL_2303                    2303
#define SFL_2304                    2304
#define SFL_2305                    2305
#define SFL_2306                    2306
#define SFL_2307                    2307
#define SFL_2308                    2308
#define SFL_2309                    2309
#define SFL_2310                    2310
#define SFL_2311                    2311
#define SFL_2312                    2312
#define SFL_2313                    2313
#define SFL_2314                    2314

#define ITM_2315                    2315 // General items / any items spare

#define ITM_STO_P_p                 2316
#define ITM_STO_N1                  2317
#define ITM_STO_M_x0                2318
#define ITM_STO_S_gamma             2319
#define ITM_STO_M_nu                2320
#define ITM_STO_R_lambda            2321
#define ITM_STO_M_d1                2322
#define ITM_STO_N_d2                2323
#define ITM_STO_M_N                 2324
#define ITM_STO_Q_K                 2325
#define ITM_STO_M_mu                2326
#define ITM_STO_S_sigma             2327
#define ITM_STO_S_s                 2328
#define ITM_STO_Q_alpha             2329
#define ITM_STO_Q_xi                2330
#define ITM_STO_Q_k                 2331
#define ITM_STO_S_lambda            2332
#define ITM_STO_M_a                 2333
#define ITM_STO_N_b                 2334
#define ITM_2335                    2335
#define ITM_REG_M                   2336
#define ITM_REG_N                   2337
#define ITM_REG_P                   2338
#define ITM_REG_Q                   2339
#define ITM_REG_R                   2340
#define ITM_REG_S                   2341
#define ITM_REG_E                   2342
#define ITM_REG_F                   2343
#define ITM_REG_G                   2344
#define ITM_REG_H                   2345
#define ITM_REG_O                   2346
#define ITM_REG_U                   2347
#define ITM_REG_V                   2348
#define ITM_REG_W                   2349
#define VAR_REGM                    2350
#define VAR_REGN                    2351
#define VAR_REGP                    2352
#define VAR_REGQ                    2353
#define VAR_REGR                    2354
#define VAR_REGS                    2355
#define VAR_REGE                    2356
#define VAR_REGF                    2357
#define VAR_REGG                    2358
#define VAR_REGH                    2359
#define VAR_REGO                    2360
#define VAR_REGU                    2361
#define VAR_REGV                    2362
#define VAR_REGW                    2363
#define ITM_M_INSC                  2364
#define ITM_M_DELC                  2365
#define ITM_M_ADDR                  2366
#define ITM_M_ADDC                  2367
#define ITM_REALSLV                 2368
#define ITM_REALSLV_LU              2369
#define ITM_CPXSLV                  2370
#define ITM_CPXSLV_LU               2371
#define ITM_DRAW                    2372
#define ITM_DRAW_LU                 2373
#define ITM_2374                    2374 // General items / any items spare
#define ITM_2375                    2375 // General items / any items spare
#define ITM_2376                    2376 // General items / any items spare
#define ITM_FPHERE                  2377
#define ITM_FPPHERE                 2378

#define VAR_CPERonA                 2379
#define ITM_RCL_CPERonA             2380

#define ITM_2381                    2381 // General items / any items spare
#define ITM_2382                    2382 // General items / any items spare

#define ITM_x_SIGN                  2383
#define ITM_CPXPLOT                 2384

#define ITM_PFACTORSMULT            2385
#define ITM_EULPHI                  2386
#define ITM_SAVEST                  2387
#define ITM_LOADST                  2388
#define ITM_SAVEAUT                 2389
#define ITM_2390                    2390 // General items / any items spare

#define ITM_USER_R47f_g             2391
#define ITM_USER_R47bk_fg           2392
#define ITM_USER_R47fg_bk           2393
#define ITM_USER_R47fg_g            2394
#define ITM_ELLIPSE                 2395

#define ITM_ANGLEQ                  2396
#define ITM_DATEQ                   2397
#define ITM_LINTQ                   2398
#define ITM_NUMBRQ                  2399
#define ITM_SINTQ                   2400
#define ITM_TIMEQ                   2401
#define ITM_TYPEQ                   2402

#define ITM_2403                    2403 // General items / any items spare
#define ITM_EDIT                    2404
#define ITM_OPEN_MENU               2405
#define ITM_2406                    2406 // General items / any items spare
#define ITM_2407                    2407 // General items / any items spare
#define ITM_MENUQ                   2408
#define ITM_SIGMA0                  2409
#define ITM_SIGMA1                  2410
#define ITM_SIGMAk                  2411
#define ITM_SIGMAp1                 2412
#define ITM_SIGMApk                 2413
#define ITM_2414                    2414 // General items / any items spare
#define ITM_INDIRECT_X              2415
#define ITM_INDIRECT_Y              2416
#define ITM_INDIRECT_Z              2417
#define ITM_INDIRECT_T              2418
#define ITM_XSWAP                   2419
#define ITM_XEDIT                   2420

//more bufferized
#define ITM_SUP_BOLD_T              2421
#define ITM_SUP_BOLD_f              2422
#define ITM_SUP_BOLD_g              2423
#define ITM_SUP_BOLD_h              2424
#define ITM_SUP_BOLD_r              2425
#define ITM_SUP_BOLD_x              2426
#define ITM_HOLLOW_UP_ARROW         2427
#define ITM_HOLLOW_DOWN_ARROW       2428
#define ITM_SPHERICAL_ANGLE         2429
#define ITM_TRIPLE_INTEGRAL         2430
#define ITM_VOLUME_INTEGRAL         2431
#define ITM_LEFT_BLOCKARROW         2432
#define ITM_UP_BLOCKARROW           2433
#define ITM_RIGHT_BLOCKARROW        2434
#define ITM_DOWN_BLOCKARROW         2435
#define ITM_POWER_SYMBOL            2436
#define ITM_u_BAR                   2437
#define ITM_v_BAR                   2438
#define ITM_w_BAR                   2439
#define ITM_z_BAR                   2440
#define ITM_v_CIRC                  2441
#define ITM_z_CIRC                  2442
#define ITM_u_CIRC2                 2443
#define ITM_theta_m                 2444
#define ITM_j_CIRC                  2445
#define ITM_k_CIRC                  2446
#define ITM_phi_m                   2447
#define ITM_SUP_SLASH               2448
#define ITM_SUP_DOT                 2449
#define ITM_SUP_COMMA               2450
#define ITM_2451                    2451
#define ITM_2452                    2452
#define ITM_2453                    2453
#define ITM_2454                    2454
#define ITM_2455                    2455
#define ITM_2456                    2456
#define ITM_2457                    2457
#define ITM_2458                    2458
#define ITM_2459                    2459
#define ITM_2460                    2460
#define ITM_2461                    2461
#define ITM_2462                    2462
#define ITM_2463                    2463
//continue after this block

#define ITM_EVtoJ                   2464
#define ITM_JtoEV                   2465
#define ITM_BANANAtoINCH            2466
#define ITM_INCHtoBANANA            2467
#define ITM_BANANAtoMM              2468
#define ITM_MMtoBANANA              2469

#define ITM_V3toCYL                 2470
#define ITM_V3toSPH                 2471
#define ITM_VVDIST                  2472
#define ITM_M_CONCAT_OLD            2473

#define ITM_STKtoV3_M               2474
#define ITM_STKtoV2                 2475
#define ITM_V3toSTK_M               2476
#define ITM_V2toSTK                 2477
#define ITM_M_CONCATB               2478
#define ITM_V100                    2479
#define ITM_V010                    2480
#define ITM_V001                    2481

#define ITM_RCLVEL1                 2482
#define ITM_RCLVEL2                 2483
#define ITM_RCLVEL3                 2484
#define ITM_STOVEL1                 2485
#define ITM_STOVEL2                 2486
#define ITM_STOVEL3                 2487
#define ITM_STKtoV3_P               2488
#define ITM_V3toSTK_P               2489
#define ITM_V10                     2490
#define ITM_V01                     2491
#define ITM_CPXexV                  2492

#define ITM_CPXtoV                  2493
#define ITM_VtoCPX                  2494
#define ITM_stkexV2                 2495
#define ITM_stkexV3                 2496
#define ITM_REGtoVEC                2497
#define ITM_VECtoREG                2498
#define ITM_2499                    2499 // General items / any items spare

#define ITM_CLKp2                   2500
#define ITM_WOY_ISO                 2501
#define ITM_WOY_US                  2502
#define ITM_WOY_ME                  2503 // Middle East
#define ITM_GET_WOY                 2504
#define ITM_WOY                     2505


#define ITM_RIBBON_CPX              2506
#define ITM_RIBBON_FIN              2507
#define ITM_RIBBON_SAV              2508
#define ITM_RIBBON_C47              2509
#define ITM_RIBBON_C47PL            2510
#define ITM_RIBBON_R47              2511
#define ITM_RIBBON_R47PL            2512
#define ITM_RIBBON_ENG              2513
#define ITM_RIBBON_FIN2             2514
#define ITM_RIBBON_SAV2             2515
#define ITM_2516                    2516
#define ITM_2517                    2517
#define ITM_2518                    2518
#define ITM_2519                    2519
#define ITM_2520                    2520
#define ITM_2521                    2521
#define ITM_2522                    2522
#define ITM_2523                    2523

#define ITM_REALMATQ                2524
#define ITM_COMPLEXMATQ             2525
#define ITM_CONFIGQ                 2526
#define ITM_ISREZQ                  2527
#define ITM_ISIMZQ                  2528
#define ITM_ISRENZQ                 2529
#define ITM_ISIMNZQ                 2530
#define ITM_ISVECT2DQ               2531
#define ITM_ISVECT3DQ               2532
#define ITM_SI_Ki                   2533
#define ITM_SI_Mi                   2534
#define ITM_SI_Gi                   2535
#define ITM_SI_Ti                   2536
#define ITM_SI_Pi                   2537
#define ITM_ALPHATRIM               2538
#define ITM_ALPHAREV                2539
#define ITM_ALPHAMID                2540
#define ITM_ALPHALEFT               2541
#define ITM_ALPHARIGHT              2542
#define ITM_ALPHALOWER              2543
#define ITM_ALPHAUPPER              2544
#define VAR_UEST                    2545
#define VAR_LEST                    2546
#define VAR_UY                      2547
#define VAR_LY                      2548

#define ITM_RNG                     2549
#define ITM_YY_DFLT                 2550
#define ITM_DENMAX2                 2551
#define ITM_2552                    2552 // General items / any items spare

#define ITM_BITSp2                  2553
#define ITM_DEG2_XFN                2554
#define ITM_RAD2_XFN                2555
#define ITM_RDP_XFN                 2556

#define ITM_RSD_XFN                 2557
#define ITM_sin_XFN                 2558
#define ITM_cos_XFN                 2559
#define ITM_tan_XFN                 2560
#define ITM_pi_XFN                  2561
#define ITM_1ONX_XFN                2562
#define ITM_atan2_XFN               2563
#define ITM_arcsin_XFN              2564
#define ITM_arccos_XFN              2565
#define ITM_arctan_XFN              2566
#define ITM_LN_XFN                  2567
#define ITM_LOG_XFN                 2568
#define ITM_EXP_XFN                 2569
#define ITM_10X_XFN                 2570
#define ITM_POWER_XFN               2571
#define ITM_SQRT_XFN                2572
#define ITM_ADD_XFN                 2573
#define ITM_SUB_XFN                 2574
#define ITM_MULT_XFN                2575
#define ITM_DIV_XFN                 2576
#define ITM_MOD_XFN                 2577
#define ITM_MODANG_XFN              2578
#define ITM_TO_XFN                  2579
#define ITM_CHS_XFN                 2580
#define ITM_2581                    2581
#define ITM_DRG_XFN                 2582
#define ITM_SQR_XFN                 2583
#define ITM_XTHROOT_XFN             2584

#define ITM_3DUP                    2585
#define ITM_3SWAP                   2586
#define ITM_3DROP                   2587
#define ITM_3STO                    2588
#define ITM_3RCL                    2589

#define ITM_NDUP_OLD                2590
#define ITM_NSWAP_OLD               2591
#define ITM_NDROP_OLD               2592
#define ITM_2STO                    2593
#define ITM_2RCL                    2594
#define ITM_4SWAP                   2595


#define ITM_2596                    2596 // General items / any items spare
#define ITM_2597                    2597 // General items / any items spare
#define ITM_MtoTH                   2598
#define ITM_THtoM                   2599

#define ITM_2600                    2600 // General items / any items spare
#define ITM_UNIFORMP                2601
#define ITM_UNIFORML                2602
#define ITM_UNIFORMU                2603
#define ITM_UNIFORMI                2604
#define ITM_2605                    2605 // General items / any items spare
#define ITM_DISUNIFORMP             2606
#define ITM_DISUNIFORML             2607
#define ITM_DISUNIFORMU             2608
#define ITM_DISUNIFORMI             2609
#define ITM_2610                    2610
#define ITM_2611                    2611
#define ITM_2612                    2612
#define ITM_2613                    2613
#define ITM_2614                    2614
#define ITM_2615                    2615
#define ITM_2616                    2616
#define ITM_2617                    2617
#define ITM_2618                    2618
#define ITM_2619                    2619


// Before adding more, leave a gap in case we add additional distributions
// Each distribution consumes five items

#define ITM_NDUP                    2620
#define ITM_NSWAP                   2621
#define ITM_NDROP                   2622
#define ITM_MATX_A_1                2623
#define ITM_3R3P                    2624

#define ITM_2625                    2625 // General items / any items spare
#define ITM_SetHP35                 2626
#define ITM_SetC47                  2627
#define ITM_SetJM                   2628
#define ITM_SetRJ                   2629
#define ITM_2630                    2630
#define ITM_2631                    2631
#define ITM_2632                    2632
#define ITM_2633                    2633
#define ITM_2634                    2634
#define ITM_2635                    2635
#define ITM_2636                    2636
#define ITM_2637                    2637
#define ITM_2638                    2638 // General items / any items spare
#define ITM_STORCL_IP               2639
#define ITM_STORCL_NPPER            2640
#define ITM_STORCL_IPonA            2641
#define ITM_STORCL_PV               2642
#define ITM_STORCL_FV               2643
#define ITM_STORCL_PMT              2644
#define ITM_STORCL_PPERonA          2645
#define ITM_STORCL_CPERonA          2646
#define ITM_dddVEL1                 2647
#define ITM_dddVEL2                 2648
#define ITM_dddVEL3                 2649
#define ITM_dddVEL                  2650
#define ITM_dddIX                   2651
#define ITM_dddEL                   2652
#define ITM_dddIJ                   2653
#define ITM_Max                     2654
#define ITM_Min                     2655
#define ITM_Config                  2656
#define ITM_Stack                   2657

#define ITM_ERGtoJ                  2658
#define ITM_JtoERG                  2659
#define ITM_FoetoJ                  2660
#define ITM_JtoFoe                  2661

#define ITM_LR_A0                   2662
#define ITM_LR_A1                   2663
#define ITM_LR_A2                   2664

#define ITM_CtoK                    2665
#define ITM_KtoC                    2666
#define ITM_RAtoK                   2667
#define ITM_KtoRA                   2668
#define ITM_RAtoF                   2669
#define ITM_FtoRA                   2670
#define ITM_EVKBtoK                 2671
#define ITM_KtoEVKB                 2672
#define ITM_FtoK                    2673
#define ITM_KtoF                    2674
#define ITM_2675                    2675
#define ITM_2676                    2676
#define ITM_2677                    2677
#define ITM_2678                    2678
#define ITM_2679                    2679
#define ITM_2680                    2680 //spares for the temp menu

#define ITM_2681                    2681 // General items / any items spare
#define ITM_PRINTERALPHA            2682
#define ITM_PRINTERHP               2683
#define ITM_PRINTERMARTEL           2684
#define ITM_PRINTEROTHER            2685
#define ITM_PRINTERDLAYQ            2686
#define ITM_PRINTERON               2687
#define ITM_PRINTEROFF              2688
#define ITM_MAN                     2689
#define ITM_NORM                    2690
#define ITM_TRACE                   2691
#define ITM_STRACE                  2692
#define ITM_PRINTERLIST             2693
#define ITM_PRINTERXFN              2694
#define ITM_PRINTERCHAR             2695
#define ITM_PRINTERTAB              2696
#define ITM_PRINTERHASH             2697
#define ITM_2698                    2698
#define ITM_PRINT_ALL_ITEMS         2699 //spares for the print menu

#define ITM_3DPHYS                  2700
#define ITM_3DXYZ                   2701
#define ITM_STKtoV3                 2702
#define ITM_V3toSTK                 2703

#define ITM_PNORM                   2704
#define ITM_NNZ                     2705
#define ITM_CNORM                   2706
#define ITM_2707                    2707
#define ITM_2708                    2708
#define ITM_2709                    2709
#define ITM_CSUM                    2710
#define ITM_2711                    2711 // General items / any items spare
#define ITM_M_CC                    2712
#define ITM_M_CMAX                  2713
#define ITM_M_CMIN                  2714
#define ITM_M_FIND                  2715

#define ITM_AMORT_P1                2716
#define ITM_AMORT_P2                2717
#define ITM_AMORT_INT               2718
#define ITM_AMORT_PRN               2719
#define ITM_AMORT_BAL               2720
#define ITM_AMORT_NXT               2721
#define ITM_2722                    2722
#define ITM_2723                    2723
#define ITM_2724                    2724
#define ITM_2725                    2725 //spares for TVM/AMORT

#define ITM_MIDENT                  2726
#define ITM_M_SQRT                  2727
#define ITM_RCLVEL                  2728
#define ITM_STOVEL                  2729
#define ITM_M_ALL                   2730
#define ITM_PL_2L                   2731
#define ITM_PGMPLT                  2732
#define ITM_2733                    2733 // General items / any items spare
#define ITM_PLTf                    2734
#define ITM_IMPLOT                  2735
#define ITM_2736                    2736 // General items / any items spare
#define ITM_M_DIMNQ                 2737
#define ITM_2738                    2738 // General items / any items spare
#define ITM_M_CONCATR               2739
#define ITM_2740                    2740
#define ITM_GET_GRAMOD              2741
#define ITM_SET_GRAMOD              2742
#define ITM_KNOTtoMPS               2743
#define ITM_MPStoKNOT               2744
#define ITM_DEGPStoRADPS            2745
#define ITM_RADPStoDEGPS            2746
#define ITM_SLUGtoKG                2747
#define ITM_KGtoSLUG                2748
#define ITM_SLINCHtoKG              2749
#define ITM_KGtoSLINCH              2750
#define ITM_BLOBtoKG                2751
#define ITM_KGtoBLOB                2752
#define ITM_TONNEtoKG               2753
#define ITM_KGtoTONNE               2754
#define ITM_SIGMAnINF               2755
#define ITM_2756                    2756 /*reserved export d47 data*/
#define ITM_XEXPORT                 2757
#define ITM_EXPATOW                 2758
#define ITM_EXPSTK                  2759
#define ITM_EXPNREG                 2760
#define ITM_EXPX                    2761
#define ITM_IMPREGS                 2762

#define ITM_GET_ADM                 2763 // VAR_NO_ADM
#define ITM_SET_ADM                 2764 // VAR_NO_ADM
//efine ITM_GET_ISM                 1606 // VAR_NO_ISM
#define ITM_SET_ISM                 2765 // VAR_NO_ISM
#define ITM_GET_REALDF              2766 // VAR_NO_REALDF
#define ITM_SET_REALDF              2767 // VAR_NO_REALDF
#define ITM_GET_NDEC                2768 // VAR_NO_NDEC
#define ITM_SET_NDEC                2769 // VAR_NO_NDEC
//efine ITM_GET_DMX                 1558 // VAR_NO_DENMAX
#define ITM_SET_DMX                 2770 // VAR_NO_DENMAX

#define ITM_CLD                     2771
#define ITM_CLLCD                   2772
#define ITM_CLALPHA                 2773
#define ITM_ALPHAIP                 2774
#define ITM_42STRING                2775
#define ITM_42APPEND                2776
#define ITM_42ARCL                  2777
#define ITM_42ASTO                  2778
#define ITM_42AROT                  2779
#define ITM_42ASHF                  2780
#define ITM_42ROTXY                 2781
#define ITM_42BITQ                  2782
#define ITM_2783                    2783 // 42→DEC
#define ITM_2784                    2784 // 42→OCT
#define ITM_XtoALPHA                2785
#define ITM_42AIP                   2786
#define ITM_42ALENG                 2787
#define ITM_42ATOX                  2788
#define ITM_42AVIEW                 2789
#define ITM_42CLA                   2790
#define ITM_42POSA                  2791
#define ITM_42PRA                   2792
#define ITM_42XTOA                  2793
#define ITM_42KEY                   2794
#define ITM_42KEYG                  2795
#define ITM_42KEYX                  2796
#define ITM_42VRMNU                 2797
#define ITM_42PROMPT                2798
#define ITM_2799                    2799 //spares for 42s compatibility functions

#define ITM_LBSFT2toPA              2800
#define ITM_PAtoLBSFT2              2801
#define ITM_INLBStoNM               2802
#define ITM_NMtoINLBS               2803
#define ITM_LBSFTtoNPM              2804
#define ITM_NPMtoLBSFT              2805
#define ITM_KGFtoN                  2806
#define ITM_NtoKGF                  2807
#define ITM_MS2toFTS2               2808
#define ITM_FTS2toMS2               2809
#define ITM_MS2toINS2               2810
#define ITM_INS2toMS2               2811
#define ITM_KSItoMPA                2812
#define ITM_MPAtoKSI                2813
#define ITM_LBSIN3toBLOBIN3         2814
#define ITM_BLOBIN3toLBSIN3         2815
#define ITM_LBSIN3toTMM3            2816
#define ITM_TMM3toLBSIN3            2817
#define ITM_LBSIN3toKGM3            2818
#define ITM_KGM3toLBSIN3            2819
#define ITM_KGM3toBLOBIN3           2820
#define ITM_BLOBIN3toKGM3           2821
#define ITM_KGM3toTMM3              2822
#define ITM_TMM3toKGM3              2823
#define ITM_LBFTtoKGM               2824
#define ITM_KGMtoLBFT               2825
#define ITM_IN3toMM3                2826
#define ITM_MM3toIN3                2827
#define ITM_IN2toMM2                2828
#define ITM_MM2toIN2                2829
#define ITM_IN4toMM4                2830
#define ITM_MM4toIN4                2831
#define ITM_IN6toMM6                2832
#define ITM_MM6toIN6                2833

#define ITM_KGFPMtoNPM              2834
#define ITM_NPMtoKGFPM              2835
#define ITM_KGFtoLBF                2836
#define ITM_LBFtoKGF                2837
#define ITM_UNSLUGtoKG              2838
#define ITM_KGtoUNSLUG              2839
#define ITM_UNSLINCHtoKG            2840
#define ITM_KGtoUNSLINCH            2841
#define ITM_2842                    2842 // CONV MENU Spares
#define ITM_2843                    2843 // CONV MENU Spares

#define ITM_EXPFLN                  2844
#define ITM_EXPLFLN                 2845
#define ITM_EXPSFL                  2846
#define ITM_2847                    2847 // General items / any items spare
#define ITM_2848                    2848 // General items / any items spare
#define ITM_2849                    2849 // General items / any items spare

#define ITM_XLEM0                   2850
#define ITM_XGEP0                   2851
#define ITM_PLTFCNS                 2852
#define ITM_2853                    2853 // General items / any items spare
#define ITM_2854                    2854 // General items / any items spare
#define ITM_2855                    2855 // General items / any items spare
#define ITM_2856                    2856 // General items / any items spare
#define ITM_2857                    2857 // General items / any items spare
#define ITM_2858                    2858 // General items / any items spare
#define ITM_2859                    2859 // General items / any items spare

#define ITM_MPHtoKNOT               2860
#define ITM_KNOTtoMPH               2861
#define ITM_MPHtoFPS                2862
#define ITM_FPStoMPH                2863
#define ITM_MGEUStoK100K            2864
#define ITM_K100KtoMGEUS            2865
#define ITM_MGEUKtoK100K            2866
#define ITM_K100KtoMGEUK            2867
#define ITM_MIKtoK100K              2868
#define ITM_K100KtoMIK              2869

#define ITM_DEGtoMIL                2870
#define ITM_MILtoDEG                2871
#define ITM_DEGtoARCSEC             2872
#define ITM_ARCSECtoDEG             2873
#define ITM_DEGtoMRAD               2874
#define ITM_MRADtoDEG               2875

//==============================================================================
         // CONV step 4/6 of ADDING to CONV    [grep for "add conversion items to CONV menu"]
         // =================================================================================
         // Rename or add two adjacent CONV items/spares below into the new pair's ITM_<X>to<Y> / ITM_<Y>to<X> defines
         // Increment LAST_ITEM only when the spares are exhausted.
         // It is best to only add CONV items incrementally, without re-using old available slots, to keep the tables sorted incrementally
         // Next Step: CONV step 5/6 in src/c47/items.c.
//==============================================================================
#define ITM_2876                    2876 // General items / any items spare (reserved: Units, kPa→Pa)
#define ITM_2877                    2877 // General items / any items spare (reserved: Units, Pa→kPa)
#define ITM_2878                    2878 // General items / any items spare (reserved: Units, kWh→J)
#define ITM_2879                    2879 // General items / any items spare (reserved: Units, J→kWh)
#define ITM_MWtoDBM                 2880
#define ITM_DBMtoMW                 2881
#define ITM_PGMDRV                  2882
#define ITM_F1DRV                   2883
#define ITM_F2DRV                   2884
#define ITM_WtoDBW                  2885
#define ITM_DBWtoW                  2886
#define ITM_VtoDBV                  2887
#define ITM_DBVtoV                  2888
#define ITM_2889                    2889 // General items / any items spare (CONV spare)
#define ITM_2890                    2890 // General items / any items spare (CONV spare)
#define ITM_2891                    2891 // General items / any items spare (CONV spare)
#define ITM_2892                    2892 // General items / any items spare (CONV spare)
#define ITM_2893                    2893 // General items / any items spare (CONV spare)
#define ITM_2894                    2894 // General items / any items spare (CONV spare)
#define ITM_2895                    2895 // General items / any items spare (CONV spare)
#define ITM_2896                    2896 // General items / any items spare (CONV spare)
#define ITM_2897                    2897 // General items / any items spare (CONV spare)
#define ITM_2898                    2898 // General items / any items spare (CONV spare)
#define ITM_2899                    2899 // General items / any items spare (CONV spare)
#define ITM_2900                    2900 // General items / any items spare (CONV spare)
#define ITM_2901                    2901 // General items / any items spare (CONV spare)
#define ITM_2902                    2902 // General items / any items spare (CONV spare)
#define ITM_2903                    2903 // General items / any items spare (CONV spare)
#define ITM_2904                    2904 // General items / any items spare (CONV spare)
#define ITM_2905                    2905 // General items / any items spare (CONV spare)
#define ITM_2906                    2906 // General items / any items spare (CONV spare)
#define ITM_2907                    2907 // General items / any items spare (CONV spare)
#define ITM_2908                    2908 // General items / any items spare (CONV spare)
#define ITM_2909                    2909 // General items / any items spare (CONV spare)
#define ITM_2910                    2910 // General items / any items spare (CONV spare)
#define ITM_2911                    2911 // General items / any items spare (CONV spare)
#define ITM_2912                    2912 // General items / any items spare (CONV spare)
#define ITM_2913                    2913 // General items / any items spare (CONV spare)
#define ITM_2914                    2914 // General items / any items spare (CONV spare)
#define ITM_2915                    2915 // General items / any items spare (CONV spare)
#define ITM_2916                    2916 // General items / any items spare (CONV spare)
#define ITM_2917                    2917 // General items / any items spare (CONV spare)
#define ITM_2918                    2918 // General items / any items spare (CONV spare)
#define ITM_2919                    2919 // General items / any items spare (CONV spare)

#define ITM_IF                      2920 // STRUCT programming
#define ITM_ELSE                    2921 // STRUCT programming
#define ITM_ENDIF                   2922 // STRUCT programming
#define MNU_STRUCTPGM               2923 // STRUCT programming
#define ITM_DO                      2924 // STRUCT programming
#define ITM_WHILE                   2925 // STRUCT programming
#define ITM_ENDDO                   2926 // STRUCT programming
#define ITM_FOR                     2927 // STRUCT programming
#define ITM_NEXT                    2928 // STRUCT programming
#define ITM_FORx                    2929 // STRUCT programming, the form of FOR that VALID has not passed
#define ITM_NEXTx                   2930 // STRUCT programming, the form of NEXT that VALID has not passed
#define ITM_VALID                   2931 // STRUCT programming
#define ITM_AUTOVALID               2932 // STRUCT programming
#define ITM_FORYX                   2933 // STRUCT programming, the FOR that is given a start and an end and works out its own step
#define ITM_FORYXx                  2934 // STRUCT programming, the form of FORyx that VALID has not passed
#define ITM_REPEAT                  2935 // STRUCT programming, the loop that tests at the end
#define ITM_UNTIL                   2936 // STRUCT programming, its test and closer
#define ITM_CLSTRUC                 2937 // STRUCT programming
#define ITM_FORTOP                  2938 // STRUCT programming, the FOR that compares before the first pass
#define ITM_FORTOPx                 2939 // STRUCT programming, the form of FORTOP that VALID has not passed
#define ITM_2940                    2940 // General items / any items spare (reserved: STRUCT, spare)
#define ITM_2941                    2941 // General items / any items spare (reserved: STRUCT, spare)
#define ITM_2942                    2942 // General items / any items spare (reserved: STRUCT, spare)
#define ITM_2943                    2943 // General items / any items spare (reserved: STRUCT, spare)
#define ITM_2944                    2944 // General items / any items spare (reserved: STRUCT, spare)

#define MNU_TAM                     2945
#define MNU_TAMALPHA                2946
#define MNU_TAMCMP                  2947
#define MNU_TAMFLAG                 2948
#define MNU_TAMINDIRECT             2949
#define MNU_TAMLABEL                2950
#define MNU_TAMLBLONLY              2951
#define MNU_TAMLOCALLABEL           2952
#define MNU_TAMMENU                 2953
#define MNU_TAMNONREG               2954
#define MNU_TAMNONREGMAX            2955
#define MNU_TAMNONREGTRK            2956
#define MNU_TAMNORM                 2957
#define MNU_TAMRCL                  2958
#define MNU_TAMRCL_TVM              2959
#define MNU_TAMSHUFFLE              2960
#define MNU_TAMSTO                  2961
#define MNU_TAMSTO_TVM              2962
#define MNU_TAMVARONLY              2963
#define ITM_2964                    2964 // General items / any items spare (reserved: FIN12C, TAMSTO menu)
#define ITM_2965                    2965 // General items / any items spare (reserved: FIN12C, TAMRCL menu)
#define ITM_2966                    2966 // General items / any items spare (reserved: Tam Menus)
#define ITM_2967                    2967 // General items / any items spare (reserved: Tam Menus)
#define ITM_2968                    2968 // General items / any items spare (reserved: Tam Menus)
#define ITM_2969                    2969 // General items / any items spare (reserved: Tam Menus)
#define ITM_2970                    2970 // General items / any items spare (reserved: Tam Menus)
#define ITM_2971                    2971 // General items / any items spare (reserved: Tam Menus)
#define ITM_2972                    2972 // General items / any items spare (reserved: Tam Menus)
#define ITM_2973                    2973 // General items / any items spare (reserved: Tam Menus)

// Distribution menus. One run, so that showingProbMenu is a single range test.
#define MNU_BINOM                   2974
#define MNU_CAUCH                   2975
#define MNU_CHI2                    2976
#define MNU_DISTR                   2977
#define MNU_DISUNIFORM              2978
#define MNU_EXPON                   2979
#define MNU_F                       2980
#define MNU_GEOM                    2981
#define MNU_GEV                     2982
#define MNU_HYPER                   2983
#define MNU_LOGIS                   2984
#define MNU_NORML                   2985
#define MNU_PARETO                  2986
#define MNU_POISS                   2987
#define MNU_STDNORML                2988
#define MNU_T                       2989
#define MNU_UNIFORM                 2990
#define MNU_WEIBL                   2991
#define ITM_2992                    2992 // General items / any items spare (reserved: Distribution menus)
#define ITM_2993                    2993 // General items / any items spare (reserved: Distribution menus)
#define ITM_2994                    2994 // General items / any items spare (reserved: Distribution menus)
#define ITM_2995                    2995 // General items / any items spare (reserved: Distribution menus)
#define ITM_2996                    2996 // General items / any items spare (reserved: Distribution menus)

#define PROBMENUSTART  MNU_BINOM
#define PROBMENUEND    ITM_2996

#define MNU_1STDERIV                2997
#define MNU_2NDDERIV                2998
#define MNU_42                      2999
#define MNU_ADV                     3000
#define MNU_AIMCATALOG              3001
#define MNU_ALLVARS                 3002
#define MNU_ALPHA                   3003
#define MNU_ALPHA_OMEGA             3004
#define MNU_alpha_omega             3005
#define MNU_ALPHAFN                 3006
#define MNU_ALPHAINTL               3007
#define MNU_ALPHAintl               3008
#define MNU_ALPHAMATH               3009
#define MNU_ALPHAMISC               3010
#define MNU_AMORT                   3011
#define MNU_ANGLECONV_43S           3012
#define MNU_ANGLES                  3013
#define MNU_ASN_N                   3014
#define MNU_AUDIO                   3015
#define MNU_BASE                    3016
#define MNU_BASE2                   3017
#define MNU_BITS                    3018
#define MNU_BITSET                  3019
#define MNU_BLUE_C47                3020
#define MNU_CASHFL                  3021
#define MNU_CATALOG                 3022
#define MNU_CHARS                   3023
#define MNU_CLK                     3024
#define MNU_CLR                     3025
#define MNU_CONFIGS                 3026
#define MNU_CONST                   3027
#define MNU_CONV_F_LOAD             3028
#define MNU_CONV_M_LOAD             3029
#define MNU_CONV_MATERL             3030
#define MNU_CONV_P_LOAD             3031
#define MNU_CONV_SECTION            3032
#define MNU_CONVA                   3033
#define MNU_CONVANG                 3034
#define MNU_CONVCHEF                3035
#define MNU_CONVE                   3036
#define MNU_CONVFP                  3037
#define MNU_CONVHUM                 3038
#define MNU_CONVM                   3039
#define MNU_CONVP                   3040
#define MNU_CONVS                   3041
#define MNU_CONVTEMP                3042
#define MNU_CONVV                   3043
#define MNU_CONVX                   3044
#define MNU_CONVYMMV                3045
#define MNU_CPX                     3046
#define MNU_CPXS                    3047
#define MNU_DATES                   3048
#define MNU_DELETE                  3049
#define MNU_DEV                     3050
#define MNU_DISP                    3051
#define MNU_DYNAMIC                 3052
#define MNU_EE                      3053
#define MNU_EIMCATALOG              3054
#define MNU_ELLIPT                  3055
#define MNU_EQ_EDIT                 3056
#define MNU_EQN                     3057
#define MNU_EXP                     3058
#define MNU_FCNS                    3059
#define MNU_FCNS_EIM                3060
#define MNU_FIN                     3061
#define MNU_FLAGS                   3062
#define MNU_FLG                     3063
#define MNU_GAP_L                   3064
#define MNU_GAP_R                   3065
#define MNU_GAP_RX                  3066
#define MNU_Grapher                 3067
#define MNU_GRAPHS                  3068
#define MNU_HIST                    3069
#define MNU_HOME                    3070
#define MNU_HPLOT                   3071
#define MNU_INFO                    3072
#define MNU_INL_TST                 3073
#define MNU_INTS                    3074
#define MNU_IO                      3075
#define MNU_KEYS                    3076
#define MNU_LINTS                   3077
#define MNU_LOOP                    3078
#define MNU_M_EDIT                  3079
#define MNU_MATRS                   3080
#define MNU_MATX                    3081
#define MNU_MENU                    3082
#define MNU_MENUS                   3083
#define MNU_MISC                    3084
#define MNU_MODE                    3085
#define MNU_MODEL                   3086
#define MNU_MULTSTK                 3087
#define MNU_MVAR                    3088
#define MNU_MyAlpha                 3089
#define MNU_MyMenu                  3090
#define MNU_NUMBRS                  3091
#define MNU_NUMTHEORY               3092
#define MNU_ORTHOG                  3093
#define MNU_PARTS                   3094
#define MNU_PFN                     3095
#define MNU_PFN_1                   3096
#define MNU_PFN_2                   3097
#define MNU_PFN_3                   3098
#define MNU_PLOT_ASSESS             3099
#define MNU_PLOT_FUNC               3100
#define MNU_PLOT_SCATR              3101
#define MNU_PLOT_STAT               3102
#define MNU_PLOTTING                3103
#define MNU_PREF                    3104
#define MNU_PREFIX                  3105
#define MNU_PRINT                   3106
#define MNU_PRINTER                 3107
#define MNU_PROB                    3108
#define MNU_PROG                    3109
#define MNU_PROGS                   3110
#define MNU_REALS                   3111
#define MNU_REG                     3112
#define MNU_REGR                    3113
#define MNU_RESETS                  3114
#define MNU_RIBBONS                 3115
#define MNU_Sf                      3116
#define MNU_Sf_TOOL                 3117
#define MNU_Sfdx                    3118
#define MNU_SHOW                    3119
#define MNU_SIMQ                    3120
#define MNU_SINTS                   3121
#define MNU_Solver                  3122
#define MNU_Solver_TOOL             3123
#define MNU_STAT                    3124
#define MNU_STK                     3125
#define MNU_STRINGS                 3126
#define MNU_STRUCT                  3127
#define MNU_SYSFL                   3128
#define MNU_TEST                    3129
#define MNU_TIMERF                  3130
#define MNU_TIMES                   3131
#define MNU_TRG_C47                 3132
#define MNU_TRG_C47_MORE            3133
#define MNU_TRG_R47                 3134
#define MNU_TRI                     3135
#define MNU_TVM                     3136
#define MNU_UNITCONV                3137
#define MNU_VAR                     3138
#define MNU_VARS                    3139
#define MNU_VECCONV                 3140
#define MNU_VECT                    3141
#define MNU_XFN                     3142
#define MNU_XXFCNS                  3143
#define MNU_YESNO                   3144
#define MNU_POLY                    3145
#define MNU_USRMENU                 3146
#define MNU_USRMENUS                3147
#define ITM_3148                    3148 // General items / any items spare (reserved: FIN12C, VARFIN menu)
#define ITM_3149                    3149 // General items / any items spare (reserved: FIN12C, dynamic softmenu spare)
#define ITM_3150                    3150 // General items / any items spare (reserved: FIN12C, dynamic softmenu spare)
#define ITM_3151                    3151 // General items / any items spare (reserved: FIN12C, dynamic softmenu spare)
#define MNU_RMODE                   3152
#define ITM_3153                    3153 // General items / any items spare (reserved: Menus)
#define ITM_3154                    3154 // General items / any items spare (reserved: Menus)
#define ITM_3155                    3155 // General items / any items spare (reserved: Menus)
#define MNU_DRM                     3156
#define MNU_RNDMODES                3157
#define ITM_3158                    3158 // General items / any items spare (reserved: Menus)
#define ITM_3159                    3159 // General items / any items spare (reserved: Menus)
#define ITM_3160                    3160 // General items / any items spare (reserved: Menus)
#define ITM_3161                    3161 // General items / any items spare (reserved: Menus)
#define ITM_3162                    3162 // General items / any items spare (reserved: Menus)
#define ITM_3163                    3163 // General items / any items spare (reserved: Menus)
#define ITM_3164                    3164 // General items / any items spare (reserved: Menus)
#define ITM_3165                    3165 // General items / any items spare (reserved: Menus)
#define ITM_3166                    3166 // General items / any items spare (reserved: Menus)
#define ITM_3167                    3167 // General items / any items spare (reserved: Menus)
#define ITM_3168                    3168 // General items / any items spare (reserved: Menus)
#define ITM_3169                    3169 // General items / any items spare (reserved: Menus)
#define ITM_3170                    3170 // General items / any items spare (reserved: Menus)
#define ITM_3171                    3171 // General items / any items spare (reserved: Menus)
#define ITM_3172                    3172 // General items / any items spare (reserved: Menus)
#define ITM_3173                    3173 // General items / any items spare (reserved: Menus)
#define ITM_3174                    3174 // General items / any items spare (reserved: Menus)
#define ITM_3175                    3175 // General items / any items spare (reserved: Menus)
#define ITM_3176                    3176 // General items / any items spare (reserved: Menus)
#define ITM_3177                    3177 // General items / any items spare (reserved: Menus)
#define ITM_3178                    3178 // General items / any items spare (reserved: Menus)
#define ITM_3179                    3179 // General items / any items spare (reserved: Menus)
#define ITM_3180                    3180 // General items / any items spare (reserved: Menus)

#define ITM_3181                    3181 // General items / any items spare (reserved: Units, →SI)
#define ITM_3182                    3182 // General items / any items spare (reserved: Units, SI→)
#define ITM_3183                    3183 // General items / any items spare (reserved: Units, UNIT.OP)
#define ITM_3184                    3184 // General items / any items spare (reserved: Units, UBASE)
#define ITM_3185                    3185 // General items / any items spare (reserved: Units, UVAL)
#define ITM_3186                    3186 // General items / any items spare (reserved: Units, CONVERT)
#define ITM_3187                    3187 // General items / any items spare (reserved: Units, →UNIT)
#define ITM_3188                    3188 // General items / any items spare (reserved: Units, UFACT)
#define ITM_3189                    3189 // General items / any items spare (reserved: Units, UNIT.PFX)
#define ITM_3190                    3190 // General items / any items spare (reserved: Units, UEXPR)
#define ITM_3191                    3191 // General items / any items spare (reserved: Units, UNITS)
#define ITM_3192                    3192 // General items / any items spare (reserved: Units, U:LENG)
#define ITM_3193                    3193 // General items / any items spare (reserved: Units, U:AREA)
#define ITM_3194                    3194 // General items / any items spare (reserved: Units, U:VOL)
#define ITM_3195                    3195 // General items / any items spare (reserved: Units, U:MASS)
#define ITM_3196                    3196 // General items / any items spare (reserved: Units, U:TIME)
#define ITM_3197                    3197 // General items / any items spare (reserved: Units, U:TEMP)
#define ITM_3198                    3198 // General items / any items spare (reserved: Units, U:PRES)
#define ITM_3199                    3199 // General items / any items spare (reserved: Units, U:FORCE)
#define ITM_3200                    3200 // General items / any items spare (reserved: Units, U:ENRG)
#define ITM_3201                    3201 // General items / any items spare (reserved: Units, U:POWR)
#define ITM_3202                    3202 // General items / any items spare (reserved: Units, U:SPD)
#define ITM_3203                    3203 // General items / any items spare (reserved: Units, U:ANGL)
#define ITM_3204                    3204 // General items / any items spare (reserved: Units, U:ELEC)
#define ITM_3205                    3205 // General items / any items spare (reserved: Units, U:STRUCT)
#define ITM_3206                    3206 // General items / any items spare (reserved: Units, U:LIGHT)
#define ITM_3207                    3207 // General items / any items spare (reserved: Units, U:RAD)
#define ITM_3208                    3208 // General items / any items spare (reserved: Units, U:VISC)
#define ITM_3209                    3209 // General items / any items spare (reserved: Units, U:SI×)
#define ITM_3210                    3210 // General items / any items spare (reserved: Units, U.FCNS)
#define ITM_3211                    3211 // General items / any items spare (reserved: Units, U:NIM)
#define ITM_3212                    3212 // General items / any items spare (reserved: Units)
#define ITM_3213                    3213 // General items / any items spare (reserved: Units)
#define ITM_3214                    3214 // General items / any items spare (reserved: Units)
#define ITM_3215                    3215 // General items / any items spare (reserved: Units)
#define ITM_3216                    3216 // General items / any items spare (reserved: Units)
#define ITM_3217                    3217 // General items / any items spare (reserved: Units)
#define ITM_3218                    3218 // General items / any items spare (reserved: Units)
#define ITM_3219                    3219 // General items / any items spare (reserved: Units)
#define ITM_3220                    3220 // General items / any items spare (reserved: Units)
#define ITM_3221                    3221 // General items / any items spare (reserved: Units)
#define ITM_3222                    3222 // General items / any items spare (reserved: Units)
#define ITM_3223                    3223 // General items / any items spare (reserved: Units)
#define ITM_3224                    3224 // General items / any items spare (reserved: Units)
#define ITM_3225                    3225 // General items / any items spare (reserved: Units)
#define ITM_3226                    3226 // General items / any items spare (reserved: Units)
#define ITM_3227                    3227 // General items / any items spare (reserved: Units)
#define ITM_3228                    3228 // General items / any items spare (reserved: Units)
#define ITM_3229                    3229 // General items / any items spare (reserved: Units)
#define ITM_3230                    3230 // General items / any items spare (reserved: Units)
#define ITM_3231                    3231 // General items / any items spare (reserved: Units)

#define ITM_3232                    3232 // General items / any items spare (reserved: Logic extension, test to SI)
#define ITM_3233                    3233 // General items / any items spare (reserved: Logic extension, ANDbits)
#define ITM_3234                    3234 // General items / any items spare (reserved: Logic extension, ORbits)
#define ITM_3235                    3235 // General items / any items spare (reserved: Logic extension, XORbits)
#define ITM_3236                    3236 // General items / any items spare (reserved: Logic extension, spare)
#define ITM_3237                    3237 // General items / any items spare (reserved: Logic extension, spare)
#define ITM_3238                    3238 // General items / any items spare (reserved: Logic extension, spare)
#define ITM_3239                    3239 // General items / any items spare (reserved: Logic extension, spare)
#define ITM_3240                    3240 // General items / any items spare (reserved: Logic extension, spare)
#define ITM_3241                    3241 // General items / any items spare (reserved: Logic extension, spare)
#define ITM_3242                    3242 // General items / any items spare (reserved: Logic extension, spare)
#define ITM_3243                    3243 // General items / any items spare (reserved: Logic extension, spare)

// FIN12C reservations: the items and menus of the FIN-12C branch at commit c9eb9d9eb, one spare per item, reserved so that nothing else takes these numbers.
// Each module keeps two spares of its own, sixteen sit between the items and the menus at 3448 to 3463, and the four paging keys are at 1313 to 1316.
#define ITM_3244                    3244 // General items / any items spare (reserved: FIN12C, TVM2 12XN)
#define ITM_3245                    3245 // General items / any items spare (reserved: FIN12C, TVM2 12DIVI)
#define ITM_3246                    3246 // General items / any items spare (reserved: FIN12C, TVM2 spare)
#define ITM_3247                    3247 // General items / any items spare (reserved: FIN12C, TVM2 spare)
#define ITM_3248                    3248 // General items / any items spare (reserved: FIN12C, AMORT2 XTRPMT)
#define ITM_3249                    3249 // General items / any items spare (reserved: FIN12C, AMORT2 PREV)
#define ITM_3250                    3250 // General items / any items spare (reserved: FIN12C, AMORT2 BALP1)
#define ITM_3251                    3251 // General items / any items spare (reserved: FIN12C, AMORT2 BALP2)
#define ITM_3252                    3252 // General items / any items spare (reserved: FIN12C, AMORT2 SPMT)
#define ITM_3253                    3253 // General items / any items spare (reserved: FIN12C, AMORT2 SPRN)
#define ITM_3254                    3254 // General items / any items spare (reserved: FIN12C, AMORT2 TTLINT)
#define ITM_3255                    3255 // General items / any items spare (reserved: FIN12C, AMORT2 TTLPAY)
#define ITM_3256                    3256 // General items / any items spare (reserved: FIN12C, AMORT2 spare)
#define ITM_3257                    3257 // General items / any items spare (reserved: FIN12C, AMORT2 spare)
#define ITM_3258                    3258 // General items / any items spare (reserved: FIN12C, AMORT2 spare)
#define ITM_3259                    3259 // General items / any items spare (reserved: FIN12C, AMORT2 spare)
#define ITM_3260                    3260 // General items / any items spare (reserved: FIN12C, AMORT2 IPRNPC)
#define ITM_3261                    3261 // General items / any items spare (reserved: FIN12C, AMORT2 CLXTP)
#define ITM_3262                    3262 // General items / any items spare (reserved: FIN12C, AMORT2 spare)
#define ITM_3263                    3263 // General items / any items spare (reserved: FIN12C, AMORT2 spare)
#define ITM_3264                    3264 // General items / any items spare (reserved: FIN12C, CFLO CFJ)
#define ITM_3265                    3265 // General items / any items spare (reserved: FIN12C, CFLO NJ)
#define ITM_3266                    3266 // General items / any items spare (reserved: FIN12C, CFLO I)
#define ITM_3267                    3267 // General items / any items spare (reserved: FIN12C, CFLO NPV)
#define ITM_3268                    3268 // General items / any items spare (reserved: FIN12C, CFLO IRR)
#define ITM_3269                    3269 // General items / any items spare (reserved: FIN12C, CFLO CLCFLO)
#define ITM_3270                    3270 // General items / any items spare (reserved: FIN12C, CFLO INS)
#define ITM_3271                    3271 // General items / any items spare (reserved: FIN12C, CFLO DEL)
#define ITM_3272                    3272 // General items / any items spare (reserved: FIN12C, CFLO EAA)
#define ITM_3273                    3273 // General items / any items spare (reserved: FIN12C, CFLO NFV)
#define ITM_3274                    3274 // General items / any items spare (reserved: FIN12C, CFLO PB)
#define ITM_3275                    3275 // General items / any items spare (reserved: FIN12C, CFLO DPB)
#define ITM_3276                    3276 // General items / any items spare (reserved: FIN12C, CFLO PI)
#define ITM_3277                    3277 // General items / any items spare (reserved: FIN12C, CFLO SCFJ)
#define ITM_3278                    3278 // General items / any items spare (reserved: FIN12C, CFLO MIRR)
#define ITM_3279                    3279 // General items / any items spare (reserved: FIN12C, CFLO spare)
#define ITM_3280                    3280 // General items / any items spare (reserved: FIN12C, CFLO spare)
#define ITM_3281                    3281 // General items / any items spare (reserved: FIN12C, CFLO RI)
#define ITM_3282                    3282 // General items / any items spare (reserved: FIN12C, CFLO spare)
#define ITM_3283                    3283 // General items / any items spare (reserved: FIN12C, CFLO spare)
#define ITM_3284                    3284 // General items / any items spare (reserved: FIN12C, CFLO spare)
#define ITM_3285                    3285 // General items / any items spare (reserved: FIN12C, MCS ITER)
#define ITM_3286                    3286 // General items / any items spare (reserved: FIN12C, MCS VARPC)
#define ITM_3287                    3287 // General items / any items spare (reserved: FIN12C, MCS SEED)
#define ITM_3288                    3288 // General items / any items spare (reserved: FIN12C, MCS RUN)
#define ITM_3289                    3289 // General items / any items spare (reserved: FIN12C, MCS CLMCS)
#define ITM_3290                    3290 // General items / any items spare (reserved: FIN12C, MCS TMNPV)
#define ITM_3291                    3291 // General items / any items spare (reserved: FIN12C, MCS TMIRR)
#define ITM_3292                    3292 // General items / any items spare (reserved: FIN12C, MCS RNSEED)
#define ITM_3293                    3293 // General items / any items spare (reserved: FIN12C, MCS spare)
#define ITM_3294                    3294 // General items / any items spare (reserved: FIN12C, MCS spare)
#define ITM_3295                    3295 // General items / any items spare (reserved: FIN12C, CFLO MACD)
#define ITM_3296                    3296 // General items / any items spare (reserved: FIN12C, CFLO MODD)
#define ITM_3297                    3297 // General items / any items spare (reserved: FIN12C, CFLO CVXITY)
#define ITM_3298                    3298 // General items / any items spare (reserved: FIN12C, CFLO DV01)
#define ITM_3299                    3299 // General items / any items spare (reserved: FIN12C, SENS spare)
#define ITM_3300                    3300 // General items / any items spare (reserved: FIN12C, SENS spare)
#define ITM_3301                    3301 // General items / any items spare (reserved: FIN12C, ICNV CPA)
#define ITM_3302                    3302 // General items / any items spare (reserved: FIN12C, ICNV NOMTOEFF)
#define ITM_3303                    3303 // General items / any items spare (reserved: FIN12C, ICNV NOMTOCC)
#define ITM_3304                    3304 // General items / any items spare (reserved: FIN12C, ICNV EFFTOCC)
#define ITM_3305                    3305 // General items / any items spare (reserved: FIN12C, ICNV EFFTONOM)
#define ITM_3306                    3306 // General items / any items spare (reserved: FIN12C, ICNV CCTONOM)
#define ITM_3307                    3307 // General items / any items spare (reserved: FIN12C, ICNV CCTOEFF)
#define ITM_3308                    3308 // General items / any items spare (reserved: FIN12C, ICNV CLICNV)
#define ITM_3309                    3309 // General items / any items spare (reserved: FIN12C, ICNV spare)
#define ITM_3310                    3310 // General items / any items spare (reserved: FIN12C, ICNV spare)
#define ITM_3311                    3311 // General items / any items spare (reserved: FIN12C, BOND SDT)
#define ITM_3312                    3312 // General items / any items spare (reserved: FIN12C, BOND CPN)
#define ITM_3313                    3313 // General items / any items spare (reserved: FIN12C, BOND RDT)
#define ITM_3314                    3314 // General items / any items spare (reserved: FIN12C, BOND RV)
#define ITM_3315                    3315 // General items / any items spare (reserved: FIN12C, BOND YLD)
#define ITM_3316                    3316 // General items / any items spare (reserved: FIN12C, BOND PRI)
#define ITM_3317                    3317 // General items / any items spare (reserved: FIN12C, BOND AI)
#define ITM_3318                    3318 // General items / any items spare (reserved: FIN12C, BOND DUR)
#define ITM_3319                    3319 // General items / any items spare (reserved: FIN12C, BOND MACDUR)
#define ITM_3320                    3320 // General items / any items spare (reserved: FIN12C, BOND MCVXITY)
#define ITM_3321                    3321 // General items / any items spare (reserved: FIN12C, BOND DV01)
#define ITM_3322                    3322 // General items / any items spare (reserved: FIN12C, BOND CLBOND)
#define ITM_3323                    3323 // General items / any items spare (reserved: FIN12C, BOND 2CPY)
#define ITM_3324                    3324 // General items / any items spare (reserved: FIN12C, BOND ACT)
#define ITM_3325                    3325 // General items / any items spare (reserved: FIN12C, BOND ACT365)
#define ITM_3326                    3326 // General items / any items spare (reserved: FIN12C, BOND 360D)
#define ITM_3327                    3327 // General items / any items spare (reserved: FIN12C, BOND spare)
#define ITM_3328                    3328 // General items / any items spare (reserved: FIN12C, BOND spare)
#define ITM_3329                    3329 // General items / any items spare (reserved: FIN12C, DEPR COST)
#define ITM_3330                    3330 // General items / any items spare (reserved: FIN12C, DEPR SALV)
#define ITM_3331                    3331 // General items / any items spare (reserved: FIN12C, DEPR LIFE)
#define ITM_3332                    3332 // General items / any items spare (reserved: FIN12C, DEPR FCTR)
#define ITM_3333                    3333 // General items / any items spare (reserved: FIN12C, DEPR RATE)
#define ITM_3334                    3334 // General items / any items spare (reserved: FIN12C, DEPR PRF)
#define ITM_3335                    3335 // General items / any items spare (reserved: FIN12C, DEPR SLN)
#define ITM_3336                    3336 // General items / any items spare (reserved: FIN12C, DEPR SYD)
#define ITM_3337                    3337 // General items / any items spare (reserved: FIN12C, DEPR DBFIX)
#define ITM_3338                    3338 // General items / any items spare (reserved: FIN12C, DEPR DBFCTR)
#define ITM_3339                    3339 // General items / any items spare (reserved: FIN12C, DEPR DBRATE)
#define ITM_3340                    3340 // General items / any items spare (reserved: FIN12C, DEPR DBCROSS)
#define ITM_3341                    3341 // General items / any items spare (reserved: FIN12C, DEPR DBFIXPC)
#define ITM_3342                    3342 // General items / any items spare (reserved: FIN12C, DEPR DBFCTRPC)
#define ITM_3343                    3343 // General items / any items spare (reserved: FIN12C, DEPR XOVER)
#define ITM_3344                    3344 // General items / any items spare (reserved: FIN12C, DEPR CLDEPR)
#define ITM_3345                    3345 // General items / any items spare (reserved: FIN12C, DEPR P1)
#define ITM_3346                    3346 // General items / any items spare (reserved: FIN12C, DEPR P2)
#define ITM_3347                    3347 // General items / any items spare (reserved: FIN12C, DEPR SDEP)
#define ITM_3348                    3348 // General items / any items spare (reserved: FIN12C, DEPR POOL)
#define ITM_3349                    3349 // General items / any items spare (reserved: FIN12C, DEPR spare)
#define ITM_3350                    3350 // General items / any items spare (reserved: FIN12C, DEPR spare)
#define ITM_3351                    3351 // General items / any items spare (reserved: FIN12C, PROFIT COST)
#define ITM_3352                    3352 // General items / any items spare (reserved: FIN12C, PROFIT PRICE)
#define ITM_3353                    3353 // General items / any items spare (reserved: FIN12C, PROFIT MARGIN)
#define ITM_3354                    3354 // General items / any items spare (reserved: FIN12C, PROFIT MARKUP)
#define ITM_3355                    3355 // General items / any items spare (reserved: FIN12C, PROFIT CLPROFIT)
#define ITM_3356                    3356 // General items / any items spare (reserved: FIN12C, PROFIT MODMRGN)
#define ITM_3357                    3357 // General items / any items spare (reserved: FIN12C, PROFIT MODMRKUP)
#define ITM_3358                    3358 // General items / any items spare (reserved: FIN12C, PROFIT spare)
#define ITM_3359                    3359 // General items / any items spare (reserved: FIN12C, PROFIT spare)
#define ITM_3360                    3360 // General items / any items spare (reserved: FIN12C, CVP FIXCOST)
#define ITM_3361                    3361 // General items / any items spare (reserved: FIN12C, CVP VARCOST)
#define ITM_3362                    3362 // General items / any items spare (reserved: FIN12C, CVP PRICE)
#define ITM_3363                    3363 // General items / any items spare (reserved: FIN12C, CVP PROFIT)
#define ITM_3364                    3364 // General items / any items spare (reserved: FIN12C, CVP QTY)
#define ITM_3365                    3365 // General items / any items spare (reserved: FIN12C, CVP CLCVP)
#define ITM_3366                    3366 // General items / any items spare (reserved: FIN12C, CVP spare)
#define ITM_3367                    3367 // General items / any items spare (reserved: FIN12C, CVP spare)
#define ITM_3368                    3368 // General items / any items spare (reserved: FIN12C, DATE DT1)
#define ITM_3369                    3369 // General items / any items spare (reserved: FIN12C, DATE DT2)
#define ITM_3370                    3370 // General items / any items spare (reserved: FIN12C, DATE DAYS)
#define ITM_3371                    3371 // General items / any items spare (reserved: FIN12C, DATE MMTODT2)
#define ITM_3372                    3372 // General items / any items spare (reserved: FIN12C, DATE DAYS360)
#define ITM_3373                    3373 // General items / any items spare (reserved: FIN12C, DATE EOMTODT1)
#define ITM_3374                    3374 // General items / any items spare (reserved: FIN12C, DATE EOMTODT2)
#define ITM_3375                    3375 // General items / any items spare (reserved: FIN12C, DATE DAYS365Y)
#define ITM_3376                    3376 // General items / any items spare (reserved: FIN12C, DATE BUSDAYS)
#define ITM_3377                    3377 // General items / any items spare (reserved: FIN12C, DATE DAYS360Y)
#define ITM_3378                    3378 // General items / any items spare (reserved: FIN12C, DATE BOMTODT1)
#define ITM_3379                    3379 // General items / any items spare (reserved: FIN12C, DATE BOMTODT2)
#define ITM_3380                    3380 // General items / any items spare (reserved: FIN12C, DATE TODAY)
#define ITM_3381                    3381 // General items / any items spare (reserved: FIN12C, DATE CLDATE)
#define ITM_3382                    3382 // General items / any items spare (reserved: FIN12C, DATE spare)
#define ITM_3383                    3383 // General items / any items spare (reserved: FIN12C, DATE spare)
#define ITM_3384                    3384 // General items / any items spare (reserved: FIN12C, CAPM RFR)
#define ITM_3385                    3385 // General items / any items spare (reserved: FIN12C, CAPM BETA)
#define ITM_3386                    3386 // General items / any items spare (reserved: FIN12C, CAPM EMR)
#define ITM_3387                    3387 // General items / any items spare (reserved: FIN12C, CAPM EROI)
#define ITM_3388                    3388 // General items / any items spare (reserved: FIN12C, CAPM CLCAPM)
#define ITM_3389                    3389 // General items / any items spare (reserved: FIN12C, CAPM spare)
#define ITM_3390                    3390 // General items / any items spare (reserved: FIN12C, CAPM spare)
#define ITM_3391                    3391 // General items / any items spare (reserved: FIN12C, WACC MVEQT)
#define ITM_3392                    3392 // General items / any items spare (reserved: FIN12C, WACC MVDBT)
#define ITM_3393                    3393 // General items / any items spare (reserved: FIN12C, WACC MVPRF)
#define ITM_3394                    3394 // General items / any items spare (reserved: FIN12C, WACC TAXR)
#define ITM_3395                    3395 // General items / any items spare (reserved: FIN12C, WACC CEQT)
#define ITM_3396                    3396 // General items / any items spare (reserved: FIN12C, WACC CDBT)
#define ITM_3397                    3397 // General items / any items spare (reserved: FIN12C, WACC CPRF)
#define ITM_3398                    3398 // General items / any items spare (reserved: FIN12C, WACC CALC)
#define ITM_3399                    3399 // General items / any items spare (reserved: FIN12C, WACC CLWACC)
#define ITM_3400                    3400 // General items / any items spare (reserved: FIN12C, WACC spare)
#define ITM_3401                    3401 // General items / any items spare (reserved: FIN12C, WACC spare)
#define ITM_3402                    3402 // General items / any items spare (reserved: FIN12C, FX AMTA)
#define ITM_3403                    3403 // General items / any items spare (reserved: FIN12C, FX RATOB)
#define ITM_3404                    3404 // General items / any items spare (reserved: FIN12C, FX RCTOB)
#define ITM_3405                    3405 // General items / any items spare (reserved: FIN12C, FX AMTATOB)
#define ITM_3406                    3406 // General items / any items spare (reserved: FIN12C, FX AMTATOC)
#define ITM_3407                    3407 // General items / any items spare (reserved: FIN12C, FX FXATOC)
#define ITM_3408                    3408 // General items / any items spare (reserved: FIN12C, FX RBTOA)
#define ITM_3409                    3409 // General items / any items spare (reserved: FIN12C, FX RBTOC)
#define ITM_3410                    3410 // General items / any items spare (reserved: FIN12C, FX FXCTOA)
#define ITM_3411                    3411 // General items / any items spare (reserved: FIN12C, FX CLFX)
#define ITM_3412                    3412 // General items / any items spare (reserved: FIN12C, FX spare)
#define ITM_3413                    3413 // General items / any items spare (reserved: FIN12C, FX spare)
#define ITM_3414                    3414 // General items / any items spare (reserved: FIN12C, FIN 360US)
#define ITM_3415                    3415 // General items / any items spare (reserved: FIN12C, FIN 360EUR)
#define ITM_3416                    3416 // General items / any items spare (reserved: FIN12C, FIN GET_ACT)
#define ITM_3417                    3417 // General items / any items spare (reserved: FIN12C, FIN SET_ACT)
#define ITM_3418                    3418 // General items / any items spare (reserved: FIN12C, FIN GET_REGD)
#define ITM_3419                    3419 // General items / any items spare (reserved: FIN12C, FIN SET_REGD)
#define ITM_3420                    3420 // General items / any items spare (reserved: FIN12C, FIN GET_TM)
#define ITM_3421                    3421 // General items / any items spare (reserved: FIN12C, FIN SET_TM)
#define ITM_3422                    3422 // General items / any items spare (reserved: FIN12C, FIN GET_MOD)
#define ITM_3423                    3423 // General items / any items spare (reserved: FIN12C, FIN SET_MOD)
#define ITM_3424                    3424 // General items / any items spare (reserved: FIN12C, FIN GET_2CPY)
#define ITM_3425                    3425 // General items / any items spare (reserved: FIN12C, FIN SET_2CPY)
#define ITM_3426                    3426 // General items / any items spare (reserved: FIN12C, FIN GET_RNSD)
#define ITM_3427                    3427 // General items / any items spare (reserved: FIN12C, FIN SET_RNSD)
#define ITM_3428                    3428 // General items / any items spare (reserved: FIN12C, FIN GET_POOL)
#define ITM_3429                    3429 // General items / any items spare (reserved: FIN12C, FIN SET_POOL)
#define ITM_3430                    3430 // General items / any items spare (reserved: FIN12C, AMORT P1)
#define ITM_3431                    3431 // General items / any items spare (reserved: FIN12C, AMORT P2)
#define ITM_3432                    3432 // General items / any items spare (reserved: FIN12C, AMORT INT)
#define ITM_3433                    3433 // General items / any items spare (reserved: FIN12C, AMORT NXT)
#define ITM_3434                    3434 // General items / any items spare (reserved: FIN12C, FIN FFIX)
#define ITM_3435                    3435 // General items / any items spare (reserved: FIN12C, FIN FSIG)
#define ITM_3436                    3436 // General items / any items spare (reserved: FIN12C, FIN RGCHN)
#define ITM_3437                    3437 // General items / any items spare (reserved: FIN12C, FIN RGEUR)
#define ITM_3438                    3438 // General items / any items spare (reserved: FIN12C, FIN RGJP)
#define ITM_3439                    3439 // General items / any items spare (reserved: FIN12C, FIN RGUS)
#define ITM_3440                    3440 // General items / any items spare (reserved: FIN12C, FIN RGCAN)
#define ITM_3441                    3441 // General items / any items spare (reserved: FIN12C, FIN CRDGS)
#define ITM_3442                    3442 // General items / any items spare (reserved: FIN12C, FIN CRCENT)
#define ITM_3443                    3443 // General items / any items spare (reserved: FIN12C, FIN RGIND)
#define ITM_3444                    3444 // General items / any items spare (reserved: FIN12C, DEPR DEP)
#define ITM_3445                    3445 // General items / any items spare (reserved: FIN12C, DEPR RBV)
#define ITM_3446                    3446 // General items / any items spare (reserved: FIN12C, DEPR RDV)
#define ITM_3447                    3447 // General items / any items spare (reserved: FIN12C, DEPR TABLE)
#define ITM_3448                    3448 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3449                    3449 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3450                    3450 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3451                    3451 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3452                    3452 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3453                    3453 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3454                    3454 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3455                    3455 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3456                    3456 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3457                    3457 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3458                    3458 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3459                    3459 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3460                    3460 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3461                    3461 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3462                    3462 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3463                    3463 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3464                    3464 // General items / any items spare (reserved: FIN12C, menu FREGION)
#define ITM_3465                    3465 // General items / any items spare (reserved: FIN12C, menu FINPREF)
#define ITM_3466                    3466 // General items / any items spare (reserved: FIN12C, menu FX)
#define ITM_3467                    3467 // General items / any items spare (reserved: FIN12C, menu WACC)
#define ITM_3468                    3468 // General items / any items spare (reserved: FIN12C, menu CAPM)
#define ITM_3469                    3469 // General items / any items spare (reserved: FIN12C, menu DATE)
#define ITM_3470                    3470 // General items / any items spare (reserved: FIN12C, menu CVP)
#define ITM_3471                    3471 // General items / any items spare (reserved: FIN12C, menu PROFIT)
#define ITM_3472                    3472 // General items / any items spare (reserved: FIN12C, menu DEPR)
#define ITM_3473                    3473 // General items / any items spare (reserved: FIN12C, menu BOND)
#define ITM_3474                    3474 // General items / any items spare (reserved: FIN12C, menu ICNV)
#define ITM_3475                    3475 // General items / any items spare (reserved: FIN12C, menu SENS)
#define ITM_3476                    3476 // General items / any items spare (reserved: FIN12C, menu MCS)
#define ITM_3477                    3477 // General items / any items spare (reserved: FIN12C, menu DEPRTB)
#define ITM_3478                    3478 // General items / any items spare (reserved: FIN12C, menu CFLO)
#define ITM_3479                    3479 // General items / any items spare (reserved: FIN12C, menu AMORT2)
#define ITM_3480                    3480 // General items / any items spare (reserved: FIN12C, menu TVM2)
#define ITM_3481                    3481 // General items / any items spare (reserved: FIN12C, MSDCF N1)
#define ITM_3482                    3482 // General items / any items spare (reserved: FIN12C, MSDCF N2)
#define ITM_3483                    3483 // General items / any items spare (reserved: FIN12C, MSDCF TV)
#define ITM_3484                    3484 // General items / any items spare (reserved: FIN12C, MSDCF TVPC)
#define ITM_3485                    3485 // General items / any items spare (reserved: FIN12C, MSDCF PV1)
#define ITM_3486                    3486 // General items / any items spare (reserved: FIN12C, MSDCF PV2)
#define ITM_3487                    3487 // General items / any items spare (reserved: FIN12C, MSDCF PV3)
#define ITM_3488                    3488 // General items / any items spare (reserved: FIN12C, EXPTVM)
#define ITM_3489                    3489 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3490                    3490 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3491                    3491 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3492                    3492 // General items / any items spare (reserved: FIN12C, FRM menu)
#define ITM_3493                    3493 // General items / any items spare (reserved: FIN12C, FIN GET_RM)
#define ITM_3494                    3494 // General items / any items spare (reserved: FIN12C, FIN SET_RM)
#define ITM_3495                    3495 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3496                    3496 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3497                    3497 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3498                    3498 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3499                    3499 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3500                    3500 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3501                    3501 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3502                    3502 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3503                    3503 // General items / any items spare (reserved: FIN12C, FIN spare)
#define ITM_3504                    3504 // General items / any items spare (reserved: FIN12C, menu FREGION)
#define ITM_3505                    3505 // General items / any items spare (reserved: FIN12C, menu FINPREF)
#define ITM_3506                    3506 // General items / any items spare (reserved: FIN12C, menu FX)
#define ITM_3507                    3507 // General items / any items spare (reserved: FIN12C, menu WACC)
#define ITM_3508                    3508 // General items / any items spare (reserved: FIN12C, menu CAPM)
#define ITM_3509                    3509 // General items / any items spare (reserved: FIN12C, menu DATE)
#define ITM_3510                    3510 // General items / any items spare (reserved: FIN12C, menu CVP)
#define ITM_3511                    3511 // General items / any items spare (reserved: FIN12C, menu PROFIT)
#define ITM_3512                    3512 // General items / any items spare (reserved: FIN12C, menu DEPR)
#define ITM_3513                    3513 // General items / any items spare (reserved: FIN12C, menu BOND)
#define ITM_3514                    3514 // General items / any items spare (reserved: FIN12C, menu ICNV)
#define ITM_3515                    3515 // General items / any items spare (reserved: FIN12C, menu SENS)
#define ITM_3516                    3516 // General items / any items spare (reserved: FIN12C, menu MCS)
#define ITM_3517                    3517 // General items / any items spare (reserved: FIN12C, menu DEPRTB)
#define ITM_3518                    3518 // General items / any items spare (reserved: FIN12C, menu CFLO)
#define ITM_3519                    3519 // General items / any items spare (reserved: FIN12C, menu AMORT2)
#define ITM_3520                    3520 // General items / any items spare (reserved: FIN12C, menu TVM)
#define ITM_3521                    3521 // General items / any items spare (reserved: FIN12C, menu GGM)
#define ITM_3522                    3522 // General items / any items spare (reserved: FIN12C, menu BSM)
#define ITM_3523                    3523 // General items / any items spare (reserved: FIN12C, menu MSDCF)
#define ITM_3524                    3524 // General items / any items spare (reserved: FIN12C, menu FRM)
#define ITM_3525                    3525 // General items / any items spare (reserved: GUM, UNCERT CALC)
#define ITM_3526                    3526 // General items / any items spare (reserved: GUM, UNCERT BUDGET)
#define ITM_3527                    3527 // General items / any items spare (reserved: GUM, UNCERT EXPAND)
#define ITM_3528                    3528 // General items / any items spare (reserved: GUM, UNCERT EDIT)
#define ITM_3529                    3529 // General items / any items spare (reserved: GUM, UNCERT CORR)
#define ITM_3530                    3530 // General items / any items spare (reserved: GUM, UNCERT MEASURAND)
#define ITM_3531                    3531 // General items / any items spare (reserved: GUM, UNCERT COVP)
#define ITM_3532                    3532 // General items / any items spare (reserved: GUM, UNCT menu)
#define ITM_3533                    3533 // General items / any items spare (reserved: GUM, spare)
#define ITM_3534                    3534 // General items / any items spare (reserved: GUM, spare)
#define ITM_3535                    3535 // General items / any items spare (reserved: GUM, spare)

#define LAST_ITEM                   3536

#define SCREENDUMP                  9875 // Used to signal screen dump operated (not to be confused with SNAP)
#define NOPARAM                     9876 // Item for function who don't need an item
#define CONFIRMED                   9877 // Confirmation for RESET, CLPALL, CLALL
#define NOT_CONFIRMED               9878 // Confirmation for RESET, CLPALL, CLALL

#define ITM_PROD_SIGN               9999 // Multiplication sign × or ·

#define INVALID_MENU                LAST_ITEM

bool_t   itemNotAvail           (int16_t itemNr);
char     *lastFuncCatalogName   (void);
char     *lastFuncSoftmenuName  (void);
int16_t  lastSTORCL             (void);
int16_t  lastFuncNo             (void);
uint16_t indirectionType        (uint16_t func);
bool_t   isFunctionOldParam16   (uint16_t func);
char     *getItemCatalogName    (int16_t itemNr);


typedef void (*function_t)(uint16_t);
typedef function_t (*param)();
function_t  getItemFunc           (int16_t itemNr);


void reallyRunFunction(int16_t func, uint16_t param);

/**
 * Runs a function.
 *
 * \param[in] func Index in the indexOfItems area of the function to run
 */
void runFunction      (int16_t func);

///**
// * Dummy function for a function (part of an item) to be coded.
// *
// * \param[in] unusedButMandatoryParameter
// */
//void fnToBeCoded      (void);

/**
 * Dummy function for an item to be coded.
 *
 * \param[in] unusedButMandatoryParameter
 */
void itemToBeCoded    (uint16_t unusedButMandatoryParameter);

/**
 * No OPeration.
 *
 * \param[in] unusedButMandatoryParameter
 */
void fnNop            (uint16_t unusedButMandatoryParameter);

/**
 * Do nothing!
 */
void doNothing        (void);
#endif // !ITEMS_H
