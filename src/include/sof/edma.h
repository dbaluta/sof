#ifndef _EDMA_H
#define _EDMA_H

#define EDMA_CH_CSR                     0x00
#define EDMA_CH_ES                      0x04
#define EDMA_CH_INT                     0x08
#define EDMA_CH_SBR                     0x0C
#define EDMA_CH_PRI                     0x10
#define EDMA_TCD_SADDR                  0x20
#define EDMA_TCD_SOFF                   0x24
#define EDMA_TCD_ATTR                   0x26
#define EDMA_TCD_NBYTES                 0x28
#define EDMA_TCD_SLAST                  0x2C
#define EDMA_TCD_DADDR                  0x30
#define EDMA_TCD_DOFF                   0x34
#define EDMA_TCD_CITER_ELINK            0x36
#define EDMA_TCD_CITER                  0x36
#define EDMA_TCD_DLAST_SGA              0x38
#define EDMA_TCD_CSR                    0x3C
#define EDMA_TCD_BITER_ELINK            0x3E
#define EDMA_TCD_BITER                  0x3E

#define EDMA_CH_SBR_RD                  BIT(22)
#define EDMA_CH_SBR_WR                  BIT(21)
#define EDMA_CH_CSR_ERQ                 BIT(0)
#define EDMA_CH_CSR_EARQ                BIT(1)
#define EDMA_CH_CSR_EEI                 BIT(2)
#define EDMA_CH_CSR_DONE                BIT(30)
#define EDMA_CH_CSR_ACTIVE              BIT(31)

#define EDMA_TCD_ATTR_DSIZE(x)          (((x) & 0x0007))
#define EDMA_TCD_ATTR_DMOD(x)           (((x) & 0x001F) << 3)
#define EDMA_TCD_ATTR_SSIZE(x)          (((x) & 0x0007) << 8)
#define EDMA_TCD_ATTR_SMOD(x)           (((x) & 0x001F) << 11)
#define EDMA_TCD_ATTR_SSIZE_8BIT        (0x0000)
#define EDMA_TCD_ATTR_SSIZE_16BIT       (0x0100)
#define EDMA_TCD_ATTR_SSIZE_32BIT       (0x0200)
#define EDMA_TCD_ATTR_SSIZE_64BIT       (0x0300)
#define EDMA_TCD_ATTR_SSIZE_16BYTE      (0x0400)
#define EDMA_TCD_ATTR_SSIZE_32BYTE      (0x0500)
#define EDMA_TCD_ATTR_SSIZE_64BYTE      (0x0600)
#define EDMA_TCD_ATTR_DSIZE_8BIT        (0x0000)
#define EDMA_TCD_ATTR_DSIZE_16BIT       (0x0001)
#define EDMA_TCD_ATTR_DSIZE_32BIT       (0x0002)
#define EDMA_TCD_ATTR_DSIZE_64BIT       (0x0003)
#define EDMA_TCD_ATTR_DSIZE_16BYTE      (0x0004)
#define EDMA_TCD_ATTR_DSIZE_32BYTE      (0x0005)
#define EDMA_TCD_ATTR_DSIZE_64BYTE      (0x0006)

#define EDMA_TCD_SOFF_SOFF(x)           (x)
#define EDMA_TCD_NBYTES_NBYTES(x)       (x)
#define EDMA_TCD_NBYTES_MLOFF(x)        (x << 10)
#define EDMA_TCD_NBYTES_DMLOE           (1 << 30)
#define EDMA_TCD_NBYTES_SMLOE           (1 << 31)
#define EDMA_TCD_SLAST_SLAST(x)         (x)
#define EDMA_TCD_DADDR_DADDR(x)         (x)
#define EDMA_TCD_CITER_CITER(x)         ((x) & 0x7FFF)
#define EDMA_TCD_DOFF_DOFF(x)           (x)
#define EDMA_TCD_DLAST_SGA_DLAST_SGA(x) (x)
#define EDMA_TCD_BITER_BITER(x)         ((x) & 0x7FFF)

#define EDMA_TCD_CSR_START              BIT(0)
#define EDMA_TCD_CSR_INT_MAJOR          BIT(1)
#define EDMA_TCD_CSR_INT_HALF           BIT(2)
#define EDMA_TCD_CSR_D_REQ              BIT(3)
#define EDMA_TCD_CSR_E_SG               BIT(4)
#define EDMA_TCD_CSR_E_LINK             BIT(5)
#define EDMA_TCD_CSR_ACTIVE             BIT(6)
#define EDMA_TCD_CSR_DONE               BIT(7)

#define DMA_MEM_TO_DEV 0
#define DMA_DEV_TO_MEM 1
#define DMA_DEV_TO_DEV 2

#define IRQSTR_QXP_ADDR       0x51080000
#define IRQSTR_QM_ADDR        0x510A0000

#define SAI_ADDR              0x59040000
#define EDMA_ADDR_SAI_TX      0x592d0000
#define EDMA_ADDR_SAI_RX      0x592c0000

#define ESAI_ADDR             0x59010000
#define EDMA_ADDR_ESAI_TX     0x59270000
#define EDMA_ADDR_ESAI_RX     0x59260000

#define ASRC_ADDR		0x59000000
#define EDMA_ADDR_ASRC_RXA	0x59200000
#define EDMA_ADDR_ASRC_TXA	0x59230000

struct fsl_edma_hw_tcd {
	uint32_t  saddr;
	uint16_t  soff;
	uint16_t  attr;
	uint32_t  nbytes;
	uint32_t  slast;
	uint32_t  daddr;
	uint16_t  doff;
	uint16_t  citer;
	uint32_t  dlast_sga;
	uint16_t  csr;
	uint16_t  biter;
} __attribute__ ((aligned(32)));

#if 0
void edma_init(volatile void *edma_addr, uint32_t type, struct nxp_edma_hw_tcd *tcd,
	       volatile void *dev_addr, volatile void *dev2_addr,
	       volatile void *dma_buf_addr,
	       uint32_t period_size, uint32_t period_count);

void edma_start(volatile void * edma_addr,
		struct nxp_edma_hw_tcd * tcd,
		uint32_t is_rx);

void edma_stop(volatile void * edma_addr);
void edma_dump(volatile void * edma_addr);
#endif

int edma_init(void);

const struct dma_ops edma_ops;

#endif
