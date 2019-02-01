/* Transmit Register */
#define IMX_MU_xTRn(x)		(0x00 + 4 * (x))
/* Receive Register */
#define IMX_MU_xRRn(x)		(0x10 + 4 * (x))
/* Status Register */
#define IMX_MU_xSR		0x20
#define IMX_MU_xSR_GIPn(x)	BIT(28 + (3 - (x)))
#define IMX_MU_xSR_RFn(x)	BIT(24 + (3 - (x)))
#define IMX_MU_xSR_TEn(x)	BIT(20 + (3 - (x)))
#define IMX_MU_xSR_BRDIP	BIT(9)

/* Control Register */
#define IMX_MU_xCR		0x24
/* General Purpose Interrupt Enable */
#define IMX_MU_xCR_GIEn(x)	BIT(28 + (3 - (x)))
/* Receive Interrupt Enable */
#define IMX_MU_xCR_RIEn(x)	BIT(24 + (3 - (x)))
/* Transmit Interrupt Enable */
#define IMX_MU_xCR_TIEn(x)	BIT(20 + (3 - (x)))
/* General Purpose Interrupt Request */
#define IMX_MU_xCR_GIRn(x)	BIT(16 + (3 - (x)))

static inline uint32_t imx_mu_read(uint32_t reg)
{
	return *((volatile uint32_t*)(MU_BASE + reg));
}

static inline void imx_mu_write(uint32_t reg, uint32_t val)
{
	*((volatile uint32_t*)(MU_BASE + reg)) = val;
}

static inline uint32_t imx_mu_xcr_rmw(uint32_t set, uint32_t clr)
{
	//unsigned long flags;
	uint32_t val;

//	spin_lock_irqsave(&priv->xcr_lock, flags);
	val = imx_mu_read(IMX_MU_xCR);
	val &= ~clr;
	val |= set;
	imx_mu_write(val, IMX_MU_xCR);
//	spin_unlock_irqrestore(&priv->xcr_lock, flags);

	return val;
}

