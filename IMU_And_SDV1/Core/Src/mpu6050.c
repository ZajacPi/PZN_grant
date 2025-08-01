/*
 * mpu6050.c
 *
 *  Created on: Jul 16, 2025
 *      Author: jakub
 */


#include<mpu6050.h>
#include<main.h>

#include <math.h>
#define RAD_TO_DEG 57.2957795f
#define ALPHA 0.98f  // Complementary filter constant

// These will hold global angle estimates, that we want to measure
float roll = 0.0f;
float pitch = 0.0f;

uint32_t last_time = 0;

extern I2C_HandleTypeDef hi2c1;

void mpu6050_init(){
	  printf(".................................................\n");
	printf("Checking connection...\n");
	  HAL_StatusTypeDef ret = HAL_I2C_IsDeviceReady(&hi2c1, MPU6050_ADDR, 1, 100);
	  if (ret == HAL_OK){
		  printf("Device ready!\n");
	  }
	  else{
		  printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		  printf("ERROR: Device not detected. Check connection.\n");
		  printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	  }
	  uint8_t temp_data = 0b00001000; // set to
	  ret = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, CONFIG_GYRO, 1, &temp_data, 1, 100);
	  if (ret == HAL_OK){
		  printf("Gyroscope configured!\n");
	  }
	  else{
		  printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		  printf("ERROR: Cannot configure gyroscope.\n");
		  printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	  }
	   temp_data = 0b00001000; // set to +- 4g
		  ret = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, CONFIG_ACC, 1, &temp_data, 1, 100);
		  if (ret == HAL_OK){
			  printf("Accelerometer configured!\n");
		  }
		  else{
			  printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			  printf("ERROR: Cannot configure accelerometer.\n");
			  printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		  }

	  temp_data = 0b00001000; // set to +- 4g
		  ret = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, PWR_MANAGEMENT, 1, &temp_data, 1, 100);
		  if (ret == HAL_OK){
			  printf("Power management successful,thermometer turned off, exit sleep mode!\n");
		  }
		  else{
			  printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			  printf("ERROR: Cannot configure power management.\n");
			  printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		  }
		  printf("Initialisation finished. \n");
		  printf(".................................................\n");

}

void mpu6050_read (){
	uint8_t data_buffer[2];
	int16_t x_acc;
	HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, DATA+1, 1, data_buffer, 2, 100);
	x_acc = ((int16_t)data_buffer[0] << 8) | data_buffer[1];
	printf("x axis acceleration: %d\n", x_acc);
}
void mpu6050_read_all() {
    uint8_t raw_data[14];
    int16_t acc_x, acc_y, acc_z;
    int16_t temp_raw;
    int16_t gyro_x, gyro_y, gyro_z;

    // Read 14 bytes: Accel (6), Temp (2), Gyro (6)
    if (HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, DATA_START_ADDR, 1, raw_data, 14, 100) != HAL_OK) {
        printf("Failed to read sensor data!\n");
        return;
    }

    // Combine high and low bytes
    acc_x = (int16_t)(raw_data[0] << 8 | raw_data[1]);
    acc_y = (int16_t)(raw_data[2] << 8 | raw_data[3]);
    acc_z = (int16_t)(raw_data[4] << 8 | raw_data[5]);
    temp_raw = (int16_t)(raw_data[6] << 8 | raw_data[7]);
    gyro_x = (int16_t)(raw_data[8] << 8 | raw_data[9]);
    gyro_y = (int16_t)(raw_data[10] << 8 | raw_data[11]);
    gyro_z = (int16_t)(raw_data[12] << 8 | raw_data[13]);

    // Optional: Convert to real units
    float ax_g = acc_x / 8192.0f;   // for ±4g
    float ay_g = acc_y / 8192.0f;
    float az_g = acc_z / 8192.0f;

    float gx_dps = gyro_x / 65.5f;  // for ±500°/s
    float gy_dps = gyro_y / 65.5f;
    float gz_dps = gyro_z / 65.5f;

    float temp_c = temp_raw / 340.0f + 36.53f;

    // Print results
    printf("Accel (g): X=%.2f Y=%.2f Z=%.2f\n", ax_g, ay_g, az_g);
    printf("Gyro  (°/s): X=%.2f Y=%.2f Z=%.2f\n", gx_dps, gy_dps, gz_dps);
    printf("Temp  (°C): %.2f\n", temp_c);
}

void mpu6050_complementary_filter() {
    int16_t acc_x, acc_y, acc_z;
    int16_t gyro_x, gyro_y, gyro_z;

    uint8_t data[14];
    HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, DATA_START_ADDR, 1, data, 14, 100);

    acc_x = (int16_t)(data[0] << 8 | data[1]);
    acc_y = (int16_t)(data[2] << 8 | data[3]);
    acc_z = (int16_t)(data[4] << 8 | data[5]);
    gyro_x = (int16_t)(data[8] << 8 | data[9]);
    gyro_y = (int16_t)(data[10] << 8 | data[11]);
    gyro_z = (int16_t)(data[12] << 8 | data[13]);

    // Convert to physical values (adjust if using different ranges)
    float ax = acc_x / 8192.0f;   // ±4g range (acc sensitivity = 8192 LSB/g)
    float ay = acc_y / 8192.0f;
    float az = acc_z / 8192.0f;

    float gx = gyro_x / 65.5f;    // ±500°/s range (gyro sensitivity = 65.5 LSB/°/s)
    float gy = gyro_y / 65.5f;

    // Compute delta time in seconds
    uint32_t now = HAL_GetTick();
    float dt = (now - last_time) / 1000.0f;
    last_time = now;

    // Compute angles from accelerometer
    float acc_roll = atan2(ay, az) * RAD_TO_DEG;
    float acc_pitch = atan2(-ax, sqrt(ay * ay + az * az)) * RAD_TO_DEG;

    // Complementary filter
    roll = ALPHA * (roll + gx * dt) + (1 - ALPHA) * acc_roll;
    pitch = ALPHA * (pitch + gy * dt) + (1 - ALPHA) * acc_pitch;
    // TODO: zapisywać roll, pitch oraz czas od momentu zapisu
    printf("ROLL: %.2f\tPITCH: %.2f\n", roll, pitch);
}
