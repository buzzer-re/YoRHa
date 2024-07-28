#pragma once

/* Interrupts for local APIC LVT entries other than the timer. */
#define	APIC_LOCAL_INTS	240
#define	APIC_ERROR_INT	APIC_LOCAL_INTS
#define	APIC_THERMAL_INT (APIC_LOCAL_INTS + 1)
#define	APIC_CMC_INT	(APIC_LOCAL_INTS + 2)

#define	APIC_IPI_INTS	(APIC_LOCAL_INTS + 3)
#define	IPI_RENDEZVOUS	(APIC_IPI_INTS)		/* Inter-CPU rendezvous. */
#define	IPI_INVLTLB	(APIC_IPI_INTS + 1)	/* TLB Shootdown IPIs */
#define	IPI_INVLPG	(APIC_IPI_INTS + 2)
#define	IPI_INVLRNG	(APIC_IPI_INTS + 3)
#define	IPI_INVLCACHE	(APIC_IPI_INTS + 4)
/* Vector to handle bitmap based IPIs */
#define	IPI_BITMAP_VECTOR	(APIC_IPI_INTS + 6) 

/* IPIs handled by IPI_BITMAPED_VECTOR  (XXX ups is there a better place?) */
#define	IPI_AST		0 	/* Generate software trap. */
#define IPI_PREEMPT     1
#define IPI_HARDCLOCK   2
#define IPI_BITMAP_LAST IPI_HARDCLOCK
#define IPI_IS_BITMAPED(x) ((x) <= IPI_BITMAP_LAST)

#define	IPI_STOP	(APIC_IPI_INTS + 7)	/* Stop CPU until restarted. */
#define	IPI_SUSPEND	(APIC_IPI_INTS + 8)	/* Suspend CPU until restarted. */
#define	IPI_STOP_HARD	(APIC_IPI_INTS + 9)	/* Stop CPU with a NMI. */