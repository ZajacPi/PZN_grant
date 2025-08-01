/*
 * mpu6050.h
 *
 *  Created on: Jul 13, 2025
 *      Author: Piotr Zając
 */

#ifndef INC_MPU6050_H_
#define INC_MPU6050_H_
#define MPU6050_ADDR 0x68 << 1
//commands
#define CONFIG_GYRO 27
#define CONFIG_ACC 28
#define PWR_MANAGEMENT 107

#define DATA 59
#define DATA_START_ADDR     0x3B  // Start of Accel X

void mpu6050_init();
void mpu6050_read();
void mpu6050_read_all(void);
void mpu6050_complementary_filter(void);
#endif /* INC_MPU6050_H_ */
