#ifndef __SEN219
#define __SEN219

		#define STATUS_OK  0
		#define BUFF_SIZE  60

		//--- REGISTERS CODE FOR THE INA219
		#define CONFIG_REG  0x00
		#define SHUNT_REG   0x01
		#define BUS_REG	    0x02
		#define POWER_REG   0x03
		#define CURRENT_REG 0x04
		#define CALIB_REG   0x05

		//--- DEFAULT CONFIGURATION VALUE FOR QARPEDIEM REQUIREMENTS:
		//--- 16V range, 12bits resolution, without averaging, shunt & bus
		//--- For other configurations look at the page 19 of the INA219 datasheet
		#define FAST_CONFIG 0x1E47 

		// -- Calibration constants
		#define CALIBRATION_CST 		0.04096
		#define CALIBRATION_MSK			0xFFFE


		////////////////////////////////////////////////////////////////////////////////////
		//--- The methode of calculation used to calibrate the Sensor is present in the
		//--- INA219 datasheet on page 12
		unsigned int calulateCalibrationValue(float maxExpectedCurrent, float shuntResistor);

		////////////////////////////////////////////////////////////////
		//--- Opens the I2C interface on the Raspberry pie
		int I2C_init(char* filename, int* i2c_fd, int slaveAddr);


		/////////////////////////////////////////////////////////////////////////////
		//--- write() returns the number of bytes actually written.
		//--- if it doesn't match then an error occurred (e.g. no response from the device)
		int I2C_write(int i2c_fd, unsigned char *wr_buffer,int length);


		//////////////////////////////////////////////////////////////////////////////
		//--- read() returns the number of bytes actually read.
		//--- if it doesn't match then an error occurred (ex no response from the device)
		int I2C_read(int i2c_fd, unsigned char *rd_buffer, int length);


		//////////////////////////////////////////////////////////////////////////////
		//--- Allows you to get the value of a specific register on the Sensor
		//--- The parameter is the registers code (DEFINED all ABOVE)
		int get(int parameter);

		//////////////////////////////////////////////////////////////////////////////
		//--- Allows you to Set the value of a specific register on the Sensor
		void set(int param, int value);

		/////////////////////////////////////////////////////////////////////////////
		//--- Gives the value in mV of the bus voltage (between IN- & GND)
		int getBusVoltage();

		//////////////////////////////////////////////////////////////////////////////
		void initSensor(float maxCurrent, float shuntResistor);


		//////////////////////////////////////////////////////////////////////////////
		//--- The main function that Initializes a given SEN219 device 
		//--- SensorADDR : is the I2C address of the Sensor
		//--- maxCurrent : is needed for calibration this is the highest expected Current at the point of measurement. refer to datasheet.
		void setVoltageSensor(int SensorADDR, float maxCurrent, float shuntResistor);
#endif