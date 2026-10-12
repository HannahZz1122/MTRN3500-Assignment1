#include "Galil.h"

#include <sstream>
#include <iomanip>

#define DEFAULT_ADDRESS "192.168.0.120 -d"

// HELPER FUNCTIONS
void CheckGReturn(GReturn result) {
	if (result != G_NO_ERROR)
	{
		throw std::runtime_error("Error in DigitalOutput: " + std::to_string(result));
	}
}

Galil::Galil() {
	Functions = new EmbeddedFunctions();
	g = nullptr;
	Address = DEFAULT_ADDRESS;
	GReturn result = Functions->GOpen(Address.c_str(), &g);
	CheckGReturn(result);
	ControlParameters[0] = 0.0; // Kp
	ControlParameters[1] = 0.0; // Ki
	ControlParameters[2] = 0.0; // Kd
	setPoint = 0;
	ownsFunctions = true;
	result = 0;
}

Galil::Galil(EmbeddedFunctions* Funcs, GCStringIn address) {
	// Constructor implementation
	Functions = Funcs;
	Address = address;
	g = nullptr;
	GReturn result = Funcs->GOpen(Address.c_str(), &g);
	CheckGReturn(result);
	ControlParameters[0] = 0.0; // Kp
	ControlParameters[1] = 0.0; // Ki
	ControlParameters[2] = 0.0; // Kd
	setPoint = 0;
	ownsFunctions = false;
	result = 0;
}

Galil::Galil(const Galil& other) {
	// Copy constructor implementation
	Functions = new EmbeddedFunctions(*other.Functions);// see if has copy constructor
	g = nullptr;
	Address = other.Address;
	GReturn result = Functions->GOpen(Address.c_str(), &g);
	CheckGReturn(result);
	ControlParameters[0] = other.ControlParameters[0];
	ControlParameters[1] = other.ControlParameters[1];
	ControlParameters[2] = other.ControlParameters[2];
	setPoint = other.setPoint;
	ownsFunctions = true;
	result = 0;
}

Galil::~Galil() {
	// Destructor implementation
	Functions->GClose(g);
	if (ownsFunctions) {
		delete Functions;
	}
	Functions = nullptr;
	g = nullptr;
}

// DIGITAL OUTPUTS
void Galil::DigitalOutput(uint16_t value) {
	uint8_t lowByte = value & 0xFF;
	uint8_t highByte = (value >> 8) & 0xFF;
	std::string command = "OP " +
		std::to_string(lowByte) +
		"," +
		std::to_string(highByte) + ";";
	char buffer[256] = {};
	GSize buffer_len = 256;
	GSize bytes_returned = 0;
	result = Functions->GCommand(g, command.c_str(), buffer, buffer_len, &bytes_returned);
	CheckGReturn(result);
	return;
}

void Galil::DigitalByteOutput(bool bank, uint8_t value) {
	// Implementation for digital byte output
	std::string command;
	if (!bank)
	{
		command = "OP " +
			std::to_string(value) +
			"," + "_OP1" + ";";
	}
	else
	{
		command = "OP _OP0," +
			std::to_string(value) +
			 ";";
	}
	char buffer[256] = {};
	GSize buffer_len = 256;
	GSize bytes_returned = 0;
	result = Functions->GCommand(g, command.c_str(), buffer, buffer_len, &bytes_returned);
	CheckGReturn(result);
	return;
}

void Galil::DigitalBitOutput(bool val, uint8_t bit) {
	// Implementation for digital bit output
	// range check for bit
	if (bit > 15) {
		throw std::invalid_argument("Invalid bit number");
	}
	std::string command = "OB " + 
		std::to_string(bit) +
		"," +
		std::to_string(val) + ";";
	char buffer[256] = {};
	GSize buffer_len = 256;
	GSize bytes_returned = 0;
	result = Functions->GCommand(g, command.c_str(), buffer, buffer_len, &bytes_returned);
	CheckGReturn(result);
	return;
}

uint16_t Galil::DigitalInput() {
	uint16_t DI = 0;
	for (int i = 0; i <= 15; i++) {
		std::string command = "MG @IN[" + std::to_string(i) + "];"; // Command to read digital inputs
		char buffer[256] = {};
		GSize buffer_len = 256;
		GSize bytes_returned = 0;
		result = Functions->GCommand(g, command.c_str(), buffer, buffer_len, &bytes_returned);
		CheckGReturn(result);
		DI |= (std::stoi(std::string(buffer)) << i);
	}
	return DI;
}

uint8_t Galil::DigitalByteInput(bool bank) {
	// Implementation for digital byte input
	uint8_t DI = 0;
	int i, j;
	if (!bank) {
		i = 0;
		j = 7;
	}
	else {
		i = 8;
		j = 15;
	}
	for (; i <= j; i++) {
		std::string command = "MG @IN[" + std::to_string(i) + "];"; // Command to read digital inputs
		char buffer[256] = {};
		GSize buffer_len = 256;
		GSize bytes_returned = 0;
		result = Functions->GCommand(g, command.c_str(), buffer, buffer_len, &bytes_returned);
		CheckGReturn(result);
		if (!bank)
		{
			DI |= (std::stoi(std::string(buffer)) << i);
		}
		else
		{
			DI |= (std::stoi(std::string(buffer)) << (i - 8));
		}
	}
	return DI;
}

bool Galil::DigitalBitInput(uint8_t bit) {
	// Implementation for digital bit input
	// range check for bit
	if (bit > 15) {
		throw std::invalid_argument("Invalid bit number");
	}
	std::string command = "MG @IN[" + std::to_string(bit) + "];"; // Command to read digital inputs
	char buffer[256] = {};
	GSize buffer_len = 256;
	GSize bytes_returned = 0;
	result = Functions->GCommand(g, command.c_str(), buffer, buffer_len, &bytes_returned);
	CheckGReturn(result);
	return (std::stoi(std::string(buffer)) != 0);
}

bool Galil::CheckSuccessfulWrite() {
	// Implementation for checking successful write
	return result == 0;
}

// ANALOG FUNCITONS
float Galil::AnalogInput(uint8_t channel) {
	// Implementation for analog input
	if (channel > 7) {
		throw std::invalid_argument("Invalid channel number");
	}
	std::string command = "MG @AN[" + std::to_string(channel) + "];";
	char buffer[256] = {};
	GSize buffer_len = 256;
	GSize bytes_returned = 0;
	result = Functions->GCommand(g, command.c_str(), buffer, buffer_len, &bytes_returned);
	CheckGReturn(result); // may change this later cuz this is not a write command but for now it is fine
	return std::stof(std::string(buffer));
}

void Galil::AnalogOutput(uint8_t channel, double voltage) {
	// Implementation for analog output
	if (channel > 7) {
		throw std::invalid_argument("Invalid channel number");
	}
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(2) << voltage;
	std::string voltageStr = oss.str();
	std::string command = "AO " + std::to_string(channel) + "," + voltageStr + ";";
	char buffer[256] = {};
	GSize buffer_len = 256;
	GSize bytes_returned = 0;
	result = Functions->GCommand(g, command.c_str(), buffer, buffer_len, &bytes_returned);
	CheckGReturn(result);
}

void Galil::AnalogInputRange(uint8_t channel, uint8_t range) {
	// Implementation for analog input range
	if (channel > 7) {
		throw std::invalid_argument("Invalid channel number");
	}
	if (range > 4 || range < 1) {
		throw std::invalid_argument("Invalid range number");
	}
	std::string command = "AQ " + std::to_string(channel) + "," + std::to_string(range) + ";";
	char buffer[256] = {};
	GSize buffer_len = 256;
	GSize bytes_returned = 0;
	result = Functions->GCommand(g, command.c_str(), buffer, buffer_len, &bytes_returned);
	CheckGReturn(result);
}

// ENCODER
void Galil::WriteEncoder() {
	// Implementation for writing to encoder
	std::string command = "WE 0,0;";
	char buffer[256] = {};
	GSize buffer_len = 256;
	GSize bytes_returned = 0;
	result = Functions->GCommand(g, command.c_str(), buffer, buffer_len, &bytes_returned);
	CheckGReturn(result);
}

int Galil::ReadEncoder() {
	// Implementation for reading encoder
	std::string command = "QE 0;";
	char buffer[256] = {};
	GSize buffer_len = 256;
	GSize bytes_returned = 0;
	result = Functions->GCommand(g, command.c_str(), buffer, buffer_len, &bytes_returned);
	CheckGReturn(result);
	return std::stoi(std::string(buffer));
}

void Galil::setSetPoint(int s) {
	// Implementation for setting set point
	setPoint = s;
}

double Galil::getSetPoint() {
	// Implementation for getting set point
	return setPoint;
}

void Galil::setKp(double gain) {
	// Implementation for setting Kp
	ControlParameters[0] = gain;
}

double Galil::getKp() {
	// Implementation for getting Kp
	return ControlParameters[0];
}

void Galil::setKi(double gain) {
	// Implementation for setting Ki
	ControlParameters[1] = gain;
}

double Galil::getKi() {
	// Implementation for getting Ki
	return ControlParameters[1];
}

void Galil::setKd(double gain) {
	// Implementation for setting Kd
	ControlParameters[2] = gain;
}

double Galil::getKd() {
	// Implementation for getting Kd
	return ControlParameters[2];
}

// OPERATOR OVERLOADS
std::ostream& operator<<(std::ostream& output, Galil& galil) {
	// Implementation for operator overload
	char buffer[256] = {};
	GSize buffer_len = 256;
	GSize bytes_returned = 0;
	GReturn result = galil.Functions->GInfo(galil.g, buffer, buffer_len);
	CheckGReturn(result);
	output << "GInfo: " << std::string(buffer) << std::endl << std::endl;
	result = galil.Functions->GVersion(buffer, buffer_len);
	CheckGReturn(result);
	output << "GVersion: " << std::string(buffer) << std::endl << std::endl;
	return output;
}
// Operator overload for '<<' operator. So the user can say cout << Galil;
// This function should print out the output of GInfo and GVersion, with
// two newLines after each.

Galil& Galil::operator=(const Galil& other) {
	// Copy assignment operator implementation
	if (this != &other) {
		if (Functions != nullptr && g != nullptr) {
			Functions->GClose(g);
		}
		if (ownsFunctions) {
			delete Functions;
		}
		Functions = new EmbeddedFunctions(*other.Functions);
		g = nullptr;
		Address = other.Address;
		GReturn resultTemp = Functions->GOpen(Address.c_str(), &g);
		CheckGReturn(resultTemp);
		ControlParameters[0] = other.ControlParameters[0];
		ControlParameters[1] = other.ControlParameters[1];
		ControlParameters[2] = other.ControlParameters[2];
		setPoint = other.setPoint;
		ownsFunctions = true;
		result = other.result;
	}
	return *this;
}
