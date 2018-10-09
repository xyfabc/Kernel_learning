#ifndef __JZ4775_PLL_H__
#define __JZ4775_PLL_H__

#define APLL_M 99
#define APLL_N 2
#define APLL_OD 0
#define CPAPCR_M_N_OD ((APLL_M << 19) | (APLL_N << 13) | (APLL_OD << 9))
#define MPLL_M 99
#define MPLL_N 2
#define MPLL_OD 0
#define CPMPCR_M_N_OD ((MPLL_M << 19) | (MPLL_N << 13) | (MPLL_OD << 9))

#endif //__JZ4775_PLL_H__


