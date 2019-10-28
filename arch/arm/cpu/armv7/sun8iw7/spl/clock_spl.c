/*
**********************************************************************************************************************
*
*						           the Embedded Secure Bootloader System
*
*
*						       Copyright(C), 2006-2014, Allwinnertech Co., Ltd.
*                                           All Rights Reserved
*
* File    :
*
* By      :
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/
#include "common.h"
#include "asm/io.h"
#include "asm/armv7.h"
#include "asm/arch/cpu.h"
#include "asm/arch/ccmu.h"
#include "asm/arch/timer.h"
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
static int clk_set_divd(void)
{
	unsigned int reg_val;

	//config axi
	reg_val = readl(CCM_CPU_L2_AXI_CTRL);
	reg_val &= ~(0x03 << 8);
	reg_val |=  (0x01 << 8);
	reg_val |=  (1 << 0);
	writel(reg_val, CCM_CPU_L2_AXI_CTRL);
	
	//config ahb
	reg_val = readl(CCM_AHB1_APB1_CTRL);;
	reg_val &= ~((0x03 << 12) | (0x03 << 8) |(0x03 << 4));
	reg_val |=  (0x02 << 12);
	reg_val |=  (2 << 6);
	reg_val |=  (1 << 8);

	writel(reg_val, CCM_AHB1_APB1_CTRL);

	return 0;
}
/*******************************************************************************
*��������: set_pll
*����ԭ�ͣ�void set_pll( void )
*��������: ����CPUƵ��
*��ڲ���: void
*�� �� ֵ: void
*��    ע:
*******************************************************************************/
void set_pll( void )
{
    unsigned int reg_val;
    unsigned int i;
    //����ʱ��ΪĬ��408M

    //�л���24M
    __msdelay(300);
	reg_val = readl(CCM_CPU_L2_AXI_CTRL);
    reg_val &= ~(0x01 << 16);
    reg_val |=  (0x01 << 16);
	reg_val |=  (0x01 << 0);
    writel(reg_val, CCM_CPU_L2_AXI_CTRL);
    //��ʱ���ȴ�ʱ���ȶ�
    for(i=0; i<0x400; i++);
	//��дPLL1
    reg_val = (0x01<<12)|(0x01<<31);
    writel(reg_val, CCM_PLL1_CPUX_CTRL);
    //��ʱ���ȴ�ʱ���ȶ�
#ifndef CONFIG_FPGA
	do
	{
		reg_val = readl(CCM_PLL1_CPUX_CTRL);
	}
	while(!(reg_val & (0x1 << 28)));
#endif
    //�޸�AXI,AHB,APB��Ƶ
    clk_set_divd();
		//dma reset
	writel(readl(CCM_AHB1_RESET_CTRL)  | (1 << 6), CCM_AHB1_RESET_CTRL);
	for(i=0;i<100;i++);
	//gating clock for dma pass
	writel(readl(CCM_AHB1_GATE0_CTRL) | (1 << 6), CCM_AHB1_GATE0_CTRL);
	writel(7, (0x01c20000+0x20));
#if 0
	//��MBUS,clk src is pll6
	writel(0x80000000, CCM_MBUS_RESET_CTRL);       //Assert mbus domain
	writel(0x81000002, CCM_MBUS_SCLK_CTRL0);  //dram>600M, so mbus from 300M->400M
	//ʹ��PLL6
	writel(readl(CCM_PLL6_MOD_CTRL) | (1U << 31), CCM_PLL6_MOD_CTRL);
#endif
	//ʹ��PLL6 
	writel(readl(CCM_PLL6_MOD_CTRL) | (1U << 31), CCM_PLL6_MOD_CTRL);
	__usdelay(100);

	writel(0x00000002, CCM_MBUS_SCLK_CTRL0);
	__usdelay(1);//����MBUS�ķ�Ƶ����
	writel(0x01000002, CCM_MBUS_SCLK_CTRL0);
	__usdelay(1);//ѡ��MBUS��Դͷ
	writel(0x81000002, CCM_MBUS_SCLK_CTRL0); 
	__usdelay(1);//��MBUSʱ��

    //�л�ʱ�ӵ�COREPLL��
    reg_val = readl(CCM_CPU_L2_AXI_CTRL);
    reg_val &= ~(0x03 << 16);
    reg_val |=  (0x02 << 16);
    writel(reg_val, CCM_CPU_L2_AXI_CTRL);
	__usdelay(1000);
	CP15DMB;
	CP15ISB;
	//��GPIO
	writel(readl(CCM_APB1_GATE0_CTRL)		|	(1 << 5), CCM_APB1_GATE0_CTRL);
	writel(readl(SUNXI_RPRCM_BASE + 0x28)   | 		0x01, SUNXI_RPRCM_BASE + 0x28);
    return  ;
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
void reset_pll( void )
{
	writel(0x00010000, CCM_CPU_L2_AXI_CTRL);
	writel(0x00001000, CCM_PLL1_CPUX_CTRL);
	writel(0x00001010, CCM_AHB1_APB1_CTRL);

	return ;
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
void set_gpio_gate(void)
{
	writel(readl(CCM_APB1_GATE0_CTRL)		|	(1 << 5), CCM_APB1_GATE0_CTRL);
	writel(readl(SUNXI_RPRCM_BASE + 0x28)	|		0x01, SUNXI_RPRCM_BASE + 0x28);
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
void set_ccmu_normal(void)
{
	writel(7, CCM_SECURITY_REG);
	writel(0xfff, DMA_BASE + 0x20);
}
