all: wc wc_multi

wc_multi: wc_multi.c wc.h wc_core.c
	gcc wc_multi.c wc_core.c -g -o wc_multi

wc: wc.c wc.h wc_core.c
	gcc wc.c wc_core.c -g -o wc

clean:
	rm -f wc wc_multi
