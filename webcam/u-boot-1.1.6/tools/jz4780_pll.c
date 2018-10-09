#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>

#include <../include/config.h>

double vco_min = 3e+08;
double vco_max = 1.5e+09;
double ref_min = 183105;
double ref_max = 1.5e+09;
int nr_min = 1;
int nr_max = 64;
int nf_min = 1;
int nf_max = 4096;
int no_min = 1;
int no_max = 16;
int no_even = 1;
int nb_min = 1;
int nb_max = 4096;
int max_vco = 1;
int ref_rng = 1;

#define APLL	0
#define MPLL	1
#define VPLL	2
#define EPLL	3
int pll_sel = 0;

#define CONFIG_HEAD "/* Automatically generated - do not edit */ \n\n#ifndef __JZ4780_PLL_H__\n#define __JZ4780_PLL_H__\n\n"
#define CONFIG_TAIL "\n#endif //__JZ4780_PLL_H__\n\n"

static char *cmdname;

static int write_file(int fd, char* data, int len)
{
	if (write(fd, data, strlen(data)) != strlen(data)) {
		fprintf (stderr, "%s: Write error on %s: %s\n",
                 cmdname, "jz_serial.h", strerror(errno));
		return (-1);
	}
    
    return (0);
}

char *fmt_hex (long long num, int nbits, char *d);

char *fmt_hex (long long num, int nbits, char *d)
{
	sprintf (d, "0x%0*llX", (nbits - 1) / 4 + 1, num);
	return (d);
}

int main(int argc, char **argv)
{
	int nr, nrx, nf, nfi, no, noe, not, nor, nore, nb, first, found;
	long long nfx;
	double fin, fout, fvco;
	double val, nval, err, merr, terr;
	int x_nrx, x_no, x_nb;
	long long x_nfx;
	double x_fvco, x_err;
	char d[1024];

	int fd;
	char *pllm;
	char *plln;
	char *pllod;
	char *cppcr_m_n_od;

	cmdname = argv[0];

	terr = -1;
#if 1
	while (argc > 1) {
		if (! strcmp (argv[1], "-n")) {
			max_vco = 0;
		} else if (! strcmp (argv[1], "-r")) {
			ref_rng = 0;
		} else if ((! strcmp (argv[1], "-f")) && (argc > 2)) {
			nf = atoi (argv[2]);
			if (nf > nf_min) nf_min = nf;
			++argv;
			--argc;
		} else if ((! strcmp (argv[1], "-v")) && (argc > 2)) {
			fvco = atof (argv[2]);
			if (fvco > vco_min) vco_min = fvco;
			++argv;
			--argc;
		} else if ((! strcmp (argv[1], "-o")) && (argc > 2)) {
			pll_sel = atoi(argv[2]);
			printf("pll_sel = %d\n", pll_sel);
			++argv;
			--argc;
		} else {
			break;
		}
		++argv;
		--argc;
	}
	if ((argc == 3) || (argc == 4)) {
	//	fin = atof (argv[1]);
	//	fout = atof (argv[2]);

	//	val = fout / fin;
		if (argc == 4) {
			terr = atof (argv[3]);
			if (terr == -1) {
				terr = 1 / ((double) nf);
			}
		}
	} else {
		fprintf (stderr, "Calculates PLL settings given the input and output frequency and optional\nerror tolerance.  If max_error is specified, the optimal PLL bit settings\n are printed that achieve an output frequency within the error tolerance,\notherwise a list of settings is printed using progressively larger\nmultiplication factors to reduce the error (use -1 for error within\nresolution limit and -2 for lowest error).\n");
		fprintf (stderr, "\n");
		fprintf (stderr, "Usage: $0 [-n] [-r] [-f nf_min] [-v vco_min] fin fout [max_error]\n");
		fprintf (stderr, "\n");
		fprintf (stderr, "\t-n - do not maximize VCO frequency\n");
		fprintf (stderr, "\t-r - do not range check NR or minimum fin/NR\n");
		fprintf (stderr, "\t-f nf_min - minimum allowed NF value\n");
		fprintf (stderr, "\t-v vco_min - minimum allowed VCO frequency\n");

		exit (1);
	}
#else
	//	fin = CFG_EXTAL;
	//	fout = CFG_CPU_SPEED;
		fin = 12000000;
		fout = 1500000000;
#endif

	if ((fd = open("../include/asm-mips/jz4780_pll.h", O_RDWR | O_CREAT, 0666)) < 0) {
		fprintf (stderr, "%s: Can't open %s: %s\n",
                 cmdname,  "jz4780_pll.h", strerror(errno));
		return -1;
	}
	if (write_file(fd, CONFIG_HEAD, strlen(CONFIG_HEAD)) < 0)
        	return -1;

retry:
	fin = CFG_EXTAL;
	switch (pll_sel) {
	case APLL:
		fout = CFG_CPU_SPEED;
		break;
	case MPLL:
		fout = CFG_DDR_SPEED;
		break;
	default:
		break;
	}
	val = fout / fin;

	if (fout * no_min > vco_max) {
		fprintf (stderr, "Error:  Maximum frequency exceeded.\n");
		exit (1);
	}
	if (fout * no_max < vco_min) {
		fprintf (stderr, "Error:  Minimum frequency exceeded.\n");
		exit (1);
	}

	first = 1;
	if (terr == -1) {
		printf ("NR\tNF\tOD\tNB\tFvco\t\terror\n");
		printf ("------------------------------------------------------\n");
	} else if (terr != -2) {
		first = 0;
		if (terr == 0) terr = 1e-16;
		merr = terr;
	}
	found = 0;
	for (nfi = val; nfi < nf_max; ++nfi) {
		nr = rint (((double) nfi) / val);
		if (nr == 0) continue;
		if ((ref_rng) && (nr < nr_min)) continue;
		if (fin / ((double) nr) > ref_max) continue;
		nrx = nr;
		nf = nfx = nfi;
		nval = ((double) nfx) / ((double) nr);
		if (nf == 0) nf = 1;
		err = 1 - nval / val;

		if ((first) || (fabs (err) < merr * (1 - 1e-6))) {
			not = floor (vco_max / fout);
			for (no = (not > no_max)? no_max : not; no > no_min; --no) {
				if ((no_even) && (no & 1)) {
					continue;
				}
				if ((ref_rng) && ((nr / no) < nr_min)) continue;
				if ((nr % no) == 0) break;
			}
			if ((nr % no) != 0) continue;
			nor = ((not > no_max)? no_max : not) / no;
			nore = nf_max / nf;
			if (nor > nore) nor = nore;
			if (((no_even) && (no & 1)) && (nor & 1) && (nor > 1)) --nor;
			noe = ceil (vco_min / fout);
			if (! max_vco) {
				nore = (noe - 1) / no + 1;
				if ((no_even) && (no & 1) && (nor > nore) && (nore & 1)) ++nore;
				nor = nore;
				not = 0;	/* force next if to fail */
			}
			if ((((no * nor) < (not >> 1)) || ((no * nor) < noe)) &&
					((no * nor) < (nf_max / nf))) {
				no = nf_max / nf;
				if (no > no_max) no = no_max;
				if (no > not) no = not;
				if ((no_even) && (no & 1) && (no > 1)) --no;
				nfx *= no;
				nf *= no;
			} else {
				nrx /= no;
				nfx *= nor;
				nf *= nor;
				no *= nor;
				if (no > no_max) continue;
				if ((no_even) && (no & 1) && (no > 1)) continue;
			}
		//	printf("-->L%d: no=%d, nf=%d, nfx=%lld, nrx=%d\n", __LINE__, no, nf, nfx, nrx);

			nb = rint (nfx / 2.0);
			if (nb < nb_min) nb = nb_min;
			if (nb > nb_max) continue;

			fvco = fin / ((double) nrx) * ((double) nfx);
			if (fvco < vco_min) continue;
			if (fvco > vco_max) continue;
			if (nf < nf_min) continue;
			if ((ref_rng) && (fin / ((double) nrx) < ref_min)) continue;
			if ((ref_rng) && (nrx > nr_max)) continue;

			found = 1;
			if (terr == -2) {
				x_nrx = nrx;
				x_nfx = nfx;
				x_no = no;
				x_nb = nb;
				x_fvco = fvco;
				x_err = err;
			} else if (terr != -1) {
				break;
			}
			first = 0;
			merr = fabs (err);
			if (terr == -2) continue;
			printf ("%d\t%lld\t%d\t%d\t%e\t%#+g\n",
				nrx, nfx, no, nb, fvco, err);
		}
		if (err == 0) break;
	}
	if (! found) {
		fprintf (stderr, "Error:  No workable settings found.\n");
		exit (1);
	}
	if (terr != -1) {
		if (terr == -2) {
			nrx = x_nrx;
			nfx = x_nfx;
			no = x_no;
			nb = x_nb;
			fvco = x_fvco;
			err = x_err;
		} else if (fabs (err) >= terr * (1 - 1e-6)) {
			fprintf (stderr, "Error:  No appropriate ratio found.\n");
			exit (1);
		}

		printf ("NR = %d\n", nrx);
		printf ("NF = %lld\n", nfx);
		printf ("OD = %d\n", no);
		printf ("NB = %d\n", nb);

		printf ("\n");
		printf ("Fin  = %g\n", fin);
		printf ("Fvco = %g\n", fvco);
		printf ("Fout = %g\n", fvco / no);
		printf ("error = %+g\n", err);

		printf ("\n");
		printf ("CLKR[5:0] = %s\n", fmt_hex (nrx-1, 6, d));
		printf ("CLKF[12:0] = %s\n", fmt_hex (nfx-1, 13, d));
		printf ("CLKOD[3:0] = %s\n", fmt_hex ((no-1)*1, 4, d));
		printf ("BWADJ[11:0] = %s\n", fmt_hex (nb-1, 12, d));
	}

	switch (pll_sel) {
	case APLL:
		asprintf(&pllm, "#define APLL_M %d\n", nf - 1);
		asprintf(&plln, "#define APLL_N %d\n", nr - 1);
		asprintf(&pllod, "#define APLL_OD %d\n", no - 1);
		asprintf(&cppcr_m_n_od, "#define CPAPCR_M_N_OD ((APLL_M << 19) | (APLL_N << 13) | (APLL_OD << 9))\n");
		break;
	case MPLL:
		asprintf(&pllm, "#define MPLL_M %d\n", nf - 1);
		asprintf(&plln, "#define MPLL_N %d\n", nr - 1);
		asprintf(&pllod, "#define MPLL_OD %d\n", no - 1);
		asprintf(&cppcr_m_n_od, "#define CPMPCR_M_N_OD ((MPLL_M << 19) | (MPLL_N << 13) | (MPLL_OD << 9))\n");
		break;
	default:
		break;
	}

	if (write_file(fd, pllm, strlen(pllm)) < 0)
		goto out;
	if (write_file(fd, plln, strlen(plln)) < 0)
		goto out;
	if (write_file(fd, pllod, strlen(pllod)) < 0)
		goto out;
	if (write_file(fd, cppcr_m_n_od, strlen(cppcr_m_n_od)) < 0)
		goto out;

	if (pll_sel == APLL) {
		pll_sel++;
		goto retry;
	}

	if (write_file(fd, CONFIG_TAIL, strlen(CONFIG_TAIL)) < 0)
		goto out;
    
	fsync(fd);
    
out:    
	free(pllm);
	free(plln);
	free(pllod);
	free(cppcr_m_n_od);
    
	close(fd);
 
	exit (0);
}
