/*
 * The file define all the common macro for the board based on the JZ4760
 */


#ifndef __JZ4770_COMMON_H__
#define __JZ4770_COMMON_H__


#define __CFG_EXTAL     (CFG_EXTAL / 1000000)
#define __CFG_APLL_OUT  (CFG_CPU_SPEED / 1000000)
#define __CFG_MPLL_OUT  (CFG_DDR_SPEED / 1000000)

/*pll_0*/ 
#if (__CFG_APLL_OUT > 1200)
	#error "APLL output can NOT more than 1200MHZ"
#elif (__CFG_APLL_OUT > 600)
	#define __APLL_BS          1
	#define __APLL_OD          0
#elif (__CFG_APLL_OUT > 300)
	#define __APLL_BS          0
	#define __APLL_OD          0
#elif (__CFG_APLL_OUT > 155)
	#define __APLL_BS          0
	#define __APLL_OD          1
#elif (__CFG_APLL_OUT > 76)
	#define __APLL_BS          0
	#define __APLL_OD          3
#elif (__CFG_APLL_OUT > 47)
	#define __APLL_BS          0
	#define __APLL_OD          7
#else
	#error "APLL ouptput can NOT less than 48"
#endif

#define __APLL_NO		0
#define NR 			(__APLL_NO + 1)
#define NO 			(__APLL_OD + 1)
#define __APLL_MO		(((__CFG_APLL_OUT / __CFG_EXTAL) * NR * NO) - 1)
#define NF 			(__APLL_MO + 1)
#define FOUT			(__CFG_EXTAL * NF / NR / NO)

#if ((__CFG_EXTAL / NR > 50) || (__CFG_EXTAL / NR < 10))
	#error "Can NOT set the value to APLL_N"
#endif

#if ((__APLL_MO > 127) || (__APLL_MO < 1))
	#error "Can NOT set the value to APLL_M"
#endif

#if (__APLL_BS == 1)
	#if (((FOUT * NO) > 1200) || ((FOUT * NO) < 500))
		#error "FVCO check failed : BS = 1"
	#endif
#elif (__APLL_BS == 0)
	#if (((FOUT * NO) > 600) || ((FOUT * NO) < 300))
		#error "FVCO check failed : BS = 0"
	#endif
#endif

#define CPAPCR_M_N_OD	((__APLL_MO << 24) | (__APLL_NO << 18) | (__APLL_OD << 16) | (__APLL_BS << 31))


/**************************************************************************************************************/

#if (__CFG_MPLL_OUT > 1200)
	#error "MPLL output can NO1T more than 1200MHZ"
#elif (__CFG_MPLL_OUT > 600)
	#define __MPLL_BS          1
	#define __MPLL_OD          0
#elif (__CFG_MPLL_OUT > 300)
	#define __MPLL_BS          0
	#define __MPLL_OD          0
#elif (__CFG_MPLL_OUT > 155)
	#define __MPLL_BS          0
	#define __MPLL_OD          1
#elif (__CFG_MPLL_OUT > 76)
	#define __MPLL_BS          0
	#define __MPLL_OD          3
#elif (__CFG_MPLL_OUT > 47)
	#define __MPLL_BS          0
	#define __MPLL_OD          7
#else
	#error "MPLL ouptput can NOT less than 48"
#endif

#define __MPLL_NO1		0
#define NR1 			(__MPLL_NO1 + 1)
#define NO1 			(__MPLL_OD + 1)
#define __MPLL_MO		(((__CFG_MPLL_OUT / __CFG_EXTAL) * NR1 * NO1) - 1)
#define NF1 			(__MPLL_MO + 1)
#define FOUT1			(__CFG_EXTAL * NF1 / NR1 / NO1)

#if ((__CFG_EXTAL / NR1 > 50) || (__CFG_EXTAL / NR1 < 10))
	#error "Can NOT set the value to MPLL_N"
#endif

#if ((__MPLL_MO > 127) || (__MPLL_MO < 1))
	#error "Can NOT set the value to MPLL_M"
#endif

#if (__MPLL_BS == 1)
	#if (((FOUT1 * NO1) > 1200) || ((FOUT1 * NO1) < 500))
		#error "FVCO1 check failed : BS1 = 1"
	#endif
#elif (__MPLL_BS == 0)
	#if (((FOUT1 * NO1) > 600) || ((FOUT1 * NO1) < 300))
		#error "FVCO1 check failed : BS1 = 0"
	#endif
#endif

#define CPMPCR_M_N_OD	((__MPLL_MO << 24) | (__MPLL_NO1 << 18) | (__MPLL_OD << 16) | (__MPLL_BS << 31))

#endif /* __JZ4770_COMMON_H__ */
