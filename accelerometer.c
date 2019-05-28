#include "accelerometer.h"

#define CTRL_REG1 0x2A //address to set accelerometer to active mode
#define ACTIVE_MODE 0x01 //value to set accelerometer to active mode
#define XYZ_DATA_CFG 0X0E
#define RANGE 0x00
#define MAX_VAL 4096 //
#define BIT_8 256
#define BIT_11 2048

int initI2cBus(char* bus, int address){
	int i2cFileDesc = open(bus, O_RDWR);
	if (i2cFileDesc < 0) {
		printf("I2C: Unable to open bus for read/write (%s)\n", bus);
		perror("Error is:");
		exit(1);
	}

	int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
	if (result < 0) {
		perror("I2C: Unable to set I2C device to slave address.");
		exit(1);
	}
	return i2cFileDesc;
}

void writeI2cReg (int i2cFileDesc, 
							unsigned char regAddr, unsigned char value)
{
	unsigned char buff[2];
	buff[0] = regAddr;
	buff[1] = value;
	int res = write(i2cFileDesc, buff, 2);
	if (res != 2) {
		perror("I2C: Unable to write i2c register.");
		exit(1);
	}
}

unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr)
{
// To read a register, must first write the address
	int res = write(i2cFileDesc, &regAddr, sizeof(regAddr));
	if (res != sizeof(regAddr)) {
		perror("I2C: Unable to write to i2c register.");
		exit(1);
	}
// Now read the value and return it
	char value = 0;
	res = read(i2cFileDesc, &value, sizeof(value));
	if (res != sizeof(value)) {
		perror("I2C: Unable to read from i2c register");
		exit(1);
	}
	return value;
}

int accelerometerInit(){
	writeToFile(I2C1_BUS,"BB-I2C1");//enable i2c-1
	int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
	writeI2cReg(i2cFileDesc, CTRL_REG1, ACTIVE_MODE);
	writeI2cReg(i2cFileDesc, XYZ_DATA_CFG, RANGE);
	x = 0;
	y = 0;
	z = 0;
	Sleeeep(0.5, 0);
	return i2cFileDesc;
}


void readAccelerometer(int i2cFileDesc){
	char reg[1] = {0x00};
	write(i2cFileDesc, reg, 1);
	if(read(i2cFileDesc, data, 7) != 7)
	{
		printf("read data error \n");
	}
	// x = (data[1] & 0xFF) | ((data[2] & 0x0F) << 8);
	// y = (data[3] & 0xFF) | ((data[4] & 0x0F) << 8);
	// z = (data[5] & 0xFF) | ((data[6] & 0x0F) << 8);
	x = ((data[1] * BIT_8) + data[2]) / 16;
	y = ((data[3] * BIT_8) + data[4]) / 16;
	z = ((data[5] * BIT_8) + data[6]) / 16;
	if(x > BIT_11) x -= MAX_VAL;
	if(y > BIT_11) y -= MAX_VAL;
	if(z > BIT_11) z -= MAX_VAL;

}

int calculateDiff(int val1, int val2){
	int diff = 0;
	if (val1 > val2) diff = val1- val2;
	else diff = val2 - val1;
	
	return diff;
}

void closeI2C(int i2cFileDesc){
	close(i2cFileDesc);
}