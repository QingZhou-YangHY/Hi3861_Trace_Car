#ifndef PCA_H
#define PCA_H
#define PCA9555_READ_ADDR     (0X4B)
#define PCA9555_WRITE_ADDR    (0X4A)
#define WRITELEN  2
/* 个人自定义PAC9555的寄存器参数值 */
#define	PCA9555_POLARITY0   0x00 // 极性反转值0(0不反转 1反转,PIN脚为输入时有效,)
#define	PCA9555_POLARITY1   0x00 // 极性反转值1
#define	PCA9555_DERECTION0  0xFF // 方向配置值0 (0输出 1输入)
#define	PCA9555_DERECTION1  0x0F // 方向配置值1  高4位输出脚，低四位输入脚
/* 定义PAC9555的寄存器地址 */
#define	PCA9555_REG_IN0     0x00 // 输入寄存器0地址
#define	PCA9555_REG_IN1     0x01 // 输入寄存器1地址
#define	PCA9555_REG_OUT0    0x02 // 输出寄存器0地址
#define	PCA9555_REG_OUT1    0x03 // 输出寄存器1地址
#define	PCA9555_REG_POL0    0x04 // 极性反转寄存器0地址(PIN脚为输入时有效)
#define	PCA9555_REG_POL1    0x05 // 极性反转寄存器1地址
#define	PCA9555_REG_CFG0    0x06 // 方向配置寄存器0地址
#define	PCA9555_REG_CFG1    0x07 // 方向配置寄存器1地址
#define RED_LED             0x09
#define GREEN_LED           0x12
#define BLUE_LED            0x24
#define WHITE_LED           0xff
#define LED_OFF             0x00
#define RIGHT_LED           0x07
#define LEFT_LED            0x38
typedef void (*PCA_EventProcFunc)(unsigned char);
void InitPCA9555(void);
void PCA_MainProc(void);
void PCA_RegisterEventProcFunc(PCA_EventProcFunc func);
void PCA_UnregisterEventProcFunc(void);
uint32_t PCA_WriteReg(uint8_t reg_addr, uint8_t reg_val);
#endif  // AHT20_H